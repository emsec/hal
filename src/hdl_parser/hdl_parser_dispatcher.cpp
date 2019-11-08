#include "hdl_parser/hdl_parser_dispatcher.h"

#include "core/log.h"

#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"

#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/module_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/netlist_event_handler.h"

#include "hdl_parser/hdl_parser.h"
#include "hdl_parser/hdl_parser_verilog.h"
#include "hdl_parser/hdl_parser_vhdl.h"
#include "netlist/gate_library/gate_library_manager.h"

namespace hdl_parser_dispatcher
{
    program_options get_cli_options()
    {
        program_options description;
        description.add("--language", "parse language (optional, normally determined by file name extension)", {program_options::REQUIRED_PARAM});
        description.add("--gate-library", "parse gate-level library", {program_options::REQUIRED_PARAM});
        return description;
    }

    std::set<std::string> get_gui_option()
    {
        return {"vhdl", "verilog"};
    }

    std::shared_ptr<netlist> parse(const hal::path& file_name, const program_arguments& args)
    {
        // all supported extension->language mappings
        std::map<std::string, std::string> file_endings = {{".vhdl", "vhdl"}, {".vhd", "vhdl"}, {".v", "verilog"}};
        auto extension                                  = file_name.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        auto language = file_endings[extension];
        if (language.empty())
        {
            if (!args.is_option_set("--language"))
            {
                log_error("hdl_parser", "{}", "no language specific (did you forget the \'language\' switch?).");
                return nullptr;
            }
            language = args.get_parameter("--language");
        }
        else
            log_info("hdl_parser", "selected language '{}' by file name extension.", language);

        std::transform(language.begin(), language.end(), language.begin(), ::tolower);
        if (!args.is_option_set("--gate-library"))
        {
            log_warning("hdl_parser", "no gate library specified. trying to auto-detect gate library...");
            for (const auto& it : gate_library_manager::get_gate_libraries())
            {
                // log_manager::get_instance().deactivate_channel("hdl_parser");
                // log_manager::get_instance().deactivate_channel("netlist.internal");
                std::shared_ptr<netlist> netlist = parse(it.first, language, file_name);
                // log_manager::get_instance().activate_channel("hdl_parser");
                // log_manager::get_instance().activate_channel("netlist.internal");
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
        return parse(gate_library, language, file_name);
    }

    std::shared_ptr<netlist> parse(const std::string& gate_library, const std::string& language, const hal::path& file_name)
    {
        auto begin_time = std::chrono::high_resolution_clock::now();

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

        netlist_event_handler::enable(false);
        gate_event_handler::enable(false);
        net_event_handler::enable(false);
        module_event_handler::enable(false);

        if (language == "vhdl")
            g = hdl_parser_vhdl(ss).parse(gate_library);
        else if (language == "verilog")
            g = hdl_parser_verilog(ss).parse(gate_library);
        else
            log_error("hdl_parser", "language '{}' is unkown", language);

        if (g != nullptr)
        {
            g->set_input_filename(file_name.string());
        }

        netlist_event_handler::enable(true);
        gate_event_handler::enable(true);
        net_event_handler::enable(true);
        module_event_handler::enable(true);

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

    std::shared_ptr<netlist> parse(const std::string& gate_library, const std::string& language, const std::string& file_name)
    {
        return hdl_parser_dispatcher::parse(gate_library, language, hal::path(file_name));
    }
}    // namespace hdl_parser_dispatcher
