#include "hal_core/netlist/netlist_internal_manager.h"

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    NetlistInternalManager::NetlistInternalManager(Netlist* nl, EventHandler* eh)
    {
        m_netlist       = nl;
        m_event_handler = eh;
        assert(nl != nullptr);
        assert(eh != nullptr);
    }

    //######################################################################
    //###                      netlist                                   ###
    //######################################################################

    Result<std::unique_ptr<Netlist>> NetlistInternalManager::copy_netlist(const Netlist* nl) const
    {
        std::unique_ptr<Netlist> c_netlist = netlist_factory::create_netlist(nl->m_gate_library);
        if (c_netlist == nullptr)
        {
            return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to create netlist");
        }

        c_netlist->enable_automatic_net_checks(false);

        // manager, netlist_id, and top_module are set in the constructor

        // copy design name, device name, input filename
        c_netlist->set_design_name(nl->get_design_name());
        c_netlist->set_device_name(nl->get_device_name());
        c_netlist->set_input_filename(nl->get_input_filename());

        // copy nets
        for (const Net* net : nl->m_nets)
        {
            Net* c_net = c_netlist->create_net(net->m_id, net->m_name);
            if (c_net == nullptr)
            {
                return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to create copied net '" + net->m_name + "' with ID " + std::to_string(net->m_id));
            }
            c_net->m_data = net->m_data;
        }

        // copy gates
        for (const Gate* gate : nl->m_gates)
        {
            Gate* c_gate = c_netlist->create_gate(gate->m_id, gate->m_type, gate->m_name, gate->m_x, gate->m_y);
            if (c_gate == nullptr)
            {
                return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to create copied gate '" + gate->m_name + "' with ID " + std::to_string(gate->m_id));
            }

            for (const auto& [name, func] : gate->get_boolean_functions(true))
            {
                c_gate->add_boolean_function(name, func);
            }

            for (const Endpoint* in_point : gate->get_fan_in_endpoints())
            {
                const auto net_id = in_point->get_net()->m_id;
                auto c_net        = c_netlist->get_net_by_id(net_id);
                if (c_net == nullptr)
                {
                    return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to get copied net by ID " + std::to_string(net_id));
                }

                if (!c_net->add_destination(c_gate, in_point->get_pin()))
                {
                    return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to add destination to copied net '" + c_net->m_name + "' with ID "
                               + std::to_string(c_net->m_id));
                }
            }

            for (const Endpoint* out_point : gate->get_fan_out_endpoints())
            {
                const auto net_id = out_point->get_net()->m_id;
                auto c_net        = c_netlist->get_net_by_id(net_id);
                if (c_net == nullptr)
                {
                    return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to get copied net by ID " + std::to_string(net_id));
                }

                if (!c_net->add_source(c_gate, out_point->get_pin()))
                {
                    return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to add source to copied net '" + c_net->m_name + "' with ID "
                               + std::to_string(c_net->m_id));
                }
            }

            c_gate->m_data = gate->m_data;
        }

        // copy modules
        for (const Module* module : nl->m_modules)
        {
            // ignore top module, since this is already created by the constructor
            if (module->m_id == 1)
            {
                c_netlist->m_top_module->m_data = module->get_data_map();
                c_netlist->m_top_module->m_type = module->m_type;
                continue;
            }

            std::vector<Gate*> c_gates;
            for (const Gate* gate : module->m_gates)
            {
                // find gates of module in the copied netlist by id
                Gate* c_gate = c_netlist->get_gate_by_id(gate->m_id);
                if (c_gate == nullptr)
                {
                    return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to get copied gate by ID " + std::to_string(gate->m_id));
                }
                c_gates.push_back(c_gate);
            }

            // create all modules with the top module as parent module and update later
            Module* c_module = c_netlist->create_module(module->m_id, module->m_name, c_netlist->m_top_module, c_gates);
            if (c_module == nullptr)
            {
                return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to create copied module '" + module->m_name + "' with ID " + std::to_string(module->m_id));
            }

            c_module->m_data = module->m_data;
            c_module->m_type = module->m_type;
        }

        // update parent_module in modules
        for (const Module* module : nl->m_modules)
        {
            // ignore top_module
            if (module->m_parent == nullptr)
            {
                continue;
            }

            // find parent and child module in the copied netlist by id
            const u32 module_id = module->m_id;
            const u32 parent_id = module->m_parent->m_id;
            Module* c_module    = c_netlist->get_module_by_id(module_id);
            Module* c_parent    = c_netlist->get_module_by_id(parent_id);

            if (!c_module->set_parent_module(c_parent))
            {
                return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to set copied module '" + c_parent->m_name + "' with ID " + std::to_string(module->m_id)
                           + " as parent module of '" + c_module->m_name + "' with ID " + std::to_string(c_module->m_id));
            }
        }

        // copy groupings
        for (const Grouping* grouping : nl->m_groupings)
        {
            Grouping* c_grouping = c_netlist->create_grouping(grouping->m_id, grouping->m_name);
            if (c_grouping == nullptr)
            {
                return ERR("could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to create copied grouping '" + grouping->m_name + "' with ID "
                           + std::to_string(grouping->m_id));
            }

            for (const Module* module : grouping->m_modules)
            {
                const u32 module_id = module->m_id;
                c_grouping->assign_module_by_id(module_id);
            }

            for (const Net* net : grouping->m_nets)
            {
                const u32 net_id = net->m_id;
                c_grouping->assign_net_by_id(net_id);
            }

            for (const Gate* gate : grouping->m_gates)
            {
                const u32 gate_id = gate->m_id;
                c_grouping->assign_gate_by_id(gate_id);
            }
        }

        // mark globals
        for (const Net* global_input_net : nl->m_global_input_nets)
        {
            Net* c_global_input_net = c_netlist->get_net_by_id(global_input_net->m_id);
            c_netlist->mark_global_input_net(c_global_input_net);
        }
        for (const Net* global_output_net : nl->m_global_output_nets)
        {
            Net* c_global_output_net = c_netlist->get_net_by_id(global_output_net->m_id);
            c_netlist->mark_global_output_net(c_global_output_net);
        }
        for (const Gate* gnd_gate : nl->m_gnd_gates)
        {
            Gate* c_gnd_gate = c_netlist->get_gate_by_id(gnd_gate->m_id);
            c_netlist->mark_gnd_gate(c_gnd_gate);
        }
        for (const Gate* vcc_gate : nl->m_vcc_gates)
        {
            Gate* c_vcc_gate = c_netlist->get_gate_by_id(vcc_gate->m_id);
            c_netlist->mark_vcc_gate(c_vcc_gate);
        }

        c_netlist->m_design_name = nl->m_design_name;
        c_netlist->m_device_name = nl->m_device_name;
        c_netlist->m_file_name   = nl->m_file_name;

        // update ids last, after all the creation
        c_netlist->m_next_gate_id  = nl->m_next_gate_id;
        c_netlist->m_used_gate_ids = nl->m_used_gate_ids;
        c_netlist->m_free_gate_ids = nl->m_free_gate_ids;

        c_netlist->m_next_net_id  = nl->m_next_net_id;
        c_netlist->m_used_net_ids = nl->m_used_net_ids;
        c_netlist->m_free_net_ids = nl->m_free_net_ids;

        c_netlist->m_next_module_id  = nl->m_next_module_id;
        c_netlist->m_used_module_ids = nl->m_used_module_ids;
        c_netlist->m_free_module_ids = nl->m_free_module_ids;

        c_netlist->m_next_grouping_id  = nl->m_next_grouping_id;
        c_netlist->m_used_grouping_ids = nl->m_used_grouping_ids;
        c_netlist->m_free_grouping_ids = nl->m_free_grouping_ids;

        // copy module port names
        for (Module* module : nl->m_modules)
        {
            Module* c_module = c_netlist->get_module_by_id(module->m_id);
            c_module->update_nets();

            for (const std::unique_ptr<PinGroup<ModulePin>>& pin_group : module->m_pin_groups)
            {
                std::vector<ModulePin*> c_pins;
                for (ModulePin* pin : pin_group->get_pins())
                {
                    if (const auto res = c_module->create_pin(pin->get_id(), pin->get_name(), c_netlist->get_net_by_id(pin->get_net()->m_id), PinType::none, false); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(),
                                          "could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to create copied module pin '" + pin->get_name() + "' of module '"
                                              + c_module->m_name + "' with ID " + std::to_string(c_module->m_id));
                    }
                    else
                    {
                        c_pins.push_back(res.get());
                    }
                }

                if (const auto res = c_module->create_pin_group(
                        pin_group->get_id(), pin_group->get_name(), c_pins, pin_group->get_direction(), pin_group->get_type(), pin_group->is_ascending(), pin_group->get_start_index());
                    res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not copy netlist with ID " + std::to_string(nl->get_id()) + ": failed to create copied module pin group '" + pin_group->get_name() + "' of module '"
                                          + c_module->m_name + "' with ID " + std::to_string(c_module->m_id));
                }
            }

            c_module->m_next_input_index  = module->m_next_input_index;
            c_module->m_next_inout_index  = module->m_next_inout_index;
            c_module->m_next_output_index = module->m_next_output_index;
        }

        c_netlist->enable_automatic_net_checks(true);
        return OK(std::move(c_netlist));
    }

    //######################################################################
    //###                      gates                                     ###
    //######################################################################

    Gate* NetlistInternalManager::create_gate(const u32 id, GateType* gt, const std::string& name, i32 x, i32 y)
    {
        if (id == 0)
        {
            log_error("gate", "ID 0 represents an invalid gate ID.");
            return nullptr;
        }
        if (m_netlist->m_used_gate_ids.find(id) != m_netlist->m_used_gate_ids.end())
        {
            log_error("gate", "gate ID {} is already taken in netlist with ID {}.", id, m_netlist->m_netlist_id);
            return nullptr;
        }
        if (gt == nullptr)
        {
            log_error("gate", "nullptr given for gate type.", id);
            return nullptr;
        }
        if (this->is_gate_type_invalid(gt))
        {
            log_error("gate", "gate type '{}' with ID {} is invalid.", gt->get_name(), gt->get_id());
            return nullptr;
        }
        if (utils::trim(name).empty())
        {
            log_error("gate", "gate name cannot be empty.");
            return nullptr;
        }

        auto new_gate = std::unique_ptr<Gate>(new Gate(this, m_event_handler, id, gt, name, x, y));

        auto free_id_it = m_netlist->m_free_gate_ids.find(id);
        if (free_id_it != m_netlist->m_free_gate_ids.end())
        {
            m_netlist->m_free_gate_ids.erase(free_id_it);
        }

        m_netlist->m_used_gate_ids.insert(id);

        // add gate to top module
        new_gate->m_module = m_netlist->m_top_module;

        auto raw = new_gate.get();

        m_netlist->m_gates_map[id] = std::move(new_gate);
        m_netlist->m_gates_set.insert(raw);
        m_netlist->m_gates.push_back(raw);

        m_netlist->m_top_module->m_gates_map[id] = raw;
        m_netlist->m_top_module->m_gates.push_back(raw);

        // notify
        m_event_handler->notify(ModuleEvent::event::gate_assigned, m_netlist->m_top_module, id);
        m_event_handler->notify(GateEvent::event::created, raw);

        return raw;
    }

    bool NetlistInternalManager::delete_gate(Gate* gate)
    {
        if (!m_netlist->is_gate_in_netlist(gate))
        {
            return false;
        }

        for (auto ep : gate->get_fan_out_endpoints())
        {
            if (!net_remove_source(ep->get_net(), ep))
            {
                return false;
            }
        }

        for (auto ep : gate->get_fan_in_endpoints())
        {
            if (!net_remove_destination(ep->get_net(), ep))
            {
                return false;
            }
        }

        // remove from grouping
        if (Grouping* g = gate->get_grouping(); g != nullptr)
        {
            g->remove_gate(gate);
        }

        // check global_gnd and global_vcc gates
        m_netlist->unmark_gnd_gate(gate);
        m_netlist->unmark_vcc_gate(gate);

        // remove gate from modules
        gate->m_module->m_gates_map.erase(gate->m_module->m_gates_map.find(gate->get_id()));
        utils::unordered_vector_erase(gate->m_module->m_gates, gate);

        auto it  = m_netlist->m_gates_map.find(gate->get_id());
        auto ptr = std::move(it->second);
        m_netlist->m_gates_map.erase(it);
        m_netlist->m_gates_set.erase(gate);
        utils::unordered_vector_erase(m_netlist->m_gates, gate);

        // free ids
        m_netlist->m_free_gate_ids.insert(gate->get_id());
        m_netlist->m_used_gate_ids.erase(gate->get_id());

        m_event_handler->notify(ModuleEvent::event::gate_removed, gate->m_module, gate->get_id());
        m_event_handler->notify(GateEvent::event::removed, gate);

        return true;
    }

    bool NetlistInternalManager::is_gate_type_invalid(GateType* gt) const
    {
        return !m_netlist->m_gate_library->contains_gate_type(gt);
    }

    //######################################################################
    //###                       nets                                     ###
    //######################################################################

    Net* NetlistInternalManager::create_net(const u32 id, const std::string& name)
    {
        if (id == 0)
        {
            log_error("net", "ID 0 represents an invalid net ID.");
            return nullptr;
        }
        if (m_netlist->m_used_net_ids.find(id) != m_netlist->m_used_net_ids.end())
        {
            log_error("net", "net ID {} is already taken in netlist with ID {}.", id, m_netlist->m_netlist_id);
            return nullptr;
        }
        if (utils::trim(name).empty())
        {
            log_error("net", "net name cannot be empty.");
            return nullptr;
        }

        auto new_net = std::unique_ptr<Net>(new Net(this, m_event_handler, id, name));

        auto free_id_it = m_netlist->m_free_net_ids.find(id);
        if (free_id_it != m_netlist->m_free_net_ids.end())
        {
            m_netlist->m_free_net_ids.erase(free_id_it);
        }

        m_netlist->m_used_net_ids.insert(id);

        // add net to netlist
        auto raw                  = new_net.get();
        m_netlist->m_nets_map[id] = std::move(new_net);
        m_netlist->m_nets_set.insert(raw);
        m_netlist->m_nets.push_back(raw);

        // notify
        m_event_handler->notify(NetEvent::event::created, raw);

        return raw;
    }

    bool NetlistInternalManager::delete_net(Net* net)
    {
        if (!m_netlist->is_net_in_netlist(net))
        {
            return false;
        }

        auto dsts = net->m_destinations_raw;
        for (auto dst : dsts)
        {
            if (!this->net_remove_destination_internal(net, dst))
            {
                return false;
            }
        }

        auto srcs = net->m_sources_raw;
        for (auto src : srcs)
        {
            if (!this->net_remove_source_internal(net, src))
            {
                return false;
            }
        }

        // remove from grouping
        if (Grouping* g = net->get_grouping(); g != nullptr)
        {
            g->remove_net(net);
        }

        // check global_input and global_output gates
        m_netlist->unmark_global_input_net(net);
        m_netlist->unmark_global_output_net(net);

        // remove net from netlist
        auto it  = m_netlist->m_nets_map.find(net->get_id());
        auto ptr = std::move(it->second);
        m_netlist->m_nets_map.erase(it);
        m_netlist->m_nets_set.erase(net);
        utils::unordered_vector_erase(m_netlist->m_nets, net);

        m_netlist->m_free_net_ids.insert(net->get_id());
        m_netlist->m_used_net_ids.erase(net->get_id());

        m_event_handler->notify(NetEvent::event::removed, net);

        return true;
    }

    Endpoint* NetlistInternalManager::net_add_source(Net* net, Gate* gate, GatePin* pin)
    {
        if (!m_netlist->is_net_in_netlist(net) || !m_netlist->is_gate_in_netlist(gate) || pin == nullptr)
        {
            return nullptr;
        }

        if (net->is_a_source(gate, pin))
        {
            log_error("net",
                      "pin '{}' of gate '{}' with ID {} is already a source of net '{}' with ID {} in netlist with ID {}.",
                      pin->get_name(),
                      gate->get_name(),
                      gate->get_id(),
                      net->get_name(),
                      net->get_id(),
                      m_netlist->m_netlist_id);
            return nullptr;
        }

        // check whether pin is valid for this gate
        auto output_pins = gate->get_type()->get_output_pins();

        if ((std::find(output_pins.begin(), output_pins.end(), pin) == output_pins.end()))
        {
            log_error("net", "gate '{}' with ID {} has no output pin called '{}' in netlist with ID {}.", gate->get_name(), gate->get_id(), pin->get_name(), m_netlist->m_netlist_id);
            return nullptr;
        }

        // check whether src has already an assigned net
        if (auto fan_out_net = gate->get_fan_out_net(pin); fan_out_net != nullptr)
        {
            log_error("net",
                      "gate '{}' with ID {} is already connected to net '{}' with ID {} at output pin '{}', cannot assign new net '{}' with ID {} in netlist with ID {}.",
                      gate->get_name(),
                      gate->get_id(),
                      fan_out_net->get_name(),
                      fan_out_net->get_id(),
                      pin->get_name(),
                      net->get_name(),
                      net->get_id(),
                      m_netlist->m_netlist_id);
            return nullptr;
        }

        auto new_endpoint     = std::unique_ptr<Endpoint>(new Endpoint(gate, pin, net, false));
        auto new_endpoint_raw = new_endpoint.get();
        net->m_sources.push_back(std::move(new_endpoint));
        net->m_sources_raw.push_back(new_endpoint_raw);
        gate->m_out_endpoints.push_back(new_endpoint_raw);
        gate->m_out_nets.push_back(net);

        // update internal nets and port nets
        if (m_net_checks_enabled)
        {
            if (!gate->get_module()->check_net(net, true))
            {
                return nullptr;
            }

            for (Endpoint* ep : net->get_destinations())
            {
                if (!ep->get_gate()->get_module()->check_net(net, true))
                {
                    return nullptr;
                }
            }
        }

        m_event_handler->notify(NetEvent::event::src_added, net, gate->get_id());

        return new_endpoint_raw;
    }

    bool NetlistInternalManager::net_remove_source_internal(Net* net, Endpoint* ep)
    {
        auto gate = ep->get_gate();

        if (!m_netlist->is_net_in_netlist(net) || !m_netlist->is_gate_in_netlist(gate) || !net->is_a_source(ep))
        {
            return false;
        }

        for (u32 i = 0; i < net->m_sources.size(); ++i)
        {
            if (net->m_sources_raw[i] == ep)
            {
                utils::unordered_vector_erase(gate->m_out_endpoints, ep);
                utils::unordered_vector_erase(gate->m_out_nets, net);
                net->m_sources[i] = std::move(net->m_sources.back());
                net->m_sources.pop_back();
                net->m_sources_raw[i] = net->m_sources_raw.back();
                net->m_sources_raw.pop_back();
                m_event_handler->notify(NetEvent::event::src_removed, net, gate->get_id());
                return true;
            }
        }

        return false;
    }

    bool NetlistInternalManager::net_remove_source(Net* net, Endpoint* ep)
    {
        auto gate = ep->get_gate();

        if (!net_remove_source_internal(net, ep))
        {
            log_warning("net",
                        "output pin '{}' of gate '{}' with ID {} is not a source of net '{}' with ID {} in netlist with ID {}",
                        ep->get_pin()->get_name(),
                        gate->get_name(),
                        gate->get_id(),
                        net->get_name(),
                        net->get_id(),
                        m_netlist->m_netlist_id);
        }
        else
        {
            // update internal nets and port nets
            if (m_net_checks_enabled)
            {
                if (!gate->get_module()->check_net(net, true))
                {
                    return false;
                }

                for (Endpoint* dst : net->get_destinations())
                {
                    if (!dst->get_gate()->get_module()->check_net(net, true))
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    Endpoint* NetlistInternalManager::net_add_destination(Net* net, Gate* gate, GatePin* pin)
    {
        if (!m_netlist->is_net_in_netlist(net) || !m_netlist->is_gate_in_netlist(gate) || pin == nullptr)
        {
            return nullptr;
        }

        if (net->is_a_destination(gate, pin))
        {
            log_error("net",
                      "pin '{}' of gate '{}' with ID {} is already a destination of net '{}' with ID {} in netlist with ID {}.",
                      pin->get_name(),
                      gate->get_name(),
                      gate->get_id(),
                      net->get_name(),
                      net->get_id(),
                      m_netlist->m_netlist_id);
            return nullptr;
        }

        // check whether pin is valid for this gate
        auto input_pins = gate->get_type()->get_input_pins();

        if ((std::find(input_pins.begin(), input_pins.end(), pin) == input_pins.end()))
        {
            log_error("net", "gate '{}' with ID {} has no input pin called '{}' in netlist with ID {}.", gate->get_name(), gate->get_id(), pin->get_name(), m_netlist->m_netlist_id);
            return nullptr;
        }

        // check whether dst has already an assigned net
        if (auto fan_in_net = gate->get_fan_in_net(pin); fan_in_net != nullptr)
        {
            log_error("net",
                      "gate '{}' with ID {} is already connected to net '{}' with ID {} at input pin '{}', cannot assign new net '{}' with ID {} in netlist with ID {}.",
                      gate->get_name(),
                      gate->get_id(),
                      fan_in_net->get_name(),
                      fan_in_net->get_id(),
                      pin->get_name(),
                      net->get_name(),
                      net->get_id(),
                      m_netlist->m_netlist_id);
            return nullptr;
        }

        std::unique_ptr<Endpoint> new_endpoint = std::unique_ptr<Endpoint>(new Endpoint(gate, pin, net, true));
        Endpoint* new_endpoint_raw             = new_endpoint.get();
        net->m_destinations.push_back(std::move(new_endpoint));
        net->m_destinations_raw.push_back(new_endpoint_raw);
        gate->m_in_endpoints.push_back(new_endpoint_raw);
        gate->m_in_nets.push_back(net);

        // update internal nets and port nets
        if (m_net_checks_enabled)
        {
            if (!gate->get_module()->check_net(net, true))
            {
                return nullptr;
            }

            for (Endpoint* ep : net->get_sources())
            {
                if (!ep->get_gate()->get_module()->check_net(net, true))
                {
                    return nullptr;
                }
            }
        }

        m_event_handler->notify(NetEvent::event::dst_added, net, gate->get_id());

        return new_endpoint_raw;
    }

    bool NetlistInternalManager::net_remove_destination_internal(Net* net, Endpoint* ep)
    {
        auto gate = ep->get_gate();

        if (!m_netlist->is_net_in_netlist(net) || !m_netlist->is_gate_in_netlist(gate) || !net->is_a_destination(ep))
        {
            return false;
        }

        for (u32 i = 0; i < net->m_destinations.size(); ++i)
        {
            if (net->m_destinations_raw[i] == ep)
            {
                utils::unordered_vector_erase(gate->m_in_endpoints, ep);
                utils::unordered_vector_erase(gate->m_in_nets, net);
                net->m_destinations[i] = std::move(net->m_destinations.back());
                net->m_destinations.pop_back();
                net->m_destinations_raw[i] = net->m_destinations_raw.back();
                net->m_destinations_raw.pop_back();
                m_event_handler->notify(NetEvent::event::dst_removed, net, gate->get_id());
                return true;
            }
        }

        return false;
    }

    bool NetlistInternalManager::net_remove_destination(Net* net, Endpoint* ep)
    {
        auto gate = ep->get_gate();

        if (!net_remove_destination_internal(net,ep))
        {
            log_warning("net",
                        "input pin '{}' of gate '{}' with ID {} is not a destination of net '{}' with ID {} in netlist with ID {}",
                        ep->get_pin()->get_name(),
                        gate->get_name(),
                        gate->get_id(),
                        net->get_name(),
                        net->get_id(),
                        m_netlist->m_netlist_id);
        }
        else
        {    // update internal nets and port nets
            if (m_net_checks_enabled)
            {
                if (!gate->get_module()->check_net(net, true))
                {
                    return false;
                }

                for (Endpoint* src : net->get_sources())
                {
                    if (!src->get_gate()->get_module()->check_net(net, true))
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    //######################################################################
    //###                          modules                               ###
    //######################################################################

    Module* NetlistInternalManager::create_module(const u32 id, Module* parent, const std::string& name)
    {
        if (id == 0)
        {
            log_error("module", "ID 0 represents an invalid module ID.");
            return nullptr;
        }
        if (m_netlist->m_used_module_ids.find(id) != m_netlist->m_used_module_ids.end())
        {
            log_error("module", "module ID {} is already taken in netlist with ID {}.", id, m_netlist->m_netlist_id);
            return nullptr;
        }
        if (utils::trim(name).empty())
        {
            log_error("module", "module name cannot be empty.");
            return nullptr;
        }
        if (parent == nullptr && m_netlist->m_top_module != nullptr)
        {
            log_error("module", "parent module cannot not be nullptr.");
            return nullptr;
        }
        if (parent != nullptr && m_netlist != parent->get_netlist())
        {
            log_error("module", "parent module must belong to netlist with ID {}.", m_netlist->m_netlist_id);
            return nullptr;
        }

        auto m = std::unique_ptr<Module>(new Module(this, m_event_handler, id, parent, name));

        auto free_id_it = m_netlist->m_free_module_ids.find(id);
        if (free_id_it != m_netlist->m_free_module_ids.end())
        {
            m_netlist->m_free_module_ids.erase(free_id_it);
        }

        m_netlist->m_used_module_ids.insert(id);

        auto raw                     = m.get();
        m_netlist->m_modules_map[id] = std::move(m);
        m_netlist->m_modules_set.insert(raw);
        m_netlist->m_modules.push_back(raw);

        if (parent != nullptr)
        {
            parent->m_submodules_map[id] = raw;
            parent->m_submodules.push_back(raw);
        }

        m_event_handler->notify(ModuleEvent::event::created, raw);

        if (parent != nullptr)
        {
            m_event_handler->notify(ModuleEvent::event::submodule_added, parent, id);
        }

        return raw;
    }

    bool NetlistInternalManager::delete_module(Module* to_remove)
    {
        if (!m_netlist->is_module_in_netlist(to_remove))
        {
            return false;
        }

        if (to_remove == m_netlist->m_top_module)
        {
            return false;
        }

        // at this point parent is guaranteed to be not null

        // remove from grouping
        if (Grouping* g = to_remove->get_grouping(); g != nullptr)
        {
            g->remove_module(to_remove);
        }

        // move gates and nets to parent, work on a copy since assign_gate will modify m_gates
        std::vector<Gate*> gates_copy = to_remove->m_gates;
        to_remove->m_parent->assign_gates(gates_copy);

        // move all submodules to parent
        for (auto sm : to_remove->m_submodules)
        {
            to_remove->m_parent->m_submodules_map[sm->get_id()] = sm;
            to_remove->m_parent->m_submodules.push_back(sm);

            m_event_handler->notify(ModuleEvent::event::submodule_removed, sm->get_parent_module(), sm->get_id());

            sm->m_parent = to_remove->m_parent;

            m_event_handler->notify(ModuleEvent::event::parent_changed, sm, 0);
            m_event_handler->notify(ModuleEvent::event::submodule_added, to_remove->m_parent, sm->get_id());
        }

        // remove module from parent
        to_remove->m_parent->m_submodules_map.erase(to_remove->get_id());
        utils::unordered_vector_erase(to_remove->m_parent->m_submodules, to_remove);
        m_event_handler->notify(ModuleEvent::event::submodule_removed, to_remove->m_parent, to_remove->get_id());

        auto it  = m_netlist->m_modules_map.find(to_remove->get_id());
        auto ptr = std::move(it->second);
        m_netlist->m_modules_map.erase(it);
        m_netlist->m_modules_set.erase(to_remove);
        utils::unordered_vector_erase(m_netlist->m_modules, to_remove);

        m_netlist->m_free_module_ids.insert(to_remove->get_id());
        m_netlist->m_used_module_ids.erase(to_remove->get_id());

        m_event_handler->notify(ModuleEvent::event::removed, to_remove);
        return true;
    }

    bool NetlistInternalManager::module_assign_gate(Module* m, Gate* g)
    {
        return module_assign_gates(m, {g});
    }

    bool NetlistInternalManager::module_assign_gates(Module* module, const std::vector<Gate*>& gates)
    {
        // check all gates for validity
        if (module == nullptr)
        {
            log_error("module", "module cannot be a nullptr.");
            return false;
        }

        std::map<Module*, u32> prev_modules;

        for (const Gate* g : gates)
        {
            if (g == nullptr)
            {
                log_error("module", "gate cannot be a nullptr.");
                return false;
            }

            Module* prev_mod = g->m_module;

            if (prev_mod == module)
            {
                log_error("module",
                          "gate '{}' with ID {} is already contained in module '{}' with ID {} in netlist with ID {}.",
                          g->get_name(),
                          g->get_id(),
                          module->get_name(),
                          module->get_id(),
                          m_netlist->m_netlist_id);
                return false;
            }

            if (auto it = prev_modules.find(prev_mod); it == prev_modules.end())
            {
                prev_modules[prev_mod] = 1;
            }
            else
            {
                std::get<1>(*it)++;
            }
        }

        m_event_handler->notify(ModuleEvent::event::gates_assign_begin, module, gates.size());
        for (const auto& [prev_mod, num_gates] : prev_modules)
        {
            m_event_handler->notify(ModuleEvent::event::gates_remove_begin, prev_mod, num_gates);
        }
        prev_modules.clear();

        // re-assign gates
        std::unordered_map<Module*, std::unordered_set<Net*>> nets_to_check;
        for (Gate* g : gates)
        {
            // remove gate from old module
            Module* prev_mod = g->m_module;
            const auto it    = prev_mod->m_gates_map.find(g->get_id());
            assert(it != prev_mod->m_gates_map.end());
            prev_mod->m_gates_map.erase(it);

            utils::unordered_vector_erase(prev_mod->m_gates, g);

            // move gate to new module
            module->m_gates_map[g->get_id()] = g;
            module->m_gates.push_back(g);
            g->m_module = module;

            // collect affected nets
            std::vector<Net*> fan_in = g->get_fan_in_nets();
            nets_to_check[prev_mod].insert(fan_in.begin(), fan_in.end());
            nets_to_check[module].insert(fan_in.begin(), fan_in.end());

            std::vector<Net*> fan_out = g->get_fan_out_nets();
            nets_to_check[prev_mod].insert(fan_out.begin(), fan_out.end());
            nets_to_check[module].insert(fan_out.begin(), fan_out.end());
            m_event_handler->notify(ModuleEvent::event::gate_removed, prev_mod, g->get_id());
            m_event_handler->notify(ModuleEvent::event::gate_assigned, module, g->get_id());

            if (auto prev_mod_it = prev_modules.find(prev_mod); prev_mod_it == prev_modules.end())
            {
                prev_modules[prev_mod] = 1;
            }
            else
            {
                std::get<1>(*prev_mod_it)++;
            }
        }

        if (m_net_checks_enabled)
        {
            for (const auto& [affected_module, nets] : nets_to_check)
            {
                for (Net* net : nets)
                {
                    if (!affected_module->check_net(net, true))
                    {
                        return false;
                    }
                }
            }
        }

        m_event_handler->notify(ModuleEvent::event::gates_assign_end, module, gates.size());
        for (const auto& [prev_mod, num_gates] : prev_modules)
        {
            m_event_handler->notify(ModuleEvent::event::gates_remove_end, prev_mod, num_gates);
        }

        return true;
    }

    bool NetlistInternalManager::module_check_net(Module* module, Net* net, bool recursive)
    {
        return module->check_net(net, recursive);
    }

    //######################################################################
    //###                      groupings                                 ###
    //######################################################################

    Grouping* NetlistInternalManager::create_grouping(u32 id, const std::string name)
    {
        if (id == 0)
        {
            log_error("grouping", "ID 0 represents an invalid grouping ID.");
            return nullptr;
        }
        if (m_netlist->m_used_grouping_ids.find(id) != m_netlist->m_used_grouping_ids.end())
        {
            log_error("grouping", "grouping ID {} is already taken in netlist with ID {}.", id, m_netlist->m_netlist_id);
            return nullptr;
        }
        if (utils::trim(name).empty())
        {
            log_error("grouping", "grouping name cannot be empty.");
            return nullptr;
        }

        auto new_grouping = std::unique_ptr<Grouping>(new Grouping(this, m_event_handler, id, name));

        auto free_id_it = m_netlist->m_free_grouping_ids.find(id);
        if (free_id_it != m_netlist->m_free_grouping_ids.end())
        {
            m_netlist->m_free_grouping_ids.erase(free_id_it);
        }

        m_netlist->m_used_grouping_ids.insert(id);

        auto raw = new_grouping.get();

        m_netlist->m_groupings_map[id] = std::move(new_grouping);
        m_netlist->m_groupings_set.insert(raw);
        m_netlist->m_groupings.push_back(raw);

        // notify
        m_event_handler->notify(GroupingEvent::event::created, raw);

        return raw;
    }

    bool NetlistInternalManager::delete_grouping(Grouping* grouping)
    {
        if (!m_netlist->is_grouping_in_netlist(grouping))
        {
            return false;
        }

        for (Gate* gate : grouping->get_gates())
        {
            gate->m_grouping = nullptr;
        }

        for (Net* net : grouping->get_nets())
        {
            net->m_grouping = nullptr;
        }

        for (Module* module : grouping->get_modules())
        {
            module->m_grouping = nullptr;
        }

        auto it  = m_netlist->m_groupings_map.find(grouping->get_id());
        auto ptr = std::move(it->second);
        m_netlist->m_groupings_map.erase(it);
        m_netlist->m_groupings_set.erase(grouping);
        utils::unordered_vector_erase(m_netlist->m_groupings, grouping);

        // free ids
        m_netlist->m_free_grouping_ids.insert(grouping->get_id());
        m_netlist->m_used_grouping_ids.erase(grouping->get_id());

        // notify
        m_event_handler->notify(GroupingEvent::event::removed, grouping);

        return true;
    }

    bool NetlistInternalManager::grouping_assign_gate(Grouping* grouping, Gate* gate, bool force)
    {
        if (gate == nullptr || gate->get_grouping() == grouping)
        {
            return false;
        }

        u32 gate_id = gate->get_id();

        if (Grouping* other = gate->get_grouping(); other != nullptr)
        {
            if (force)
            {
                other->remove_gate(gate);
            }
            else
            {
                log_error("grouping",
                          "gate '{}' with ID {} is already part of another grouping called '{}' with ID {} in netlist with ID {}.",
                          gate->get_name(),
                          gate_id,
                          other->get_name(),
                          other->get_id(),
                          m_netlist->m_netlist_id);
                return false;
            }
        }

        grouping->m_gates.push_back(gate);
        grouping->m_gates_map.emplace(gate_id, gate);
        gate->m_grouping = grouping;

        m_event_handler->notify(GroupingEvent::event::gate_assigned, grouping, gate_id);

        return true;
    }

    bool NetlistInternalManager::grouping_remove_gate(Grouping* grouping, Gate* gate)
    {
        if (gate == nullptr)
        {
            return false;
        }

        u32 gate_id = gate->get_id();

        if (!grouping->contains_gate(gate))
        {
            log_error("grouping",
                      "gate '{}' with ID {} is not part of grouping '{}' with ID {} in netlist with ID {}.",
                      gate->get_name(),
                      gate_id,
                      grouping->get_name(),
                      grouping->get_id(),
                      m_netlist->m_netlist_id);
            return false;
        }

        auto vec_it = std::find(grouping->m_gates.begin(), grouping->m_gates.end(), gate);

        *vec_it = grouping->m_gates.back();
        grouping->m_gates.pop_back();
        grouping->m_gates_map.erase(gate_id);
        gate->m_grouping = nullptr;

        m_event_handler->notify(GroupingEvent::event::gate_removed, grouping, gate_id);

        return true;
    }

    bool NetlistInternalManager::grouping_assign_net(Grouping* grouping, Net* net, bool force)
    {
        if (net == nullptr || net->get_grouping() == grouping)
        {
            return false;
        }

        u32 net_id = net->get_id();

        if (Grouping* other = net->get_grouping(); other != nullptr)
        {
            if (force)
            {
                other->remove_net(net);
            }
            else
            {
                log_error("grouping",
                          "net '{}' with ID {} is already part of another grouping called '{}' with ID {} in netlist with ID {}.",
                          net->get_name(),
                          net_id,
                          other->get_name(),
                          other->get_id(),
                          m_netlist->m_netlist_id);
                return false;
            }
        }

        grouping->m_nets.push_back(net);
        grouping->m_nets_map.emplace(net_id, net);
        net->m_grouping = grouping;

        m_event_handler->notify(GroupingEvent::event::net_assigned, grouping, net_id);

        return true;
    }

    bool NetlistInternalManager::grouping_remove_net(Grouping* grouping, Net* net)
    {
        if (net == nullptr)
        {
            return false;
        }

        u32 net_id = net->get_id();

        if (!grouping->contains_net(net))
        {
            log_error("grouping",
                      "net '{}' with ID {} is not part of grouping '{}' with ID {} in netlist with ID {}.",
                      net->get_name(),
                      net_id,
                      grouping->get_name(),
                      grouping->get_id(),
                      m_netlist->m_netlist_id);
            return false;
        }

        auto vec_it = std::find(grouping->m_nets.begin(), grouping->m_nets.end(), net);

        *vec_it = grouping->m_nets.back();
        grouping->m_nets.pop_back();
        grouping->m_nets_map.erase(net_id);
        net->m_grouping = nullptr;

        m_event_handler->notify(GroupingEvent::event::net_removed, grouping, net_id);

        return true;
    }

    bool NetlistInternalManager::grouping_assign_module(Grouping* grouping, Module* module, bool force)
    {
        if (module == nullptr || module->get_grouping() == grouping)
        {
            return false;
        }

        u32 module_id = module->get_id();

        if (Grouping* other = module->get_grouping(); other != nullptr)
        {
            if (force)
            {
                other->remove_module(module);
            }
            else
            {
                log_error("grouping",
                          "module '{}' with ID {} is already part of another grouping called '{}' with ID {} in netlist with ID {}.",
                          module->get_name(),
                          module_id,
                          other->get_name(),
                          other->get_id(),
                          m_netlist->m_netlist_id);
                return false;
            }
        }

        grouping->m_modules.push_back(module);
        grouping->m_modules_map.emplace(module_id, module);
        module->m_grouping = grouping;

        m_event_handler->notify(GroupingEvent::event::module_assigned, grouping, module_id);

        return true;
    }

    bool NetlistInternalManager::grouping_remove_module(Grouping* grouping, Module* module)
    {
        if (module == nullptr)
        {
            return false;
        }

        u32 module_id = module->get_id();

        if (!grouping->contains_module(module))
        {
            log_error("grouping",
                      "module '{}' with ID {} is not part of grouping '{}' with ID {} in netlist with ID {}.",
                      module->get_name(),
                      module_id,
                      grouping->get_name(),
                      grouping->get_id(),
                      m_netlist->m_netlist_id);
            return false;
        }

        auto vec_it = std::find(grouping->m_modules.begin(), grouping->m_modules.end(), module);

        *vec_it = grouping->m_modules.back();
        grouping->m_modules.pop_back();
        grouping->m_modules_map.erase(module_id);
        module->m_grouping = nullptr;

        m_event_handler->notify(GroupingEvent::event::module_removed, grouping, module_id);

        return true;
    }

    //######################################################################
    //###                           caches                               ###
    //######################################################################

    void NetlistInternalManager::clear_caches()
    {
        m_lut_function_cache.clear();
    }
}    // namespace hal
