#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <regex>
#include <sstream>

namespace hal
{
    namespace gate_library_parser_manager
    {
        namespace
        {
            std::unordered_map<std::string, std::vector<std::string>> m_parser_to_extensions;
            std::unordered_map<std::string, std::pair<std::string, ParserFactory>> m_extension_to_parser;

            ParserFactory get_parser_factory_for_file(const std::filesystem::path& file_name)
            {
                auto extension = utils::to_lower(file_name.extension().string());
                if (!extension.empty() && extension[0] != '.')
                {
                    extension = "." + extension;
                }

                if (auto it = m_extension_to_parser.find(extension); it != m_extension_to_parser.end())
                {
                    log_info("hdl_parser", "selected gate library parser '{}'.", it->second.first);
                    return it->second.second;
                }

                log_error("hdl_parser", "no gate library parser registered for file extension '{}'.", extension);
                return ParserFactory();
            }
        }    // namespace

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
                    log_warning("gate_library_parser", "file extension '{}' is already associated with parser '{}'.", ext, it->second.first);
                    continue;
                }
                m_extension_to_parser.emplace(ext, std::make_pair(name, parser_factory));
                m_parser_to_extensions[name].push_back(ext);

                log_info("gate_library_parser", "registered gate library parser '{}' for file extension '{}'.", name, ext);
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
                        log_info("gate_library_parser", "unregistered gate library parser '{}' for file extension '{}'.", name, ext);
                    }
                }
                m_parser_to_extensions.erase(it);
            }
        }

        std::unique_ptr<GateLibrary> parse(std::filesystem::path file_path)
        {
            auto factory = get_parser_factory_for_file(file_path);
            if (!factory)
            {
                return nullptr;
            }

            auto parser = factory();

            auto begin_time = std::chrono::high_resolution_clock::now();
            auto gate_lib   = parser->parse(file_path);
            if (gate_lib == nullptr)
            {
                log_error("gate_library_parser", "failed to parse gate library from file '{}'.", file_path.string());
                return nullptr;
            }

            log_info("gate_library_parser",
                     "parsed gate library '{}' from file '{}' in {:2.2f} seconds.",
                     gate_lib->get_name(),
                     file_path.string(),
                     (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

            return gate_lib;
        }
    }    // namespace gate_library_parser_manager
}    // namespace hal
