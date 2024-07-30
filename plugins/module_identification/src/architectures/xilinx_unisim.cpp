#include "module_identification/architectures/xilinx_unisim.h"

#include "hal_core/defines.h"
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
        namespace xilinx_unisim
        {
            namespace {
                /**
                 * @brief Finds and returns carry chains within a given netlist.
                 *
                 * This function identifies all carry chains in the provided netlist by retrieving
                 * carry gates and organizing them into chains based on their connections. Only 
                 * chains with more than two gates are considered.
                 *
                 * @param[in] nl - Pointer to the netlist to search for carry chains.
                 * @returns OK() and a vector of unique pointers to BaseCandidate on success, an error otherwise.
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
                        auto chain_res = netlist_utils::get_gate_chain(current_gate, {carry_type->get_pin_by_name("CI")}, {carry_type->get_pin_by_name("CO(3)")});
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
                            log_debug("module_identification", "\tcarry_gate: {}", carry_chain.front()->get_name());
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
                        base_candidates.push_back(std::move(base_candidate));
                    }

                    return base_candidates;
                }

                /**
                 * @brief Builds a set of structural candidates from a given base candidate.
                 *
                 * This function analyzes the gates in the provided base candidate to generate multiple structural candidates,
                 * taking into account specific properties and connections of the gates. It includes various strategies such as
                 * generating large candidates, candidates only with carry chains, and adding gates based on their types and connection to the carry chain gates.
                 *
                 * @param[in] base_candidate - Pointer to the base candidate from which structural candidates are generated.
                 * @returns OK() and a vector of candidates on success, an error otherwise.
                 */
                std::vector<std::unique_ptr<StructuralCandidate>> build_structural_candidates(BaseCandidate* base_candidate)
                {
                    std::vector<std::unique_ptr<StructuralCandidate>> candidates;

                    std::vector<Gate*> carry_chain = base_candidate->m_gates;

                    Gate* first_carry = carry_chain.front();

                    // check if first gate is carry
                    if (!first_carry->get_type()->has_property(GateTypeProperty::c_carry))
                    {
                        return std::vector<std::unique_ptr<StructuralCandidate>>();
                    }
                    log_debug("module_identification", "\tcarry_chain: {}", first_carry->get_name());

                    // generate very big candidates and go back until sequential gate or IO
                    {
                        std::unordered_set<Gate*> big_candidate_gates = {carry_chain.begin(), carry_chain.end()};
                        std::unordered_map<const Net*, std::unordered_set<Gate*>> cache;

                        for (const auto& gate : carry_chain)
                        {
                            // TODO make this use the NetlistTraversal decorator
                            auto gates = netlist_utils::get_next_gates(gate, false, 0, [](const Gate* g) {
                                bool add_gate = true;
                                if (g->get_type()->has_property(hal::GateTypeProperty::io))
                                {
                                    add_gate = false;
                                }
                                // if (g->get_type()->has_property(hal::GateTypeProperty::c_carry))
                                // {
                                //     add_gate = false;
                                // }
                                if (g->get_type()->has_property(hal::GateTypeProperty::dsp))
                                {
                                    add_gate = false;
                                }
                                if (g->get_type()->has_property(hal::GateTypeProperty::power))
                                {
                                    add_gate = false;
                                }
                                if (g->get_type()->has_property(hal::GateTypeProperty::ground))
                                {
                                    add_gate = false;
                                }
                                if (!g->get_type()->has_property(hal::GateTypeProperty::combinational))
                                {
                                    add_gate = false;
                                }
                                return add_gate;
                            });

                            for (const auto& gate_to_add : gates)
                            {
                                big_candidate_gates.insert(gate_to_add);
                            }
                        }

                        if (big_candidate_gates.size() < (carry_chain.size() * 128))
                        {
                            // create candidate with many many gates
                            auto gate_vec                                      = utils::to_vector(big_candidate_gates);
                            std::unique_ptr<StructuralCandidate> candidate_big = std::make_unique<StructuralCandidate>(base_candidate, gate_vec);
                            log_debug("module_identification", "big_candidate is {} big", candidate_big->m_gates.size());
                            candidates.push_back(std::move(candidate_big));
                        }
                        else
                        {
                            log_debug("module_identification", "big_candidate is too big with {} gates", big_candidate_gates.size());
                        }
                    }

                    // create candidate with only carry chain, nothing else
                    std::vector<Gate*> candidate_gates(carry_chain);

                    std::unique_ptr<StructuralCandidate> first_candidate = std::make_unique<StructuralCandidate>(base_candidate, candidate_gates);
                    if (first_candidate == nullptr)
                    {
                        log_error("module_identification", "nullptr candidate after creation");
                    }
                    candidates.push_back(std::move(first_candidate));

                    // get gates infront of CARRY
                    for (const auto& gate : carry_chain)
                    {
                        for (const auto& pred_endp : gate->get_predecessors())
                        {
                            auto pred_gate = pred_endp->get_gate();

                            if (pred_gate->is_gnd_gate() || pred_gate->is_vcc_gate())
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::io))
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::sequential))
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                            {
                                candidate_gates.push_back(pred_gate);
                            }
                        }
                    }

                    std::unique_ptr<StructuralCandidate> candidate_all_gates_infront_level_1 = std::make_unique<StructuralCandidate>(base_candidate, candidate_gates);
                    if (candidate_all_gates_infront_level_1 == nullptr)
                    {
                        log_error("module_identification", "nullptr candidate after creation");
                    }
                    candidates.push_back(std::move(candidate_all_gates_infront_level_1));

                    // get gates infront of SI and DI of CARRY
                    std::vector<Gate*> si_di0_candidate_gates(carry_chain);
                    std::vector<std::string> allowed_pins_si_di0 = {"S(0)", "S(1)", "S(2)", "S(3)", "DI(0)"};

                    for (const auto& gate : carry_chain)
                    {
                        for (const auto& pin : allowed_pins_si_di0)
                        {
                            const auto& pred_endp = gate->get_predecessor(pin);
                            if (pred_endp == nullptr)
                            {
                                continue;
                            }
                            auto pred_gate = pred_endp->get_gate();
                            if (pred_gate->is_gnd_gate() || pred_gate->is_vcc_gate())
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::io))
                            {
                                log_error("module_identification", "\tfound IO gate, but why?");
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                            {
                                continue;
                            }
                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::sequential))
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                            {
                                // log_debug("module_identification", "\t\tadding {}", pred_gate->get_name());
                                si_di0_candidate_gates.push_back(pred_gate);
                            }
                        }
                    }
                    std::unique_ptr<StructuralCandidate> candidate_gates_infront_of_si_di0 = std::make_unique<StructuralCandidate>(base_candidate, si_di0_candidate_gates);
                    if (candidate_gates_infront_of_si_di0 == nullptr)
                    {
                        log_error("module_identification", "nullptr candidate after creation");
                    }
                    candidates.push_back(std::move(candidate_gates_infront_of_si_di0));

                    // get gates infront of SI and DI of CARRY
                    // log_debug("module_identification", "\tsi_di pred:");
                    std::vector<Gate*> si_di_candidate_gates(carry_chain);
                    std::vector<std::string> allowed_pins_si_di = {"S(0)", "S(1)", "S(2)", "S(3)", "DI(0)", "DI(1)", "DI(2)", "DI(3)"};

                    for (const auto& gate : carry_chain)
                    {
                        for (const auto& pin : allowed_pins_si_di)
                        {
                            const auto& pred_endp = gate->get_predecessor(pin);
                            if (pred_endp == nullptr)
                            {
                                continue;
                            }
                            auto pred_gate = pred_endp->get_gate();
                            if (pred_gate->is_gnd_gate() || pred_gate->is_vcc_gate())
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::io))
                            {
                                log_error("module_identification", "\tfound IO gate, but why?");
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                            {
                                continue;
                            }
                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::sequential))
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                            {
                                si_di_candidate_gates.push_back(pred_gate);
                            }
                        }
                    }
                    std::unique_ptr<StructuralCandidate> candidate_gates_infront_of_si_di = std::make_unique<StructuralCandidate>(base_candidate, si_di_candidate_gates);
                    if (candidate_gates_infront_of_si_di == nullptr)
                    {
                        log_error("module_identification", "nullptr candidate after creation");
                    }
                    candidates.push_back(std::move(candidate_gates_infront_of_si_di));

                    // get gates infront of SI of CARRY
                    std::vector<Gate*> si_candidate_gates(carry_chain);

                    std::vector<std::string> allowed_pins_si = {"S(0)", "S(1)", "S(2)", "S(3)"};
                    for (const auto& gate : carry_chain)
                    {
                        for (const auto& pin : allowed_pins_si)
                        {
                            const auto& pred_endp = gate->get_predecessor(pin);
                            if (pred_endp == nullptr)
                            {
                                continue;
                            }
                            auto pred_gate = pred_endp->get_gate();
                            if (pred_gate->is_gnd_gate() || pred_gate->is_vcc_gate())
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::io))
                            {
                                log_error("module_identification", "\tfound IO gate, but why?");
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                            {
                                continue;
                            }
                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::sequential))
                            {
                                continue;
                            }

                            if (pred_gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                            {
                                // log_debug("module_identification", "\t\tadding {}", pred_gate->get_name());
                                si_candidate_gates.push_back(pred_gate);
                            }
                        }
                    }

                    std::vector<Gate*> candidate_gates_infront_of_si_gates;
                    for (const auto& g : si_candidate_gates)
                    {
                        candidate_gates_infront_of_si_gates.push_back(g);
                    }
                    std::unique_ptr<StructuralCandidate> candidate_gates_infront_of_si = std::make_unique<StructuralCandidate>(base_candidate, candidate_gates_infront_of_si_gates);

                    candidates.push_back(std::move(candidate_gates_infront_of_si));

                    std::vector<std::unique_ptr<StructuralCandidate>> candidates_to_add;

                    // get additional layer
                    for (const auto& cand : candidates)
                    {
                        if (cand == nullptr)
                        {
                            log_error("module_identification", "nullptr candidate");
                            continue;
                        }

                        if (cand->m_gates.empty())
                        {
                            log_error("module_identification", "candidate has no gates");
                            continue;
                        }
                        std::set<Gate*> additional_layer_gates;

                        for (const auto& gate : cand->m_gates)
                        {
                            if (gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                            {
                                continue;
                            }

                            for (const auto& pred_endp : gate->get_predecessors())
                            {
                                if (pred_endp == nullptr)
                                {
                                    continue;
                                }

                                auto pred_gate = pred_endp->get_gate();

                                if (pred_gate == nullptr)
                                {
                                    continue;
                                }

                                if (pred_gate->is_gnd_gate() || pred_gate->is_vcc_gate())
                                {
                                    continue;
                                }

                                if (pred_gate->get_type()->has_property(hal::GateTypeProperty::io))
                                {
                                    continue;
                                }

                                if (pred_gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                                {
                                    continue;
                                }

                                if (pred_gate->get_type()->has_property(hal::GateTypeProperty::sequential))
                                {
                                    continue;
                                }

                                if (pred_gate->get_type()->has_property(hal::GateTypeProperty::combinational))
                                {
                                    additional_layer_gates.insert(pred_gate);
                                }
                            }
                        }
                        std::vector<Gate*> all_gates(cand->m_gates);
                        for (const auto& g : additional_layer_gates)
                        {
                            all_gates.push_back(g);
                        }
                        std::unique_ptr<StructuralCandidate> additional_gate_candidate = std::make_unique<StructuralCandidate>(base_candidate, all_gates);
                        candidates_to_add.push_back(std::move(additional_gate_candidate));
                    }

                    for (auto& candidate_to_add : candidates_to_add)
                    {
                        candidates.push_back(std::move(candidate_to_add));
                    }

                    std::vector<std::unique_ptr<StructuralCandidate>> inv_candidates_to_add;

                    // get only inverters in front of candidate
                    for (const auto& cand : candidates)
                    {
                        if (cand == nullptr)
                        {
                            log_error("module_identification", "nullptr candidate");
                            continue;
                        }

                        if (cand->m_gates.empty())
                        {
                            log_error("module_identification", "candidate has no gates");
                            continue;
                        }
                        std::set<Gate*> additional_layer_gates;

                        for (const auto& gate : cand->m_gates)
                        {
                            if (gate->get_type()->has_property(hal::GateTypeProperty::c_carry))
                            {
                                continue;
                            }

                            for (const auto& pred_endp : gate->get_predecessors())
                            {
                                if (pred_endp == nullptr)
                                {
                                    continue;
                                }

                                auto pred_gate = pred_endp->get_gate();

                                if (pred_gate == nullptr)
                                {
                                    continue;
                                }

                                if (pred_gate->get_type()->has_property(hal::GateTypeProperty::c_inverter))
                                {
                                    additional_layer_gates.insert(pred_gate);
                                }
                            }
                        }
                        std::vector<Gate*> all_gates(cand->m_gates);
                        for (const auto& g : additional_layer_gates)
                        {
                            all_gates.push_back(g);
                        }
                        std::unique_ptr<StructuralCandidate> additional_gate_candidate = std::make_unique<StructuralCandidate>(base_candidate, all_gates);
                        inv_candidates_to_add.push_back(std::move(additional_gate_candidate));
                    }

                    for (auto& candidate_to_add : inv_candidates_to_add)
                    {
                        candidates.push_back(std::move(candidate_to_add));
                    }

                    // NOTE: i noticed that there are counter candidates that are not built correctly
                    // for each input net of the first carry gate and for each of its successors gates add a new candidate that includes this gate
                    {
                        std::vector<std::unique_ptr<StructuralCandidate>> new_candidates;
                        for (const auto& c : candidates)
                        {
                            for (const auto& in_net : carry_chain.front()->get_fan_in_nets())
                            {
                                if (in_net->is_gnd_net() || in_net->is_vcc_net())
                                {
                                    continue;
                                }

                                for (const auto& dst : in_net->get_destinations())
                                {
                                    if (dst->get_gate() == carry_chain.front())
                                    {
                                        continue;
                                    }

                                    if (dst->get_gate() == nullptr)
                                    {
                                        continue;
                                    }

                                    bool is_inverter         = dst->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter);
                                    bool is_single_input_lut = dst->get_gate()->get_type()->has_property(GateTypeProperty::c_lut) && (dst->get_gate()->get_boolean_functions().size() == 1)
                                                            && (dst->get_gate()->get_boolean_functions().begin()->second.get_variable_names().size() == 1);
                                    // place some constraints on the added gate
                                    if (!is_inverter && !is_single_input_lut)
                                    {
                                        continue;
                                    }

                                    std::unique_ptr<StructuralCandidate> new_candidate = std::make_unique<StructuralCandidate>(base_candidate, c->m_gates);
                                    new_candidate->m_gates.emplace_back(dst->get_gate());
                                    new_candidates.emplace_back(std::move(new_candidate));
                                }
                            }
                        }
                        for (auto& nc : new_candidates)
                        {
                            candidates.push_back(std::move(nc));
                        }
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
                            candidates_to_return.push_back(std::move(cand));
                        }
                    }

                    return candidates_to_return;
                }
            }   // namespace

            std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> generate_structural_candidates(const Netlist* nl)
            {
                std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> base_to_structural_candidates;

                auto base_candidates = find_carry_chains(nl);

                log_info("module_identification", "found {} carry chains, building structural variants now...", base_candidates.size());

                for (auto& base_candidate : base_candidates)
                {
                    std::vector<std::unique_ptr<StructuralCandidate>> structural_candidates = build_structural_candidates(base_candidate.get());
                    base_to_structural_candidates.push_back({std::move(base_candidate), std::move(structural_candidates)});
                }

                return base_to_structural_candidates;
            }
        }    // namespace xilinx_unisim
    }        // namespace module_identification
}    // namespace hal