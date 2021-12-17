
#include "verilator/verilator.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "verilator/templates.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace hal
{
    namespace verilator
    {
        namespace converter
        {
            std::vector<std::string> get_vector_for_const_char(const char** txt)
            {
                std::vector<std::string> retval;
                for (int i = 0; txt[i]; i++)
                {
                    retval.push_back(std::string(txt[i]));
                }
                return retval;
            }
        }    // namespace converter

        void remove_unwanted_parameters_from_netlist(Netlist* nl)
        {
            for (const auto& gate : nl->get_gates())
            {
                gate->delete_data("generic", "X_COORDINATE");
                gate->delete_data("generic", "Y_COORDINATE");

                // TODO: remove this from netlist
                gate->delete_data("generic", "READ_MODE");
                gate->delete_data("generic", "WRITE_MODE");

                if (!gate->get_type()->has_property(hal::GateTypeProperty::lut) && gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                {
                    gate->delete_data("generic", "INIT");
                }
            }
        }

        std::string escape_net_name(std::string net_name)
        {
            std::string new_net_name;
            new_net_name = utils::replace(net_name, std::string("'"), std::string("__027"));
            return new_net_name;
        }

        const int VerilatorEngine::s_command_lines = 1;

        bool VerilatorEngine::setSimulationInput(SimulationInput* simInput)
        {
            //server_execution = false;
            const std::vector<const Gate*> simulation_gates(simInput->get_gates().begin(), simInput->get_gates().end());
            m_partial_netlist = netlist_utils::get_partial_netlist(simulation_gates.at(0)->get_netlist(), simulation_gates);

            remove_unwanted_parameters_from_netlist(m_partial_netlist.get());

            m_simulator_dir = directory();
            m_design_name   = m_partial_netlist->get_design_name();
            if (m_design_name.empty())
            {
                m_design_name = "dummy";
                log_warning("verilator", "no design name for partial metlist, set to '{}'.", m_design_name);
            }
            else
                std::cerr << "m_design_name = <" << m_design_name << ">" << std::endl;

            std::filesystem::path netlist_verilog = m_simulator_dir / std::string(m_partial_netlist->get_design_name() + ".v");

            // check for provided models and set path
            std::filesystem::path provided_models = get_engine_property("provided_models");
            if (provided_models.empty())
            {
                log_info("verilator", "the property 'provided_models' has not been set (use set_engine_property method to assign).");
            }

            if (!get_engine_property("num_of_threads").empty())
            {
                m_num_of_threads = std::stoi(get_engine_property("num_of_threads"));
            }

            if (!write_testbench_files(simInput))
            {
                log_error("verilator", "error, testbench files for verilog could not be written");
            }

            if (!converter::convert_gate_library_to_verilog(m_partial_netlist.get(), m_simulator_dir, provided_models))
            {
                log_error("verilator", "could not create gate definitions in verilog");
                return false;
            };

            netlist_writer_manager::write(m_partial_netlist.get(), netlist_verilog);

            return true;    // everything ok
        }

        bool VerilatorEngine::write_testbench_files(SimulationInput* simInput)
        {
            // set inputs in testbench
            std::stringstream simulation_data;

            log_info("verilator", "simulating {} net events", simInput->get_simulation_net_events().size());

            u64 sim_counter                      = 0;
            u64 max_events_per_partial_testbench = 20000;
            bool initial_event                   = true;

            for (const auto& sim_event : simInput->get_simulation_net_events())
            {
                u32 duration = sim_event.get_simulation_duration();

                for (const auto& [net, boolean_value] : sim_event)
                {
                    switch (boolean_value)
                    {
                        case BooleanFunction::Value::ONE:
                            simulation_data << "\tdut->" << escape_net_name(net->get_name()) << " = 0x1;";
                            break;

                        case BooleanFunction::Value::ZERO:
                            simulation_data << "\tdut->" << escape_net_name(net->get_name()) << " = 0x0;";
                            break;

                        case BooleanFunction::Value::X:
                            // do nothing for initial event
                            if (!initial_event)
                            {
                                log_error("verilator",
                                          "cannot assign value '{}' to net '{}' at {}ps, only supporting 1, 0 aborting...",
                                          BooleanFunction::to_string(boolean_value),
                                          net->get_name(),
                                          duration);
                                return false;
                            }

                            break;

                        default:
                            log_error(
                                "verilator", "cannot assign value '{}' to net '{}' at {}ps, only supporting 1, 0 aborting...", BooleanFunction::to_string(boolean_value), net->get_name(), duration);
                            return false;
                            break;
                    }
                }
                simulation_data << "\tdut->eval();" << std::endl;
                simulation_data << "\tm_trace->dump(sim_time);" << std::endl;
                simulation_data << "\tsim_time += " << duration << ";" << std::endl;
                simulation_data << std::endl;
                initial_event = false;

                // split testbench or write the rest to the last file
                if ((sim_counter % max_events_per_partial_testbench) == (max_events_per_partial_testbench - 1) || sim_counter == simInput->get_simulation_net_events().size() - 1)
                {
                    log_debug("verilator", "creating new partial testbench {}", m_partial_testbenches);
                    std::string partial_testbench_cpp = get_partial_testbench_cpp_template();

                    partial_testbench_cpp = utils::replace(partial_testbench_cpp, std::string("<top_system>"), m_partial_netlist->get_design_name());
                    partial_testbench_cpp = utils::replace(partial_testbench_cpp, std::string("<insert_trace_here>"), simulation_data.str());
                    partial_testbench_cpp = utils::replace(partial_testbench_cpp, std::string("<part_xy>"), "part_" + std::to_string(m_partial_testbenches));

                    // write partial test bench
                    std::ofstream partial_testbench_cpp_file(m_simulator_dir / std::string("part_" + std::to_string(m_partial_testbenches) + ".cpp"));
                    partial_testbench_cpp_file << partial_testbench_cpp;
                    partial_testbench_cpp_file.close();

                    simulation_data.str(std::string());
                    m_partial_testbenches++;
                }
                sim_counter++;
            }

            // write testbench.cpp & testbench.h
            std::stringstream partial_testbenches_cpp;
            std::stringstream partial_testbenches_h;

            std::string testbench_cpp = get_testbench_cpp_template();
            std::string testbench_h   = get_testbench_h_template();

            for (u64 i = 0; i < m_partial_testbenches; i++)
            {
                partial_testbenches_cpp << "\tpart_" << i << "(dut, m_trace, sim_time);" << std::endl;
                partial_testbenches_h << "void part_" << i << "(Vchip* dut, VerilatedFstC* m_trace, vluint64_t& sim_time);" << std::endl;
            }

            testbench_cpp = utils::replace(testbench_cpp, std::string("<insert_trace_here>"), partial_testbenches_cpp.str());
            testbench_cpp = utils::replace(testbench_cpp, std::string("<top_system>"), m_partial_netlist->get_design_name());

            testbench_h = utils::replace(testbench_h, std::string("<top_system>"), m_partial_netlist->get_design_name());
            testbench_h = utils::replace(testbench_h, std::string("<insert_partial_testbench_here>"), partial_testbenches_h.str());

            std::ofstream testbench_cpp_file(m_simulator_dir / "testbench.cpp");
            testbench_cpp_file << testbench_cpp;
            testbench_cpp_file.close();

            std::ofstream testbench_h_file(m_simulator_dir / "testbench.h");
            testbench_h_file << testbench_h;
            testbench_h_file.close();

            // write execute.sh
            std::string makefile = get_makefile_template();

            makefile = utils::replace(makefile, std::string("<design_name>"), m_partial_netlist->get_design_name());
            makefile = utils::replace(makefile, std::string("<num_of_trace_threads>"), std::string("--trace-threads 8"));     // TODO: add parameter...
            makefile = utils::replace(makefile, std::string("<num_of_binary_trace_threads>"), std::string("--threads 8"));    // TODO: add parameter...
            makefile = utils::replace(makefile, std::string("<verilated_threads.o>"), std::string("verilated_threads.o"));    // TODO: add parameter...
            makefile = utils::replace(makefile, std::string("<num_of_build_threads>"), std::to_string(m_num_of_threads));

            std::ofstream makefile_file(m_simulator_dir / "execute.sh");
            makefile_file << makefile;
            makefile_file.close();

            std::filesystem::permissions(m_simulator_dir / "execute.sh", std::filesystem::perms::owner_all | std::filesystem::perms::group_all);

            return true;
        }

        int VerilatorEngine::numberCommandLines() const
        {
            return s_command_lines;
        }

        std::vector<std::string> VerilatorEngine::commandLine(int lineIndex) const
        {
            // returns commands to be executed

            switch (lineIndex)
            {
                case 0: {
                    const char* cl[]                = {"./execute.sh", nullptr};
                    std::vector<std::string> retval = converter::get_vector_for_const_char(cl);
                    return retval;
                }
                break;
                default:
                    break;
            }
            // switch (lineIndex)
            // {
            //     case 0: {
            //         const char* cl[]                = {"verilator",
            //                             "-I.",
            //                             "-Wall",
            //                             "-Wno-fatal",
            //                             "--MMD",
            //                             "-trace",
            //                             "-y",
            //                             "gate_definitions/",
            //                             "--Mdir",
            //                             "obj_dir",
            //                             "--exe",
            //                             "-cc",
            //                             "-DSIM_VERILATOR",
            //                             "--trace-depth",
            //                             "2",
            //                             "-CFLAGS",
            //                             "-mcmodel=large",
            //                             "testbench.cpp",
            //                             nullptr};
            //         std::vector<std::string> retval = converter::get_vector_for_const_char(cl);
            //         for (u64 i = 0; i < m_partial_testbenches; i++)
            //         {
            //             retval.push_back("part_" + std::to_string(i) + ".cpp");
            //         }
            //         retval.push_back(m_design_name + ".v");
            //         return retval;
            //     }
            //     break;
            //     case 1: {
            //         const char* cl[]                = {"make", "-j4", "--no-print-directory", "-C", "obj_dir/", "-f", nullptr};
            //         std::vector<std::string> retval = converter::get_vector_for_const_char(cl);
            //         retval.push_back("V" + m_design_name + ".mk");

            //         retval[1] = "-j" + std::to_string(m_num_of_threads);
            //         return retval;
            //     }
            //     break;
            //     case 2: {
            //         std::vector<std::string> retval;
            //         retval.push_back("obj_dir/V" + m_design_name);
            //         return retval;
            //     }
            //     break;
            //     default:
            //         break;
            // }
            return std::vector<std::string>();
        }

        VerilatorEngine::VerilatorEngine(const std::string& nam) : SimulationEngineScripted(nam)
        {
            mRequireClockEvents = true;
        }

        bool VerilatorEngine::finalize()
        {
            mResultFilename = std::string(m_simulator_dir / "waveform.fst");
            mState          = Done;
            return true;
        }

        SimulationEngine* VerilatorEngineFactory::createEngine() const
        {
            return new VerilatorEngine(mName);
        }

    }    // namespace verilator
}    // namespace hal
