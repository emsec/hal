#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/utils.h"

namespace hal
{
    SubgraphNetlistDecorator::SubgraphNetlistDecorator(const Netlist& netlist) : m_netlist(netlist)
    {
    }

    Result<std::unique_ptr<Netlist>> SubgraphNetlistDecorator::copy_subgraph_netlist(const std::vector<const Gate*>& subgraph_gates) const
    {
        std::unique_ptr<Netlist> c_netlist = netlist_factory::create_netlist(m_netlist.get_gate_library());
        c_netlist->enable_automatic_net_checks(false);

        // manager, netlist_id, and top_module are set in the constructor

        // get all relevant nets
        std::set<Net*> all_nets_to_copy;
        for (const Gate* gate : m_netlist.get_gates())
        {
            for (const auto& input_net_of_gate : gate->get_fan_in_nets())
            {
                all_nets_to_copy.insert(input_net_of_gate);
            }
            for (const auto& output_net_gate : gate->get_fan_out_nets())
            {
                all_nets_to_copy.insert(output_net_gate);
            }
        }

        // copy nets
        for (const Net* net : all_nets_to_copy)
        {
            Net* new_net = c_netlist->create_net(net->get_id(), net->get_name());
            new_net->set_data_map(net->get_data_map());
        }

        // copy gates
        for (const Gate* gate : subgraph_gates)
        {
            Gate* new_gate = c_netlist->create_gate(gate->get_id(), gate->get_type(), gate->get_name(), gate->get_location_x(), gate->get_location_y());

            for (const auto& [name, func] : gate->get_boolean_functions(true))
            {
                new_gate->add_boolean_function(name, func);
            }

            for (const Endpoint* in_point : gate->get_fan_in_endpoints())
            {
                const auto net_id = in_point->get_net()->get_id();
                auto c_net        = c_netlist->get_net_by_id(net_id);

                c_net->add_destination(new_gate, in_point->get_pin());
            }

            for (const Endpoint* out_point : gate->get_fan_out_endpoints())
            {
                const auto net_id = out_point->get_net()->get_id();
                auto c_net        = c_netlist->get_net_by_id(net_id);

                c_net->add_source(new_gate, out_point->get_pin());
            }

            new_gate->set_data_map(gate->get_data_map());

            // mark gnd and vcc gates
            if (gate->is_gnd_gate())
            {
                c_netlist->mark_gnd_gate(new_gate);
            }
            if (gate->is_vcc_gate())
            {
                c_netlist->mark_vcc_gate(new_gate);
            }
        }

        Module* top_module   = m_netlist.get_top_module();
        Module* c_top_module = c_netlist->get_top_module();

        for (Net* c_net : c_netlist->get_nets())
        {
            Net* net = m_netlist.get_net_by_id(c_net->get_id());

            // mark new global inputs
            if (c_net->get_num_of_sources() == 0)
            {
                if (net->get_num_of_sources() != 0 || net->is_global_input_net())
                {
                    c_netlist->mark_global_input_net(c_net);
                }
            }

            // mark nets that had a destination previously but now dont as global outputs
            if (c_net->get_num_of_destinations() == 0)
            {
                if (net->get_num_of_destinations() != 0 || net->is_global_output_net())
                {
                    c_netlist->mark_global_output_net(c_net);
                }
            }
        }

        // update input and output nets
        c_top_module->update_nets();

        // create module pins for top module
        for (Net* c_input_net : c_top_module->get_input_nets())
        {
            // either use existing name of pin or generate new one
            if (auto pin = top_module->get_pin_by_net(m_netlist.get_net_by_id(c_input_net->get_id())); pin != nullptr)
            {
                if (auto res = c_top_module->create_pin(pin->get_name(), c_input_net, pin->get_type()); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not copy subgraph netlist: unable to create pin with name '" + pin->get_name() + "'");
                }
            }
            else
            {
                if (auto res = c_top_module->create_pin(c_input_net->get_name(), c_input_net); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not copy subgraph netlist: unable to create pin '" + c_input_net->get_name() + "'");
                }
            }
        }
        for (Net* c_output_net : c_top_module->get_output_nets())
        {
            // either use existing name of pin or generate new one
            if (auto pin = top_module->get_pin_by_net(m_netlist.get_net_by_id(c_output_net->get_id())); pin != nullptr)
            {
                if (auto res = c_top_module->create_pin(pin->get_name(), c_output_net, pin->get_type()); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not copy subgraph netlist: unable to create pin '" + pin->get_name() + "'");
                }
            }
            else
            {
                if (auto res = c_top_module->create_pin(c_output_net->get_name(), c_output_net); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not copy subgraph netlist: unable to create pin '" + c_output_net->get_name() + "'");
                }
            }
        }

        // copy some meta data
        c_netlist->set_design_name(m_netlist.get_design_name());
        c_netlist->set_device_name(m_netlist.get_device_name());
        c_netlist->set_input_filename(m_netlist.get_input_filename());

        // update ids last, after all the creation
        c_netlist->set_next_gate_id(m_netlist.get_next_gate_id());
        c_netlist->set_used_gate_ids(m_netlist.get_used_gate_ids());
        c_netlist->set_free_gate_ids(m_netlist.get_free_gate_ids());

        c_netlist->set_next_net_id(m_netlist.get_next_net_id());
        c_netlist->set_used_net_ids(m_netlist.get_used_net_ids());
        c_netlist->set_free_net_ids(m_netlist.get_free_net_ids());

        c_netlist->set_next_module_id(m_netlist.get_next_module_id());
        c_netlist->set_used_module_ids(m_netlist.get_used_module_ids());
        c_netlist->set_free_module_ids(m_netlist.get_free_module_ids());

        c_netlist->set_next_grouping_id(m_netlist.get_next_grouping_id());
        c_netlist->set_used_grouping_ids(m_netlist.get_used_grouping_ids());
        c_netlist->set_free_grouping_ids(m_netlist.get_free_grouping_ids());

        c_netlist->enable_automatic_net_checks(true);

        return OK(std::move(c_netlist));
    }

    Result<std::unique_ptr<Netlist>> SubgraphNetlistDecorator::copy_subgraph_netlist(const std::vector<Gate*>& subgraph_gates) const
    {
        const auto subgraph_gates_const = std::vector<const Gate*>(subgraph_gates.begin(), subgraph_gates.end());
        if (auto res = copy_subgraph_netlist(subgraph_gates_const); res.is_ok())
        {
            return res;
        }
        else
        {
            return ERR(res.get_error());
        }
    }

    Result<std::unique_ptr<Netlist>> SubgraphNetlistDecorator::copy_subgraph_netlist(const Module* subgraph_module) const
    {
        if (auto res = copy_subgraph_netlist(subgraph_module->get_gates()); res.is_ok())
        {
            return res;
        }
        else
        {
            return ERR(res.get_error());
        }
    }

    namespace
    {
        Result<BooleanFunction> get_function_of_gate(const Gate* gate, const GatePin* output_pin, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
        {
            if (auto it = cache.find({gate->get_id(), output_pin}); it != cache.end())
            {
                return OK(it->second);
            }
            else
            {
                BooleanFunction bf = gate->get_boolean_function(output_pin);

                std::vector<std::string> input_vars = utils::to_vector(bf.get_variable_names());
                while (!input_vars.empty())
                {
                    const std::string var = input_vars.back();
                    input_vars.pop_back();

                    const GatePin* pin = gate->get_type()->get_pin_by_name(var);
                    if (pin == nullptr)
                    {
                        return ERR("could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to get input pin '" + var + "' by name");
                    }

                    const PinDirection pin_dir = pin->get_direction();
                    if (pin_dir == PinDirection::input)
                    {
                        const Net* const input_net = gate->get_fan_in_net(var);
                        if (input_net == nullptr)
                        {
                            // if no net is connected, the input pin name cannot be replaced
                            return ERR("could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to get fan-in net at pin '"
                                       + pin->get_name() + "'");
                        }

                        const auto net_dec = BooleanFunctionNetDecorator(*input_net);
                        bf                 = bf.substitute(var, net_dec.get_boolean_variable_name());
                    }
                    else if ((pin_dir == PinDirection::internal) || (pin_dir == PinDirection::output))
                    {
                        BooleanFunction bf_interal = gate->get_boolean_function(var);
                        if (bf_interal.is_empty())
                        {
                            return ERR("could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id())
                                       + ": failed to get Boolean function at output pin '" + pin->get_name() + "'");
                        }

                        const std::vector<std::string> internal_input_vars = utils::to_vector(bf_interal.get_variable_names());
                        input_vars.insert(input_vars.end(), internal_input_vars.begin(), internal_input_vars.end());

                        if (auto substituted = bf.substitute(var, bf_interal); substituted.is_error())
                        {
                            return ERR_APPEND(substituted.get_error(),
                                              "could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to substitute variable '" + var
                                                  + "' with another Boolean function");
                        }
                        else
                        {
                            bf = substituted.get();
                        }
                    }
                }

                cache.insert({{gate->get_id(), output_pin}, bf});
                return OK(bf);
            }
        }

        Result<std::monostate> subgraph_function_bfs(Net* n,
                                                     BooleanFunction& current,
                                                     std::vector<Net*> stack,
                                                     const std::vector<const Gate*>& subgraph_gates,
                                                     std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
        {
            if (n->get_num_of_sources() > 1)
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": number of sources is greater than 1");
            }
            else if (n->get_num_of_sources() == 0)
            {
                return OK({});
            }

            if (auto it = std::find(stack.begin(), stack.end(), n); it != stack.end())
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": subgraph contains a cyclic dependency: "
                           + utils::join(" -> ", it, stack.end(), [](auto nlog) { return nlog->get_name() + " (ID: " + std::to_string(nlog->get_id()) + ")"; }) + " -> " + n->get_name()
                           + " (ID: " + std::to_string(n->get_id()) + ")");
            }

            stack.push_back(n);

            Gate* src_gate   = n->get_sources()[0]->get_gate();
            GatePin* src_pin = n->get_sources()[0]->get_pin();

            if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src_gate) != subgraph_gates.end())
            {
                if (auto func = get_function_of_gate(src_gate, src_pin, cache); func.is_error())
                {
                    return ERR_APPEND(func.get_error(),
                                      "could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": failed to get Boolean function of gate '"
                                          + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()));
                }
                else
                {
                    const auto net_dec = BooleanFunctionNetDecorator(*n);
                    if (auto substitution = current.substitute(net_dec.get_boolean_variable_name(), func.get()); substitution.is_error())
                    {
                        return ERR_APPEND(substitution.get_error(),
                                          "could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": failed to substitute net with Boolean function");
                    }
                    else
                    {
                        current = substitution.get();
                    }
                }

                for (Net* sn : src_gate->get_fan_in_nets())
                {
                    if (auto res = subgraph_function_bfs(sn, current, stack, subgraph_gates, cache); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()));
                    }
                }
            }

            return OK({});
        }
    }    // namespace

    Result<BooleanFunction> SubgraphNetlistDecorator::get_subgraph_function(const std::vector<const Gate*>& subgraph_gates,
                                                                            const Net* subgraph_output,
                                                                            std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const
    {
        /* check validity of subgraph_gates */
        if (subgraph_output == nullptr)
        {
            return ERR("could not get subgraph function: net is a 'nullptr'");
        }
        else if (subgraph_gates.empty())
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": subgraph contains no gates");
        }
        else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id())
                       + ": subgraph contains a gate that is a 'nullptr'");
        }
        else if (subgraph_output->get_num_of_sources() > 1)
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": net has more than one source");
        }
        else if (subgraph_output->get_num_of_sources() == 0 && !subgraph_output->is_global_input_net())
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": net has no sources");
        }

        const Gate* start_gate = subgraph_output->get_sources()[0]->get_gate();
        if (std::find(subgraph_gates.begin(), subgraph_gates.end(), start_gate) == subgraph_gates.end() || subgraph_output->is_global_input_net())
        {
            const auto net_dec = BooleanFunctionNetDecorator(*subgraph_output);
            return OK(net_dec.get_boolean_variable());
        }

        const GatePin* src_pin = subgraph_output->get_sources()[0]->get_pin();

        BooleanFunction result;
        if (auto func = get_function_of_gate(start_gate, src_pin, cache); func.is_error())
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": failed to get function of start gate '"
                       + start_gate->get_name() + "' with ID " + std::to_string(start_gate->get_id()));
        }
        else
        {
            result = func.get();
        }

        for (Net* n : start_gate->get_fan_in_nets())
        {
            if (auto res = subgraph_function_bfs(n, result, {}, subgraph_gates, cache); res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id())
                                      + ": failed to get function at net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()));
            }
        }

        return OK(result);
    }

    Result<BooleanFunction>
        SubgraphNetlistDecorator::get_subgraph_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const
    {
        const auto subgraph_gates_const = std::vector<const Gate*>(subgraph_gates.begin(), subgraph_gates.end());
        if (auto res = get_subgraph_function(subgraph_gates_const, subgraph_output, cache); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            return res;
        }
    }

    Result<BooleanFunction>
        SubgraphNetlistDecorator::get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const
    {
        if (auto res = get_subgraph_function(subgraph_module->get_gates(), subgraph_output, cache); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            return res;
        }
    }

    Result<BooleanFunction> SubgraphNetlistDecorator::get_subgraph_function(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) const
    {
        std::map<std::pair<u32, const GatePin*>, BooleanFunction> cache;
        if (auto res = get_subgraph_function(subgraph_gates, subgraph_output, cache); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            return res;
        }
    }

    Result<BooleanFunction> SubgraphNetlistDecorator::get_subgraph_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) const
    {
        const auto subgraph_gates_const = std::vector<const Gate*>(subgraph_gates.begin(), subgraph_gates.end());
        if (auto res = get_subgraph_function(subgraph_gates_const, subgraph_output); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            return res;
        }
    }

    Result<BooleanFunction> SubgraphNetlistDecorator::get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output) const
    {
        if (auto res = get_subgraph_function(subgraph_module->get_gates(), subgraph_output); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            return res;
        }
    }
}    // namespace hal