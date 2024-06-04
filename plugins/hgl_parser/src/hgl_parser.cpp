#include "hgl_parser/hgl_parser.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/result.h"
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
            return ERR("could not parse HGL file '" + m_path.string() + "' : unable to open file");
        }

        char buffer[65536];
        rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);
        fclose(fp);

        if (document.HasParseError())
        {
            return ERR("could not parse HGL file '" + m_path.string() + "': failed parsing JSON format");
        }

        if (auto res = parse_gate_library(document); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not parse HGL file '" + m_path.string() + "': failed to parse gate library");
        }

        return OK(std::move(m_gate_lib));
    }

    Result<std::monostate> HGLParser::parse_gate_library(const rapidjson::Document& document)
    {
        if (!document.HasMember("version") || !document["version"].IsUint() || document["version"].GetUint() < HGL_FORMAT_VERSION)
        {
            log_warning("hgl_parser", "you are using an outdated HGL file format that might not support all features, please make sure to re-write the HGL file using the HAL HGL writer.");
        }

        if (document.HasMember("version") && document["version"].IsUint())
        {
            file_version = document["version"].GetUint();
        }

        if (!document.HasMember("library"))
        {
            return ERR("could not parse gate library: missing 'library' node");
        }

        const std::string gate_lib_name = document["library"].GetString();
        m_gate_lib                      = std::make_unique<GateLibrary>(m_path, gate_lib_name);

        if (document.HasMember("gate_locations") && document["gate_locations"].IsObject())
        {
            auto gate_locs = document["gate_locations"].GetObject();

            if (!gate_locs.HasMember("data_category") || !gate_locs["data_category"].IsString())
            {
                return ERR("could not parse gate library '" + gate_lib_name + "': missing 'data_category' entry for gate locations");
            }

            if (!gate_locs.HasMember("data_x_identifier") || !gate_locs["data_x_identifier"].IsString())
            {
                return ERR("could not parse gate library '" + gate_lib_name + "': missing 'data_x_identifier' entry for gate locations");
            }

            if (!gate_locs.HasMember("data_y_identifier") || !gate_locs["data_y_identifier"].IsString())
            {
                return ERR("could not parse gate library '" + gate_lib_name + "': missing 'data_y_identifier' entry for gate locations");
            }

            m_gate_lib->set_gate_location_data_category(gate_locs["data_category"].GetString());
            m_gate_lib->set_gate_location_data_identifiers(gate_locs["data_x_identifier"].GetString(), gate_locs["data_y_identifier"].GetString());
        }

        if (!document.HasMember("cells"))
        {
            return ERR("could not parse gate library '" + gate_lib_name + "': missing 'cells' node");
        }

        for (const auto& gate_type : document["cells"].GetArray())
        {
            if (auto res = parse_gate_type(gate_type); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse gate library '" + gate_lib_name + "': failed parsing gate type");
            }
        }

        return OK({});
    }

    Result<std::monostate> HGLParser::parse_gate_type(const rapidjson::Value& gate_type)
    {
        if (!gate_type.HasMember("name") || !gate_type["name"].IsString())
        {
            return ERR("could not parse gate type: missing or invalid name");
        }

        const std::string name = gate_type["name"].GetString();
        std::set<GateTypeProperty> properties;
        if (gate_type.HasMember("types") && gate_type["types"].IsArray())
        {
            for (const auto& base_type : gate_type["types"].GetArray())
            {
                const std::string property_str = base_type.GetString();

                try
                {
                    GateTypeProperty property = enum_from_string<GateTypeProperty>(property_str);
                    properties.insert(property);
                }
                catch (const std::runtime_error&)
                {
                    // legacy support
                    if (property_str == "buffer")
                    {
                        properties.insert(GateTypeProperty::c_buffer);
                    }
                    else if (property_str == "carry")
                    {
                        properties.insert(GateTypeProperty::c_carry);
                    }
                    else if (property_str == "mux")
                    {
                        properties.insert(GateTypeProperty::c_mux);
                    }
                    else if (property_str == "lut")
                    {
                        properties.insert(GateTypeProperty::c_lut);
                    }
                    else
                    {
                        return ERR("could not parse gate type '" + name + "': invalid property '" + base_type.GetString() + "'");
                    }
                }
            }
        }
        else
        {
            properties = {GateTypeProperty::combinational};
        }

        std::unique_ptr<GateTypeComponent> parent_component = nullptr;
        if (gate_type.HasMember("lut_config") && gate_type["lut_config"].IsObject())
        {
            if (auto res = parse_lut_config(gate_type["lut_config"]); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "': failed parsing LUT configuration");
            }
            else
            {
                parent_component = res.get();
            }
        }
        else if (gate_type.HasMember("ff_config") && gate_type["ff_config"].IsObject())
        {
            if (auto res = parse_ff_config(gate_type["ff_config"]); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "': failed parsing FF configuration");
            }
            else
            {
                parent_component = res.get();
            }
        }
        else if (gate_type.HasMember("latch_config") && gate_type["latch_config"].IsObject())
        {
            if (auto res = parse_latch_config(gate_type["latch_config"]); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "': failed parsing latch configuration");
            }
            else
            {
                parent_component = res.get();
            }
        }
        else if (gate_type.HasMember("ram_config") && gate_type["ram_config"].IsObject())
        {
            if (auto res = parse_ram_config(gate_type["ram_config"]); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "': failed parsing RAM configuration");
            }
            else
            {
                parent_component = res.get();
            }
        }

        GateType* gt = m_gate_lib->create_gate_type(name, properties, std::move(parent_component));
        if (gt == nullptr)
        {
            return ERR("could not parse gate type '" + name + "': failed to create gate type");
        }

        if (file_version >= 2)
        {
            if (gate_type.HasMember("pin_groups") && gate_type["pin_groups"].IsArray())
            {
                for (const auto& pg_val : gate_type["pin_groups"].GetArray())
                {
                    if (!pg_val.HasMember("name") || !pg_val["name"].IsString())
                    {
                        return ERR("could not parse pin group: missing or invalid name");
                    }
                    std::string pg_name = pg_val["name"].GetString();

                    if (!pg_val.HasMember("direction") || !pg_val["direction"].IsString())
                    {
                        return ERR("could not parse pin group '" + pg_name + "': missing or invalid pin direction");
                    }
                    PinDirection pg_direction;
                    std::string pg_direction_str = pg_val["direction"].GetString();
                    try
                    {
                        pg_direction = enum_from_string<PinDirection>(pg_direction_str);
                    }
                    catch (const std::runtime_error&)
                    {
                        return ERR("could not parse pin '" + pg_name + "': invalid pin direction '" + pg_direction_str + "'");
                    }

                    if (!pg_val.HasMember("type") || !pg_val["type"].IsString())
                    {
                        return ERR("could not parse pin group '" + pg_name + "': missing or invalid pin type");
                    }
                    PinType pg_type;
                    std::string pg_type_str = pg_val["type"].GetString();
                    try
                    {
                        pg_type = enum_from_string<PinType>(pg_type_str);
                    }
                    catch (const std::runtime_error&)
                    {
                        return ERR("could not parse pin group '" + pg_name + "': invalid pin type '" + pg_type_str + "'");
                    }

                    if (!pg_val.HasMember("ascending") || !pg_val["ascending"].IsBool())
                    {
                        return ERR("could not parse pin group '" + pg_name + "': missing or ascending property");
                    }
                    bool ascending = pg_val["ascending"].GetBool();

                    bool ordered = false;
                    if (pg_val.HasMember("ordered") && pg_val["ordered"].IsBool())
                    {
                        ordered = pg_val["ordered"].GetBool();
                    }

                    if (!pg_val.HasMember("start_index") || !pg_val["start_index"].IsUint())
                    {
                        return ERR("could not parse pin group '" + pg_name + "': missing or start index");
                    }
                    u32 start_index = pg_val["start_index"].GetUint();

                    if (pg_val.HasMember("pins") && pg_val["pins"].IsArray())
                    {
                        std::vector<GatePin*> pins;

                        for (const auto& p_val : pg_val["pins"].GetArray())
                        {
                            if (!p_val.HasMember("name") || !p_val["name"].IsString())
                            {
                                return ERR("could not parse pin: missing or invalid name");
                            }
                            std::string p_name = p_val["name"].GetString();

                            if (!p_val.HasMember("direction") || !p_val["direction"].IsString())
                            {
                                return ERR("could not parse pin '" + p_name + "': missing or invalid pin direction");
                            }
                            PinDirection p_direction;
                            std::string p_direction_str = p_val["direction"].GetString();
                            try
                            {
                                p_direction = enum_from_string<PinDirection>(p_direction_str);
                            }
                            catch (const std::runtime_error&)
                            {
                                return ERR("could not parse pin '" + p_name + "': invalid pin direction '" + p_direction_str + "'");
                            }

                            PinType p_type;
                            if (p_val.HasMember("type") && p_val["type"].IsString())
                            {
                                std::string p_type_str = p_val["type"].GetString();
                                try
                                {
                                    p_type = enum_from_string<PinType>(p_type_str);
                                }
                                catch (const std::runtime_error&)
                                {
                                    return ERR("could not parse pin '" + p_name + "': invalid pin type '" + p_type_str + "'");
                                }
                            }
                            else
                            {
                                p_type = PinType::none;
                            }

                            auto p_res = gt->create_pin(p_name, p_direction, p_type, false);
                            if (p_res.is_error())
                            {
                                return ERR_APPEND(p_res.get_error(), "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed to create pin '" + p_name + "'");
                            }
                            pins.push_back(p_res.get());

                            if (p_val.HasMember("function") && p_val["function"].IsString())
                            {
                                if (auto res = BooleanFunction::from_string(p_val["function"].GetString()); res.is_error())
                                {
                                    return ERR_APPEND(res.get_error(),
                                                      "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed parsing Boolean function with name '" + p_name
                                                          + "' from string");
                                }
                                else
                                {
                                    gt->add_boolean_function(p_name, res.get());
                                }
                            }

                            if (p_val.HasMember("x_function") && p_val["x_function"].IsString())
                            {
                                if (auto res = BooleanFunction::from_string(p_val["x_function"].GetString()); res.is_error())
                                {
                                    return ERR_APPEND(res.get_error(),
                                                      "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed parsing Boolean function with name '" + p_name
                                                          + "_undefined' from string");
                                }
                                else
                                {
                                    gt->add_boolean_function(p_name + "_undefined", res.get());
                                }
                            }

                            if (p_val.HasMember("z_function") && p_val["z_function"].IsString())
                            {
                                if (auto res = BooleanFunction::from_string(p_val["z_function"].GetString()); res.is_error())
                                {
                                    return ERR_APPEND(res.get_error(),
                                                      "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed parsing Boolean function with name '" + p_name
                                                          + "_tristate' from string");
                                }
                                else
                                {
                                    gt->add_boolean_function(p_name + "_tristate", res.get());
                                }
                            }
                        }

                        auto pg_res = gt->create_pin_group(pg_name, pins, pg_direction, pg_type, ascending, start_index, ordered);
                        if (pg_res.is_error())
                        {
                            return ERR_APPEND(pg_res.get_error(),
                                              "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed to create pin group '" + pg_name + "'");
                        }
                    }
                    else
                    {
                        return ERR("could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": no pins given for pin group with name '" + pg_name + "'");
                    }
                }
            }
        }
        else if (file_version < 2)
        {
            std::map<std::string, std::string> pins_to_groups;
            std::map<std::string, GroupCtx> groups_to_pins;

            if (gate_type.HasMember("groups") && gate_type["groups"].IsArray())
            {
                for (const auto& group_val : gate_type["groups"].GetArray())
                {
                    // read name
                    std::string pg_name;
                    if (!group_val.HasMember("name") || !group_val["name"].IsString())
                    {
                        return ERR("could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": missing or invalid pin group name");
                    }

                    pg_name = group_val["name"].GetString();
                    if (!group_val.HasMember("pins") || !group_val["pins"].IsArray())
                    {
                        return ERR("could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": missing or invalid pins for pin group '" + pg_name + "'");
                    }

                    // TODO will need changes to HGL format to be fancy
                    i32 start     = -1;
                    i32 direction = 0;
                    std::vector<std::string> pins;
                    GroupCtx pin_group_info;
                    for (const auto& pin_obj : group_val["pins"].GetArray())
                    {
                        if (!pin_obj.IsObject())
                        {
                            return ERR("could not parse pin group '" + name + "' with ID " + std::to_string(gt->get_id()) + ": invalid pin group assignment");
                        }
                        const auto pin_val   = pin_obj.GetObject().MemberBegin();
                        u32 pin_index        = std::stoul(pin_val->name.GetString());
                        std::string pin_name = pin_val->value.GetString();
                        pin_group_info.pins.push_back(pin_name);
                        pins_to_groups[pin_name] = pg_name;

                        // if (auto res = gt->get_pin_by_name(pin_name); res == nullptr)
                        // {
                        //     return ERR("could not parse pin group '" + name + "': failed to get pin by name '" + pin_name + "'");
                        // }
                        // else
                        // {
                        //     pins.push_back(res);
                        // }

                        if (start == -1)
                        {
                            start = pin_index;
                        }
                        else
                        {
                            direction = (start < (i32)pin_index) ? 1 : -1;
                        }
                    }

                    pin_group_info.ascending   = (direction == 1) ? true : false;
                    pin_group_info.start_index = start;

                    // if (auto res = gt->create_pin_group(name, pins, pins.at(0)->get_direction(), pins.at(0)->get_type(), , start); res.is_error())
                    // {
                    //     return ERR_APPEND(res.get_error(), "could not parse pin group '" + name + "': failed to create pin group");
                    // }
                    groups_to_pins[pg_name] = pin_group_info;
                }
            }

            PinCtx pin_ctx;
            if (gate_type.HasMember("pins") && gate_type["pins"].IsArray())
            {
                for (const auto& pin : gate_type["pins"].GetArray())
                {
                    if (auto res = parse_pin(pin_ctx, pin); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed parsing pin");
                    }
                }
            }

            for (const auto& pin_name : pin_ctx.pins)
            {
                if (const auto it = pins_to_groups.find(pin_name); it == pins_to_groups.end())
                {
                    if (auto res = gt->create_pin(pin_name, pin_ctx.pin_to_direction.at(pin_name), pin_ctx.pin_to_type.at(pin_name), true); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed to create pin '" + pin_name);
                    }
                }
                else
                {
                    if (gt->get_pin_group_by_name(it->second) != nullptr)
                    {
                        continue;
                    }

                    std::vector<GatePin*> pins;
                    auto pg_info = groups_to_pins.at(it->second);
                    for (const auto& p_name : pg_info.pins)
                    {
                        if (auto res = gt->create_pin(p_name, pin_ctx.pin_to_direction.at(p_name), pin_ctx.pin_to_type.at(p_name), false); res.is_error())
                        {
                            return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed to create pin '" + p_name);
                        }
                        else
                        {
                            pins.push_back(res.get());
                        }
                    }
                    if (auto res = gt->create_pin_group(it->second, pins, pins.front()->get_direction(), pins.front()->get_type(), pg_info.ascending, pg_info.start_index); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed to create pin group '" + it->second);
                    }
                }
            }

            for (const auto& [f_name, func] : pin_ctx.boolean_functions)
            {
                if (auto res = BooleanFunction::from_string(func); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not parse gate type '" + name + "' with ID " + std::to_string(gt->get_id()) + ": failed parsing Boolean function with name '" + f_name + "' from string");
                }
                else
                {
                    gt->add_boolean_function(f_name, res.get());
                }
            }
        }

        return OK({});
    }

    Result<std::monostate> HGLParser::parse_pin(PinCtx& pin_ctx, const rapidjson::Value& pin)
    {
        if (!pin.HasMember("name") || !pin["name"].IsString())
        {
            return ERR("could not parse pin: missing or invalid name");
        }

        std::string name = pin["name"].GetString();
        if (!pin.HasMember("direction") || !pin["direction"].IsString())
        {
            return ERR("could not parse pin '" + name + "': missing or invalid pin direction");
        }

        std::string direction = pin["direction"].GetString();
        try
        {
            pin_ctx.pin_to_direction[name] = enum_from_string<PinDirection>(direction);
            pin_ctx.pins.push_back(name);
        }
        catch (const std::runtime_error&)
        {
            return ERR("could not parse pin '" + name + "': invalid pin direction '" + direction + "'");
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
                return ERR("could not parse pin '" + name + "': invalid pin type '" + type_str + "'");
            }
        }
        else
        {
            pin_ctx.pin_to_type[name] = PinType::none;
        }

        return OK({});
    }

    Result<std::unique_ptr<GateTypeComponent>> HGLParser::parse_lut_config(const rapidjson::Value& lut_config)
    {
        if (!lut_config.HasMember("bit_order") || !lut_config["bit_order"].IsString())
        {
            return ERR("could not parse LUT configuration: missing or invalid bit order");
        }

        if (!lut_config.HasMember("data_category") || !lut_config["data_category"].IsString())
        {
            return ERR("could not parse LUT configuration: missing or invalid data category for LUT initialization");
        }

        if (!lut_config.HasMember("data_identifier") || !lut_config["data_identifier"].IsString())
        {
            return ERR("could not parse LUT configuration: missing or invalid data identifier for LUT initialization");
        }

        std::unique_ptr<GateTypeComponent> init_component = GateTypeComponent::create_init_component(lut_config["data_category"].GetString(), {lut_config["data_identifier"].GetString()});
        return OK(GateTypeComponent::create_lut_component(std::move(init_component), std::string(lut_config["bit_order"].GetString()) == "ascending"));
    }

    Result<std::unique_ptr<GateTypeComponent>> HGLParser::parse_ff_config(const rapidjson::Value& ff_config)
    {
        if (!ff_config.HasMember("state") || !ff_config["state"].IsString())
        {
            return ERR("could not parse flip-flop configuration: missing or invalid state identifier");
        }
        if (!ff_config.HasMember("neg_state") || !ff_config["neg_state"].IsString())
        {
            return ERR("could not parse flip-flop configuration: missing or invalid negated state identifier");
        }
        if (!ff_config.HasMember("next_state") || !ff_config["next_state"].IsString())
        {
            return ERR("could not parse flip-flop configuration: missing or invalid next state function");
        }
        if (!ff_config.HasMember("clocked_on") || !ff_config["clocked_on"].IsString())
        {
            return ERR("could not parse flip-flop configuration: missing or invalid clock function");
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
                return ERR("could not parse flip-flop configuration: missing or invalid data identifier for flip-flop initialization");
            }
            init_component = GateTypeComponent::create_init_component(ff_config["data_category"].GetString(), init_identifiers);
        }
        else if (ff_config.HasMember("data_identifier") && ff_config["data_identifier"].IsString())
        {
            return ERR("could not parse flip-flop configuration: missing or invalid data category for flip-flop initialization");
        }

        std::string state_identifier                       = ff_config["state"].GetString();
        std::string neg_state_identifier                   = ff_config["neg_state"].GetString();
        std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(std::move(init_component), state_identifier, neg_state_identifier);
        assert(state_component != nullptr);

        BooleanFunction next_state_function;
        if (auto res = BooleanFunction::from_string(ff_config["next_state"].GetString()); res.is_error())
        {
            return ERR("could not parse flip-flop configuration: failed parsing next state function from string");
        }
        else
        {
            next_state_function = res.get();
        }
        BooleanFunction clocked_on_function;
        if (auto res = BooleanFunction::from_string(ff_config["clocked_on"].GetString()); res.is_error())
        {
            return ERR("could not parse flip-flop configuration: failed parsing clock function from string");
        }
        else
        {
            clocked_on_function = res.get();
        }

        std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_ff_component(std::move(state_component), next_state_function, clocked_on_function);

        FFComponent* ff_component = component->convert_to<FFComponent>();
        assert(ff_component != nullptr);

        if (ff_config.HasMember("clear_on") && ff_config["clear_on"].IsString())
        {
            if (auto res = BooleanFunction::from_string(ff_config["clear_on"].GetString()); res.is_error())
            {
                return ERR("could not parse flip-flop configuration: failed parsing asynchronous reset function from string");
            }
            else
            {
                ff_component->set_async_reset_function(res.get());
            }
        }

        if (ff_config.HasMember("preset_on") && ff_config["preset_on"].IsString())
        {
            if (auto res = BooleanFunction::from_string(ff_config["preset_on"].GetString()); res.is_error())
            {
                return ERR("could not parse flip-flop configuration: failed parsing asynchronous set function from string");
            }
            else
            {
                ff_component->set_async_set_function(res.get());
            }
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
                return ERR("could not parse flip-flop configuration: failed parsing state on concurrent asynchronous set and reset from string");
            }

            if (const auto behav = enum_from_string<AsyncSetResetBehavior>(ff_config["neg_state_clear_preset"].GetString(), AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
            {
                cp2 = behav;
            }
            else
            {
                return ERR("could not parse flip-flop configuration: failed parsing negated state on concurrent asynchronous set and reset from string");
            }

            ff_component->set_async_set_reset_behavior(cp1, cp2);
        }
        else if ((has_state && !has_neg_state) || (!has_state && has_neg_state))
        {
            return ERR("could not parse flip-flop configuration: missing state or negated state on concurrent asynchronous set and reset");
        }

        return OK(std::move(component));
    }

    Result<std::unique_ptr<GateTypeComponent>> HGLParser::parse_latch_config(const rapidjson::Value& latch_config)
    {
        if (!latch_config.HasMember("state") || !latch_config["state"].IsString())
        {
            return ERR("could not parse latch configuration: missing or invalid state identifier");
        }
        if (!latch_config.HasMember("neg_state") || !latch_config["neg_state"].IsString())
        {
            return ERR("could not parse latch configuration: missing or invalid negated state identifier");
        }

        std::string state_identifier                       = latch_config["state"].GetString();
        std::string neg_state_identifier                   = latch_config["neg_state"].GetString();
        std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(nullptr, state_identifier, neg_state_identifier);
        assert(state_component != nullptr);

        std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_latch_component(std::move(state_component));
        LatchComponent* latch_component              = component->convert_to<LatchComponent>();
        assert(latch_component != nullptr);

        if (latch_config.HasMember("data_in") && latch_config["data_in"].IsString() && latch_config.HasMember("enable_on") && latch_config["enable_on"].IsString())
        {
            if (auto res = BooleanFunction::from_string(latch_config["data_in"].GetString()); res.is_error())
            {
                return ERR("could not parse latch configuration: failed parsing data in function from string");
            }
            else
            {
                latch_component->set_data_in_function(res.get());
            }

            if (auto res = BooleanFunction::from_string(latch_config["enable_on"].GetString()); res.is_error())
            {
                return ERR("could not parse latch configuration: failed parsing enable function from string");
            }
            else
            {
                latch_component->set_enable_function(res.get());
            }
        }
        else if (latch_config.HasMember("data_in") && latch_config["data_in"].IsString())
        {
            return ERR("could not parse latch configuration: missing or invalid enable function");
        }
        else if (latch_config.HasMember("enable_on") && latch_config["enable_on"].IsString())
        {
            return ERR("could not parse latch configuration: missing or invalid data in function");
        }

        if (latch_config.HasMember("clear_on") && latch_config["clear_on"].IsString())
        {
            if (auto res = BooleanFunction::from_string(latch_config["clear_on"].GetString()); res.is_error())
            {
                return ERR("could not parse latch configuration: failed parsing asynchronous reset function from string");
            }
            else
            {
                latch_component->set_async_reset_function(res.get());
            }
        }

        if (latch_config.HasMember("preset_on") && latch_config["preset_on"].IsString())
        {
            if (auto res = BooleanFunction::from_string(latch_config["preset_on"].GetString()); res.is_error())
            {
                return ERR("could not parse latch configuration: failed parsing asynchronous set function from string");
            }
            else
            {
                latch_component->set_async_set_function(res.get());
            }
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
                return ERR("could not parse latch configuration: failed parsing state on concurrent asynchronous set and reset from string");
            }

            if (const auto behav = enum_from_string<AsyncSetResetBehavior>(latch_config["neg_state_clear_preset"].GetString(), AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
            {
                cp2 = behav;
            }
            else
            {
                return ERR("could not parse latch configuration: failed parsing negated state on concurrent asynchronous set and reset from string");
            }

            latch_component->set_async_set_reset_behavior(cp1, cp2);
        }
        else if ((has_state && !has_neg_state) || (!has_state && has_neg_state))
        {
            return ERR("could not parse latch configuration: missing state or negated state on concurrent asynchronous set and reset");
        }

        return OK(std::move(component));
    }

    Result<std::unique_ptr<GateTypeComponent>> HGLParser::parse_ram_config(const rapidjson::Value& ram_config)
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
                return ERR("could not parse RAM configuration: missing or invalid data identifier for RAM initialization");
            }
            sub_component = GateTypeComponent::create_init_component(ram_config["data_category"].GetString(), init_identifiers);
        }
        else if (ram_config.HasMember("data_identifiers") && ram_config["data_identifiers"].IsArray())
        {
            return ERR("could not parse RAM configuration: missing or invalid data category for RAM initialization");
        }

        if (!ram_config.HasMember("bit_size") || !ram_config["bit_size"].IsUint())
        {
            return ERR("could not parse RAM configuration: missing or invalid bit size");
        }

        if (!ram_config.HasMember("ram_ports") || !ram_config["ram_ports"].IsArray())
        {
            return ERR("could not parse RAM configuration: missing or invalid RAM ports");
        }

        for (const auto& ram_port : ram_config["ram_ports"].GetArray())
        {
            if (!ram_port.HasMember("data_group") || !ram_port["data_group"].IsString())
            {
                return ERR("could not parse RAM configuration: missing or invalid data pin groups for RAM port");
            }

            if (!ram_port.HasMember("address_group") || !ram_port["address_group"].IsString())
            {
                return ERR("could not parse RAM configuration: missing or invalid address pin groups for RAM port");
            }

            if (!ram_port.HasMember("clocked_on") || !ram_port["clocked_on"].IsString())
            {
                return ERR("could not parse RAM configuration: missing or invalid clock function for RAM port");
            }

            if (!ram_port.HasMember("enabled_on") || !ram_port["enabled_on"].IsString())
            {
                return ERR("could not parse RAM configuration: missing or invalid enable function for RAM port");
            }

            if (!ram_port.HasMember("is_write") || !ram_port["is_write"].IsBool())
            {
                return ERR("could not parse RAM configuration: missing or invalid write flag for RAM port");
            }

            BooleanFunction clocked_on_function;
            if (auto res = BooleanFunction::from_string(ram_port["clocked_on"].GetString()); res.is_error())
            {
                return ERR("could not parse RAM configuration: failed parsing clock function from string");
            }
            else
            {
                clocked_on_function = res.get();
            }

            BooleanFunction enabled_on_function;
            if (auto res = BooleanFunction::from_string(ram_port["enabled_on"].GetString()); res.is_error())
            {
                return ERR("could not parse RAM configuration: failed parsing clock function from string");
            }
            else
            {
                enabled_on_function = res.get();
            }

            sub_component = GateTypeComponent::create_ram_port_component(
                std::move(sub_component), ram_port["data_group"].GetString(), ram_port["address_group"].GetString(), clocked_on_function, enabled_on_function, ram_port["is_write"].GetBool());
        }

        std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_ram_component(std::move(sub_component), ram_config["bit_size"].GetUint());

        return OK(std::move(component));
    }
}    // namespace hal