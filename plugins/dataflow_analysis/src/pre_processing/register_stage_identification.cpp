#include "dataflow_analysis/pre_processing/register_stage_identification.h"

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/utils/parallel_for_each.h"
#include "dataflow_analysis/utils/progress_printer.h"
#include "dataflow_analysis/utils/utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <algorithm>
#include <chrono>
#include <list>
#include <queue>
#include <random>

namespace hal
{
    namespace dataflow
    {
        namespace pre_processing
        {
            void identify_register_stages(NetlistAbstraction& netlist_abstr)
            {
                measure_block_time("pre_processing_pass 'identify_register_stages'");

                struct stage_context
                {
                    std::string name;
                    const std::unordered_map<u32, std::unordered_set<u32>>& connections;
                    std::vector<std::vector<u32>> stages;
                };

                std::vector<stage_context> directional_stages = {{"forward", netlist_abstr.gate_to_successors, {}}, {"backward", netlist_abstr.gate_to_predecessors, {}}};

                for (auto& ctx : directional_stages)
                {
                    log_info("dataflow", "directional register stages: {}", ctx.name);

                    std::unordered_set<u32> unassigned_gates;
                    for (const auto& g : netlist_abstr.all_sequential_gates)
                    {
                        unassigned_gates.insert(g->get_id());
                    }

                    {
                        measure_block_time(ctx.name + " analysis");
                        progress_printer progress_bar;
                        float cnt = 0;
                        std::unordered_map<u32, u32> stage_index_of_gate;

                        for (const auto& sequential_gate : netlist_abstr.all_sequential_gates)
                        {
                            cnt++;
                            progress_bar.print_progress(cnt / netlist_abstr.all_sequential_gates.size());

                            auto current = sequential_gate->get_id();

                            std::unordered_set<u32> next_stages;

                            if (!ctx.connections.at(current).empty())
                            {
                                for (auto suc : ctx.connections.at(current))
                                {
                                    if (auto it = stage_index_of_gate.find(suc); it != stage_index_of_gate.end())
                                    {
                                        next_stages.insert(it->second);
                                    }
                                }

                                // no stages
                                if (next_stages.empty())
                                {
                                    ctx.stages.emplace_back(ctx.connections.at(current).begin(), ctx.connections.at(current).end());
                                    auto new_stage_id = ctx.stages.size() - 1;
                                    for (auto g : ctx.connections.at(current))
                                    {
                                        stage_index_of_gate[g] = new_stage_id;
                                    }
                                }

                                // multiple stages? merge!
                                if (next_stages.size() > 1)
                                {
                                    auto it                = next_stages.begin();
                                    auto merge_stage_index = *it;
                                    it++;
                                    for (; it != next_stages.end(); it = next_stages.erase(it))
                                    {
                                        auto& stage_to_merge = ctx.stages[*it];
                                        for (auto g : stage_to_merge)
                                        {
                                            stage_index_of_gate[g] = merge_stage_index;
                                        }
                                        ctx.stages[merge_stage_index].insert(ctx.stages[merge_stage_index].end(), stage_to_merge.begin(), stage_to_merge.end());
                                        stage_to_merge.clear();
                                    }
                                }

                                // only one stage? directly or after merging
                                if (next_stages.size() == 1)
                                {
                                    auto stage_index      = *(next_stages.begin());
                                    auto& connected_gates = ctx.connections.at(current);
                                    ctx.stages[stage_index].insert(ctx.stages[stage_index].end(), connected_gates.begin(), connected_gates.end());
                                    for (auto g : connected_gates)
                                    {
                                        stage_index_of_gate[g] = stage_index;
                                    }
                                }

                                for (auto g : ctx.connections.at(current))
                                {
                                    if (auto it = unassigned_gates.find(g); it != unassigned_gates.end())
                                    {
                                        unassigned_gates.erase(it);
                                    }
                                }
                            }
                        }
                        progress_bar.clear();
                    }

                    {
                        ctx.stages.erase(std::remove_if(ctx.stages.begin(), ctx.stages.end(), [](auto& stage) { return stage.empty(); }), ctx.stages.end());

                        measure_block_time("splitting stages");

                        for (u32 i = 0; i < ctx.stages.size(); ++i)
                        {
                            std::sort(ctx.stages[i].begin(), ctx.stages[i].end());
                            ctx.stages[i].erase(std::unique(ctx.stages[i].begin(), ctx.stages[i].end()), ctx.stages[i].end());
                        }

                        progress_printer progress_bar;
                        float cnt = 0;

                        u32 stages_to_split = ctx.stages.size();
                        for (u32 i = 0; i < stages_to_split; ++i)
                        {
                            ++cnt;
                            progress_bar.print_progress(cnt / stages_to_split);

                            std::unordered_map<u32, std::vector<u32>> move_out_reasons;
                            for (auto g : ctx.stages[i])
                            {
                                for (auto next : ctx.connections.at(g))
                                {
                                    if (next != g)
                                    {
                                        if (std::binary_search(ctx.stages[i].begin(), ctx.stages[i].end(), next) && ctx.connections.at(next).find(g) == ctx.connections.at(next).end())
                                        {
                                            move_out_reasons[next].push_back(g);
                                        }
                                    }
                                }
                            }

                            for (auto it = move_out_reasons.begin(); it != move_out_reasons.end(); ++it)
                            {
                                std::sort(it->second.begin(), it->second.end());
                                it->second.erase(std::unique(it->second.begin(), it->second.end()), it->second.end());
                            }

                            auto remaining       = ctx.stages[i];
                            bool first_iteration = true;
                            while (!remaining.empty())
                            {
                                std::vector<u32> keep;
                                for (auto g : remaining)
                                {
                                    if (move_out_reasons[g].empty())
                                    {
                                        keep.push_back(g);
                                    }
                                }

                                if (keep.empty() || keep.size() == remaining.size())
                                {
                                    if (!first_iteration)
                                    {
                                        ctx.stages.push_back(remaining);
                                    }
                                    break;
                                }

                                std::sort(keep.begin(), keep.end());
                                if (first_iteration)
                                {
                                    ctx.stages[i] = keep;
                                }
                                else
                                {
                                    ctx.stages.push_back(keep);
                                }

                                {
                                    std::vector<u32> diff;
                                    std::set_difference(remaining.begin(), remaining.end(), keep.begin(), keep.end(), std::back_inserter(diff));
                                    remaining = std::move(diff);
                                }

                                for (auto r : remaining)
                                {
                                    auto& reasons = move_out_reasons[r];
                                    std::vector<u32> diff;
                                    std::set_difference(reasons.begin(), reasons.end(), keep.begin(), keep.end(), std::back_inserter(diff));
                                    reasons = std::move(diff);
                                }
                                first_iteration = false;
                            }
                        }

                        progress_bar.clear();
                    }

                    for (const auto& g : unassigned_gates)
                    {
                        ctx.stages.push_back({g});
                    }
                }

                {
                    log_info("dataflow", "merging directional stages...");
                    measure_block_time("merging directional stages");

                    progress_printer progress_bar;

                    for (u32 i = 0; i < 2; ++i)
                    {
                        std::sort(directional_stages[i].stages.begin(), directional_stages[i].stages.end(), [](auto& a, auto& b) { return a.size() < b.size(); });
                    }

                    float cnt = 0;
                    for (u32 i = 0; i < 2; ++i)
                    {
                        for (const auto& stage : directional_stages[i].stages)
                        {
                            cnt++;
                            progress_bar.print_progress(cnt / (directional_stages[0].stages.size() + directional_stages[1].stages.size()));
                            for (auto it = directional_stages[1 - i].stages.begin(); it != directional_stages[1 - i].stages.end();)
                            {
                                auto& other_stage = *it;
                                if (stage.size() < other_stage.size())
                                {
                                    break;
                                }

                                if (std::includes(stage.begin(), stage.end(), other_stage.begin(), other_stage.end()))
                                {
                                    it = directional_stages[1 - i].stages.erase(it);
                                }
                                else
                                {
                                    ++it;
                                }
                            }
                        }
                    }

                    progress_bar.clear();
                }

                std::vector<std::vector<u32>> final_stages;

                {
                    log_info("dataflow", "sorting gates into final stages...");
                    measure_block_time("sorting gates into final stages");

                    final_stages.reserve(directional_stages[0].stages.size() + directional_stages[1].stages.size());
                    for (u32 i = 0; i < 2; ++i)
                    {
                        final_stages.insert(final_stages.end(), directional_stages[i].stages.begin(), directional_stages[i].stages.end());
                    }

                    netlist_abstr.gate_to_register_stages.clear();
                    for (u32 s = 0; s < final_stages.size(); ++s)
                    {
                        for (auto g : final_stages[s])
                        {
                            netlist_abstr.gate_to_register_stages[g].insert(s);
                        }
                    }
                }

                {
                    log_info("dataflow", "spreading multi-stages...");
                    measure_block_time("spreading multi-stages");

                    // step 1: precompute all stage indices that are in one multi-stage

                    progress_printer progress_bar;
                    float cnt = 0;
                    std::vector<std::unordered_set<u32>> spread_stages;
                    std::set<std::vector<u32>> seen;
                    for (auto& [g, stages] : netlist_abstr.gate_to_register_stages)
                    {
                        progress_bar.print_progress(++cnt / netlist_abstr.gate_to_register_stages.size());

                        if (stages.size() > 1)
                        {
                            // collect current multi stage in vector for fast iteration + low memory
                            auto stages_vec = std::vector<u32>(stages.begin(), stages.end());

                            // seen this multi stage before? skip!
                            if (seen.find(stages_vec) != seen.end())
                            {
                                continue;
                            }
                            seen.insert(stages_vec);

                            // check if there is an existing multi stage to extend
                            auto it = std::find_if(spread_stages.begin(), spread_stages.end(), [&](auto& group) {
                                return std::any_of(stages_vec.begin(), stages_vec.end(), [&](auto id) { return group.find(id) != group.end(); });
                            });

                            if (it != spread_stages.end())
                            {
                                // extend multi stage
                                it->insert(stages.begin(), stages.end());
                            }
                            else
                            {
                                // register new multi stage
                                spread_stages.emplace_back(stages.begin(), stages.end());
                            }
                        }
                    }

                    // step 2: set the stages of all affected gates to the respective multi stage

                    progress_bar.clear();
                    progress_bar.reset();

                    cnt = 0;
                    for (const auto& combine : spread_stages)
                    {
                        progress_bar.print_progress(++cnt / netlist_abstr.gate_to_register_stages.size());

                        for (auto stage_id : combine)
                        {
                            for (auto gate_id : final_stages[stage_id])
                            {
                                netlist_abstr.gate_to_register_stages[gate_id] = combine;
                            }
                        }
                    }
                    progress_bar.clear();
                }

                log_info("dataflow", "found {} stages", final_stages.size());
            }

        }    // namespace pre_processing
    }        // namespace dataflow
}    // namespace hal