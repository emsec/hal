#include "core/log.h"
#include "core/plugin_manager.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/hdl_parser/hdl_parser_manager.h"
#include "netlist/hdl_writer/hdl_writer_manager.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "plugin_netlist_simulator/plugin_netlist_simulator.h"
#include "test_utils/include/test_def.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

namespace hal
{
    class SimulatorTest : public ::testing::Test
    {
    protected:
        NetlistSimulatorPlugin* plugin;
        std::unique_ptr<NetlistSimulator> sim;

        virtual void SetUp()
        {
            NO_COUT_BLOCK;

            plugin_manager::load_all_plugins();
            gate_library_manager::load_all();
            auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("libnetlist_simulator");
        }

        virtual void TearDown()
        {
            NO_COUT_BLOCK;
            plugin_manager::unload_all_plugins();
        }

        Simulation simulate(u64 nanoseconds)
        {
            sim->simulate(nanoseconds);
            return sim->get_current_state();
        }

        bool cmp_sim_data(const Simulation& vcd_sim, const Simulation& hal_sim)
        {
            auto a_events = vcd_sim.get_events();
            auto b_events = hal_sim.get_events();
            for (auto it = b_events.begin(); it != b_events.end();)
            {
                auto srcs = it->first->get_sources();
                if (srcs.size() == 1 && (srcs[0].get_gate()->is_gnd_gate() || srcs[0].get_gate()->is_vcc_gate())) // && a_events.find(it->first) == a_events.end())
                {
                    it = b_events.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            auto signal_to_string = [](auto v) -> std::string {
                if (v >= 0)
                    return std::to_string(v);
                return "X";
            };

            std::set<Net*> a_nets;
            std::set<Net*> b_nets;
            for (auto it : a_events)
            {
                a_nets.insert(it.first);
            }
            for (auto it : b_events)
            {
                b_nets.insert(it.first);
            }

            std::map<std::string, Net*> sorted_unmatching_events;

            for (auto it : a_events)
            {
                auto net       = it.first;
                auto& events_a = it.second;
                auto& events_b = b_events[net];

                if (events_a != events_b)
                {
                    sorted_unmatching_events[net->get_name()] = net;
                }
            }

            u64 earliest_mismatch = -1;
            std::vector<Net*> earliest_mismatch_nets;
            auto update_mismatch = [&](auto ev) {
                if (ev.time < earliest_mismatch)
                {
                    earliest_mismatch      = ev.time;
                    earliest_mismatch_nets = {ev.affected_net};
                }
                else if (ev.time == earliest_mismatch)
                {
                    earliest_mismatch_nets.push_back(ev.affected_net);
                }
            };

            for (auto [name, net] : sorted_unmatching_events)
            {
                auto& events_a        = a_events[net];
                auto& events_b        = b_events[net];
                u32 max_number_length = 0;
                if (!events_a.empty() && !events_b.empty())
                {
                    max_number_length = std::to_string(std::max(events_a.back().time, events_b.back().time)).size();
                }
                std::cout << "difference in net " << net->get_name() << " id=" << net->get_id() << ":" << std::endl;
                std::cout << "vcd:" << std::setfill(' ') << std::setw(max_number_length + 5) << ""
                          << "hal:" << std::endl;
                for (u32 i = 0, j = 0; i < events_a.size() || j < events_b.size();)
                {
                    if (i < events_a.size() && j < events_b.size())
                    {
                        if (events_a[i] == events_b[j])
                        {
                            std::cout << signal_to_string(events_a[i].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].time << "ns";
                            std::cout << " | ";
                            std::cout << signal_to_string(events_b[j].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].time << "ns";
                            std::cout << std::endl;
                            i++;
                            j++;
                        }
                        else if (events_a[i].time == events_b[j].time)
                        {
                            update_mismatch(events_a[i]);
                            std::cout << signal_to_string(events_a[i].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].time << "ns";
                            std::cout << " | ";
                            std::cout << signal_to_string(events_b[j].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].time << "ns";
                            std::cout << "  <--" << std::endl;
                            i++;
                            j++;
                        }
                        else
                        {
                            if (events_a[i].time < events_b[j].time)
                            {
                                update_mismatch(events_a[i]);
                                std::cout << signal_to_string(events_a[i].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].time << "ns";
                                std::cout << " | ";
                                std::cout << std::endl;
                                i++;
                            }
                            else
                            {
                                update_mismatch(events_b[j]);
                                std::cout << "    " << std::setfill(' ') << std::setw(max_number_length) << ""
                                          << "  ";
                                std::cout << " | ";
                                std::cout << signal_to_string(events_b[j].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].time << "ns";
                                std::cout << std::endl;
                                j++;
                            }
                        }
                    }
                    else if (i < events_a.size())
                    {
                        update_mismatch(events_a[i]);
                        std::cout << signal_to_string(events_a[i].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].time << "ns";
                        std::cout << " | ";
                        std::cout << std::endl;
                        i++;
                    }
                    else
                    {
                        update_mismatch(events_b[j]);
                        std::cout << "    " << std::setfill(' ') << std::setw(max_number_length) << ""
                                  << "  ";
                        std::cout << " | ";
                        std::cout << signal_to_string(events_b[j].new_value) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].time << "ns";
                        std::cout << std::endl;
                        j++;
                    }
                }
                std::cout << std::endl;
            }

            if (a_events.size() != b_events.size())
            {
                std::cout << "WARNING SIZE MISMATCH" << std::endl;
                if (a_events.size() > b_events.size())
                {
                    std::cout << "more nets are captured in the vcd file:" << std::endl;
                    std::vector<Net*> mismatch;
                    std::set_difference(a_nets.begin(), a_nets.end(), b_nets.begin(), b_nets.end(), std::back_inserter(mismatch));
                    for (auto x : mismatch)
                    {
                        std::cout << "  " << x->get_name() << std::endl;
                    }
                }
                else
                {
                    std::cout << "more nets are captured in the simulation output:" << std::endl;
                    std::vector<Net*> mismatch;
                    std::set_difference(b_nets.begin(), b_nets.end(), a_nets.begin(), a_nets.end(), std::back_inserter(mismatch));
                    for (auto x : mismatch)
                    {
                        std::cout << "  " << x->get_name() << std::endl;
                    }
                }

                if (sorted_unmatching_events.empty())
                {
                    std::cout << "everything that could be compared was correct, though!" << std::endl;
                }
            }

            if (!earliest_mismatch_nets.empty())
            {
                std::cout << "earliest mismatch at " << earliest_mismatch << "ns" << std::endl;
                for (auto net : earliest_mismatch_nets)
                    std::cout << "mismatch at " << net->get_name() << std::endl;
            }
            if (a_events == b_events)
            {
                std::cout << "simulation correct!" << std::endl;
                return true;
            }

            return false;
        }

        Simulation parse_vcd(Netlist* netlist, const std::string vcdfile, const int duration_clk_cycle)
        {
            Simulation vcd_trace;
            std::ifstream infile(vcdfile);
            std::string line;

            //map for storing the variable names from the vcd and their identifiers
            std::map<std::string, std::string> identifier_to_net_name;
            //map for storing the u32 id of the net and its corresponding signal value at a specific cycle
            std::map<Net*, SignalValue> current_state;

            //map for storing gate names and corresponding u32 id for all nets of the netlist
            std::map<std::string, Net*> net_name_to_net;
            //Fill map
            for (auto net : netlist->get_nets())
            {
                net_name_to_net[net->get_name()] = net;
                current_state[net]               = SignalValue::Z;
            }

            int time = 0;    //current timestamp of vcd file

            // fill map with (signal) names from vcd as key and identifiers as value
            while (std::getline(infile, line))
            {
                line = core_utils::trim(line);
                if (core_utils::starts_with(line, std::string("$var ")))
                {
                    line                   = line.substr(0, line.find_last_of(" "));
                    line                   = core_utils::trim(line);
                    std::string name       = line.substr(line.find_last_of(" ") + 1);
                    line                   = line.substr(0, line.find_last_of(" "));
                    std::string identifier = line.substr(line.find_last_of(" ") + 1);

                    identifier_to_net_name.emplace(identifier, name);
                }

                if (line == "$enddefinitions $end")
                {
                    break;
                }
            }
            while (std::getline(infile, line))
            {
                line                   = core_utils::trim(line);
                std::string identifier = line.substr(1, line.length());
                Net* current_net       = nullptr;
                if (auto it = identifier_to_net_name.find(identifier); it != identifier_to_net_name.end())
                {
                    current_net = net_name_to_net[it->second];
                }
                if (line[0] == '#')
                {
                    //new cycle number
                    time = std::stoi(line.substr(1, line.length())) / 1000;
                }
                else if (line[0] == '0')
                {
                    if (current_state[current_net] != SignalValue::ZERO)
                    {
                        Event e;
                        e.affected_net = current_net;
                        e.time         = time;
                        e.new_value    = SignalValue::ZERO;
                        vcd_trace.add_event(e);
                        current_state[current_net] = e.new_value;
                    }
                }
                else if (line[0] == '1')
                {
                    if (current_state[current_net] != SignalValue::ONE)
                    {
                        Event e;
                        e.affected_net = current_net;
                        e.time         = time;
                        e.new_value    = SignalValue::ONE;
                        vcd_trace.add_event(e);
                        current_state[current_net] = e.new_value;
                    }
                }
                else if (line[0] == 'x')
                {
                    if (current_state[current_net] != SignalValue::X)
                    {
                        Event e;
                        e.affected_net = current_net;
                        e.time         = time;
                        e.new_value    = SignalValue::X;
                        vcd_trace.add_event(e);
                        current_state[current_net] = e.new_value;
                    }
                }
            }

            return vcd_trace;
        }
    };    // namespace hal

    TEST_F(SimulatorTest, half_adder)
    {
        return;
        TEST_START
        sim = plugin->create_simulator();

        std::string path_netlist = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/half_adder/halfaddernetlist_flattened_by_hal.v";
        if (!core_utils::file_exists(path_netlist))
        {
            FAIL() << "netlis for counter-test not found: " << path_netlist;
        }

        //create netlist from path
        auto lib = gate_library_manager::get_gate_library_by_name("XILINX_UNISIM");
        if (lib == nullptr)
        {
            FAIL() << "XILINX_UNISIM gate library not found";
        }

        std::unique_ptr<Netlist> nl;
        {
            NO_COUT_BLOCK;
            nl = hdl_parser_manager::parse(path_netlist, lib);
            if (nl == nullptr)
            {
                FAIL() << "netlist couldn't be parsed";
            }
        }

        //path to vcd
        std::string path_vcd = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/half_adder/dump.vcd";
        if (!core_utils::file_exists(path_vcd))
        {
            FAIL() << "dump for half_adder-test not found: " << path_vcd;
        }
        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, 10000);

        //vector of states for hal simulation
        Simulation hal_sim_traces;
        //vector of gates for simulation function add_gates
        std::vector<Gate*> vector_of_gates;
        //add gates to vector
        for (const auto& gate : nl->get_gates())
            vector_of_gates.push_back(gate);
        //vector of traces for interim result
        Simulation interim_traces;

        //prepare simulation
        sim->add_gates(vector_of_gates);
        sim->load_initial_values();
        auto A = *(nl->get_nets([](auto net) { return net->get_name() == "A"; }).begin());
        auto B = *(nl->get_nets([](auto net) { return net->get_name() == "B"; }).begin());

        //start simulation
        //Testbench
        sim->set_input(A, SignalValue::ZERO);    //A=0
        sim->set_input(B, SignalValue::ZERO);    //B=0
        hal_sim_traces = simulate(10);

        sim->set_input(A, SignalValue::ZERO);    //A=0
        sim->set_input(B, SignalValue::ONE);     //B=1
        hal_sim_traces = simulate(10);

        sim->set_input(A, SignalValue::ONE);     //A=1
        sim->set_input(B, SignalValue::ZERO);    //B=0
        hal_sim_traces = simulate(10);

        sim->set_input(A, SignalValue::ONE);    //A=1
        sim->set_input(B, SignalValue::ONE);    //B=1
        hal_sim_traces = simulate(10);

        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, hal_sim_traces));
        TEST_END
    }

    TEST_F(SimulatorTest, counter)
    {
        return;
        TEST_START
        sim = plugin->create_simulator();

        //path to netlist
        std::string path_netlist = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/counter/counternetlist_flattened_by_hal.vhd";
        if (!core_utils::file_exists(path_netlist))
            FAIL() << "netlist for counter-test not found: " << path_netlist;

        //create netlist from path
        auto lib = gate_library_manager::get_gate_library_by_name("XILINX_UNISIM");
        if (lib == nullptr)
        {
            FAIL() << "XILINX_UNISIM gate library not found";
        }

        std::unique_ptr<Netlist> nl;
        {
            NO_COUT_BLOCK;
            nl = hdl_parser_manager::parse(path_netlist, lib);
            if (nl == nullptr)
            {
                FAIL() << "netlist couldn't be parsed";
            }
        }

        //path to vcd
        std::string path_vcd = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/counter/dump.vcd";
        if (!core_utils::file_exists(path_vcd))
            FAIL() << "dump for counter-test not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, 10000);

        //vector of states for hal simulation
        Simulation hal_sim_traces;
        //vector of gates for simulation function add_gates
        std::vector<Gate*> vector_of_gates;
        //add gates to vector
        for (const auto& gate : nl->get_gates())
            vector_of_gates.push_back(gate);
        //vector of traces for interim result
        Simulation interim_traces;

        //prepare simulation
        sim->add_gates(vector_of_gates);
        sim->load_initial_values();

        // retrieve nets
        auto reset          = *(nl->get_nets([](auto net) { return net->get_name() == "Reset"; }).begin());
        auto clock          = *(nl->get_nets([](auto net) { return net->get_name() == "Clock"; }).begin());
        auto Clock_enable_B = *(nl->get_nets([](auto net) { return net->get_name() == "Clock_enable_B"; }).begin());
        auto output_0       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_0"; }).begin());
        auto output_1       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_1"; }).begin());
        auto output_2       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_2"; }).begin());
        auto output_3       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_3"; }).begin());

        sim->add_clock_period(clock, 10);

        //start simulation
        //testbench
        sim->set_input(Clock_enable_B, SignalValue::ONE);    //#Clock_enable_B <= '1';
        sim->set_input(reset, SignalValue::ZERO);            //#Reset <= '0';
        hal_sim_traces = simulate(40);                       //#WAIT FOR 40 NS; -> simulate 4 clock cycle  - cycle 0, 1, 2, 3

        sim->set_input(Clock_enable_B, SignalValue::ZERO);    //#Clock_enable_B <= '0';
        hal_sim_traces = simulate(110);                       //#WAIT FOR 110 NS; -> simulate 11 clock cycle  - cycle 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14

        sim->set_input(reset, SignalValue::ONE);    //#Reset <= '1';
        hal_sim_traces = simulate(20);              //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 15, 16

        sim->set_input(reset, SignalValue::ZERO);    //#Reset <= '0';
        hal_sim_traces = simulate(70);               //#WAIT FOR 70 NS; -> simulate 7 clock cycle  - cycle 17, 18, 19, 20, 21, 22, 23

        sim->set_input(Clock_enable_B, SignalValue::ONE);    //#Clock_enable_B <= '1';
        hal_sim_traces = simulate(23);                       //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 24, 25

        sim->set_input(reset, SignalValue::ONE);    //#Reset <= '1';
        hal_sim_traces = simulate(20);              //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 26, 27
                                                    //#3 additional traces á 10 NS to get 300 NS simulation time
        hal_sim_traces = simulate(20);              //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 28, 29
                                                    //#for last "cycle" set clock to 0, in the final state clock stays ZERO and does not switch to 1 anymore
        hal_sim_traces = simulate(5);               //#WAIT FOR 10 NS; -> simulate 1 clock cycle  - cycle 30

        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, hal_sim_traces));
        TEST_END
    }

    TEST_F(SimulatorTest, toycipher)
    {
        return;
        TEST_START
        sim = plugin->create_simulator();

        //path to netlist
        std::string path_netlist = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/toycipher/cipher_flat.vhd";
        if (!core_utils::file_exists(path_netlist))
            FAIL() << "netlist for toycipher-test not found: " << path_netlist;

        //create netlist from path
        auto lib = gate_library_manager::get_gate_library_by_name("XILINX_UNISIM");
        if (lib == nullptr)
        {
            FAIL() << "XILINX_UNISIM gate library not found";
        }

        std::unique_ptr<Netlist> nl;
        {
            NO_COUT_BLOCK;
            nl = hdl_parser_manager::parse(path_netlist, lib);
            if (nl == nullptr)
            {
                FAIL() << "netlist couldn't be parsed";
            }
        }

        //path to vcd
        std::string path_vcd = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/toycipher/dump.vcd";
        if (!core_utils::file_exists(path_vcd))
            FAIL() << "dump for toycipher-test not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, 10000);

        //vector of states for hal simulation
        Simulation hal_sim_traces;
        //vector of gates for simulation function add_gates
        std::vector<Gate*> vector_of_gates;
        //add gates to vector
        for (const auto& gate : nl->get_gates())
            vector_of_gates.push_back(gate);
        //vector of traces for interim result
        Simulation interim_traces;

        //prepare simulation
        sim->add_gates(vector_of_gates);
        sim->load_initial_values();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "CLK"; }).begin());

        sim->add_clock_period(clk, 10);

        std::set<Net*> key_set, plaintext_set;
        auto start = *(nl->get_nets([](auto net) { return net->get_name() == "START"; }).begin());

        for (int i = 0; i < 16; i++)
        {
            std::string name = "KEY_" + std::to_string(i);
            key_set.insert(*(nl->get_nets([name](auto net) { return net->get_name() == name; }).begin()));
        }

        for (int i = 0; i < 16; i++)
        {
            std::string name = "PLAINTEXT_" + std::to_string(i);
            plaintext_set.insert(*(nl->get_nets([name](auto net) { return net->get_name() == name; }).begin()));
        }

        int input_nets_amount = key_set.size() + plaintext_set.size();

        if (clk != nullptr)
            input_nets_amount++;

        if (start != nullptr)
            input_nets_amount++;

        if (input_nets_amount != sim->get_input_nets().size())
            FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim->get_input_nets().size();

        //start simulation
        //testbench

        for (const auto& net : plaintext_set)    //PLAINTEXT <= (OTHERS => '0');
            sim->set_input(net, SignalValue::ZERO);

        for (const auto& net : key_set)    //KEY <= (OTHERS => '0');
            sim->set_input(net, SignalValue::ZERO);

        sim->set_input(start, SignalValue::ZERO);    //START <= '0';
        hal_sim_traces = simulate(10);               //WAIT FOR 10 NS;

        sim->set_input(start, SignalValue::ONE);    //START <= '1';
        hal_sim_traces = simulate(10);              //WAIT FOR 10 NS;

        sim->set_input(start, SignalValue::ZERO);    //START <= '0';
        hal_sim_traces = simulate(100);              //WAIT FOR 100 NS;

        for (const auto& net : plaintext_set)    //PLAINTEXT <= (OTHERS => '1');
            sim->set_input(net, SignalValue::ONE);

        for (const auto& net : key_set)    //KEY <= (OTHERS => '1');
            sim->set_input(net, SignalValue::ONE);

        sim->set_input(start, SignalValue::ZERO);    //START <= '0';
        hal_sim_traces = simulate(10);               //WAIT FOR 10 NS;

        sim->set_input(start, SignalValue::ONE);    //START <= '1';
        hal_sim_traces = simulate(10);              //WAIT FOR 10 NS;

        sim->set_input(start, SignalValue::ZERO);    //START <= '0';
        hal_sim_traces = simulate(100);              //WAIT FOR 100 NS;

        for (const auto& net : plaintext_set)    //PLAINTEXT <= (OTHERS => '0');
            sim->set_input(net, SignalValue::ZERO);

        for (const auto& net : key_set)    //KEY <= (OTHERS => '0');
            sim->set_input(net, SignalValue::ZERO);

        sim->set_input(start, SignalValue::ZERO);    //START <= '0';

        hal_sim_traces = simulate(10);
        sim->set_input(start, SignalValue::ONE);    //START <= '1';

        hal_sim_traces = simulate(10);
        sim->set_input(start, SignalValue::ZERO);    //START <= '0';

        hal_sim_traces = simulate(30);

        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, hal_sim_traces));
        TEST_END
    }

    TEST_F(SimulatorTest, sha256)
    {
        TEST_START
        sim = plugin->create_simulator();

        //path to netlist
        std::string path_netlist = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/sha256/sha256_flat.vhd";
        if (!core_utils::file_exists(path_netlist))
            FAIL() << "netlist for sha256 not found: " << path_netlist;

        //create netlist from path
        auto lib = gate_library_manager::get_gate_library_by_name("XILINX_UNISIM");
        if (lib == nullptr)
        {
            FAIL() << "XILINX_UNISIM gate library not found";
        }

        std::unique_ptr<Netlist> nl;
        {
            NO_COUT_BLOCK;
            nl = hdl_parser_manager::parse(path_netlist, lib);
            if (nl == nullptr)
            {
                FAIL() << "netlist couldn't be parsed";
            }
        }

        //path to vcd
        std::string path_vcd = core_utils::get_base_directory().string() + "/bin/hal_plugins/test-files/sha256/dump.vcd";
        if (!core_utils::file_exists(path_vcd))
            FAIL() << "dump for sha256 not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, 10000);

        std::cout << vcd_traces.get_events().size() << std::endl;
        // return;

        //vector of states for hal simulation
        Simulation hal_sim_traces;
        //vector of gates for simulation function add_gates
        std::vector<Gate*> vector_of_gates;
        //add gates to vector
        for (const auto& gate : nl->get_gates())
            vector_of_gates.push_back(gate);
        //vector of traces for interim result
        Simulation interim_traces;

        //prepare simulation
        sim->add_gates(vector_of_gates);
        sim->load_initial_values();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "clk"; }).begin());

        std::cout << "#clock events: " << vcd_traces.get_events()[clk].size() << std::endl;

        sim->add_clock_period(clk, 10);

        auto start = *(nl->get_nets([](auto net) { return net->get_name() == "data_ready"; }).begin());

        auto rst = *(nl->get_nets([](auto net) { return net->get_name() == "rst"; }).begin());

        std::vector<Net*> input_bits;
        for (int i = 0; i < 512; i++)
        {
            std::string name = "msg_block_in_" + std::to_string(i);
            input_bits.push_back(*(nl->get_nets([name](auto net) { return net->get_name() == name; }).begin()));
        }

        int input_nets_amount = input_bits.size();

        if (clk != nullptr)
            input_nets_amount++;

        if (rst != nullptr)
            input_nets_amount++;

        if (start != nullptr)
            input_nets_amount++;

        if (input_nets_amount != sim->get_input_nets().size())
            FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim->get_input_nets().size();

        //start simulation
        std::cout << "starting simulation" << std::endl;
        //testbench

        // msg <= x"61626380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000018";
        std::string hex_input = "61626380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000018";
        for (u32 i = 0; i < hex_input.size(); i += 2)
        {
            u8 byte = std::stoul(hex_input.substr(i, 2), nullptr, 16);
            for (u32 j = 0; j < 8; ++j)
            {
                sim->set_input(input_bits[i * 4 + j], (SignalValue)((byte >> (7 - j)) & 1));
            }
        }

        sim->set_input(rst, SignalValue::ONE);       //RST <= '1';
        sim->set_input(start, SignalValue::ZERO);    //START <= '0';
        hal_sim_traces = simulate(10);               //WAIT FOR 10 NS;

        // sim->set_input(rst, SignalValue::ZERO);    //RST <= '0';
        // hal_sim_traces = simulate(10);             //WAIT FOR 10 NS;

        // sim->set_input(start, SignalValue::ONE);    //START <= '1';
        // hal_sim_traces = simulate(10);              //WAIT FOR 10 NS;

        // sim->set_input(start, SignalValue::ZERO);    //START <= '0';
        // hal_sim_traces = simulate(10);               //WAIT FOR 10 NS;

        // hal_sim_traces = simulate(2000);

        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, hal_sim_traces));
        TEST_END
    }
}    // namespace hal
