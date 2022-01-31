#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_data.h"
#include "test_utils/include/test_def.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

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

        bool cmp_sim_data(NetlistSimulatorController* reference_simulation_ctrl, NetlistSimulatorController* simulation_ctrl, int tolerance = 200)
        {
            bool no_errors                     = true;
            WaveDataList* reference_simulation = reference_simulation_ctrl->get_waves();
            WaveDataList* engine_simulation    = simulation_ctrl->get_waves();
            std::cout << "comparing outputs..." << std::endl;
            std::cout << "reference has " << reference_simulation->size() << " and engine simulation " << engine_simulation->size() << " nets" << std::endl;

            // TODO @ Jörn: remove GND and VCC from simulation_ctrl
            // for (auto it = b_events.begin(); it != b_events.end();)
            // {
            //     auto srcs = it->first->get_sources();
            //     if (srcs.size() == 1 && (srcs[0]->get_gate()->is_gnd_gate() || srcs[0]->get_gate()->is_vcc_gate()) && a_events.find(it->first) == a_events.end())
            //     {
            //         it = b_events.erase(it);
            //     }
            //     else
            //     {
            //         ++it;
            //     }
            // }

            auto signal_to_string = [](auto v) -> std::string {
                if (v >= 0)
                    return std::to_string(v);
                return "X";
            };

            // get all reference simulation net ids
            std::set<u32> reference_simulation_nets;
            for (auto it : *reference_simulation)
            {
                reference_simulation_nets.insert(it->id());
            }

            // get all  simulation net ids
            std::set<u32> engine_simulation_nets;
            for (auto it : *engine_simulation)
            {
                engine_simulation_nets.insert(it->id());
            }

            // identify missmatches
            std::cout << "searching for mismatches..." << std::endl;

            std::set<u32> unmatching_nets;

            for (auto it_ref : *reference_simulation)
            {
                int iwave_sim = engine_simulation->waveIndexByNetId(it_ref->id());
                if (iwave_sim < 0)
                {
                    no_errors = false;
                    std::cout << "error: net: " << it_ref->name().toStdString() << " (" << it_ref->id() << ") in reference, but not in simulated output" << std::endl;
                }
                else
                {
                    if (!it_ref->isEqual(*engine_simulation->at(iwave_sim), tolerance))
                    {
                        no_errors = false;
                        unmatching_nets.insert(it_ref->id());
                    }
                }
            }

            if (unmatching_nets.size() != 0)
            {
                no_errors = false;
                std::cout << "error: found " << unmatching_nets.size() << " unmatching nets..." << std::endl;
            }

            std::cout << "printing mismatches (if any)..." << std::endl;

            u64 earliest_mismatch = -1;
            std::vector<u32> earliest_mismatch_nets;
            auto update_mismatch = [&](u64 time, u32 net) {
                if (time < earliest_mismatch)
                {
                    earliest_mismatch      = time;
                    earliest_mismatch_nets = {net};
                }
                else if (time == earliest_mismatch)
                {
                    earliest_mismatch_nets.push_back(net);
                }
            };

            for (auto net_id : unmatching_nets)
            {
                std::vector<std::pair<u64, int>> events_a;
                WaveData* wave_data_a;
                for (auto it_sim : *reference_simulation)
                {
                    if (it_sim->id() == net_id)
                    {
                        wave_data_a = it_sim;
                        events_a    = it_sim->get_events();
                    }
                }

                std::vector<std::pair<u64, int>> events_b;
                WaveData* wave_data_b;
                for (auto it_sim : *engine_simulation)
                {
                    if (it_sim->id() == net_id)
                    {
                        wave_data_b = it_sim;
                        events_b    = it_sim->get_events();
                    }
                }

                u32 max_number_length = 0;
                if (!events_a.empty() && !events_b.empty())
                {
                    max_number_length = std::to_string(std::max(events_a.back().first, events_b.back().first)).size();
                }

                std::cout << "difference in net " << wave_data_a->name().toStdString() << " id=" << net_id << ":" << std::endl;
                std::cout << "reference:" << std::setfill(' ') << std::setw(max_number_length + 5) << ""
                          << "engine:" << std::endl;

                for (u32 i = 0, j = 0; i < events_a.size() || j < events_b.size();)
                {
                    if (i < events_a.size() && j < events_b.size())
                    {
                        if (abs((int)(events_a[i].first - events_b[j].first)) < tolerance)
                        {
                            if (events_a[i].second == events_b[j].second)
                            {
                                std::cout << signal_to_string(events_a[i].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].first << "ns";
                                std::cout << " | ";
                                std::cout << signal_to_string(events_b[j].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].first << "ns";
                                std::cout << std::endl;
                                i++;
                                j++;
                            }
                            else
                            {
                                update_mismatch(events_a[i].first, net_id);
                                std::cout << signal_to_string(events_a[i].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].first << "ns";
                                std::cout << " | ";
                                std::cout << signal_to_string(events_b[j].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].first << "ns";
                                std::cout << "  <--" << std::endl;
                                i++;
                                j++;
                            }
                        }
                        else
                        {
                            if (events_a[i].first < events_b[j].first)
                            {
                                update_mismatch(events_a[i].first, net_id);
                                std::cout << signal_to_string(events_a[i].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].first << "ns";
                                std::cout << " | ";
                                std::cout << std::endl;
                                i++;
                            }
                            else
                            {
                                update_mismatch(events_b[j].first, net_id);
                                std::cout << "    " << std::setfill(' ') << std::setw(max_number_length) << ""
                                          << "  ";
                                std::cout << " | ";
                                std::cout << signal_to_string(events_b[j].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].first << "ns";
                                std::cout << std::endl;
                                j++;
                            }
                        }
                    }
                    else if (i < events_a.size())
                    {
                        update_mismatch(events_a[i].first, net_id);
                        std::cout << signal_to_string(events_a[i].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_a[i].first << "ns";
                        std::cout << " | ";
                        std::cout << std::endl;
                        i++;
                    }
                    else
                    {
                        update_mismatch(events_b[j].first, net_id);
                        std::cout << "    " << std::setfill(' ') << std::setw(max_number_length) << ""
                                  << "  ";
                        std::cout << " | ";
                        std::cout << signal_to_string(events_b[j].second) << " @ " << std::setfill(' ') << std::setw(max_number_length) << events_b[j].first << "ns";
                        std::cout << std::endl;
                        j++;
                    }
                }
                std::cout << std::endl;
            }

            if (reference_simulation->size() != engine_simulation->size())
            {
                std::cout << "WARNING SIZE MISMATCH" << std::endl;
                if (reference_simulation->size() > engine_simulation->size())
                {
                    no_errors = false;
                    std::cout << "more nets are captured in the reference vcd file:" << std::endl;
                    std::vector<u32> mismatch;
                    std::set_difference(reference_simulation_nets.begin(), reference_simulation_nets.end(), engine_simulation_nets.begin(), engine_simulation_nets.end(), std::back_inserter(mismatch));
                    for (auto x : mismatch)
                    {
                        int iwave = reference_simulation->waveIndexByNetId(x);
                        std::cout << "  " << x << " " << (iwave < 0 ? "" : reference_simulation->at(iwave)->name().toUtf8().data()) << std::endl;
                    }
                }
                else
                {
                    std::cout << "more nets are captured in the engine_simulation output:" << std::endl;
                    std::vector<u32> mismatch;
                    std::set_difference(engine_simulation_nets.begin(), engine_simulation_nets.end(), reference_simulation_nets.begin(), reference_simulation_nets.end(), std::back_inserter(mismatch));
                    const char* artifical_added[] = {"'0'", "'1'", nullptr};
                    for (auto x : mismatch)
                    {
                        int iwave = engine_simulation->waveIndexByNetId(x);
                        std::string waveName(iwave < 0 ? "" : engine_simulation->at(iwave)->name().toUtf8().data());
                        if (!waveName.empty())
                        {
                            bool take_it_easy = false;
                            for (int i = 0; artifical_added[i]; i++)
                            {
                                if (waveName == artifical_added[i])
                                {
                                    take_it_easy = true;
                                    break;
                                }
                            }
                            if (!take_it_easy)
                                no_errors = false;
                        }
                        std::cout << "  " << x << " " << (iwave < 0 ? "" : engine_simulation->at(iwave)->name().toUtf8().data()) << std::endl;
                    }
                }

                if (unmatching_nets.empty())
                {
                    std::cout << "everything that could be compared was correct, though!" << std::endl;
                }
            }

            if (no_errors)
            {
                std::cout << "simulation correct!" << std::endl;
            }
            else
            {
                std::cout << "simulation incorrect, have fun debugging!" << std::endl;
            }

            return no_errors;
        }

    };    // namespace hal
    TEST_F(SimulatorTest, half_adder)
    {
        // return;
        TEST_START
        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorControllerPlugin>("netlist_simulator_controller");

        auto sim_ctrl_verilator = plugin->create_simulator_controller("half_adder_simulator");
        auto verilator_engine   = sim_ctrl_verilator->create_simulation_engine("verilator");
        //verilator_engine->set_engine_property("ssh_server", "mpi");

        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::NoGatesSelected);
        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Preparing);

        auto sim_ctrl_reference = plugin->create_simulator_controller("half_adder_reference");

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

        //prepare simulation
        sim_ctrl_verilator->add_gates(nl->get_gates());
        sim_ctrl_verilator->set_no_clock_used();
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::ParameterSetup);
        sim_ctrl_verilator->initialize();

        sim_ctrl_reference->add_gates(nl->get_gates());
        sim_ctrl_reference->set_no_clock_used();
        sim_ctrl_reference->initialize();

        //read vcd
        EXPECT_TRUE(sim_ctrl_reference->import_vcd(path_vcd, NetlistSimulatorController::FilterInputFlag::CompleteNetlist));

        // get nets
        Net* A = *(nl->get_nets([](auto net) { return net->get_name() == "A"; }).begin());
        Net* B = *(nl->get_nets([](auto net) { return net->get_name() == "B"; }).begin());

        //start simulation
        {
            measure_block_time("simulation");
            //Testbench
            sim_ctrl_verilator->set_input(A, BooleanFunction::Value::ZERO);    //A=0
            sim_ctrl_verilator->set_input(B, BooleanFunction::Value::ZERO);    //B=0
            sim_ctrl_verilator->simulate(10 * 1000);

            sim_ctrl_verilator->set_input(A, BooleanFunction::Value::ZERO);    //A=0
            sim_ctrl_verilator->set_input(B, BooleanFunction::Value::ONE);     //B=1
            sim_ctrl_verilator->simulate(10 * 1000);

            sim_ctrl_verilator->set_input(A, BooleanFunction::Value::ONE);     //A=1
            sim_ctrl_verilator->set_input(B, BooleanFunction::Value::ZERO);    //B=0
            sim_ctrl_verilator->simulate(10 * 1000);

            sim_ctrl_verilator->set_input(A, BooleanFunction::Value::ONE);    //A=1
            sim_ctrl_verilator->set_input(B, BooleanFunction::Value::ONE);    //B=1
            sim_ctrl_verilator->simulate(10 * 1000);
            sim_ctrl_verilator->run_simulation();

            EXPECT_FALSE(verilator_engine->get_state() == SimulationEngine::State::Failed);

            while (verilator_engine->get_state() == SimulationEngine::State::Running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        if (verilator_engine->get_state() == SimulationEngine::State::Failed)
        {
            FAIL() << "engine failed";
        }

        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Done);
        EXPECT_FALSE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::EngineFailed);

        sim_ctrl_verilator->get_results();

        for (Net* n : nl->get_nets())
        {
            sim_ctrl_verilator->get_waveform_by_net(n);
            sim_ctrl_reference->get_waveform_by_net(n);
        }

        // TODO @ Jörn: LOAD ALL WAVES TO MEMORY
        EXPECT_TRUE(sim_ctrl_verilator->get_waves()->size() == (int)nl->get_nets().size());
        EXPECT_TRUE(sim_ctrl_reference->get_waves()->size() == (int)nl->get_nets().size());

        //Test if maps are equal
        EXPECT_TRUE(cmp_sim_data(sim_ctrl_reference.get(), sim_ctrl_verilator.get()));
        TEST_END
    }

    TEST_F(SimulatorTest, counter)
    {
        // return;
        TEST_START

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorControllerPlugin>("netlist_simulator_controller");

        auto sim_ctrl_verilator = plugin->create_simulator_controller("counter_simulator");
        auto verilator_engine   = sim_ctrl_verilator->create_simulation_engine("verilator");
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::NoGatesSelected);
        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Preparing);

        auto sim_ctrl_reference = plugin->create_simulator_controller("counter_reference");

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

        //prepare simulation
        sim_ctrl_verilator->add_gates(nl->get_gates());
        sim_ctrl_verilator->set_no_clock_used();
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::ParameterSetup);
        sim_ctrl_verilator->initialize();

        sim_ctrl_reference->add_gates(nl->get_gates());

        Net* clock = *(nl->get_nets([](const Net* net) { return net->get_name() == "Clock"; }).begin());
        sim_ctrl_verilator->add_clock_period(clock, 10000);

        sim_ctrl_reference->initialize();

        //read vcd
        EXPECT_TRUE(sim_ctrl_reference->import_vcd(path_vcd, NetlistSimulatorController::FilterInputFlag::CompleteNetlist));

        // retrieve nets
        Net* reset          = *(nl->get_nets([](const Net* net) { return net->get_name() == "Reset"; }).begin());
        Net* Clock_enable_B = *(nl->get_nets([](const Net* net) { return net->get_name() == "Clock_enable_B"; }).begin());
        //        Net* output_0       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_0"; }).begin());
        //        Net* output_1       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_1"; }).begin());
        //        Net* output_2       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_2"; }).begin());
        //        Net* output_3       = *(nl->get_nets([](const Net* net) { return net->get_name() == "Output_3"; }).begin());

        //start simulation
        {
            measure_block_time("simulation");
            //testbench
            sim_ctrl_verilator->set_input(Clock_enable_B, BooleanFunction::Value::ONE);    //#Clock_enable_B <= '1';
            sim_ctrl_verilator->set_input(reset, BooleanFunction::Value::ZERO);            //#Reset <= '0';
            sim_ctrl_verilator->simulate(40 * 1000);                                       //#WAIT FOR 40 NS; -> simulate 4 clock cycle  - cycle 0, 1, 2, 3

            sim_ctrl_verilator->set_input(Clock_enable_B, BooleanFunction::Value::ZERO);    //#Clock_enable_B <= '0';
            sim_ctrl_verilator->simulate(110 * 1000);                                       //#WAIT FOR 110 NS; -> simulate 11 clock cycle  - cycle 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14

            sim_ctrl_verilator->set_input(reset, BooleanFunction::Value::ONE);    //#Reset <= '1';
            sim_ctrl_verilator->simulate(20 * 1000);                              //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 15, 16

            sim_ctrl_verilator->set_input(reset, BooleanFunction::Value::ZERO);    //#Reset <= '0';
            sim_ctrl_verilator->simulate(70 * 1000);                               //#WAIT FOR 70 NS; -> simulate 7 clock cycle  - cycle 17, 18, 19, 20, 21, 22, 23

            sim_ctrl_verilator->set_input(Clock_enable_B, BooleanFunction::Value::ONE);    //#Clock_enable_B <= '1';
            sim_ctrl_verilator->simulate(23 * 1000);                                       //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 24, 25

            sim_ctrl_verilator->set_input(reset, BooleanFunction::Value::ONE);    //#Reset <= '1';
            sim_ctrl_verilator->simulate(20 * 1000);                              //#WAIT FOR 20 NS; -> simulate 2 clock cycle  - cycle 26, 27
                //#3 additional traces á 10 NS to get 300 NS simulation time
            sim_ctrl_verilator->simulate(17 * 1000);    //# remaining 17 NS to simulate 300 NS in total

            sim_ctrl_verilator->initialize();
            sim_ctrl_verilator->run_simulation();

            EXPECT_FALSE(verilator_engine->get_state() == SimulationEngine::State::Failed);

            while (verilator_engine->get_state() == SimulationEngine::State::Running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        if (verilator_engine->get_state() == SimulationEngine::State::Failed)
        {
            FAIL() << "engine failed";
        }

        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Done);
        EXPECT_FALSE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::EngineFailed);

        sim_ctrl_verilator->get_results();

        for (Net* n : nl->get_nets())
        {
            sim_ctrl_verilator->get_waveform_by_net(n);
            sim_ctrl_reference->get_waveform_by_net(n);
        }

        // TODO @ Jörn: LOAD ALL WAVES TO MEMORY
        EXPECT_TRUE(sim_ctrl_verilator->get_waves()->size() == (int)nl->get_nets().size());
        EXPECT_TRUE(sim_ctrl_reference->get_waves()->size() == (int)nl->get_nets().size());

        //Test if maps are equal
        bool equal = cmp_sim_data(sim_ctrl_reference.get(), sim_ctrl_verilator.get());
        EXPECT_TRUE(equal);
        TEST_END
    }

    TEST_F(SimulatorTest, toycipher)
    {
        // return;
        TEST_START

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorControllerPlugin>("netlist_simulator_controller");

        auto sim_ctrl_verilator = plugin->create_simulator_controller("tocipher_simulator");
        auto verilator_engine   = sim_ctrl_verilator->create_simulation_engine("verilator");
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::NoGatesSelected);
        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Preparing);

        auto sim_ctrl_reference = plugin->create_simulator_controller("tocipher_reference");

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

        sim_ctrl_reference->add_gates(nl->get_gates());
        sim_ctrl_reference->initialize();
        EXPECT_TRUE(sim_ctrl_reference->import_vcd(path_vcd, NetlistSimulatorController::FilterInputFlag::CompleteNetlist));

        //prepare simulation
        sim_ctrl_verilator->add_gates(nl->get_gates());
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::ParameterSetup);
        sim_ctrl_verilator->initialize();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "CLK"; }).begin());
        sim_ctrl_verilator->add_clock_period(clk, 10000);

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

        if (input_nets_amount != (int)sim_ctrl_verilator->get_input_nets().size())
            FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim_ctrl_verilator->get_input_nets().size();

        // set GND and VCC
        Net* GND = *(nl->get_nets([](auto net) { return net->is_gnd_net(); }).begin());
        if (GND != nullptr)
        {
            sim_ctrl_verilator->set_input(GND, BooleanFunction::Value::ZERO);    // set GND to zero
        }

        Net* VCC = *(nl->get_nets([](auto net) { return net->is_vcc_net(); }).begin());
        if (VCC != nullptr)
        {
            sim_ctrl_verilator->set_input(VCC, BooleanFunction::Value::ONE);    // set VCC to zero
        }

        //start simulation
        {
            measure_block_time("simulation");
            //testbench

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '0');
                sim_ctrl_verilator->set_input(net, BooleanFunction::Value::ZERO);

            for (auto net : key_set)    //KEY <= (OTHERS => '0');
                sim_ctrl_verilator->set_input(net, BooleanFunction::Value::ZERO);

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim_ctrl_verilator->simulate(10 * 1000);                               //WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';
            sim_ctrl_verilator->simulate(10 * 1000);                              //WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim_ctrl_verilator->simulate(100 * 1000);                              //WAIT FOR 100 NS;

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '1');
                sim_ctrl_verilator->set_input(net, BooleanFunction::Value::ONE);

            for (auto net : key_set)    //KEY <= (OTHERS => '1');
                sim_ctrl_verilator->set_input(net, BooleanFunction::Value::ONE);

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim_ctrl_verilator->simulate(10 * 1000);                               //WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';
            sim_ctrl_verilator->simulate(10 * 1000);                              //WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim_ctrl_verilator->simulate(100 * 1000);                              //WAIT FOR 100 NS;

            for (auto net : plaintext_set)    //PLAINTEXT <= (OTHERS => '0');
                sim_ctrl_verilator->set_input(net, BooleanFunction::Value::ZERO);

            for (auto net : key_set)    //KEY <= (OTHERS => '0');
                sim_ctrl_verilator->set_input(net, BooleanFunction::Value::ZERO);

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';

            sim_ctrl_verilator->simulate(10 * 1000);
            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';

            sim_ctrl_verilator->simulate(10 * 1000);
            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';

            sim_ctrl_verilator->simulate(25 * 1000);

            sim_ctrl_verilator->initialize();
            sim_ctrl_verilator->run_simulation();

            EXPECT_FALSE(verilator_engine->get_state() == SimulationEngine::State::Failed);

            while (verilator_engine->get_state() == SimulationEngine::State::Running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        if (verilator_engine->get_state() == SimulationEngine::State::Failed)
        {
            FAIL() << "engine failed";
        }

        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Done);
        EXPECT_FALSE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::EngineFailed);

        sim_ctrl_verilator->get_results();

        for (Net* n : nl->get_nets())
        {
            sim_ctrl_verilator->get_waveform_by_net(n);
            sim_ctrl_reference->get_waveform_by_net(n);
        }

        // TODO @ Jörn: LOAD ALL WAVES TO MEMORY
        EXPECT_TRUE(sim_ctrl_verilator->get_waves()->size() == (int)nl->get_nets().size());
        EXPECT_TRUE(sim_ctrl_reference->get_waves()->size() <= (int)nl->get_nets().size());    // net might have additional '0' and '1'

        //Test if maps are equal
        bool equal = cmp_sim_data(sim_ctrl_reference.get(), sim_ctrl_verilator.get());
        EXPECT_TRUE(equal);
        TEST_END
    }

    TEST_F(SimulatorTest, sha256)
    {
        // return;
        TEST_START

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorControllerPlugin>("netlist_simulator_controller");

        auto sim_ctrl_verilator = plugin->create_simulator_controller("sha256_simulator");
        auto verilator_engine   = sim_ctrl_verilator->create_simulation_engine("verilator");
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::NoGatesSelected);
        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Preparing);

        auto sim_ctrl_reference = plugin->create_simulator_controller("sha256_reference");

        //verilator_engine->set_engine_property("ssh_server", "mpi");

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
        //read vcd
        sim_ctrl_reference->initialize();
        sim_ctrl_reference->add_gates(nl->get_gates());
        EXPECT_TRUE(sim_ctrl_reference->import_vcd(path_vcd, NetlistSimulatorController::FilterInputFlag::CompleteNetlist));

        //prepare simulation
        sim_ctrl_verilator->add_gates(nl->get_gates());
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::ParameterSetup);

        sim_ctrl_verilator->initialize();

        // retrieve nets
        auto clk = *(nl->get_nets([](auto net) { return net->get_name() == "clk"; }).begin());

        sim_ctrl_verilator->add_clock_period(clk, 10000);

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

        if (input_nets_amount != (int)sim_ctrl_verilator->get_input_nets().size())
            FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim_ctrl_verilator->get_input_nets().size();

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
                    sim_ctrl_verilator->set_input(input_bits[i * 4 + j], (BooleanFunction::Value)((byte >> (7 - j)) & 1));
                }
            }

            sim_ctrl_verilator->set_input(rst, BooleanFunction::Value::ONE);       //RST <= '1';
            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim_ctrl_verilator->simulate(10 * 1000);                               //WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(rst, BooleanFunction::Value::ZERO);    //RST <= '0';
            sim_ctrl_verilator->simulate(10 * 1000);                             //WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ONE);    //START <= '1';
            sim_ctrl_verilator->simulate(10 * 1000);                              //WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(start, BooleanFunction::Value::ZERO);    //START <= '0';
            sim_ctrl_verilator->simulate(10 * 1000);                               //WAIT FOR 10 NS;

            sim_ctrl_verilator->simulate(1995 * 1000);

            sim_ctrl_verilator->initialize();
            sim_ctrl_verilator->run_simulation();

            EXPECT_FALSE(verilator_engine->get_state() == SimulationEngine::State::Failed);

            while (verilator_engine->get_state() == SimulationEngine::State::Running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        if (verilator_engine->get_state() == SimulationEngine::State::Failed)
        {
            FAIL() << "engine failed";
        }

        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Done);
        EXPECT_FALSE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::EngineFailed);

        sim_ctrl_verilator->get_results();

        int netCount = 0;
        for (Net* n : nl->get_nets())
        {
            if (netCount % 1000 == 0)
            {
                if (netCount)
                {
                    std::cerr << "load in memory done for " << netCount << " nets" << std::endl;
                    bool equal = cmp_sim_data(sim_ctrl_reference.get(), sim_ctrl_verilator.get());
                    EXPECT_TRUE(equal);
                }
                sim_ctrl_reference->get_waves()->clearAll();
                sim_ctrl_verilator->get_waves()->clearAll();
            }
            //            std::cerr << ++netCount << " import net " << n->get_id() << " [" << n->get_name() << "]" << std::endl;
            sim_ctrl_verilator->get_waveform_by_net(n);
            sim_ctrl_reference->get_waveform_by_net(n);
            ++netCount;
        }
        std::cerr << "load in memory done for " << netCount << " nets" << std::endl;

        // TODO @ Jörn: LOAD ALL WAVES TO MEMORY
        EXPECT_TRUE(sim_ctrl_verilator->get_waves()->size() == (int)nl->get_nets().size() % 1000 );
        EXPECT_TRUE(sim_ctrl_reference->get_waves()->size() == (int)nl->get_nets().size() % 1000 );

        //Test if maps are equal
        if (!sim_ctrl_reference->get_waves()->isEmpty())
        {
            bool equal = cmp_sim_data(sim_ctrl_reference.get(), sim_ctrl_verilator.get());
            EXPECT_TRUE(equal);
        }
        TEST_END
    }

    TEST_F(SimulatorTest, bram_lattice)
    {
        // return;
        TEST_START
        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorControllerPlugin>("netlist_simulator_controller");

        auto sim_ctrl_verilator = plugin->create_simulator_controller("bram_lattice_simulator");
        auto verilator_engine   = sim_ctrl_verilator->create_simulation_engine("verilator");
        verilator_engine->set_engine_property("provided_models", utils::get_base_directory().string() + "/bin/hal_plugins/test-files/bram/provided_models");

        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::NoGatesSelected);
        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Preparing);

        auto sim_ctrl_reference = plugin->create_simulator_controller("bram_lattice_reference");

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

        sim_ctrl_reference->add_gates(nl->get_gates());
        sim_ctrl_reference->initialize();
        EXPECT_TRUE(sim_ctrl_reference->import_vcd(path_vcd, NetlistSimulatorController::FilterInputFlag::CompleteNetlist));

        std::cout << "read simulation file" << std::endl;

        //prepare simulation
        sim_ctrl_verilator->add_gates(nl->get_gates());
        EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::ParameterSetup);
        sim_ctrl_verilator->initialize();

        auto clk         = *(nl->get_nets([](auto net) { return net->get_name() == "clk"; }).begin());
        u32 clock_period = 10000;
        sim_ctrl_verilator->add_clock_period(clk, clock_period);

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

        std::vector<Net*> mask;
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_0"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_1"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_2"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_3"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_4"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_5"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_6"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_7"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_8"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_9"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_10"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_11"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_12"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_13"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_14"; }).begin()));
        mask.push_back(*(nl->get_nets([](auto net) { return net->get_name() == "mask_15"; }).begin()));

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

        if (clk != nullptr)
            input_nets_amount++;

        for (const auto& din_net : din)
        {
            if (din_net != nullptr)
                input_nets_amount++;
        }

        for (const auto& mask_net : mask)
        {
            if (mask_net != nullptr)
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

        if (input_nets_amount != sim_ctrl_verilator->get_input_nets().size())
        {
            for (const auto& net : sim_ctrl_verilator->get_input_nets())
            {
                std::cout << net->get_name() << std::endl;
            }
            FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim_ctrl_verilator->get_input_nets().size();
        }

        //start simulation
        std::cout << "starting simulation" << std::endl;
        //testbench

        {
            measure_block_time("simulation");
            for (const auto& input_net : sim_ctrl_verilator->get_input_nets())
            {
                sim_ctrl_verilator->set_input(input_net, BooleanFunction::Value::ZERO);
            }

            //            uint16_t data_write = 0xffff;
            //            uint16_t data_read  = 0x0000;
            //            uint8_t addr        = 0xff;

            sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

            // write data without wclke
            // waddr       <= x"ff";
            for (const auto& write_addr_net : write_addr)
            {
                sim_ctrl_verilator->set_input(write_addr_net, BooleanFunction::Value::ONE);
            }
            // din         <= x"ffff";
            for (const auto& din_net : din)
            {
                sim_ctrl_verilator->set_input(din_net, BooleanFunction::Value::ONE);
            }

            sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= '1';

            sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ZERO);    // write_en    <= '0';
            sim_ctrl_verilator->simulate(1 * clock_period);                           // WAIT FOR 10 NS;

            // read data without rclke
            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en     <= '1';

            // raddr       <= x"ff";
            for (const auto& read_addr_net : read_addr)
            {
                sim_ctrl_verilator->set_input(read_addr_net, BooleanFunction::Value::ONE);
            }

            sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

            sim_ctrl_verilator->simulate(5 * clock_period);    // WAIT FOR 50 NS;
            // printf("sent %08x, received: %08x\n", data_write, data_read);

            // // write data with wclke
            //  waddr   <= x"ff";
            for (const auto& write_addr_net : write_addr)
            {
                sim_ctrl_verilator->set_input(write_addr_net, BooleanFunction::Value::ONE);
            }
            // din     <= x"ffff";
            for (const auto& din_net : din)
            {
                sim_ctrl_verilator->set_input(din_net, BooleanFunction::Value::ONE);
            }

            sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= '1';
            sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ONE);       // wclke       <= '1';

            sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ZERO);    // write_en    <= '0';
            sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ZERO);       // wclke       <= '0';

            sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

            // // read data without rclke
            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= '1';

            // raddr      <= x"ff";
            for (const auto& read_addr_net : read_addr)
            {
                sim_ctrl_verilator->set_input(read_addr_net, BooleanFunction::Value::ONE);
            }

            sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

            // data_read = read_data();

            sim_ctrl_verilator->simulate(5 * clock_period);    // WAIT FOR 50 NS;
            // printf("sent %08x, received: %08x\n", data_write, data_read);

            // // read data with rclke
            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= '1';
            // raddr      <= x"ff";
            for (const auto& read_addr_net : read_addr)
            {
                sim_ctrl_verilator->set_input(read_addr_net, BooleanFunction::Value::ONE);
            }
            sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);    // rclke      <= '1';

            sim_ctrl_verilator->simulate(2 * clock_period);                        // WAIT FOR 20 NS;
            sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ZERO);    // rclke      <= '0';

            sim_ctrl_verilator->simulate(5 * clock_period);    // WAIT FOR 50 NS;

            // read some address and see what the result from INIT value is
            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= '1';

            // todo: bitorder could be wrong?
            //raddr      <= x"66";
            sim_ctrl_verilator->set_input(read_addr.at(7), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(read_addr.at(6), BooleanFunction::Value::ONE);
            //            sim_ctrl_verilator->set_input(read_addr.at(5), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(read_addr.at(4), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(read_addr.at(3), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(read_addr.at(2), BooleanFunction::Value::ONE);
            //            sim_ctrl_verilator->set_input(read_addr.at(1), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(read_addr.at(0), BooleanFunction::Value::ZERO);

            sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);    // rclke      <= '1';

            sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

            //data_read = read_data();

            // waddr       <= x"43"; 0100 0011
            sim_ctrl_verilator->set_input(write_addr.at(7), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(write_addr.at(6), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(write_addr.at(5), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(write_addr.at(4), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(write_addr.at(3), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(write_addr.at(2), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(write_addr.at(1), BooleanFunction::Value::ONE);
            //            sim_ctrl_verilator->set_input(write_addr.at(0), BooleanFunction::Value::ONE);

            // din         <= x"1111";
            sim_ctrl_verilator->set_input(din.at(15), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(14), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(13), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(12), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(11), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(10), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(9), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(8), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(7), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(6), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(5), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(4), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(3), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(2), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(din.at(1), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(0), BooleanFunction::Value::ONE);

            sim_ctrl_verilator->simulate(1 * clock_period);    // WAIT FOR 10 NS;

            sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= '1';
            sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ONE);       // wclke       <= '1';
            sim_ctrl_verilator->simulate(2 * clock_period);                          // WAIT FOR 20 NS;

            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);    // read_en    <= 1';
            sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);      // rclke       <= '1';

            // raddr      <= x"43";
            //            sim_ctrl_verilator->set_input(read_addr.at(7), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(read_addr.at(6), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(read_addr.at(5), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(read_addr.at(4), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(read_addr.at(3), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(read_addr.at(2), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(read_addr.at(1), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(read_addr.at(0), BooleanFunction::Value::ONE);

            sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

            // din <= x "ff11";
            sim_ctrl_verilator->set_input(din.at(15), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(14), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(13), BooleanFunction::Value::ONE);
            //            sim_ctrl_verilator->set_input(din.at(12), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(11), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(10), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(din.at(9), BooleanFunction::Value::ONE);
            //            sim_ctrl_verilator->set_input(din.at(8), BooleanFunction::Value::ONE);
            //            sim_ctrl_verilator->set_input(din.at(7), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(6), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(5), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(4), BooleanFunction::Value::ONE);
            //            sim_ctrl_verilator->set_input(din.at(3), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(2), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(1), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(din.at(0), BooleanFunction::Value::ONE);

            sim_ctrl_verilator->simulate(20 * clock_period);    // WAIT FOR 20 NS;

            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ZERO);    // read_en    <= 0';
            sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ZERO);      // rclke       <= '0';
            sim_ctrl_verilator->simulate(2 * clock_period);                          // WAIT FOR 20 NS;

            // mask        <= x"ffff";
            sim_ctrl_verilator->set_input(mask.at(15), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(14), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(13), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(12), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(11), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(10), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(9), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(8), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(7), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(6), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(5), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(4), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(3), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(2), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(1), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(0), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

            sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ZERO);    // write_en    <= 0';
            sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ZERO);       // wclke       <= '0';
            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ONE);      // read_en    <= 1';
            sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ONE);        // rclke       <= '1';
            sim_ctrl_verilator->simulate(2 * clock_period);                           // WAIT FOR 20 NS;

            sim_ctrl_verilator->set_input(write_en, BooleanFunction::Value::ONE);    // write_en    <= 1';
            sim_ctrl_verilator->set_input(wclke, BooleanFunction::Value::ONE);       // wclke       <= '1';
            sim_ctrl_verilator->set_input(read_en, BooleanFunction::Value::ZERO);    // read_en    <= 0';
            sim_ctrl_verilator->set_input(rclke, BooleanFunction::Value::ZERO);      // rclke       <= '0';

            // mask        <= x"1111";
            sim_ctrl_verilator->set_input(mask.at(15), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(14), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(13), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(mask.at(12), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(11), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(10), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(9), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(mask.at(8), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(7), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(6), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(5), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(mask.at(4), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->set_input(mask.at(3), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(2), BooleanFunction::Value::ZERO);
            sim_ctrl_verilator->set_input(mask.at(1), BooleanFunction::Value::ZERO);
            //            sim_ctrl_verilator->set_input(mask.at(0), BooleanFunction::Value::ONE);
            sim_ctrl_verilator->simulate(2 * clock_period);    // WAIT FOR 20 NS;

            sim_ctrl_verilator->simulate(100 * clock_period);    // WAIT FOR 100*10 NS;

            sim_ctrl_verilator->initialize();
            sim_ctrl_verilator->run_simulation();

            EXPECT_FALSE(verilator_engine->get_state() == SimulationEngine::State::Failed);

            while (verilator_engine->get_state() == SimulationEngine::State::Running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        if (verilator_engine->get_state() == SimulationEngine::State::Failed)
        {
            FAIL() << "engine failed";
        }

        EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Done);
        EXPECT_FALSE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::EngineFailed);

        sim_ctrl_verilator->get_results();

        for (Net* n : nl->get_nets())
        {
            sim_ctrl_verilator->get_waveform_by_net(n);
            sim_ctrl_reference->get_waveform_by_net(n);
        }

        // TODO @ Jörn: LOAD ALL WAVES TO MEMORY
        EXPECT_TRUE(sim_ctrl_verilator->get_waves()->size() == (int)nl->get_nets().size());
        EXPECT_TRUE(sim_ctrl_reference->get_waves()->size() == (int)nl->get_nets().size());

        //Test if maps are equal
        bool equal = cmp_sim_data(sim_ctrl_reference.get(), sim_ctrl_verilator.get());
        EXPECT_TRUE(equal);
        TEST_END
    }
}    // namespace hal
