#include "dataflow_analysis/evaluation/evaluation.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/output_generation/textual_output.h"
#include "dataflow_analysis/processing/passes/group_by_control_signals.h"
#include "dataflow_analysis/utils/parallel_for_each.h"
#include "dataflow_analysis/utils/progress_printer.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <algorithm>
#include <limits>

namespace hal
{
    namespace dataflow
    {
        namespace evaluation
        {
            using scoring = std::map<std::set<u32>, float>;

            namespace
            {
                scoring compute_scores(const processing::Result& result)
                {
                    std::map<std::set<u32>, float> results_by_vote;

                    for (const auto& state : result.unique_groupings)
                    {
                        for (const auto& it : state->gates_of_group)
                        {
                            std::set<u32> sorted_gates(it.second.begin(), it.second.end());
                            results_by_vote[sorted_gates] += result.pass_combinations_leading_to_grouping.at(state).size();
                        }
                    }

                    return results_by_vote;
                }

                /*
        bool unordered_sets_overlap(const std::unordered_set<u32>& set_1, const std::unordered_set<u32>& set_2)
        {
            for (const auto& element_1 : set_1)
            {
                if (set_2.find(element_1) != set_2.end())
                {
                    return true;
                }
            }
            return false;
        }

        std::shared_ptr<Grouping> generate_output_sebus(const Configuration& config, context& ctx, const std::shared_ptr<Grouping>& initial_grouping, const scoring& scores)
        {
            // log_info("dataflow", "start scoring");
            auto& netlist_abstr = initial_grouping->netlist_abstr;

            std::unordered_set<u32> gathered_gates;
            std::vector<std::unordered_set<u32>> final_groups;

            // first find all groups with prioritised sizes
            if (!config.prioritized_sizes.empty())
            {
                // sort all groups of all results by size and scoring
                std::map<u32, std::map<float, std::vector<std::unordered_set<u32>>>> size_sorted_results;
                for (const auto& [group_sg, group_votes] : scores)
                {
                    std::unordered_set<u32> unordered_gates(group_sg.begin(), group_sg.end());
                    size_sorted_results[group_sg.size()][group_votes].push_back(unordered_gates);
                }

                // for each prioritiesed size find all non overlapping groups with this size
                for (const auto& size : config.prioritized_sizes)
                {
                    // groups of this size
                    const auto& groups_of_size = size_sorted_results[size];
                    for (auto it = groups_of_size.rbegin(); it != groups_of_size.rend(); ++it)
                    {
                        // single group
                        const auto groups = it->second;
                        for (const auto& group : groups)
                        {
                            if (!unordered_sets_overlap(group, gathered_gates))
                            {
                                gathered_gates.insert(group.begin(), group.end());
                                final_groups.push_back(group);
                            }
                        }
                    }
                }
            }

            // sort all groups of all result states by their control signals and by their score
            std::map<std::vector<u32>, std::map<float, std::vector<std::unordered_set<u32>>>> sorted_results;
            for (const auto& [group_sg, group_votes] : scores)
            {
                std::unordered_set<u32> unordered_gates(group_sg.begin(), group_sg.end());
                auto gate_id     = *group_sg.begin();
                const auto& fingerprint = netlist_abstr.gate_to_fingerprint.at(gate_id);
                sorted_results[fingerprint][group_votes].push_back(unordered_gates);
            }

            // recreate each of the control groups with groups of the structural results
            for (const auto& [control_group_id, control_group] : ctx.control_state->gates_of_group)
            {
                // get the control signals of the current control_group
                auto gate_id     = *control_group.begin();
                const auto& fingerprint = netlist_abstr.gate_to_fingerprint.at(gate_id);

                // get all groups from all result states with the same control singals
                const auto& result_groups = sorted_results.at(fingerprint);

                // gather groups which are subgroups of the control group, prioritise higher scores
                std::vector<std::unordered_set<u32>> new_groups;
                for (auto it = result_groups.rbegin(); it != result_groups.rend(); it++)
                {
                    const auto& vote_groups = it->second;
                    for (const auto& vote_group : vote_groups)
                    {
                        if (vote_group.size() > 1 && !unordered_sets_overlap(vote_group, gathered_gates))
                        {
                            new_groups.push_back(vote_group);
                            gathered_gates.insert(vote_group.begin(), vote_group.end());
                        }
                    }
                }

                // manage gates not gathered in the previous process
                if (ctx.phase == 0)
                {
                    // in phase 0 assigne each gate to its own group
                    for (auto gate : control_group)
                    {
                        if (gathered_gates.find(gate) == gathered_gates.end())
                        {
                            new_groups.push_back({gate});
                        }
                    }
                }
                else
                {
                    // in phase 1 group the remaining gates in the biggest possible groups,
                    // where each group consists of gates with the same control signals, type and overlapping register stages
                    std::unordered_set<u32> single_gates_group;
                    for (auto gate : control_group)
                    {
                        if (gathered_gates.find(gate) == gathered_gates.end())
                        {
                            single_gates_group.insert(gate);
                        }
                    }
                    if (!single_gates_group.empty())
                    {
                        std::unordered_set<u32> grouped_gates;
                        for (auto gate : single_gates_group)
                        {
                            if (grouped_gates.find(gate) != grouped_gates.end())
                            {
                                continue;
                            }

                            std::unordered_set<u32> new_single_group;

                            const auto& reg_stages = netlist_abstr.gate_to_register_stages.at(gate);
                            std::set<u32> register_stage_intersect(reg_stages.begin(), reg_stages.end());

                            const auto& type = netlist_abstr.nl->get_gate_by_id(gate)->get_type();
                            for (const auto& test_gate : single_gates_group)
                            {
                                if (grouped_gates.find(test_gate) != grouped_gates.end())
                                {
                                    continue;
                                }

                                const auto& test_type = netlist_abstr.nl->get_gate_by_id(gate)->get_type();
                                if (type == test_type)
                                {
                                    const auto& test_reg_stages = netlist_abstr.gate_to_register_stages.at(test_gate);
                                    std::set<u32> sorted_test_reg_stages(test_reg_stages.begin(), test_reg_stages.end());

                                    std::set<u32> new_intersect;
                                    std::set_intersection(register_stage_intersect.begin(),
                                                          register_stage_intersect.end(),
                                                          sorted_test_reg_stages.begin(),
                                                          sorted_test_reg_stages.end(),
                                                          std::inserter(new_intersect, new_intersect.begin()));

                                    if (!new_intersect.empty())
                                    {
                                        new_single_group.insert(test_gate);
                                        grouped_gates.insert(test_gate);

                                        register_stage_intersect.clear();
                                        register_stage_intersect.insert(new_intersect.begin(), new_intersect.end());

                                        gathered_gates.insert(test_gate);
                                    }
                                }
                            }

                            new_groups.push_back(new_single_group);
                        }
                    }
                }

                final_groups.insert(final_groups.end(), new_groups.begin(), new_groups.end());
                // log_info("dataflow", "gathered = {}, all = {}", gathered_gates.size(), netlist_abstr.all_sequential_gates.size());
            }

            // create resulting state
            std::shared_ptr<Grouping> output = std::make_shared<Grouping>(netlist_abstr);
            u32 id_counter                   = -1;

            for (const auto& group : final_groups)
            {
                u32 new_group_id = ++id_counter;

                output->group_control_fingerprint_map[new_group_id] = netlist_abstr.gate_to_fingerprint.at(*group.begin());

                output->gates_of_group.emplace(new_group_id, group);
                for (const auto& sg : group)
                {
                    output->parent_group_of_gate.emplace(sg, new_group_id);
                }
            }

            // log_info("dataflow", "scoring executed in {:3.2f}s", seconds_since(begin_time));

            return output;
        }
        */

                std::shared_ptr<Grouping> generate_output(const Configuration& config, const std::shared_ptr<Grouping>& initial_grouping, const scoring& scores)
                {
                    measure_block_time("majority voting");
                    auto& netlist_abstr = initial_grouping->netlist_abstr;

                    const u32 bad_group_size = 7;

                    struct candidate
                    {
                        const std::set<u32>* group;
                        float score;
                    };

                    std::vector<candidate> sorted_results;

                    // sort the scoring results
                    // result has all good groups sorted by score followed by all bad groups sorted by score
                    sorted_results.reserve(scores.size());
                    std::transform(scores.begin(), scores.end(), std::back_inserter(sorted_results), [](auto& it) { return candidate{&it.first, it.second}; });
                    std::sort(sorted_results.begin(), sorted_results.end(), [&](auto a, auto b) {
                        auto a_prio = std::find(config.prioritized_sizes.begin(), config.prioritized_sizes.end(), a.group->size());
                        auto b_prio = std::find(config.prioritized_sizes.begin(), config.prioritized_sizes.end(), b.group->size());

                        if (a_prio < b_prio)
                        {
                            return true;
                        }
                        if (a_prio > b_prio)
                        {
                            return false;
                        }

                        auto a_is_bad = a.group->size() <= bad_group_size;
                        auto b_is_bad = b.group->size() <= bad_group_size;

                        if (!a_is_bad && b_is_bad)
                        {
                            return true;
                        }
                        if (!b_is_bad && a_is_bad)
                        {
                            return false;
                        }

                        return a.score > b.score;
                    });

                    // mark all sequential gates as unassigned gates
                    std::vector<u32> unassigned_gates;
                    unassigned_gates.reserve(netlist_abstr.all_sequential_gates.size());
                    std::transform(netlist_abstr.all_sequential_gates.begin(), netlist_abstr.all_sequential_gates.end(), std::back_inserter(unassigned_gates), [](auto& g) { return g->get_id(); });

                    // sort unassignes gates to be able to use std::algorithms
                    std::sort(unassigned_gates.begin(), unassigned_gates.end());

                    std::shared_ptr<Grouping> output = std::make_shared<Grouping>(netlist_abstr);

                    u32 id_counter = -1;

                    const float percent_scan = 0.1f;

                    log_info("dataflow", "got {} voting groups", sorted_results.size());

                    // scan groups until all or done
                    float original_size = sorted_results.size();
                    progress_printer progress_bar;
                    while (!sorted_results.empty())
                    {
                        progress_bar.print_progress((original_size - sorted_results.size()) / original_size);

                        // precompute the group indices of each gate
                        std::unordered_map<u32, u32> max_group_size_of_gate;
                        std::unordered_map<u32, std::vector<u32>> groups_of_gate;

                        for (auto g : unassigned_gates)
                        {
                            max_group_size_of_gate.emplace(g, 0);
                            groups_of_gate.emplace(g, std::vector<u32>{});
                        }

                        for (u32 i = 0; i < sorted_results.size(); ++i)
                        {
                            auto size = (u32)sorted_results[i].group->size();
                            for (auto g : *sorted_results[i].group)
                            {
                                auto it    = max_group_size_of_gate.find(g);
                                it->second = std::max(it->second, size);
                                groups_of_gate.at(g).push_back(i);
                            }
                        }

                        // counts sequential gates that would end up in bad groups for each scanned candidate
                        // find the current scan range
                        bool first_is_bad       = sorted_results.front().group->size() <= bad_group_size;
                        bool first_is_preferred = std::find(config.prioritized_sizes.begin(), config.prioritized_sizes.end(), sorted_results.front().group->size()) != config.prioritized_sizes.end();
                        float lower_score_bound = sorted_results.front().score * (1.f - percent_scan);

                        u32 num_scanned_groups = 0;
                        for (; num_scanned_groups < sorted_results.size(); ++num_scanned_groups)
                        {
                            if (sorted_results[num_scanned_groups].score < lower_score_bound)
                            {
                                break;
                            }

                            auto scanned_group_size = sorted_results[num_scanned_groups].group->size();

                            if (!first_is_bad && scanned_group_size <= bad_group_size)
                            {
                                break;
                            }
                            if (first_is_preferred && std::find(config.prioritized_sizes.begin(), config.prioritized_sizes.end(), scanned_group_size) == config.prioritized_sizes.end())
                            {
                                break;
                            }
                        }

                        // scan the first few groups
                        std::vector<float> badness_score_of_group(num_scanned_groups, 0.0f);

                        utils::parallel_for_each(0, num_scanned_groups, [&](u32 scanned_group_idx) {
                            auto& scanned_group = *sorted_results[scanned_group_idx].group;
                            // get all unassigned gates that are not in this group
                            std::vector<u32> unaffected_gates;
                            unaffected_gates.reserve(unassigned_gates.size() - scanned_group.size());
                            std::set_difference(unassigned_gates.begin(), unassigned_gates.end(), scanned_group.begin(), scanned_group.end(), std::back_inserter(unaffected_gates));

                            float score = 0.f;

                            // ##############################################

                            std::unordered_set<u32> affected_groups;

                            for (auto g : scanned_group)
                            {
                                auto& gg = groups_of_gate.at(g);
                                affected_groups.insert(gg.begin(), gg.end());
                            }
                            affected_groups.erase(scanned_group_idx);

                            for (auto gr : affected_groups)
                            {
                                if (std::find(config.prioritized_sizes.begin(), config.prioritized_sizes.end(), sorted_results[gr].group->size()) != config.prioritized_sizes.end())
                                {
                                    score += 1.0f;
                                }
                            }

                            // get the maximum size of the groups of each unaffected gate
                            for (auto g : unaffected_gates)
                            {
                                if (max_group_size_of_gate.at(g) <= bad_group_size)
                                {
                                    score += 1.0f;
                                }
                            }

                            // ##############################################

                            badness_score_of_group[scanned_group_idx] = score;
                        });

                        // log_info("dataflow", "scanned {}/{} groups in this iteration", num_scanned_groups, sorted_results.size());

                        // get the scanned group that would result in the least amount of bad groups
                        u32 best_choice = std::distance(badness_score_of_group.begin(), std::min_element(badness_score_of_group.begin(), badness_score_of_group.end()));
                        auto best_group = *sorted_results[best_choice].group;

                        // add this group to the final output
                        {
                            u32 new_group_id = ++id_counter;

                            output->group_control_fingerprint_map[new_group_id] = netlist_abstr.gate_to_fingerprint.at(*best_group.begin());
                            output->operations_on_group_allowed[new_group_id]   = initial_grouping->operations_on_group_allowed.at(best_choice);

                            output->gates_of_group[new_group_id].insert(best_group.begin(), best_group.end());
                            for (const auto& sg : best_group)
                            {
                                output->parent_group_of_gate[sg] = new_group_id;
                            }
                        }

                        unassigned_gates.erase(std::remove_if(unassigned_gates.begin(), unassigned_gates.end(), [&best_group](auto id) { return best_group.find(id) != best_group.end(); }),
                                               unassigned_gates.end());

                        // remove all candidate groupings that contain any of the newly assigned gates
                        sorted_results.erase(std::remove_if(sorted_results.begin(),
                                                            sorted_results.end(),
                                                            [&best_group](auto check_group) {
                                                                return std::any_of(check_group.group->begin(), check_group.group->end(), [&best_group](auto check_group_gate) {
                                                                    return best_group.find(check_group_gate) != best_group.end();
                                                                });
                                                            }),
                                             sorted_results.end());
                    }
                    progress_bar.clear();

                    for (auto g : unassigned_gates)
                    {
                        u32 new_group_id = ++id_counter;

                        output->group_control_fingerprint_map[new_group_id] = netlist_abstr.gate_to_fingerprint.at(g);
                        output->operations_on_group_allowed[new_group_id]   = true;

                        output->gates_of_group[new_group_id].insert(g);
                        output->parent_group_of_gate[g] = new_group_id;
                    }
                    return output;
                }

            }    // namespace

            evaluation::Result run(const Configuration& config, Context& ctx, const std::shared_ptr<Grouping>& initial_grouping, const processing::Result& result)
            {
                evaluation::Result output;
                output.is_final_result = false;

                auto scores = compute_scores(result);

                /*
        const int last_phase = 1;

        output.merged_result = generate_output_sebus(config, ctx, initial_grouping, scores);

        if (std::any_of(ctx.partial_results.begin(), ctx.partial_results.end(), [&](auto& seen) { return *seen == *output.merged_result; }))
        {
            if (*ctx.partial_results.back() == *output.merged_result)
            {
                log_info("dataflow", "result does not improve anymore");
            }
            else
            {
                log_info("dataflow", "found a cycle");
            }
            if (ctx.phase == last_phase)
            {
                output.is_final_result = true;
                log_info("dataflow", "voting done");
            }
            else
            {
                ctx.partial_results.clear();
                ctx.phase++;
                log_info("dataflow", "starting next phase");
            }
        }

        ctx.partial_results.push_back(output.merged_result);

        return output;
        */

                output.merged_result = generate_output(config, initial_grouping, scores);

                if (std::any_of(ctx.partial_results.begin(), ctx.partial_results.end(), [&](auto& seen) { return *seen == *output.merged_result; }))
                {
                    if (*ctx.partial_results.back() == *output.merged_result)
                    {
                        log_info("dataflow", "result does not improve anymore");
                    }
                    else
                    {
                        log_info("dataflow", "found a cycle");
                    }

                    output.is_final_result = true;
                    log_info("dataflow", "voting done");
                }

                ctx.partial_results.push_back(output.merged_result);

                return output;
            }
        }    // namespace evaluation
    }        // namespace dataflow
}    // namespace hal