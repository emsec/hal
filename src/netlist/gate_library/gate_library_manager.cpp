#include "netlist/gate_library/gate_library_manager.h"

#include "core/log.h"
#include "core/utils.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_library_parser/gate_library_parser_liberty.h"

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
            std::map<std::filesystem::path, std::shared_ptr<GateLibrary>> m_gate_libraries;

            std::shared_ptr<GateLibrary> load_liberty(const std::filesystem::path& path)
            {
                std::shared_ptr<GateLibrary> lib = nullptr;

                std::ifstream file(path.string().c_str());

                if (file)
                {
                    std::stringstream buffer;

                    buffer << file.rdbuf();

                    GateLibraryParserLiberty parser(path, buffer);
                    lib = parser.parse();

                    file.close();

                    if (lib == nullptr)
                    {
                        log_error("gate_library_manager", "failed to load gate library '{}'.", path.string());
                    }
                }
                else
                {
                    log_error("gate_library_manager", "could not open gate library file '{}'", path.string());
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
    }    // namespace gate_library_manager
}    // namespace hal
