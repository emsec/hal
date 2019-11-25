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
        std::map<std::string, std::shared_ptr<gate_library>> m_gate_libraries;

        std::shared_ptr<gate_library> load_liberty(const hal::path& path)
        {
            auto begin_time                   = std::chrono::high_resolution_clock::now();
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
                    log_error("netlist", "failed to load gate library '{}'.", path.string());
                }
                else
                {
                    log_info("netlist",
                             "loaded gate library '{}' from '{}' in {:2.2f} seconds.",
                             lib->get_name(),
                             path.string().substr(path.string().find_last_of("/") + 1),
                             (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);
                }
            }
            else
            {
                log_error("netlist", "could not open gate library file '{}'", path.string());
            }

            return lib;
        }

        bool is_duplicate(const std::shared_ptr<gate_library> lib)
        {
            for (const auto& it : m_gate_libraries)
            {
                if (it.first == lib->get_name())
                {
                    return true;
                }
            }

            return false;
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
                    log_error("netlist", "no GND gate found in parsed library but gate types 'GND' and '{}' already exist.", name);
                    return false;
                }
                auto gt = std::make_shared<gate_type>(name);
                gt->add_output_pin("O");
                gt->add_boolean_function("O", boolean_function::ZERO);
                lib->add_gate_type(gt);
                log_info("netlist", "gate library did not contain a GND gate, auto-generated type '{}'", name);
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
                    log_error("netlist", "no VCC gate found in parsed library but gate types 'VCC' and '{}' already exist.", name);
                    return false;
                }
                auto gt = std::make_shared<gate_type>(name);
                gt->add_output_pin("O");
                gt->add_boolean_function("O", boolean_function::ONE);
                lib->add_gate_type(gt);
                log_info("netlist", "gate library did not contain a VCC gate, auto-generated type '{}'", name);
            }

            return true;
        }

        std::shared_ptr<gate_library> load(const hal::path& path)
        {
            std::shared_ptr<gate_library> lib;

            if (core_utils::ends_with(path.string(), ".lib"))
            {
                lib = load_liberty(path);
            }
            else
            {
                log_error("netlist", "no gate library parser found for '{}'.", path.string());
            }

            if (lib == nullptr || is_duplicate(lib) || !prepare_library(lib))
            {
                return nullptr;
            }

            m_gate_libraries[lib->get_name()] = lib;

            return lib;
        }
    }    // namespace

    std::shared_ptr<gate_library> get_gate_library(const std::string& name)
    {
        auto it = m_gate_libraries.find(name);

        if (it != m_gate_libraries.end())
        {
            return it->second;
        }
        else
        {
            std::shared_ptr<gate_library> lib;
            hal::path path_liberty = core_utils::get_file(name + ".lib", core_utils::get_gate_library_directories());

            if (!path_liberty.empty())
            {
                lib = load(path_liberty);
            }
            else
            {
                log_error("netlist", "could not find gate library file '{}'.", name + ".lib");
                return nullptr;
            }

            return lib;
        }
    }

    void load_all()
    {
        std::vector<hal::path> lib_dirs = core_utils::get_gate_library_directories();

        for (const auto& lib_dir : lib_dirs)
        {
            if (!hal::fs::exists(lib_dir))
            {
                continue;
            }

            log_info("netlist", "Reading all definitions from {}.", lib_dir.string());

            for (const auto& lib_path : core_utils::recursive_directory_range(lib_dir))
            {
                load(lib_path.path());
            }
        }
    }

    std::map<std::string, std::shared_ptr<gate_library>> get_gate_libraries()
    {
        return m_gate_libraries;
    }
}    // namespace gate_library_manager
