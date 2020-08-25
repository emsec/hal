#include "hal_core/netlist/hdl_parser/hdl_parser_manager.h"

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/hdl_parser/hdl_parser.h"
#include "hal_core/netlist/netlist.h"

#include <fstream>

namespace hal
{
    namespace hdl_parser_manager
    {
        namespace
        {
            std::unordered_map<std::string, std::vector<std::string>> m_parser_to_extensions;
            std::unordered_map<std::string, std::pair<std::string, ParserFactory>> m_extension_to_parser;

            ParserFactory get_parser_factory_for_file(const std::filesystem::path& file_name)
            {
                std::string extension = utils::to_lower(file_name.extension().string());
                if (!extension.empty() && extension[0] != '.')
                {
                    extension = "." + extension;
                }

                if (auto it = m_extension_to_parser.find(extension); it != m_extension_to_parser.end())
                {
                    log_info("hdl_parser", "selected parser: {}", it->second.first);
                    return it->second.second;
                }

                log_error("hdl_parser", "no hdl parser registered for file type '{}'", extension);
                return ParserFactory();
            }

            std::vector<std::unique_ptr<Netlist>>
                dispatch_parse(const std::filesystem::path& file_name, std::unique_ptr<HDLParser> parser, const GateLibrary* gate_library = nullptr, bool break_on_match = true)
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
                        log_error("hdl_parser", "could not open file '{}'", file_name.string());
                        return {};
                    }
                    stream << ifs.rdbuf();
                    ifs.close();
                }

                if (!parser->parse(stream))
                {
                    log_error("hdl_parser", "could not parse file '{}'.", file_name.string());
                    return {};
                }

                log_info("hdl_parser",
                         "finished parsing in {:2.2f} seconds.",
                         (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

                std::vector<std::unique_ptr<Netlist>> netlists;

                if (gate_library != nullptr)
                {
                    begin_time = std::chrono::high_resolution_clock::now();

                    log_info("hdl_parser", "instantiating '{}' with gate library '{}'...", file_name.string(), gate_library->get_name());

                    std::unique_ptr<Netlist> netlist = parser->instantiate(gate_library);
                    if (netlist == nullptr)
                    {
                        log_error("hdl_parser", "could not instantiate file '{}' using gate library '{}'.", file_name.string(), gate_library->get_name());
                        return {};
                    }

                    netlist->set_input_filename(file_name.string());

                    log_info("hdl_parser",
                             "instantiated '{}' in {:2.2f} seconds.",
                             file_name.string(),
                             (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

                    netlists.push_back(std::move(netlist));
                }
                else
                {
                    log_warning("hdl_parser", "no (valid) gate library specified, trying to auto-detect gate library...");
                    gate_library_manager::load_all();

                    for (GateLibrary* lib_it : gate_library_manager::get_gate_libraries())
                    {
                        begin_time = std::chrono::high_resolution_clock::now();

                        log_info("hdl_parser", "instantiating '{}' with gate library '{}'...", file_name.string(), lib_it->get_name());

                        std::unique_ptr<Netlist> netlist = parser->instantiate(lib_it);
                        if (netlist == nullptr)
                        {
                            log_error("hdl_parser", "could not instantiate file '{}' using gate library '{}'.", file_name.string(), lib_it->get_name());
                            continue;
                        }

                        netlist->set_input_filename(file_name.string());

                        log_info("hdl_parser",
                                 "instantiated '{}' in {:2.2f} seconds.",
                                 file_name.string(),
                                 (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

                        netlists.push_back(std::move(netlist));

                        if (break_on_match)
                        {
                            break;
                        }
                    }

                    if (netlists.empty())
                    {
                        log_error("hdl_parser", "could not find suitable gate library.");
                        return {};
                    }
                }

                return netlists;
            }
        }    // namespace

        ProgramOptions get_cli_options()
        {
            return ProgramOptions();
        }

        void register_parser(const std::string& name, const ParserFactory& parser_factory, const std::vector<std::string>& supported_file_extensions)
        {
            for (auto ext : supported_file_extensions)
            {
                ext = utils::trim(utils::to_lower(ext));
                if (!ext.empty() && ext[0] != '.')
                {
                    ext = "." + ext;
                }
                if (auto it = m_extension_to_parser.find(ext); it != m_extension_to_parser.end())
                {
                    log_warning("hdl_parser", "file type '{}' already has associated parser '{}', it remains unchanged", ext, it->second.first);
                    continue;
                }
                m_extension_to_parser.emplace(ext, std::make_pair(name, parser_factory));
                m_parser_to_extensions[name].push_back(ext);

                log_info("hdl_parser", "registered hdl parser '{}' for file type '{}'", name, ext);
            }
        }

        void unregister_parser(const std::string& name)
        {
            if (auto it = m_parser_to_extensions.find(name); it != m_parser_to_extensions.end())
            {
                for (const auto& ext : it->second)
                {
                    if (auto rm_it = m_extension_to_parser.find(ext); rm_it != m_extension_to_parser.end())
                    {
                        m_extension_to_parser.erase(rm_it);
                        log_info("hdl_parser", "unregistered hdl parser '{}' which was registered for file type '{}'", name, ext);
                    }
                }
                m_parser_to_extensions.erase(it);
            }
        }

        std::unique_ptr<Netlist> parse(const std::filesystem::path& file_name, const ProgramArguments& args)
        {
            ParserFactory factory = get_parser_factory_for_file(file_name);
            if (!factory)
            {
                return nullptr;
            }

            const GateLibrary* gate_library = nullptr;

            if (args.is_option_set("--gate-library"))
            {
                std::string gate_library_file = args.get_parameter("--gate-library");
                gate_library                  = gate_library_manager::get_gate_library(gate_library_file);
                if (gate_library == nullptr)
                {
                    log_error("hdl_parser", "invalid gate library '{}' specified by user.", gate_library_file);
                    return nullptr;
                }
            }

            return parse(file_name, gate_library);
        }

        std::unique_ptr<Netlist> parse(const std::filesystem::path& file_name, const GateLibrary* gate_library)
        {
            ParserFactory factory = get_parser_factory_for_file(file_name);
            if (!factory)
            {
                return nullptr;
            }

            std::vector<std::unique_ptr<Netlist>> netlists = dispatch_parse(file_name, factory(), gate_library);

            if (netlists.empty())
            {
                return nullptr;
            }

            return std::move(netlists.at(0));
        }

        std::vector<std::unique_ptr<Netlist>> parse_all(const std::filesystem::path& file_name)
        {
            ParserFactory factory = get_parser_factory_for_file(file_name);
            if (!factory)
            {
                return {};
            }

            return dispatch_parse(file_name, factory(), nullptr, false);
        }
    }    // namespace hdl_parser_manager
}    // namespace hal
