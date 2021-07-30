#include "dataflow_analysis/processing/processing.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/processing/configuration.h"
#include "dataflow_analysis/processing/context.h"
#include "dataflow_analysis/processing/pass_collection.h"
#include "dataflow_analysis/processing/result.h"
#include "dataflow_analysis/utils/progress_printer.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <iomanip>
#include <iostream>
#include <thread>

namespace hal
{
    namespace dataflow
    {
        namespace processing
        {
            namespace
            {
                progress_printer m_progress_printer;

                void process_pass_configuration(const Configuration& config, Context& ctx)
                {
                    u32 num_passes = ctx.current_passes.size();

                    while (true)
                    {
                        u32 start_id;
                        u32 end_id;

                        // fetch next work package
                        {
                            std::lock_guard<std::mutex> guard(ctx.progress_mutex);

                            if (ctx.done)
                            {
                                break;
                            }

                            // adaptive workload:
                            // every thread gets between 1 and 20 passes, depending on the number of remaining passes
                            // this improves cpu utilization and reduces number of mutex locks
                            start_id             = ctx.pass_counter;
                            u32 remaining_passes = num_passes - start_id;

                            u32 work = 20;

                            if (remaining_passes < config.num_threads * work / 2)
                            {
                                work = std::max(1u, remaining_passes / config.num_threads);
                            }

                            end_id = std::min(start_id + work, (u32)num_passes);

                            ctx.pass_counter = end_id;
                            if (ctx.pass_counter >= num_passes)
                            {
                                ctx.done = true;
                            }
                        }

                        for (u32 current_id = start_id; current_id < end_id; ++current_id)
                        {
                            const auto& [current_state, current_pass] = ctx.current_passes[current_id];

                            if (auto it = ctx.pass_outcome.find({current_state, current_pass.id}); it != ctx.pass_outcome.end())
                            {
                                // early exit, outcome is already known
                                std::lock_guard guard(ctx.result_mutex);
                                ctx.new_recurring_results.emplace_back(current_state, current_pass.id, it->second);
                                ctx.finished_passes++;
                                m_progress_printer.print_progress((float)ctx.finished_passes / ctx.current_passes.size(),
                                                                  std::to_string(ctx.finished_passes) + "\\" + std::to_string(ctx.current_passes.size()) + " ("
                                                                      + std::to_string(ctx.new_unique_groupings.size()) + " new results)");
                                continue;
                            }

                            // process work
                            auto new_state = current_pass.function(current_state);

                            // aggregate result
                            std::shared_ptr<Grouping> duplicate = nullptr;
                            for (const auto& other : ctx.result.unique_groupings)
                            {
                                if (*new_state == *other)
                                {
                                    duplicate = other;
                                    break;
                                }
                            }
                            {
                                std::lock_guard guard(ctx.result_mutex);
                                if (duplicate == nullptr)
                                {
                                    ctx.new_unique_groupings.emplace_back(current_state, current_pass.id, new_state);
                                }
                                else
                                {
                                    ctx.new_recurring_results.emplace_back(current_state, current_pass.id, duplicate);
                                }

                                ctx.finished_passes++;
                                m_progress_printer.print_progress((float)ctx.finished_passes / ctx.current_passes.size(),
                                                                  std::to_string(ctx.finished_passes) + "\\" + std::to_string(ctx.current_passes.size()) + " ("
                                                                      + std::to_string(ctx.new_unique_groupings.size()) + " new results)");
                            }
                        }
                    }
                }

                std::vector<std::pair<std::shared_ptr<Grouping>, PassConfiguration>> generate_pass_combinations(Context& ctx, const std::shared_ptr<Grouping>& initial_grouping)
                {
                    // create current layer of pass combinations;
                    std::vector<std::pair<std::shared_ptr<Grouping>, PassConfiguration>> output;

                    if (initial_grouping != nullptr)
                    {
                        for (const auto& pass : pass_collection::get_passes(ctx.result.pass_combinations_leading_to_grouping[initial_grouping]))
                        {
                            output.emplace_back(initial_grouping, pass);
                        }
                    }
                    else
                    {
                        for (const auto& state : ctx.result.unique_groupings)
                        {
                            for (const auto& pass : pass_collection::get_passes(ctx.result.pass_combinations_leading_to_grouping[state]))
                            {
                                output.emplace_back(state, pass);
                            }
                        }
                    }

                    return output;
                }

            }    // namespace

            Result run(const Configuration& config, const std::shared_ptr<Grouping>& initial_grouping)
            {
                log_info("dataflow", "starting pipeline with {} threads", config.num_threads);

                Context ctx;
                ctx.num_iterations = 0;
                ctx.phase          = 0;
                ctx.end_reached    = false;

                for (u32 layer = 0; layer < config.pass_layers; layer++)
                {
                    log_info("dataflow", "start processing layer {}", layer);
                    auto begin_time = std::chrono::high_resolution_clock::now();

                    // get all pass combinations of layer
                    ctx.current_passes = generate_pass_combinations(ctx, (layer == 0) ? initial_grouping : nullptr);

                    // preparations
                    ctx.done            = false;
                    ctx.pass_counter    = 0;
                    ctx.finished_passes = 0;

                    m_progress_printer = progress_printer(30);

                    // spawn threads
                    std::vector<std::thread> workers;
                    for (u32 t = 0; t < config.num_threads - 1; ++t)
                    {
                        workers.emplace_back([&]() { process_pass_configuration(config, ctx); });
                    }

                    process_pass_configuration(config, ctx);

                    // wait for threads to finish
                    for (auto& worker : workers)
                    {
                        worker.join();
                    }

                    m_progress_printer.clear();

                    log_info("dataflow", "  finished in {:3.2f}s, processed {} passes, filtering results...", seconds_since(begin_time), ctx.finished_passes, ctx.new_unique_groupings.size());

                    auto all_new_results = ctx.new_recurring_results;

                    begin_time = std::chrono::high_resolution_clock::now();

                    // filter same results of different threads
                    u32 num_unique_filtered = 0;
                    std::vector<bool> do_not_consider(ctx.new_unique_groupings.size(), false);
                    for (u32 i = 0; i < ctx.new_unique_groupings.size(); ++i)
                    {
                        if (do_not_consider[i])
                        {
                            continue;
                        }
                        const auto& [start_state_i, pass_i, new_state_i] = ctx.new_unique_groupings[i];
                        for (u32 j = i + 1; j < ctx.new_unique_groupings.size(); ++j)
                        {
                            if (do_not_consider[j])
                            {
                                continue;
                            }
                            const auto& [start_state_j, pass_j, new_state_j] = ctx.new_unique_groupings[j];

                            // j is a duplicate of i
                            if (*new_state_i == *new_state_j)
                            {
                                do_not_consider[j] = true;
                                all_new_results.emplace_back(start_state_j, pass_j, new_state_i);
                            }
                        }
                        ctx.result.unique_groupings.push_back(new_state_i);
                        all_new_results.push_back(ctx.new_unique_groupings[i]);
                        num_unique_filtered++;
                    }
                    log_info("dataflow", "  filtered results in {:3.2f}s, got {} new unique results", seconds_since(begin_time), num_unique_filtered);

                    begin_time = std::chrono::high_resolution_clock::now();
                    ctx.new_recurring_results.clear();
                    ctx.new_unique_groupings.clear();

                    // fill results: compute path by appending pass id to the path of the prev round
                    for (const auto& [start_state, pass, new_state] : all_new_results)
                    {
                        ctx.pass_outcome[{start_state, pass}] = new_state;

                        const auto& start_pass_combinations = ctx.result.pass_combinations_leading_to_grouping[start_state];
                        auto& new_pass_combinations         = ctx.result.pass_combinations_leading_to_grouping[new_state];
                        if (start_pass_combinations.empty())
                        {
                            std::vector<pass_id> path{pass};
                            new_pass_combinations.push_back(path);
                            ctx.result.groupings[path] = new_state;
                        }
                        else
                        {
                            std::vector<std::vector<pass_id>> new_paths;    // temporary memory to avoid modification while looping
                            new_paths.reserve(start_pass_combinations.size());
                            for (const auto& path : start_pass_combinations)
                            {
                                if (path.size() != layer)
                                {
                                    continue;
                                }
                                std::vector<pass_id> new_path(path);
                                new_path.push_back(pass);
                                new_paths.push_back(new_path);
                                ctx.result.groupings[new_path] = new_state;
                            }
                            new_pass_combinations.insert(new_pass_combinations.end(), new_paths.begin(), new_paths.end());
                        }
                    }
                    log_info("dataflow", "  total: {} unique states", ctx.result.unique_groupings.size());
                }
                
                return ctx.result;
            }

        }    // namespace processing
    }        // namespace dataflow
}    // namespace hal