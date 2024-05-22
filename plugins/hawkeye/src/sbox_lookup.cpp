#include "hawkeye/sbox_lookup.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hawkeye/state_candidate.h"

#include <algorithm>

namespace hal
{
    namespace hawkeye
    {
        Result<std::string>
            identify_sbox(const StateCandidate* candidate, const std::set<Gate*>& component, const std::set<Gate*>& input_gates, const std::set<Gate*>& output_gates, const SBoxDatabase& db)
        {
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

            if (actual_control_inputs.size() >= 8)
            {
                const auto bf_const_0                 = BooleanFunction::Const(0, 1);
                const auto bf_const_1                 = BooleanFunction::Const(1, 1);
                std::vector<BooleanFunction> bf_const = {bf_const_0, bf_const_1};

                // set all other inputs to '0'
                for (const auto* other_in : actual_other_inputs)
                {
                    for (auto& bf : bfs)
                    {
                        const auto sub_res = bf.substitute(BooleanFunctionNetDecorator(*other_in).get_boolean_variable_name(), bf_const_0);
                        if (sub_res.is_error())
                        {
                            return ERR(sub_res.get_error());
                        }
                        bf = sub_res.get().simplify();
                    }
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
                    std::vector<std::vector<BooleanFunction::Value>> truth_tables_inverted;
                    for (j = 0; j < bfs.size(); j++)
                    {
                        const auto& bf    = bfs.at(j);
                        const auto tt_res = bf.substitute(control_values).map<std::vector<std::vector<BooleanFunction::Value>>>([&actual_state_input_names](auto&& bf) {
                            return bf.simplify().compute_truth_table(actual_state_input_names);
                        });
                        if (tt_res.is_error())
                        {
                            return ERR(tt_res.get_error());
                        }

                        auto tmp = tt_res.get().front();
                        for (u32 k = 0; k < tmp.size(); k++)
                        {
                            truth_tables_inverted[k].push_back(tmp.at(k));
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
                        return sbox_res;
                    }
                }
            }

            return ERR("could not identify S-box");
        }
    }    // namespace hawkeye
}    // namespace hal