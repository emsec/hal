
#include "verilator/verilator.h"
#include "verilator/path_to_verilator_executable.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
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
#include <regex>

namespace hal
{
    namespace verilator
    {
        void remove_unwanted_parameters_from_netlist(Netlist* nl)
        {
            const auto* gl = nl->get_gate_library();

            std::vector<std::pair<std::string, std::vector<std::string>>> lut_init_data;
            for (const auto& lut_type : gl->get_gate_types([](const GateType* gt) { return gt->has_property(GateTypeProperty::c_lut); }))
            {
                InitComponent* init_component = lut_type.second->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                if (init_component == nullptr)
                {
                    continue;
                }

                lut_init_data.push_back(std::make_pair(init_component->get_init_category(), init_component->get_init_identifiers()));
            }

            for (const auto& gate : nl->get_gates())
            {
                const auto& data_category   = gl->get_gate_location_data_category();
                const auto& data_identifier = gl->get_gate_location_data_identifiers();

                gate->delete_data(data_category, data_identifier.first);
                gate->delete_data(data_category, data_identifier.second);

                // TODO: remove this from netlist
                gate->delete_data("generic", "READ_MODE");
                gate->delete_data("generic", "WRITE_MODE");

                if (!gate->get_type()->has_property(hal::GateTypeProperty::c_lut) && gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                {
                    for (const auto& [init_category, init_identifiers] : lut_init_data)
                    {
                        for (const auto& init_identifier : init_identifiers)
                        {
                            gate->delete_data(init_category, init_identifier);
                        }
                    }
                }
            }
        }

        std::string escape_net_name(std::string net_name)
        {
            std::string new_net_name;
            new_net_name = utils::replace(net_name, std::string("__"), std::string("___05F"));
            new_net_name = utils::replace(new_net_name, std::string("'"), std::string("__027"));
            new_net_name = utils::replace(new_net_name, std::string("("), std::string("__028"));
            new_net_name = utils::replace(new_net_name, std::string(")"), std::string("__029"));
            new_net_name = utils::replace(new_net_name, std::string("["), std::string("__05b"));
            new_net_name = utils::replace(new_net_name, std::string("]"), std::string("__05d"));
            return new_net_name;
        }

        void cleanup_gate_type_names(const Netlist* nl, const std::filesystem::path& netlist_verilog)
        {
            std::unordered_map<GateType*,std::string> gate_type_names = converter::get_gate_gate_types_from_netlist(nl);
            std::vector<std::pair<std::string,std::string> > replace_names;
            for (auto it : gate_type_names)
                if (it.first->get_name() != it.second)
                    replace_names.push_back(std::make_pair(it.first->get_name(), it.second));
            if (replace_names.empty()) return;
            std::ifstream ifstr;
            ifstr.open(netlist_verilog);
            std::string buffer;
            std::string line;
            while(ifstr)
            {
                std::getline(ifstr, line);
                for (auto jt : replace_names)
                {
                    size_t pos = 0;
                    for (;;) // loop until string not found (break)
                    {
                        pos = line.find(jt.first, pos);
                        if (pos == std::string::npos) break;
                        line.replace(pos, jt.first.size(), jt.second);
                        pos += jt.second.size();
                    }
                }
                buffer += line + "\n";
            }
            ifstr.close();
            std::ofstream ofstr(netlist_verilog, std::ofstream::trunc);
            ofstr << buffer;
            ofstr.close();
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

            cleanup_gate_type_names(m_partial_netlist.get(), netlist_verilog);

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
                callback << "\n"
                         << "  auto " << net_name << "_net = netMap.find(std::string(\"" << input_net->get_name() << "\"));\n"
                         << "  if (" << net_name << "_net == netMap.end()) {\n"
                         << "     std::cerr << \"no SALEAE input data found for net " << net_name << "\" << std::endl;\n"
                         << "  }\n"
                         << "  else {\n"
                         << "     if (!sp.register_callback(" << net_name << "_net->second, set_simulation_value, &dut->" << net_name << ")) {\n"
                         << "         std::cerr << \"cannot initialize callback for net " << net_name << "\" << std::endl;\n"
                         << "     }\n"
                         << "  }\n"
                         << std::endl;

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
                    std::vector<std::string> retval = {"verilator",             // 0
                                                       "-I.",                   // 1
                                                       "-Wall",                 // 2
                                                       "-Wno-fatal",            // 3
                                                       "-CFLAGS",               // 4
                                                       "-O3",                   // 5
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

#if defined(__APPLE__)
                    if (strlen(path_to_verilator_executable))
                    {
                        retval[0] = path_to_verilator_executable + std::string("verilator");
                    }
#endif

                    if (strlen(path_to_rapidjson_includedir))
                    {
                        retval[5] = retval[5] + " -I" + path_to_rapidjson_includedir;
                    }

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
