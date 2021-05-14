#include "hal_core/netlist/gate_library/gate_library_manager.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <iostream>

namespace hal
{
    namespace gate_library_manager
    {
        namespace
        {
            std::map<std::filesystem::path, std::unique_ptr<GateLibrary>> m_gate_libraries;

            bool prepare_library(const std::unique_ptr<GateLibrary>& lib)
            {
                auto gate_types = lib->get_gate_types();

                for (const auto& [gt_name, gt] : gate_types)
                {
                    if (gt->has_property(GateTypeProperty::power))
                    {
                        lib->mark_vcc_gate_type(gt);
                    }
                    else if (gt->has_property(GateTypeProperty::ground))
                    {
                        lib->mark_gnd_gate_type(gt);
                    }
                }

                if (lib->get_gnd_gate_types().empty())
                {
                    std::string name = "HAL_GND";
                    if (gate_types.find(name) != gate_types.end())
                    {
                        log_error("gate_library_manager", "no 'GND' gate type found in gate library, but gate type 'HAL_GND' already exists.");
                        return false;
                    }

                    GateType* gt = lib->create_gate_type(name, {GateTypeProperty::combinational, GateTypeProperty::ground});
                    gt->add_output_pin("O");
                    gt->add_boolean_function("O", BooleanFunction::ZERO);
                    lib->mark_gnd_gate_type(gt);
                    log_info("gate_library_manager", "gate library did not contain a GND gate, auto-generated type '{}'.", name);
                }

                if (lib->get_vcc_gate_types().empty())
                {
                    std::string name = "HAL_VDD";
                    if (gate_types.find(name) != gate_types.end())
                    {
                        log_error("gate_library_manager", "no 'VDD' gate found in gate library, but gate type 'HAL_VDD' already exists.");
                        return false;
                    }

                    GateType* gt = lib->create_gate_type(name, {GateTypeProperty::combinational, GateTypeProperty::power});
                    gt->add_output_pin("O");
                    gt->add_boolean_function("O", BooleanFunction::ONE);
                    lib->mark_vcc_gate_type(gt);
                    log_info("gate_library_manager", "gate library did not contain a VDD gate, auto-generated type '{}'.", name);
                }

                return true;
            }
        }    // namespace

        GateLibrary* load(std::filesystem::path file_path, bool reload)
        {
            if (!std::filesystem::exists(file_path))
            {
                log_error("gate_library_manager", "gate library file '{}' does not exist.", file_path.string());
                return nullptr;
            }

            if (!file_path.is_absolute())
            {
                file_path = std::filesystem::absolute(file_path);
            }

            if (!reload)
            {
                if (auto it = m_gate_libraries.find(file_path); it != m_gate_libraries.end())
                {
                    log_info("gate_library_parser", "the gate library file '{}' is already loaded.", file_path.string());
                    return it->second.get();
                }
            }

            std::unique_ptr<GateLibrary> gate_lib = gate_library_parser_manager::parse(file_path);
            if (gate_lib == nullptr)
            {
                return nullptr;
            }

            if (!prepare_library(gate_lib))
            {
                return nullptr;
            }

            GateLibrary* res                     = gate_lib.get();
            m_gate_libraries[file_path.string()] = std::move(gate_lib);
            return res;
        }

        void load_all(bool reload)
        {
            std::vector<std::filesystem::path> lib_dirs = utils::get_gate_library_directories();

            for (const auto& lib_dir : lib_dirs)
            {
                if (!std::filesystem::exists(lib_dir))
                {
                    continue;
                }

                log_info("gate_library_manager", "loading all gate library files from {}.", lib_dir.string());

                for (const auto& lib_path : utils::RecursiveDirectoryRange(lib_dir))
                {
                    load(lib_path.path(), reload);
                }
            }
        }

        bool save(std::filesystem::path file_path, GateLibrary* gate_lib, bool overwrite)
        {
            if (std::filesystem::exists(file_path))
            {
                if (overwrite)
                {
                    log_info("gate_library_manager", "gate library file '{}' already exists and will be overwritten.", file_path.string());
                }
                else
                {
                    log_error("gate_library_manager", "gate library file '{}' already exists, aborting.", file_path.string());
                    return false;
                }
            }

            if (!file_path.is_absolute())
            {
                file_path = std::filesystem::absolute(file_path);
            }

            return gate_library_writer_manager::write(gate_lib, file_path);
        }

        GateLibrary* get_gate_library(const std::string& file_path)
        {
            std::filesystem::path absolute_path;

            if (std::filesystem::exists(file_path))
            {
                // if an existing file is queried, load it by its absolute path
                absolute_path = std::filesystem::absolute(file_path);
            }
            else
            {
                // if a non existing file is queried, search for it in the standard directories
                auto stripped_name = std::filesystem::path(file_path).filename();
                log_info("gate_library_manager", "file '{}' does not exist, searching for '{}' in the default gate library directories...", file_path, stripped_name.string());

                auto lib_path = utils::get_file(stripped_name, utils::get_gate_library_directories());
                if (lib_path.empty())
                {
                    log_info("gate_library_manager", "could not find gate library file '{}'.", stripped_name.string());
                    return nullptr;
                }
                absolute_path = std::filesystem::absolute(lib_path);
            }

            // absolute path to file is known, check if it is already loaded
            if (auto it = m_gate_libraries.find(absolute_path.string()); it != m_gate_libraries.end())
            {
                return it->second.get();
            }

            // not loaded yet -> load
            return load(absolute_path);
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
    }    // namespace gate_library_manager
}    // namespace hal