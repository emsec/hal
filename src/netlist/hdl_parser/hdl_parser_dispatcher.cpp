#include "netlist/hdl_parser/hdl_parser_dispatcher.h"

#include "core/log.h"
#include "netlist/event_system/event_controls.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/hdl_parser/hdl_parser.h"
#include "netlist/hdl_parser/hdl_parser_verilog.h"
#include "netlist/hdl_parser/hdl_parser_vhdl.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"

namespace hal
{
    namespace HDLParserDispatcher
    {
        namespace
        {
            template<typename T>
            std::shared_ptr<Netlist> parse_hdl(const std::filesystem::path& file_name, const std::vector<std::shared_ptr<GateLibrary>>& gate_libraries)
            {
                std::ifstream ifs;
                std::stringstream ss;

                auto begin_time = std::chrono::high_resolution_clock::now();

                log_info("hdl_parser", "parsing '{}'...", file_name.string());

                ifs.open(file_name.c_str(), std::ifstream::in);
                if (!ifs.is_open())
                {
                    log_error("hdl_parser", "cannot open '{}'", file_name.string());
                    return nullptr;
                }
                ss << ifs.rdbuf();
                ifs.close();

                auto parser = T(ss);
                if (!parser.parse())
                {
                    log_error("hdl_parser", "parser cannot parse file '{}'.", file_name.string());
                    return nullptr;
                }

                log_info("hdl_parser",
                         "parsed '{}' in {:2.2f} seconds.",
                         file_name.string(),
                         (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

                for (const auto& gate_library : gate_libraries)
                {
                    begin_time = std::chrono::high_resolution_clock::now();

                    log_info("hdl_parser", "instantiating '{}' using gate library '{}'...", file_name.string(), gate_library->get_name());

                    auto gl = gate_library_manager::get_gate_library(gate_library->get_path());
                    if (gl == nullptr)
                    {
                        log_critical("netlist", "error loading gate library '{}'.", gate_library->get_name());
                        return nullptr;
                    }

                    std::shared_ptr<Netlist> netlist = parser.instantiate(gl);
                    if (netlist == nullptr)
                    {
                        log_error("hdl_parser", "parser cannot instantiate file '{}' using gate library '{}'.", file_name.string(), gate_library->get_name());
                        continue;
                    }

                    netlist->set_input_filename(file_name.string());

                    log_info("hdl_parser",
                             "instantiated '{}' in {:2.2f} seconds.",
                             file_name.string(),
                             (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

                    return netlist;
                }

                log_error("hdl_parser", "no suitable gate library found!");
                return nullptr;
            }
        }    // namespace

        ProgramOptions get_cli_options()
        {
            ProgramOptions description;
            description.add("--parser", "use a specific parser (optional, normally determined by file name extension)", {ProgramOptions::REQUIRED_PARAM});
            return description;
        }

        std::set<std::string> get_gui_option()
        {
            return {"vhdl", "verilog"};
        }

        std::shared_ptr<Netlist> parse(const std::filesystem::path& file_name, const ProgramArguments& args)
        {
            log_info("hdl_parser", "finding a parser for '{}'...", file_name.string());

            // all supported extension->parser_name mappings
            std::map<std::string, std::string> file_endings = {{".vhdl", "vhdl"}, {".vhd", "vhdl"}, {".v", "verilog"}};
            auto extension                                  = core_utils::to_lower(file_name.extension().string());

            auto parser_name = file_endings[extension];
            if (parser_name.empty())
            {
                if (!args.is_option_set("--parser"))
                {
                    log_error("hdl_parser", "no parser found for '{}'. Use '--parser' to use a specific parser.", extension);
                    return nullptr;
                }
                parser_name = args.get_parameter("--parser");
            }
            else
            {
                log_info("hdl_parser", "selected parser '{}' by file name extension.", parser_name);
            }

            std::vector<std::shared_ptr<GateLibrary>> gate_libraries;
            if (args.is_option_set("--gate-library"))
            {
                auto user_lib = gate_library_manager::get_gate_library(args.get_parameter("--gate-library"));
                if (user_lib == nullptr)
                {
                    return nullptr;
                }
                gate_libraries.push_back(user_lib);
            }
            else
            {
                log_warning("hdl_parser", "no (valid) gate library specified. trying to auto-detect gate library...");
                gate_library_manager::load_all();

                for (const auto& lib_it : gate_library_manager::get_gate_libraries())
                {
                    gate_libraries.push_back(lib_it);
                }
            }

            if (parser_name == "vhdl")
            {
                return parse_hdl<HDLParserVHDL>(file_name, gate_libraries);
            }
            else if (parser_name == "verilog")
            {
                return parse_hdl<HDLParserVerilog>(file_name, gate_libraries);
            }
            else
            {
                log_error("hdl_parser", "parser '{}' is unknown.", parser_name);
                return nullptr;
            }
        }

        std::shared_ptr<Netlist> parse(const std::shared_ptr<GateLibrary>& gate_library, const std::string& parser_name, const std::filesystem::path& file_name)
        {
            if (parser_name == "vhdl")
            {
                return parse_hdl<HDLParserVHDL>(file_name, {gate_library});
            }
            else if (parser_name == "verilog")
            {
                return parse_hdl<HDLParserVerilog>(file_name, {gate_library});
            }
            else
            {
                log_error("hdl_parser", "parser '{}' is unknown.", parser_name);
                return nullptr;
            }
        }
    }    // namespace HDLParserDispatcher
}    // namespace hal
