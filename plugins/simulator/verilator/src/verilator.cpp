
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

        const int VerilatorEngine::s_command_lines = 3;

        bool VerilatorEngine::setSimulationInput(SimulationInput* simInput)
        {
            mSimulationInput = simInput;

            //server_execution = false;
            const std::vector<const Gate*> simulation_gates(simInput->get_gates().begin(), simInput->get_gates().end());
            m_partial_netlist = netlist_utils::get_partial_netlist(simulation_gates.at(0)->get_netlist(), simulation_gates);

            remove_unwanted_parameters_from_netlist(m_partial_netlist.get());

            //m_simulator_dir = "/mnt/scratch/nils.albartus/simulation_100s_spi/";
            m_simulator_dir = get_working_directory();
            m_design_name   = m_partial_netlist->get_design_name();
            m_compiler      = get_engine_property("compiler");

            if (m_design_name.empty())
            {
                m_design_name = "dummy";
                log_warning("verilator", "no design name for partial metlist, set to '{}'.", m_design_name);
            }

            std::filesystem::path netlist_verilog = m_simulator_dir / std::string(m_partial_netlist->get_design_name() + ".v");

            // check for provided models and set path
            std::filesystem::path provided_models = get_engine_property("provided_models");
            if (provided_models.empty())
            {
                log_info("verilator", "the property 'provided_models' has not been set (use set_engine_property method to assign).");
            }

            // get engine properties
            if (!get_engine_property("num_of_threads").empty())
            {
                m_num_of_threads = std::stoi(get_engine_property("num_of_threads"));
            }

            // prepare folder
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
            // write necessary parser files
            if (!install_saleae_parser(m_simulator_dir.string()))
            {
                log_error("verilator", "could not install saleae parser in directory '{}'.", m_simulator_dir.string());
                return false;
            }

            // write / copy saleae binaries
            std::string testbench_cpp = get_testbench_cpp_template();
            testbench_cpp             = utils::replace(testbench_cpp, std::string("<design_name>"), m_partial_netlist->get_design_name());

            // set callbacks in parser
            std::stringstream callbacks;
            for (const auto& input_net : simInput->get_input_nets())
            {
                std::string net_name = escape_net_name(input_net->get_name());
                std::stringstream callback;
                callback
                        << "\n"
                        << "  auto " << net_name << "_net = netMap.find(std::string(\"" << input_net->get_name() << "\"));\n"
                        << "  if (" << net_name << "_net == netMap.end()) {\n"
                        << "     std::cerr << \"no SALEAE input data found for net " << net_name << "\" << std::endl;\n"
                        << "  }\n"
                        << "  else {\n"
                        << "     if (!sp.register_callback(" << net_name << "_net->second, set_simulation_value, &dut->" << net_name << ")) {\n"
                        << "         std::cerr << \"cannot initialize callback for net " << net_name << "\" << std::endl;\n"
                        << "     }\n"
                        << "  }\n" << std::endl;
            
                //callback << "printf(\"dut->" << net_name << ": %x\\n\", &dut->" << net_name << ");" << std::endl;

                callbacks << callback.str() << std::endl;
            }

            testbench_cpp = utils::replace(testbench_cpp, std::string("<set_callbacks>"), callbacks.str());

            std::ofstream testbench_cpp_file(m_simulator_dir / "testbench.cpp");
            testbench_cpp_file << testbench_cpp;
            testbench_cpp_file.close();

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
                    std::vector<std::string> retval = {"verilator",
                                                       "-I.",
                                                       "-Wall",
                                                       "-Wno-fatal",
                                                       "--MMD",
                                                       "-trace",
                                                    //    "--trace-threads",
                                                    //    "1",
                                                    //    "--threads",
                                                    //    "1",
                                                       "-y",
                                                       "gate_definitions/",
                                                       "--Mdir",
                                                       "obj_dir",
                                                       "-O3",
                                                       "--noassert",
                                                       "-CFLAGS",
                                                       "-O3",
                                                       "--exe",
                                                       "-cc",
                                                       "-DSIM_VERILATOR",
                                                       "--trace-depth",
                                                       "2",
                                                       "--trace-underscore",
                                                       "--coverage-underscore",
                                                       "testbench.cpp",
                                                       "saleae_directory.cpp",
                                                       "saleae_parser.cpp",
                                                       "saleae_file.cpp",
                                                       m_design_name + ".v"};

                    if (!m_compiler.empty())
                    {
                        retval.push_back("--compiler");
                        retval.push_back(m_compiler);
                    }
                    return retval;
                    break;
                }
                case 1: {
                    return {"make", "-j" + std::to_string(m_num_of_threads), "--no-print-directory", "-C", "obj_dir/", "-f", "V" + m_design_name + ".mk"};
                    break;
                }
                case 2: {
                    return {"obj_dir/V" + m_design_name};
                    break;
                }
                default:
                    break;
            }
            return {};
        }

        VerilatorEngine::VerilatorEngine(const std::string& nam) : SimulationEngineScripted(nam)
        {
            mRequireClockEvents = true;
        }

        bool VerilatorEngine::finalize()
        {
            mResultFilename = std::string(m_simulator_dir / "waveform.vcd");
            mState          = Done;
            return true;
        }

        SimulationEngine* VerilatorEngineFactory::createEngine() const
        {
            return new VerilatorEngine(mName);
        }

    }    // namespace verilator
}    // namespace hal
