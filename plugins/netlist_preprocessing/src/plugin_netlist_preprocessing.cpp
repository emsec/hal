#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistPreprocessingPlugin>();
    }

    std::string NetlistPreprocessingPlugin::get_name() const
    {
        return std::string("netlist_preprocessing");
    }

    std::string NetlistPreprocessingPlugin::get_version() const
    {
        return std::string("0.1");
    }

    Result<u32> NetlistPreprocessingPlugin::remove_unused_lut_inputs(Netlist* nl)
    {
        u32 num_eps = 0;

        // get net connected to GND
        const std::vector<Gate*>& gnd_gates = nl->get_gnd_gates();
        if (gnd_gates.empty())
        {
            return ERR("could not remove unused LUT endpoints from netlist with ID " + std::to_string(nl->get_id()) + ": no GND net available within netlist");
        }
        Net* gnd_net = gnd_gates.front()->get_fan_out_nets().front();

        // iterate all LUT gates
        for (const auto& gate : nl->get_gates([](Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
        {
            std::vector<Endpoint*> fan_in                              = gate->get_fan_in_endpoints();
            std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();

            // skip if more than one function
            if (functions.size() != 1)
            {
                continue;
            }

            // only pins used as variables in Boolean function are considered active
            auto active_pins = functions.begin()->second.get_variable_names();

            // if there are more fan-in nets than there are active pins, remove those that are not used within the Boolean function and reconnect to GND
            if (fan_in.size() > active_pins.size())
            {
                for (const auto& ep : fan_in)
                {
                    if (std::find(active_pins.begin(), active_pins.end(), ep->get_pin()->get_name()) == active_pins.end())
                    {
                        GatePin* pin = ep->get_pin();
                        if (!ep->get_net()->remove_destination(gate, pin))
                        {
                            log_warning(
                                "netlist_preprocessing", "failed to remove unused input from LUT gate '{}' with ID {} from netlist with ID {}.", gate->get_name(), gate->get_id(), nl->get_id());
                        }
                        else if (!gnd_net->add_destination(gate, pin))
                        {
                            log_warning(
                                "netlist_preprocessing", "failed to reconnect unused input of LUT gate '{}' with ID {} to GND in netlist with ID {}.", gate->get_name(), gate->get_id(), nl->get_id());
                        }
                        else
                        {
                            num_eps++;
                        }
                    }
                }
            }
        }

        return OK(num_eps);
    }

    Result<u32> NetlistPreprocessingPlugin::remove_buffers(Netlist* nl)
    {
        return ERR("not implemented");
    }

    Result<u32> NetlistPreprocessingPlugin::remove_redundant_logic(Netlist* nl)
    {
        const auto& nets   = nl->get_nets();
        auto nets_to_check = std::set<Net*>(nets.begin(), nets.end());

        u32 delete_ctr = 0;
        while (!nets_to_check.empty())
        {
            auto* current_net = *nets_to_check.begin();
            nets_to_check.erase(current_net);

            // only continue for nets with multiple destinations
            if (current_net->get_num_of_destinations() <= 1)
            {
                nets_to_check.erase(current_net);
                continue;
            }

            std::set<Gate*> visited_gates;
            std::vector<std::vector<Gate*>> gates_to_delete;
            auto destinations = current_net->get_destinations();
            for (u32 i = 0; i < destinations.size(); i++)
            {
                auto* master_destination = destinations.at(i);
                auto* master_gate        = master_destination->get_gate();

                // check if we have already identified the current master gate as duplicate of some other gate
                if (visited_gates.find(master_gate) != visited_gates.end())
                {
                    continue;
                }

                // skip everything that is not combinational, a FF, or a latch
                auto* master_type = master_gate->get_type();

                // cache master input nets and endpoints
                std::variant<std::vector<Net*>, std::map<GatePin*, Net*>> master_inputs;
                if (master_type->has_property(GateTypeProperty::combinational))
                {
                    // for combinational gates, the order of inputs will be considered by the final SMT check only (accounts for commutative Boolean functions)
                    auto tmp = master_gate->get_fan_in_nets();
                    std::sort(tmp.begin(), tmp.end());
                    master_inputs = std::move(tmp);
                }
                else if (master_type->has_property(GateTypeProperty::ff) || master_type->has_property(GateTypeProperty::latch))
                {
                    // for FF and latch gates, pins and nets must match exactly
                    std::map<GatePin*, Net*> tmp;
                    for (auto& ep : master_gate->get_fan_in_endpoints())
                    {
                        tmp[ep->get_pin()] = ep->get_net();
                    }
                    master_inputs = std::move(tmp);
                }
                else
                {
                    continue;
                }

                // identify duplicate gates
                std::vector<Gate*> duplicate_gates = {master_gate};
                for (u32 j = i + 1; j < destinations.size(); j++)
                {
                    auto* current_destination = destinations.at(j);
                    auto* current_gate        = current_destination->get_gate();

                    if (current_gate == master_gate)
                    {
                        continue;
                    }

                    // check if we have already identified the current gate as duplicate of some other gate
                    if (visited_gates.find(current_gate) != visited_gates.end())
                    {
                        continue;
                    }

                    // check against master gate type
                    auto* current_type = current_gate->get_type();
                    if (current_type != master_type)
                    {
                        continue;
                    }

                    // check current inputs against master input nets
                    std::variant<std::vector<Net*>, std::map<GatePin*, Net*>> current_inputs;
                    if (master_type->has_property(GateTypeProperty::combinational))
                    {
                        // for combinational gates, the order of inputs will be considered by the final SMT check only (accounts for commutative Boolean functions)
                        auto tmp = current_gate->get_fan_in_nets();
                        std::sort(tmp.begin(), tmp.end());
                        current_inputs = std::move(tmp);
                    }
                    else if (master_type->has_property(GateTypeProperty::ff) || master_type->has_property(GateTypeProperty::latch))
                    {
                        // for FF and latch gates, pins and nets must match exactly
                        std::map<GatePin*, Net*> tmp;
                        for (auto& ep : current_gate->get_fan_in_endpoints())
                        {
                            tmp[ep->get_pin()] = ep->get_net();
                        }
                        current_inputs = std::move(tmp);
                    }

                    if (current_inputs != master_inputs)
                    {
                        continue;
                    }

                    if (master_type->has_property(GateTypeProperty::combinational))
                    {
                        // SMT Boolean function equivalence check for combinational gates
                        bool skip_gate = false;
                        for (const auto* pin : master_type->get_output_pins())
                        {
                            const auto solver_res =
                                master_gate->get_resolved_boolean_function(pin)
                                    .map<BooleanFunction>([pin, current_gate](auto&& bf_master) {
                                        return current_gate->get_resolved_boolean_function(pin).map<BooleanFunction>(
                                            [bf_master = std::move(bf_master)](auto&& bf_current) mutable { return BooleanFunction::Eq(std::move(bf_master), bf_current.clone(), 1); });
                                    })
                                    .map<BooleanFunction>([](auto&& bf_eq) { return BooleanFunction::Not(bf_eq.clone(), 1); })
                                    .map<SMT::SolverResult>([](auto&& bf_not) -> Result<SMT::SolverResult> { return SMT::Solver({SMT::Constraint(std::move(bf_not))}).query(SMT::QueryConfig()); });

                            if (solver_res.is_error() || !solver_res.get().is_unsat())
                            {
                                skip_gate = true;
                                break;
                            }
                        }

                        if (skip_gate)
                        {
                            continue;
                        }
                    }
                    else if (master_type->has_property(GateTypeProperty::ff) || master_type->has_property(GateTypeProperty::latch))
                    {
                        auto current_inputs = current_gate->get_fan_in_endpoints();
                    }

                    // gate is determined to be duplicate of other gate
                    duplicate_gates.push_back(current_gate);
                    visited_gates.insert(current_gate);
                }

                // remove duplicate gates
                if (duplicate_gates.size() > 1)
                {
                    gates_to_delete.push_back(duplicate_gates);
                }
            }

            for (auto& duplicate_gates : gates_to_delete)
            {
                auto* surviver_gate = duplicate_gates.front();
                std::map<GatePin*, Net*> out_pins_to_nets;
                for (auto* ep : surviver_gate->get_fan_out_endpoints())
                {
                    out_pins_to_nets[ep->get_pin()] = ep->get_net();
                    nets_to_check.insert(ep->get_net());
                }

                for (u32 k = 1; k < duplicate_gates.size(); k++)
                {
                    auto* current_gate = duplicate_gates.at(k);
                    for (auto* ep : current_gate->get_fan_out_endpoints())
                    {
                        auto* ep_net = ep->get_net();
                        auto* ep_pin = ep->get_pin();

                        if (auto it = out_pins_to_nets.find(ep_pin); it != out_pins_to_nets.end())
                        {
                            // surviver already has net connected to this output -> add destination to surviver's net
                            for (auto* dst : ep_net->get_destinations())
                            {
                                auto* dst_gate = dst->get_gate();
                                auto* dst_pin  = dst->get_pin();
                                dst->get_net()->remove_destination(dst);
                                it->second->add_destination(dst_gate, dst_pin);
                            }
                            if (!nl->delete_net(ep_net))
                            {
                                log_warning("netlist_preprocessing", "could not delete net '{}' with ID {} from netlist with ID {}.", ep_net->get_name(), ep_net->get_id(), nl->get_id());
                            }
                            nets_to_check.erase(ep_net);
                        }
                        else
                        {
                            // surviver does not feature net on this output pin -> connect this net to surviver
                            ep_net->add_source(surviver_gate, ep_pin);
                            out_pins_to_nets[ep_pin] = ep_net;
                            nets_to_check.insert(ep_net);
                        }
                    }

                    if (!nl->delete_gate(current_gate))
                    {
                        log_warning("netlist_preprocessing", "could not delete gate '{}' with ID {} from netlist with ID {}.", current_gate->get_name(), current_gate->get_id(), nl->get_id());
                    }
                    else
                    {
                        delete_ctr++;
                    }
                }
            }
        }

        return OK(delete_ctr);
    }
}    // namespace hal
