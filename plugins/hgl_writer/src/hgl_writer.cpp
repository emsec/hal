#include "hgl_writer/hgl_writer.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_lut.h"
#include "hal_core/utilities/log.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

#include <iostream>
#include <string>

namespace hal
{
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

    std::unordered_map<GateTypeSequential::ClearPresetBehavior, std::string> HGLWriter::m_behavior_to_string = {{GateTypeSequential::ClearPresetBehavior::L, "L"},
                                                                                                                {GateTypeSequential::ClearPresetBehavior::H, "H"},
                                                                                                                {GateTypeSequential::ClearPresetBehavior::N, "N"},
                                                                                                                {GateTypeSequential::ClearPresetBehavior::T, "T"},
                                                                                                                {GateTypeSequential::ClearPresetBehavior::X, "X"}};

    bool HGLWriter::write_gate_library(rapidjson::Document& document, const GateLibrary* gate_lib)
    {
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        // library name
        document.AddMember("library", gate_lib->get_name(), allocator);

        // gate types
        rapidjson::Value cells(rapidjson::kArrayType);
        for (const auto& [gt_name, gt] : gate_lib->get_gate_types())
        {
            rapidjson::Value cell(rapidjson::kObjectType);

            // name
            cell.AddMember("name", gt_name, allocator);

            // base type
            GateType::BaseType gt_type = gt->get_base_type();
            switch (gt_type)
            {
                case GateType::BaseType::combinational:
                    cell.AddMember("type", "combinational", allocator);
                    break;
                case GateType::BaseType::ff:
                    cell.AddMember("type", "ff", allocator);
                    break;
                case GateType::BaseType::latch:
                    cell.AddMember("type", "latch", allocator);
                    break;
                case GateType::BaseType::lut:
                    cell.AddMember("type", "lut", allocator);
                    break;
            }

            std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();

            // lut_config, ff_config, latch_config
            GateType::BaseType base_type = gt->get_base_type();
            if (base_type == GateType::BaseType::lut)
            {
                rapidjson::Value lut_config(rapidjson::kObjectType);
                const GateTypeLut* gt_lut = static_cast<const GateTypeLut*>(gt);

                // bit_order
                if (gt_lut->is_config_data_ascending_order())
                {
                    lut_config.AddMember("bit_order", "ascending", allocator);
                }
                else
                {
                    lut_config.AddMember("bit_order", "descending", allocator);
                }

                // data_category and data_identifier
                lut_config.AddMember("data_category", gt_lut->get_config_data_category(), allocator);
                lut_config.AddMember("data_identifier", gt_lut->get_config_data_identifier(), allocator);

                cell.AddMember("lut_config", lut_config, allocator);
            }
            else if (base_type == GateType::BaseType::ff)
            {
                rapidjson::Value ff_config(rapidjson::kObjectType);
                const GateTypeSequential* gt_ff = static_cast<const GateTypeSequential*>(gt);

                // data_category, data_identifier
                ff_config.AddMember("data_category", gt_ff->get_init_data_category(), allocator);
                ff_config.AddMember("data_identifier", gt_ff->get_init_data_identifier(), allocator);

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

                std::pair<GateTypeSequential::ClearPresetBehavior, GateTypeSequential::ClearPresetBehavior> cp_behav = gt_ff->get_clear_preset_behavior();
                if (cp_behav.first != GateTypeSequential::ClearPresetBehavior::U)
                {
                    ff_config.AddMember("state_clear_preset", m_behavior_to_string[cp_behav.first], allocator);
                }
                if (cp_behav.second != GateTypeSequential::ClearPresetBehavior::U)
                {
                    ff_config.AddMember("neg_state_clear_preset", m_behavior_to_string[cp_behav.second], allocator);
                }

                cell.AddMember("ff_config", ff_config, allocator);
            }
            else if (base_type == GateType::BaseType::latch)
            {
                rapidjson::Value latch_config(rapidjson::kObjectType);
                const GateTypeSequential* gt_latch = static_cast<const GateTypeSequential*>(gt);

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

                std::pair<GateTypeSequential::ClearPresetBehavior, GateTypeSequential::ClearPresetBehavior> cp_behav = gt_latch->get_clear_preset_behavior();
                if (cp_behav.first != GateTypeSequential::ClearPresetBehavior::U)
                {
                    latch_config.AddMember("state_clear_preset", m_behavior_to_string[cp_behav.first], allocator);
                }
                if (cp_behav.second != GateTypeSequential::ClearPresetBehavior::U)
                {
                    latch_config.AddMember("neg_state_clear_preset", m_behavior_to_string[cp_behav.second], allocator);
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
                if (pin_ctx.function != "")
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
                    group.AddMember("direction", group_ctx.direction, allocator);

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
        std::vector<std::string> input_pins  = gt->get_input_pins();
        std::vector<std::string> output_pins = gt->get_output_pins();

        std::unordered_map<std::string, std::unordered_set<std::string>> type_to_pins;
        type_to_pins["power"]  = gt->get_power_pins();
        type_to_pins["ground"] = gt->get_ground_pins();

        GateType::BaseType base_type = gt->get_base_type();

        if (base_type == GateType::BaseType::ff || base_type == GateType::BaseType::latch)
        {
            const GateTypeSequential* gt_seq = static_cast<const GateTypeSequential*>(gt);

            type_to_pins["state"]     = gt_seq->get_state_pins();
            type_to_pins["neg_state"] = gt_seq->get_negated_state_pins();
            type_to_pins["clock"]     = gt_seq->get_clock_pins();
            type_to_pins["enable"]    = gt_seq->get_enable_pins();
            type_to_pins["reset"]     = gt_seq->get_reset_pins();
            type_to_pins["set"]       = gt_seq->get_set_pins();
            type_to_pins["data"]      = gt_seq->get_data_pins();
        }
        else if (base_type == GateType::BaseType::lut)
        {
            const GateTypeLut* gt_lut = static_cast<const GateTypeLut*>(gt);
            type_to_pins["lut"]       = gt_lut->get_lut_pins();
        }

        for (const auto& in_pin : input_pins)
        {
            PinCtx res_pin;
            res_pin.name      = in_pin;
            res_pin.direction = "input";
            res.push_back(res_pin);
        }

        for (const auto& out_pin : output_pins)
        {
            PinCtx res_pin;

            if (auto it = std::find_if(res.begin(), res.end(), [out_pin](PinCtx p) { return p.name == out_pin; }); it == res.end())
            {
                res_pin.name      = out_pin;
                res_pin.direction = "output";
                res.push_back(res_pin);
            }
            else
            {
                it->direction = "inout";
            }

            if (const auto it = functions.find(out_pin); it != functions.end())
            {
                res_pin.function = it->second.to_string();
            }

            if (const auto it = functions.find(out_pin + "_undefined"); it != functions.end())
            {
                res_pin.x_function = it->second.to_string();
            }

            if (const auto it = functions.find(out_pin + "_tristate"); it != functions.end())
            {
                res_pin.z_function = it->second.to_string();
            }
        }

        for (auto& res_pin : res)
        {
            for (const auto& [pin_type, pins] : type_to_pins)
            {
                if (pins.find(res_pin.name) != pins.end())
                {
                    res_pin.type = pin_type;
                    break;
                }
            }
        }

        return res;
    }

    std::vector<HGLWriter::GroupCtx> HGLWriter::get_groups(GateType* gt)
    {
        std::vector<GroupCtx> res;
        std::unordered_map<std::string, std::map<u32, std::string>> input_groups  = gt->get_input_pin_groups();
        std::unordered_map<std::string, std::map<u32, std::string>> output_groups = gt->get_output_pin_groups();

        for (const auto& [in_group, index_to_pin] : input_groups)
        {
            GroupCtx res_group;
            res_group.name         = in_group;
            res_group.direction    = "input";
            res_group.index_to_pin = index_to_pin;
            res.push_back(res_group);
        }

        for (const auto& [out_group, index_to_pin] : output_groups)
        {
            if (auto it = std::find_if(res.begin(), res.end(), [out_group](GroupCtx g) { return g.name == out_group; }); it == res.end())
            {
                GroupCtx res_group;

                res_group.name         = out_group;
                res_group.direction    = "output";
                res_group.index_to_pin = index_to_pin;
                res.push_back(res_group);
            }
            else
            {
                it->direction = "inout";
            }
        }

        return res;
    }
}    // namespace hal
