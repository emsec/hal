// #include "module_identification/utils.h"
#include "boolean_influence/include/boolean_influence/plugin_boolean_influence.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/utilities/log.h"

#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdlib.h>

namespace hal
{
    namespace module_identification
    {
        std::vector<Net*> get_input_nets(const std::vector<Gate*>& gates)
        {
            std::set<Net*> in_nets;
            for (Gate* g : gates)
            {
                // input nets: if predecessor is not in passed gates, it's an input net
                for (Net* n : g->get_fan_in_nets())
                {
                    if (!(n->is_gnd_net()) && !(n->is_vcc_net()))
                    {
                        if (n->is_global_input_net())
                        {
                            in_nets.insert(n);
                        }
                        else
                        {
                            if (n->get_num_of_sources() > 1)
                            {
                                log_error("module_identification", "Found multi driven net {} with ID {}! This is not handled yet!", n->get_name(), n->get_id());
                                continue;
                            }

                            for (Endpoint* s : n->get_sources())
                            {
                                if (std::find(gates.begin(), gates.end(), s->get_gate()) == gates.end())
                                {
                                    in_nets.insert(n);
                                }
                            }
                        }
                    }
                }
            }
            return {in_nets.begin(), in_nets.end()};
        }

        std::vector<Net*> get_output_nets(const std::vector<Gate*>& gates, bool only_external_destinations)
        {
            std::set<Net*> out_nets;
            for (Gate* g : gates)
            {
                // output nets: if successor is not in passed gates, it's an output net
                for (Net* n : g->get_fan_out_nets())
                {
                    if (n->is_global_output_net())
                    {
                        out_nets.insert(n);
                    }
                    else
                    {
                        bool is_output_net;
                        // for this variant we consider a net only an output net if it has ONLY destinations outside the subgraph
                        if (only_external_destinations)
                        {
                            is_output_net = !n->get_destinations().empty();
                            for (Endpoint* d : n->get_destinations())
                            {
                                if (std::find(gates.begin(), gates.end(), d->get_gate()) != gates.end())
                                {
                                    is_output_net = false;
                                    break;
                                }
                            }
                        }
                        // for this variant we consider a net an output net if it has ANY destination outside the subgraph
                        else
                        {
                            is_output_net = false;
                            for (Endpoint* d : n->get_destinations())
                            {
                                if (std::find(gates.begin(), gates.end(), d->get_gate()) == gates.end())
                                {
                                    is_output_net = true;
                                    break;
                                }
                            }
                        }

                        if (is_output_net)
                        {
                            out_nets.insert(n);
                        }
                    }
                }
            }
            return {out_nets.begin(), out_nets.end()};
        }

        std::set<u32> find_neighboring_registers(const Net* n, const std::vector<std::vector<Gate*>>& registers)
        {
            std::set<u32> reg_indices;

            // std::unordered_map<const Net*, std::unordered_set<Gate*>> cache;
            const auto seq_inputs_res =
                NetlistTraversalDecorator(*(n->get_netlist())).get_next_matching_gates(/* cache ,*/ n, false, [](const auto& g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
            if (seq_inputs_res.is_error())
            {
                log_error("module_identification", "{}", seq_inputs_res.get_error().get());
            }
            const auto seq_inputs = seq_inputs_res.get();

            for (const auto& si : seq_inputs)
            {
                for (u32 reg_idx = 0; reg_idx < registers.size(); reg_idx++)
                {
                    const auto& reg = registers.at(reg_idx);
                    if (std::find(reg.begin(), reg.end(), si) != reg.end())
                    {
                        reg_indices.insert(reg_idx);
                        break;
                    }
                }
            }

            return reg_indices;
        }

        std::set<u32> find_neighboring_registers(const std::vector<Net*> nets, const std::vector<std::vector<Gate*>>& registers)
        {
            std::set<u32> reg_indices;

            for (const auto& n : nets)
            {
                const auto neighbors = find_neighboring_registers(n, registers);
                reg_indices.insert(neighbors.begin(), neighbors.end());
            }

            return reg_indices;
        }

        namespace
        {
            std::vector<std::vector<u32>> permutations(const std::vector<u32>& initial)
            {
                std::vector<u32> p = initial;
                std::vector<std::vector<u32>> result;

                std::sort(p.begin(), p.end());

                do
                {
                    result.push_back({p.begin(), p.end()});
                } while (std::next_permutation(p.begin(), p.end()));

                return result;
            }

            u32 calculate_permuation_score(const std::vector<u32>& permutation, const std::vector<std::set<u32>>& neighboring_regs, const std::vector<std::set<u32>>& new_neighboring_regs)
            {
                u32 overlap          = 0;
                u32 non_over_lapping = 0;
                for (u32 op_idx = 0; op_idx < permutation.size(); op_idx++)
                {
                    const u32 new_op_idx = permutation.at(op_idx);

                    std::vector<u32> intersection;
                    std::set_intersection(neighboring_regs.at(op_idx).begin(),
                                          neighboring_regs.at(op_idx).end(),
                                          new_neighboring_regs.at(new_op_idx).begin(),
                                          new_neighboring_regs.at(new_op_idx).end(),
                                          std::back_inserter(intersection));

                    non_over_lapping += new_neighboring_regs.at(new_op_idx).size() - intersection.size();
                }

                return non_over_lapping;
            }
        }    // namespace

        std::vector<std::vector<Net*>> reorder_commutative_operands(const std::vector<std::vector<Net*>>& operands, const std::vector<std::vector<Gate*>>& registers, const u32 permute_start_index)
        {
            if (operands.size() > 6)
            {
                log_warning("module_identification", "reconstruction of 6+ commutative operands not yet supported and might take some time");
            }

            std::vector<std::vector<Net*>> corrected_operands;

            // vector of {0, ... , num_operands-1}
            std::vector<u32> op_indices(operands.size());
            std::iota(op_indices.begin(), op_indices.end(), 0);
            auto all_permutations = permutations(op_indices);

            std::vector<std::set<u32>> neighboring_regs(operands.size(), std::set<u32>{});
            for (u32 net_idx = 0; net_idx < operands.front().size(); net_idx++)
            {
                std::vector<u32> best_permutation;

                // find neighboring regs for each current operand net
                std::vector<std::set<u32>> new_neighboring_regs;
                for (u32 op_idx = 0; op_idx < operands.size(); op_idx++)
                {
                    new_neighboring_regs.push_back(find_neighboring_registers(operands.at(op_idx).at(net_idx), registers));
                }

                if (net_idx < permute_start_index)
                {
                    best_permutation = op_indices;
                }
                else
                {
                    // sort permutations/operand assigments based on the overlap of neighboring registers they produce and pick the one with the biggest overlap
                    std::sort(all_permutations.begin(), all_permutations.end(), [&neighboring_regs, &new_neighboring_regs](const auto& p1, const auto& p2) {
                        return calculate_permuation_score(p1, neighboring_regs, new_neighboring_regs) < calculate_permuation_score(p2, neighboring_regs, new_neighboring_regs);
                    });

                    best_permutation = all_permutations.front();
                }

                for (u32 op_idx = 0; op_idx < best_permutation.size(); op_idx++)
                {
                    const u32 new_op_idx = best_permutation.at(op_idx);

                    Net* new_net = operands.at(new_op_idx).at(net_idx);

                    if (op_idx >= corrected_operands.size())
                    {
                        corrected_operands.push_back({});
                    }
                    corrected_operands.at(op_idx).push_back(new_net);
                }

                for (u32 op_idx = 0; op_idx < operands.size(); op_idx++)
                {
                    neighboring_regs.at(op_idx).insert(new_neighboring_regs.at(op_idx).begin(), new_neighboring_regs.at(op_idx).end());
                }
            }

            return corrected_operands;
        }

    }    // namespace module_identification
}    // namespace hal