#include "module_identification/processing/post_processing.h"

#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "module_identification/candidates/verified_candidate.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/utils/utils.h"

#include <deque>
#include <limits>
#include <numeric>

// #define DEBUG_PRINT

namespace hal
{
    namespace module_identification
    {
        namespace
        {
            std::vector<std::vector<VerifiedCandidate>> filter_out_redundant_const_muls(std::vector<std::vector<VerifiedCandidate>>& candidate_sets)
            {
                std::set<std::vector<Net*>> found_counter_for_output;
                for (const auto& cs : candidate_sets)
                {
                    if (std::find(cs.front().m_types.begin(), cs.front().m_types.end(), CandidateType::counter) != cs.front().m_types.end())
                    {
                        found_counter_for_output.insert(cs.front().m_output_nets);
                    }
                }

                std::vector<std::vector<VerifiedCandidate>> filtered_candidates;
                for (const auto& cs : candidate_sets)
                {
                    if (std::find(cs.front().m_types.begin(), cs.front().m_types.end(), CandidateType::constant_multiplication) != cs.front().m_types.end())
                    {
                        if (found_counter_for_output.find(cs.front().m_output_nets) != found_counter_for_output.end())
                        {
                            continue;
                        }
                    }

                    filtered_candidates.push_back(cs);
                }

                return filtered_candidates;
            }

            // calculates how many registers both feed into the operands of a candidate and into the control signals
            u32 calculate_shared_source_regs(const Netlist* nl, const std::vector<VerifiedCandidate>& candidates, const std::vector<std::vector<Gate*>>& registers)
            {
                u32 shared_input_regs = 0;
                for (const auto& c : candidates)
                {
                    std::set<u32> op_regs;
                    for (const auto& nets : c.m_operands)
                    {
                        // TODO make this use the util find_neighboring_registers() function
                        for (const auto& n : nets)
                        {
                            const auto seq_inputs_res =
                                NetlistTraversalDecorator(*nl).get_next_matching_gates(n, false, [](const auto& g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
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
                                        op_regs.insert(reg_idx);
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // TODO make this use the util find_neighboring_registers() function
                    std::set<u32> ctrl_regs;
                    for (const auto& n : c.m_control_signals)
                    {
                        const auto seq_inputs_res =
                            NetlistTraversalDecorator(*nl).get_next_matching_gates(/*cache, */ n, false, [](const auto& g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
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
                                    ctrl_regs.insert(reg_idx);
                                    break;
                                }
                            }
                        }
                    }

                    std::vector<u32> intersection;
                    std::set_intersection(op_regs.begin(), op_regs.end(), ctrl_regs.begin(), ctrl_regs.end(), std::back_inserter(intersection));

                    shared_input_regs += intersection.size();
                }

                return shared_input_regs;
            }

            // calculates the maximum amount of inputs not covered by the candidate operands or control signals
            u32 calcualte_ignored_input_signals(const std::vector<VerifiedCandidate>& candidate_set)
            {
                u32 max_inputs_ignored = 0;
                for (const auto& c : candidate_set)
                {
                    // first collect all operand nets
                    std::set<Net*> covered_nets;
                    for (const auto& op : c.m_operands)
                    {
                        for (const auto& net : op)
                        {
                            if (net->is_gnd_net() || net->is_vcc_net())
                            {
                                continue;
                            }
                            covered_nets.insert(net);
                        }
                    }

                    // we treat control signals as covered
                    covered_nets.insert(c.m_control_signals.begin(), c.m_control_signals.end());

                    // second collect all nets that are not part of the operands
                    u32 ignored_inputs        = 0;
                    const auto all_input_nets = get_input_nets(c.m_gates);
                    for (const auto& input_net : all_input_nets)
                    {
                        if (input_net->is_gnd_net() || input_net->is_vcc_net())
                        {
                            continue;
                        }

                        if (covered_nets.find(input_net) == covered_nets.end())
                        {
                            ignored_inputs++;
                        }
                    }

                    max_inputs_ignored = std::max(max_inputs_ignored, ignored_inputs);
                }

                return max_inputs_ignored;
            }

            // calculates the maximum amount of nets belonging to the input operands and the outputs
            u32 calculate_maximum_io_signals(const std::vector<VerifiedCandidate>& candidate_set)
            {
                u32 max_io_signals = 0;
                for (const auto& c : candidate_set)
                {
                    // first collect all operand nets
                    std::set<Net*> io_signals;
                    for (const auto& op : c.m_operands)
                    {
                        for (const auto& net : op)
                        {
                            if (net->is_gnd_net() || net->is_vcc_net())
                            {
                                continue;
                            }
                            io_signals.insert(net);
                        }
                    }

                    io_signals.insert(c.m_output_nets.begin(), c.m_output_nets.end());

                    max_io_signals = std::max(max_io_signals, (u32)io_signals.size());
                }

                return max_io_signals;
            }

            // calculates the maximum amount of outputs not covered by the candidate output
            u32 calcualte_ignored_output_signals(const std::vector<VerifiedCandidate>& candidate_set)
            {
                u32 max_outputs_ignored = 0;
                for (const auto& c : candidate_set)
                {
                    const auto all_outputs     = get_output_nets(c.m_gates, false).size();
                    const auto c_outputs       = c.m_output_nets.size();
                    const auto outputs_ignored = (c_outputs > all_outputs) ? 0 : all_outputs - c_outputs;

                    if (outputs_ignored > max_outputs_ignored)
                    {
                        max_outputs_ignored = outputs_ignored;
                    }
                }

                return max_outputs_ignored;
            }

            // filters a set of candidates by a provided criteria and returns only the sets performing the best
            std::vector<std::vector<VerifiedCandidate>> filter_sets_by(const std::vector<std::vector<VerifiedCandidate>>& candidate_sets,
                                                                       const std::vector<std::vector<Gate*>>& registers,
                                                                       const bool minimize,
                                                                       const std::function<u32(const std::vector<VerifiedCandidate>&, const std::vector<std::vector<Gate*>>& registers)>& criteria)
            {
                std::vector<u32> fitlered_indices;
                u32 best_criteria_value = minimize ? std::numeric_limits<u32>::max() : std::numeric_limits<u32>::min();
                for (u32 idx = 0; idx < candidate_sets.size(); idx++)
                {
                    const u32 criteria_val = criteria(candidate_sets.at(idx), registers);
                    const bool new_best    = minimize ? (criteria_val < best_criteria_value) : (criteria_val > best_criteria_value);
                    const bool equal       = (criteria_val == best_criteria_value);

                    if (new_best)
                    {
                        best_criteria_value = criteria_val;
                        fitlered_indices.clear();
                        fitlered_indices.push_back(idx);
                        continue;
                    }

                    if (equal)
                    {
                        fitlered_indices.push_back(idx);
                    }
                }

                std::vector<std::vector<VerifiedCandidate>> filtered_candidates;
                for (const auto& idx : fitlered_indices)
                {
                    filtered_candidates.push_back(candidate_sets.at(idx));
                }

                return filtered_candidates;
            }

            std::vector<VerifiedCandidate> select_best_candidate_set(const Netlist* nl, const std::vector<VerifiedCandidate>& candidates, const std::vector<std::vector<Gate*>>& registers)
            {
                // dedupe candidates, dont know how this happens
                std::vector<VerifiedCandidate> unique_candidates = candidates;
                unique_candidates.erase(std::unique(unique_candidates.begin(), unique_candidates.end()), unique_candidates.end());

#ifdef DEBUG_PRINT
                std::cout << "Dedupe candidates to a size of " << unique_candidates.size() << std::endl;
#endif

                // sort the candidates into sets of candidates with the same gates and control signals
                std::map<std::pair<std::vector<Gate*>, std::vector<Net*>>, std::vector<VerifiedCandidate>> gates_control_to_candidate_sets;
                for (const auto& c : unique_candidates)
                {
                    gates_control_to_candidate_sets[{c.m_gates, c.m_control_signals}].push_back(c);
                }

                std::vector<std::vector<VerifiedCandidate>> candidate_sets;
                for (const auto& [_, c_set] : gates_control_to_candidate_sets)
                {
                    candidate_sets.push_back(c_set);
                }

#ifdef DEBUG_PRINT
                std::cout << "Sorted candidates into " << candidate_sets.size() << " candidate sets" << std::endl;
#endif

                /**
                 * Workaround: Some counter operations can be represented as a shifted addition x - (x >> 1).
                 * If there are counter candidates and constant multiplications candidates with the same output nets and an operand shift of -1, discard the constant multiplication
                 */
                candidate_sets = filter_out_redundant_const_muls(candidate_sets);
#ifdef DEBUG_PRINT
                std::cout << "Filtered redundant constant multiplication candidates, left with " << candidate_sets.size() << std::endl;
#endif

                // filter the candidate sets such that only the ones with the least input signals not part of the operands survive
                candidate_sets = filter_sets_by(candidate_sets, registers, true, [](const auto& c_set, const auto& _registers) {
                    UNUSED(_registers);
                    return calcualte_ignored_input_signals(c_set);
                });
#ifdef DEBUG_PRINT
                std::cout << "Filtered candidates with ignored input signals, left with " << candidate_sets.size() << std::endl;
#endif

                // filter the candidate sets such that only the ones with the most input and output signals survive
                candidate_sets = filter_sets_by(candidate_sets, registers, false, [](const auto& c_set, const auto& _registers) {
                    UNUSED(_registers);
                    return calculate_maximum_io_signals(c_set);
                });
#ifdef DEBUG_PRINT
                std::cout << "Filtered candidates with most IO signals, left with " << candidate_sets.size() << std::endl;
#endif

                // filter the candidate set such that only the ones with the least control signals survive
                candidate_sets = filter_sets_by(candidate_sets, registers, true, [](const auto& c_set, const auto& _registers) {
                    UNUSED(_registers);
                    return c_set.front().m_control_signals.size();
                });
#ifdef DEBUG_PRINT
                std::cout << "Filtered candidates with control signal count, left with " << candidate_sets.size() << std::endl;
#endif

                // filter the candidate sets such that only the ones with the most gates survive
                candidate_sets = filter_sets_by(candidate_sets, registers, false, [](const auto& c_set, const auto& _registers) {
                    UNUSED(_registers);
                    return c_set.front().m_gates.size();
                });
#ifdef DEBUG_PRINT
                std::cout << "Filtered candidates with candidate gate count, left with " << candidate_sets.size() << std::endl;
#endif

                // select candidate sets with the control signals that share the least source flip flops with the operands bits in the DANA grouping
                candidate_sets = filter_sets_by(candidate_sets, registers, true, [nl](const auto& c_set, const auto& r) { return calculate_shared_source_regs(nl, c_set, r); });
#ifdef DEBUG_PRINT
                std::cout << "Filtered candidates with shared source reg count, left with " << candidate_sets.size() << std::endl;
#endif

                // select the candidate set with the least amount of "module outputs" that are not part of the output bitvector
                candidate_sets = filter_sets_by(candidate_sets, registers, true, [](const auto& c_set, const auto& _registers) {
                    UNUSED(_registers);
                    return calcualte_ignored_output_signals(c_set);
                });
#ifdef DEBUG_PRINT
                std::cout << "Filtered candidates with ignored output signal count, left with " << candidate_sets.size() << std::endl;
#endif

                // select the candidate set with the most candidates
                candidate_sets = filter_sets_by(candidate_sets, registers, false, [](const auto& c_set, const auto& _registers) {
                    UNUSED(_registers);
                    return c_set.size();
                });
#ifdef DEBUG_PRINT
                std::cout << "Reduced candidates to {} sets" << candidate_sets.size() << std::endl;
#endif

                // TODO remove debug printing
                if (candidate_sets.size() > 1)
                {
                    log_warning("module_identification", "Found {} optimal candidate sets, need to investigate furhter.", candidate_sets.size());
                }

                return candidate_sets.front();
            }
        }    // namespace

        namespace
        {
            // calculates the sum of all registers that feed into any candidate operand
            u32 calculate_operand_source_regs(const Netlist* nl, const VerifiedCandidate& candidate, const std::vector<std::vector<Gate*>>& registers)
            {
                u32 total_source_regs = 0;
                for (const auto& nets : candidate.m_operands)
                {
                    std::set<u32> op_regs;
                    for (const auto& n : nets)
                    {
                        const auto seq_inputs_res =
                            NetlistTraversalDecorator(*nl).get_next_matching_gates(/*cache, */ n, false, [](const auto& g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
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
                                    op_regs.insert(reg_idx);
                                    break;
                                }
                            }
                        }
                    }

                    total_source_regs += op_regs.size();
                }

                return total_source_regs;
            }

            VerifiedCandidate select_best_sign_extended_comparison_candidate(const Netlist* nl, const std::vector<VerifiedCandidate>& candidates)
            {
                std::set<Net*> unique_nets;
                for (const auto& op_nets : candidates.front().m_operands)
                {
                    for (const auto& net : op_nets)
                    {
                        if (net->is_gnd_net() || net->is_vcc_net())
                        {
                            continue;
                        }

                        unique_nets.insert(net);
                    }
                }

                // find the set of gates that is reachable by more than one net of the operands nets
                const std::set<const Gate*> candidate_gates = {candidates.front().m_gates.begin(), candidates.front().m_gates.end()};

                std::map<Net*, std::set<Gate*>> net_to_reachable_gates;
                std::map<Gate*, u32> gate_to_reach_counter;

                for (const auto& net : unique_nets)
                {
                    const auto reachable_res =
                        NetlistTraversalDecorator(*nl).get_next_matching_gates_until(net, true, [&candidate_gates](const auto& g) { return candidate_gates.find(g) != candidate_gates.end(); });

                    net_to_reachable_gates.insert({net, reachable_res.get()});

                    for (const auto& gate : reachable_res.get())
                    {
                        gate_to_reach_counter[gate] += 1;
                    }
                }

                // for furhter analysis we do not consider all gates of the candidate, but only the gates that are reached by at least two other nets
                std::set<Gate*> core_gates;
                for (const auto& [gate, count] : gate_to_reach_counter)
                {
                    if (count > 1)
                    {
                        core_gates.insert(gate);
                    }
                }

                u32 best_candidate_idx = 0;
                u32 most_reached_gates = 0;

                for (u32 idx = 0; idx < candidates.size(); idx++)
                {
                    const auto& candidate = candidates.at(idx);

                    // find the sign extended bit for each candidate
                    Net* signed_net = nullptr;
                    for (const auto& operand : candidate.m_operands)
                    {
                        if (operand[operand.size() - 1] == operand[operand.size() - 2])
                        {
                            signed_net = operand[operand.size() - 1];
                            break;
                        }
                    }
                    if (signed_net == nullptr)
                    {
                        continue;
                    }

                    // select the candidate whose extended sign bit leads to most of the core gates
                    std::vector<Gate*> reachable_core_gates;
                    std::set_intersection(
                        net_to_reachable_gates.at(signed_net).begin(), net_to_reachable_gates.at(signed_net).end(), core_gates.begin(), core_gates.end(), std::back_inserter(reachable_core_gates));

                    if (reachable_core_gates.size() > most_reached_gates)
                    {
                        most_reached_gates = reachable_core_gates.size();
                        best_candidate_idx = idx;
                    }
                }

                return candidates.at(best_candidate_idx);
            }

            std::vector<VerifiedCandidate> filter_candidates_by(const std::vector<VerifiedCandidate>& candidates,
                                                                const std::vector<std::vector<Gate*>>& registers,
                                                                const bool minimize,
                                                                const std::function<u32(const VerifiedCandidate&, const std::vector<std::vector<Gate*>>& registers)>& criteria)
            {
                std::vector<u32> fitlered_indices;
                u32 best_criteria_value = minimize ? std::numeric_limits<u32>::max() : std::numeric_limits<u32>::min();
                for (u32 idx = 0; idx < candidates.size(); idx++)
                {
                    const u32 criteria_val = criteria(candidates.at(idx), registers);
                    const bool new_best    = minimize ? (criteria_val < best_criteria_value) : (criteria_val > best_criteria_value);
                    const bool equal       = (criteria_val == best_criteria_value);

                    if (new_best)
                    {
                        best_criteria_value = criteria_val;
                        fitlered_indices.clear();
                        fitlered_indices.push_back(idx);
                        continue;
                    }

                    if (equal)
                    {
                        fitlered_indices.push_back(idx);
                    }
                }

                std::vector<VerifiedCandidate> filtered_candidates;
                for (const auto& idx : fitlered_indices)
                {
                    filtered_candidates.push_back(candidates.at(idx));
                }

                return filtered_candidates;
            }

            std::vector<VerifiedCandidate> select_best_candidates(const Netlist* nl, const std::vector<VerifiedCandidate>& candidates, const std::vector<std::vector<Gate*>>& registers)
            {
                auto best_candidates = candidates;

                /**
                 * Select the candidates with the lowest amount of output nets that are not covered by the candidate
                 */
                best_candidates = filter_candidates_by(best_candidates, registers, true, [](const auto& c, const auto& _r) {
                    UNUSED(_r);
                    const auto all_outputs        = get_output_nets(c.m_gates, false).size();
                    const auto c_outputs          = c.m_output_nets.size();
                    const auto additional_outputs = (c_outputs > all_outputs) ? 0 : all_outputs - c_outputs;
                    return additional_outputs;
                });

#ifdef DEBUG_PRINT
                std::cout << "reduced candidate set to size " << best_candidates.size() << std::endl;
#endif

                /**
                 * Select the candidates containing the most non-const variables in the operands
                 */
                best_candidates = filter_candidates_by(best_candidates, registers, false, [](const auto& c, const auto& _r) {
                    UNUSED(_r);
                    u32 non_constant_count = 0;
                    for (const auto& nets : c.m_operands)
                    {
                        for (const auto& net : nets)
                        {
                            non_constant_count = (!net->is_gnd_net() && !net->is_vcc_net()) ? non_constant_count + 1 : non_constant_count;
                        }
                    }
                    return non_constant_count;
                });

#ifdef DEBUG_PRINT
                std::cout << "reduced candidate set to size " << best_candidates.size() << std::endl;
#endif

                /**
                 * Select the candidates with the lowest amount of constant signals 
                 * We find that the extensions with constants allowed for more irregular candidate formations. 
                 */

                best_candidates = filter_candidates_by(best_candidates, registers, true, [](const auto& c, const auto& _r) {
                    UNUSED(_r);
                    u32 constant_count = 0;
                    for (const auto& nets : c.m_operands)
                    {
                        for (const auto& net : nets)
                        {
                            constant_count = (!net->is_gnd_net() && !net->is_vcc_net()) ? constant_count : constant_count + 1;
                        }
                    }
                    return constant_count;
                });

#ifdef DEBUG_PRINT
                std::cout << "reduced candidate set to size " << best_candidates.size() << std::endl;
#endif

                /**
                 * Select the candidates with the lowest amount of signals in multiple operands
                 * Usually a candidate where each signal belongs to exactly one operand is prefered.
                 */

                best_candidates = filter_candidates_by(best_candidates, registers, true, [](const auto& c, const auto& _r) {
                    UNUSED(_r);
                    u32 multi_count = 0;
                    std::map<Net*, std::set<u32>> net_to_ops;
                    for (u32 op_idx = 0; op_idx < c.m_operands.size(); op_idx++)
                    {
                        const auto& nets = c.m_operands.at(op_idx);
                        for (const auto& net : nets)
                        {
                            net_to_ops[net].insert(op_idx);
                        }
                    }

                    for (const auto& [_, ops] : net_to_ops)
                    {
                        if (ops.size() > 1)
                        {
                            multi_count++;
                        }
                    }
                    return multi_count;
                });

#ifdef DEBUG_PRINT
                std::cout << "reduced candidate set to size " << best_candidates.size() << std::endl;
#endif

                /**
                 * Select the candidates with the lowest amount of source registers for each operand.
                 * Usually an operand that sources from exactly one reg is preferable.
                 */

                best_candidates = filter_candidates_by(best_candidates, registers, true, [nl](const auto& c, const auto& r) { return calculate_operand_source_regs(nl, c, r); });

#ifdef DEBUG_PRINT
                std::cout << "reduced candidate set to size " << best_candidates.size() << std::endl;
#endif

                /**
                 * This is a special rule only applying to sign extended comparisons.
                 * We 
                 */
                std::vector<VerifiedCandidate> reducable_signed_comparisons;
                std::vector<std::vector<VerifiedCandidate>::iterator> to_remove;
                for (auto it = best_candidates.begin(); it != best_candidates.end(); it++)
                {
                    if (*(it->m_types.begin()) == CandidateType::signed_less_equal || *(it->m_types.begin()) == CandidateType::signed_less_than)
                    {
                        reducable_signed_comparisons.push_back((*it));
                        to_remove.push_back(it);
                    }
                }
                for (const auto& elem : to_remove)
                {
                    best_candidates.erase(elem);
                }
                if (reducable_signed_comparisons.size() > 0)
                {
                    best_candidates.push_back(select_best_sign_extended_comparison_candidate(nl, reducable_signed_comparisons));
                }

                return best_candidates;
            }
        }    // namespace

        namespace
        {
            /*
            * This function tries to find a offset between to origins with the help of a previously generated offset matrix.
            * That matrix stores every known offset between two origins.
            * By building a chain of known origin-offset pairs we try to find offsets even for origins that do not share an already known offset.
            * During the chain building we populate the matrix along the way incase we find a valid offset.
            */
            Result<i32> get_offset(const u32& org1, const u32& org2, std::map<u32, std::map<u32, i32>>& m, std::set<std::set<u32>>& v)
            {
                if (v.find({org1, org2}) != v.end())
                {
                    return ERR("Already tried to follow that offset.");
                }

                v.insert({org1, org2});

                if (org1 == org2)
                {
                    m[org1][org2] = 0;
                    return OK(0);
                }

                if (m.find(org1) == m.end())
                {
                    return ERR("No valid offset to other origins.");
                }

                if (m.at(org1).find(org2) != m.at(org1).end())
                {
                    return OK(m.at(org1).at(org2));
                }

                for (auto& [dst, first_proxy_offset] : m.at(org1))
                {
                    auto second_proxy_offset_res = get_offset(dst, org2, m, v);
                    if (second_proxy_offset_res.is_error())
                    {
                        continue;
                    }
                    i32 second_proxy_offset = second_proxy_offset_res.get();

                    m[org1][org2] = first_proxy_offset + second_proxy_offset;
                    return OK(first_proxy_offset + second_proxy_offset);
                }

                return ERR("Not able to find a offset connection.");
            }

            /*
            * This function tries to build an offset matrix that maps each operand origin to all the other operand origins that overlap by providing an index for the same net.
            * Since that index maybe different we calulate an offset and check whether that offset is the same for all nets where the two origins overlap.
            * The matrix is populated in a way that the offsetat matrix[org_0][org_1] allow the user to calculate the index_1 = index_0 + offset.
            */
            Result<std::map<u32, std::map<u32, i32>>> build_offset_matrix(const std::map<Net*, std::map<u32, u32>>& indices)
            {
                // offset at matrix[org_0][org_1] means index_0 + offset = index_1
                std::map<u32, std::map<u32, i32>> origin_offset_matrix;

                for (const auto& [net, org_to_idx] : indices)
                {
                    std::map<u32, u32> all_possible_indices;

                    // fill all possible indices
                    for (const auto& [org, idx] : org_to_idx)
                    {
                        all_possible_indices[org] = idx;
                    }

                    // check whether all possible indices are just shifted version of each other with a stable offset
                    for (const auto& [org, idx] : org_to_idx)
                    {
                        for (const auto& [already_set_org, already_set_index] : all_possible_indices)
                        {
                            // there does not yet exist an offset between the already set index and the one to be added next
                            if (origin_offset_matrix[org].find(already_set_org) == origin_offset_matrix[org].end())
                            {
                                i32 new_index = idx;
                                i32 offset    = already_set_index - new_index;

                                origin_offset_matrix[org][already_set_org] = offset;
                                origin_offset_matrix[already_set_org][org] = -offset;
                            }
                            // check wether the already existing offset leads to the same index
                            else
                            {
                                i32 new_index = idx;
                                i32 offset    = origin_offset_matrix.at(org).at(already_set_org);

                                if (new_index + offset != i32(already_set_index))
                                {
                                    return ERR("unable to build offset matrix: failed to find valid offset for net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + " between "
                                               + std::to_string(org) + " and " + std::to_string(already_set_org));
                                }
                            }
                        }
                    }
                }

                return OK(origin_offset_matrix);
            }

            /**
             * This is pretty much cloned from the bitorder propagation since the task at hand is pretty similar.
             */
            std::vector<Net*> reconstruct_shifted_operand(const std::vector<std::vector<Net*>>& operands)
            {
                std::map<Net*, std::map<u32, u32>> indices;
                for (u32 op_idx = 0; op_idx < operands.size(); op_idx++)
                {
                    const auto& operand = operands.at(op_idx);
                    std::set<Net*> included_nets;

                    bool found_non_const_net = false;
                    for (u32 net_idx = 0; net_idx < operand.size(); net_idx++)
                    {
                        const auto& net = operand.at(net_idx);
                        if (!net->is_gnd_net() && !net->is_vcc_net())
                        {
                            found_non_const_net = true;
                        }

                        // only include the first index of each net of the operand
                        if (included_nets.find(net) != included_nets.end())
                        {
                            continue;
                        }

                        // do not include constant extensions (skip constant nets that come after non const nets)
                        if ((net->is_gnd_net() || net->is_vcc_net()) && found_non_const_net)
                        {
                            continue;
                        }

                        included_nets.insert(net);

                        indices[net].insert({op_idx, net_idx});
                    }
                }

                // try to find a consens between the different possible indices
                std::map<Net*, i32> consens_bitindices;

                auto offset_matrix_res = build_offset_matrix(indices);
                if (offset_matrix_res.is_error())
                {
                    log_error("module_identification", "failed to build offset matrix:\n{}", offset_matrix_res.get_error().get());
                    return {};
                }
                auto offset_matrix = offset_matrix_res.get();

                auto base_line = offset_matrix.begin()->first;

                for (const auto& [net, possible_indices] : indices)
                {
                    // pair of first possible org_mod and org_pin_group
                    u32 org = possible_indices.begin()->first;
                    // index at first possible origin
                    i32 org_index = possible_indices.begin()->second;
                    std::set<std::set<u32>> v;
                    auto offset_res = get_offset(org, base_line, offset_matrix, v);
                    if (offset_res.is_error())
                    {
                        log_error("module_identification",
                                  "failed to reconstruct shifted operand: failed to find offset for net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + " bewteen "
                                      + std::to_string(org) + " and " + std::to_string(base_line));
                        return {};
                    }
                    else
                    {
                        i32 offset              = offset_res.get();
                        consens_bitindices[net] = org_index + offset;
                        //std::cout << "Org Index: " << org_index << " Offset: " << offset << std::endl;
                    }
                }

                std::vector<Net*> ordered_nets;
                for (const auto& [net, _] : consens_bitindices)
                {
                    ordered_nets.push_back(net);
                }

                std::sort(ordered_nets.begin(), ordered_nets.end(), [&consens_bitindices](const auto& n1, const auto& n2) { return consens_bitindices.at(n1) < consens_bitindices.at(n2); });

                return ordered_nets;
            }

            Result<std::monostate> reconstruct_operands(VerifiedCandidate& vc, const std::vector<std::vector<Gate*>>& registers)
            {
                UNUSED(registers);
                std::vector<std::vector<Net*>> operands = vc.m_operands;
                std::vector<std::vector<Net*>> reconstructed_operands;

                if (vc.m_types.size() != 1)
                {
                    // return ERR("cannot reconstruct operands for candidate with multiple types");
                    return OK({});
                }
                const auto candidate_type = *(vc.m_types.begin());

                if (candidate_type == CandidateType::constant_multiplication)
                {
                    reconstructed_operands = {reconstruct_shifted_operand(operands)};
                    vc.m_operands          = reconstructed_operands;
                }

                return OK({});
            }

            Result<VerifiedCandidate> merge_candidate_set(const Netlist* nl, const std::vector<VerifiedCandidate>& candidate_set, const std::vector<std::vector<Gate*>>& registers)
            {
                std::map<std::map<Net*, BooleanFunction::Value>, std::vector<VerifiedCandidate>> ctrl_to_candidates;
                std::vector<VerifiedCandidate> filtered_candidates;

                // merge candidates that share a control mapping
                for (const auto& vc : candidate_set)
                {
                    // if candidate has more than one control mapping it is already merged
                    if (vc.m_control_signal_mappings.size() == 1)
                    {
                        ctrl_to_candidates[vc.m_control_signal_mappings.front()].push_back(vc);
                    }
                    else
                    {
                        filtered_candidates.push_back(vc);
                    }
                }

                for (const auto& [_, c] : ctrl_to_candidates)
                {
                    if (c.size() > 1)
                    {
                        // if we find more than one candidate for a given control mapping we have to decide on one of them
                        auto best_candidate = select_best_candidates(nl, c, registers).front();

                        const auto res = reconstruct_operands(best_candidate, registers);
                        if (res.is_error())
                        {
                            return ERR_APPEND(res.get_error(), "cannot merge canidate set: failed operand reconstruction");
                        }

                        filtered_candidates.push_back(best_candidate);
                    }
                    else
                    {
                        auto candidate = c.front();
                        const auto res = reconstruct_operands(candidate, registers);
                        if (res.is_error())
                        {
                            return ERR_APPEND(res.get_error(), "cannot merge canidate set: failed operand reconstruction");
                        }

                        filtered_candidates.push_back(candidate);
                    }
                }

#ifdef DEBUG_PRINT
                std::cout << "OPTIMAL CANDIDATE SET [" << filtered_candidates.size() << "]: " << std::endl;
                for (const auto& c : filtered_candidates)
                {
                    std::cout << c.get_candidate_info() << std::endl;
                }
#endif

                auto merge_res = VerifiedCandidate::merge(filtered_candidates);
                if (merge_res.is_error())
                {
                    return ERR_APPEND(merge_res.get_error(), "Could not merge candidate set in post processing: failed to merge candidates");
                }

                return merge_res;
            }

        }    // namespace

        VerifiedCandidate post_processing(const std::vector<VerifiedCandidate>& verified_candidates, const Netlist* nl, const std::vector<std::vector<Gate*>>& registers)
        {
            log_info("module_identification", "processing module for carry chain with {} verified variants", verified_candidates.size());

            if (verified_candidates.empty())
            {
                return VerifiedCandidate();
            }

#ifdef DEBUG_PRINT
            std::cout << "ALL CANDIDATES [" << verified_candidates.size() << "]: " << std::endl;
            for (const auto& vc : verified_candidates)
            {
                std::cout << vc.get_candidate_info() << std::endl;
            }
#endif

            // TODO check where duplicate candidates are created in the first place
            std::vector<VerifiedCandidate> unique_verified_candidates;
            for (const auto& vc : verified_candidates)
            {
                bool is_unique = true;
                for (const auto& uvc : unique_verified_candidates)
                {
                    if (uvc == vc)
                    {
                        is_unique = false;
                        break;
                    }
                }

                if (is_unique)
                {
                    unique_verified_candidates.push_back(vc);
                }
            }

            log_info("module_identification", "processing module for carry chain with {} unique verified variants", unique_verified_candidates.size());

#ifdef DEBUG_PRINT
            std::cout << "ALL UNIQUE CANDIDATES [" << unique_verified_candidates.size() << "]: " << std::endl;
            for (const auto& vc : unique_verified_candidates)
            {
                std::cout << vc.get_candidate_info() << std::endl;
            }
#endif
            const auto best_candidate_set = select_best_candidate_set(nl, unique_verified_candidates, registers);

            log_info("module_identification", "selected best candidate set containing {} candidates.", best_candidate_set.size());

#ifdef DEBUG_PRINT
            // TODO remove debug printing
            std::cout << "PRE-MERGE CANDIDATE SET [" << best_candidate_set.size() << "]: " << std::endl;
            for (const auto& c : best_candidate_set)
            {
                std::cout << c.get_candidate_info() << std::endl;
            }
#endif

            const auto best_candidate = merge_candidate_set(nl, best_candidate_set, registers);

            return best_candidate.get();
        }

    }    // namespace module_identification
}    // namespace hal