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
            /** The INIT size of a LUT is 2^n bits for n inputs and has to fit the 32 bit size of a LUT output configuration. */
            const size_t MAX_LUT_INPUTS = 31;

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

            bool has_legacy_lut_config(const rapidjson::Value& cell)
            {
                const rapidjson::Value* lut_config = find_member(cell, "lut_config");
                return lut_config != nullptr && lut_config->IsObject() && !lut_config->HasMember("output_pins");
            }

            /**
             * Replace the flat LUT configuration by an `output_pins` array.
             *
             * The flat configuration made every LUT output pin depend on all input pins of the cell and on the whole INIT
             * value, so each output pin gets exactly that, which keeps the behavior of the cell unchanged.
             *
             * @returns An error message if the configuration cannot be migrated, an empty string otherwise.
             */
            std::string upgrade_lut_config(rapidjson::Value& cell, const std::string& cell_name, rapidjson::Document::AllocatorType& allocator, std::vector<std::string>& warnings)
            {
                if (!has_legacy_lut_config(cell))
                {
                    // already in the new format or nothing to upgrade
                    return "";
                }

                rapidjson::Value& lut_config = cell["lut_config"];
                const rapidjson::Value* data_identifier = find_member(lut_config, "data_identifier");
                if (data_identifier == nullptr || !data_identifier->IsString())
                {
                    return "cell '" + cell_name + "': LUT configuration has no data identifier";
                }

                std::vector<std::string> input_pins;
                std::vector<std::string> lut_output_pins;
                collect_pins(cell, input_pins, lut_output_pins);

                if (lut_output_pins.empty())
                {
                    return "cell '" + cell_name + "': no output pin with type 'lut' found";
                }

                if (input_pins.empty())
                {
                    return "cell '" + cell_name + "': no input pins found";
                }

                if (input_pins.size() > MAX_LUT_INPUTS)
                {
                    return "cell '" + cell_name + "': " + std::to_string(input_pins.size()) + " input pins exceed the supported maximum of " + std::to_string(MAX_LUT_INPUTS) + " for a LUT";
                }

                if (lut_output_pins.size() > 1)
                {
                    warnings.push_back("cell '" + cell_name + "': each of its " + std::to_string(lut_output_pins.size())
                                       + " LUT output pins was migrated to depend on all input pins and the whole INIT value, as before. If an output uses only part of "
                                         "them, e.g., output O5 of a Xilinx LUT6_2, adjust its 'input_pins' and 'bit_count' manually.");
                }

                const rapidjson::Value* bit_order = find_member(lut_config, "bit_order");
                const bool is_ascending           = (bit_order == nullptr) || (bit_order->IsString() && std::string(bit_order->GetString()) == "ascending");

                rapidjson::Value output_pins(rapidjson::kArrayType);
                for (const auto& lut_output_pin : lut_output_pins)
                {
                    rapidjson::Value inputs(rapidjson::kArrayType);
                    for (const auto& pin : input_pins)
                    {
                        inputs.PushBack(make_string(pin, allocator), allocator);
                    }

                    rapidjson::Value output_pin(rapidjson::kObjectType);
                    output_pin.AddMember("pin", make_string(lut_output_pin, allocator), allocator);
                    output_pin.AddMember("data_identifier", rapidjson::Value(*data_identifier, allocator), allocator);
                    output_pin.AddMember("bit_offset", 0, allocator);
                    output_pin.AddMember("bit_count", u64(1) << input_pins.size(), allocator);
                    output_pin.AddMember("input_pins", inputs, allocator);
                    output_pin.AddMember("is_ascending", is_ascending, allocator);
                    output_pins.PushBack(output_pin, allocator);
                }

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
                return "";
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

        Result<u32> get_version(const rapidjson::Document& document)
        {
            if (!document.IsObject())
            {
                return ERR("document is not a JSON object");
            }
            if (!document.HasMember("version"))
            {
                return OK(u32(1));
            }
            if (!document["version"].IsUint())
            {
                return ERR("invalid version entry");
            }
            return OK(document["version"].GetUint());
        }

        bool needs_upgrade(const rapidjson::Document& document)
        {
            auto version = get_version(document);
            if (version.is_error())
            {
                return false;
            }
            if (version.get() < TARGET_VERSION)
            {
                return true;
            }
            if (version.get() > TARGET_VERSION || !document.HasMember("cells") || !document["cells"].IsArray())
            {
                return false;
            }

            // earlier upgrades marked files as up to date without migrating all LUT configurations
            const auto& cells = document["cells"].GetArray();
            return std::any_of(cells.begin(), cells.end(), [](const rapidjson::Value& cell) { return has_legacy_lut_config(cell); });
        }

        Result<std::vector<std::string>> upgrade_document(rapidjson::Document& document)
        {
            auto version_res = get_version(document);
            if (version_res.is_error())
            {
                return ERR_APPEND(version_res.get_error(), "could not upgrade HGL document");
            }
            const u32 version = version_res.get();

            std::vector<std::string> warnings;
            if (!needs_upgrade(document))
            {
                return OK(warnings);
            }

            if (version < 2)
            {
                return ERR("could not upgrade HGL document: version " + std::to_string(version) + " uses a legacy pin format that cannot be upgraded automatically");
            }

            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            std::vector<std::string> errors;

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
                    if (std::string error = upgrade_lut_config(cell, cell_name, allocator, warnings); !error.empty())
                    {
                        errors.push_back(error);
                        continue;
                    }
                    add_parameters(cell, allocator);
                }
            }

            if (!errors.empty())
            {
                // a partially upgraded document must not be marked as up to date, the parser would not notice what is missing
                std::string message = "could not upgrade HGL document, the following cells have to be migrated manually: ";
                for (size_t i = 0; i < errors.size(); ++i)
                {
                    message += (i == 0 ? "" : "; ") + errors[i];
                }
                return ERR(message);
            }

            if (document.HasMember("version"))
            {
                document["version"].SetUint(TARGET_VERSION);
            }
            else
            {
                document.AddMember("version", TARGET_VERSION, allocator);
            }

            return OK(warnings);
        }

        std::filesystem::path get_backup_path(const std::filesystem::path& file_path)
        {
            std::filesystem::path backup_path = file_path;
            backup_path.replace_extension(".json");

            // a file repaired after an earlier upgrade already has a backup, which must not be overwritten
            std::error_code ec;
            for (u32 i = 1; std::filesystem::exists(backup_path, ec); ++i)
            {
                backup_path = file_path;
                backup_path.replace_extension("." + std::to_string(i) + ".json");
            }
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
