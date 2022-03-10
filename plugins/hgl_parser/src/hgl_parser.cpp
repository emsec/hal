#include "hgl_parser/hgl_parser.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
#include "hal_core/utilities/log.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

namespace hal
{
    Result<std::unique_ptr<GateLibrary>> HGLParser::parse(const std::filesystem::path& file_path)
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

        if (document.HasMember("gate_locations") && document["gate_locations"].IsObject())
        {
            auto gate_locs = document["gate_locations"].GetObject();

            if (!gate_locs.HasMember("data_category") || !gate_locs["data_category"].IsString())
            {
                log_error("hgl_parser", "missing 'data_category' for gate locations.");
                return false;
            }

            if (!gate_locs.HasMember("data_x_identifier") || !gate_locs["data_x_identifier"].IsString())
            {
                log_error("hgl_parser", "missing 'data_x_identifier' for gate locations.");
                return false;
            }

            if (!gate_locs.HasMember("data_y_identifier") || !gate_locs["data_y_identifier"].IsString())
            {
                log_error("hgl_parser", "missing 'data_y_identifier' for gate locations.");
                return false;
            }

            m_gate_lib->set_gate_location_data_category(gate_locs["data_category"].GetString());
            m_gate_lib->set_gate_location_data_identifiers(gate_locs["data_x_identifier"].GetString(), gate_locs["data_y_identifier"].GetString());
        }

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
        std::set<GateTypeProperty> properties;
        PinCtx pin_ctx;

        if (!gate_type.HasMember("name") || !gate_type["name"].IsString())
        {
            log_error("hgl_parser", "invalid name for at least one gate type.");
            return false;
        }
        name = gate_type["name"].GetString();

        if (gate_type.HasMember("types") && gate_type["types"].IsArray())
        {
            for (const auto& base_type : gate_type["types"].GetArray())
            {
                try
                {
                    GateTypeProperty property = enum_from_string<GateTypeProperty>(base_type.GetString());
                    properties.insert(property);
                }
                catch (const std::runtime_error&)
                {
                    log_error("hgl_parser", "invalid base type '{}' given for gate type '{}'.", base_type.GetString(), name);
                    return false;
                }
            }
        }
        else
        {
            properties = {GateTypeProperty::combinational};
        }

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

        std::unique_ptr<GateTypeComponent> parent_component = nullptr;
        std::vector<std::string> bf_vars                    = pin_ctx.pins;    // we MUST allow for output pins here

        if (gate_type.HasMember("lut_config") && gate_type["lut_config"].IsObject())
        {
            parent_component = parse_lut_config(gate_type["lut_config"], name);
            if (parent_component == nullptr)
            {
                return false;
            }
        }
        else if (gate_type.HasMember("ff_config") && gate_type["ff_config"].IsObject())
        {
            parent_component = parse_ff_config(gate_type["ff_config"], name, bf_vars);
            if (parent_component == nullptr)
            {
                return false;
            }
        }
        else if (gate_type.HasMember("latch_config") && gate_type["latch_config"].IsObject())
        {
            parent_component = parse_latch_config(gate_type["latch_config"], name, bf_vars);
            if (parent_component == nullptr)
            {
                return false;
            }
        }
        else if (gate_type.HasMember("ram_config") && gate_type["ram_config"].IsObject())
        {
            parent_component = parse_ram_config(gate_type["ram_config"], name, bf_vars);
            if (parent_component == nullptr)
            {
                return false;
            }
        }

        GateType* gt = m_gate_lib->create_gate_type(name, properties, std::move(parent_component));

        for (const auto& pin : pin_ctx.pins)
        {
            gt->add_pin(pin, pin_ctx.pin_to_direction.at(pin), pin_ctx.pin_to_type.at(pin));
        }

        if (gate_type.HasMember("groups") && gate_type["groups"].IsArray())
        {
            for (const auto& group_val : gate_type["groups"].GetArray())
            {
                if (!parse_group(gt, group_val, name))
                {
                    return false;
                }
            }
        }

        for (const auto& [f_name, func] : pin_ctx.boolean_functions)
        {
            auto function = BooleanFunction::from_string(func);
            gt->add_boolean_function(f_name, (function.is_ok()) ? function.get() : BooleanFunction());
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
        try
        {
            pin_ctx.pin_to_direction[name] = enum_from_string<PinDirection>(direction);
            pin_ctx.pins.push_back(name);
        }
        catch (const std::runtime_error&)
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
            try
            {
                pin_ctx.pin_to_type[name] = enum_from_string<PinType>(type_str);
            }
            catch (const std::runtime_error&)
            {
                log_warning("hgl_parser", "invalid type '{}' given for pin '{}' of gate type '{}'.", type_str, name, gt_name);
                return false;
            }
        }
        else
        {
            pin_ctx.pin_to_type[name] = PinType::none;
        }

        return true;
    }

    bool HGLParser::parse_group(GateType* gt, const rapidjson::Value& group, const std::string& gt_name)
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
        if (!group.HasMember("pins") || !group["pins"].IsArray())
        {
            log_error("hgl_parser", "no valid pins given for group '{}' of gate type '{}'.", name, gt_name);
            return false;
        }

        std::vector<std::pair<u32, std::string>> pins;
        for (const auto& pin_obj : group["pins"].GetArray())
        {
            if (!pin_obj.IsObject())
            {
                log_error("hgl_parser", "invalid pin group assignment given for group '{}' of gate type '{}'.", name, gt_name);
                return false;
            }
            const auto pin_val   = pin_obj.GetObject().MemberBegin();
            u32 pin_index        = std::stoul(pin_val->name.GetString());
            std::string pin_name = pin_val->value.GetString();
            pins.push_back(std::make_pair(pin_index, pin_name));
        }

        return gt->assign_pin_group(name, pins);
    }

    std::unique_ptr<GateTypeComponent> HGLParser::parse_lut_config(const rapidjson::Value& lut_config, const std::string& gt_name)
    {
        if (!lut_config.HasMember("bit_order") || !lut_config["bit_order"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'bit_order' specification for LUT gate type '{}'.", gt_name);
            return nullptr;
        }

        if (!lut_config.HasMember("data_category") || !lut_config["data_category"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'data_category' specification for LUT gate type '{}'.", gt_name);
            return nullptr;
        }

        if (!lut_config.HasMember("data_identifier") || !lut_config["data_identifier"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'data_identifier' specification for LUT gate type '{}'.", gt_name);
            return nullptr;
        }

        std::unique_ptr<GateTypeComponent> init_component = GateTypeComponent::create_init_component(lut_config["data_category"].GetString(), {lut_config["data_identifier"].GetString()});

        return GateTypeComponent::create_lut_component(std::move(init_component), std::string(lut_config["bit_order"].GetString()) == "ascending");
    }

    std::unique_ptr<GateTypeComponent> HGLParser::parse_ff_config(const rapidjson::Value& ff_config, const std::string& gt_name, std::vector<std::string>& bf_vars)
    {
        if (!ff_config.HasMember("state") || !ff_config["state"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'state' specification for flip-flop gate type '{}'.", gt_name);
            return nullptr;
        }
        if (!ff_config.HasMember("neg_state") || !ff_config["neg_state"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'neg_state' specification for flip-flop gate type '{}'.", gt_name);
            return nullptr;
        }
        if (!ff_config.HasMember("next_state") || !ff_config["next_state"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'next_state' specification for flip-flop gate type '{}'.", gt_name);
            return nullptr;
        }
        if (!ff_config.HasMember("clocked_on") || !ff_config["clocked_on"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'clocked_on' specification for flip-flop gate type '{}'.", gt_name);
            return nullptr;
        }

        std::unique_ptr<GateTypeComponent> init_component = nullptr;
        if (ff_config.HasMember("data_category") && ff_config["data_category"].IsString())
        {
            std::vector<std::string> init_identifiers;
            if (ff_config.HasMember("data_identifier") && ff_config["data_identifier"].IsString())
            {    // backward compatability
                init_identifiers.push_back(ff_config["data_identifier"].GetString());
            }
            else
            {
                log_error("hgl_parser", "invalid or missing 'data_identifier' specification for flip-flop gate type '{}'.", gt_name);
                return nullptr;
            }
            init_component = GateTypeComponent::create_init_component(ff_config["data_category"].GetString(), init_identifiers);
        }
        else if (ff_config.HasMember("data_identifier") && ff_config["data_identifier"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'data_category' specification for flip-flop gate type '{}'.", gt_name);
            return nullptr;
        }

        std::string state_identifier                       = ff_config["state"].GetString();
        std::string neg_state_identifier                   = ff_config["neg_state"].GetString();
        std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(std::move(init_component), state_identifier, neg_state_identifier);
        bf_vars.push_back(state_identifier);
        bf_vars.push_back(neg_state_identifier);
        assert(state_component != nullptr);

        auto next_state_function = BooleanFunction::from_string(ff_config["next_state"].GetString());
        auto clocked_on_function = BooleanFunction::from_string(ff_config["clocked_on"].GetString());

        std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_ff_component(
            std::move(state_component), (next_state_function.is_ok()) ? next_state_function.get() : BooleanFunction(), (clocked_on_function.is_ok()) ? clocked_on_function.get() : BooleanFunction());

        FFComponent* ff_component = component->convert_to<FFComponent>();
        assert(ff_component != nullptr);

        if (ff_config.HasMember("clear_on") && ff_config["clear_on"].IsString())
        {
            auto clear_on_function = BooleanFunction::from_string(ff_config["clear_on"].GetString());
            ff_component->set_async_reset_function((clear_on_function.is_ok()) ? clear_on_function.get() : BooleanFunction());
        }

        if (ff_config.HasMember("preset_on") && ff_config["preset_on"].IsString())
        {
            auto preset_on_function = BooleanFunction::from_string(ff_config["preset_on"].GetString());
            ff_component->set_async_set_function((preset_on_function.is_ok()) ? preset_on_function.get() : BooleanFunction());
        }

        bool has_state     = ff_config.HasMember("state_clear_preset") && ff_config["state_clear_preset"].IsString();
        bool has_neg_state = ff_config.HasMember("neg_state_clear_preset") && ff_config["neg_state_clear_preset"].IsString();

        if (has_state && has_neg_state)
        {
            AsyncSetResetBehavior cp1, cp2;

            if (const auto behav = enum_from_string<AsyncSetResetBehavior>(ff_config["state_clear_preset"].GetString(), AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
            {
                cp1 = behav;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for state of flip-flop gate type '{}'.", ff_config["state_clear_preset"].GetString(), gt_name);
                return nullptr;
            }

            if (const auto behav = enum_from_string<AsyncSetResetBehavior>(ff_config["neg_state_clear_preset"].GetString(), AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
            {
                cp2 = behav;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for negated state of flip-flop gate type '{}'.", ff_config["neg_state_clear_preset"].GetString(), gt_name);
                return nullptr;
            }

            ff_component->set_async_set_reset_behavior(cp1, cp2);
        }
        else if ((has_state && !has_neg_state) || (!has_state && has_neg_state))
        {
            log_error("hgl_parser", "requires specification of the clear-preset behavior for the state as well as the negated state for flip-flop gate type '{}'.", gt_name);
            return nullptr;
        }

        return component;
    }

    std::unique_ptr<GateTypeComponent> HGLParser::parse_latch_config(const rapidjson::Value& latch_config, const std::string& gt_name, std::vector<std::string>& bf_vars)
    {
        if (!latch_config.HasMember("state") || !latch_config["state"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'state' specification for latch gate type '{}'.", gt_name);
            return nullptr;
        }
        if (!latch_config.HasMember("neg_state") || !latch_config["neg_state"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'neg_state' specification for latch gate type '{}'.", gt_name);
            return nullptr;
        }

        std::string state_identifier                       = latch_config["state"].GetString();
        std::string neg_state_identifier                   = latch_config["neg_state"].GetString();
        std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(nullptr, state_identifier, neg_state_identifier);
        bf_vars.push_back(state_identifier);
        bf_vars.push_back(neg_state_identifier);
        assert(state_component != nullptr);

        std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_latch_component(std::move(state_component));
        LatchComponent* latch_component              = component->convert_to<LatchComponent>();
        assert(latch_component != nullptr);

        if (latch_config.HasMember("data_in") && latch_config["data_in"].IsString() && latch_config.HasMember("enable_on") && latch_config["enable_on"].IsString())
        {
            auto data_in_function = BooleanFunction::from_string(latch_config["data_in"].GetString());
            latch_component->set_data_in_function((data_in_function.is_ok()) ? data_in_function.get() : BooleanFunction());

            auto enable_on_function = BooleanFunction::from_string(latch_config["enable_on"].GetString());
            latch_component->set_enable_function((enable_on_function.is_ok()) ? enable_on_function.get() : BooleanFunction());
        }
        else if (latch_config.HasMember("data_in") && latch_config["data_in"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'enable_on' specification for latch gate type '{}'.", gt_name);
            return nullptr;
        }
        else if (latch_config.HasMember("enable_on") && latch_config["enable_on"].IsString())
        {
            log_error("hgl_parser", "invalid or missing 'data_in' specification for latch gate type '{}'.", gt_name);
            return nullptr;
        }

        if (latch_config.HasMember("clear_on") && latch_config["clear_on"].IsString())
        {
            auto clear_on_function = BooleanFunction::from_string(latch_config["clear_on"].GetString());
            latch_component->set_async_reset_function((clear_on_function.is_ok()) ? clear_on_function.get() : BooleanFunction());
        }

        if (latch_config.HasMember("preset_on") && latch_config["preset_on"].IsString())
        {
            auto preset_on_function = BooleanFunction::from_string(latch_config["preset_on"].GetString());
            latch_component->set_async_set_function((preset_on_function.is_ok()) ? preset_on_function.get() : BooleanFunction());
        }

        bool has_state     = latch_config.HasMember("state_clear_preset") && latch_config["state_clear_preset"].IsString();
        bool has_neg_state = latch_config.HasMember("neg_state_clear_preset") && latch_config["neg_state_clear_preset"].IsString();

        if (has_state && has_neg_state)
        {
            AsyncSetResetBehavior cp1, cp2;

            if (const auto behav = enum_from_string<AsyncSetResetBehavior>(latch_config["state_clear_preset"].GetString(), AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
            {
                cp1 = behav;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for state of latch gate type '{}'.", latch_config["state_clear_preset"].GetString(), gt_name);
                return nullptr;
            }

            if (const auto behav = enum_from_string<AsyncSetResetBehavior>(latch_config["neg_state_clear_preset"].GetString(), AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
            {
                cp2 = behav;
            }
            else
            {
                log_error("hgl_parser", "invalid clear-preset behavior '{}' for negated state of latch gate type '{}'.", latch_config["neg_state_clear_preset"].GetString(), gt_name);
                return nullptr;
            }

            latch_component->set_async_set_reset_behavior(cp1, cp2);
        }
        else if ((has_state && !has_neg_state) || (!has_state && has_neg_state))
        {
            log_error("hgl_parser", "requires specification of the clear-preset behavior for the state as well as the negated state for latch gate type '{}'.", gt_name);
            return nullptr;
        }

        return component;
    }

    std::unique_ptr<GateTypeComponent> HGLParser::parse_ram_config(const rapidjson::Value& ram_config, const std::string& gt_name, const std::vector<std::string>& bf_vars)
    {
        std::unique_ptr<GateTypeComponent> sub_component = nullptr;
        if (ram_config.HasMember("data_category") && ram_config["data_category"].IsString())
        {
            std::vector<std::string> init_identifiers;
            if (ram_config.HasMember("data_identifiers") && ram_config["data_identifiers"].IsArray())
            {    // now allows for multiple identifiers (required for BRAM)

                for (const auto& identifier : ram_config["data_identifiers"].GetArray())
                {
                    init_identifiers.push_back(identifier.GetString());
                }
            }
            else
            {
                log_error("hgl_parser", "invalid or missing 'data_identifiers' specification for RAM gate type '{}'.", gt_name);
                return nullptr;
            }
            sub_component = GateTypeComponent::create_init_component(ram_config["data_category"].GetString(), init_identifiers);
        }
        else if (ram_config.HasMember("data_identifiers") && ram_config["data_identifiers"].IsArray())
        {
            log_error("hgl_parser", "invalid or missing 'data_category' specification for RAM gate type '{}'.", gt_name);
            return nullptr;
        }

        if (!ram_config.HasMember("bit_size") || !ram_config["bit_size"].IsUint())
        {
            log_error("hgl_parser", "invalid or missing 'bit_size' specification for RAM gate type '{}'.", gt_name);
            return nullptr;
        }

        if (!ram_config.HasMember("ram_ports") || !ram_config["ram_ports"].IsArray())
        {
            log_error("hgl_parser", "invalid or missing 'ram_ports' specification for RAM gate type '{}'.", gt_name);
            return nullptr;
        }

        for (const auto& ram_port : ram_config["ram_ports"].GetArray())
        {
            if (!ram_port.HasMember("data_group") || !ram_port["data_group"].IsString())
            {
                log_error("hgl_parser", "invalid or missing 'data_groups' specification for RAM port gate type '{}'.", gt_name);
                return nullptr;
            }

            if (!ram_port.HasMember("address_group") || !ram_port["address_group"].IsString())
            {
                log_error("hgl_parser", "invalid or missing 'address_groups' specification for RAM port gate type '{}'.", gt_name);
                return nullptr;
            }

            if (!ram_port.HasMember("clocked_on") || !ram_port["clocked_on"].IsString())
            {
                log_error("hgl_parser", "invalid or missing 'clocked_on' specification for RAM port gate type '{}'.", gt_name);
                return nullptr;
            }

            if (!ram_port.HasMember("enabled_on") || !ram_port["enabled_on"].IsString())
            {
                log_error("hgl_parser", "invalid or missing 'enabled_on' specification for RAM port gate type '{}'.", gt_name);
                return nullptr;
            }

            if (!ram_port.HasMember("is_write") || !ram_port["is_write"].IsBool())
            {
                log_error("hgl_parser", "invalid or missing 'is_write' specification for RAM port gate type '{}'.", gt_name);
                return nullptr;
            }

            auto clocked_on_function = BooleanFunction::from_string(ram_port["clocked_on"].GetString());
            auto enabled_on_function = BooleanFunction::from_string(ram_port["enabled_on"].GetString());

            sub_component = GateTypeComponent::create_ram_port_component(std::move(sub_component),
                                                                         ram_port["data_group"].GetString(),
                                                                         ram_port["address_group"].GetString(),
                                                                         (clocked_on_function.is_ok()) ? clocked_on_function.get() : BooleanFunction(),
                                                                         (enabled_on_function.is_ok()) ? enabled_on_function.get() : BooleanFunction(),
                                                                         ram_port["is_write"].GetBool());
        }

        std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_ram_component(std::move(sub_component), ram_config["bit_size"].GetUint());

        return component;
    }
}    // namespace hal
