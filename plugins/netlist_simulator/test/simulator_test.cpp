#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/hdl_parser/hdl_parser_manager.h"
#include "hal_core/netlist/hdl_writer/hdl_writer_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "netlist_simulator/plugin_netlist_simulator.h"
#include "test_utils/include/test_def.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

namespace hal
{
#define seconds_since(X) ((double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - (X)).count() / 1000)

#define measure_block_time(X) measure_block_time_t UNIQUE_NAME(X);

    class measure_block_time_t
    {
    public:
        measure_block_time_t(const std::string& section_name)
        {
            m_name       = section_name;
            m_begin_time = std::chrono::high_resolution_clock::now();
        }

        ~measure_block_time_t()
        {
            std::cout << m_name << " took " << std::setprecision(2) << seconds_since(m_begin_time) << "s" << std::endl;
        }

    private:
        std::string m_name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_begin_time;
    };

    class SimulatorTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            plugin_manager::load_all_plugins();
            gate_library_manager::get_gate_library("XILINX_UNISIM.lib");
        }

        virtual void TearDown()
        {
            NO_COUT_BLOCK;
            plugin_manager::unload_all_plugins();
        }

        bool cmp_sim_data(const Simulation& vcd_sim, const Simulation& hal_sim)
        {
            auto a_events = vcd_sim.get_events();
            auto b_events = hal_sim.get_events();

            std::cout << "comparing outputs..." << std::endl;

            for (auto it = b_events.begin(); it != b_events.end();)
            {
                auto srcs = it->first->get_sources();
                if (srcs.size() == 1 && (srcs[0]->get_gate()->is_gnd_gate() || srcs[0]->get_gate()->is_vcc_gate()) && a_events.find(it->first) == a_events.end())
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

            std::cout << "finding mismatches..." << std::endl;

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

            std::cout << "printing mismatches..." << std::endl;

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
                std::vector<Gate*> mismatch_sources;
                for (auto net : earliest_mismatch_nets)
                {
                    std::cout << "mismatch at " << net->get_name() << std::endl;
                    auto srcs = net->get_sources();
                    std::transform(srcs.begin(), srcs.end(), std::back_inserter(mismatch_sources), [](auto& ep) { return ep->get_gate(); });
                }
                std::unordered_set<Gate*> mismatch_sources_set(mismatch_sources.begin(), mismatch_sources.end());
                for (auto it = mismatch_sources.begin(); it != mismatch_sources.end();)
                {
                    auto g       = *it;
                    auto preds   = g->get_unique_predecessors();
                    bool removed = false;
                    for (auto pred : preds)
                    {
                        if (pred == g)
                        {
                            continue;
                        }
                        if (mismatch_sources_set.find(pred) != mismatch_sources_set.end())
                        {
                            it      = mismatch_sources.erase(it);
                            removed = true;
                            break;
                        }
                    }
                    if (!removed)
                    {
                        ++it;
                    }
                }
                std::cout << std::endl;
                for (auto g : mismatch_sources)
                {
                    std::cout << "mismatch source " << g->get_name() << std::endl;
                }
            }
            if (a_events == b_events)
            {
                std::cout << "simulation correct!" << std::endl;
                return true;
            }

            return false;
        }

        Simulation parse_vcd(Netlist* netlist, const std::string vcdfile, bool normalize_to_nanoseconds)
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
                line = utils::trim(line);
                if (utils::starts_with(line, std::string("$var ")))
                {
                    line                   = line.substr(0, line.find_last_of(" "));
                    line                   = utils::trim(line);
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
                line = utils::trim(line);
                if (line.empty())
                    continue;
                std::string identifier = line.substr(1, line.length());
                Net* current_net       = nullptr;
                if (auto it = identifier_to_net_name.find(identifier); it != identifier_to_net_name.end())
                {
                    if (auto it2 = net_name_to_net.find(it->second); it2 != net_name_to_net.end())
                    {
                        current_net = it2->second;
                    }
                    else if (auto it2 = net_name_to_net.find(utils::to_lower(it->second)); it2 != net_name_to_net.end())
                    {
                        current_net = it2->second;
                    }
                }
                if (line[0] == '#')
                {
                    //new cycle number
                    time = std::stoi(line.substr(1, line.length()));
                    if (normalize_to_nanoseconds)
                    {
                        time /= 1000;
                        time *= 1000;
                    }
                }
                else if (line[0] == '0')
                {
                    if (current_net == nullptr)
                    {
                        std::cout << "ERROR: no net found for identifier " << identifier << std::endl;
                        return Simulation();
                    }
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
                    if (current_net == nullptr)
                    {
                        std::cout << "ERROR: no net found for identifier " << identifier << std::endl;
                        return Simulation();
                    }
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
                    if (current_net == nullptr)
                    {
                        std::cout << "ERROR: no net found for identifier " << identifier << std::endl;
                        return Simulation();
                    }
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
        // return;
        TEST_START
        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("libnetlist_simulator");
        auto sim    = plugin->create_simulator();

        std::string path_netlist = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/half_adder/halfaddernetlist_flattened_by_hal.v";
        if (!utils::file_exists(path_netlist))
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
        std::string path_vcd = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/half_adder/dump.vcd";
        if (!utils::file_exists(path_vcd))
        {
            FAIL() << "dump for half_adder-test not found: " << path_vcd;
        }
        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, true);

        //prepare simulation
        sim->add_gates(nl->get_gates());
        sim->load_initial_values_from_netlist();
        auto A = *(nl->get_nets([](auto net) { return net->get_name() == "A"; }).begin());
        auto B = *(nl->get_nets([](auto net) { return net->get_name() == "B"; }).begin());

        //start simulation
        {
            measure_block_time("simulation");
            //Testbench
            sim->set_input(A, SignalValue::ZERO);    //A=0
            sim->set_input(B, SignalValue::ZERO);    //B=0
            sim->simulate(10 * 1000);

            sim->set_input(A, SignalValue::ZERO);    //A=0
            sim->set_input(B, SignalValue::ONE);     //B=1
            sim->simulate(10 * 1000);

            sim->set_input(A, SignalValue::ONE);     //A=1
            sim->set_input(B, SignalValue::ZERO);    //B=0
            sim->simulate(10 * 1000);

            sim->set_input(A, SignalValue::ONE);    //A=1
            sim->set_input(B, SignalValue::ONE);    //B=1
            sim->simulate(10 * 1000);
        }

        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, sim->get_simulation_state()));
        TEST_END
    }

    TEST_F(SimulatorTest, counter)
    {
        // return;
        TEST_START

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("libnetlist_simulator");
        auto sim    = plugin->create_simulator();

        //path to netlist
        std::string path_netlist = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/counter/counternetlist_flattened_by_hal.vhd";
        if (!utils::file_exists(path_netlist))
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
        std::string path_vcd = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/counter/dump.vcd";
        if (!utils::file_exists(path_vcd))
            FAIL() << "dump for counter-test not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, true);

        //prepare simulation
        sim->add_gates(nl->get_gates());
        sim->load_initial_values_from_netlist();

        // retrieve nets
        auto reset          = *(nl->get_nets([](auto net) { return net->get_name() == "Reset"; }).begin());
        auto clock          = *(nl->get_nets([](auto net) { return net->get_name() == "Clock"; }).begin());
        auto Clock_enable_B = *(nl->get_nets([](auto net) { return net->get_name() == "Clock_enable_B"; }).begin());
        auto output_0       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_0"; }).begin());
        auto output_1       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_1"; }).begin());
        auto output_2       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_2"; }).begin());
        auto output_3       = *(nl->get_nets([](auto net) { return net->get_name() == "Output_3"; }).begin());

        sim->add_clock_period(clock, 10000);

        //start simulation
        {
            measure_block_time("simulation");
            //testbench
            sim->set_input(Clock_enable_B, SignalValue::ONE);    //#Clock_enable_B <= '1';
            sim->set_input(reset, SignalValue::ZERO);            //#Reset <= '0';
            sim->simulate(40 * 1000);                            //#WAIT FOR 40 NS; -> simulate 4 clock cycle  - cycle 0, 1, 2, 3

            sim->set_input(Clock_enable_B, SignalValue::ZERO);    //#Clock_enable_B <= '0';
            sim->simulate(110 * 1000);                            //#WAIT FOR 110 NS; -> simulate 11 clock cycle  - cycle 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14

            sim->set_input(reset, SignalValue::ONE);    //#Reset <= '1';
            sim->simulate(20 * 1000);                   //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 15, 16

            sim->set_input(reset, SignalValue::ZERO);    //#Reset <= '0';
            sim->simulate(70 * 1000);                    //#WAIT FOR 70 NS; -> simulate 7 clock cycle  - cycle 17, 18, 19, 20, 21, 22, 23

            sim->set_input(Clock_enable_B, SignalValue::ONE);    //#Clock_enable_B <= '1';
            sim->simulate(23 * 1000);                            //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 24, 25

            sim->set_input(reset, SignalValue::ONE);    //#Reset <= '1';
            sim->simulate(20 * 1000);                   //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 26, 27
                                                        //#3 additional traces á 10 NS to get 300 NS simulation time
            sim->simulate(20 * 1000);                   //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 28, 29
                                                        //#for last "cycle" set clock to 0, in the final state clock stays ZERO and does not switch to 1 anymore
            sim->simulate(5 * 1000);                    //#WAIT FOR 10 NS; -> simulate 1 clock cycle  - cycle 30
        }
        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, sim->get_simulation_state()));
        TEST_END
    }

    TEST_F(SimulatorTest, toycipher)
    {
        // return;
        TEST_START

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("libnetlist_simulator");
        auto sim    = plugin->create_simulator();

        //path to netlist
        std::string path_netlist = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/toycipher/cipher_flat.vhd";
        if (!utils::file_exists(path_netlist))
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
        std::string path_vcd = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/toycipher/dump.vcd";
        if (!utils::file_exists(path_vcd))
            FAIL() << "dump for toycipher-test not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, true);

        //prepare simulation
        sim->add_gates(nl->get_gates());
        sim->load_initial_values_from_netlist();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "CLK"; }).begin());

        sim->add_clock_period(clk, 10000);

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
        {
            measure_block_time("simulation");
            //testbench

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '0');
                sim->set_input(net, SignalValue::ZERO);

            for (auto net : key_set)    //KEY <= (OTHERS => '0');
                sim->set_input(net, SignalValue::ZERO);

            sim->set_input(start, SignalValue::ZERO);    //START <= '0';
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->set_input(start, SignalValue::ONE);    //START <= '1';
            sim->simulate(10 * 1000);                   //WAIT FOR 10 NS;

            sim->set_input(start, SignalValue::ZERO);    //START <= '0';
            sim->simulate(100 * 1000);                   //WAIT FOR 100 NS;

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '1');
                sim->set_input(net, SignalValue::ONE);

            for (auto net : key_set)    //KEY <= (OTHERS => '1');
                sim->set_input(net, SignalValue::ONE);

            sim->set_input(start, SignalValue::ZERO);    //START <= '0';
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->set_input(start, SignalValue::ONE);    //START <= '1';
            sim->simulate(10 * 1000);                   //WAIT FOR 10 NS;

            sim->set_input(start, SignalValue::ZERO);    //START <= '0';
            sim->simulate(100 * 1000);                   //WAIT FOR 100 NS;

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '0');
                sim->set_input(net, SignalValue::ZERO);

            for (auto net : key_set)    //KEY <= (OTHERS => '0');
                sim->set_input(net, SignalValue::ZERO);

            sim->set_input(start, SignalValue::ZERO);    //START <= '0';

            sim->simulate(10 * 1000);
            sim->set_input(start, SignalValue::ONE);    //START <= '1';

            sim->simulate(10 * 1000);
            sim->set_input(start, SignalValue::ZERO);    //START <= '0';

            sim->simulate(30 * 1000);
        }

        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, sim->get_simulation_state()));
        TEST_END
    }

    TEST_F(SimulatorTest, sha256)
    {
        // return;
        TEST_START

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("libnetlist_simulator");
        auto sim    = plugin->create_simulator();

        //path to netlist
        std::string path_netlist = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/sha256/sha256_flat.vhd";
        if (!utils::file_exists(path_netlist))
            FAIL() << "netlist for sha256 not found: " << path_netlist;

        std::string path_netlist_hal = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/sha256/sha256_flat.hal";

        //create netlist from path
        auto lib = gate_library_manager::get_gate_library_by_name("XILINX_UNISIM");
        if (lib == nullptr)
        {
            FAIL() << "XILINX_UNISIM gate library not found";
        }

        std::unique_ptr<Netlist> nl;
        {
            std::cout << "loading netlist: " << path_netlist << "..." << std::endl;
            if (utils::file_exists(path_netlist_hal))
            {
                std::cout << ".hal file found for test netlist, loading this one." << std::endl;
                nl = netlist_serializer::deserialize_from_file(path_netlist_hal);
            }
            else
            {
                NO_COUT_BLOCK;
                nl = hdl_parser_manager::parse(path_netlist, lib);
                netlist_serializer::serialize_to_file(nl.get(), path_netlist_hal);
            }
            if (nl == nullptr)
            {
                FAIL() << "netlist couldn't be parsed";
            }
        }

        // hdl_writer_manager::write(nl.get(), "sha256_flat.vhd");

        //path to vcd
        std::string path_vcd = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/sha256/dump.vcd";
        if (!utils::file_exists(path_vcd))
            FAIL() << "dump for sha256 not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, true);

        //prepare simulation
        sim->add_gates(nl->get_gates());
        sim->load_initial_values_from_netlist();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "clk"; }).begin());

        sim->add_clock_period(clk, 10000);

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

        {
            measure_block_time("simulation");

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
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->set_input(rst, SignalValue::ZERO);    //RST <= '0';
            sim->simulate(10 * 1000);                  //WAIT FOR 10 NS;

            sim->set_input(start, SignalValue::ONE);    //START <= '1';
            sim->simulate(10 * 1000);                   //WAIT FOR 10 NS;

            sim->set_input(start, SignalValue::ZERO);    //START <= '0';
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->simulate(2000 * 1000);
        }

        // Test if maps are equal

        EXPECT_TRUE(cmp_sim_data(vcd_traces, sim->get_simulation_state()));
        TEST_END
    }
}    // namespace hal
