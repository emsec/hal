#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
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
            gate_library_manager::get_gate_library("XILINX_UNISIM.hgl");
            gate_library_manager::get_gate_library("ice40ultra.hgl");
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

            std::set<const Net*> a_nets;
            std::set<const Net*> b_nets;
            for (auto it : a_events)
            {
                a_nets.insert(it.first);
            }
            for (auto it : b_events)
            {
                b_nets.insert(it.first);
            }

            std::cout << "finding mismatches..." << std::endl;

            std::map<std::string, const Net*> sorted_unmatching_events;

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
            std::vector<const Net*> earliest_mismatch_nets;
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
                    std::vector<const Net*> mismatch;
                    std::set_difference(a_nets.begin(), a_nets.end(), b_nets.begin(), b_nets.end(), std::back_inserter(mismatch));
                    for (auto x : mismatch)
                    {
                        std::cout << "  " << x->get_name() << std::endl;
                    }
                }
                else
                {
                    std::cout << "more nets are captured in the simulation output:" << std::endl;
                    std::vector<const Net*> mismatch;
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
            std::map<const Net*, BooleanFunction::Value> current_state;

            //map for storing gate names and corresponding u32 id for all nets of the netlist
            std::map<std::string, const Net*> net_name_to_net;
            //Fill map
            for (auto net : netlist->get_nets())
            {
                net_name_to_net[net->get_name()] = net;
                current_state[net]               = BooleanFunction::Value::Z;
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
                const Net* current_net = nullptr;
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
                    if (current_state[current_net] != BooleanFunction::Value::ZERO)
                    {
                        Event e;
                        e.affected_net = current_net;
                        e.time         = time;
                        e.new_value    = BooleanFunction::Value::ZERO;
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
                    if (current_state[current_net] != BooleanFunction::Value::ONE)
                    {
                        Event e;
                        e.affected_net = current_net;
                        e.time         = time;
                        e.new_value    = BooleanFunction::Value::ONE;
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
                    if (current_state[current_net] != BooleanFunction::Value::X)
                    {
                        Event e;
                        e.affected_net = current_net;
                        e.time         = time;
                        e.new_value    = BooleanFunction::Value::X;
                        vcd_trace.add_event(e);
                        current_state[current_net] = e.new_value;
                    }
                }
            }

            return vcd_trace;
        }

        void set_16_bit_net_vector(NetlistSimulator* sim, std::vector<Net*> nets, uint16_t number)
        {
            for (int i = 0; i < 16; i++)
            {
                sim->set_input(nets.at(i), (0x1 & number << i) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
            }
        }
    };    // namespace hal

    TEST_F(SimulatorTest, half_adder)
    {
        // return;
        TEST_START
        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("netlist_simulator");
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
            nl = netlist_parser_manager::parse(path_netlist, lib);
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
        sim->initialize_sequential_gates();
        Net* A = *(nl->get_nets([](auto net) { return net->get_name() == "A"; }).begin());
        Net* B = *(nl->get_nets([](auto net) { return net->get_name() == "B"; }).begin());

        //start simulation
        {
            measure_block_time("simulation");
            //Testbench
            sim->set_input(A, BooleanFunction::Value::ZERO);    //A=0
            sim->set_input(B, BooleanFunction::Value::ZERO);    //B=0
            sim->simulate(10 * 1000);

            sim->set_input(A, BooleanFunction::Value::ZERO);    //A=0
            sim->set_input(B, BooleanFunction::Value::ONE);     //B=1
            sim->simulate(10 * 1000);

            sim->set_input(A, BooleanFunction::Value::ONE);     //A=1
            sim->set_input(B, BooleanFunction::Value::ZERO);    //B=0
            sim->simulate(10 * 1000);

            sim->set_input(A, BooleanFunction::Value::ONE);    //A=1
            sim->set_input(B, BooleanFunction::Value::ONE);    //B=1
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

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("netlist_simulator");
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
            nl = netlist_parser_manager::parse(path_netlist, lib);
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
        sim->initialize_sequential_gates();

        // retrieve nets
        Net* reset          = *(nl->get_nets([](const Net* net) { return net->get_name() == "Reset"; }).begin());
        Net* clock          = *(nl->get_nets([](const Net* net) { return net->get_name() == "Clock"; }).begin());
        Net* Clock_enable_B = *(nl->get_nets([](const Net* net) { return net->get_name() == "Clock_enable_B"; }).begin());
        Net* output_0       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_0"; }).begin());
        Net* output_1       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_1"; }).begin());
        Net* output_2       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_2"; }).begin());
        Net* output_3       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_3"; }).begin());

        sim->add_clock_period(clock, 10000);

        //start simulation
        {
            measure_block_time("simulation");
            //testbench
            sim->set_input(Clock_enable_B, BooleanFunction::Value::ONE);    //#Clock_enable_B <= '1';
            sim->set_input(reset, BooleanFunction::Value::ZERO);            //#Reset <= '0';
            sim->simulate(40 * 1000);                            //#WAIT FOR 40 NS; -> simulate 4 clock cycle  - cycle 0, 1, 2, 3

            sim->set_input(Clock_enable_B, BooleanFunction::Value::ZERO);    //#Clock_enable_B <= '0';
            sim->simulate(110 * 1000);                            //#WAIT FOR 110 NS; -> simulate 11 clock cycle  - cycle 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14

            sim->set_input(reset, BooleanFunction::Value::ONE);    //#Reset <= '1';
            sim->simulate(20 * 1000);                   //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 15, 16

            sim->set_input(reset, BooleanFunction::Value::ZERO);    //#Reset <= '0';
            sim->simulate(70 * 1000);                    //#WAIT FOR 70 NS; -> simulate 7 clock cycle  - cycle 17, 18, 19, 20, 21, 22, 23

            sim->set_input(Clock_enable_B, BooleanFunction::Value::ONE);    //#Clock_enable_B <= '1';
            sim->simulate(23 * 1000);                            //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 24, 25

            sim->set_input(reset, BooleanFunction::Value::ONE);    //#Reset <= '1';
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

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("netlist_simulator");
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
            nl = netlist_parser_manager::parse(path_netlist, lib);
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
        sim->initialize_sequential_gates();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "CLK"; }).begin());

        sim->add_clock_period(clk, 10000);

        std::set<const Net*> key_set, plaintext_set;
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
                sim->set_input(net, BooleanFunction::Value::ZERO);

            for (auto net : key_set)    //KEY <= (OTHERS => '0');
                sim->set_input(net, BooleanFunction::Value::ZERO);

            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';
            sim->simulate(10 * 1000);                   //WAIT FOR 10 NS;

            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim->simulate(100 * 1000);                   //WAIT FOR 100 NS;

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '1');
                sim->set_input(net, BooleanFunction::Value::ONE);

            for (auto net : key_set)    //KEY <= (OTHERS => '1');
                sim->set_input(net, BooleanFunction::Value::ONE);

            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';
            sim->simulate(10 * 1000);                   //WAIT FOR 10 NS;

            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim->simulate(100 * 1000);                   //WAIT FOR 100 NS;

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '0');
                sim->set_input(net, BooleanFunction::Value::ZERO);

            for (auto net : key_set)    //KEY <= (OTHERS => '0');
                sim->set_input(net, BooleanFunction::Value::ZERO);

            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';

            sim->simulate(10 * 1000);
            sim->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';

            sim->simulate(10 * 1000);
            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';

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

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("netlist_simulator");
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
                NO_COUT_BLOCK;
                nl = netlist_serializer::deserialize_from_file(path_netlist_hal);
            }
            else
            {
                NO_COUT_BLOCK;
                nl = netlist_parser_manager::parse(path_netlist, lib);
                netlist_serializer::serialize_to_file(nl.get(), path_netlist_hal);
            }
            if (nl == nullptr)
            {
                FAIL() << "netlist couldn't be parsed";
            }
        }

        //path to vcd
        std::string path_vcd = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/sha256/dump.vcd";
        if (!utils::file_exists(path_vcd))
            FAIL() << "dump for sha256 not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, true);

        //prepare simulation
        sim->add_gates(nl->get_gates());
        sim->initialize_sequential_gates();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "clk"; }).begin());

        sim->add_clock_period(clk, 10000);

        auto start = *(nl->get_nets([](auto net) { return net->get_name() == "data_ready"; }).begin());

        auto rst = *(nl->get_nets([](auto net) { return net->get_name() == "rst"; }).begin());

        std::vector<const Net*> input_bits;
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
                    sim->set_input(input_bits[i * 4 + j], (BooleanFunction::Value)((byte >> (7 - j)) & 1));
                }
            }

            sim->set_input(rst, BooleanFunction::Value::ONE);       //RST <= '1';
            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->set_input(rst, BooleanFunction::Value::ZERO);    //RST <= '0';
            sim->simulate(10 * 1000);                  //WAIT FOR 10 NS;

            sim->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';
            sim->simulate(10 * 1000);                   //WAIT FOR 10 NS;

            sim->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim->simulate(10 * 1000);                    //WAIT FOR 10 NS;

            sim->simulate(2000 * 1000);
        }

        // Test if maps are equal

        EXPECT_TRUE(cmp_sim_data(vcd_traces, sim->get_simulation_state()));
        TEST_END
    }

    TEST_F(SimulatorTest, bram_lattice)
    {
        // return;
        TEST_START

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("netlist_simulator");
        auto sim    = plugin->create_simulator();

        //path to netlist
        std::string path_netlist = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/bram/bram_netlist.v";
        if (!utils::file_exists(path_netlist))
            FAIL() << "netlist for bram not found: " << path_netlist;

        std::string path_netlist_hal = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/bram/bram_netlist.hal";

        auto lib = gate_library_manager::get_gate_library_by_name("ICE40ULTRA");
        if (lib == nullptr)
        {
            FAIL() << "ice40ultra gate library not found";
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
                nl = netlist_parser_manager::parse(path_netlist, lib);
                netlist_serializer::serialize_to_file(nl.get(), path_netlist_hal);
            }
            if (nl == nullptr)
            {
                FAIL() << "netlist couldn't be parsed";
            }
        }

        //path to vcd
        std::string path_vcd = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/bram/trace.vcd";
        if (!utils::file_exists(path_vcd))
            FAIL() << "dump for bram not found: " << path_vcd;

        //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
        Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, true);

        std::cout << "read simulation file" << std::endl;

        //prepare simulation
        sim->add_gates(nl->get_gates());
        sim->initialize_sequential_gates();

        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "clk"; }).begin());

        sim->add_clock_period(clk, 10000);

        std::vector<Net*> din;
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_0"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_1"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_2"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_3"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_4"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_5"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_6"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_7"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_8"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_9"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_10"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_11"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_12"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_13"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_14"; }).begin()));
        din.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "din_15"; }).begin()));

        std::vector<Net*> read_addr;
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_0"; }).begin()));
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_1"; }).begin()));
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_2"; }).begin()));
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_3"; }).begin()));
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_4"; }).begin()));
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_5"; }).begin()));
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_6"; }).begin()));
        read_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "raddr_7"; }).begin()));

        std::vector<Net*> write_addr;
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_0"; }).begin()));
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_1"; }).begin()));
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_2"; }).begin()));
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_3"; }).begin()));
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_4"; }).begin()));
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_5"; }).begin()));
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_6"; }).begin()));
        write_addr.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "waddr_7"; }).begin()));

        auto write_en = *(nl->get_nets([](auto net) { return net->get_name() == "write_en"; }).begin());
        auto read_en  = *(nl->get_nets([](auto net) { return net->get_name() == "read_en"; }).begin());
        auto rclke    = *(nl->get_nets([](auto net) { return net->get_name() == "rclke"; }).begin());
        auto wclke    = *(nl->get_nets([](auto net) { return net->get_name() == "wclke"; }).begin());

        u32 input_nets_amount = 0;

        // clk has to be counted twice?
        if (clk != nullptr)
            input_nets_amount++;

        if (clk != nullptr)
            input_nets_amount++;

        for (const auto& din_net : din)
        {
            if (din_net != nullptr)
                input_nets_amount++;
        }

        for (const auto& write_addr_net : write_addr)
        {
            if (write_addr_net != nullptr)
                input_nets_amount++;
        }

        for (const auto& read_addr_net : read_addr)
        {
            if (read_addr_net != nullptr)
                input_nets_amount++;
        }

        if (write_en != nullptr)
            input_nets_amount++;

        if (read_en != nullptr)
            input_nets_amount++;

        if (rclke != nullptr)
            input_nets_amount++;

        if (wclke != nullptr)
            input_nets_amount++;

        if (input_nets_amount != sim->get_input_nets().size())
        {
            for (const auto& net : sim->get_input_nets())
            {
                std::cout << net->get_name() << std::endl;
            }
            FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim->get_input_nets().size();
        }

        //start simulation
        std::cout << "starting simulation" << std::endl;
        //testbench

        {
            measure_block_time("simulation");
            for (const auto& input_net : sim->get_input_nets())
            {
                sim->set_input(input_net, BooleanFunction::Value::ZERO);

                // TODO @speith, warum ist clk zweimal drin?
                if (input_net->get_name() == "clk")
                {
                    std::cout << "clk" << std::endl;
                }
            }

            uint16_t data_write = 0xffff;
            uint16_t data_read  = 0x0000;
            uint8_t addr        = 0xff;

            sim->simulate(1 * 1000);    // tb->wait_for_n_clocks(1);
            sim->simulate(5 * 1000);    // tb->wait_for_n_clocks(5);

            // write data without wclke
            // set_write_addr(addr);
            for (const auto& write_addr_net : write_addr)
            {
                sim->set_input(write_addr_net, BooleanFunction::Value::ONE);
            }
            // write_data(data_write);
            for (const auto& din_net : din)
            {
                sim->set_input(din_net, BooleanFunction::Value::ONE);
            }

            //TODO @speith: check units? *1000 correct?
            sim->simulate(1 * 1000);    // tb->wait_for_n_clocks(1);

            sim->set_input(write_en, BooleanFunction::Value::ONE);    // set_write_en(0x1);

            sim->simulate(1 * 1000);    // tb->wait_for_n_clocks(1);

            sim->set_input(write_en, BooleanFunction::Value::ZERO);    // set_write_en(0x0);
            sim->simulate(1 * 1000);                        // tb->wait_for_n_clocks(1);

            // // read data without rclke
            sim->set_input(read_en, BooleanFunction::Value::ZERO);    // set_read_en(0x1);
            // set_read_addr(addr);
            for (const auto& read_addr_net : read_addr)
            {
                sim->set_input(read_addr_net, BooleanFunction::Value::ONE);
            }

            sim->simulate(2 * 1000);    // tb->wait_for_n_clocks(2);

            // data_read = read_data();

            sim->simulate(5 * 1000);    // tb->wait_for_n_clocks(5);
            // printf("sent %08x, received: %08x\n", data_write, data_read);

            // // write data with wclke
            // set_write_addr(addr);
            for (const auto& write_addr_net : write_addr)
            {
                sim->set_input(write_addr_net, BooleanFunction::Value::ONE);
            }
            // write_data(data_write)
            for (const auto& din_net : din)
            {
                sim->set_input(din_net, BooleanFunction::Value::ONE);
            }

            sim->simulate(1 * 1000);    // tb->wait_for_n_clocks(1);

            sim->set_input(write_en, BooleanFunction::Value::ONE);    // set_write_en(0x1);
            sim->set_input(wclke, BooleanFunction::Value::ONE);       // set_wclke(0x1);

            sim->simulate(1 * 1000);    // tb->wait_for_n_clocks(1);

            sim->set_input(write_en, BooleanFunction::Value::ZERO);    // set_write_en(0x0);
            sim->set_input(wclke, BooleanFunction::Value::ZERO);       // set_wclke(0x0);

            sim->simulate(1 * 1000);    // tb->wait_for_n_clocks(1);

            // // read data without rclke
            sim->set_input(read_en, BooleanFunction::Value::ZERO);    // set_read_en(0x1);
            // set_read_addr(addr);
            for (const auto& read_addr_net : read_addr)
            {
                sim->set_input(read_addr_net, BooleanFunction::Value::ONE);
            }

            sim->simulate(2 * 1000);    // tb->wait_for_n_clocks(2);

            // data_read = read_data();

            sim->simulate(5 * 1000);    // tb->wait_for_n_clocks(5);
            // printf("sent %08x, received: %08x\n", data_write, data_read);

            // // read data with rclke
            sim->set_input(read_en, BooleanFunction::Value::ONE);    // set_read_en(0x1);
            // set_read_addr(addr);
            for (const auto& read_addr_net : read_addr)
            {
                sim->set_input(read_addr_net, BooleanFunction::Value::ONE);
            }
            sim->set_input(rclke, BooleanFunction::Value::ONE);    // set_rclke(0x1);

            sim->simulate(2 * 1000);                     // tb->wait_for_n_clocks(2);
            sim->set_input(rclke, BooleanFunction::Value::ZERO);    // set_rclke(0x0);

            // data_read = read_data();

            sim->simulate(5 * 1000);    // tb->wait_for_n_clocks(5);
            // printf("sent %08x, received: %08x\n", data_write, data_read);

            // read some address and see what the result from INIT value is
            sim->set_input(read_en, BooleanFunction::Value::ONE);    // set_read_en(0x1);

            // todo: bitorder could be wrong?
            //set_read_addr(0x66); // 1010 1010
            sim->set_input(read_addr.at(0), BooleanFunction::Value::ZERO);
            sim->set_input(read_addr.at(1), BooleanFunction::Value::ONE);
            sim->set_input(read_addr.at(2), BooleanFunction::Value::ZERO);
            sim->set_input(read_addr.at(3), BooleanFunction::Value::ONE);
            sim->set_input(read_addr.at(4), BooleanFunction::Value::ZERO);
            sim->set_input(read_addr.at(5), BooleanFunction::Value::ONE);
            sim->set_input(read_addr.at(6), BooleanFunction::Value::ZERO);
            sim->set_input(read_addr.at(7), BooleanFunction::Value::ONE);

            sim->set_input(rclke, BooleanFunction::Value::ONE);    //set_rclke(0x1);

            sim->simulate(2 * 1000);                     // tb->wait_for_n_clocks(2);
            sim->set_input(rclke, BooleanFunction::Value::ZERO);    // set_rclke(0x0);

            //data_read = read_data();

            sim->simulate(100 * 1000);    // tb->wait_for_n_clocks(100);
        }

        // Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(vcd_traces, sim->get_simulation_state()));
        TEST_END
    }

    // TEST_F(SimulatorTest, dsp_lattice)
    // {
    //     // return;
    //     TEST_START

    //     auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorPlugin>("netlist_simulator");
    //     auto sim    = plugin->create_simulator();

    //     //path to netlist
    //     std::string path_netlist = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/dsp/dsp_netlist.v";
    //     if (!utils::file_exists(path_netlist))
    //         FAIL() << "netlist for dsp not found: " << path_netlist;

    //     std::string path_netlist_hal = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/dsp/dsp_netlist.hal";

    //     auto lib = gate_library_manager::get_gate_library_by_name("ICE40ULTRA");
    //     if (lib == nullptr)
    //     {
    //         FAIL() << "ice40ultra gate library not found";
    //     }

    //     std::unique_ptr<Netlist> nl;
    //     {
    //         std::cout << "loading netlist: " << path_netlist << "..." << std::endl;
    //         if (utils::file_exists(path_netlist_hal))
    //         {
    //             std::cout << ".hal file found for test netlist, loading this one." << std::endl;
    //             nl = netlist_serializer::deserialize_from_file(path_netlist_hal);
    //         }
    //         else
    //         {
    //             NO_COUT_BLOCK;
    //             nl = netlist_parser_manager::parse(path_netlist, lib);
    //             netlist_serializer::serialize_to_file(nl.get(), path_netlist_hal);
    //         }
    //         if (nl == nullptr)
    //         {
    //             FAIL() << "netlist couldn't be parsed";
    //         }
    //     }

    //     //path to vcd
    //     std::string path_vcd = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/dsp/trace.vcd";
    //     if (!utils::file_exists(path_vcd))
    //         FAIL() << "dump for bram not found: " << path_vcd;

    //     //read vcd and transform to vector of states, clock = 10000 ps = 10 ns
    //     Simulation vcd_traces = parse_vcd(nl.get(), path_vcd, true);

    //     std::cout << "read simulation file" << std::endl;

    //     //prepare simulation
    //     sim->add_gates(nl->get_gates());
    //     sim->initialize_sequential_gates();

    //     auto CLK = *(nl->get_nets([](auto net) { return net->get_name() == "CLK"; }).begin());

    //     sim->add_clock_period(CLK, 10000);

    //     std::vector<Net*> A;
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_0"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_1"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_2"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_3"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_4"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_5"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_6"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_7"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_8"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_9"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_10"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_11"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_12"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_13"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_14"; }).begin()));
    //     A.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "A_15"; }).begin()));

    //     std::vector<Net*> B;
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_0"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_1"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_2"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_3"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_4"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_5"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_6"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_7"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_8"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_9"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_10"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_11"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_12"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_13"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_14"; }).begin()));
    //     B.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "B_15"; }).begin()));

    //     std::vector<Net*> C;
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_0"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_1"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_2"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_3"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_4"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_5"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_6"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_7"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_8"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_9"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_10"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_11"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_12"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_13"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_14"; }).begin()));
    //     C.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "C_15"; }).begin()));

    //     std::vector<Net*> D;
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_0"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_1"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_2"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_3"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_4"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_5"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_6"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_7"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_8"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_9"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_10"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_11"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_12"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_13"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_14"; }).begin()));
    //     D.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "D_15"; }).begin()));

    //     auto IRSTTOP   = *(nl->get_nets([](auto net) { return net->get_name() == "IRSTTOP"; }).begin());
    //     auto IRSTBOT   = *(nl->get_nets([](auto net) { return net->get_name() == "IRSTBOT"; }).begin());
    //     auto ORSTTOP   = *(nl->get_nets([](auto net) { return net->get_name() == "ORSTTOP"; }).begin());
    //     auto ORSTBOT   = *(nl->get_nets([](auto net) { return net->get_name() == "ORSTBOT"; }).begin());
    //     auto AHOLD     = *(nl->get_nets([](auto net) { return net->get_name() == "AHOLD"; }).begin());
    //     auto BHOLD     = *(nl->get_nets([](auto net) { return net->get_name() == "BHOLD"; }).begin());
    //     auto CHOLD     = *(nl->get_nets([](auto net) { return net->get_name() == "CHOLD"; }).begin());
    //     auto DHOLD     = *(nl->get_nets([](auto net) { return net->get_name() == "DHOLD"; }).begin());
    //     auto OHOLDTOP  = *(nl->get_nets([](auto net) { return net->get_name() == "OHOLDTOP"; }).begin());
    //     auto OHOLDBOT  = *(nl->get_nets([](auto net) { return net->get_name() == "OHOLDBOT"; }).begin());
    //     auto OLOADTOP  = *(nl->get_nets([](auto net) { return net->get_name() == "OLOADTOP"; }).begin());
    //     auto OLOADBOT  = *(nl->get_nets([](auto net) { return net->get_name() == "OLOADBOT"; }).begin());
    //     auto ADDSUBTOP = *(nl->get_nets([](auto net) { return net->get_name() == "ADDSUBTOP"; }).begin());
    //     auto ADDSUBBOT = *(nl->get_nets([](auto net) { return net->get_name() == "ADDSUBBOT"; }).begin());
    //     auto CI        = *(nl->get_nets([](auto net) { return net->get_name() == "CI"; }).begin());
    //     auto CE        = *(nl->get_nets([](auto net) { return net->get_name() == "CE"; }).begin());

    //     u32 input_nets_amount = 0;

    //     if (CLK != nullptr)
    //         input_nets_amount++;

    //     for (const auto& A_net : A)
    //     {
    //         if (A_net != nullptr)
    //             input_nets_amount++;
    //     }
    //     for (const auto& B_net : B)
    //     {
    //         if (B_net != nullptr)
    //             input_nets_amount++;
    //     }
    //     for (const auto& C_net : C)
    //     {
    //         if (C_net != nullptr)
    //             input_nets_amount++;
    //     }
    //     for (const auto& D_net : D)
    //     {
    //         if (D_net != nullptr)
    //             input_nets_amount++;
    //     }

    //     if (IRSTTOP != nullptr)
    //         input_nets_amount++;

    //     if (IRSTBOT != nullptr)
    //         input_nets_amount++;

    //     if (ORSTTOP != nullptr)
    //         input_nets_amount++;

    //     if (ORSTBOT != nullptr)
    //         input_nets_amount++;

    //     if (AHOLD != nullptr)
    //         input_nets_amount++;

    //     if (BHOLD != nullptr)
    //         input_nets_amount++;

    //     if (CHOLD != nullptr)
    //         input_nets_amount++;

    //     if (DHOLD != nullptr)
    //         input_nets_amount++;

    //     if (OHOLDTOP != nullptr)
    //         input_nets_amount++;

    //     if (OHOLDBOT != nullptr)
    //         input_nets_amount++;

    //     if (OLOADTOP != nullptr)
    //         input_nets_amount++;

    //     if (OLOADBOT != nullptr)
    //         input_nets_amount++;

    //     if (ADDSUBTOP != nullptr)
    //         input_nets_amount++;

    //     if (ADDSUBBOT != nullptr)
    //         input_nets_amount++;

    //     if (CI != nullptr)
    //         input_nets_amount++;

    //     if (CE != nullptr)
    //         input_nets_amount++;

    //     if (input_nets_amount != sim->get_input_nets().size())
    //     {
    //         for (const auto& net : sim->get_input_nets())
    //         {
    //             std::cout << net->get_name() << std::endl;
    //         }
    //         FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim->get_input_nets().size();
    //     }

    //     //start simulation
    //     std::cout << "starting simulation" << std::endl;
    //     //testbench

    //     {
    //         measure_block_time("simulation");
    //         for (const auto& input_net : sim->get_input_nets())
    //         {
    //             sim->set_input(input_net, BooleanFunction::Value::ZERO);
    //         }

    //         set_16_bit_net_vector(sim.get(), A, 6666);     // set_a(6666);
    //         set_16_bit_net_vector(sim.get(), B, 1234);     // set_b(1234);
    //         set_16_bit_net_vector(sim.get(), C, 4371);     // set_c(4371);
    //         set_16_bit_net_vector(sim.get(), D, 43714);    // set_d(43714);

    //         // tb->m_core->AHOLD     = 0x0;
    //         // tb->m_core->BHOLD     = 0x0;
    //         // tb->m_core->CHOLD     = 0x0;
    //         // tb->m_core->DHOLD     = 0x0;
    //         // tb->m_core->OHOLDTOP  = 0x0;
    //         // tb->m_core->OHOLDBOT  = 0x0;
    //         // tb->m_core->OLOADTOP  = 0x0;
    //         // tb->m_core->OLOADBOT  = 0x0;
    //         // tb->m_core->ADDSUBTOP = 0x0;
    //         // tb->m_core->ADDSUBBOT = 0x0;
    //         // tb->m_core->CI        = 0x0;
    //         // tb->m_core->CE        = 0x0;
    //         // --> already done before :)

    //         // tb->opentrace("trace.vcd");

    //         sim->simulate(1 * 1000);    // tb->wait_for_n_clocks(1);

    //         sim->set_input(CE, BooleanFunction::Value::ZERO);    // tb->m_core->CE = 0x0;

    //         sim->set_input(IRSTTOP, BooleanFunction::Value::ONE);    // tb->m_core->IRSTTOP = 0x1;
    //         sim->set_input(IRSTBOT, BooleanFunction::Value::ONE);    // tb->m_core->IRSTBOT = 0x1;
    //         sim->set_input(ORSTTOP, BooleanFunction::Value::ONE);    // tb->m_core->ORSTTOP = 0x1;
    //         sim->set_input(ORSTBOT, BooleanFunction::Value::ONE);    // tb->m_core->ORSTBOT = 0x1;

    //         sim->simulate(10 * 1000);    // tb->wait_for_n_clocks(10);

    //         sim->set_input(CE, BooleanFunction::Value::ONE);    // tb->m_core->CE = 0x1;

    //         sim->simulate(10 * 1000);    // tb->wait_for_n_clocks(10);

    //         sim->set_input(IRSTTOP, BooleanFunction::Value::ZERO);    // tb->m_core->IRSTTOP = 0x0;
    //         sim->set_input(IRSTBOT, BooleanFunction::Value::ZERO);    // tb->m_core->IRSTBOT = 0x0;
    //         sim->set_input(ORSTTOP, BooleanFunction::Value::ZERO);    // tb->m_core->ORSTTOP = 0x0;
    //         sim->set_input(ORSTBOT, BooleanFunction::Value::ZERO);    // tb->m_core->ORSTBOT = 0x0;

    //         sim->simulate(10 * 1000);    // tb->wait_for_n_clocks(10);

    //         sim->set_input(AHOLD, BooleanFunction::Value::ONE);    // tb->m_core->AHOLD = 0x1;
    //         sim->simulate(5 * 1000);                    // tb->wait_for_n_clocks(5);

    //         sim->set_input(OHOLDTOP, BooleanFunction::Value::ONE);    // tb->m_core->OHOLDTOP = 0x1;
    //         sim->set_input(OHOLDBOT, BooleanFunction::Value::ONE);    // tb->m_core->OHOLDBOT = 0x1;
    //         sim->simulate(5 * 1000);                       // tb->wait_for_n_clocks(5);

    //         sim->set_input(AHOLD, BooleanFunction::Value::ZERO);       // tb->m_core->AHOLD    = 0x0;
    //         sim->set_input(OHOLDTOP, BooleanFunction::Value::ZERO);    // tb->m_core->OHOLDTOP = 0x0;
    //         sim->set_input(OHOLDBOT, BooleanFunction::Value::ZERO);    // tb->m_core->OHOLDBOT = 0x0;
    //         sim->simulate(5 * 1000);                        // tb->wait_for_n_clocks(5);

    //         sim->set_input(OLOADTOP, BooleanFunction::Value::ONE);     // tb->m_core->OLOADTOP  = 0x1;
    //         sim->set_input(ADDSUBTOP, BooleanFunction::Value::ONE);    // tb->m_core->ADDSUBTOP = 0x1;
    //         sim->simulate(5 * 1000);                        // tb->wait_for_n_clocks(5);

    //         sim->set_input(OLOADTOP, BooleanFunction::Value::ZERO);     // tb->m_core->OLOADTOP  = 0x0;
    //         sim->set_input(ADDSUBTOP, BooleanFunction::Value::ZERO);    // tb->m_core->ADDSUBTOP = 0x0;
    //         sim->set_input(ADDSUBBOT, BooleanFunction::Value::ONE);     // tb->m_core->ADDSUBBOT = 0x1;
    //         sim->simulate(5 * 1000);                         // tb->wait_for_n_clocks(5);

    //         sim->set_input(ORSTTOP, BooleanFunction::Value::ONE);    // tb->m_core->ORSTTOP = 0x1;
    //         sim->simulate(5 * 1000);                      // tb->wait_for_n_clocks(5);

    //         sim->set_input(ADDSUBBOT, BooleanFunction::Value::ZERO);    // tb->m_core->ADDSUBBOT = 0x0;
    //         sim->set_input(ORSTTOP, BooleanFunction::Value::ZERO);      // tb->m_core->ORSTTOP   = 0x0;
    //         sim->simulate(5 * 1000);                         // tb->wait_for_n_clocks(5);

    //         sim->set_input(IRSTTOP, BooleanFunction::Value::ONE);    // tb->m_core->IRSTTOP = 0x1;
    //         sim->set_input(CHOLD, BooleanFunction::Value::ONE);      // tb->m_core->CHOLD   = 0x1;
    //         sim->simulate(5 * 1000);                      // tb->wait_for_n_clocks(5);

    //         sim->set_input(IRSTTOP, BooleanFunction::Value::ZERO);    // tb->m_core->IRSTTOP = 0x0;
    //         sim->set_input(CHOLD, BooleanFunction::Value::ZERO);      // tb->m_core->CHOLD   = 0x0;

    //         set_16_bit_net_vector(sim.get(), A, 1632);      // set_a(1632);
    //         set_16_bit_net_vector(sim.get(), B, 764254);    // set_b(764254); // TODO @nils this is shit
    //         set_16_bit_net_vector(sim.get(), C, 12523);     // set_c(12523);
    //         set_16_bit_net_vector(sim.get(), D, 263);       // set_d(263);
    //         sim->simulate(5 * 1000);                        // tb->wait_for_n_clocks(5);

    //         sim->set_input(OLOADTOP, BooleanFunction::Value::ONE);    // tb->m_core->OLOADTOP = 0x1;
    //         sim->set_input(OLOADBOT, BooleanFunction::Value::ONE);    // tb->m_core->OLOADBOT = 0x1;
    //         sim->simulate(5 * 1000);                      // tb->wait_for_n_clocks(5);

    //         sim->set_input(OLOADTOP, BooleanFunction::Value::ZERO);    // tb->m_core->OLOADTOP = 0x0;
    //         sim->set_input(OLOADBOT, BooleanFunction::Value::ZERO);    // tb->m_core->OLOADBOT = 0x0;
    //         sim->simulate(5 * 1000);                      // tb->wait_for_n_clocks(5);

    //         sim->set_input(ADDSUBTOP, BooleanFunction::Value::ONE);    // tb->m_core->ADDSUBTOP = 0x1;
    //         sim->set_input(ADDSUBBOT, BooleanFunction::Value::ZERO);    // tb->m_core->ADDSUBBOT = 0x0;
    //         sim->simulate(5 * 1000);                      // tb->wait_for_n_clocks(5);

    //         sim->set_input(ADDSUBTOP, BooleanFunction::Value::ZERO);    // tb->m_core->ADDSUBTOP = 0x0;
    //         sim->set_input(ADDSUBBOT, BooleanFunction::Value::ONE);    // tb->m_core->ADDSUBBOT = 0x1;
    //         sim->simulate(5 * 1000);                      // tb->wait_for_n_clocks(5);

    //         sim->set_input(ADDSUBTOP, BooleanFunction::Value::ZERO);    // tb->m_core->ADDSUBTOP = 0x0;
    //         sim->set_input(ADDSUBBOT, BooleanFunction::Value::ZERO);    // tb->m_core->ADDSUBBOT = 0x0;
    //         sim->simulate(5 * 1000);                      // tb->wait_for_n_clocks(5);

    //         sim->simulate(5 * 1000);    // tb->wait_for_n_clocks(5);
    //     }

    //     // Test if maps are equal
    //     EXPECT_TRUE(cmp_sim_data(vcd_traces, sim->get_simulation_state()));
    //     TEST_END
    // }
}    // namespace hal
