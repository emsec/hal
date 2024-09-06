#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"

#include "hal_core/netlist/decorators/boolean_function_decorator.h"
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

    Result<std::unique_ptr<Netlist>> SubgraphNetlistDecorator::copy_subgraph_netlist(const std::vector<const Gate*>& subgraph_gates, const bool all_global_io) const
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

        Module* c_top_module = c_netlist->get_top_module();

        for (Net* c_net : c_netlist->get_nets())
        {
            Net* net = m_netlist.get_net_by_id(c_net->get_id());

            if (all_global_io)
            {
                // mark all nets as global input that either lost a source or were a global input originally
                if ((c_net->get_num_of_sources() < net->get_num_of_sources()) || net->is_global_input_net())
                {
                    c_netlist->mark_global_input_net(c_net);
                }

                // mark all nets as global output that either lost a destination or were a global output originally
                if ((c_net->get_num_of_destinations() < net->get_num_of_destinations()) || net->is_global_output_net())
                {
                    c_netlist->mark_global_output_net(c_net);
                }
            }
            else
            {
                // mark all nets as global input that now have zero sources but had a source in the orginal netlist or were a global input originally
                if ((c_net->get_num_of_sources() == 0 && net->get_num_of_sources() != 0) || net->is_global_input_net())
                {
                    c_netlist->mark_global_input_net(c_net);
                }

                // mark all nets as global output that now have zero destinations but had at least one destination in the orginal netlist or were a global output originally
                if ((net->get_num_of_destinations() != 0 && c_net->get_num_of_destinations() == 0) || net->is_global_output_net())
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
            if (auto res = c_top_module->create_pin(c_input_net->get_name(), c_input_net); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not copy subgraph netlist: unable to create pin '" + c_input_net->get_name() + "'");
            }
        }
        for (Net* c_output_net : c_top_module->get_output_nets())
        {
            if (auto res = c_top_module->create_pin(c_output_net->get_name(), c_output_net); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not copy subgraph netlist: unable to create pin '" + c_output_net->get_name() + "'");
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

    Result<std::unique_ptr<Netlist>> SubgraphNetlistDecorator::copy_subgraph_netlist(const std::vector<Gate*>& subgraph_gates, const bool all_global_io) const
    {
        const auto subgraph_gates_const = std::vector<const Gate*>(subgraph_gates.begin(), subgraph_gates.end());
        if (auto res = copy_subgraph_netlist(subgraph_gates_const, all_global_io); res.is_ok())
        {
            return res;
        }
        else
        {
            return ERR(res.get_error());
        }
    }

    Result<std::unique_ptr<Netlist>> SubgraphNetlistDecorator::copy_subgraph_netlist(const Module* subgraph_module, const bool all_global_io) const
    {
        if (auto res = copy_subgraph_netlist(subgraph_module->get_gates(), all_global_io); res.is_ok())
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
        Result<BooleanFunction> subgraph_function_recursive(const Net* n,
                                                            const std::vector<const Gate*>& subgraph_gates,
                                                            std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache,
                                                            std::unordered_set<const Net*>& on_stack)
        {
            if (on_stack.find(n) != on_stack.end())
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": subgraph contains a cycle!");
            }

            const std::vector<Endpoint*> sources = n->get_sources();

            // net is multi driven
            if (sources.size() > 1)
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": cannot handle multi driven nets! Encountered at net "
                           + std::to_string(n->get_id()) + ".");
            }

            // net has no source
            if (sources.empty())
            {
                const auto net_dec = BooleanFunctionNetDecorator(*n);
                return OK(net_dec.get_boolean_variable());
            }

            const Endpoint* src_ep = sources.front();

            if (src_ep->get_gate() == nullptr)
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": gate at source for net " + std::to_string(n->get_id())
                           + " is null.");
            }

            const Gate* src_gate = src_ep->get_gate();

            // source is not in subgraph gates
            if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src_gate) == subgraph_gates.end())
            {
                const auto net_dec = BooleanFunctionNetDecorator(*n);
                return OK(net_dec.get_boolean_variable());
            }

            if (const auto it = gate_cache.find({src_gate->get_id(), src_ep->get_pin()}); it != gate_cache.end())
            {
                return OK(it->second);
            }

            const auto bf_res = src_gate->get_resolved_boolean_function(src_ep->get_pin());
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(), "could not get subgraph function of net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to get function of gate.");
            }
            BooleanFunction gate_func = bf_res.get().simplify_local();

            on_stack.insert(n);

            std::map<std::string, BooleanFunction> input_to_bf;

            for (const std::string& in_net_str : gate_func.get_variable_names())
            {
                u32 in_net_id = std::stoi(in_net_str.substr(4));
                Net* in_net   = src_gate->get_netlist()->get_net_by_id(in_net_id);

                if (in_net == nullptr)
                {
                    return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": cannot find in_net " + in_net_str + " at gate "
                               + std::to_string(src_gate->get_id()) + "!");
                }

                auto input_bf_res = subgraph_function_recursive(in_net, subgraph_gates, gate_cache, on_stack);

                if (input_bf_res.is_error())
                {
                    // NOTE since this can lead to a deep recursion we dont append the error and instead just forward this.
                    return input_bf_res;
                }

                input_to_bf.insert({in_net_str, input_bf_res.get()});
            }

            auto res = gate_func.substitute(input_to_bf);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": failed to substitute inputs for gate function "
                                      + gate_func.to_string() + ".");
            }

            on_stack.erase(n);

            auto bf = res.get();

            gate_cache.insert({{src_gate->get_id(), src_ep->get_pin()}, bf});

            return OK(bf);
        }

        Result<std::set<const Net*>> subgraph_function_inputs_recursive(const Net* n,
                                                                        const std::vector<const Gate*>& subgraph_gates,
                                                                        std::map<std::pair<u32, const GatePin*>, std::set<const Net*>>& gate_cache,
                                                                        std::unordered_set<const Net*>& on_stack)
        {
            if (on_stack.find(n) != on_stack.end())
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": subgraph contains a cycle!");
            }

            const std::vector<Endpoint*> sources = n->get_sources();

            // net is multi driven
            if (sources.size() > 1)
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": cannot handle multi driven nets! Encountered at net "
                           + std::to_string(n->get_id()) + ".");
            }

            // net has no source
            if (sources.empty())
            {
                return OK({n});
            }

            const Endpoint* src_ep = sources.front();

            if (src_ep->get_gate() == nullptr)
            {
                return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": gate at source for net " + std::to_string(n->get_id())
                           + " is null.");
            }

            const Gate* src_gate = src_ep->get_gate();

            // source is not in subgraph gates
            if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src_gate) == subgraph_gates.end())
            {
                return OK({n});
            }

            if (const auto it = gate_cache.find({src_gate->get_id(), src_ep->get_pin()}); it != gate_cache.end())
            {
                return OK(it->second);
            }

            const auto bf_res = src_gate->get_resolved_boolean_function(src_ep->get_pin());
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(), "could not get subgraph function of net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + ": failed to get function of gate.");
            }
            BooleanFunction gate_func = bf_res.get().simplify_local();

            on_stack.insert(n);

            std::set<const Net*> inputs;

            for (const std::string& in_net_str : gate_func.get_variable_names())
            {
                auto in_net       = BooleanFunctionNetDecorator::get_net_from(n->get_netlist(), in_net_str).get();
                auto new_nets_res = subgraph_function_inputs_recursive(in_net, subgraph_gates, gate_cache, on_stack);
                if (new_nets_res.is_error())
                {
                    return new_nets_res;
                }

                for (const auto& new_net : new_nets_res.get())
                {
                    inputs.insert(new_net);
                }
            }

            on_stack.erase(n);

            gate_cache.insert({{src_gate->get_id(), src_ep->get_pin()}, inputs});

            return OK(inputs);
        }

    }    // namespace

    Result<BooleanFunction> SubgraphNetlistDecorator::get_subgraph_function(const std::vector<const Gate*>& subgraph_gates,
                                                                            const Net* subgraph_output,
                                                                            std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache) const
    {
        // check validity of subgraph_gates
        if (subgraph_gates.empty())
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": subgraph contains no gates");
        }
        else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id())
                       + ": subgraph contains a gate that is a 'nullptr'");
        }
        else if (subgraph_output == nullptr)
        {
            return ERR("could not get subgraph function: net is a 'nullptr'");
        }
        else if (subgraph_output->get_num_of_sources() > 1)
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": net has more than one source");
        }
        else if (subgraph_output->is_global_input_net())
        {
            const auto net_dec = BooleanFunctionNetDecorator(*subgraph_output);
            return OK(net_dec.get_boolean_variable());
        }
        else if (subgraph_output->get_num_of_sources() == 0)
        {
            const auto net_dec = BooleanFunctionNetDecorator(*subgraph_output);
            return OK(net_dec.get_boolean_variable());
            //return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": net has no sources");
        }

        const Gate* start_gate = subgraph_output->get_sources()[0]->get_gate();
        if (std::find(subgraph_gates.begin(), subgraph_gates.end(), start_gate) == subgraph_gates.end())
        {
            const auto net_dec = BooleanFunctionNetDecorator(*subgraph_output);
            return OK(net_dec.get_boolean_variable());
        }

        std::unordered_set<const Net*> on_stack;

        return subgraph_function_recursive(subgraph_output, subgraph_gates, gate_cache, on_stack);
    }

    Result<BooleanFunction> SubgraphNetlistDecorator::get_subgraph_function(const std::vector<Gate*>& subgraph_gates,
                                                                            const Net* subgraph_output,
                                                                            std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache) const
    {
        const auto subgraph_gates_const = std::vector<const Gate*>(subgraph_gates.begin(), subgraph_gates.end());
        if (auto res = get_subgraph_function(subgraph_gates_const, subgraph_output, gate_cache); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            return res;
        }
    }

    Result<BooleanFunction>
        SubgraphNetlistDecorator::get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache) const
    {
        if (auto res = get_subgraph_function(subgraph_module->get_gates(), subgraph_output, gate_cache); res.is_error())
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
        std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;
        if (auto res = get_subgraph_function(subgraph_gates, subgraph_output, gate_cache); res.is_error())
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

    Result<std::set<const Net*>> SubgraphNetlistDecorator::get_subgraph_function_inputs(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) const
    {
        // check validity of subgraph_gates
        if (subgraph_gates.empty())
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": subgraph contains no gates");
        }
        else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id())
                       + ": subgraph contains a gate that is a 'nullptr'");
        }
        else if (subgraph_output == nullptr)
        {
            return ERR("could not get subgraph function: net is a 'nullptr'");
        }
        else if (subgraph_output->get_num_of_sources() > 1)
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": net has more than one source");
        }
        else if (subgraph_output->is_global_input_net())
        {
            return OK({subgraph_output});
        }
        else if (subgraph_output->get_num_of_sources() == 0)
        {
            return ERR("could not get subgraph function of net '" + subgraph_output->get_name() + "' with ID " + std::to_string(subgraph_output->get_id()) + ": net has no sources");
        }

        const Gate* start_gate = subgraph_output->get_sources()[0]->get_gate();
        if (std::find(subgraph_gates.begin(), subgraph_gates.end(), start_gate) == subgraph_gates.end())
        {
            return OK({subgraph_output});
        }

        std::unordered_set<const Net*> on_stack;
        std::map<std::pair<u32, const GatePin*>, std::set<const Net*>> gate_cache;

        return subgraph_function_inputs_recursive(subgraph_output, subgraph_gates, gate_cache, on_stack);
    }

    Result<std::set<const Net*>> SubgraphNetlistDecorator::get_subgraph_function_inputs(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) const
    {
        const auto subgraph_gates_const = std::vector<const Gate*>(subgraph_gates.begin(), subgraph_gates.end());
        return get_subgraph_function_inputs(subgraph_gates_const, subgraph_output);
    }

    Result<std::set<const Net*>> SubgraphNetlistDecorator::get_subgraph_function_inputs(const Module* subgraph_module, const Net* subgraph_output) const
    {
        return get_subgraph_function_inputs(subgraph_module->get_gates(), subgraph_output);
    }

}    // namespace hal