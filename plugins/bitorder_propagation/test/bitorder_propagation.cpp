#include "bitorder_propagation/bitorder_propagation.h"

#include "gate_library_test_utils.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"

#include <fstream>

namespace hal
{
    /**
     * Tests of the bit-order propagation.
     *
     * These describe what the plugin does today rather than what it ought to do, so that the types its
     * interface is written in can be replaced without the behaviour changing unnoticed. The plugin had
     * no tests at all before.
     */
    class BitorderPropagationTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
        }

        virtual void TearDown()
        {
            test_utils::remove_sandbox_directory();
        }

        /**
         * A netlist of two modules joined by a bus of `width` nets: `width` buffers inside the source
         * module each drive one net, and `width` buffers inside the destination module each read one.
         *
         * The nets are handed over in the order they were created, so a test can state a bit order for
         * one side and check what the other side ends up with.
         */
        struct Fixture
        {
            std::unique_ptr<Netlist> netlist;
            Module* src_module                 = nullptr;
            Module* dst_module                 = nullptr;
            PinGroup<ModulePin>* src_pin_group = nullptr;
            PinGroup<ModulePin>* dst_pin_group = nullptr;
            std::vector<Net*> bus;
        };

        Fixture build_bus(u32 width)
        {
            Fixture f;
            f.netlist             = test_utils::create_empty_netlist();
            const GateLibrary* gl = f.netlist->get_gate_library();

            std::vector<Gate*> src_gates, dst_gates;
            for (u32 i = 0; i < width; i++)
            {
                Gate* driver = f.netlist->create_gate(gl->get_gate_type_by_name("BUF"), "driver_" + std::to_string(i));
                Gate* reader = f.netlist->create_gate(gl->get_gate_type_by_name("BUF"), "reader_" + std::to_string(i));
                src_gates.push_back(driver);
                dst_gates.push_back(reader);

                Net* n = test_utils::connect(f.netlist.get(), driver, "O", reader, "I", "bus_" + std::to_string(i));
                f.bus.push_back(n);

                // Give each buffer something to read, so that the source module has an input as well.
                Net* stimulus = f.netlist->create_net("stimulus_" + std::to_string(i));
                stimulus->add_destination(driver, "I");
                stimulus->mark_global_input_net();
            }

            f.src_module = f.netlist->create_module("src", f.netlist->get_top_module(), src_gates);
            f.dst_module = f.netlist->create_module("dst", f.netlist->get_top_module(), dst_gates);

            // A net crossing a module boundary gets a pin of its own, and each pin a group of its own.
            // Collect them into one group per side, which is what a bus looks like and what the
            // propagation works on.
            std::vector<ModulePin*> src_pins, dst_pins;
            for (auto* n : f.bus)
            {
                src_pins.push_back(f.src_module->get_pin_by_net(n));
                dst_pins.push_back(f.dst_module->get_pin_by_net(n));
            }

            // Ascending from 0, so that the group is indexed 0..width-1 like a bus. The default is
            // descending from 0, which indexes the group 0, -1, -2, ... and leaves it unable to accept
            // any positive index later on.
            auto src_group = f.src_module->create_pin_group("OUT", src_pins, PinDirection::output, PinType::none, true, 0);
            auto dst_group = f.dst_module->create_pin_group("IN", dst_pins, PinDirection::input, PinType::none, true, 0);
            f.src_pin_group = src_group.is_ok() ? src_group.get() : nullptr;
            f.dst_pin_group = dst_group.is_ok() ? dst_group.get() : nullptr;
            return f;
        }
    };

    /**
     * A pin group whose bit order is known hands that order to the pin group it drives.
     *
     * Functions: propagate_module_pingroup_bitorder
     */
    TEST_F(BitorderPropagationTest, check_propagate_to_connected_pin_group)
    {
        TEST_START
        {
            Fixture f = build_bus(4);
            ASSERT_NE(f.netlist, nullptr);

            std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>> known;
            std::map<Net*, u32> order;
            for (u32 i = 0; i < f.bus.size(); i++)
            {
                order[f.bus.at(i)] = i;
            }

            ASSERT_NE(f.src_pin_group, nullptr);
            ASSERT_NE(f.dst_pin_group, nullptr);
            ASSERT_EQ(f.src_pin_group->get_pins().size(), 4);
            ASSERT_EQ(f.dst_pin_group->get_pins().size(), 4);

            known[{f.src_module, f.src_pin_group}] = order;

            auto res = bitorder_propagation::propagate_module_pingroup_bitorder(known, {{f.dst_module, f.dst_pin_group}});
            ASSERT_TRUE(res.is_ok());

            const auto& all = res.get();
            const auto it   = all.find({f.dst_module, f.dst_pin_group});
            ASSERT_NE(it, all.end());
            EXPECT_EQ(it->second, order);
        }
        TEST_END
    }

    /**
     * A bit order with gaps in it is rejected unless non-continuous orders are allowed.
     *
     * Functions: propagate_module_pingroup_bitorder
     */
    TEST_F(BitorderPropagationTest, check_continuous_bitorder_is_enforced_on_request)
    {
        TEST_START
        {
            // Indices 0, 1, 2 and 4: a bus of four nets whose order leaves a hole at 3.
            const std::vector<u32> indices = {0, 1, 2, 4};

            for (const bool enforce_continuous : {true, false})
            {
                Fixture f = build_bus(4);
                ASSERT_NE(f.netlist, nullptr);
                ASSERT_NE(f.src_pin_group, nullptr);

                std::map<Net*, u32> order;
                for (u32 i = 0; i < f.bus.size(); i++)
                {
                    order[f.bus.at(i)] = indices.at(i);
                }

                std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>> known;
                known[{f.src_module, f.src_pin_group}] = order;

                auto res = bitorder_propagation::propagate_module_pingroup_bitorder(known, {{f.dst_module, f.dst_pin_group}}, enforce_continuous);
                ASSERT_TRUE(res.is_ok());

                const auto& all = res.get();
                const auto it   = all.find({f.dst_module, f.dst_pin_group});

                if (enforce_continuous)
                {
                    // The hole makes the order invalid, so nothing is reconstructed for the destination.
                    EXPECT_EQ(it, all.end());
                }
                else
                {
                    // The destination is reconstructed, and its indices come out continuous even so:
                    // what the flag permits is accepting an order with a hole in it as input, not
                    // carrying that hole over to what is reconstructed from it.
                    ASSERT_NE(it, all.end());
                    std::set<u32> reconstructed;
                    for (const auto& [_, index] : it->second)
                    {
                        reconstructed.insert(index);
                    }
                    EXPECT_EQ(reconstructed, std::set<u32>({0, 1, 2, 3}));
                }
            }
        }
        TEST_END
    }

    /**
     * Propagating to a pin group that is already known leaves it alone and reports it back unchanged.
     *
     * Functions: propagate_module_pingroup_bitorder
     */
    TEST_F(BitorderPropagationTest, check_known_bitorder_is_reported_back)
    {
        TEST_START
        {
            Fixture f = build_bus(4);
            ASSERT_NE(f.netlist, nullptr);
            ASSERT_NE(f.src_pin_group, nullptr);

            std::map<Net*, u32> order;
            for (u32 i = 0; i < f.bus.size(); i++)
            {
                order[f.bus.at(i)] = i;
            }

            std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>> known;
            known[{f.src_module, f.src_pin_group}] = order;

            auto res = bitorder_propagation::propagate_module_pingroup_bitorder(known, {{f.dst_module, f.dst_pin_group}});
            ASSERT_TRUE(res.is_ok());

            // The result carries the orders that were already known as well as the ones just found.
            const auto& all = res.get();
            const auto it   = all.find({f.src_module, f.src_pin_group});
            ASSERT_NE(it, all.end());
            EXPECT_EQ(it->second, order);
        }
        TEST_END
    }

    /**
     * Reordering renames the pins of a group so that their names carry the propagated indices.
     *
     * Functions: reorder_module_pin_groups
     */
    TEST_F(BitorderPropagationTest, check_reorder_renames_pins)
    {
        TEST_START
        {
            Fixture f = build_bus(4);
            ASSERT_NE(f.netlist, nullptr);
            ASSERT_NE(f.dst_pin_group, nullptr);

            std::map<Net*, u32> order;
            for (u32 i = 0; i < f.bus.size(); i++)
            {
                order[f.bus.at(i)] = i;
            }

            std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>> to_apply;
            to_apply[{f.dst_module, f.dst_pin_group}] = order;

            auto res = bitorder_propagation::reorder_module_pin_groups(to_apply);
            ASSERT_TRUE(res.is_ok());

            // Every net of the bus now sits at the index it was given, and the group counts as ordered.
            auto* group = f.dst_module->get_pin_by_net(f.bus.front())->get_group().first;
            ASSERT_NE(group, nullptr);
            EXPECT_EQ(group->get_pins().size(), 4);
            for (u32 i = 0; i < f.bus.size(); i++)
            {
                auto* pin = f.dst_module->get_pin_by_net(f.bus.at(i));
                ASSERT_NE(pin, nullptr);
                EXPECT_EQ(pin->get_group().first, group);
                EXPECT_EQ(pin->get_group().second, i);
            }
        }
        TEST_END
    }

    /**
     * The export writes the problem out as json and reports which word each pin group became.
     *
     * Functions: export_bitorder_propagation_information
     */
    TEST_F(BitorderPropagationTest, check_export_writes_the_problem)
    {
        TEST_START
        {
            Fixture f = build_bus(4);
            ASSERT_NE(f.netlist, nullptr);
            ASSERT_NE(f.src_pin_group, nullptr);

            std::map<Net*, u32> order;
            for (u32 i = 0; i < f.bus.size(); i++)
            {
                order[f.bus.at(i)] = i;
            }

            std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>> known;
            known[{f.src_module, f.src_pin_group}] = order;

            const std::string path = test_utils::create_sandbox_path("bitorder_export.json").string();
            auto res               = bitorder_propagation::export_bitorder_propagation_information(known, {{f.dst_module, f.dst_pin_group}}, path);
            ASSERT_TRUE(res.is_ok());

            // Every pin group involved is reported with the index of the word it became in the file.
            const auto& word_of = res.get();
            EXPECT_EQ(word_of.size(), 2);
            EXPECT_NE(word_of.find({f.src_module, f.src_pin_group}), word_of.end());
            EXPECT_NE(word_of.find({f.dst_module, f.dst_pin_group}), word_of.end());

            std::ifstream file(path);
            ASSERT_TRUE(file.is_open());
            const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            EXPECT_NE(content.find("word_definitions"), std::string::npos);
            EXPECT_NE(content.find("known_bit_order"), std::string::npos);
            EXPECT_NE(content.find("connected_words"), std::string::npos);
        }
        TEST_END
    }
}    // namespace hal
