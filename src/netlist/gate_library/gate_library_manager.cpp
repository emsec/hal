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

        std::vector<std::string> json_to_vector(const rapidjson::Value& val)
        {
            auto array = val.GetArray();
            std::vector<std::string> res;
            res.reserve(array.Size());
            for (const auto& x : array)
            {
                res.push_back(x.GetString());
            }
            return res;
        }

        void insert_json_array(std::vector<std::string>* vec, const rapidjson::Value& val)
        {
            auto array = json_to_vector(val);
            vec->insert(vec->end(), array.begin(), array.end());
        }

        void insert_json_array(std::set<std::string>* set, const rapidjson::Value& val)
        {
            for (const auto& x : json_to_vector(val))
            {
                set->insert(x);
            }
        }

        std::shared_ptr<gate_library> deserialize(const rapidjson::Document& document)
        {
            if (!document.HasMember("library"))
            {
                log_error("netlist", "gate library has to be defined in the main 'library' json node");
                return nullptr;
            }

            auto& library_node = document["library"];

            if (!library_node.HasMember("library_name"))
            {
                log_error("netlist", "gate library name has to be defined in a 'library_name' json child node");
                return nullptr;
            }

            std::shared_ptr<gate_library> lib = std::make_shared<gate_library>(core_utils::trim(library_node["library_name"].GetString()));

            if (library_node.HasMember("vhdl_includes"))
            {
                insert_json_array(lib->get_vhdl_includes(), library_node["vhdl_includes"]);
            }
            if (library_node.HasMember("global_gnd_nodes"))
            {
                insert_json_array(lib->get_global_gnd_gate_types(), library_node["global_gnd_nodes"]);
            }
            if (library_node.HasMember("global_vcc_nodes"))
            {
                insert_json_array(lib->get_global_vcc_gate_types(), library_node["global_vcc_nodes"]);
            }

            if (lib->get_name().empty())
            {
                log_error("netlist", "gate library name is empty");
                return nullptr;
            }
            if (lib->get_global_gnd_gate_types()->empty())
            {
                log_error("netlist", "gate library does not specify global gnd gates");
                return nullptr;
            }
            if (lib->get_global_vcc_gate_types()->empty())
            {
                log_error("netlist", "gate library does not specify global vcc gates");
                return nullptr;
            }

            // deserialize joint style: elements
            if (library_node.HasMember("elements"))
            {
                for (const auto& element : library_node["elements"].GetObject())
                {
                    std::string name = element.name.GetString();
                    lib->get_gate_types()->insert(name);
                    {
                        auto& map         = *lib->get_gate_type_map_to_input_pin_types();
                        auto new_elements = json_to_vector(element.value.GetArray()[0]);
                        for (const auto& x : new_elements)
                        {
                            lib->get_input_pin_types()->insert(x);
                        }
                        map[name].insert(map[name].end(), new_elements.begin(), new_elements.end());
                    }
                    {
                        auto& map         = *lib->get_gate_type_map_to_inout_pin_types();
                        auto new_elements = json_to_vector(element.value.GetArray()[1]);
                        for (const auto& x : new_elements)
                        {
                            lib->get_inout_pin_types()->insert(x);
                        }
                        map[name].insert(map[name].end(), new_elements.begin(), new_elements.end());
                    }
                    {
                        auto& map         = *lib->get_gate_type_map_to_output_pin_types();
                        auto new_elements = json_to_vector(element.value.GetArray()[2]);
                        for (const auto& x : new_elements)
                        {
                            lib->get_output_pin_types()->insert(x);
                        }
                        map[name].insert(map[name].end(), new_elements.begin(), new_elements.end());
                    }
                }
            }

            // deserialize split style: element_types, elements_input_types, elements_output_types, elements_inout_types
            if (library_node.HasMember("element_types"))
            {
                insert_json_array(lib->get_gate_types(), library_node["element_types"]);
                if (library_node.HasMember("elements_input_types"))
                {
                    auto& map = *lib->get_gate_type_map_to_input_pin_types();
                    for (const auto& key_element : library_node["elements_input_types"].GetObject())
                    {
                        auto new_elements = json_to_vector(key_element.value);
                        for (const auto& x : new_elements)
                        {
                            lib->get_input_pin_types()->insert(x);
                        }
                        map[key_element.name.GetString()].insert(map[key_element.name.GetString()].end(), new_elements.begin(), new_elements.end());
                    }
                }
                if (library_node.HasMember("elements_output_types"))
                {
                    auto& map = *lib->get_gate_type_map_to_output_pin_types();
                    for (const auto& key_element : library_node["elements_output_types"].GetObject())
                    {
                        auto new_elements = json_to_vector(key_element.value);
                        for (const auto& x : new_elements)
                        {
                            lib->get_output_pin_types()->insert(x);
                        }
                        map[key_element.name.GetString()].insert(map[key_element.name.GetString()].end(), new_elements.begin(), new_elements.end());
                    }
                }
                if (library_node.HasMember("elements_inout_types"))
                {
                    auto& map = *lib->get_gate_type_map_to_inout_pin_types();
                    for (const auto& key_element : library_node["elements_inout_types"].GetObject())
                    {
                        auto new_elements = json_to_vector(key_element.value);
                        for (const auto& x : new_elements)
                        {
                            lib->get_inout_pin_types()->insert(x);
                        }
                        map[key_element.name.GetString()].insert(map[key_element.name.GetString()].end(), new_elements.begin(), new_elements.end());
                    }
                }
            }

            return lib;
        }

        std::shared_ptr<gate_library> load_json(const hal::path& path)
        {
            auto begin_time                   = std::chrono::high_resolution_clock::now();
            std::shared_ptr<gate_library> lib = nullptr;

            FILE* pFile = fopen(path.string().c_str(), "rb");

            if (pFile != NULL)
            {
                char buffer[65536];
                rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
                rapidjson::Document document;
                document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);

                lib = deserialize(document);

                fclose(pFile);

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
            hal::path path_json    = core_utils::get_file(name + ".json", core_utils::get_gate_library_directories());
            hal::path path_liberty = core_utils::get_file(name + ".lib", core_utils::get_gate_library_directories());

            if (!path_json.empty())
            {
                lib = load_json(path_json);
            }
            else if (!path_liberty.empty())
            {
                lib = load_liberty(path_liberty);
            }
            else
            {
                log_error("netlist", "could not find gate library file '{}' or '{}'.", name + ".json", name + ".lib");
                return nullptr;
            }

            if (lib != nullptr)
            {
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
                if (core_utils::ends_with(lib_path.path().string(), ".json"))
                {
                    auto lib = load_json(lib_path.path());

                    if (lib != nullptr)
                    {
                        m_gate_libraries[lib->get_name()] = lib;
                    }
                }
                else if (core_utils::ends_with(lib_path.path().string(), ".lib"))
                {
                    auto lib = load_liberty(lib_path.path());

                    if (lib != nullptr)
                    {
                        m_gate_libraries[lib->get_name()] = lib;
                    }
                }
            }
        }
    }

    std::map<std::string, std::shared_ptr<gate_library>> get_gate_libraries()
    {
        return m_gate_libraries;
    }
}    // namespace gate_library_manager
