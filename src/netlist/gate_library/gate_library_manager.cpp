#include "netlist/gate_library/gate_library_manager.h"

#include "core/log.h"
#include "core/utils.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_library_liberty_parser.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

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

                lib = gate_library_liberty_parser::parse(buffer);

                file.close();

                if (lib == nullptr)
                {
                    log_error("netlist", "failed to load gate library '{}'.", path.string());
                }
                else
                {
                    log_info("netlist",
                             "loaded gate library '{}' in {:2.2f} seconds.",
                             lib->get_name(),
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
                lib = load_liberty(path_liberty);
            }
            else
            {
                log_error("netlist", "could not find gate library file '{}'.", name + ".lib");
                return nullptr;
            }

            if (lib != nullptr)
            {
                if (is_duplicate(lib))
                {
                    log_error("netlist", "a gate library with the name '{}' already exists, discarding current one.", lib->get_name());
                    return nullptr;
                }

                m_gate_libraries[name] = lib;
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
                std::shared_ptr<gate_library> lib;

                if (core_utils::ends_with(lib_path.path().string(), ".lib"))
                {
                    lib = load_liberty(lib_path.path());
                }

                if (lib != nullptr)
                {
                    if (is_duplicate(lib))
                    {
                        log_error("netlist", "a gate library with the name '{}' already exists, discarding current one.", lib->get_name());
                        continue;
                    }

                    m_gate_libraries[lib->get_name()] = lib;
                }
            }
        }
    }

    std::map<std::string, std::shared_ptr<gate_library>> get_gate_libraries()
    {
        return m_gate_libraries;
    }
}    // namespace gate_library_manager
