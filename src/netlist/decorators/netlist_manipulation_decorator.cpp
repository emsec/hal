#include "hal_core/netlist/decorators/netlist_manipulation_decorator.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    NetlistManipulationDecorator::NetlistManipulationDecorator(Netlist& netlist) : m_netlist(netlist)
    {
    }

    Result<std::monostate> NetlistManipulationDecorator::delete_modules(const std::function<bool(const Module*)>& filter)
    {
        for (auto module : m_netlist.get_modules(filter))
        {
            if (!module->is_top_module())
            {
                if (!m_netlist.delete_module(module))
                {
                    return ERR("could not delete modules of netlist with ID " + std::to_string(m_netlist.get_id()) + ": failed to delete module '" + module->get_name() + "' with ID "
                               + std::to_string(module->get_id()));
                }
            }
        }
        return OK({});
    }

    Result<std::monostate> NetlistManipulationDecorator::replace_gate(Gate* gate, GateType* target_type, const std::map<GatePin*, GatePin*>& pin_map)
    {
        if (gate == nullptr)
        {
            return ERR("could not replace gate: gate is a 'nullptr'");
        }
        if (target_type == nullptr)
        {
            return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": target gate type is a 'nullptr'");
        }

        u32 gate_id                       = gate->get_id();
        std::string gate_name             = gate->get_name();
        std::pair<i32, i32> gate_location = gate->get_location();
        std::vector<Endpoint*> fan_in     = gate->get_fan_in_endpoints();
        std::vector<Endpoint*> fan_out    = gate->get_fan_out_endpoints();
        Module* gate_module               = gate->get_module();
        Grouping* gate_grouping           = gate->get_grouping();
        auto gate_data                    = gate->get_data_map();

        std::map<GatePin*, Net*> in_nets;
        std::map<GatePin*, Net*> out_nets;

        // map new input pins to nets
        for (Endpoint* in_ep : fan_in)
        {
            if (const auto it = pin_map.find(in_ep->get_pin()); it != pin_map.end())
            {
                in_nets[it->second] = in_ep->get_net();
            }
        }

        for (Endpoint* out_ep : fan_out)
        {
            if (const auto it = pin_map.find(out_ep->get_pin()); it != pin_map.end())
            {
                out_nets[it->second] = out_ep->get_net();
            }
        }

        // remove old gate
        if (m_netlist.delete_gate(gate) == false)
        {
            return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to delete old gate of type '" + gate->get_type()->get_name() + "'");
        }
        // create new gate
        Gate* new_gate = m_netlist.create_gate(gate_id, target_type, gate_name, gate_location.first, gate_location.second);
        if (new_gate == nullptr)
        {
            return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to create replacement gate of type '" + target_type->get_name() + "'");
        }

        // reconnect nets
        for (auto [in_pin, in_net] : in_nets)
        {
            if (!in_net->add_destination(new_gate, in_pin))
            {
                return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to reconnect input net '" + in_net->get_name() + "' with ID "
                           + std::to_string(in_net->get_id()) + " to pin '" + in_pin->get_name() + "' of the replacement gate");
            }
        }

        for (const auto& [out_pin, out_net] : out_nets)
        {
            if (!out_net->add_source(new_gate, out_pin))
            {
                return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to reconnect output net '" + out_net->get_name() + "' with ID "
                           + std::to_string(out_net->get_id()) + " to pin '" + out_pin->get_name() + "' of the replacement gate");
            }
        }

        // restore data, module, and grouping
        if (!gate_module->is_top_module())
        {
            gate_module->assign_gate(new_gate);
        }
        if (gate_grouping != nullptr)
        {
            gate_grouping->assign_gate(new_gate);
        }
        if (!gate_data.empty())
        {
            new_gate->set_data_map(gate_data);
        }

        return OK({});
    }

    Result<std::monostate> NetlistManipulationDecorator::connect_gates(Gate* src_gate, GatePin* src_pin, Gate* dst_gate, GatePin* dst_pin)
    {
        if (src_gate == nullptr || dst_gate == nullptr)
        {
            return ERR("could not connect source gate with destination gate: nullptr given for source or destination gate");
        }

        if (src_pin == nullptr || dst_pin == nullptr)
        {
            return ERR("could not connect source gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + " with destination gate '" + dst_gate->get_name() + "' with ID "
                       + std::to_string(dst_gate->get_id()) + ": nullptr given for source or destination pin");
        }

        auto src_net = src_gate->get_fan_out_net(src_pin);
        auto dst_net = dst_gate->get_fan_in_net(dst_pin);

        Net* net;
        if (src_net == nullptr)
        {
            if (dst_net == nullptr)
            {
                const std::string net_name = "gate_" + std::to_string(src_gate->get_id()) + "_to_gate_" + std::to_string(dst_gate->get_id());
                net                        = m_netlist.create_net(net_name);
                if (net == nullptr)
                {
                    return ERR("could not connect source gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + "through pin '" + src_pin->get_name()
                               + "' with destination gate '" + dst_gate->get_name() + "' with ID " + std::to_string(dst_gate->get_id()) + " through pin '" + dst_pin->get_name()
                               + "': failed to create new net with name '" + net_name + "'");
                }

                if (!net->add_destination(dst_gate, dst_pin))
                {
                    return ERR("could not connect source gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + "through pin '" + src_pin->get_name()
                               + "' with destination gate '" + dst_gate->get_name() + "' with ID " + std::to_string(dst_gate->get_id()) + " through pin '" + dst_pin->get_name()
                               + "': failed to add destination to existing net '" + net->get_name() + "' + with ID " + std::to_string(net->get_id()));
                }
            }
            else
            {
                net = dst_net;
            }

            if (!net->add_source(src_gate, src_pin))
            {
                return ERR("could not connect source gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + "through pin '" + src_pin->get_name()
                           + "' with destination gate '" + dst_gate->get_name() + "' with ID " + std::to_string(dst_gate->get_id()) + " through pin '" + dst_pin->get_name()
                           + "': failed to add source to existing net '" + net->get_name() + "' + with ID " + std::to_string(net->get_id()));
            }
        }
        else
        {
            if (dst_net == nullptr)
            {
                net = src_net;
            }
            else
            {
                return ERR("could not connect source gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + "through pin '" + src_pin->get_name()
                           + "' with destination gate '" + dst_gate->get_name() + "' with ID " + std::to_string(dst_gate->get_id()) + " through pin '" + dst_pin->get_name()
                           + "': both source gate and destination gate aleady have a net assigned to the specified pins");
            }

            if (!net->add_destination(dst_gate, dst_pin))
            {
                return ERR("could not connect source gate '" + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()) + "through pin '" + src_pin->get_name()
                           + "' with destination gate '" + dst_gate->get_name() + "' with ID " + std::to_string(dst_gate->get_id()) + " through pin '" + dst_pin->get_name()
                           + "': failed to add destination to existing net '" + net->get_name() + "' + with ID " + std::to_string(net->get_id()));
            }
        }

        return OK({});
    }

    Result<std::monostate> NetlistManipulationDecorator::connect_nets(Net* master_net, Net* slave_net)
    {
        if (master_net == nullptr || slave_net == nullptr)
        {
            return ERR("");
        }

        // safe all module pin information from the net soon to be deleted
        std::map<Module*, std::tuple<std::string, std::string, u32, PinDirection, PinType>> module_pins;
        for (const auto& m : m_netlist.get_modules())
        {
            if (const auto pin = m->get_pin_by_net(slave_net); pin != nullptr)
            {
                module_pins.insert({m, {pin->get_group().first->get_name(), pin->get_name(), pin->get_group().second, pin->get_direction(), pin->get_type()}});
            }
        }

        for (auto* src_ep : slave_net->get_sources())
        {
            auto* src_gate = src_ep->get_gate();
            auto* src_pin  = src_ep->get_pin();

            slave_net->remove_source(src_ep);
            master_net->add_source(src_gate, src_pin);
        }

        for (auto* dst_ep : slave_net->get_destinations())
        {
            auto* dst_gate = dst_ep->get_gate();
            auto* dst_pin  = dst_ep->get_pin();

            slave_net->remove_destination(dst_ep);
            master_net->add_destination(dst_gate, dst_pin);
        }

        if (slave_net->is_global_input_net())
        {
            m_netlist.mark_global_input_net(master_net);
        }
        if (slave_net->is_global_output_net())
        {
            m_netlist.mark_global_output_net(master_net);
        }

        m_netlist.delete_net(slave_net);

        for (auto& [m, pin_info] : module_pins)
        {
            const std::string pingroup_name = std::get<0>(pin_info);
            const std::string pin_name      = std::get<1>(pin_info);
            const PinType pin_type          = std::get<4>(pin_info);

            if (auto* pin = m->get_pin_by_net(master_net); pin != nullptr)
            {
                pin->set_name(pin_name);
                pin->set_type(pin_type);

                // remove pin from current pin group
                auto current_pin_group = pin->get_group().first;
                current_pin_group->remove_pin(pin).get();
                if (current_pin_group->get_pins().empty())
                {
                    m->delete_pin_group(current_pin_group);
                }

                // check for existing pingroup otherwise create it
                auto pin_groups = m->get_pin_groups([pingroup_name](const auto& pg) { return pg->get_name() == pingroup_name; });
                PinGroup<ModulePin>* pin_group;
                if (pin_groups.empty())
                {
                    pin_group = m->create_pin_group(pingroup_name, {}, PinDirection::none, pin_type).get();
                }
                else
                {
                    pin_group = pin_groups.front();
                }

                pin_group->assign_pin(pin).get();
                pin_group->move_pin(pin, std::get<2>(pin_info)).get();
            }
            else
            {
                return ERR("Cannot replace buffers: Failed to get pin " + pin_name + " at module " + m->get_name() + " with ID " + std::to_string(m->get_id()));
            }
        }

        return OK({});
    }
}    // namespace hal