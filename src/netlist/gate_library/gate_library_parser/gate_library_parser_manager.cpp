#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
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

            std::map<std::filesystem::path, std::unique_ptr<GateLibrary>> m_gate_libraries;

            ParserFactory get_parser_factory_for_file(const std::filesystem::path& file_name)
            {
                auto extension = utils::to_lower(file_name.extension().string());
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

            std::unique_ptr<GateLibrary> dispatch_parse(const std::filesystem::path& path)
            {
                auto factory = get_parser_factory_for_file(path);

                if (!factory)
                {
                    return nullptr;
                }

                auto parser = factory();
                auto lib    = parser->parse(path);

                if (lib == nullptr)
                {
                    log_error("gate_library_parser", "failed to load gate library '{}'.", path.string());
                }

                return lib;
            }

            bool prepare_library(const std::unique_ptr<GateLibrary>& lib)
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
                        log_error("gate_library_parser", "no GND gate found in parsed library but gate types 'GND' and '{}' already exist.", name);
                        return false;
                    }
                    auto gt = std::make_unique<GateType>(name);
                    gt->add_output_pin("O");
                    gt->add_boolean_function("O", BooleanFunction::ZERO);
                    lib->add_gate_type(std::move(gt));
                    log_info("gate_library_parser", "gate library did not contain a GND gate, auto-generated type '{}'", name);
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
                        log_error("gate_library_parser", "no VCC gate found in parsed library but gate types 'VCC' and '{}' already exist.", name);
                        return false;
                    }
                    auto gt = std::make_unique<GateType>(name);
                    gt->add_output_pin("O");
                    gt->add_boolean_function("O", BooleanFunction::ONE);
                    lib->add_gate_type(std::move(gt));
                    log_info("gate_library_parser", "gate library did not contain a VCC gate, auto-generated type '{}'", name);
                }

                return true;
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
                    log_warning("gate_library_parser", "file type '{}' already has associated parser '{}', it remains unchanged", ext, it->second.first);
                    continue;
                }
                m_extension_to_parser.emplace(ext, std::make_pair(name, parser_factory));
                m_parser_to_extensions[name].push_back(ext);

                log_info("gate_library_parser", "registered gate library parser '{}' for file type '{}'", name, ext);
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
                        log_info("gate_library_parser", "unregistered gate library parser '{}' which was registered for file type '{}'", name, ext);
                    }
                }
                m_parser_to_extensions.erase(it);
            }
        }

        GateLibrary* load_file(std::filesystem::path path, bool reload_if_existing)
        {
            if (!std::filesystem::exists(path))
            {
                log_error("gate_library_parser", "gate library file '{}' does not exist.", path.string());
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
                    log_info("gate_library_parser", "the gate library file '{}' is already loaded.", path.string());
                    return it->second.get();
                }
            }

            std::unique_ptr<GateLibrary> lib;
            auto begin_time = std::chrono::high_resolution_clock::now();
            if (utils::ends_with(path.string(), std::string(".lib")))
            {
                log_info("gate_library_parser", "loading file '{}'...", path.string());
                lib = dispatch_parse(path);
            }
            else
            {
                log_error("gate_library_parser", "no parser found for '{}'.", path.string());
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

            log_info("gate_library_parser", "loaded gate library '{}' in {:2.2f} seconds.", lib->get_name(), elapsed);

            auto res                        = lib.get();
            m_gate_libraries[path.string()] = std::move(lib);
            return res;
        }

        void load_all(bool reload_if_existing)
        {
            std::vector<std::filesystem::path> lib_dirs = utils::get_gate_library_directories();

            for (const auto& lib_dir : lib_dirs)
            {
                if (!std::filesystem::exists(lib_dir))
                {
                    continue;
                }

                log_info("gate_library_parser", "Reading all definitions from {}.", lib_dir.string());

                for (const auto& lib_path : utils::RecursiveDirectoryRange(lib_dir))
                {
                    load_file(lib_path.path(), reload_if_existing);
                }
            }
        }

        GateLibrary* get_gate_library(const std::string& file_name)
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
                log_info("gate_library_parser", "'{}' does not exist, searching for '{}' in standard directories...", file_name, stripped_name.string());
                auto lib_path = utils::get_file(stripped_name, utils::get_gate_library_directories());
                if (lib_path.empty())
                {
                    log_info("gate_library_parser", "could not find any gate library file named '{}'.", stripped_name.string());
                    return nullptr;
                }
                absolute_path = std::filesystem::absolute(lib_path);
            }

            // absolute path to file is known, check if it is already loaded
            if (auto it = m_gate_libraries.find(absolute_path.string()); it != m_gate_libraries.end())
            {
                return it->second.get();
            }

            // not already loaded -> load
            return load_file(absolute_path);
        }

        GateLibrary* get_gate_library_by_name(const std::string& lib_name)
        {
            for (const auto& it : m_gate_libraries)
            {
                if (it.second->get_name() == lib_name)
                {
                    return it.second.get();
                }
            }
            return nullptr;
        }

        std::vector<GateLibrary*> get_gate_libraries()
        {
            std::vector<GateLibrary*> res;
            res.reserve(m_gate_libraries.size());
            for (const auto& it : m_gate_libraries)
            {
                res.push_back(it.second.get());
            }
            return res;
        }
    }    // namespace gate_library_parser_manager
}    // namespace hal
