#include "netlist/gate_library/gate_library_manager.h"

#include "core/log.h"
#include "core/utils.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_library_parser.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

namespace hal
{
    namespace gate_library_manager
    {
        namespace
        {
            std::unordered_map<GateLibraryParser*, std::vector<std::string>> m_parser_to_extensions;
            std::unordered_map<std::string, GateLibraryParser*> m_extension_to_parser;
            std::map<std::filesystem::path, std::shared_ptr<GateLibrary>> m_gate_libraries;

            GateLibraryParser* get_parser_for_file(const std::filesystem::path& file_name)
            {
                auto extension = core_utils::to_lower(file_name.extension().string());
                if (!extension.empty() && extension[0] != '.')
                {
                    extension = "." + extension;
                }

                GateLibraryParser* parser = nullptr;
                if (auto it = m_extension_to_parser.find(extension); it != m_extension_to_parser.end())
                {
                    parser = it->second;
                }
                if (parser == nullptr)
                {
                    log_error("gate_library_manager", "no gate library parser registered for file type '{}'", extension);
                    return nullptr;
                }

                log_info("gate_library_manager", "selected gate library parser '{}'", parser->get_name());

                return parser;
            }

            std::shared_ptr<GateLibrary> load_liberty(const std::filesystem::path& path)
            {
                auto parser = get_parser_for_file(path);

                if (parser == nullptr)
                {
                    return nullptr;
                }

                std::stringstream stream;
                {
                    std::ifstream ifs;
                    ifs.open(path.string().c_str(), std::ifstream::in);
                    if (!ifs.is_open())
                    {
                        log_error("gate_library_manager", "cannot open '{}'", path.string());
                        return nullptr;
                    }
                    stream << ifs.rdbuf();
                    ifs.close();
                }

                auto lib = parser->parse(path, &stream);

                if (lib == nullptr)
                {
                    log_error("gate_library_manager", "failed to load gate library '{}'.", path.string());
                }

                return lib;
            }

            bool prepare_library(std::shared_ptr<GateLibrary>& lib)
            {
                auto types = lib->get_gate_types();

                if (lib->get_gnd_gate_types().empty())
                {
                    std::string name = "GND";
                    if (types.find(name) != types.end())
                    {
                        name += " (auto generated)";
                    }
                    if (types.find(name) != types.end())
                    {
                        log_error("gate_library_manager", "no GND gate found in parsed library but gate types 'GND' and '{}' already exist.", name);
                        return false;
                    }
                    auto gt = std::make_shared<GateType>(name);
                    gt->add_output_pin("O");
                    gt->add_boolean_function("O", BooleanFunction::ZERO);
                    lib->add_gate_type(gt);
                    log_info("gate_library_manager", "gate library did not contain a GND gate, auto-generated type '{}'", name);
                }

                if (lib->get_vcc_gate_types().empty())
                {
                    std::string name = "VCC";
                    if (types.find(name) != types.end())
                    {
                        name += " (auto generated)";
                    }
                    if (types.find(name) != types.end())
                    {
                        log_error("gate_library_manager", "no VCC gate found in parsed library but gate types 'VCC' and '{}' already exist.", name);
                        return false;
                    }
                    auto gt = std::make_shared<GateType>(name);
                    gt->add_output_pin("O");
                    gt->add_boolean_function("O", BooleanFunction::ONE);
                    lib->add_gate_type(gt);
                    log_info("gate_library_manager", "gate library did not contain a VCC gate, auto-generated type '{}'", name);
                }

                return true;
            }
        }    // namespace

        void register_parser(GateLibraryParser* parser, const std::vector<std::string>& supported_file_extensions)
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
                    log_warning("gate_library_manager", "file type '{}' already has associated parser '{}', it remains unchanged", ext, it->second->get_name());
                    continue;
                }
                m_extension_to_parser.emplace(ext, parser);
                m_parser_to_extensions[parser].push_back(ext);

                log_info("gate_library_manager", "registered gate library parser '{}' for file type '{}'", parser->get_name(), ext);
            }
        }

        void unregister_parser(GateLibraryParser* parser)
        {
            if (auto it = m_parser_to_extensions.find(parser); it != m_parser_to_extensions.end())
            {
                for (const auto& ext : it->second)
                {
                    if (auto rm_it = m_extension_to_parser.find(ext); rm_it != m_extension_to_parser.end())
                    {
                        m_extension_to_parser.erase(rm_it);
                        log_info("gate_library_manager", "unregistered gate library parser '{}' which was registered for file type '{}'", parser->get_name(), ext);
                    }
                }
                m_parser_to_extensions.erase(it);
            }
        }

        std::shared_ptr<GateLibrary> load_file(std::filesystem::path path, bool reload_if_existing)
        {
            if (!std::filesystem::exists(path))
            {
                log_error("gate_library_manager", "gate library file '{}' does not exist.", path.string());
                return nullptr;
            }

            if (!path.is_absolute())
            {
                path = std::filesystem::absolute(path);
            }

            if (!reload_if_existing)
            {
                auto it = m_gate_libraries.find(path);
                if (it != m_gate_libraries.end())
                {
                    log_info("gate_library_manager", "the gate library file '{}' is already loaded.", path.string());
                    return it->second;
                }
            }

            std::shared_ptr<GateLibrary> lib;
            auto begin_time = std::chrono::high_resolution_clock::now();
            if (core_utils::ends_with(path.string(), std::string(".lib")))
            {
                log_info("gate_library_manager", "loading file '{}'...", path.string());
                lib = load_liberty(path);
            }
            else
            {
                log_error("gate_library_manager", "no parser found for '{}'.", path.string());
            }

            if (lib == nullptr)
            {
                return nullptr;
            }

            auto elapsed = (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

            if (!prepare_library(lib))
            {
                return nullptr;
            }

            log_info("gate_library_manager", "loaded gate library '{}' in {:2.2f} seconds.", lib->get_name(), elapsed);

            m_gate_libraries[path.string()] = lib;

            return lib;
        }

        void load_all(bool reload_if_existing)
        {
            std::vector<std::filesystem::path> lib_dirs = core_utils::get_gate_library_directories();

            for (const auto& lib_dir : lib_dirs)
            {
                if (!std::filesystem::exists(lib_dir))
                {
                    continue;
                }

                log_info("gate_library_manager", "Reading all definitions from {}.", lib_dir.string());

                for (const auto& lib_path : core_utils::RecursiveDirectoryRange(lib_dir))
                {
                    load_file(lib_path.path(), reload_if_existing);
                }
            }
        }

        std::shared_ptr<GateLibrary> get_gate_library(const std::string& file_name)
        {
            std::filesystem::path absolute_path;

            if (std::filesystem::exists(file_name))
            {
                // if an existing file is queried, load it by its absolute path
                absolute_path = std::filesystem::absolute(file_name);
            }
            else
            {
                // if a non existing file is queried, search for it in the standard directories
                auto stripped_name = std::filesystem::path(file_name).filename();
                log_info("gate_library_manager", "'{}' does not exist, searching for '{}' in standard directories...", file_name, stripped_name.string());
                auto lib_path = core_utils::get_file(stripped_name, core_utils::get_gate_library_directories());
                if (lib_path.empty())
                {
                    log_info("gate_library_manager", "could not find any gate library file named '{}'.", stripped_name.string());
                    return nullptr;
                }
                absolute_path = std::filesystem::absolute(lib_path);
            }

            // absolute path to file is known, check if it is already loaded
            if (auto it = m_gate_libraries.find(absolute_path.string()); it != m_gate_libraries.end())
            {
                return it->second;
            }

            // not already loaded -> load
            return load_file(absolute_path);
        }

        std::vector<std::shared_ptr<GateLibrary>> get_gate_libraries()
        {
            std::vector<std::shared_ptr<GateLibrary>> res;
            res.reserve(m_gate_libraries.size());
            for (const auto& it : m_gate_libraries)
            {
                res.push_back(it.second);
            }
            return res;
        }

        void unload()
        {
            m_gate_libraries.clear();
        }
    }    // namespace gate_library_manager
}    // namespace hal
