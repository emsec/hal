#include "hgl_writer/hgl_writer.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
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

    bool HGLWriter::write_gate_library(rapidjson::Document& document, const GateLibrary* gate_lib)
    {
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        // library name
        document.AddMember("library", gate_lib->get_name(), allocator);

        // gate location data specification
        rapidjson::Value gate_locs(rapidjson::kObjectType);
        gate_locs.AddMember("data_category", gate_lib->get_gate_location_data_category(), allocator);
        const std::pair<std::string, std::string>& location_identifiers = gate_lib->get_gate_location_data_identifiers();
        gate_locs.AddMember("data_x_identifier", location_identifiers.first, allocator);
        gate_locs.AddMember("data_y_identifier", location_identifiers.second, allocator);
        document.AddMember("gate_locations", gate_locs, allocator);

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
            std::set<GateTypeProperty> properties = gt->get_properties();

            rapidjson::Value bts(rapidjson::kArrayType);

            for (const auto& property : properties)
            {
                std::string bt_str = enum_to_string<GateTypeProperty>(property);
                bts.PushBack(rapidjson::Value{}.SetString(bt_str.c_str(), bt_str.length(), allocator), allocator);
            }

            cell.AddMember("types", bts, allocator);

            std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();

            // lut_config, ff_config, latch_config, ram_config
            if (LUTComponent* lut_component = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }); lut_component != nullptr)
            {
                rapidjson::Value lut_config(rapidjson::kObjectType);

                InitComponent* init_component = lut_component->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                if (init_component == nullptr)
                {
                    log_error("hgl_writer", "missing InitComponent for LUT initialization data of gate type '{}'.", gt->get_name());
                    return false;
                }

                // bit_order
                if (lut_component->is_init_ascending())
                {
                    lut_config.AddMember("bit_order", "ascending", allocator);
                }
                else
                {
                    lut_config.AddMember("bit_order", "descending", allocator);
                }

                // data_category and data_identifier
                lut_config.AddMember("data_category", init_component->get_init_category(), allocator);
                lut_config.AddMember("data_identifier", init_component->get_init_identifiers().front(), allocator);

                cell.AddMember("lut_config", lut_config, allocator);
            }
            else if (FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); }); ff_component != nullptr)
            {
                rapidjson::Value ff_config(rapidjson::kObjectType);

                StateComponent* state_component = ff_component->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });
                if (state_component == nullptr)
                {
                    log_error("hgl_writer", "missing StateComponent for FF state identifiers of gate type '{}'.", gt->get_name());
                    return false;
                }

                ff_config.AddMember("state", state_component->get_state_identifier(), allocator);
                ff_config.AddMember("neg_state", state_component->get_neg_state_identifier(), allocator);

                InitComponent* init_component = state_component->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                if (init_component != nullptr)
                {
                    // data_category, data_identifier
                    ff_config.AddMember("data_category", init_component->get_init_category(), allocator);
                    ff_config.AddMember("data_identifier", init_component->get_init_identifiers().front(), allocator);
                }

                // next_state, clocked_on, clear_on, preset_on
                ff_config.AddMember("next_state", ff_component->get_next_state_function().to_string(), allocator);
                ff_config.AddMember("clocked_on", ff_component->get_clock_function().to_string(), allocator);
                if (BooleanFunction bf = ff_component->get_async_reset_function(); !bf.is_empty())
                {
                    ff_config.AddMember("clear_on", bf.to_string(), allocator);
                }
                if (BooleanFunction bf = ff_component->get_async_set_function(); !bf.is_empty())
                {
                    ff_config.AddMember("preset_on", bf.to_string(), allocator);
                }

                std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> cp_behav = ff_component->get_async_set_reset_behavior();
                if (cp_behav.first != AsyncSetResetBehavior::undef)
                {
                    ff_config.AddMember("state_clear_preset", enum_to_string<AsyncSetResetBehavior>(cp_behav.first), allocator);
                }
                if (cp_behav.second != AsyncSetResetBehavior::undef)
                {
                    ff_config.AddMember("neg_state_clear_preset", enum_to_string<AsyncSetResetBehavior>(cp_behav.second), allocator);
                }

                cell.AddMember("ff_config", ff_config, allocator);
            }
            else if (LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* c) { return LatchComponent::is_class_of(c); }); latch_component != nullptr)
            {
                rapidjson::Value latch_config(rapidjson::kObjectType);

                StateComponent* state_component = latch_component->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });
                if (state_component == nullptr)
                {
                    log_error("hgl_writer", "missing StateComponent for latch state identifiers of gate type '{}'.", gt->get_name());
                    return false;
                }

                latch_config.AddMember("state", state_component->get_state_identifier(), allocator);
                latch_config.AddMember("neg_state", state_component->get_neg_state_identifier(), allocator);

                // next_state, clocked_on, clear_on, preset_on
                if (BooleanFunction bf = latch_component->get_data_in_function(); !bf.is_empty())
                {
                    latch_config.AddMember("data_in", bf.to_string(), allocator);
                }
                if (BooleanFunction bf = latch_component->get_enable_function(); !bf.is_empty())
                {
                    latch_config.AddMember("enable_on", bf.to_string(), allocator);
                }
                if (BooleanFunction bf = latch_component->get_async_reset_function(); !bf.is_empty())
                {
                    latch_config.AddMember("clear_on", bf.to_string(), allocator);
                }
                if (BooleanFunction bf = latch_component->get_async_set_function(); !bf.is_empty())
                {
                    latch_config.AddMember("preset_on", bf.to_string(), allocator);
                }

                std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> cp_behav = latch_component->get_async_set_reset_behavior();
                if (cp_behav.first != AsyncSetResetBehavior::undef)
                {
                    latch_config.AddMember("state_clear_preset", enum_to_string<AsyncSetResetBehavior>(cp_behav.first), allocator);
                }
                if (cp_behav.second != AsyncSetResetBehavior::undef)
                {
                    latch_config.AddMember("neg_state_clear_preset", enum_to_string<AsyncSetResetBehavior>(cp_behav.second), allocator);
                }

                cell.AddMember("latch_config", latch_config, allocator);
            }
            else if (RAMComponent* ram_component = gt->get_component_as<RAMComponent>([](const GateTypeComponent* c) { return RAMComponent::is_class_of(c); }); ram_component != nullptr)
            {
                rapidjson::Value ram_config(rapidjson::kObjectType);

                InitComponent* init_component = ram_component->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                if (init_component == nullptr)
                {
                    return false;
                }

                // data_category, data_identifier
                ram_config.AddMember("data_category", init_component->get_init_category(), allocator);

                rapidjson::Value identifiers(rapidjson::kArrayType);
                for (const std::string& identifier : init_component->get_init_identifiers())
                {
                    identifiers.PushBack(rapidjson::Value{}.SetString(identifier.c_str(), identifier.length(), allocator), allocator);
                }
                ram_config.AddMember("data_identifiers", identifiers, allocator);
                ram_config.AddMember("bit_size", ram_component->get_bit_size(), allocator);

                rapidjson::Value ram_ports(rapidjson::kArrayType);
                for (const GateTypeComponent* component : ram_component->get_components([](const GateTypeComponent* c) { return RAMPortComponent::is_class_of(c); }))
                {
                    const RAMPortComponent* port_component = component->convert_to<RAMPortComponent>();
                    if (port_component == nullptr)
                    {
                        return false;
                    }

                    rapidjson::Value port(rapidjson::kObjectType);
                    port.AddMember("data_group", port_component->get_data_group(), allocator);
                    port.AddMember("address_group", port_component->get_address_group(), allocator);
                    port.AddMember("clocked_on", port_component->get_clock_function().to_string(), allocator);
                    port.AddMember("enabled_on", port_component->get_enable_function().to_string(), allocator);
                    port.AddMember("is_write", port_component->is_write_port(), allocator);
                    ram_ports.PushBack(port, allocator);
                }
                ram_config.AddMember("ram_ports", ram_ports, allocator);
                cell.AddMember("ram_config", ram_config, allocator);
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

            // groups (avoid writing pin groups automatically created for each pin)
            const std::vector<PinGroup<GatePin>*> pin_groups = gt->get_pin_groups([](const PinGroup<GatePin>* pg) {
                if (pg->size() == 1)
                {
                    const GatePin* p = pg->get_pins().front();
                    return pg->get_name() != p->get_name() || pg->get_direction() != p->get_direction() || pg->get_type() != p->get_type() || pg->get_start_index() != 0 || pg->is_ascending() != false;
                }
                return true;
            });
            if (!pin_groups.empty())
            {
                rapidjson::Value groups(rapidjson::kArrayType);

                for (const auto* pin_group : pin_groups)
                {
                    rapidjson::Value group(rapidjson::kObjectType);

                    group.AddMember("name", pin_group->get_name(), allocator);

                    rapidjson::Value group_pin_array(rapidjson::kArrayType);
                    u32 i         = pin_group->get_start_index();
                    i32 direction = pin_group->is_ascending() ? 1 : -1;
                    for (const auto* pin : pin_group->get_pins())
                    {
                        rapidjson::Value group_pin(rapidjson::kObjectType);
                        std::string index       = std::to_string(i);
                        const std::string& name = pin->get_name();
                        rapidjson::Value v_index(index.c_str(), index.size(), allocator);
                        rapidjson::Value v_pin(name.c_str(), name.size(), allocator);
                        group_pin.AddMember(v_index, v_pin, allocator);
                        group_pin_array.PushBack(group_pin, allocator);
                        i += direction;
                    }
                    group.AddMember("pins", group_pin_array, allocator);

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
        for (const auto* pin : gt->get_pins())
        {
            PinCtx res_pin;
            res_pin.name      = pin->get_name();
            res_pin.direction = enum_to_string(pin->get_direction());
            res_pin.type      = enum_to_string(pin->get_type());

            if (pin->get_direction() == PinDirection::output || pin->get_direction() == PinDirection::inout)
            {
                if (const auto it = functions.find(pin->get_name()); it != functions.end())
                {
                    res_pin.function = it->second.to_string();
                }

                if (const auto it = functions.find(pin->get_name() + "_undefined"); it != functions.end())
                {
                    res_pin.x_function = it->second.to_string();
                }

                if (const auto it = functions.find(pin->get_name() + "_tristate"); it != functions.end())
                {
                    res_pin.z_function = it->second.to_string();
                }
            }

            res.push_back(res_pin);
        }

        return res;
    }
}    // namespace hal