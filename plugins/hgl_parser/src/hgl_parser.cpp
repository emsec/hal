#include "hgl_parser/hgl_parser.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/log.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

namespace hal
{
    const std::unordered_map<std::string, GateType::BaseType> HGLParser::m_string_to_base_type = {{"combinational", GateType::BaseType::combinational},
                                                                                                  {"ff", GateType::BaseType::ff},
                                                                                                  {"latch", GateType::BaseType::latch},
                                                                                                  {"lut", GateType::BaseType::lut},
                                                                                                  {"ram", GateType::BaseType::ram},
                                                                                                  {"io", GateType::BaseType::io}};

    const std::unordered_map<std::string, GateType::PinType> HGLParser::m_string_to_pin_type = {{"none", GateType::PinType::none},
                                                                                                {"power", GateType::PinType::power},
                                                                                                {"ground", GateType::PinType::ground},
                                                                                                {"lut", GateType::PinType::lut},
                                                                                                {"state", GateType::PinType::state},
                                                                                                {"neg_state", GateType::PinType::neg_state},
                                                                                                {"clock", GateType::PinType::clock},
                                                                                                {"enable", GateType::PinType::enable},
                                                                                                {"set", GateType::PinType::set},
                                                                                                {"reset", GateType::PinType::reset},
                                                                                                {"data", GateType::PinType::data},
                                                                                                {"address", GateType::PinType::address},
                                                                                                {"io_pad", GateType::PinType::io_pad}};

    const std::unordered_map<std::string, GateType::ClearPresetBehavior> HGLParser::m_string_to_behavior = {{"L", GateType::ClearPresetBehavior::L},
                                                                                                            {"H", GateType::ClearPresetBehavior::H},
                                                                                                            {"N", GateType::ClearPresetBehavior::N},
                                                                                                            {"T", GateType::ClearPresetBehavior::T},
                                                                                                            {"X", GateType::ClearPresetBehavior::X}};

    std::unique_ptr<GateLibrary> HGLParser::parse(const std::filesystem::path& file_path)
    {
        m_path = file_path;

        FILE* fp = fopen(file_path.string().c_str(), "r");
        if (fp == NULL)
        {
            log_error("hgl_parser", "unable to open '{}' for reading.", file_path.string());
            return nullptr;
        }

        char buffer[65536];
        rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);
        fclose(fp);

        if (document.HasParseError())
        {
            log_error("hgl_parser", "encountered parsing error while reading '{}'.", file_path.string());
            return nullptr;
        }

        if (!parse_gate_library(document))
        {
            return nullptr;
        }

        return std::move(m_gate_lib);
    }

    bool HGLParser::parse_gate_library(const rapidjson::Document& document)
    {
        if (!document.HasMember("library"))
        {
            log_error("hgl_parser", "file does not include 'library' node.");
            return false;
        }

        m_gate_lib = std::make_unique<GateLibrary>(m_path, document["library"].GetString());

        if (!document.HasMember("cells"))
        {
            log_error("hgl_parser", "file does not include 'cells' node.");
            return false;
        }

        for (const auto& gate_type : document["cells"].GetArray())
        {
            if (!parse_gate_type(gate_type))
            {
                return false;
            }
        }

        return true;
    }

    bool HGLParser::parse_gate_type(const rapidjson::Value& gate_type)
    {
        std::string name;
        GateType::BaseType type;
        PinCtx pin_ctx;
        GroupCtx group_ctx;

        if (!gate_type.HasMember("name") || !gate_type["name"].IsString())
        {
            log_error("hgl_parser", "invalid name for at least one gate type.");
            return false;
        }
        name = gate_type["name"].GetString();

        if (gate_type.HasMember("type") && gate_type["type"].IsString())
        {
            std::string type_str = gate_type["type"].GetString();
            if (const auto it = m_string_to_base_type.find(type_str); it != m_string_to_base_type.end())
            {
                type = it->second;
            }
            else
            {
                log_error("hgl_parser", "invalid base type '{}' given for gate type '{}'.", type_str, name);
                return false;
            }
        }
        else
        {
            type = GateType::BaseType::combinational;
        }

        GateType* gt = m_gate_lib->create_gate_type(name, type);

        if (gate_type.HasMember("pins") && gate_type["pins"].IsArray())
        {
            for (const auto& pin : gate_type["pins"].GetArray())
            {
                if (!parse_pin(pin_ctx, pin, name))
                {
                    return false;
                }
            }
        }

        gt->add_input_pins(pin_ctx.input_pins);
        gt->add_output_pins(pin_ctx.output_pins);

        for (const auto& [pin, pin_type] : pin_ctx.pin_to_type)
        {
            gt->assign_pin_type(pin, pin_type);
        }

        if (gate_type.HasMember("groups") && gate_type["groups"].IsArray())
        {
            for (const auto& group : gate_type["groups"].GetArray())
            {
                if (!parse_group(group_ctx, group, name))
                {
                    return false;
                }
            }
        }

        gt->assign_input_pin_groups(group_ctx.input_groups);
        gt->assign_output_pin_groups(group_ctx.output_groups);

        if (type == GateType::BaseType::lut)
        {
            GateType* gt_lut = gt;

            if (!gate_type.HasMember("lut_config") || !gate_type["lut_config"].IsObject())
            {
                log_error("hgl_parser", "invalid or missing LUT config for gate type '{}'.", name);
                return false;
            }

            if (!parse_lut_config(gt_lut, gate_type["lut_config"]))
            {
                return false;
            }
        }
        else if (type == GateType::BaseType::ff)
        {
            GateType* gt_ff = gt;

            if (!gate_type.HasMember("ff_config") || !gate_type["ff_config"].IsObject())
            {
                log_error("hgl_parser", "invalid or missing flip-flop config for gate type '{}'.", name);
                return false;
            }

            if (!parse_ff_config(gt_ff, gate_type["ff_config"]))
            {
                return false;
            }
        }
        else if (type == GateType::BaseType::latch)
        {
            GateType* gt_latch = gt;

            if (!gate_type.HasMember("latch_config") || !gate_type["latch_config"].IsObject())
            {
                log_error("hgl_parser", "invalid or missing latch config for gate type '{}'.", name);
                return false;
            }

            if (!parse_latch_config(gt_latch, gate_type["latch_config"]))
            {
                return false;
            }
        }

        for (const auto& [f_name, func] : pin_ctx.boolean_functions)
        {
            gt->add_boolean_function(f_name, BooleanFunction::from_string(func, pin_ctx.input_pins));
        }

        return true;
    }

    bool HGLParser::parse_pin(PinCtx& pin_ctx, const rapidjson::Value& pin, const std::string& gt_name)
    {
        if (!pin.HasMember("name") || !pin["name"].IsString())
        {
            log_error("hgl_parser", "invalid name for at least one pin of gate type '{}'.", gt_name);
            return false;
        }

        std::string name = pin["name"].GetString();

        if (!pin.HasMember("direction") || !pin["direction"].IsString())
        {
            log_error("hgl_parser", "invalid direction for pin '{}' of gate type '{}'.", name, gt_name);
            return false;
        }

        std::string direction = pin["direction"].GetString();
        if (direction == "input")
        {
            pin_ctx.input_pins.push_back(name);
        }
        else if (direction == "output")
        {
            pin_ctx.output_pins.push_back(name);
        }
        else if (direction == "inout")
        {
            pin_ctx.input_pins.push_back(name);
            pin_ctx.output_pins.push_back(name);
        }
        else
        {
            log_warning("hgl_parser", "invalid direction '{}' given for pin '{}' of gate type '{}'.", direction, name, gt_name);
            return false;
        }

        if (pin.HasMember("function") && pin["function"].IsString())
        {
            pin_ctx.boolean_functions[name] = pin["function"].GetString();
        }

        if (pin.HasMember("x_function") && pin["x_function"].IsString())
        {
            pin_ctx.boolean_functions[name + "_undefined"] = pin["x_function"].GetString();
        }

        if (pin.HasMember("z_function") && pin["z_function"].IsString())
        {
            pin_ctx.boolean_functions[name + "_tristate"] = pin["z_function"].GetString();
        }

        if (pin.HasMember("type") && pin["type"].IsString())
        {
            std::string type_str = pin["type"].GetString();
            if (auto it = m_string_to_pin_type.find(type_str); it != m_string_to_pin_type.end())
            {
                pin_ctx.pin_to_type.emplace(name, it->second);
            }
            else
            {
                log_warning("hgl_parser", "invalid type '{}' given for pin '{}' of gate type '{}'.", type_str, name, gt_name);
                return false;
            }
        }

        return true;
    }

    bool HGLParser::parse_group(GroupCtx& group_ctx, const rapidjson::Value& group, const std::string& gt_name)
    {
        // read name
        std::string name;
        if (!group.HasMember("name") || !group["name"].IsString())
        {
            log_error("hgl_parser", "invalid name for at least one pin of gate type '{}'.", gt_name);
            return false;
        }
        name = group["name"].GetString();

        // read index to pin mapping
        std::map<u32, std::string> index_to_pin;
        if (!group.HasMember("pins") || !group["pins"].IsObject())
        {
            log_error("hgl_parser", "no pins given for group '{}' of gate type '{}'.", name, gt_name);
            return false;
        }

        const auto pins_obj = group["pins"].GetObject();
        for (auto it = pins_obj.MemberBegin(); it != pins_obj.MemberEnd(); ++it)
        {
            u32 pin_index        = std::stoul(it->name.GetString());
            std::string pin_name = it->value.GetString();
            index_to_pin.emplace(pin_index, pin_name);
        }

        // read direction
        if (!group.HasMember("direction") || !group["direction"].IsString())
        {
            log_error("hgl_parser", "invalid direction for pin '{}' of gate type '{}'.", name, gt_name);
            return false;
        }
        std::string direction = group["direction"].GetString();
        if (direction == "input")
        {
            group_ctx.input_groups.emplace(name, index_to_pin);
        }
        else if (direction == "output")
        {
            group_ctx.output_groups.emplace(name, index_to_pin);
        }
        else if (direction == "inout")
        {
            group_ctx.input_groups.emplace(name, index_to_pin);
            group_ctx.output_groups.emplace(name, index_to_pin);
        }
        else
        {
            log_warning("hgl_parser", "invalid direction '{}' given for pin '{}' of gate type '{}'.", direction, name, gt_name);
            return false;
        }

        return true;
    }

    bool HGLParser::parse_lut_config(GateType* gt_lut, const rapidjson::Value& lut_config)
    {
        if (!lut_config.HasMember("bit_order") || !lut_config["bit_order"].IsString())
        {
            log_error("hgl_parser", "invalid bit order for LUT gate type '{}'.", gt_lut->get_name());
            return false;
        }

        if (std::string(lut_config["bit_order"].GetString()) == "ascending")
        {
            gt_lut->set_lut_init_ascending(true);
        }
        else
        {
            gt_lut->set_lut_init_ascending(false);
        }

        if (!lut_config.HasMember("data_category") || !lut_config["data_category"].IsString())
        {
            log_error("hgl_parser", "invalid data category for LUT gate type '{}'.", gt_lut->get_name());
            return false;
        }

        gt_lut->set_config_data_category(lut_config["data_category"].GetString());

        if (!lut_config.HasMember("data_identifier") || !lut_config["data_identifier"].IsString())
        {
            log_error("hgl_parser", "invalid data identifier for LUT gate type '{}'.", gt_lut->get_name());
            return false;
        }

        gt_lut->set_config_data_identifier(lut_config["data_identifier"].GetString());

        return true;
    }

    bool HGLParser::parse_ff_config(GateType* gt_ff, const rapidjson::Value& ff_config)
    {
        if (ff_config.HasMember("next_state") && ff_config["next_state"].IsString())
        {
            gt_ff->add_boolean_function("next_state", BooleanFunction::from_string(ff_config["next_state"].GetString(), gt_ff->get_input_pins()));
        }

        if (ff_config.HasMember("clocked_on") && ff_config["clocked_on"].IsString())
        {
            gt_ff->add_boolean_function("clock", BooleanFunction::from_string(ff_config["clocked_on"].GetString(), gt_ff->get_input_pins()));
        }

        if (ff_config.HasMember("clear_on") && ff_config["clear_on"].IsString())
        {
            gt_ff->add_boolean_function("clear", BooleanFunction::from_string(ff_config["clear_on"].GetString(), gt_ff->get_input_pins()));
        }

        if (ff_config.HasMember("preset_on") && ff_config["preset_on"].IsString())
        {
            gt_ff->add_boolean_function("preset", BooleanFunction::from_string(ff_config["preset_on"].GetString(), gt_ff->get_input_pins()));
        }

        bool has_state     = ff_config.HasMember("state_clear_preset") && ff_config["state_clear_preset"].IsString();
        bool has_neg_state = ff_config.HasMember("neg_state_clear_preset") && ff_config["neg_state_clear_preset"].IsString();

        if (has_state && has_neg_state)
        {
            GateType::ClearPresetBehavior cp1, cp2;

            if (const auto it = m_string_to_behavior.find(ff_config["state_clear_preset"].GetString()); it != m_string_to_behavior.end())
            {
                cp1 = it->second;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for state of flip-flop gate type '{}'.", ff_config["state_clear_preset"].GetString(), gt_ff->get_name());
                return false;
            }

            if (const auto it = m_string_to_behavior.find(ff_config["neg_state_clear_preset"].GetString()); it != m_string_to_behavior.end())
            {
                cp2 = it->second;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for negated state of flip-flop gate type '{}'.", ff_config["neg_state_clear_preset"].GetString(), gt_ff->get_name());
                return false;
            }

            gt_ff->set_clear_preset_behavior(cp1, cp2);
        }
        else if ((has_state && !has_neg_state) || (!has_state && has_neg_state))
        {
            log_error("hgl_parser", "requires specification of the clear-preset behavior for the state as well as the negated state for flip-flop gate type '{}'.", gt_ff->get_name());
            return false;
        }

        if (ff_config.HasMember("data_category") && ff_config["data_category"].IsString())
        {
            gt_ff->set_config_data_category(ff_config["data_category"].GetString());
        }

        if (ff_config.HasMember("data_identifier") && ff_config["data_identifier"].IsString())
        {
            gt_ff->set_config_data_identifier(ff_config["data_identifier"].GetString());
        }

        return true;
    }

    bool HGLParser::parse_latch_config(GateType* gt_latch, const rapidjson::Value& latch_config)
    {
        if (latch_config.HasMember("data_in") && latch_config["data_in"].IsString())
        {
            gt_latch->add_boolean_function("data", BooleanFunction::from_string(latch_config["data_in"].GetString(), gt_latch->get_input_pins()));
        }

        if (latch_config.HasMember("enable_on") && latch_config["enable_on"].IsString())
        {
            gt_latch->add_boolean_function("enable", BooleanFunction::from_string(latch_config["enable_on"].GetString(), gt_latch->get_input_pins()));
        }

        if (latch_config.HasMember("clear_on") && latch_config["clear_on"].IsString())
        {
            gt_latch->add_boolean_function("clear", BooleanFunction::from_string(latch_config["clear_on"].GetString(), gt_latch->get_input_pins()));
        }

        if (latch_config.HasMember("preset_on") && latch_config["preset_on"].IsString())
        {
            gt_latch->add_boolean_function("preset", BooleanFunction::from_string(latch_config["preset_on"].GetString(), gt_latch->get_input_pins()));
        }

        bool has_state     = latch_config.HasMember("state_clear_preset") && latch_config["state_clear_preset"].IsString();
        bool has_neg_state = latch_config.HasMember("neg_state_clear_preset") && latch_config["neg_state_clear_preset"].IsString();

        if (has_state && has_neg_state)
        {
            GateType::ClearPresetBehavior cp1, cp2;

            if (const auto it = m_string_to_behavior.find(latch_config["state_clear_preset"].GetString()); it != m_string_to_behavior.end())
            {
                cp1 = it->second;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for state of flip-flop gate type '{}'.", latch_config["state_clear_preset"].GetString(), gt_latch->get_name());
                return false;
            }

            if (const auto it = m_string_to_behavior.find(latch_config["neg_state_clear_preset"].GetString()); it != m_string_to_behavior.end())
            {
                cp2 = it->second;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for negated state of flip-flop gate type '{}'.", latch_config["neg_state_clear_preset"].GetString(), gt_latch->get_name());
                return false;
            }

            gt_latch->set_clear_preset_behavior(cp1, cp2);
        }
        else if ((has_state && !has_neg_state) || (!has_state && has_neg_state))
        {
            log_error("hgl_parser", "requires specification of the clear-preset behavior for the state as well as the negated state for flip-flop gate type '{}'.", gt_latch->get_name());
            return false;
        }

        return true;
    }
}    // namespace hal
