#include "hgl_writer/hgl_writer.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/utilities/log.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

#include <iostream>
#include <string>

namespace hal
{
    const std::unordered_map<GateType::BaseType, std::string> HGLWriter::m_base_type_to_string = {{GateType::BaseType::combinational, "combinational"},
                                                                                                  {GateType::BaseType::ff, "ff"},
                                                                                                  {GateType::BaseType::latch, "latch"},
                                                                                                  {GateType::BaseType::lut, "lut"},
                                                                                                  {GateType::BaseType::ram, "ram"},
                                                                                                  {GateType::BaseType::io, "io"},
                                                                                                  {GateType::BaseType::buffer, "buffer"},
                                                                                                  {GateType::BaseType::dsp, "dsp"}};

    const std::unordered_map<GateType::PinType, std::string> HGLWriter::m_pin_type_to_string = {{GateType::PinType::none, "none"},
                                                                                                {GateType::PinType::power, "power"},
                                                                                                {GateType::PinType::ground, "ground"},
                                                                                                {GateType::PinType::lut, "lut"},
                                                                                                {GateType::PinType::state, "state"},
                                                                                                {GateType::PinType::neg_state, "neg_state"},
                                                                                                {GateType::PinType::clock, "clock"},
                                                                                                {GateType::PinType::enable, "enable"},
                                                                                                {GateType::PinType::set, "set"},
                                                                                                {GateType::PinType::reset, "reset"},
                                                                                                {GateType::PinType::data, "data"},
                                                                                                {GateType::PinType::address, "address"}};

    const std::unordered_map<GateType::ClearPresetBehavior, std::string> HGLWriter::m_behavior_to_string = {{GateType::ClearPresetBehavior::L, "L"},
                                                                                                            {GateType::ClearPresetBehavior::H, "H"},
                                                                                                            {GateType::ClearPresetBehavior::N, "N"},
                                                                                                            {GateType::ClearPresetBehavior::T, "T"},
                                                                                                            {GateType::ClearPresetBehavior::X, "X"}};

    bool HGLWriter::write(const GateLibrary* gate_lib, const std::filesystem::path& file_path)
    {
        if (gate_lib == nullptr)
        {
            log_error("hgl_writer", "received a nullptr as gate library.", file_path.string());
            return false;
        }

        FILE* fp = fopen(file_path.string().c_str(), "w");
        if (fp == NULL)
        {
            log_error("hgl_writer", "unable to open '{}' for writing.", file_path.string());
            return false;
        }

        rapidjson::Document document;
        document.SetObject();

        if (!write_gate_library(document, gate_lib))
        {
            fclose(fp);
            return false;
        }

        char writeBuffer[65536];
        rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        document.Accept(writer);

        fclose(fp);

        return true;
    }

    bool HGLWriter::write_gate_library(rapidjson::Document& document, const GateLibrary* gate_lib)
    {
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        // library name
        document.AddMember("library", gate_lib->get_name(), allocator);

        // gate types
        std::vector<GateType*> gate_types;

        for (const auto& gt : gate_lib->get_gate_types())
        {
            if ((gt.first != "HAL_GND") && (gt.first != "HAL_VDD"))
            {
                gate_types.push_back(gt.second);
            }
        }

        std::sort(gate_types.begin(), gate_types.end(), [](GateType* l, GateType* r) { return l->get_id() < r->get_id(); });

        rapidjson::Value cells(rapidjson::kArrayType);
        for (const auto gt : gate_types)
        {
            rapidjson::Value cell(rapidjson::kObjectType);

            // name
            cell.AddMember("name", gt->get_name(), allocator);

            // base type
            GateType::BaseType base_type = gt->get_base_type();
            cell.AddMember("type", m_base_type_to_string.at(base_type), allocator);

            std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();

            // lut_config, ff_config, latch_config
            if (base_type == GateType::BaseType::lut)
            {
                rapidjson::Value lut_config(rapidjson::kObjectType);

                // bit_order
                if (gt->is_lut_init_ascending())
                {
                    lut_config.AddMember("bit_order", "ascending", allocator);
                }
                else
                {
                    lut_config.AddMember("bit_order", "descending", allocator);
                }

                // data_category and data_identifier
                lut_config.AddMember("data_category", gt->get_config_data_category(), allocator);
                lut_config.AddMember("data_identifier", gt->get_config_data_identifier(), allocator);

                cell.AddMember("lut_config", lut_config, allocator);
            }
            else if (base_type == GateType::BaseType::ff)
            {
                rapidjson::Value ff_config(rapidjson::kObjectType);

                // data_category, data_identifier
                ff_config.AddMember("data_category", gt->get_config_data_category(), allocator);
                ff_config.AddMember("data_identifier", gt->get_config_data_identifier(), allocator);

                // next_state, clocked_on, clear_on, preset_on
                if (const auto it = functions.find("next_state"); it != functions.end())
                {
                    ff_config.AddMember("next_state", it->second.to_string(), allocator);
                }
                if (const auto it = functions.find("clock"); it != functions.end())
                {
                    ff_config.AddMember("clocked_on", it->second.to_string(), allocator);
                }
                if (const auto it = functions.find("clear"); it != functions.end())
                {
                    ff_config.AddMember("clear_on", it->second.to_string(), allocator);
                }
                if (const auto it = functions.find("preset"); it != functions.end())
                {
                    ff_config.AddMember("preset_on", it->second.to_string(), allocator);
                }

                std::pair<GateType::ClearPresetBehavior, GateType::ClearPresetBehavior> cp_behav = gt->get_clear_preset_behavior();
                if (cp_behav.first != GateType::ClearPresetBehavior::U)
                {
                    ff_config.AddMember("state_clear_preset", m_behavior_to_string.at(cp_behav.first), allocator);
                }
                if (cp_behav.second != GateType::ClearPresetBehavior::U)
                {
                    ff_config.AddMember("neg_state_clear_preset", m_behavior_to_string.at(cp_behav.second), allocator);
                }

                cell.AddMember("ff_config", ff_config, allocator);
            }
            else if (base_type == GateType::BaseType::latch)
            {
                rapidjson::Value latch_config(rapidjson::kObjectType);

                // next_state, clocked_on, clear_on, preset_on
                if (const auto it = functions.find("data"); it != functions.end())
                {
                    latch_config.AddMember("data_in", it->second.to_string(), allocator);
                }
                if (const auto it = functions.find("enable"); it != functions.end())
                {
                    latch_config.AddMember("enable_on", it->second.to_string(), allocator);
                }
                if (const auto it = functions.find("clear"); it != functions.end())
                {
                    latch_config.AddMember("clear_on", it->second.to_string(), allocator);
                }
                if (const auto it = functions.find("preset"); it != functions.end())
                {
                    latch_config.AddMember("preset_on", it->second.to_string(), allocator);
                }

                std::pair<GateType::ClearPresetBehavior, GateType::ClearPresetBehavior> cp_behav = gt->get_clear_preset_behavior();
                if (cp_behav.first != GateType::ClearPresetBehavior::U)
                {
                    latch_config.AddMember("state_clear_preset", m_behavior_to_string.at(cp_behav.first), allocator);
                }
                if (cp_behav.second != GateType::ClearPresetBehavior::U)
                {
                    latch_config.AddMember("neg_state_clear_preset", m_behavior_to_string.at(cp_behav.second), allocator);
                }

                cell.AddMember("latch_config", latch_config, allocator);
            }

            // pins
            std::vector<PinCtx> pin_data = get_pins(gt, functions);
            rapidjson::Value pins(rapidjson::kArrayType);

            for (const auto& pin_ctx : pin_data)
            {
                rapidjson::Value pin(rapidjson::kObjectType);

                pin.AddMember("name", pin_ctx.name, allocator);
                pin.AddMember("direction", pin_ctx.direction, allocator);
                if (pin_ctx.type != "")
                {
                    pin.AddMember("type", pin_ctx.type, allocator);
                }
                if (pin_ctx.function != "")
                {
                    pin.AddMember("function", pin_ctx.function, allocator);
                }
                if (pin_ctx.x_function != "")
                {
                    pin.AddMember("x_function", pin_ctx.x_function, allocator);
                }
                if (pin_ctx.z_function != "")
                {
                    pin.AddMember("z_function", pin_ctx.z_function, allocator);
                }

                pins.PushBack(pin, allocator);
            }

            cell.AddMember("pins", pins, allocator);

            // groups
            std::vector<GroupCtx> group_data = get_groups(gt);
            if (!group_data.empty())
            {
                rapidjson::Value groups(rapidjson::kArrayType);

                for (const auto& group_ctx : group_data)
                {
                    rapidjson::Value group(rapidjson::kObjectType);

                    group.AddMember("name", group_ctx.name, allocator);

                    rapidjson::Value group_pins(rapidjson::kObjectType);
                    for (const auto& [index, group_pin] : group_ctx.index_to_pin)
                    {
                        std::string s = std::to_string(index);
                        rapidjson::Value v_index(s.c_str(), s.size(), allocator);
                        rapidjson::Value v_pin(group_pin.c_str(), group_pin.size(), allocator);
                        group_pins.AddMember(v_index, v_pin, allocator);
                    }
                    group.AddMember("pins", group_pins, allocator);

                    groups.PushBack(group, allocator);
                }

                cell.AddMember("groups", groups, allocator);
            }

            cells.PushBack(cell, allocator);
        }

        document.AddMember("cells", cells, allocator);

        return true;
    }

    std::vector<HGLWriter::PinCtx> HGLWriter::get_pins(GateType* gt, const std::unordered_map<std::string, BooleanFunction>& functions)
    {
        std::vector<PinCtx> res;
        const std::vector<std::string>& pins                                            = gt->get_pins();
        const std::unordered_map<std::string, GateType::PinDirection>& pin_to_direction = gt->get_pin_directions();
        const std::unordered_map<std::string, GateType::PinType>& pin_to_type           = gt->get_pin_types();

        for (const auto& pin : pins)
        {
            PinCtx res_pin;
            res_pin.name = pin;
            if (pin_to_type.at(pin) != GateType::PinType::none)
            {
                res_pin.type = m_pin_type_to_string.at(pin_to_type.at(pin));
            }
            switch (pin_to_direction.at(pin))
            {
                case GateType::PinDirection::none:
                    continue;
                case GateType::PinDirection::input:
                    res_pin.direction = "input";
                    break;
                case GateType::PinDirection::output:
                    res_pin.direction = "output";
                    break;
                case GateType::PinDirection::inout:
                    res_pin.direction = "inout";
                    break;
                case GateType::PinDirection::internal:
                    res_pin.direction = "internal";
                    break;
            }

            if (pin_to_direction.at(pin) == GateType::PinDirection::output || pin_to_direction.at(pin) == GateType::PinDirection::inout)
            {
                if (const auto it = functions.find(pin); it != functions.end())
                {
                    res_pin.function = it->second.to_string();
                }

                if (const auto it = functions.find(pin + "_undefined"); it != functions.end())
                {
                    res_pin.x_function = it->second.to_string();
                }

                if (const auto it = functions.find(pin + "_tristate"); it != functions.end())
                {
                    res_pin.z_function = it->second.to_string();
                }
            }

            res.push_back(res_pin);
        }

        return res;
    }

    std::vector<HGLWriter::GroupCtx> HGLWriter::get_groups(GateType* gt)
    {
        std::vector<GroupCtx> res;
        std::unordered_map<std::string, std::map<u32, std::string>> groups = gt->get_pin_groups();

        for (const auto& [group, index_to_pin] : groups)
        {
            GroupCtx res_group;
            res_group.name         = group;
            res_group.index_to_pin = index_to_pin;
            res.push_back(res_group);
        }

        return res;
    }
}    // namespace hal
