#include "dataflow_analysis/api/dataflow.h"

#include "dataflow_analysis/api/configuration.h"
#include "dataflow_analysis/api/result.h"
#include "dataflow_analysis/plugin_dataflow.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_parameter.h"
#include "hal_core/utilities/program_arguments.h"
#include "netlist_test_utils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

namespace hal
{
    namespace
    {
        constexpr u32 register_size = 8;

        // group IDs start at 0, so this stands for no group
        constexpr u32 no_group = std::numeric_limits<u32>::max();

        std::unordered_set<Gate*> as_set(const std::vector<Gate*>& gates)
        {
            return std::unordered_set<Gate*>(gates.begin(), gates.end());
        }

        std::string read_file(const std::filesystem::path& path)
        {
            std::ifstream ifs(path);
            std::stringstream ss;
            ss << ifs.rdbuf();
            return ss.str();
        }
    }    // namespace

    class DataflowTest : public ::testing::Test
    {
    protected:
        std::unique_ptr<Netlist> m_nl;
        std::map<std::string, std::vector<Gate*>> m_registers;
        std::map<std::string, Net*> m_control;
        Gate* m_comb_gate = nullptr;
        std::filesystem::path m_sandbox;

        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            m_sandbox = test_utils::create_sandbox_directory();
            create_netlist();
            ASSERT_NE(m_nl, nullptr);
        }

        virtual void TearDown()
        {
            m_nl.reset();
            test_utils::remove_sandbox_directory();
        }

        /**
         * Four 8-bit registers a, b, c, and d that form a loop a -> b -> c -> d -> a, with b also reading itself
         * (rotated), c also reading itself, and d also reading a:
         *   a[i] = in[i] ^ d[i]         (DFFE, enable en_a)
         *   b[i] = a[i] ^ b[(i + 1) % 8]  (DFFE, enable en_b)
         *   c[i] = b[i] & c[i]            (DFFE, enable en_c)
         *   d[i] = c[i] ^ a[i]            (DFFR, reset rst)
         * So the groups have edges in both directions between each other and self-loops, and every type of
         * control pin occurs.
         */
        void create_netlist()
        {
            m_nl                  = test_utils::create_empty_netlist();
            const GateLibrary* gl = m_nl->get_gate_library();

            auto global_input = [this](const std::string& name) {
                Net* net = m_nl->create_net(name);
                net->mark_global_input_net();
                return net;
            };
            auto output = [this](Gate* gate, const std::string& pin, const std::string& name) {
                Net* net = m_nl->create_net(name);
                net->add_source(gate, pin);
                return net;
            };

            m_control["clk"] = global_input("clk");
            m_control["rst"] = global_input("rst");
            for (const auto* reg : {"a", "b", "c"})
            {
                m_control[std::string("en_") + reg] = global_input(std::string("en_") + reg);
            }

            std::map<std::string, std::vector<Net*>> q;
            for (const auto* reg : {"a", "b", "c", "d"})
            {
                const bool has_enable = std::string(reg) != "d";
                for (u32 i = 0; i < register_size; i++)
                {
                    const std::string name = std::string(reg) + std::to_string(i);
                    Gate* ff               = m_nl->create_gate(gl->get_gate_type_by_name(has_enable ? "DFFE" : "DFFR"), name);
                    m_control["clk"]->add_destination(ff, "CLK");
                    if (has_enable)
                    {
                        m_control[std::string("en_") + reg]->add_destination(ff, "EN");
                    }
                    else
                    {
                        m_control["rst"]->add_destination(ff, "R");
                    }
                    m_registers[reg].push_back(ff);
                    q[reg].push_back(output(ff, "Q", "q_" + name));
                }
            }

            // data input of a flip-flop: a gate of the given type over two nets
            auto data_input = [&](const std::string& type, Net* in0, Net* in1, Gate* ff) {
                Gate* gate = m_nl->create_gate(gl->get_gate_type_by_name(type), "d_" + ff->get_name());
                in0->add_destination(gate, "I0");
                in1->add_destination(gate, "I1");
                output(gate, "O", "n_" + ff->get_name())->add_destination(ff, "D");
                return gate;
            };

            for (u32 i = 0; i < register_size; i++)
            {
                Gate* gate = data_input("XOR2", global_input("in" + std::to_string(i)), q["d"][i], m_registers["a"][i]);
                if (i == 0)
                {
                    m_comb_gate = gate;
                }
                data_input("XOR2", q["a"][i], q["b"][(i + 1) % register_size], m_registers["b"][i]);
                data_input("AND2", q["b"][i], q["c"][i], m_registers["c"][i]);
                data_input("XOR2", q["c"][i], q["a"][i], m_registers["d"][i]);
            }
        }

        dataflow::Configuration flip_flop_config() const
        {
            return dataflow::Configuration(m_nl.get()).with_flip_flops();
        }

        static dataflow::Result analyze(const dataflow::Configuration& config)
        {
            auto res = dataflow::analyze(config);
            if (res.is_error())
            {
                ADD_FAILURE() << "dataflow analysis failed: " << res.get_error().get();
            }
            return res.get();
        }

        dataflow::Result analyze() const
        {
            return analyze(flip_flop_config());
        }

        /**
         * The ID of the group that consists of exactly the given gates, or no_group if there is none.
         */
        static u32 group_of(const dataflow::Result& res, const std::vector<Gate*>& gates)
        {
            const auto wanted = as_set(gates);
            for (const auto& [group_id, group] : res.get_groups())
            {
                if (group == wanted)
                {
                    return group_id;
                }
            }
            return no_group;
        }

        /**
         * The group IDs of the registers a, b, c, and d. Fails the test if a register is not a group of its own.
         */
        std::map<std::string, u32> register_groups(const dataflow::Result& res) const
        {
            std::map<std::string, u32> ids;
            for (const auto& [name, gates] : m_registers)
            {
                ids[name] = group_of(res, gates);
                EXPECT_NE(ids[name], no_group) << "register " << name << " is not a group of its own";
            }
            return ids;
        }

        /**
         * Checks that every gate is in exactly one group, and that the successors and predecessors of every group are
         * exactly the groups of the successors and predecessors of its gates.
         */
        static void expect_consistent(const dataflow::Result& res)
        {
            const auto& groups = res.get_groups();

            std::unordered_set<Gate*> seen;
            for (const auto& [group_id, gates] : groups)
            {
                EXPECT_FALSE(gates.empty()) << "group " << group_id << " is empty";
                for (auto* gate : gates)
                {
                    EXPECT_TRUE(seen.insert(gate).second) << "gate " << gate->get_name() << " is in more than one group";
                    const auto parent = res.get_group_id_of_gate(gate);
                    ASSERT_TRUE(parent.is_ok()) << "gate " << gate->get_name() << " has no group";
                    EXPECT_EQ(parent.get(), group_id) << "gate " << gate->get_name() << " points to the wrong group";
                }
            }
            EXPECT_EQ(seen, as_set(res.get_gates()));

            for (const auto& [group_id, gates] : groups)
            {
                std::unordered_set<u32> expected_successors, expected_predecessors;
                for (auto* gate : gates)
                {
                    const auto successors   = res.get_gate_successors(gate);
                    const auto predecessors = res.get_gate_predecessors(gate);
                    ASSERT_TRUE(successors.is_ok());
                    ASSERT_TRUE(predecessors.is_ok());
                    for (auto* suc : successors.get())
                    {
                        const auto suc_group = res.get_group_id_of_gate(suc);
                        ASSERT_TRUE(suc_group.is_ok()) << "successor " << suc->get_name() << " has no group";
                        expected_successors.insert(suc_group.get());
                    }
                    for (auto* pred : predecessors.get())
                    {
                        const auto pred_group = res.get_group_id_of_gate(pred);
                        ASSERT_TRUE(pred_group.is_ok()) << "predecessor " << pred->get_name() << " has no group";
                        expected_predecessors.insert(pred_group.get());
                    }
                }

                const auto successors   = res.get_group_successors(group_id);
                const auto predecessors = res.get_group_predecessors(group_id);
                ASSERT_TRUE(successors.is_ok());
                ASSERT_TRUE(predecessors.is_ok());
                EXPECT_EQ(successors.get(), expected_successors) << "successors of group " << group_id;
                EXPECT_EQ(predecessors.get(), expected_predecessors) << "predecessors of group " << group_id;
            }
        }

        static std::vector<u32> sorted_group_ids(const dataflow::Result& res)
        {
            std::vector<u32> ids;
            for (const auto& [group_id, _] : res.get_groups())
            {
                ids.push_back(group_id);
            }
            std::sort(ids.begin(), ids.end());
            return ids;
        }

        /**
         * Merges the given groups, checks that the new group holds exactly their gates and that the old groups are
         * gone, and returns the ID of the new group.
         */
        static u32 merge_and_check(dataflow::Result& res, const std::vector<u32>& group_ids)
        {
            std::unordered_set<Gate*> gates;
            for (const auto group_id : group_ids)
            {
                const auto& group = res.get_groups().at(group_id);
                gates.insert(group.begin(), group.end());
            }

            const auto merged = res.merge_groups(group_ids);
            if (merged.is_error())
            {
                ADD_FAILURE() << "merging failed: " << merged.get_error().get();
                return no_group;
            }
            const u32 new_id = merged.get();

            const auto new_gates = res.get_gates_of_group(new_id);
            EXPECT_TRUE(new_gates.is_ok());
            if (new_gates.is_ok())
            {
                EXPECT_EQ(new_gates.get(), gates);
            }
            for (const auto group_id : group_ids)
            {
                EXPECT_TRUE(res.get_gates_of_group(group_id).is_error()) << "merged group " << group_id << " still exists";
            }
            expect_consistent(res);
            return new_id;
        }

        std::vector<Module*> dana_modules() const
        {
            return m_nl->get_modules([](const Module* m) { return m->get_name().rfind("DANA_", 0) == 0; });
        }

        static std::map<std::string, PinGroup<ModulePin>*> pin_groups_by_name(const Module* module)
        {
            std::map<std::string, PinGroup<ModulePin>*> pin_groups;
            for (auto* pin_group : module->get_pin_groups())
            {
                pin_groups[pin_group->get_name()] = pin_group;
            }
            return pin_groups;
        }
    };

    // ---------------------------------------------------------------------------------------------------------------
    // Configuration

    TEST_F(DataflowTest, configuration_defaults)
    {
        const dataflow::Configuration config(m_nl.get());
        EXPECT_EQ(config.netlist, m_nl.get());
        EXPECT_EQ(config.min_group_size, 8u);
        EXPECT_TRUE(config.expected_sizes.empty());
        EXPECT_TRUE(config.known_gate_groups.empty());
        EXPECT_TRUE(config.known_net_groups.empty());
        EXPECT_TRUE(config.gate_types.empty());
        EXPECT_TRUE(config.control_pin_types.empty());
        EXPECT_FALSE(config.enable_stages);
        EXPECT_FALSE(config.enforce_type_consistency);
    }

    TEST_F(DataflowTest, configuration_setters_are_chainable)
    {
        dataflow::Configuration config(m_nl.get());
        EXPECT_EQ(&config.with_min_group_size(4), &config);
        EXPECT_EQ(config.min_group_size, 4u);
        EXPECT_EQ(&config.with_expected_sizes({8, 16}), &config);
        EXPECT_EQ(config.expected_sizes, (std::vector<u32>{8, 16}));
        EXPECT_EQ(&config.with_stage_identification(), &config);
        EXPECT_TRUE(config.enable_stages);
        config.with_stage_identification(false);
        EXPECT_FALSE(config.enable_stages);
        EXPECT_EQ(&config.with_type_consistency(), &config);
        EXPECT_TRUE(config.enforce_type_consistency);
        config.with_type_consistency(false);
        EXPECT_FALSE(config.enforce_type_consistency);
    }

    TEST_F(DataflowTest, configuration_gate_and_control_pin_types)
    {
        const GateLibrary* gl = m_nl->get_gate_library();
        const GateType* dff   = gl->get_gate_type_by_name("DFF");
        const GateType* dffe  = gl->get_gate_type_by_name("DFFE");

        dataflow::Configuration config(m_nl.get());
        config.with_gate_types(std::set<const GateType*>{dff});
        config.with_gate_types(std::set<const GateType*>{dffe});
        EXPECT_EQ(config.gate_types, (std::set<const GateType*>{dff, dffe}));
        config.with_gate_types(std::set<const GateType*>{dffe}, true);
        EXPECT_EQ(config.gate_types, (std::set<const GateType*>{dffe}));

        std::set<const GateType*> ff_types;
        for (const auto& [_, type] : gl->get_gate_types([](const GateType* gt) { return gt->has_property(GateTypeProperty::ff); }))
        {
            ff_types.insert(type);
        }
        ASSERT_FALSE(ff_types.empty());
        dataflow::Configuration by_property(m_nl.get());
        by_property.with_gate_types(std::set<GateTypeProperty>{GateTypeProperty::ff});
        EXPECT_EQ(by_property.gate_types, ff_types);

        dataflow::Configuration flip_flops(m_nl.get());
        flip_flops.with_gate_types(std::set<const GateType*>{gl->get_gate_type_by_name("XOR2")});
        flip_flops.with_flip_flops();
        EXPECT_EQ(flip_flops.gate_types, ff_types) << "with_flip_flops overwrites the gate types";
        EXPECT_EQ(flip_flops.control_pin_types, (std::set<PinType>{PinType::clock, PinType::enable, PinType::set, PinType::reset}));

        dataflow::Configuration pins(m_nl.get());
        pins.with_control_pin_types({PinType::clock});
        pins.with_control_pin_types({PinType::enable});
        EXPECT_EQ(pins.control_pin_types, (std::set<PinType>{PinType::clock, PinType::enable}));
        pins.with_control_pin_types({PinType::reset}, true);
        EXPECT_EQ(pins.control_pin_types, (std::set<PinType>{PinType::reset}));
    }

    TEST_F(DataflowTest, configuration_known_groups)
    {
        Module* module = m_nl->create_module("known", m_nl->get_top_module(), m_registers["a"]);
        ASSERT_NE(module, nullptr);

        dataflow::Configuration config(m_nl.get());
        config.with_known_groups(std::vector<Module*>{module});
        ASSERT_EQ(config.known_gate_groups.size(), 1u);
        EXPECT_EQ(as_set(config.known_gate_groups[0]), as_set(m_registers["a"]));

        config.with_known_groups(std::vector<std::vector<Gate*>>{m_registers["b"]});
        ASSERT_EQ(config.known_gate_groups.size(), 2u);
        EXPECT_EQ(as_set(config.known_gate_groups[1]), as_set(m_registers["b"]));

        config.with_known_groups(std::vector<std::vector<Gate*>>{m_registers["c"]}, true);
        ASSERT_EQ(config.known_gate_groups.size(), 1u);
        EXPECT_EQ(as_set(config.known_gate_groups[0]), as_set(m_registers["c"]));

        std::vector<u32> ids;
        for (auto* gate : m_registers["d"])
        {
            ids.push_back(gate->get_id());
        }
        config.with_known_groups(std::vector<std::vector<u32>>{ids}, true);
        ASSERT_EQ(config.known_gate_groups.size(), 1u);
        EXPECT_EQ(as_set(config.known_gate_groups[0]), as_set(m_registers["d"]));

        config.with_known_groups(std::unordered_map<u32, std::unordered_set<Gate*>>{{42, as_set(m_registers["a"])}});
        ASSERT_EQ(config.known_gate_groups.size(), 2u);
        EXPECT_EQ(as_set(config.known_gate_groups[1]), as_set(m_registers["a"]));
    }

    TEST_F(DataflowTest, configuration_known_structures)
    {
        const GateType* xor2  = m_comb_gate->get_type();
        const auto pin_groups = xor2->get_pin_groups();
        const auto xor2_gates = m_nl->get_gates([xor2](const Gate* g) { return g->get_type() == xor2; });
        ASSERT_FALSE(pin_groups.empty());

        // by gate: one net group per pin group of its type, holding the nets at these pins
        dataflow::Configuration by_gate(m_nl.get());
        by_gate.with_known_structures(std::vector<Gate*>{m_comb_gate});
        ASSERT_EQ(by_gate.known_net_groups.size(), pin_groups.size());
        std::unordered_set<Net*> nets;
        for (const auto& group : by_gate.known_net_groups)
        {
            nets.insert(group.begin(), group.end());
        }
        std::unordered_set<Net*> gate_nets;
        for (auto* net : m_comb_gate->get_fan_in_nets())
        {
            gate_nets.insert(net);
        }
        for (auto* net : m_comb_gate->get_fan_out_nets())
        {
            gate_nets.insert(net);
        }
        EXPECT_EQ(nets, gate_nets);

        // by gate type: the same for every gate of the type
        dataflow::Configuration by_type(m_nl.get());
        by_type.with_known_structures(std::unordered_set<const GateType*>{xor2});
        EXPECT_EQ(by_type.known_net_groups.size(), pin_groups.size() * xor2_gates.size());

        // with selected pin groups only
        dataflow::Configuration by_pin_group(m_nl.get());
        by_pin_group.with_known_structures(std::vector<std::pair<Gate*, std::vector<PinGroup<GatePin>*>>>{{m_comb_gate, {pin_groups.front()}}});
        EXPECT_EQ(by_pin_group.known_net_groups.size(), 1u);

        // by module: one net group per pin group of the module
        Module* module = m_nl->create_module("structure", m_nl->get_top_module(), m_registers["a"]);
        ASSERT_NE(module, nullptr);
        dataflow::Configuration by_module(m_nl.get());
        by_module.with_known_structures(std::vector<Module*>{module});
        EXPECT_EQ(by_module.known_net_groups.size(), module->get_pin_groups().size());

        // overwrite
        by_module.with_known_structures(std::vector<Gate*>{m_comb_gate}, true);
        EXPECT_EQ(by_module.known_net_groups.size(), pin_groups.size());
    }

    TEST_F(DataflowTest, configuration_without_netlist)
    {
        dataflow::Configuration config(nullptr);
        config.with_flip_flops();
        EXPECT_TRUE(config.gate_types.empty());
        config.with_gate_types(std::set<GateTypeProperty>{GateTypeProperty::ff});
        EXPECT_TRUE(config.gate_types.empty());
        config.with_known_groups(std::vector<std::vector<u32>>{{1, 2}});
        EXPECT_TRUE(config.known_gate_groups.empty());
        config.with_known_structures(std::unordered_set<const GateType*>{m_comb_gate->get_type()});
        EXPECT_TRUE(config.known_net_groups.empty());
        EXPECT_TRUE(dataflow::analyze(config).is_error());
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Analysis

    TEST_F(DataflowTest, analyze_rejects_incomplete_configuration)
    {
        EXPECT_TRUE(dataflow::analyze(dataflow::Configuration(nullptr)).is_error());
        EXPECT_TRUE(dataflow::analyze(dataflow::Configuration(m_nl.get())).is_error()) << "no gate types";
        EXPECT_TRUE(dataflow::analyze(dataflow::Configuration(m_nl.get()).with_gate_types(std::set<GateTypeProperty>{GateTypeProperty::ff})).is_error()) << "no control pin types";
    }

    TEST_F(DataflowTest, analyze_finds_registers)
    {
        const auto res = analyze();
        EXPECT_EQ(res.get_netlist(), m_nl.get());
        EXPECT_EQ(res.get_gates().size(), 4 * register_size);
        EXPECT_EQ(res.get_groups().size(), 4u);
        register_groups(res);
        expect_consistent(res);
    }

    TEST_F(DataflowTest, analyze_derives_dataflow_graph)
    {
        const auto res = analyze();
        auto id        = register_groups(res);
        ASSERT_FALSE(HasFailure());

        const std::map<std::string, std::set<std::string>> successors = {{"a", {"b", "d"}}, {"b", {"b", "c"}}, {"c", {"c", "d"}}, {"d", {"a"}}};
        std::map<std::string, std::set<std::string>> predecessors;
        for (const auto& [reg, sucs] : successors)
        {
            for (const auto& suc : sucs)
            {
                predecessors[suc].insert(reg);
            }
        }
        auto ids_of = [&id](const std::set<std::string>& regs) {
            std::unordered_set<u32> ids;
            for (const auto& reg : regs)
            {
                ids.insert(id[reg]);
            }
            return ids;
        };
        for (const auto& [reg, sucs] : successors)
        {
            EXPECT_EQ(res.get_group_successors(id[reg]).get(), ids_of(sucs)) << "successors of " << reg;
            EXPECT_EQ(res.get_group_predecessors(id[reg]).get(), ids_of(predecessors[reg])) << "predecessors of " << reg;
        }

        // the same on the level of single flip-flops
        auto& a = m_registers["a"];
        auto& b = m_registers["b"];
        auto& c = m_registers["c"];
        auto& d = m_registers["d"];
        for (u32 i = 0; i < register_size; i++)
        {
            const u32 next = (i + 1) % register_size;
            const u32 prev = (i + register_size - 1) % register_size;
            EXPECT_EQ(res.get_gate_successors(a[i]).get(), (std::unordered_set<Gate*>{b[i], d[i]}));
            EXPECT_EQ(res.get_gate_predecessors(a[i]).get(), (std::unordered_set<Gate*>{d[i]}));
            EXPECT_EQ(res.get_gate_successors(b[i]).get(), (std::unordered_set<Gate*>{c[i], b[prev]}));
            EXPECT_EQ(res.get_gate_predecessors(b[i]).get(), (std::unordered_set<Gate*>{a[i], b[next]}));
            EXPECT_EQ(res.get_gate_successors(c[i]).get(), (std::unordered_set<Gate*>{c[i], d[i]}));
            EXPECT_EQ(res.get_gate_predecessors(c[i]).get(), (std::unordered_set<Gate*>{b[i], c[i]}));
            EXPECT_EQ(res.get_gate_successors(d[i]).get(), (std::unordered_set<Gate*>{a[i]}));
            EXPECT_EQ(res.get_gate_predecessors(d[i]).get(), (std::unordered_set<Gate*>{a[i], c[i]}));
        }

        // gates and groups that are not part of the result
        EXPECT_TRUE(res.get_group_id_of_gate(m_comb_gate).is_error());
        EXPECT_TRUE(res.get_gate_successors(m_comb_gate).is_error());
        EXPECT_TRUE(res.get_gate_predecessors(m_comb_gate).is_error());
        const u32 unknown = sorted_group_ids(res).back() + 1000;
        EXPECT_TRUE(res.get_gates_of_group(unknown).is_error());
        EXPECT_TRUE(res.get_group_successors(unknown).is_error());
        EXPECT_TRUE(res.get_group_predecessors(unknown).is_error());
    }

    TEST_F(DataflowTest, analyze_collects_control_nets)
    {
        const auto res = analyze();
        auto id        = register_groups(res);
        ASSERT_FALSE(HasFailure());

        for (const auto* reg : {"a", "b", "c"})
        {
            const std::unordered_set<Net*> enable = {m_control[std::string("en_") + reg]};
            EXPECT_EQ(res.get_group_control_nets(id[reg], PinType::enable).get(), enable) << reg;
            EXPECT_TRUE(res.get_group_control_nets(id[reg], PinType::reset).get().empty()) << reg;
            EXPECT_EQ(res.get_gate_control_nets(m_registers[reg].front(), PinType::enable).get(), enable) << reg;
        }
        EXPECT_TRUE(res.get_group_control_nets(id["d"], PinType::enable).get().empty());
        EXPECT_EQ(res.get_group_control_nets(id["d"], PinType::reset).get(), (std::unordered_set<Net*>{m_control["rst"]}));
        EXPECT_EQ(res.get_gate_control_nets(m_registers["d"].front(), PinType::reset).get(), (std::unordered_set<Net*>{m_control["rst"]}));
        for (const auto& [reg, group_id] : id)
        {
            EXPECT_EQ(res.get_group_control_nets(group_id, PinType::clock).get(), (std::unordered_set<Net*>{m_control["clk"]})) << reg;
            EXPECT_TRUE(res.get_group_control_nets(group_id, PinType::set).get().empty()) << reg;
        }

        EXPECT_TRUE(res.get_group_control_nets(sorted_group_ids(res).back() + 1000, PinType::clock).is_error());
        EXPECT_TRUE(res.get_gate_control_nets(m_comb_gate, PinType::clock).is_error());
    }

    TEST_F(DataflowTest, analyze_only_groups_target_gate_types)
    {
        const GateType* dffe = m_nl->get_gate_library()->get_gate_type_by_name("DFFE");
        const auto res =
            analyze(dataflow::Configuration(m_nl.get()).with_gate_types(std::set<const GateType*>{dffe}).with_control_pin_types({PinType::clock, PinType::enable, PinType::reset, PinType::set}));
        EXPECT_EQ(res.get_gates().size(), 3 * register_size);
        for (auto* gate : m_registers["d"])
        {
            EXPECT_TRUE(res.get_group_id_of_gate(gate).is_error()) << gate->get_name();
        }
        for (const auto* reg : {"a", "b", "c"})
        {
            EXPECT_NE(group_of(res, m_registers[reg]), no_group) << reg;
        }
        expect_consistent(res);
    }

    TEST_F(DataflowTest, analyze_keeps_known_groups)
    {
        // two halves of register a, which the analysis on its own would group together
        const std::vector<Gate*> low(m_registers["a"].begin(), m_registers["a"].begin() + register_size / 2);
        const std::vector<Gate*> high(m_registers["a"].begin() + register_size / 2, m_registers["a"].end());

        const auto res = analyze(flip_flop_config().with_known_groups(std::vector<std::vector<Gate*>>{low, high}));
        EXPECT_NE(group_of(res, low), no_group);
        EXPECT_NE(group_of(res, high), no_group);
        for (const auto* reg : {"b", "c", "d"})
        {
            EXPECT_NE(group_of(res, m_registers[reg]), no_group) << reg;
        }
        expect_consistent(res);

        // the same with known groups given as gate IDs and as a module
        std::vector<u32> low_ids;
        for (auto* gate : low)
        {
            low_ids.push_back(gate->get_id());
        }
        EXPECT_NE(group_of(analyze(flip_flop_config().with_known_groups(std::vector<std::vector<u32>>{low_ids})), low), no_group);
        Module* module = m_nl->create_module("low", m_nl->get_top_module(), low);
        ASSERT_NE(module, nullptr);
        EXPECT_NE(group_of(analyze(flip_flop_config().with_known_groups(std::vector<Module*>{module})), low), no_group);
    }

    TEST_F(DataflowTest, analyze_with_options)
    {
        // the registers of this netlist are unambiguous, so every option still finds them
        const std::vector<dataflow::Configuration> configs = {
            flip_flop_config().with_expected_sizes({register_size}),
            flip_flop_config().with_min_group_size(register_size / 2),
            flip_flop_config().with_stage_identification(),
            flip_flop_config().with_type_consistency(),
            flip_flop_config().with_known_structures(std::vector<Gate*>{m_comb_gate}),
        };
        for (const auto& config : configs)
        {
            const auto res = analyze(config);
            register_groups(res);
            expect_consistent(res);
            if (config.enforce_type_consistency)
            {
                for (const auto& [group_id, gates] : res.get_groups())
                {
                    const GateType* type = (*gates.begin())->get_type();
                    EXPECT_TRUE(std::all_of(gates.begin(), gates.end(), [type](const Gate* g) { return g->get_type() == type; })) << "group " << group_id;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Result

    TEST_F(DataflowTest, groups_as_list)
    {
        const auto res = analyze();
        const auto all = res.get_groups_as_list();
        ASSERT_EQ(all.size(), res.get_groups().size());
        for (const auto& group : all)
        {
            EXPECT_TRUE(res.get_group_id_of_gate(group.front()).is_ok());
            EXPECT_EQ(as_set(group), res.get_groups().at(res.get_group_id_of_gate(group.front()).get()));
        }

        const u32 a_id      = group_of(res, m_registers["a"]);
        const auto filtered = res.get_groups_as_list({a_id});
        ASSERT_EQ(filtered.size(), 1u);
        EXPECT_EQ(as_set(filtered.front()), as_set(m_registers["a"]));
    }

    TEST_F(DataflowTest, write_dot)
    {
        const auto res = analyze();
        auto id        = register_groups(res);
        ASSERT_FALSE(HasFailure());

        const auto written = res.write_dot(m_sandbox / "dana.dot");
        ASSERT_TRUE(written.is_ok()) << written.get_error().get();
        EXPECT_EQ(written.get(), m_sandbox / "dana.dot");
        const std::string dot = read_file(written.get());
        EXPECT_EQ(dot.rfind("digraph {", 0), 0u);
        for (const auto& [group_id, gates] : res.get_groups())
        {
            EXPECT_NE(dot.find("\n" + std::to_string(group_id) + " [width="), std::string::npos);
            EXPECT_NE(dot.find("label=\"" + std::to_string(gates.size()) + " bit (id " + std::to_string(group_id) + ")\""), std::string::npos);
            const auto successors = res.get_group_successors(group_id);
            ASSERT_TRUE(successors.is_ok());
            for (const auto suc_id : successors.get())
            {
                EXPECT_NE(dot.find("\n" + std::to_string(group_id) + " -> " + std::to_string(suc_id) + ";"), std::string::npos);
            }
        }

        // only the given groups and the edges between them
        const auto filtered = res.write_dot(m_sandbox / "filtered.dot", {id["a"], id["b"]});
        ASSERT_TRUE(filtered.is_ok());
        const std::string filtered_dot = read_file(filtered.get());
        EXPECT_NE(filtered_dot.find("\n" + std::to_string(id["a"]) + " -> " + std::to_string(id["b"]) + ";"), std::string::npos);
        EXPECT_NE(filtered_dot.find("\n" + std::to_string(id["b"]) + " -> " + std::to_string(id["b"]) + ";"), std::string::npos);
        EXPECT_EQ(filtered_dot.find("(id " + std::to_string(id["c"]) + ")"), std::string::npos);
        EXPECT_EQ(filtered_dot.find(" -> " + std::to_string(id["d"]) + ";"), std::string::npos);

        // a wrong extension is replaced, a directory gets a default file name
        const auto replaced = res.write_dot(m_sandbox / "dana.txt");
        ASSERT_TRUE(replaced.is_ok());
        EXPECT_EQ(replaced.get(), m_sandbox / "dana.dot");
        const auto in_directory = res.write_dot(m_sandbox);
        ASSERT_TRUE(in_directory.is_ok());
        EXPECT_EQ(in_directory.get(), m_sandbox / "graph.dot");
        EXPECT_TRUE(std::filesystem::exists(m_sandbox / "graph.dot"));
        EXPECT_TRUE(res.write_dot("").is_error());
    }

    TEST_F(DataflowTest, write_txt)
    {
        const auto res = analyze();
        auto id        = register_groups(res);
        ASSERT_FALSE(HasFailure());

        ASSERT_TRUE(res.write_txt(m_sandbox / "dana.txt").is_ok());
        const std::string txt = read_file(m_sandbox / "dana.txt");
        for (const auto& [group_id, gates] : res.get_groups())
        {
            EXPECT_NE(txt.find("ID:" + std::to_string(group_id) + ", Size:" + std::to_string(gates.size())), std::string::npos);
        }
        EXPECT_NE(txt.find("Successors:"), std::string::npos);
        EXPECT_NE(txt.find("Predecessors:"), std::string::npos);
        EXPECT_NE(txt.find("EN: {" + std::to_string(m_control["en_a"]->get_id()) + "}"), std::string::npos);
        EXPECT_NE(txt.find(m_registers["a"].front()->get_name()), std::string::npos);

        ASSERT_TRUE(res.write_txt(m_sandbox / "filtered.txt", {id["a"]}).is_ok());
        const std::string filtered = read_file(m_sandbox / "filtered.txt");
        EXPECT_NE(filtered.find("ID:" + std::to_string(id["a"]) + ","), std::string::npos);
        EXPECT_EQ(filtered.find("ID:" + std::to_string(id["b"]) + ","), std::string::npos);

        ASSERT_TRUE(res.write_txt(m_sandbox / "groups.dot").is_ok());
        EXPECT_TRUE(std::filesystem::exists(m_sandbox / "groups.txt"));
        std::filesystem::remove(m_sandbox / "groups.txt");
        ASSERT_TRUE(res.write_txt(m_sandbox).is_ok());
        EXPECT_TRUE(std::filesystem::exists(m_sandbox / "groups.txt"));
        EXPECT_TRUE(res.write_txt("").is_error());
    }

    TEST_F(DataflowTest, create_modules)
    {
        const auto res = analyze();
        auto id        = register_groups(res);
        ASSERT_FALSE(HasFailure());

        const auto created = res.create_modules();
        ASSERT_TRUE(created.is_ok()) << created.get_error().get();
        ASSERT_EQ(created.get().size(), res.get_groups().size());
        for (const auto& [group_id, module] : created.get())
        {
            EXPECT_EQ(module->get_name(), "DANA_module_" + std::to_string(group_id));
            EXPECT_EQ(module->get_parent_module(), m_nl->get_top_module());
            EXPECT_EQ(as_set(module->get_gates()), res.get_groups().at(group_id));
        }

        // pin groups: one per pin of the flip-flops, named after it, the data pins with one pin per bit
        auto pin_groups = pin_groups_by_name(created.get().at(id["a"]));
        ASSERT_TRUE(pin_groups.count("i_D") && pin_groups.count("o_Q")) << "pin groups i_D and o_Q";
        EXPECT_EQ(pin_groups["i_D"]->size(), register_size);
        EXPECT_EQ(pin_groups["o_Q"]->size(), register_size);
        EXPECT_TRUE(pin_groups.count("i_EN") && pin_groups.count("i_CLK"));

        // creating them again replaces the old modules, also when only some groups are selected
        ASSERT_TRUE(res.create_modules().is_ok());
        EXPECT_EQ(dana_modules().size(), res.get_groups().size());
        const auto selected = res.create_modules({id["a"]});
        ASSERT_TRUE(selected.is_ok());
        EXPECT_EQ(selected.get().size(), 1u);
        ASSERT_EQ(dana_modules().size(), 1u);
        EXPECT_EQ(as_set(dana_modules().front()->get_gates()), as_set(m_registers["a"]));

        // a group whose gates all sit in one module is created inside of it
        Module* outer = m_nl->create_module("outer", m_nl->get_top_module(), m_registers["b"]);
        ASSERT_NE(outer, nullptr);
        const auto nested = res.create_modules({id["b"]});
        ASSERT_TRUE(nested.is_ok());
        EXPECT_EQ(nested.get().at(id["b"])->get_parent_module(), outer);
    }

    TEST_F(DataflowTest, create_modules_with_suffixes_and_pin_prefixes)
    {
        const auto res = analyze();
        auto id        = register_groups(res);
        ASSERT_FALSE(HasFailure());
        const GateType* dffe = m_nl->get_gate_library()->get_gate_type_by_name("DFFE");

        // module names by gate type, with prefixes for the pin groups
        const std::map<std::pair<PinDirection, std::string>, std::string> prefixes = {{{PinDirection::input, "D"}, "data"}, {{PinDirection::output, "Q"}, "state"}};
        const auto by_type                                                         = res.create_modules(std::map<const GateType*, std::string>{{dffe, "ffe"}}, prefixes, {});
        ASSERT_TRUE(by_type.is_ok()) << by_type.get_error().get();
        for (const auto& [reg, group_id] : id)
        {
            const std::string suffix = reg == "d" ? "module" : "ffe";
            EXPECT_EQ(by_type.get().at(group_id)->get_name(), "DANA_" + suffix + "_" + std::to_string(group_id));
        }
        auto pin_groups = pin_groups_by_name(by_type.get().at(id["a"]));
        ASSERT_TRUE(pin_groups.count("data") && pin_groups.count("state")) << "pin groups data and state";
        EXPECT_EQ(pin_groups["data"]->size(), register_size);
        EXPECT_EQ(pin_groups["state"]->size(), register_size);

        // module names by gate type property, for selected groups only
        const auto by_property = res.create_modules(std::map<GateTypeProperty, std::string>{{GateTypeProperty::ff, "reg"}}, {}, {id["c"], id["d"]});
        ASSERT_TRUE(by_property.is_ok()) << by_property.get_error().get();
        ASSERT_EQ(by_property.get().size(), 2u);
        EXPECT_EQ(by_property.get().at(id["c"])->get_name(), "DANA_reg_" + std::to_string(id["c"]));
        EXPECT_EQ(by_property.get().at(id["d"])->get_name(), "DANA_reg_" + std::to_string(id["d"]));
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Merging and splitting groups

    TEST_F(DataflowTest, merge_groups_keeps_result_consistent)
    {
        // merge the two groups with the lowest IDs until a single group is left, which merges neighbors in both
        // directions and groups with self-loops
        auto res = analyze();
        while (res.get_groups().size() > 1)
        {
            const auto ids = sorted_group_ids(res);
            merge_and_check(res, {ids[0], ids[1]});
            if (HasFailure())
            {
                return;
            }
        }
        EXPECT_EQ(res.get_groups().begin()->second.size(), 4 * register_size);
    }

    TEST_F(DataflowTest, merge_groups_keeps_control_nets)
    {
        auto res = analyze();
        auto id  = register_groups(res);
        ASSERT_FALSE(HasFailure());

        const u32 merged = merge_and_check(res, {id["a"], id["d"]});
        ASSERT_NE(merged, no_group);
        EXPECT_EQ(res.get_group_control_nets(merged, PinType::enable).get(), (std::unordered_set<Net*>{m_control["en_a"]}));
        EXPECT_EQ(res.get_group_control_nets(merged, PinType::reset).get(), (std::unordered_set<Net*>{m_control["rst"]}));
        EXPECT_EQ(res.get_group_control_nets(merged, PinType::clock).get(), (std::unordered_set<Net*>{m_control["clk"]}));
        EXPECT_TRUE(res.get_group_control_nets(id["a"], PinType::enable).is_error());
    }

    TEST_F(DataflowTest, merge_groups_of_single_gates)
    {
        // split every group into single gates, then merge them back pairwise, so that many merges join groups that
        // read and write each other
        auto res = analyze();
        for (const auto group_id : sorted_group_ids(res))
        {
            std::vector<std::unordered_set<Gate*>> singles;
            for (auto* gate : res.get_groups().at(group_id))
            {
                singles.push_back({gate});
            }
            if (singles.size() > 1)
            {
                ASSERT_TRUE(res.split_group(group_id, singles).is_ok());
                expect_consistent(res);
            }
        }
        EXPECT_EQ(res.get_groups().size(), 4 * register_size);

        // merge three groups at once, with a duplicate ID
        auto ids = sorted_group_ids(res);
        merge_and_check(res, {ids[0], ids[1], ids[2], ids[0]});

        while (res.get_groups().size() > 1 && !HasFailure())
        {
            ids = sorted_group_ids(res);
            merge_and_check(res, {ids[0], ids[1]});
        }
    }

    TEST_F(DataflowTest, split_group_keeps_result_consistent)
    {
        auto res = analyze();
        std::vector<std::unordered_set<Gate*>> registers;
        for (const auto& [_, gates] : res.get_groups())
        {
            registers.push_back(gates);
        }

        // merge everything into one group, then split it back into the groups of the analysis
        auto ids = sorted_group_ids(res);
        while (ids.size() > 1)
        {
            merge_and_check(res, {ids[0], ids[1]});
            ASSERT_FALSE(HasFailure());
            ids = sorted_group_ids(res);
        }
        const auto split = res.split_group(ids[0], registers);
        ASSERT_TRUE(split.is_ok()) << split.get_error().get();
        EXPECT_EQ(split.get().size(), registers.size());
        EXPECT_TRUE(res.get_gates_of_group(ids[0]).is_error());
        expect_consistent(res);
        register_groups(res);

        // split one register in two halves, whose neighbors must now refer to the halves
        const u32 a_id = group_of(res, m_registers["a"]);
        ASSERT_NE(a_id, no_group);
        const std::unordered_set<Gate*> low(m_registers["a"].begin(), m_registers["a"].begin() + register_size / 2);
        const std::unordered_set<Gate*> high(m_registers["a"].begin() + register_size / 2, m_registers["a"].end());
        const auto halves = res.split_group(a_id, {low, high});
        ASSERT_TRUE(halves.is_ok());
        expect_consistent(res);
        EXPECT_EQ(res.get_group_control_nets(halves.get().front(), PinType::enable).get(), (std::unordered_set<Net*>{m_control["en_a"]}));
    }

    TEST_F(DataflowTest, merge_and_split_reject_invalid_input)
    {
        auto res       = analyze();
        const auto ids = sorted_group_ids(res);
        ASSERT_GE(ids.size(), 2u);
        const u32 unknown = ids.back() + 1000;

        EXPECT_TRUE(res.merge_groups({}).is_error());
        EXPECT_TRUE(res.merge_groups({ids[0]}).is_error());
        EXPECT_TRUE(res.merge_groups({ids[0], unknown}).is_error());

        const auto& gates = res.get_groups().at(ids[0]);
        auto* foreign     = *res.get_groups().at(ids[1]).begin();
        EXPECT_TRUE(res.split_group(unknown, {{*gates.begin()}}).is_error()) << "unknown group";
        EXPECT_TRUE(res.split_group(ids[0], {}).is_error()) << "no new groups";
        EXPECT_TRUE(res.split_group(ids[0], {{foreign}}).is_error()) << "a gate of another group";
        EXPECT_TRUE(res.split_group(ids[0], {{*gates.begin()}}).is_error()) << "not all gates of the group";
        EXPECT_TRUE(res.split_group(ids[0], {gates, {*gates.begin()}}).is_error()) << "a gate in two new groups";
        EXPECT_TRUE(res.split_group(ids[0], {gates, {nullptr}}).is_error()) << "a nullptr";

        // the failed calls leave the result unchanged
        EXPECT_EQ(sorted_group_ids(res), ids);
        expect_consistent(res);
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Plugin

    TEST_F(DataflowTest, plugin_interface)
    {
        DataflowPlugin plugin;
        EXPECT_EQ(plugin.get_name(), "dataflow");
        EXPECT_FALSE(plugin.get_version().empty());
        EXPECT_FALSE(plugin.get_description().empty());
        EXPECT_TRUE(plugin.get_dependencies().empty());
        EXPECT_NE(plugin.get_first_extension<CliExtensionDataflow>(), nullptr);
        EXPECT_NE(plugin.get_first_extension<GuiExtensionDataflow>(), nullptr);
    }

    TEST_F(DataflowTest, command_line_interface)
    {
        CliExtensionDataflow cli;
        const auto options = cli.get_cli_options();
        for (const auto* flag : {"--dataflow", "--path", "--sizes", "--bad_group_size"})
        {
            EXPECT_TRUE(options.is_registered(flag)) << flag;
        }

        ProgramArguments args;
        args.set_option("--dataflow", std::vector<std::string>{});
        args.set_option("--path", m_sandbox.string());
        args.set_option("--sizes", "8,16");
        args.set_option("--bad_group_size", "8");
        EXPECT_TRUE(cli.handle_cli_call(m_nl.get(), args));
        EXPECT_TRUE(std::filesystem::exists(m_sandbox / "graph.dot"));
        EXPECT_TRUE(std::filesystem::exists(m_sandbox / "groups.txt"));
    }

    TEST_F(DataflowTest, gui_extension)
    {
        GuiExtensionDataflow gui;
        std::set<std::string> tags;
        for (const auto& parameter : gui.get_parameter())
        {
            tags.insert(parameter.get_tagname());
        }
        EXPECT_EQ(tags, (std::set<std::string>{"sizes", "min_group_size", "output", "register_stage_identification", "write_txt", "create_modules", "write_dot", "exec"}));

        std::vector<PluginParameter> parameters = {
            PluginParameter(PluginParameter::String, "sizes", "", "8"),
            PluginParameter(PluginParameter::Integer, "min_group_size", "", "8"),
            PluginParameter(PluginParameter::ExistingDir, "output", "", m_sandbox.string()),
            PluginParameter(PluginParameter::Boolean, "register_stage_identification", "", "false"),
            PluginParameter(PluginParameter::Boolean, "write_txt", "", "true"),
            PluginParameter(PluginParameter::Boolean, "write_dot", "", "true"),
            PluginParameter(PluginParameter::Boolean, "create_modules", "", "true"),
        };

        // nothing happens until the button is clicked
        gui.set_parameter(parameters);
        gui.execute_function("exec", m_nl.get(), {}, {}, {});
        EXPECT_TRUE(dana_modules().empty());

        parameters.push_back(PluginParameter(PluginParameter::PushButton, "exec", "", "clicked"));
        gui.set_parameter(parameters);
        gui.execute_function("exec", m_nl.get(), {}, {}, {});
        EXPECT_EQ(dana_modules().size(), 4u);
        EXPECT_TRUE(std::filesystem::exists(m_sandbox / "graph.dot"));
        EXPECT_TRUE(std::filesystem::exists(m_sandbox / "groups.txt"));
    }
}    // namespace hal
