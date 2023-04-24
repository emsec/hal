
#include "netlist_test_utils.h"

#include "gate_library_test_utils.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace test_utils
    {
        std::unique_ptr<Netlist> create_empty_netlist(const u32 id)
        {
            std::unique_ptr<Netlist> netlist = std::make_unique<Netlist>(get_gate_library());

            if (id != 0)
            {
                netlist->set_id(id);
            }

            return netlist;
        }

        Net* connect(Netlist* nl, Gate* src, const std::string& src_pin, Gate* dst, const std::string& dst_pin, const std::string& net_name)
        {
            Net* n = nullptr;
            if (src != nullptr && dst != nullptr)
            {
                if (n = src->get_fan_out_net(src_pin); n != nullptr)
                {
                    n->add_destination(dst, dst_pin);
                }
                else if (n = dst->get_fan_in_net(dst_pin); n != nullptr)
                {
                    n->add_source(src, src_pin);
                }
                else
                {
                    if (net_name.empty())
                    {
                        n = nl->create_net("net_" + std::to_string(src->get_id()) + "_" + std::to_string(dst->get_id()));
                    }
                    else
                    {
                        n = nl->create_net(net_name);
                    }

                    n->add_source(src, src_pin);
                    n->add_destination(dst, dst_pin);
                }
            }
            return n;
        }

        Net* connect_global_in(Netlist* nl, Gate* dst, const std::string& dst_pin, const std::string& net_name)
        {
            Net* n = nullptr;
            if (dst != nullptr)
            {
                if (n = dst->get_fan_in_net(dst_pin); n != nullptr)
                {
                    n->mark_global_input_net();
                }
                else
                {
                    if (net_name.empty())
                    {
                        n = nl->create_net("net_" + std::to_string(dst->get_id()));
                    }
                    else
                    {
                        n = nl->create_net(net_name);
                    }

                    n->add_destination(dst, dst_pin);
                    n->mark_global_input_net();
                }
            }
            return n;
        }

        Net* connect_global_out(Netlist* nl, Gate* src, const std::string& src_pin, const std::string& net_name)
        {
            Net* n = nullptr;
            if (src != nullptr)
            {
                if (n = src->get_fan_out_net(src_pin); n != nullptr)
                {
                    n->mark_global_output_net();
                }
                else
                {
                    if (net_name.empty())
                    {
                        n = nl->create_net("net_" + std::to_string(src->get_id()));
                    }
                    else
                    {
                        n = nl->create_net(net_name);
                    }

                    n->add_source(src, src_pin);
                    n->mark_global_output_net();
                }
            }
            return n;
        }

        void clear_connections(Gate* gate)
        {
            for (Endpoint* ep : gate->get_fan_in_endpoints())
            {
                ep->get_net()->remove_destination(ep);
            }

            for (Endpoint* ep : gate->get_fan_out_endpoints())
            {
                ep->get_net()->remove_source(ep);
            }
        }

        void clear_connections(Net* net)
        {
            for (Endpoint* ep : net->get_sources())
            {
                net->remove_source(ep);
            }

            for (Endpoint* ep : net->get_destinations())
            {
                net->remove_destination(ep);
            }
        }
    }    // namespace test_utils

    // TODO clean up everything below

    bool run_known_issue_tests = false;

    bool test_utils::known_issue_tests_active()
    {
        return run_known_issue_tests;
    }

    void test_utils::activate_known_issue_tests()
    {
        run_known_issue_tests = true;
    }

    void test_utils::deactivate_known_issue_tests()
    {
        run_known_issue_tests = false;
    }

    void test_utils::init_log_channels()
    {
        // All channels that are registered:
        static bool already_init = false;
        if (!already_init)
        {
            std::vector<std::string> channel_ids = {"core",
                                                    "gate_library_parser",
                                                    "gate_library_writer",
                                                    "gate_library_manager",
                                                    "gate_library",
                                                    "netlist",
                                                    "netlist_utils",
                                                    "netlist_internal",
                                                    "netlist_persistent",
                                                    "gate",
                                                    "net",
                                                    "module",
                                                    "grouping",
                                                    "netlist_parser",
                                                    "netlist_writer",
                                                    "python_context",
                                                    "test_utils"};

            for (std::string ch_id : channel_ids)
            {
                LogManager::get_instance()->add_channel(ch_id, {LogManager::create_stdout_sink()}, "info");
            }
            already_init = true;
        }
    }

    Endpoint* test_utils::get_endpoint(Netlist* nl, const int gate_id, const std::string& pin_name, bool is_destination)
    {
        if (Gate* g = nl->get_gate_by_id(gate_id); g != nullptr)
        {
            if (GatePin* pin = g->get_type()->get_pin_by_name(pin_name); pin != nullptr)
            {
                return is_destination ? g->get_fan_in_endpoint(pin) : g->get_fan_out_endpoint(pin);
            }
        }
        return nullptr;
    }

    Endpoint* test_utils::get_endpoint(Gate* g, const std::string& pin_name)
    {
        if (g == nullptr || pin_name == "")
        {
            return nullptr;
        }
        auto nl             = g->get_netlist();
        int gate_id         = g->get_id();
        auto in_pins        = g->get_type()->get_input_pin_names();
        bool is_destination = (std::find(in_pins.begin(), in_pins.end(), pin_name) != in_pins.end());
        return get_endpoint(nl, gate_id, pin_name, is_destination);
    }

    std::vector<BooleanFunction::Value> test_utils::minimize_truth_table(const std::vector<BooleanFunction::Value> tt)
    {
        int var_amt = round(log2(tt.size()));
        if ((1 << var_amt) != tt.size())
        {
            log_error("test_utils", "minimize_truth_table: Tablesize must be a power of two!");
            return std::vector<BooleanFunction::Value>();
        }
        for (int v = 0; v < var_amt; v++)
        {
            int interval = 2 << v;
            std::vector<BooleanFunction::Value> v_eq_0;
            std::vector<BooleanFunction::Value> v_eq_1;
            for (int i = 0; i < tt.size(); i++)
            {
                if (i % interval < (interval >> 1))
                {
                    v_eq_0.push_back(tt[i]);
                }
                else
                {
                    v_eq_1.push_back(tt[i]);
                }
            }
            if (v_eq_0 == v_eq_1)
            {
                return minimize_truth_table(v_eq_0);
            }
        }
        return tt;
    }

    bool test_utils::string_contains_substring(const std::string str, const std::string sub_str)
    {
        return (str.find(sub_str) != std::string::npos);
    }

    Net* test_utils::get_net_by_subname(Netlist* nl, const std::string subname)
    {
        if (nl == nullptr)
            return nullptr;
        auto nets = nl->get_nets();
        Net* res  = nullptr;
        for (auto n : nets)
        {
            std::string n_name = n->get_name();
            if (n_name.find(subname) != n_name.npos)
            {
                if (res != nullptr)
                {
                    log_error("test_utils", "get_net_by_subname: Multiple nets contain the substring '{}'! This should not happen...", subname);
                    return nullptr;
                }
                res = n;
            }
        }
        return res;
    }

    Gate* test_utils::get_gate_by_subname(Netlist* nl, const std::string subname)
    {
        if (nl == nullptr)
            return nullptr;
        auto gates = nl->get_gates();
        Gate* res  = nullptr;
        for (auto g : gates)
        {
            std::string g_name = g->get_name();
            if (g_name.find(subname) != g_name.npos)
            {
                if (res != nullptr)
                {
                    log_error("test_utils", "get_gate_by_subname: Multiple gates contain the substring '{}'! This should not happen...", subname);
                    return nullptr;
                }
                res = g;
            }
        }
        return res;
    }

    std::filesystem::path test_utils::create_sandbox_directory()
    {
        std::filesystem::path sb_path = utils::get_base_directory() / sandbox_directory_path;
        std::filesystem::create_directory(sb_path);
        return sb_path;
    }

    void test_utils::remove_sandbox_directory()
    {
        std::filesystem::remove_all((utils::get_base_directory() / sandbox_directory_path));
    }

    std::filesystem::path test_utils::create_sandbox_path(const std::string file_name)
    {
        std::filesystem::path sb_path = (utils::get_base_directory() / sandbox_directory_path);
        if (!std::filesystem::exists(sb_path))
        {
            log_error("test_utils",
                      "create_sandbox_path: sandbox is not created yet. "
                      "Please use \'create_sandbox_directory()\' to create it beforehand.");
            return std::filesystem::path();
        }
        return sb_path / file_name;
    }

    std::filesystem::path test_utils::create_sandbox_file(std::string file_name, std::string content)
    {
        std::filesystem::path sb_path = (utils::get_base_directory() / sandbox_directory_path);
        if (!std::filesystem::exists(sb_path))
        {
            log_error("test_utils",
                      "[netlist_test_utils] create_sandbox_path: sandbox is not created yet. "
                      "Please use \'create_sandbox_directory()\' to create it beforehand.");
            return std::filesystem::path();
        }
        std::filesystem::path f_path = sb_path / file_name;
        std::ofstream sb_file(f_path.string());
        sb_file << content;
        sb_file.close();
        return f_path;
    }

    std::unique_ptr<Netlist> test_utils::create_example_netlist(const int id)
    {
        NO_COUT_BLOCK;
        const GateLibrary* gl       = get_gate_library();
        std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(gl);
        nl->set_device_name("device_name");
        nl->set_design_name("design_name");
        if (id >= 0)
        {
            nl->set_id(id);
        }

        // Create the gates
        Gate* gate_0 = nl->create_gate(MIN_GATE_ID + 0, gl->get_gate_type_by_name("AND2"), "gate_0");
        Gate* gate_1 = nl->create_gate(MIN_GATE_ID + 1, gl->get_gate_type_by_name("GND"), "gate_1");
        Gate* gate_2 = nl->create_gate(MIN_GATE_ID + 2, gl->get_gate_type_by_name("VCC"), "gate_2");
        Gate* gate_3 = nl->create_gate(MIN_GATE_ID + 3, gl->get_gate_type_by_name("BUF"), "gate_3");
        Gate* gate_4 = nl->create_gate(MIN_GATE_ID + 4, gl->get_gate_type_by_name("BUF"), "gate_4");
        Gate* gate_5 = nl->create_gate(MIN_GATE_ID + 5, gl->get_gate_type_by_name("AND2"), "gate_5");
        Gate* gate_6 = nl->create_gate(MIN_GATE_ID + 6, gl->get_gate_type_by_name("OR2"), "gate_6");
        Gate* gate_7 = nl->create_gate(MIN_GATE_ID + 7, gl->get_gate_type_by_name("XOR2"), "gate_7");
        Gate* gate_8 = nl->create_gate(MIN_GATE_ID + 8, gl->get_gate_type_by_name("AND2"), "gate_8");

        gate_0->set_data("a", "b", "c", "d");
        gate_1->set_data("x", "y", "z", "w");

        // Add the nets (net_x_y1_y2... := Net between the Gate with id x and the gates y1,y2,...)
        Net* net_1_3 = nl->create_net(MIN_NET_ID + 13, "net_1_3");
        net_1_3->add_source(gate_1, "O");
        net_1_3->add_destination(gate_3, "I");

        Net* net_3_0 = nl->create_net(MIN_NET_ID + 30, "net_3_0");
        net_3_0->add_source(gate_3, "O");
        net_3_0->add_destination(gate_0, "I0");

        Net* net_2_0 = nl->create_net(MIN_NET_ID + 20, "net_2_0");
        net_2_0->add_source(gate_2, "O");
        net_2_0->add_destination(gate_0, "I1");

        Net* net_0_4_5 = nl->create_net(MIN_NET_ID + 045, "net_0_4_5");
        net_0_4_5->add_source(gate_0, "O");
        net_0_4_5->add_destination(gate_4, "I");
        net_0_4_5->add_destination(gate_5, "I0");

        Net* net_7_8 = nl->create_net(MIN_NET_ID + 78, "net_7_8");
        net_7_8->add_source(gate_7, "O");
        net_7_8->add_destination(gate_8, "I0");

        return nl;
    }

    std::unique_ptr<Netlist> test_utils::create_example_netlist_2(const int id)
    {
        NO_COUT_BLOCK;
        const GateLibrary* gl       = get_gate_library();
        std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(gl);
        nl->set_device_name("device_name");
        nl->set_design_name("design_name");
        if (id >= 0)
        {
            nl->set_id(id);
        }

        // Create the gates
        Gate* gate_0 = nl->create_gate(MIN_GATE_ID + 0, gl->get_gate_type_by_name("AND4"), "gate_0");
        Gate* gate_1 = nl->create_gate(MIN_GATE_ID + 1, gl->get_gate_type_by_name("OR4"), "gate_1");
        Gate* gate_2 = nl->create_gate(MIN_GATE_ID + 2, gl->get_gate_type_by_name("XOR4"), "gate_2");
        Gate* gate_3 = nl->create_gate(MIN_GATE_ID + 3, gl->get_gate_type_by_name("AND4"), "gate_3");

        // Add the nets (net_x_y1_y2... := Net between the Gate with id x and the gates y1,y2,...)

        Net* net_0_1_3 = nl->create_net(MIN_NET_ID + 013, "net_0_1_3");
        net_0_1_3->add_source(gate_0, "O");
        net_0_1_3->add_destination(gate_1, "I0");
        net_0_1_3->add_destination(gate_1, "I1");
        net_0_1_3->add_destination(gate_1, "I2");
        net_0_1_3->add_destination(gate_3, "I0");

        Net* net_2_1 = nl->create_net(MIN_NET_ID + 21, "net_2_1");
        net_2_1->add_source(gate_2, "O");
        net_2_1->add_destination(gate_1, "I3");

        return nl;
    }

    std::unique_ptr<Netlist> test_utils::create_example_netlist_negative(const int id)
    {
        NO_COUT_BLOCK;
        const GateLibrary* gl       = get_gate_library();
        std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(gl);
        nl->set_device_name("device_name");
        nl->set_design_name("design_name");
        if (id >= 0)
        {
            nl->set_id(id);
        }

        // Create the Gate
        Gate* gate_0 = nl->create_gate(MIN_GATE_ID + 0, gl->get_gate_type_by_name("BUF"), "gate_0");

        // Net connected to the input pin
        Net* net_X_1 = nl->create_net(MIN_GATE_ID + 0, "net_X_1");
        net_X_1->add_destination(gate_0, "I");

        // Net connected to the output pin
        Net* net_1_X = nl->create_net(MIN_GATE_ID + 1, "net_1_X");
        net_1_X->add_source(gate_0, "O");

        return nl;
    }

    Endpoint* test_utils::get_destination_by_pin(const std::vector<Endpoint*> dsts, const GatePin* pin)
    {
        for (auto dst : dsts)
        {
            if (*dst->get_pin() == *pin)
            {
                return dst;
            }
        }
        return nullptr;
    }

    Endpoint* test_utils::get_source_by_pin(const std::vector<Endpoint*> srcs, const GatePin* pin)
    {
        for (auto src : srcs)
        {
            if (*src->get_pin() == *pin)
            {
                return src;
            }
        }
        return nullptr;
    }

    bool test_utils::nets_are_equal(Net* n0, Net* n1, const bool ignore_id, const bool ignore_name)
    {
        if (n0 == nullptr || n1 == nullptr)
        {
            if (n0 == n1)
                return true;
            else
            {
                log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: One net is a nullptr.");
                return false;
            }
        }
        if (!ignore_id && n0->get_id() != n1->get_id())
        {
            log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: IDs are different (\"{}\" vs \"{}\")", n0->get_id(), n1->get_id());
            return false;
        }
        if (!ignore_name && n0->get_name() != n1->get_name())
        {
            log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: Names are different (\"{}\" vs \"{}\")", n0->get_name(), n1->get_name());
            return false;
        }
        for (auto n0_source : n0->get_sources())
        {
            if (!gates_are_equal(n0_source->get_gate(), get_source_by_pin(n1->get_sources(), n0_source->get_pin())->get_gate(), ignore_id, ignore_name))
            {
                log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: Connected gates at source pin \"{}\" are different.", n0_source->get_pin()->get_name());
                return false;
            }
        }
        for (auto n0_destination : n0->get_destinations())
        {
            if (!gates_are_equal(n0_destination->get_gate(), get_destination_by_pin(n1->get_destinations(), n0_destination->get_pin())->get_gate(), ignore_id, ignore_name))
            {
                log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: Connected gates at destination pin \"{}\" are different.", n0_destination->get_pin()->get_name());
                return false;
            }
        }
        if (n0->get_data_map() != n1->get_data_map())
        {
            log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: The stored data is different.");
            return false;
        }
        if (n0->is_global_input_net() != n1->is_global_input_net())
        {
            log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: One net is a global input net, the other one isn't.");
            return false;
        }
        if (n0->is_global_output_net() != n1->is_global_output_net())
        {
            log_info("test_utils", "nets_are_equal: Nets are not equal! Reason: One net is a global output net, the other one isn't.");
            return false;
        }

        return true;
    }

    bool test_utils::gates_are_equal(Gate* g0, Gate* g1, const bool ignore_id, const bool ignore_name)
    {
        if (g0 == nullptr || g1 == nullptr)
        {
            if (g0 == g1)
                return true;
            else
            {
                log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: One gate is a nullptr.");
                return false;
            }
        }
        if (!ignore_id && g0->get_id() != g1->get_id())
        {
            log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: IDs are different (\"{}\" vs \"{}\")", g0->get_id(), g1->get_id());
            return false;
        }
        if (!ignore_name && g0->get_name() != g1->get_name())
        {
            log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: Names are different (\"{}\" vs \"{}\")", g0->get_name(), g1->get_name());
            return false;
        }
        if (*(g0->get_type()) != *(g1->get_type()))
        {
            log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: Gates types are different (\"{}\" vs \"{}\")", g0->get_type()->get_name(), g1->get_type()->get_name());
            return false;
        }
        if (g0->get_data_map() != g1->get_data_map())
        {
            log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: The stored data is different.");
            return false;
        }
        if (g0->is_gnd_gate() != g1->is_gnd_gate())
        {
            log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: One gate is a gnd gate, the other one isn't.");
            return false;
        }
        if (g0->is_vcc_gate() != g1->is_vcc_gate())
        {
            log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: One gate is a vcc gate, the other one isn't.");
            return false;
        }
        std::unordered_map<std::string, BooleanFunction> g0_bf = g0->get_boolean_functions(true);
        std::unordered_map<std::string, BooleanFunction> g1_bf = g1->get_boolean_functions(true);
        if (std::map<std::string, BooleanFunction>(g0_bf.begin(), g0_bf.end()) != std::map<std::string, BooleanFunction>(g1_bf.begin(), g1_bf.end()))
        {
            log_info("test_utils", "gates_are_equal: Gates are not equal! Reason: The stored boolean functions are different.");
            return false;
        }
        return true;
    }

    bool test_utils::modules_are_equal(Module* m_0, Module* m_1, const bool ignore_id, const bool ignore_name)
    {
        // Not only one of them may be a nullptr
        if (m_0 == nullptr || m_1 == nullptr)
        {
            if (m_0 == m_1)
                return true;
            else
            {
                log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: One netlist is a nullptr.");
                return false;
            }
        }
        // The ids should be equal
        if (!ignore_id && m_0->get_id() != m_1->get_id())
        {
            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: IDs are different (\"{}\" vs \"{}\")", m_0->get_id(), m_1->get_id());
            return false;
        }
        // The names should be equal
        if (!ignore_name && m_0->get_name() != m_1->get_name())
        {
            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Names are different (\"{}\" vs \"{}\")", m_0->get_name(), m_1->get_name());
            return false;
        }
        // The types should be the same
        if (m_0->get_type() != m_1->get_type())
        {
            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Types are different (\"{}\" vs \"{}\")", m_0->get_type(), m_1->get_type());
            return false;
        }
        // The stored data should be equal
        if (m_0->get_data_map() != m_1->get_data_map())
        {
            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: The stored data is different.");
            return false;
        }

        // Check if gates are the same
        if (m_0->get_gates().size() != m_1->get_gates().size())
        {
            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: The number of gates is different ({} vs {})", m_0->get_gates().size(), m_1->get_gates().size());
            return false;
        }
        for (auto g_0 : m_0->get_gates())
        {
            if (ignore_id)
            {
                auto g_1_list = m_1->get_gates(gate_name_filter(g_0->get_name()));
                if (g_1_list.size() > 1)
                {
                    log_info("test_utils", "modules_are_equal: Modules can't be compared! Reason: Multiple gates with name \"{}\" are found in the second module.", g_0->get_name());
                    return false;
                }
                if (g_1_list.size() < 1)
                {
                    log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Cannot find a gate with name \"{}\" in second module.", g_0->get_name());
                    return false;
                }
                if (!gates_are_equal(g_0, *g_1_list.begin(), ignore_id))
                {
                    log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Gates with name \"{}\" are not equal.", g_0->get_name());
                    return false;
                }
            }
            else
            {
                Gate* g_1 = m_1->get_netlist()->get_gate_by_id(g_0->get_id());
                if (!gates_are_equal(g_0, g_1, ignore_id, ignore_name))
                {
                    log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Gates with name \"{}\" are not equal.", g_0->get_name());
                    return false;
                }
                if (!m_1->contains_gate(g_1))
                {
                    log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Second module does not contain a gate with name \"{}\".", g_1->get_name());
                    return false;
                }
            }
        }

        // Check that the pins and pin groups are the same
        if (m_0->get_pins().size() != m_0->get_pins().size())
        {
            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: The number of pins is different ({} vs {})", m_0->get_pins().size(), m_1->get_pins().size());
            return false;
        }
        if (m_0->get_pin_groups().size() != m_0->get_pin_groups().size())
        {
            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: The number of pin groups is different ({} vs {})", m_0->get_pin_groups().size(), m_1->get_pin_groups().size());
            return false;
        }

        for (const PinGroup<ModulePin>* pg_0 : m_0->get_pin_groups())
        {
            if (const PinGroup<ModulePin>* pg_1 = m_1->get_pin_group_by_id(pg_0->get_id()); pg_1 != nullptr)
            {
                if ((!ignore_id && (pg_0->get_id() != pg_1->get_id())) || pg_0->get_name() != pg_1->get_name() || pg_0->get_start_index() != pg_1->get_start_index()
                    || pg_0->is_ascending() != pg_1->is_ascending() || pg_0->size() != pg_1->size())
                {
                    log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Two pin groups are different (\"{}\" vs \"{}\")", pg_0->get_name(), pg_1->get_name());
                    return false;
                }

                for (const ModulePin* p_0 : pg_0->get_pins())
                {
                    if (const ModulePin* p_1 = m_1->get_pin_by_id(p_0->get_id()); p_1 != nullptr)
                    {
                        if ((!ignore_id && (p_0->get_id() != p_1->get_id())) || p_0->get_name() != p_1->get_name() || p_0->get_type() != p_1->get_type() || p_0->get_direction() != p_1->get_direction()
                            || !nets_are_equal(p_0->get_net(), p_1->get_net(), ignore_id, ignore_name))
                        {
                            log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Two pins are different (\"{}\" vs \"{}\")", p_0->get_name(), p_1->get_name());
                            return false;
                        }
                    }
                    else
                    {
                        log_info("test_utils",
                                 "modules_are_equal: Modules are not equal! Reason: There is no pin with name '{}' in pin group '{}' of module '{}' with ID {} within netlist with ID {}",
                                 p_0->get_name(),
                                 pg_1->get_name(),
                                 m_1->get_name(),
                                 m_1->get_netlist()->get_id());
                        return false;
                    }
                }
            }
            else
            {
                log_info("test_utils",
                         "modules_are_equal: Modules are not equal! Reason: There is no pin group with name '{}' in module '{}' with ID {} within netlist with ID {}",
                         pg_0->get_name(),
                         m_1->get_name(),
                         m_1->get_netlist()->get_id());
                return false;
            }
        }

        // The parents and submodules should be equal as well (to test this we only check their id, since
        // their equality will be tested as well)
        if (m_0->get_parent_module() == nullptr || m_1->get_parent_module() == nullptr)
        {
            if (m_0->get_parent_module() != m_1->get_parent_module())
            {
                log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: One module is a top module, the other one isn't.");
                return false;
            }
        }
        if (m_0->get_submodules(nullptr, true).size() != m_1->get_submodules(nullptr, true).size())
        {
            log_info("test_utils",
                     "modules_are_equal: Modules are not equal! Reason: The number of submodules are different ({} vs {})",
                     m_0->get_submodules(nullptr, true).size(),
                     m_1->get_submodules(nullptr, true).size());
            return false;
        }
        for (auto sm_0 : m_0->get_submodules(nullptr, true))
        {
            if (sm_0 == nullptr)
                continue;
            if (ignore_id)
            {
                auto sm_1_list = m_1->get_submodules(module_name_filter(sm_0->get_name()));
                if (sm_1_list.size() > 1)
                {
                    log_info("test_utils", "modules_are_equal: Modules can't be compared! Reason: Multiple submodules with name \"{}\" are found in the second module.", sm_0->get_name());
                    return false;
                }
                if (sm_1_list.size() < 1)
                {
                    log_info("test_utils", "modulesare_equal: Modules are not equal! Reason: Cannot find a submodule with name \"{}\" in second module.", sm_0->get_name());
                    return false;
                }
                if (!modules_are_equal(sm_0, *sm_1_list.begin(), ignore_id))
                {
                    log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Submodules with name \"{}\" are not equal.", sm_0->get_name());
                    return false;
                }
            }
            else
            {
                if (!modules_are_equal(sm_0, m_1->get_netlist()->get_module_by_id(sm_0->get_id()), ignore_id))
                {
                    log_info("test_utils", "modules_are_equal: Modules are not equal! Reason: Submodules with name \"{}\" are not equal.", sm_0->get_name());
                    return false;
                }
                /*if (m_1->get_netlist()->get_module_by_id(sm_0->get_id()) == nullptr) {
                    return false;
                }*/
            }
        }

        return true;
    }

    bool test_utils::groupings_are_equal(Grouping* g_0, Grouping* g_1, const bool ignore_id, const bool ignore_name)
    {
        // Not only one of them may be a nullptr
        if (g_0 == nullptr || g_1 == nullptr)
        {
            if (g_0 == g_1)
                return true;
            else
            {
                log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: One netlist is a nullptr.");
                return false;
            }
        }
        // The ids should be equal
        if (!ignore_id && g_0->get_id() != g_1->get_id())
        {
            log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: IDs are different (\"{}\" vs \"{}\")", g_0->get_id(), g_1->get_id());
            return false;
        }
        // The names should be equal
        if (!ignore_name && g_0->get_name() != g_1->get_name())
        {
            log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Names are different (\"{}\" vs \"{}\")", g_0->get_name(), g_1->get_name());
            return false;
        }

        // Check if gates are the same
        if (g_0->get_gates().size() != g_1->get_gates().size())
        {
            log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: The number of gates is different ({} vs {})", g_0->get_gates().size(), g_1->get_gates().size());
            return false;
        }
        for (auto gate_0 : g_0->get_gates())
        {
            if (ignore_id)
            {
                auto g_1_list = g_1->get_gates(gate_name_filter(gate_0->get_name()));
                if (g_1_list.size() > 1)
                {
                    log_info("test_utils", "groupings_are_equal: Groupings can't be compared! Reason: Multiple gates with name \"{}\" are found in the second grouping.", gate_0->get_name());
                    return false;
                }
                if (g_1_list.size() < 1)
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Cannot find a gate with name \"{}\" in second grouping.", gate_0->get_name());
                    return false;
                }
                if (!gates_are_equal(gate_0, *g_1_list.begin(), ignore_id))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Gates with name \"{}\" are not equal.", gate_0->get_name());
                    return false;
                }
            }
            else
            {
                Gate* gate_1 = g_1->get_netlist()->get_gate_by_id(gate_0->get_id());
                if (!gates_are_equal(gate_0, gate_1, ignore_id, ignore_name))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Gates with name \"{}\" are not equal.", gate_0->get_name());
                    return false;
                }
                if (!g_1->contains_gate(gate_1))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Second grouping does not contain a gate with name \"{}\".", gate_1->get_name());
                    return false;
                }
            }
        }

        // Check if nets are the same
        if (g_0->get_nets().size() != g_1->get_nets().size())
        {
            log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: The number of nets is different ({} vs {})", g_0->get_nets().size(), g_1->get_nets().size());
            return false;
        }
        for (auto net_0 : g_0->get_nets())
        {
            if (ignore_id)
            {
                auto g_1_list = g_1->get_nets(net_name_filter(net_0->get_name()));
                if (g_1_list.size() > 1)
                {
                    log_info("test_utils", "groupings_are_equal: Groupings can't be compared! Reason: Multiple nets with name \"{}\" are found in the second grouping.", net_0->get_name());
                    return false;
                }
                if (g_1_list.size() < 1)
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Cannot find a net with name \"{}\" in second grouping.", net_0->get_name());
                    return false;
                }
                if (!nets_are_equal(net_0, *g_1_list.begin(), ignore_id))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Nets with name \"{}\" are not equal.", net_0->get_name());
                    return false;
                }
            }
            else
            {
                Net* net_1 = g_1->get_netlist()->get_net_by_id(net_0->get_id());
                if (!nets_are_equal(net_0, net_1, ignore_id, ignore_name))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Nets with name \"{}\" are not equal.", net_0->get_name());
                    return false;
                }
                if (!g_1->contains_net(net_1))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Second grouping does not contain a net with name \"{}\".", net_1->get_name());
                    return false;
                }
            }
        }

        // Check if modules are the same
        if (g_0->get_modules().size() != g_1->get_modules().size())
        {
            log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: The number of modules is different ({} vs {})", g_0->get_modules().size(), g_1->get_modules().size());
            return false;
        }
        for (auto module_0 : g_0->get_modules())
        {
            if (ignore_id)
            {
                auto g_1_list = g_1->get_modules(module_name_filter(module_0->get_name()));
                if (g_1_list.size() > 1)
                {
                    log_info("test_utils", "groupings_are_equal: Groupings can't be compared! Reason: Multiple modules with name \"{}\" are found in the second grouping.", module_0->get_name());
                    return false;
                }
                if (g_1_list.size() < 1)
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Cannot find a module with name \"{}\" in second grouping.", module_0->get_name());
                    return false;
                }
                if (!modules_are_equal(module_0, *g_1_list.begin(), ignore_id))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Modules with name \"{}\" are not equal.", module_0->get_name());
                    return false;
                }
            }
            else
            {
                Module* module_1 = g_1->get_netlist()->get_module_by_id(module_0->get_id());
                if (!modules_are_equal(module_0, module_1, ignore_id, ignore_name))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Modules with name \"{}\" are not equal.", module_0->get_name());
                    return false;
                }
                if (!g_1->contains_module(module_1))
                {
                    log_info("test_utils", "groupings_are_equal: Groupings are not equal! Reason: Second grouping does not contain a module with name \"{}\".", module_1->get_name());
                    return false;
                }
            }
        }

        return true;
    }

    bool test_utils::netlists_are_equal(Netlist* nl_0, Netlist* nl_1, const bool ignore_id)
    {
        if (nl_0 == nullptr || nl_1 == nullptr)
        {
            if (nl_0 == nl_1)
            {
                return true;
            }
            else
            {
                log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: One netlist is a nullptr.");
                return false;
            }
        }
        // Check if the ids are the same
        if (!ignore_id && nl_0->get_id() != nl_1->get_id())
        {
            log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: IDs are different (\"{}\" vs \"{}\")", nl_0->get_id(), nl_1->get_id());
            return false;
        }
        // Check that the Gate libraries are the same
        auto t_1 = nl_0->get_gate_library()->get_name();
        auto t_2 = nl_1->get_gate_library()->get_name();
        if (nl_0->get_gate_library()->get_name() != nl_1->get_gate_library()->get_name())
        {
            log_info("test_utils",
                     "netlists_are_equal: Netlists are not equal! Reason: Gate libraries are different (\"{}\" vs \"{}\")",
                     nl_0->get_gate_library()->get_name(),
                     nl_1->get_gate_library()->get_name());
            return false;
        }
        // Check that the design/device names are the same
        if (nl_0->get_design_name() != nl_1->get_design_name())
        {
            log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Design names are different (\"{}\" vs \"{}\")", nl_0->get_design_name(), nl_1->get_design_name());
            return false;
        }
        if (nl_0->get_device_name() != nl_1->get_device_name())
        {
            log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Device names are different (\"{}\" vs \"{}\")", nl_0->get_device_name(), nl_1->get_device_name());
            return false;
        }

        // Check if gates and nets are the same
        if (nl_0->get_gates().size() != nl_1->get_gates().size())
        {
            log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: The number of gates is different ({} vs {})", nl_0->get_gates().size(), nl_1->get_gates().size());
            return false;
        }
        for (auto g_0 : nl_0->get_gates())
        {
            if (ignore_id)
            {
                auto g_1_list = nl_1->get_gates(gate_name_filter(g_0->get_name()));
                if (g_1_list.size() > 1)
                {
                    log_info("test_utils", "netlists_are_equal: Netlists can't be compared! Reason: Multiple gates with name \"{}\" are found in the second netlist.", g_0->get_name());
                    return false;
                }
                if (g_1_list.size() < 1)
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Cannot find a gate with name \"{}\" in second netlist.", g_0->get_name());
                    return false;
                }
                if (!gates_are_equal(g_0, *g_1_list.begin(), ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Gates with name \"{}\" are not equal.", g_0->get_name());
                    return false;
                }
            }
            else
            {
                if (!gates_are_equal(g_0, nl_1->get_gate_by_id(g_0->get_id()), ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Gates with name \"{}\" are not equal.", g_0->get_name());
                    return false;
                }
            }
        }

        if (nl_0->get_nets().size() != nl_1->get_nets().size())
        {
            log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: The number of nets is different ({} vs {})", nl_0->get_nets().size(), nl_1->get_nets().size());
            return false;
        }
        for (auto n_0 : nl_0->get_nets())
        {
            if (ignore_id)
            {
                auto n_1_list = nl_1->get_nets(net_name_filter(n_0->get_name()));
                if (n_1_list.size() > 1)
                {
                    log_info("test_utils", "netlists_are_equal: Netlists can't be compared! Reason: Multiple nets with name \"{}\" are found in the second netlist.", n_0->get_name());
                    return false;
                }
                if (n_1_list.size() < 1)
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Cannot find a net with name \"{}\" in second netlist.", n_0->get_name());
                    return false;
                }
                if (!nets_are_equal(n_0, *n_1_list.begin(), ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Nets with name \"{}\" are not equal.", n_0->get_name());
                    return false;
                }
            }
            else
            {
                if (!nets_are_equal(n_0, nl_1->get_net_by_id(n_0->get_id()), ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Nets with name \"{}\" are not equal.", n_0->get_name());
                    return false;
                }
            }
        }

        // Check if the modules are the same
        if (nl_0->get_modules().size() != nl_1->get_modules().size())
        {
            log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: The number of modules is different ({} vs {})", nl_0->get_modules().size(), nl_1->get_modules().size());
            return false;
        }
        std::vector<Module*> mods_1 = nl_1->get_modules();
        for (auto m_0 : nl_0->get_modules())
        {
            if (ignore_id)
            {
                auto m_1_all = nl_1->get_modules();
                auto m_1     = std::find_if(m_1_all.begin(), m_1_all.end(), [m_0](Module* m) { return m->get_name() == m_0->get_name(); });
                if (m_1 == m_1_all.end())
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Cannot find a module with name \"{}\" in second netlist.", m_0->get_name());
                    return false;
                }
                if (!modules_are_equal(m_0, *m_1, ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Modules with name \"{}\" are not equal.", m_0->get_name());
                    return false;
                }
            }
            else
            {
                if (!modules_are_equal(m_0, nl_1->get_module_by_id(m_0->get_id()), ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Modules with name \"{}\" are not equal.", m_0->get_name());
                    return false;
                }
            }
        }

        // Check of the groupings are the same
        if (nl_0->get_groupings().size() != nl_1->get_groupings().size())
        {
            log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: The number of groupings is different ({} vs {})", nl_0->get_groupings().size(), nl_1->get_groupings().size());
            return false;
        }
        std::vector<Grouping*> groupings_1 = nl_1->get_groupings();
        for (auto g_0 : nl_0->get_groupings())
        {
            if (ignore_id)
            {
                auto g_1 = std::find_if(groupings_1.begin(), groupings_1.end(), [g_0](Grouping* g) { return g->get_name() == g_0->get_name(); });
                if (g_1 == groupings_1.end())
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Cannot find a grouping with name \"{}\" in second netlist.", g_0->get_name());
                    return false;
                }
                if (!groupings_are_equal(g_0, *g_1, ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Groupings with name \"{}\" are not equal.", g_0->get_name());
                    return false;
                }
            }
            else
            {
                if (!groupings_are_equal(g_0, nl_1->get_grouping_by_id(g_0->get_id()), ignore_id))
                {
                    log_info("test_utils", "netlists_are_equal: Netlists are not equal! Reason: Groupings with name \"{}\" are not equal.", g_0->get_name());
                    return false;
                }
            }
        }

        return true;
    }

    // Filter Functions

    std::function<bool(const Module*)> test_utils::module_name_filter(const std::string& name)
    {
        return [name](const Module* m) { return m->get_name() == name; };
    }

    std::function<bool(const Grouping*)> test_utils::grouping_name_filter(const std::string& name)
    {
        return [name](const Grouping* g) { return g->get_name() == name; };
    }

    std::function<bool(const Gate*)> test_utils::gate_filter(const std::string& type, const std::string& name)
    {
        return [name, type](const Gate* g) { return g->get_name() == name && g->get_type()->get_name() == type; };
    }

    std::function<bool(const Gate*)> test_utils::gate_name_filter(const std::string& name)
    {
        return [name](const Gate* g) { return g->get_name() == name; };
    }

    std::function<bool(const Gate*)> test_utils::gate_type_filter(const std::string& type)
    {
        return [type](const Gate* g) { return g->get_type()->get_name() == type; };
    }

    std::function<bool(const Net*)> test_utils::net_name_filter(const std::string& name)
    {
        return [name](const Net* n) { return n->get_name() == name; };
    }

    std::function<bool(const Endpoint*)> test_utils::endpoint_gate_type_filter(const std::string& gate_type)
    {
        return [gate_type](const Endpoint* ep) { return ep->get_gate()->get_type()->get_name() == gate_type; };
    }

    std::function<bool(const Endpoint*)> test_utils::endpoint_gate_name_filter(const std::string& name)
    {
        return [name](const Endpoint* ep) { return ep->get_gate()->get_name() == name; };
    }

    std::function<bool(const Endpoint*)> test_utils::endpoint_pin_filter(const GatePin* pin)
    {
        return [pin](const Endpoint* ep) { return *ep->get_pin() == *pin; };
    }

    std::function<bool(const GatePin*, const Endpoint*)> test_utils::adjacent_pin_filter(const std::string& pin_name)
    {
        return [pin_name](const GatePin*, const Endpoint* ep) { return ep->get_pin()->get_name() == pin_name; };
    }
    std::function<bool(const GatePin*, const Endpoint*)> test_utils::starting_pin_filter(const std::string& pin_name)
    {
        return [pin_name](const GatePin* starting_pin, const Endpoint*) { return starting_pin->get_name() == pin_name; };
    }

    std::function<bool(const GatePin*, const Endpoint*)> test_utils::adjacent_gate_type_filter(const std::string& type)
    {
        return [type](const GatePin*, const Endpoint* ep) { return ep->get_gate()->get_type()->get_name() == type; };
    }
}    // namespace hal
