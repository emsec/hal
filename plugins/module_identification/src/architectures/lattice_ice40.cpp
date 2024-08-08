#include "module_identification/architectures/lattice_ice40.h"

#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/utilities/log.h"
#include "module_identification/candidates/base_candidate.h"
#include "module_identification/candidates/structural_candidate.h"

#include <vector>

namespace hal
{
    namespace module_identification
    {
        namespace lattice_ice40
        {
            namespace {
                /**
                 * @brief Finds carry chains within the netlist.
                 * 
                 * This function retrieves all carry gates from the netlist and groups them into chains.
                 * Each chain is formed by identifying gates that are connected via CI and CO pins.
                 * 
                 * @param[in] nl - The netlist to analyze.
                 * @returns A vector of unique pointers to base candidates representing the carry chains.
                 */
                std::vector<std::unique_ptr<BaseCandidate>> find_carry_chains(const Netlist* nl)
                {
                    std::vector<std::vector<Gate*>> carry_chains;

                    // retrieve all carry gates
                    std::vector<Gate*> carry_gates  = nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_carry); });
                    std::set<Gate*> carry_gates_set = std::set<Gate*>(carry_gates.begin(), carry_gates.end());

                    // collect carry chains until all carry gates have been analyzed
                    while (!carry_gates_set.empty())
                    {
                        Gate* current_gate         = *carry_gates_set.begin();
                        const GateType* carry_type = current_gate->get_type();

                        // get carry chains by defining appropriate filter function
                        auto chain_res = netlist_utils::get_gate_chain(current_gate, {carry_type->get_pin_by_name("CI")}, {carry_type->get_pin_by_name("CO")});
                        if (chain_res.is_error())
                        {
                            return std::vector<std::unique_ptr<BaseCandidate>>();
                        }
                        std::vector<Gate*> carry_chain = chain_res.get();

                        // remove shift register gates from candidate set
                        for (Gate* g : carry_chain)
                        {
                            carry_gates_set.erase(g);
                        }

                        // only consider carry chains with more than 2 gates for now
                        if (carry_chain.size() >= 2)
                        {
                            carry_chains.push_back(carry_chain);
                        }
                    }

                    // check whether a carry chain is a subset of another one
                    std::vector<std::vector<Gate*>> filtered_carry_chains;
                    for (u32 i = 0; i < carry_chains.size(); i++)
                    {
                        auto& c_test             = carry_chains.at(i);
                        std::set<Gate*> test_set = {c_test.begin(), c_test.end()};

                        bool is_subset = false;
                        for (u32 j = 0; j < carry_chains.size(); j++)
                        {
                            if (i == j)
                            {
                                continue;
                            }

                            auto& c_other             = carry_chains.at(j);
                            std::set<Gate*> other_set = {c_other.begin(), c_other.end()};

                            if (std::includes(other_set.begin(), other_set.end(), test_set.begin(), test_set.end()))
                            {
                                is_subset = true;
                                break;
                            }
                        }

                        if (!is_subset)
                        {
                            filtered_carry_chains.push_back(c_test);
                        }
                    }

                    std::vector<std::unique_ptr<BaseCandidate>> base_candidates;
                    for (const auto& carry_chain : filtered_carry_chains)
                    {
                        std::unique_ptr<BaseCandidate> base_candidate = std::make_unique<BaseCandidate>(carry_chain);
                        base_candidates.emplace_back(std::move(base_candidate));
                    }

                    return base_candidates;
                }

                /**
                 * @brief Generate subsets of the carry chain gates by choosing different starting gates and omitting the predecessors in the chain
                 * 
                 * Sometimes not all gates of the carry chain are actually used for the same arithmetic operation but implement miscellaneous surrounding logic.
                 * 
                 * @param[in] base_candidate - The base candidate to generate variants for.
                 * @param[in] all_base_candidates_gates - The gates of all base candidates.
                 * @returns A list of variants of the original carry chain.
                 */
                std::vector<std::vector<Gate*>> generate_carry_chain_variants(const BaseCandidate* base_candidate, const std::vector<std::vector<Gate*>>& all_base_candidates_gates)
                {
                    std::set<u32> possible_starting_points = {0};
                    for (u32 idx = 1; idx < base_candidate->m_gates.size(); idx++)
                    {
                        // check whether gate has constant input and the next one does not
                        if (idx < (base_candidate->m_gates.size() - 1))
                        {
                            const auto& g               = base_candidate->m_gates.at(idx);
                            const auto& constant_inputs = g->get_fan_in_nets([](const auto& n) { return n->is_gnd_net() || n->is_vcc_net(); });

                            const auto& g_next               = base_candidate->m_gates.at(idx + 1);
                            const auto& constant_inputs_next = g_next->get_fan_in_nets([](const auto& n) { return n->is_gnd_net() || n->is_vcc_net(); });

                            if (!constant_inputs.empty() && constant_inputs_next.empty())
                            {
                                possible_starting_points.insert(idx);
                                possible_starting_points.insert(idx + 1);
                            }
                        }

                        // check whether gate is part of multiple base candidates
                        if (idx < (base_candidate->m_gates.size() - 1))
                        {
                            const auto& g_curr = base_candidate->m_gates.at(idx);

                            bool is_part_of_multiple_base_candidates = false;
                            for (const auto& bc_gates : all_base_candidates_gates)
                            {
                                if (bc_gates == base_candidate->m_gates)
                                {
                                    continue;
                                }

                                if (std::find(bc_gates.begin(), bc_gates.end(), g_curr) != bc_gates.end())
                                {
                                    is_part_of_multiple_base_candidates = true;
                                    break;
                                }
                            }

                            if (is_part_of_multiple_base_candidates)
                            {
                                possible_starting_points.insert(idx + 1);
                            }
                        }

                        // check whether carry gate has successors outside of the carry chain, opposite to its predecessor
                        const auto& g_prev = base_candidate->m_gates.at(idx - 1);
                        const auto& g_curr = base_candidate->m_gates.at(idx);

                        bool prev_has_outside_succ = !g_prev
                                                        ->get_successors([&](const auto _gp, auto ep) {
                                                            UNUSED(_gp);
                                                            return std::find(base_candidate->m_gates.begin(), base_candidate->m_gates.end(), ep->get_gate()) == base_candidate->m_gates.end();
                                                        })
                                                        .empty();
                        bool curr_has_outside_succ = !g_curr
                                                        ->get_successors([&](const auto _gp, auto ep) {
                                                            UNUSED(_gp);
                                                            return std::find(base_candidate->m_gates.begin(), base_candidate->m_gates.end(), ep->get_gate()) == base_candidate->m_gates.end();
                                                        })
                                                        .empty();

                        if (curr_has_outside_succ && !prev_has_outside_succ)
                        {
                            possible_starting_points.insert(idx);
                        }
                    }

                    // finding too many starting points is computationally infeasible
                    if (possible_starting_points.size() > 4)
                    {
                        return {base_candidate->m_gates};
                    }

                    std::vector<std::vector<Gate*>> carry_chain_variants;
                    for (const auto& idx : possible_starting_points)
                    {
                        carry_chain_variants.push_back({base_candidate->m_gates.begin() + idx, base_candidate->m_gates.end()});
                    }

                    return carry_chain_variants;
                }


                /**
                 * @brief Builds a vector of structural candidates from a base candidate/carry chain.
                 *
                 * This function generates structural candidates by analyzing combinational gates 
                 * in the carry chain and their successors.
                 *
                 * @param[in] base_candidate - A pointer to the base candidate from which structural candidates are built.
                 * @param[in] carry_chain - A vector of pointers to Gate objects that form the carry chain.
                 * @returns OK() and a vector of candidates on success, an error otherwise.
                 */
                std::vector<std::unique_ptr<StructuralCandidate>> build_structural_candidates(BaseCandidate* base_candidate, const std::vector<Gate*>& carry_chain)
                {
                    std::vector<std::unique_ptr<StructuralCandidate>> candidates;

                    // NOTE add candidate that includes all combinational candidates in front of the carry gates
                    {
                        std::set<Gate*> all_comb_gates = {};
                        for (const auto& g : carry_chain)
                        {
                            const auto next_gates = netlist_utils::get_next_gates(g, false, 0, [](const auto ng) {
                                return ng->get_type()->has_property(GateTypeProperty::combinational) && !ng->get_type()->has_property(GateTypeProperty::c_carry)
                                    && !ng->get_type()->has_property(GateTypeProperty::ground) && !ng->get_type()->has_property(GateTypeProperty::power);
                            });
                            all_comb_gates.insert(g);
                            all_comb_gates.insert(next_gates.begin(), next_gates.end());
                        }

                        std::unique_ptr<StructuralCandidate> all_comb_candidate = std::make_unique<StructuralCandidate>(base_candidate, utils::to_vector(all_comb_gates));
                        if (all_comb_gates.size() <= (base_candidate->m_gates.size() * 16))
                        {
                            candidates.emplace_back(std::move(all_comb_candidate));
                        }
                    }

                    // NOTE Try to build a candidate that adds the  successors of the individual carry gates.
                    //      Afterwards propagate back from those gates to include all combinational predecessors
                    {
                        std::vector<Gate*> starting_gates = carry_chain;
                        std::set<Gate*> successor_gates;
                        for (const auto& sg : starting_gates)
                        {
                            for (const auto& ep : sg->get_successors())
                            {
                                if (ep != nullptr && ep->get_gate()->get_type()->has_property(GateTypeProperty::combinational) && !ep->get_gate()->get_type()->has_property(GateTypeProperty::c_carry))
                                {
                                    successor_gates.insert(ep->get_gate());
                                }
                            }
                        }

                        starting_gates.insert(starting_gates.end(), successor_gates.begin(), successor_gates.end());
                        std::set<Gate*> all_comb_gates = {};
                        for (const auto& g : starting_gates)
                        {
                            const auto next_gates = netlist_utils::get_next_gates(g, false, 0, [](const auto ng) {
                                return ng->get_type()->has_property(GateTypeProperty::combinational) && !ng->get_type()->has_property(GateTypeProperty::c_carry)
                                    && !ng->get_type()->has_property(GateTypeProperty::ground) && !ng->get_type()->has_property(GateTypeProperty::power);
                            });
                            all_comb_gates.insert(g);
                            all_comb_gates.insert(next_gates.begin(), next_gates.end());
                        }
                        // all_comb_candidate_ext->m_gates = utils::to_vector(all_comb_gates);

                        std::unique_ptr<StructuralCandidate> all_comb_candidate_ext = std::make_unique<StructuralCandidate>(base_candidate, utils::to_vector(all_comb_gates));
                        if (all_comb_gates.size() <= (base_candidate->m_gates.size() * 8))
                        {
                            candidates.emplace_back(std::move(all_comb_candidate_ext));
                        }
                    }

                    Gate* first_carry = carry_chain.front();
                    Gate* last_carry  = carry_chain.back();

                    std::vector<std::vector<Gate*>> candidate_gate_variants                   = {carry_chain};
                    std::vector<std::vector<Gate*>> candidate_gate_variants_without_last_gate = {carry_chain};

                    // check if first gate is carry
                    if (!first_carry->get_type()->has_property(GateTypeProperty::c_carry))
                    {
                        return candidates;
                    }
                    log_info("module_identification", "\tcarry_chain: {}", first_carry->get_name());

                    // get the other successors of the carry gates that are not part of the carry chain.
                    // this can introduce variants if there are multiple additional successors.
                    for (const auto& gate : carry_chain)
                    {
                        std::vector<std::vector<Gate*>> new_candidate_gate_variants;
                        std::vector<std::vector<Gate*>> new_candidate_gate_variants_without_last_gate;

                        if ((gate != last_carry) && (gate->get_successors().size() > 4))
                        {
                            return candidates;
                        }

                        for (const auto& suc_endp : gate->get_successors())
                        {
                            auto suc_gate = suc_endp->get_gate();

                            if (suc_gate->get_type()->has_property(hal::GateTypeProperty::io))
                            {
                                log_error("module_identification", "\tfound IO gate, but why?");
                            }
                            else if (suc_gate->get_type()->has_property(hal::GateTypeProperty::sequential))
                            {
                                continue;
                            }
                            else if (!suc_gate->get_type()->has_property(GateTypeProperty::c_carry))
                            {
                                // for each successor create a new candidate variant
                                for (const auto& cgv : candidate_gate_variants)
                                {
                                    if (std::find(cgv.begin(), cgv.end(), suc_gate) != cgv.end())
                                    {
                                        continue;
                                    }

                                    auto new_cgv = cgv;
                                    new_cgv.push_back(suc_gate);

                                    new_candidate_gate_variants.push_back(new_cgv);
                                }

                                if (new_candidate_gate_variants.size() > 4)
                                {
                                    return candidates;
                                }

                                if (gate != last_carry)
                                {
                                    new_candidate_gate_variants_without_last_gate = new_candidate_gate_variants;
                                }
                                else
                                {
                                    new_candidate_gate_variants_without_last_gate = candidate_gate_variants_without_last_gate;
                                }
                            }
                        }

                        if (!new_candidate_gate_variants.empty())
                        {
                            candidate_gate_variants                   = new_candidate_gate_variants;
                            candidate_gate_variants_without_last_gate = new_candidate_gate_variants_without_last_gate;
                        }
                    }

                    for (const auto& cgv : candidate_gate_variants)
                    {
                        std::set<Gate*> unique_gates    = {cgv.begin(), cgv.end()};
                        std::vector<Gate*> cgv_filtered = {unique_gates.begin(), unique_gates.end()};

                        candidates.push_back(std::make_unique<StructuralCandidate>(base_candidate, cgv_filtered));
                    }

                    for (const auto& cgv : candidate_gate_variants_without_last_gate)
                    {
                        std::set<Gate*> unique_gates    = {cgv.begin(), cgv.end()};
                        std::vector<Gate*> cgv_filtered = {unique_gates.begin(), unique_gates.end()};

                        candidates.push_back(std::make_unique<StructuralCandidate>(base_candidate, cgv_filtered));
                    }

                    // Many arithmetic operations require one or two gates that are not successors of the carry chain but instead share an input with the first gate of the chain
                    auto input_nets_first_gate = first_carry->get_fan_in_nets([](Net* net) { return !(net->is_gnd_net() || net->is_vcc_net()); });

                    std::vector<Gate*> total_successor_gates;
                    std::vector<Gate*> total_reduced_successor_gates;
                    std::vector<Gate*> total_more_reduced_successor_gates;

                    for (const auto input_net : input_nets_first_gate)
                    {
                        auto successor_enpoints = input_net->get_destinations();
                        for (const auto& ep : successor_enpoints)
                        {
                            auto gate = ep->get_gate();
                            if (gate == nullptr)
                            {
                                continue;
                            }

                            if (gate->get_type()->has_property(GateTypeProperty::sequential))
                            {
                                continue;
                            }

                            if (gate->get_type()->has_property(GateTypeProperty::c_carry))
                            {
                                continue;
                            }

                            if (gate->get_type()->has_property(hal::GateTypeProperty::io))
                            {
                                continue;
                            }

                            if (!gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                            {
                                continue;
                            }

                            total_successor_gates.push_back(gate);

                            const auto gt = gate->get_type();

                            // bool is_inv_xor_xnor     = gt->has_property(GateTypeProperty::c_inverter) || gt->has_property(GateTypeProperty::c_xor) || gt->has_property(GateTypeProperty::c_xnor);
                            // bool is_single_input_lut = gt->has_property(GateTypeProperty::c_lut) && (gate->get_fan_in_nets([](const auto& n) { return !n->is_gnd_net() && !n->is_vcc_net(); }).size() == 1);
                            bool is_not_four_input = gate->get_fan_in_nets([](Net* net) { return !(net->is_gnd_net() || net->is_vcc_net()); }).size() < 4;

                            bool to_be_considered = is_not_four_input;

                            if (!to_be_considered)
                            {
                                continue;
                            }

                            total_reduced_successor_gates.push_back(gate);

                            bool is_not_ao          = !(gt->has_property(GateTypeProperty::c_and) || gt->has_property(GateTypeProperty::c_or));
                            bool is_not_three_input = gate->get_fan_in_nets([](Net* net) { return !(net->is_gnd_net() || net->is_vcc_net()); }).size() < 3;

                            bool to_be_more_considered = is_not_three_input && is_not_ao;

                            if (!to_be_more_considered)
                            {
                                continue;
                            }

                            total_more_reduced_successor_gates.push_back(gate);
                        }
                    }

                    std::set<std::vector<Gate*>> possible_first_gates;

                    const auto considered_gates =
                        ((total_successor_gates.size() > 16) ? ((total_reduced_successor_gates.size() > 16) ? total_more_reduced_successor_gates : total_reduced_successor_gates) : total_successor_gates);

                    for (const auto& gate : considered_gates)
                    {
                        auto nets          = gate->get_fan_in_nets([](Net* net) { return !(net->is_gnd_net() || net->is_vcc_net()); });
                        bool consider_gate = true;
                        for (auto const& mandatory_input_net : input_nets_first_gate)
                        {
                            if (std::find(nets.begin(), nets.end(), mandatory_input_net) == nets.end())
                            {
                                consider_gate = false;
                                break;
                            }
                        }
                        if (!consider_gate)
                        {
                            continue;
                        }

                        possible_first_gates.insert({gate});

                        // check if first gate has inverter infront:
                        auto pres_first_gate = gate->get_predecessors();
                        std::vector<Gate*> new_inv_first_gates;
                        for (const auto& pre_first_gate : pres_first_gate)
                        {
                            if (pre_first_gate->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter))
                            {
                                new_inv_first_gates.emplace_back(pre_first_gate->get_gate());
                            }
                        }

                        for (const auto& inv_gate : new_inv_first_gates)
                        {
                            possible_first_gates.insert({gate, inv_gate});
                        }

                        // second first gate
                        for (const auto& gate2 : total_reduced_successor_gates)
                        {
                            if (gate == gate2)
                            {
                                continue;
                            }

                            possible_first_gates.insert({gate, gate2});
                        }
                    }

                    // relaxed rule: if we did not find a gate that has all inputs of the carry, we relax our rule...

                    // check if first carry has inverter infront:
                    std::vector<Gate*> first_carry_inverters;
                    for (const auto& pre_first_carry : first_carry->get_predecessors())
                    {
                        if (pre_first_carry->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter))
                        {
                            first_carry_inverters.emplace_back(pre_first_carry->get_gate());
                        }
                    }

                    if (possible_first_gates.empty())
                    {
                        for (const auto& gate : considered_gates)
                        {
                            possible_first_gates.insert({gate});

                            for (const auto& inv_gate : first_carry_inverters)
                            {
                                possible_first_gates.insert({gate, inv_gate});
                            }

                            // add second level candidate, if only one suc that is combinational
                            auto successors = gate->get_successors();
                            if (successors.size() != 1)
                            {
                                continue;
                            }
                            auto suc_gate = successors.front()->get_gate();
                            if (suc_gate->get_type()->has_property(hal::GateTypeProperty::io))
                            {
                                continue;
                            }
                            if (suc_gate->get_type()->has_property(GateTypeProperty::combinational))
                            {
                                possible_first_gates.insert({gate, suc_gate});
                            }
                        }
                    }

                    std::vector<std::vector<Gate*>> new_candidate_gate_variants;
                    for (const auto& c : candidates)
                    {
                        for (const auto& add_gates : possible_first_gates)
                        {
                            auto new_gates = c->m_gates;
                            new_gates.insert(new_gates.end(), add_gates.begin(), add_gates.end());
                            new_candidate_gate_variants.push_back(new_gates);
                        }
                    }

                    for (auto& new_cgv : new_candidate_gate_variants)
                    {
                        candidates.push_back(std::make_unique<StructuralCandidate>(base_candidate, new_cgv));
                    }

                    std::vector<std::unique_ptr<StructuralCandidate>> new_inverter_candidates;

                    // add inverters for substracts
                    for (const auto& candidate : candidates)
                    {
                        u32 inverter_counter                               = 0;
                        u32 carry_counter                                  = 0;
                        std::unique_ptr<StructuralCandidate> new_candidate = std::make_unique<StructuralCandidate>(base_candidate, candidate->m_gates);
                        for (const auto& gate : candidate->m_gates)
                        {
                            if (!gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                            {
                                auto suc_endpoints = gate->get_successors();
                                std::vector<Gate*> inverter_successors;
                                for (const auto& suc_endpoint : suc_endpoints)
                                {
                                    auto suc_gate = suc_endpoint->get_gate();
                                    if (suc_gate->get_type()->has_property(hal::GateTypeProperty::c_inverter))
                                    {
                                        inverter_successors.push_back(suc_gate);
                                    }
                                }

                                if (inverter_successors.size() > 2)
                                {
                                    continue;
                                }

                                for (const auto& is : inverter_successors)
                                {
                                    inverter_counter++;
                                    new_candidate->m_gates.push_back(is);
                                }
                            }
                            else
                            {
                                carry_counter++;
                            }
                        }
                        if (inverter_counter >= carry_counter - 1)
                        {
                            new_inverter_candidates.emplace_back(std::move(new_candidate));
                        }
                    }

                    for (auto& ic : new_inverter_candidates)
                    {
                        candidates.push_back(std::move(ic));
                    }

                    {
                        std::vector<std::unique_ptr<StructuralCandidate>> new_candidates;

                        // TODO: Probably redo this
                        // add weird last gates
                        for (const auto& candidate : candidates)
                        {
                            Gate* last_gate = candidate->m_gates.back();

                            for (const auto& pred_endp : last_gate->get_predecessors())
                            {
                                Gate* pred_gate = pred_endp->get_gate();
                                if (pred_gate->get_type()->has_property(hal::GateTypeProperty::c_carry) || !pred_gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                                {
                                    continue;
                                }

                                std::unique_ptr<StructuralCandidate> new_candidate = std::make_unique<StructuralCandidate>(base_candidate, candidate->m_gates);
                                new_candidate->m_gates.emplace_back(pred_gate);
                                new_candidates.emplace_back(std::move(new_candidate));
                            }
                        }
                        for (auto& nc : new_candidates)
                        {
                            candidates.push_back(std::move(nc));
                        }
                    }

                    // TODO this should not be happening, need to investigate this in the future
                    // delete all gnd and vcc gates from candidates
                    for (auto& cand : candidates)
                    {
                        if (!std::any_of(cand->m_gates.begin(), cand->m_gates.end(), [](const auto& g) { return g->is_gnd_gate() || g->is_vcc_gate(); }))
                        {
                            continue;
                        }


                        cand->m_gates.erase(std::remove_if(cand->m_gates.begin(),
                                                        cand->m_gates.end(),
                                                        [](const auto& g) {
                                                            return g->is_gnd_gate() || g->is_vcc_gate();    // put your condition here
                                                        }),
                                            cand->m_gates.end());

                    }

                    // remove same candidates
                    std::set<std::set<Gate*>> all_sets;
                    u32 old_size = candidates.size();
                    std::vector<std::unique_ptr<StructuralCandidate>> candidates_to_return;

                    for (auto& cand : candidates)
                    {
                        std::set<Gate*> set_gates = {cand->m_gates.begin(), cand->m_gates.end()};
                        if (all_sets.find(set_gates) == all_sets.end())
                        {
                            all_sets.insert(set_gates);
                            candidates_to_return.push_back(std::make_unique<StructuralCandidate>(base_candidate, std::vector<Gate*>{set_gates.begin(), set_gates.end()}));
                        }
                    }

                    log_info("module_identification", "\tremaining {}/{} after double candidate removal", candidates_to_return.size(), old_size);
                    return candidates_to_return;
                }
            }

            std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> generate_structural_candidates(const Netlist* nl)
            {
                std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> base_to_structural_candidates;

                auto base_candidates = find_carry_chains(nl);

                std::vector<std::vector<Gate*>> all_base_candidates_gates;
                for (const auto& bc : base_candidates)
                {
                    all_base_candidates_gates.push_back(bc->m_gates);
                }

                log_info("module_identification", "found {} carry chains, building structural variants now...", base_candidates.size());

                for (auto& base_candidate : base_candidates)
                {
                    const auto& carry_chain_variants = generate_carry_chain_variants(base_candidate.get(), all_base_candidates_gates);

                    std::vector<std::unique_ptr<StructuralCandidate>> structural_candidates;
                    for (const auto& ccv : carry_chain_variants)
                    {
                        auto new_candidates = build_structural_candidates(base_candidate.get(), ccv);
                        for (auto& nc : new_candidates)
                        {
                            structural_candidates.push_back(std::move(nc));
                        }
                    }

                    base_to_structural_candidates.push_back({std::move(base_candidate), std::move(structural_candidates)});
                }

                return base_to_structural_candidates;
            }
        }    // namespace lattice_ice40
    }        // namespace module_identification
}    // namespace hal