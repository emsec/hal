#include "netlist/gate_library/gate_library_manager.h"

#include "core/log.h"
#include "core/utils.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_library_parser/gate_library_parser_liberty.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

namespace gate_library_manager
{
    namespace
    {
        std::map<hal::path, std::shared_ptr<gate_library>> m_gate_libraries;

        std::shared_ptr<gate_library> load_liberty(const hal::path& path)
        {
            std::shared_ptr<gate_library> lib = nullptr;

            std::ifstream file(path.string().c_str());

            if (file)
            {
                std::stringstream buffer;

                buffer << file.rdbuf();

                gate_library_parser_liberty parser(buffer);
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

        bool prepare_library(std::shared_ptr<gate_library>& lib)
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
                auto gt = std::make_shared<gate_type>(name);
                gt->add_output_pin("O");
                gt->add_boolean_function("O", boolean_function::ZERO);
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
                auto gt = std::make_shared<gate_type>(name);
                gt->add_output_pin("O");
                gt->add_boolean_function("O", boolean_function::ONE);
                lib->add_gate_type(gt);
                log_info("gate_library_manager", "gate library did not contain a VCC gate, auto-generated type '{}'", name);
            }

            return true;
        }
    }    // namespace

    std::shared_ptr<gate_library> load_file(const hal::path& path, bool reload_if_existing)
    {
        auto file_name = path.string().substr(path.string().find_last_of("/") + 1);
        if (!reload_if_existing)
        {
            auto it = m_gate_libraries.find(path);
            if (it != m_gate_libraries.end())
            {
                log_info("gate_library_manager", "gate library '{}' from '{}' is already loaded.", it->second->get_name(), file_name);
                return it->second;
            }
        }

        std::shared_ptr<gate_library> lib;
        auto begin_time = std::chrono::high_resolution_clock::now();
        if (core_utils::ends_with(path.string(), ".lib"))
        {
            log_info("gate_library_manager", "loading file '{}'...", file_name);
            lib = load_liberty(path);
        }
        else
        {
            log_error("gate_library_manager", "no parser found for '{}'.", file_name);
        }

        if (lib == nullptr)
        {
            return nullptr;
        }

        auto elapsed = (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;
        log_info("gate_library_manager", "loaded '{}' in {:2.2f} seconds.", lib->get_name(), elapsed);

        for (const auto& it : m_gate_libraries)
        {
            if (it.second->get_name() == lib->get_name())
            {
                log_error("gate_library_manager", "a library '{}' was already loaded from '{}'. discarding new library.", lib->get_name(), it.first.string());
                return nullptr;
            }
        }
        if (!prepare_library(lib))
        {
            return nullptr;
        }

        m_gate_libraries[path] = lib;

        return lib;
    }

    void load_all(bool reload_if_existing)
    {
        std::vector<hal::path> lib_dirs = core_utils::get_gate_library_directories();

        for (const auto& lib_dir : lib_dirs)
        {
            if (!hal::fs::exists(lib_dir))
            {
                continue;
            }

            log_info("gate_library_manager", "Reading all definitions from {}.", lib_dir.string());

            for (const auto& lib_path : core_utils::recursive_directory_range(lib_dir))
            {
                load_file(lib_path.path(), reload_if_existing);
            }
        }
    }

    std::shared_ptr<gate_library> get_gate_library(const std::string& name)
    {
        for (const auto& it : m_gate_libraries)
        {
            if (it.second->get_name() == name)
            {
                return it.second;
            }
        }

        // log_info("gate_library_manager", "no gate library '{}' loaded. trying to load '{}.lib' now from default directories...", name, name);
        hal::path path_liberty = core_utils::get_file(name + ".lib", core_utils::get_gate_library_directories());

        if (!path_liberty.empty())
        {
            return load_file(path_liberty);
        }
        else
        {
            log_error("gate_library_manager", "could not find gate library file '{}'.", name + ".lib");
            return nullptr;
        }
    }

    std::vector<std::shared_ptr<gate_library>> get_gate_libraries()
    {
        std::vector<std::shared_ptr<gate_library>> res;
        res.reserve(m_gate_libraries.size());
        for (const auto& it : m_gate_libraries)
        {
            res.push_back(it.second);
        }
        return res;
    }
}    // namespace gate_library_manager
