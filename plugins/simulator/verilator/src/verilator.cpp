
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

        const int VerilatorEngine::s_command_lines = 3;

        bool VerilatorEngine::setSimulationInput(SimulationInput* simInput)
        {
            mSimulationInput = simInput;

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
            {
                std::cerr << "m_design_name = <" << m_design_name << ">" << std::endl;
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
            std::ofstream saleae_parser_h_file(m_simulator_dir / "saleae_parser.h");
            saleae_parser_h_file << get_saleae_parser_h();
            saleae_parser_h_file.close();

            std::ofstream saleae_parser_cpp_file(m_simulator_dir / "saleae_parser.cpp");
            saleae_parser_cpp_file << get_saleae_parser_cpp();
            saleae_parser_cpp_file.close();

            std::ofstream saleae_file_h_file(m_simulator_dir / "saleae_file.h");
            saleae_file_h_file << get_saleae_file_h();
            saleae_file_h_file.close();

            std::ofstream saleae_file_cpp_file(m_simulator_dir / "saleae_file.cpp");
            saleae_file_cpp_file << get_saleae_file_cpp();
            saleae_file_cpp_file.close();

            // write / copy saleae binaries
            std::string testbench_cpp = get_testbench_cpp_template();
            testbench_cpp             = utils::replace(testbench_cpp, std::string("<design_name>"), m_partial_netlist->get_design_name());

            // TODO: workaround set gnd vcd, because we cannot generate saleae binaries just yet
            bool set_gnd = true;    // TODO
            if (set_gnd)
                testbench_cpp = utils::replace(testbench_cpp, std::string("<set_gnd>"), std::string("dut->__0270__027 = 0x0;"));
            else
                testbench_cpp = utils::replace(testbench_cpp, std::string("<set_gnd>"), std::string(""));

            bool set_vcc = true;    // TODO
            if (set_vcc)
                testbench_cpp = utils::replace(testbench_cpp, std::string("<set_vcc>"), std::string("dut->__0271__027 = 0x1;"));
            else
                testbench_cpp = utils::replace(testbench_cpp, std::string("<set_vcc>"), std::string(""));

            // set callbacks in parser
            std::stringstream callbacks;
            for (const auto& input_net : simInput->get_input_nets())
            {
                std::string net_name = input_net->get_name();
                std::stringstream callback;
                callback << "  std::string " << net_name << " = \"" << net_name << "\";" << std::endl;
                callback << "  if (!sp.registerCallback(" << net_name << "_str, set_simulation_value, &dut->" << escape_net_name(net_name) << ")) {" << std::endl;
                callback << "    std::cerr << \"cannot initialize callback for net <\" << " << net_name << "n1467_str << \">\" << std::endl;" << std::endl;
                callback << "  }" << std::endl;
                callback << std::endl;

                callbacks << callback.str() << std::endl;
            }

            testbench_cpp = utils::replace(testbench_cpp, std::string("<set_callbacks>"), callbacks.str());

            std::ofstream testbench_cpp_file(m_simulator_dir / "saleae_parser.cpp");
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
                    const char* cl[] = {"verilator",
                                        "-I.",
                                        "-Wall",
                                        "-Wno-fatal",
                                        "--MMD",
                                        "-trace",
                                        "--trace_threads 1",
                                        "--threads 1",
                                        "-y",
                                        "gate_definitions/",
                                        "--Mdir",
                                        "obj_dir",
                                        "-O3",
                                        "--noassert",
                                        "-CFLAGS",
                                        "-O3",
                                        "-LDFLAGS",
                                        "-lstdc++fs",
                                        "--exe",
                                        "-cc",
                                        "-DSIM_VERILATOR",
                                        "--trace-depth",
                                        "2",
                                        "testbench.cpp",
                                        "saleae_parser.cpp",
                                        "saleae_file.cpp",
                                        nullptr};

                    std::vector<std::string> retval = converter::get_vector_for_const_char(cl);
                    retval.push_back(m_design_name + ".v");
                    return retval;
                }
                break;
                case 1: {
                    const char* cl[]                = {"make", "-j4", "--no-print-directory", "-C", "obj_dir/", "-f", nullptr};
                    std::vector<std::string> retval = converter::get_vector_for_const_char(cl);
                    retval.push_back("V" + m_design_name + ".mk");

                    retval[1] = "-j" + std::to_string(m_num_of_threads);
                    return retval;
                }
                break;
                case 2: {
                    std::vector<std::string> retval;
                    retval.push_back("obj_dir/V" + m_design_name);
                    return retval;
                }
                break;
                default:
                    break;
            }
            return std::vector<std::string>();
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
