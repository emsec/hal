#include "perf_test/plugin_perf_test.h"

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

#include <thread>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<PerfTestPlugin>();
    }

    std::string PerfTestPlugin::get_name() const
    {
        return std::string("perf_test");
    }

    std::string PerfTestPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void PerfTestPlugin::initialize()
    {
    }

    ProgramOptions PerfTestPlugin::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--perf_test", "executes the plugin perf_test");
        description.add("--base_path", "set base path of HAL install", {""});

        return description;
    }

    bool PerfTestPlugin::cmp_sim_data(NetlistSimulatorController* reference_simulation_ctrl, NetlistSimulatorController* simulation_ctrl, int tolerance)
    {
        bool no_errors                          = true;
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

    bool PerfTestPlugin::handle_cli_call(Netlist* nl, ProgramArguments& args)
    {
        std::string base_path;

        if (args.is_option_set("--base_path"))
        {
            if (args.get_parameter("--base_path").back() == '/')
                base_path = args.get_parameter("--base_path");
            else
                base_path = args.get_parameter("--base_path");
        }
        else
        {
            log_error("perf_test", "base_path parameter not set");
        }

        auto plugin = plugin_manager::get_plugin_instance<NetlistSimulatorControllerPlugin>("netlist_simulator_controller");

        auto sim_ctrl_verilator = plugin->create_simulator_controller("tocipher_simulator");
        auto verilator_engine   = sim_ctrl_verilator->create_simulation_engine("verilator");
        //EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::NoGatesSelected);
        //EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Preparing);

        auto sim_ctrl_reference = plugin->create_simulator_controller("tocipher_reference");

        //path to netlist
        std::string path_netlist = base_path + "/bin/hal_plugins/test-files/toycipher/cipher_flat.vhd";
        //FAIL() << "netlist for toycipher-test not found: " << path_netlist;

        //create netlist from path
        auto lib = nl->get_gate_library();
        if (lib == nullptr)
        {
            log_error("perf_test", "lib empty");
            return -1;
        }

        //path to vcd
        std::string path_vcd = base_path + "/bin/hal_plugins/test-files/toycipher/dump.vcd";
        if (!utils::file_exists(path_vcd))
        {
            log_error("perf_test", "ref vcd not found");
            return -1;
        }
        //FAIL() << "dump for toycipher-test not found: " << path_vcd;

        sim_ctrl_reference->add_gates(nl->get_gates());
        sim_ctrl_reference->initialize();
        sim_ctrl_reference->import_vcd(path_vcd, NetlistSimulatorController::FilterInputFlag::CompleteNetlist);

        //prepare simulation
        sim_ctrl_verilator->add_gates(nl->get_gates());
        //EXPECT_TRUE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::ParameterSetup);
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

        //FAIL() << "not all input nets set: actual " << input_nets_amount << " vs. " << sim_ctrl_verilator->get_input_nets().size();

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

            //EXPECT_FALSE(verilator_engine->get_state() == SimulationEngine::State::Failed);

            while (verilator_engine->get_state() == SimulationEngine::State::Running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        if (verilator_engine->get_state() == SimulationEngine::State::Failed)
        {
            //FAIL() << "engine failed";
        }

        //EXPECT_TRUE(verilator_engine->get_state() == SimulationEngine::State::Done);
        //EXPECT_FALSE(sim_ctrl_verilator->get_state() == NetlistSimulatorController::SimulationState::EngineFailed);

        sim_ctrl_verilator->get_results();

        for (Net* n : nl->get_nets())
        {
            sim_ctrl_verilator->get_waveform_by_net(n);
            sim_ctrl_reference->get_waveform_by_net(n);
        }

        // TODO @ Jörn: LOAD ALL WAVES TO MEMORY
        //EXPECT_TRUE(sim_ctrl_verilator->get_waves()->size() == (int)nl->get_nets().size());
        //EXPECT_TRUE(sim_ctrl_reference->get_waves()->size() <= (int)nl->get_nets().size());    // net might have additional '0' and '1'

        //Test if maps are equal
        bool equal = cmp_sim_data(sim_ctrl_reference.get(), sim_ctrl_verilator.get());
        return true;
    }

}    // namespace hal
