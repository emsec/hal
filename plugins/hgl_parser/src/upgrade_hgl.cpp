#include "hgl_parser/upgrade_hgl.h"

#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <system_error>
#include <utility>

namespace hal
{
    namespace hgl_upgrade
    {
        namespace
        {
            const std::map<std::string, std::string> LEGACY_TYPE_RENAMES = {{"buffer", "c_buffer"}, {"carry", "c_carry"}, {"mux", "c_mux"}, {"lut", "c_lut"}};

            /** Size of each parameter in the order the identifiers were found, the order is kept for the file. */
            using ParameterSizes = std::vector<std::pair<std::string, u64>>;

            const rapidjson::Value* find_member(const rapidjson::Value& object, const char* name)
            {
                if (!object.IsObject())
                {
                    return nullptr;
                }
                auto it = object.FindMember(name);
                return (it != object.MemberEnd()) ? &it->value : nullptr;
            }

            std::string get_string(const rapidjson::Value& object, const char* name)
            {
                const rapidjson::Value* value = find_member(object, name);
                return (value != nullptr && value->IsString()) ? value->GetString() : "";
            }

            u64 get_uint(const rapidjson::Value& object, const char* name)
            {
                const rapidjson::Value* value = find_member(object, name);
                return (value != nullptr && value->IsUint64()) ? value->GetUint64() : 0;
            }

            rapidjson::Value make_string(const std::string& str, rapidjson::Document::AllocatorType& allocator)
            {
                return rapidjson::Value(str.c_str(), str.length(), allocator);
            }

            void collect_pins(const rapidjson::Value& cell, std::vector<std::string>& input_pins, std::vector<std::string>& lut_output_pins)
            {
                const rapidjson::Value* pin_groups = find_member(cell, "pin_groups");
                if (pin_groups == nullptr || !pin_groups->IsArray())
                {
                    return;
                }

                for (const auto& pin_group : pin_groups->GetArray())
                {
                    const std::string direction = get_string(pin_group, "direction");
                    const rapidjson::Value* pins = find_member(pin_group, "pins");
                    if (pins == nullptr || !pins->IsArray())
                    {
                        continue;
                    }

                    for (const auto& pin : pins->GetArray())
                    {
                        if (direction == "input")
                        {
                            input_pins.push_back(get_string(pin, "name"));
                        }
                        else if (direction == "output" && get_string(pin, "type") == "lut")
                        {
                            lut_output_pins.push_back(get_string(pin, "name"));
                        }
                    }
                }
            }

            void rename_legacy_types(rapidjson::Value& cell, rapidjson::Document::AllocatorType& allocator)
            {
                if (!cell.HasMember("types") || !cell["types"].IsArray())
                {
                    return;
                }

                for (auto& type : cell["types"].GetArray())
                {
                    if (!type.IsString())
                    {
                        continue;
                    }
                    if (auto it = LEGACY_TYPE_RENAMES.find(type.GetString()); it != LEGACY_TYPE_RENAMES.end())
                    {
                        type = make_string(it->second, allocator);
                    }
                }
            }

            void upgrade_lut_config(rapidjson::Value& cell, const std::string& cell_name, rapidjson::Document::AllocatorType& allocator, std::vector<std::string>& warnings)
            {
                rapidjson::Value& lut_config = cell["lut_config"];
                if (!lut_config.IsObject() || lut_config.HasMember("output_pins") || !lut_config.HasMember("data_identifier"))
                {
                    // already in the new format or nothing to upgrade
                    return;
                }

                std::vector<std::string> input_pins;
                std::vector<std::string> lut_output_pins;
                collect_pins(cell, input_pins, lut_output_pins);

                if (lut_output_pins.empty())
                {
                    warnings.push_back("cell '" + cell_name + "': no output pin with type 'lut' found, LUT configuration not migrated, please update manually.");
                    return;
                }

                if (input_pins.empty())
                {
                    warnings.push_back("cell '" + cell_name + "': no input pins found, LUT configuration not migrated, please update manually.");
                    return;
                }

                if (lut_output_pins.size() > 1)
                {
                    // the input list and the INIT bit range of each output cannot be derived from the pins alone
                    warnings.push_back("cell '" + cell_name + "': has " + std::to_string(lut_output_pins.size())
                                       + " LUT output pins, multi-output LUT cells cannot be migrated automatically, please update manually.");
                    return;
                }

                const rapidjson::Value* bit_order = find_member(lut_config, "bit_order");
                const bool is_ascending           = (bit_order == nullptr) || (bit_order->IsString() && std::string(bit_order->GetString()) == "ascending");

                rapidjson::Value inputs(rapidjson::kArrayType);
                for (const auto& pin : input_pins)
                {
                    inputs.PushBack(make_string(pin, allocator), allocator);
                }

                rapidjson::Value output_pin(rapidjson::kObjectType);
                output_pin.AddMember("pin", make_string(lut_output_pins.front(), allocator), allocator);
                output_pin.AddMember("data_identifier", rapidjson::Value(lut_config["data_identifier"], allocator), allocator);
                output_pin.AddMember("bit_offset", 0, allocator);
                output_pin.AddMember("bit_count", u64(1) << input_pins.size(), allocator);
                output_pin.AddMember("input_pins", inputs, allocator);
                output_pin.AddMember("is_ascending", is_ascending, allocator);

                rapidjson::Value output_pins(rapidjson::kArrayType);
                output_pins.PushBack(output_pin, allocator);

                rapidjson::Value new_lut_config(rapidjson::kObjectType);
                if (bit_order != nullptr)
                {
                    new_lut_config.AddMember("bit_order", rapidjson::Value(*bit_order, allocator), allocator);
                }
                if (const rapidjson::Value* data_category = find_member(lut_config, "data_category"); data_category != nullptr)
                {
                    new_lut_config.AddMember("data_category", rapidjson::Value(*data_category, allocator), allocator);
                }
                new_lut_config.AddMember("output_pins", output_pins, allocator);

                // assigning keeps the position of 'lut_config' within the cell
                lut_config = new_lut_config;
            }

            u64* find_size(ParameterSizes& sizes, const std::string& identifier)
            {
                for (auto& [ident, size] : sizes)
                {
                    if (ident == identifier)
                    {
                        return &size;
                    }
                }
                return nullptr;
            }

            void set_size(ParameterSizes& sizes, const std::string& identifier, u64 size)
            {
                if (u64* existing = find_size(sizes, identifier); existing != nullptr)
                {
                    *existing = size;
                }
                else
                {
                    sizes.emplace_back(identifier, size);
                }
            }

            void add_parameters(rapidjson::Value& cell, rapidjson::Document::AllocatorType& allocator)
            {
                if (cell.HasMember("parameters"))
                {
                    return;
                }

                ParameterSizes sizes;

                if (const rapidjson::Value* lut_config = find_member(cell, "lut_config"); lut_config != nullptr)
                {
                    if (const rapidjson::Value* output_pins = find_member(*lut_config, "output_pins"); output_pins != nullptr && output_pins->IsArray())
                    {
                        for (const auto& output_pin : output_pins->GetArray())
                        {
                            const std::string identifier = get_string(output_pin, "data_identifier");
                            const u64 required           = get_uint(output_pin, "bit_offset") + get_uint(output_pin, "bit_count");
                            const u64* existing          = find_size(sizes, identifier);
                            set_size(sizes, identifier, (existing != nullptr) ? std::max(*existing, required) : required);
                        }
                    }
                }

                // flip-flops and latches hold a single bit per identifier
                for (const char* config_name : {"ff_config", "latch_config"})
                {
                    if (const rapidjson::Value* config = find_member(cell, config_name); config != nullptr && config->HasMember("data_identifier"))
                    {
                        const std::string identifier = get_string(*config, "data_identifier");
                        if (find_size(sizes, identifier) == nullptr)
                        {
                            set_size(sizes, identifier, 1);
                        }
                    }
                }

                if (const rapidjson::Value* ram_config = find_member(cell, "ram_config"); ram_config != nullptr)
                {
                    const rapidjson::Value* identifiers = find_member(*ram_config, "data_identifiers");
                    if (identifiers != nullptr && identifiers->IsArray() && !identifiers->Empty() && ram_config->HasMember("bit_size"))
                    {
                        const u64 size = get_uint(*ram_config, "bit_size") / identifiers->Size();
                        for (const auto& identifier : identifiers->GetArray())
                        {
                            set_size(sizes, identifier.IsString() ? identifier.GetString() : "", size);
                        }
                    }
                }

                if (sizes.empty())
                {
                    return;
                }

                rapidjson::Value parameters(rapidjson::kArrayType);
                for (const auto& [identifier, size] : sizes)
                {
                    rapidjson::Value parameter(rapidjson::kObjectType);
                    parameter.AddMember("name", make_string(identifier, allocator), allocator);
                    parameter.AddMember("type", "bit_vector", allocator);
                    parameter.AddMember("size", size, allocator);
                    parameter.AddMember("default", "0x0", allocator);
                    parameters.PushBack(parameter, allocator);
                }
                cell.AddMember("parameters", parameters, allocator);
            }
        }    // namespace

        Result<std::vector<std::string>> upgrade_document(rapidjson::Document& document)
        {
            if (!document.IsObject())
            {
                return ERR("could not upgrade HGL document: document is not a JSON object");
            }

            std::vector<std::string> warnings;

            u32 version = 1;
            if (document.HasMember("version"))
            {
                if (!document["version"].IsUint())
                {
                    return ERR("could not upgrade HGL document: invalid version entry");
                }
                version = document["version"].GetUint();
            }

            if (version >= TARGET_VERSION)
            {
                return OK(warnings);
            }

            if (version < 2)
            {
                return ERR("could not upgrade HGL document: version " + std::to_string(version) + " uses a legacy pin format that cannot be upgraded automatically");
            }

            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            document["version"].SetUint(TARGET_VERSION);

            if (document.HasMember("cells") && document["cells"].IsArray())
            {
                for (auto& cell : document["cells"].GetArray())
                {
                    if (!cell.IsObject())
                    {
                        continue;
                    }

                    const std::string cell_name = cell.HasMember("name") && cell["name"].IsString() ? cell["name"].GetString() : "(unknown)";

                    rename_legacy_types(cell, allocator);
                    if (cell.HasMember("lut_config"))
                    {
                        upgrade_lut_config(cell, cell_name, allocator, warnings);
                    }
                    add_parameters(cell, allocator);
                }
            }

            return OK(warnings);
        }

        std::filesystem::path get_backup_path(const std::filesystem::path& file_path)
        {
            std::filesystem::path backup_path = file_path;
            backup_path.replace_extension(".json");
            return backup_path;
        }

        Result<std::filesystem::path> replace_file(const rapidjson::Document& document, const std::filesystem::path& file_path)
        {
            const std::filesystem::path backup_path = get_backup_path(file_path);

            std::error_code ec;
            if (std::filesystem::exists(backup_path, ec))
            {
                return ERR("could not upgrade HGL file '" + file_path.string() + "': cannot keep the original as '" + backup_path.string() + "', that file already exists");
            }

            std::filesystem::rename(file_path, backup_path, ec);
            if (ec)
            {
                return ERR("could not upgrade HGL file '" + file_path.string() + "': unable to rename it to '" + backup_path.string() + "': " + ec.message());
            }

            FILE* fp = fopen(file_path.string().c_str(), "w");
            bool written = false;
            if (fp != NULL)
            {
                char buffer[65536];
                rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
                rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
                written = document.Accept(writer);
                os.Flush();
                written = (fclose(fp) == 0) && written;
            }

            if (!written)
            {
                // put the original file back, so that nothing changed for the user
                std::filesystem::remove(file_path, ec);
                std::filesystem::rename(backup_path, file_path, ec);
                return ERR("could not upgrade HGL file '" + file_path.string() + "': unable to write the upgraded file");
            }

            return OK(backup_path);
        }
    }    // namespace hgl_upgrade
}    // namespace hal
