#include "hawkeye/sbox_lookup.h"

#include "graph_algorithm/algorithms/components.h"
#include "graph_algorithm/algorithms/subgraph.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hawkeye/round_candidate.h"

#include <algorithm>

namespace hal
{
    namespace hawkeye
    {
        Result<std::vector<SBoxCandidate>> locate_sboxes(const RoundCandidate* candidate)
        {
            log_info("hawkeye", "start locating S-boxes within round function candidate...");
            auto start = std::chrono::system_clock::now();

            const auto* nl    = candidate->get_netlist();
            const auto* graph = candidate->get_graph();

            // get initial set of components
            auto comp_res = graph_algorithm::get_connected_components(graph, false).map<std::vector<std::vector<Gate*>>>([graph](const auto& comps) -> Result<std::vector<std::vector<Gate*>>> {
                std::vector<std::vector<Gate*>> res;
                for (const auto& c : comps)
                {
                    if (const auto gates_res = graph->get_gates_from_vertices(c); gates_res.is_ok())
                    {
                        res.push_back(gates_res.get());
                    }
                    else
                    {
                        return ERR(gates_res.get_error());
                    }
                }
                return OK(res);
            });
            if (comp_res.is_error())
            {
                return ERR(comp_res.get_error());
            }
            auto components = comp_res.get();

            const auto& state_input_reg  = candidate->get_input_reg();
            const auto& state_output_reg = candidate->get_output_reg();

            std::vector<SBoxCandidate> res;

            for (const auto& component : components)
            {
                // gather FFs of the component that are also part of the state input reg
                std::set<Gate*> component_input_ffs;
                for (auto* comp_g : component)
                {
                    if (state_input_reg.find(comp_g) != state_input_reg.end())
                    {
                        component_input_ffs.insert(comp_g);
                    }
                }

                u32 number_input_ffs = component_input_ffs.size();
                if (number_input_ffs < 3)
                {
                    // too small for S-box
                    continue;
                }
                else if (number_input_ffs <= 8)
                {
                    // assume to have found a single S-box
                    std::set<Gate*> sbox_output_gates;

                    for (auto* cand_gate : component)
                    {
                        // skip FFs
                        if (cand_gate->get_type()->has_property(GateTypeProperty::ff))
                        {
                            continue;
                        }

                        // output gates are all combinational gates that have no other successors but the state output reg
                        const auto suc_gates = cand_gate->get_unique_successors();
                        if (std::none_of(suc_gates.begin(), suc_gates.end(), [&state_output_reg](Gate* g) { return state_output_reg.find(g) == state_output_reg.end(); }))
                        {
                            sbox_output_gates.insert(cand_gate);
                        }
                    }

                    // create S-box candidate if input size equals output size
                    if (sbox_output_gates.size() == number_input_ffs)
                    {
                        SBoxCandidate sbox_candidate;
                        sbox_candidate.m_candidate    = candidate;
                        sbox_candidate.m_component    = component;
                        sbox_candidate.m_input_gates  = std::move(component_input_ffs);
                        sbox_candidate.m_output_gates = std::move(sbox_output_gates);
                        res.push_back(sbox_candidate);
                    }
                    continue;
                }

                // try to split component in smaller sub-components (assuming component is combination of S-box and linear layer)
                std::set<Gate*> current_subset = component_input_ffs;
                std::vector<std::vector<std::set<Gate*>>> input_groupings;

                // abuse that map keys are sorted, hence rbegin() will return max distance in map
                const auto& longest_dist_to_gates = candidate->get_longest_distance_to_gate();
                for (u32 step = 1; step < longest_dist_to_gates.rbegin()->first + 1; step++)
                {
                    if (const auto dist_it = longest_dist_to_gates.find(step); dist_it != longest_dist_to_gates.end())
                    {
                        const auto& new_gates = std::get<1>(*dist_it);
                        current_subset.insert(new_gates.begin(), new_gates.end());
                    }
                    else
                    {
                        break;
                    }

                    // generate subgraph of new sub-component
                    auto subgraph_res = graph_algorithm::get_subgraph(graph, current_subset);
                    if (subgraph_res.is_error())
                    {
                        return ERR(subgraph_res.get_error());
                    }
                    auto subgraph = std::move(subgraph_res.get());

                    auto comp_res = graph_algorithm::get_connected_components(subgraph.get(), false);
                    if (comp_res.is_error())
                    {
                        return ERR(comp_res.get_error());
                    }

                    // determine input groups feeding into distinct sub-circuits
                    std::set<u32> lens;
                    std::vector<std::vector<Gate*>> subcomponents;
                    std::vector<std::set<Gate*>> input_groups;
                    for (const auto& comp : comp_res.get())
                    {
                        auto gates_res = subgraph->get_gates_from_vertices(comp);
                        if (gates_res.is_error())
                        {
                            return ERR(gates_res.get_error());
                        }
                        auto comp_gates = gates_res.get();

                        // only consider sub-components connected to at least one input FF
                        if (std::any_of(comp_gates.begin(), comp_gates.end(), [&component_input_ffs](Gate* g) { return component_input_ffs.find(g) != component_input_ffs.end(); }))
                        {
                            std::set<Gate*> input_group;
                            for (auto* comp_g : comp_gates)
                            {
                                if (state_input_reg.find(comp_g) != state_input_reg.end())
                                {
                                    input_group.insert(comp_g);
                                }
                            }
                            lens.insert(input_group.size());
                            input_groups.push_back(std::move(input_group));
                            subcomponents.push_back(std::move(comp_gates));
                        }
                    }

                    // all input groups should have same size and comprise more than one input
                    if (lens.size() == 1 && input_groups.at(0).size() > 1 && input_groups.size() > 1)
                    {
                        input_groupings.push_back(input_groups);
                    }
                }

                const auto& input_ffs_of_gate = candidate->get_input_ffs_of_gate();

                std::vector<std::pair<std::set<Gate*>, std::set<Gate*>>> sboxes_input_output_gates;
                for (const auto& input_groups : input_groupings)
                {
                    for (const auto& input_group : input_groups)
                    {
                        std::set<Gate*> output_group;
                        for (auto* comp_gate : component)
                        {
                            // disregard output FFs
                            if (state_output_reg.find(comp_gate) != state_output_reg.end())
                            {
                                continue;
                            }

                            // disregard gates that only depend on a single input FF
                            if (input_ffs_of_gate.at(comp_gate).size() <= 1)
                            {
                                continue;
                            }

                            // disregard gates that do not only depend on the input FFs of the sub-component
                            if (!std::includes(input_group.begin(), input_group.end(), input_ffs_of_gate.at(comp_gate).begin(), input_ffs_of_gate.at(comp_gate).end()))
                            {
                                continue;
                            }

                            // disregard gates for which no successor is dependent on an additional (external) input
                            auto sucs = comp_gate->get_unique_successors();
                            if (std::all_of(sucs.begin(), sucs.end(), [&input_ffs_of_gate, &input_group](auto* sg) {
                                    return std::includes(input_group.begin(), input_group.end(), input_ffs_of_gate.at(sg).begin(), input_ffs_of_gate.at(sg).end());
                                }))
                            {
                                continue;
                            }

                            // disregard inverters at the outputs (should also be covered by next step)
                            auto preds = comp_gate->get_unique_predecessors();
                            if (comp_gate->get_type()->has_property(GateTypeProperty::c_inverter))
                            {
                                if (std::includes(output_group.begin(), output_group.end(), preds.begin(), preds.end()))
                                {
                                    continue;
                                }
                            }

                            output_group.insert(comp_gate);
                        }

                        // disregard output gates that only depend on other output gates
                        std::vector<Gate*> to_delete;
                        for (auto* out_gate : output_group)
                        {
                            const auto pred_gates = out_gate->get_unique_predecessors();
                            if (std::all_of(pred_gates.begin(), pred_gates.end(), [output_group](Gate* g) { return output_group.find(g) != output_group.end(); }))
                            {
                                to_delete.push_back(out_gate);
                            }
                        }
                        for (auto* del_gate : to_delete)
                        {
                            output_group.erase(del_gate);
                        }

                        if (input_group.size() <= 8 && output_group.size() <= 20)
                        {
                            if (output_group.size() == input_group.size() + 1)
                            {
                                for (auto* drop_gate : output_group)
                                {
                                    SBoxCandidate sbox_candidate;
                                    sbox_candidate.m_candidate    = candidate;
                                    sbox_candidate.m_component    = component;
                                    sbox_candidate.m_input_gates  = input_group;
                                    sbox_candidate.m_output_gates = output_group;
                                    sbox_candidate.m_output_gates.erase(drop_gate);
                                    res.push_back(sbox_candidate);
                                }
                            }
                            else if (output_group.size() == input_group.size() + 2)
                            {
                                for (auto* drop_gate_1 : output_group)
                                {
                                    for (auto* drop_gate_2 : output_group)
                                    {
                                        SBoxCandidate sbox_candidate;
                                        sbox_candidate.m_candidate    = candidate;
                                        sbox_candidate.m_component    = component;
                                        sbox_candidate.m_input_gates  = input_group;
                                        sbox_candidate.m_output_gates = output_group;
                                        sbox_candidate.m_output_gates.erase(drop_gate_1);
                                        sbox_candidate.m_output_gates.erase(drop_gate_2);
                                        res.push_back(sbox_candidate);
                                    }
                                }
                            }
                            else
                            {
                                SBoxCandidate sbox_candidate;
                                sbox_candidate.m_candidate    = candidate;
                                sbox_candidate.m_component    = component;
                                sbox_candidate.m_input_gates  = std::move(input_group);
                                sbox_candidate.m_output_gates = std::move(output_group);
                                res.push_back(sbox_candidate);
                            }
                        }
                    }
                }
            }

            auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "located {} S-box candidates within round function candidate in {:.2f} seconds", res.size(), duration_in_seconds);

            return OK(res);
        }

        Result<std::string> identify_sbox(const SBoxCandidate& sbox_candidate, const SBoxDatabase& db)
        {
            log_info("hawkeye", "start identifying S-box candidate...");
            auto start = std::chrono::system_clock::now();

            std::string sbox_name;

            const RoundCandidate* candidate     = sbox_candidate.m_candidate;
            const std::vector<Gate*>& component = sbox_candidate.m_component;
            const std::set<Gate*>& input_gates  = sbox_candidate.m_input_gates;
            const std::set<Gate*>& output_gates = sbox_candidate.m_output_gates;

            if (input_gates.size() == 0)
            {
                return ERR("empty set of input gates provided");
            }

            const auto* nl = candidate->get_netlist();
            const auto snd = SubgraphNetlistDecorator(*nl);

            std::vector<BooleanFunction> bfs;
            std::set<Net*> all_inputs;

            const auto& in_reg = candidate->get_input_reg();
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> cache;

            for (const auto* out_gate : output_gates)
            {
                // check whether any of the output gates have multiple outputs (this could be relaxed later)
                const auto& fan_out_nets = out_gate->get_fan_out_nets();
                if (fan_out_nets.size() != 1)
                {
                    log_error("hawkeye", "gate '{}' with ID {} has none or multiple fan-out nets, which is currently not supported", out_gate->get_name(), out_gate->get_id());
                }
                const auto* out_net = fan_out_nets.front();

                // get Boolean functions of all subgraphs described by the gates of the component and each output net
                std::vector<Gate*> subgraph_gates;
                std::copy_if(component.begin(), component.end(), std::back_inserter(subgraph_gates), [&in_reg](Gate* g) { return in_reg.find(g) == in_reg.end(); });
                auto bf_res = snd.get_subgraph_function(subgraph_gates, out_net, cache);
                if (bf_res.is_error())
                {
                    return ERR(bf_res.get_error());
                }
                bfs.push_back(bf_res.get());

                // gather all input nets actually used by the component
                auto variables = bfs.back().get_variable_names();
                std::transform(
                    variables.begin(), variables.end(), std::inserter(all_inputs, all_inputs.end()), [nl](const std::string& var) { return BooleanFunctionNetDecorator::get_net_from(nl, var).get(); });
            }

            // check whether any of the input gates have multiple outputs (not allowed for FFs)
            for (const auto* in_gate : input_gates)
            {
                const auto& fan_out_nets = in_gate->get_fan_out_nets();
                if (fan_out_nets.size() != 1)
                {
                    log_error("hawkeye", "gate '{}' with ID {} has none or multiple fan-out nets, which is currently not supported", in_gate->get_name(), in_gate->get_id());
                }
            }

            // gather state inputs actually used by the analyzed component
            const auto& state_inputs = candidate->get_state_inputs();
            std::set<const Net*> actual_state_inputs;
            std::set_intersection(all_inputs.begin(), all_inputs.end(), state_inputs.begin(), state_inputs.end(), std::inserter(actual_state_inputs, actual_state_inputs.begin()));

            // gather control inputs actually used by the analyzed component
            const auto& control_inputs = candidate->get_control_inputs();
            std::set<Net*> actual_control_inputs;
            std::set_intersection(all_inputs.begin(), all_inputs.end(), control_inputs.begin(), control_inputs.end(), std::inserter(actual_control_inputs, actual_control_inputs.begin()));

            // gather other inputs actually used by the analyzed component
            const auto& other_inputs = candidate->get_other_inputs();
            std::set<Net*> actual_other_inputs;
            std::set_intersection(all_inputs.begin(), all_inputs.end(), other_inputs.begin(), other_inputs.end(), std::inserter(actual_other_inputs, actual_other_inputs.begin()));

            // also gather unique Boolean variable names of state inputs from corresponding nets
            std::vector<std::string> actual_state_input_names;
            for (const auto* n : actual_state_inputs)
            {
                actual_state_input_names.push_back(BooleanFunctionNetDecorator(*n).get_boolean_variable_name());
            }

            if (actual_control_inputs.size() <= 8)
            {
                const auto bf_const_0                 = BooleanFunction::Const(0, 1);
                const auto bf_const_1                 = BooleanFunction::Const(1, 1);
                std::vector<BooleanFunction> bf_const = {bf_const_0, bf_const_1};

                // set all other inputs to '0'
                for (auto& bf : bfs)
                {
                    for (const auto* other_in : actual_other_inputs)
                    {
                        const auto sub_res = bf.substitute(BooleanFunctionNetDecorator(*other_in).get_boolean_variable_name(), bf_const_0);
                        if (sub_res.is_error())
                        {
                            return ERR(sub_res.get_error());
                        }
                        bf = sub_res.get();
                    }
                    // bf = bf.simplify();
                }

                // brute-force all control inputs
                for (u32 i = 0; i < (1 << actual_control_inputs.size()); i++)
                {
                    // prepare values to assign to control inputs
                    std::map<std::string, BooleanFunction> control_values;
                    u32 j = 0;
                    for (auto* ci : actual_control_inputs)
                    {
                        control_values[BooleanFunctionNetDecorator(*ci).get_boolean_variable_name()] = bf_const.at((i >> j) & 1);
                        j++;
                    }

                    // actually assign the values
                    std::vector<std::vector<BooleanFunction::Value>> truth_tables_inverted(1 << bfs.size(), std::vector<BooleanFunction::Value>(bfs.size()));
                    for (j = 0; j < bfs.size(); j++)
                    {
                        const auto& bf    = bfs.at(j);
                        const auto tt_res = bf.substitute(control_values).map<std::vector<std::vector<BooleanFunction::Value>>>([&actual_state_input_names](auto&& bf) {
                            // return bf.simplify().compute_truth_table(actual_state_input_names);
                            return bf.compute_truth_table(actual_state_input_names);
                        });
                        if (tt_res.is_error())
                        {
                            return ERR(tt_res.get_error());
                        }

                        auto tmp = tt_res.get().front();
                        for (u32 k = 0; k < tmp.size(); k++)
                        {
                            truth_tables_inverted.at(k).at(j) = tmp.at(k);
                        }
                    }

                    std::vector<u8> sbox;
                    for (const auto& tt : truth_tables_inverted)
                    {
                        const auto u64_res = BooleanFunction::to_u64(tt);
                        if (u64_res.is_error())
                        {
                            return ERR(u64_res.get_error());
                        }
                        sbox.push_back(u64_res.get());
                    }

                    // check linear independence of outputs if more outputs than inputs
                    // remove outputs that are linearly dependent on others
                    // basically uses Gauss elimination
                    if (input_gates.size() != output_gates.size())
                    {
                        std::vector<u64> mat;
                        for (j = 0; j < output_gates.size(); j++)
                        {
                            u64 sum = 0;
                            for (u32 k = 0; k < (1 << input_gates.size()); k++)
                            {
                                sum += (1 << k) * ((sbox[k] >> j) & 1);
                            }
                            mat.push_back(sum);
                        }

                        for (j = 0; j < (1 << input_gates.size()); j++)
                        {
                            u32 kk = 0;
                            for (u32 k = 0; k < output_gates.size(); k++)
                            {
                                if ((mat.at(k) >> j) & 1 == 1)
                                {
                                    kk = k;
                                    break;
                                }
                            }

                            for (u32 k = kk + 1; k < output_gates.size(); k++)
                            {
                                if ((mat.at(k) >> j) & 1 == 1)
                                {
                                    mat.at(k) = mat.at(k) ^ mat.at(kk);
                                }
                            }
                        }

                        std::vector<u32> idx;
                        for (j = 0; j < mat.size(); j++)
                        {
                            if (mat.at(j) != 0)
                            {
                                idx.push_back(j);
                            }
                        }

                        if (idx.size() == input_gates.size())
                        {
                            for (j = 0; j < sbox.size(); j++)
                            {
                                u8 new_val = 0;
                                for (u32 k = 0; k < idx.size(); k++)
                                {
                                    new_val += (1 << k) * ((sbox.at(j) >> idx.at(k)) & 1);
                                }
                                sbox.at(j) = new_val;
                            }
                        }
                    }

                    std::set<u8> sbox_set(sbox.begin(), sbox.end());
                    if (sbox.size() != sbox_set.size())
                    {
                        log_debug("hawkeye", "found non-bijective S-box");
                        continue;
                    }

                    if (const auto sbox_res = db.lookup(sbox); sbox_res.is_ok())
                    {
                        sbox_name = sbox_res.get();
                        break;
                    }
                }
            }

            auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            if (!sbox_name.empty())
            {
                log_info("hawkeye", "identified {} S-box in {:.2f} seconds", sbox_name, duration_in_seconds);
            }
            else
            {
                log_info("hawkeye", "could not identify S-box in {:.2f} seconds", duration_in_seconds);
            }

            return OK(sbox_name);
        }
    }    // namespace hawkeye
}    // namespace hal