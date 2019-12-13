#include "netlist/hdl_parser/hdl_parser_dispatcher.h"

#include "core/log.h"

#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"

#include "netlist/event_system/event_controls.h"

#include "netlist/hdl_parser/hdl_parser.h"
#include "netlist/hdl_parser/hdl_parser_verilog.h"
#include "netlist/hdl_parser/hdl_parser_vhdl.h"
#include "netlist/gate_library/gate_library_manager.h"

namespace hdl_parser_dispatcher
{
    program_options get_cli_options()
    {
        program_options description;
        description.add("--parser", "use a specific parser (optional, normally determined by file name extension)", {program_options::REQUIRED_PARAM});
        return description;
    }

    std::set<std::string> get_gui_option()
    {
        return {"vhdl", "verilog"};
    }

    std::shared_ptr<netlist> parse(const hal::path& file_name, const program_arguments& args)
    {
        log_info("hdl_parser", "finding a parser for '{}'...", file_name.string());

        // all supported extension->parser_name mappings
        std::map<std::string, std::string> file_endings = {{".vhdl", "vhdl"}, {".vhd", "vhdl"}, {".v", "verilog"}};
        auto extension                                  = file_name.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

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

        if (!args.is_option_set("--gate-library"))
        {
            log_warning("hdl_parser", "no gate library specified. trying to auto-detect gate library...");
            for (const auto& it : gate_library_manager::get_gate_libraries())
            {
                std::shared_ptr<netlist> netlist = parse(it.first, parser_name, file_name);
                if (netlist != nullptr)
                {
                    log_info("hdl_parser", "auto-selected '{}' for this netlist.", it.first);
                    return netlist;
                }
            }
            log_error("hdl_parser", "no suitable gate library found!");
            return nullptr;
        }
        std::string gate_library = args.get_parameter("--gate-library");
        return parse(gate_library, parser_name, file_name);
    }

    std::shared_ptr<netlist> parse(const std::string& gate_library, const std::string& parser_name, const hal::path& file_name)
    {
        auto begin_time = std::chrono::high_resolution_clock::now();

        log_info("hdl_parser", "parsing '{}' using gate library '{}'...", file_name.string(), gate_library);

        std::ifstream ifs;
        ifs.open(file_name.c_str(), std::ifstream::in);
        if (!ifs.is_open())
        {
            log_error("hdl_parser", "cannot open '{}'", file_name.string());
            return nullptr;
        }
        std::stringstream ss;
        ss << ifs.rdbuf();
        ifs.close();

        std::shared_ptr<netlist> g = nullptr;

        // event_controls::enable_all(false);

        if (parser_name == "vhdl")
            g = hdl_parser_vhdl(ss).parse(gate_library);
        else if (parser_name == "verilog")
            g = hdl_parser_verilog(ss).parse(gate_library);
        else
            log_error("hdl_parser", "parser '{}' is unkown", parser_name);

        if (g != nullptr)
        {
            g->set_input_filename(file_name.string());
        }

        // event_controls::enable_all(true);

        if (g == nullptr)
        {
            log_error("hdl_parser", "error while parsing '{}'!", file_name.string());
            return nullptr;
        }

        log_info("hdl_parser",
                 "parsed '{}' in {:2.2f} seconds.",
                 file_name.string(),
                 (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);
        return g;
    }

    std::shared_ptr<netlist> parse(const std::string& gate_library, const std::string& parser_name, const std::string& file_name)
    {
        return hdl_parser_dispatcher::parse(gate_library, parser_name, hal::path(file_name));
    }
}    // namespace hdl_parser_dispatcher
