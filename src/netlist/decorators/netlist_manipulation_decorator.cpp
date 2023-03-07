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

    Result<std::monostate> NetlistManipulationDecorator::replace_gate(Gate* gate, GateType* target_type, std::map<GatePin*, GatePin*> pin_map)
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
}    // namespace hal