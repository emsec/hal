#include "netlist/hdl_parser/hdl_parser_manager.h"

#include "core/log.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/hdl_parser/hdl_parser.h"
#include "netlist/netlist.h"

#include <fstream>

namespace hal
{
    namespace hdl_parser_manager
    {
        namespace
        {
            std::unordered_map<HDLParser*, std::vector<std::string>> m_parser_to_extensions;
            std::unordered_map<std::string, HDLParser*> m_extension_to_parser;

            HDLParser* get_parser_for_file(const std::filesystem::path& file_name)
            {
                auto extension = core_utils::to_lower(file_name.extension().string());
                if (!extension.empty() && extension[0] != '.')
                {
                    extension = "." + extension;
                }

                HDLParser* parser = nullptr;
                if (auto it = m_extension_to_parser.find(extension); it != m_extension_to_parser.end())
                {
                    parser = it->second;
                }
                if (parser == nullptr)
                {
                    log_error("hdl_parser", "no hdl parser registered for file type '{}'", extension);
                    return nullptr;
                }

                log_info("hdl_parser", "selected parser: {}", parser->get_name());

                return parser;
            }

            std::shared_ptr<Netlist> dispatch_parse(const std::filesystem::path& file_name, HDLParser* parser, const std::vector<const GateLibrary*>& gate_libraries)
            {
                auto begin_time = std::chrono::high_resolution_clock::now();

                log_info("hdl_parser", "parsing '{}'...", file_name.string());

                // read file into stringstream
                std::stringstream stream;
                {
                    std::ifstream ifs;
                    ifs.open(file_name.c_str(), std::ifstream::in);
                    if (!ifs.is_open())
                    {
                        log_error("hdl_parser", "cannot open '{}'", file_name.string());
                        return nullptr;
                    }
                    stream << ifs.rdbuf();
                    ifs.close();
                }

                if (!parser->parse(&stream))
                {
                    log_error("hdl_parser", "parser cannot parse file '{}'.", file_name.string());
                    return nullptr;
                }

                log_info("hdl_parser",
                         "'{}' parsed '{}' in {:2.2f} seconds.",
                         parser->get_name(),
                         file_name.string(),
                         (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

                for (const auto& gate_library : gate_libraries)
                {
                    begin_time = std::chrono::high_resolution_clock::now();

                    log_info("hdl_parser", "instantiating with gate library '{}'...", file_name.string(), gate_library->get_name());

                    auto gl = gate_library_manager::get_gate_library(gate_library->get_path());
                    if (gl == nullptr)
                    {
                        log_critical("netlist", "error loading gate library '{}'.", gate_library->get_name());
                        return nullptr;
                    }

                    std::shared_ptr<Netlist> netlist = parser->instantiate(gl);
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
            return ProgramOptions();
        }

        void register_parser(HDLParser* parser, const std::vector<std::string>& supported_file_extensions)
        {
            for (auto ext : supported_file_extensions)
            {
                ext = core_utils::trim(core_utils::to_lower(ext));
                if (!ext.empty() && ext[0] != '.')
                {
                    ext = "." + ext;
                }
                if (auto it = m_extension_to_parser.find(ext); it != m_extension_to_parser.end())
                {
                    log_warning("hdl_parser", "file type '{}' already has associated parser '{}', it remains unchanged", ext, it->second->get_name());
                    continue;
                }
                m_extension_to_parser.emplace(ext, parser);
                m_parser_to_extensions[parser].push_back(ext);

                log_info("hdl_parser", "registered hdl parser '{}' for file type '{}'", parser->get_name(), ext);
            }
        }

        void unregister_parser(HDLParser* parser)
        {
            if (auto it = m_parser_to_extensions.find(parser); it != m_parser_to_extensions.end())
            {
                for (const auto& ext : it->second)
                {
                    if (auto rm_it = m_extension_to_parser.find(ext); rm_it != m_extension_to_parser.end())
                    {
                        m_extension_to_parser.erase(rm_it);
                        log_info("hdl_parser", "unregistered hdl parser '{}' which was registered for file type '{}'", parser->get_name(), ext);
                    }
                }
                m_parser_to_extensions.erase(it);
            }
        }

        std::shared_ptr<Netlist> parse(const std::filesystem::path& file_name, const ProgramArguments& args)
        {
            auto parser = get_parser_for_file(file_name);
            if (parser == nullptr)
            {
                return nullptr;
            }

            std::vector<const GateLibrary*> gate_libraries;

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

            return dispatch_parse(file_name, parser, gate_libraries);
        }

        std::shared_ptr<Netlist> parse(const std::filesystem::path& file_name, const GateLibrary* gate_library)
        {
            auto parser = get_parser_for_file(file_name);
            if (parser == nullptr)
            {
                return nullptr;
            }
            return dispatch_parse(file_name, parser, {gate_library});
        }
    }    // namespace hdl_parser_manager
}    // namespace hal
