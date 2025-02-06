#include "module_identification/api/module_identification.h"

#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/module.h"
#include "module_identification/api/configuration.h"
#include "module_identification/api/result.h"
#include "module_identification/architectures/lattice_ice40.h"
#include "module_identification/architectures/xilinx_unisim.h"
#include "module_identification/candidates/base_candidate.h"
#include "module_identification/candidates/functional_candidate.h"
#include "module_identification/candidates/structural_candidate.h"
#include "module_identification/processing/post_processing.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/utils/statistics.h"
#include "module_identification/utils/utils.h"

#include <deque>
#include <mutex>

// #define PRINT_THREAD_INFO

namespace hal
{
    namespace module_identification
    {
        hal::Result<std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>>> generate_structural_candidates(const Netlist* nl,
                                                                                                                                                              const Configuration& config)
        {
            auto gl_name = nl->get_gate_library()->get_name();
            std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> candidates;

            if (gl_name == "ICE40ULTRA" || gl_name == "ICE40ULTRA_iPhone" || gl_name == "ICE40ULTRA_WITH_HAL_TYPES")
            {
                log_info("module_identification", "generate arithmetic structures for {}", gl_name);
                candidates = lattice_ice40::generate_structural_candidates(nl);
            }
            else if (gl_name == "XILINX_UNISIM_WITH_HAL_TYPES" || gl_name == "XILINX_UNISIM")
            {
                log_info("module_identification", "generate arithmetic structures for {}", gl_name);
                candidates = xilinx_unisim::generate_structural_candidates(nl);
            }
            else
            {
                return ERR("arithmetic structure generation not available for gate_lib: " + gl_name);
            }

            // filter out already classified modules or blocked base candidates
            std::vector<u32> filtered_indices;
            std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> filtered_candidates;

            for (u32 idx = 0; idx < candidates.size(); idx++)
            {
                auto& [base_candidate, structural_candidates] = candidates.at(idx);
                bool filtered_out                             = false;

                for (const auto& blocked_base_candidate : config.m_blocked_base_candidates)
                {
                    const std::set<Gate*> base_candidate_set = {base_candidate->m_gates.begin(), base_candidate->m_gates.end()};

                    if (base_candidate_set == blocked_base_candidate)
                    {
                        filtered_out = true;
                        break;
                    }
                }

                for (const auto& already_classified_candidates : config.m_already_classified_candidates)
                {
                    const std::set<Gate*> already_classified_candidates_set = {already_classified_candidates.begin(), already_classified_candidates.end()};

                    for (const auto& g : base_candidate->m_gates)
                    {
                        if (already_classified_candidates_set.find(g) != already_classified_candidates_set.end())
                        {
                            filtered_out = true;
                            break;
                        }
                    }

                    if (filtered_out)
                    {
                        break;
                    }
                }

                if (!filtered_out)
                {
                    filtered_indices.push_back(idx);
                }
            }

            log_info("module_identification", "Filtered out already classified candidates. Left with {} / {} base candidates", filtered_indices.size(), candidates.size());

            for (const auto& idx : filtered_indices)
            {
                auto& [base_candidate, structural_candidates] = candidates.at(idx);
                filtered_candidates.push_back({std::move(base_candidate), std::move(structural_candidates)});
            }

            for (const auto& [bc, sc_vec] : filtered_candidates)
            {
                for (const auto& sc : sc_vec)
                {
                    // TODO: this is an ugly fix
                    sc->ctx.m_gates = sc->m_gates;
                }
            }

            return OK(std::move(filtered_candidates));
        }

        hal::Result<std::vector<FunctionalCandidate>> generate_functional_candidates(StructuralCandidate* sc, const Configuration& config, Statistics& stats)
        {
            std::vector<FunctionalCandidate> result;

            Netlist* nl      = sc->m_gates.front()->get_netlist();
            auto output_nets = get_output_nets(sc->m_gates, false);

            // TODO make this a config parameter
            const u32 max_inputs                  = 130;
            bool found_oversized_boolean_function = false;

            for (const auto& o_net : output_nets)
            {
                const auto input_nets_res = SubgraphNetlistDecorator(*nl).get_subgraph_function_inputs(sc->m_gates, o_net);
                if (input_nets_res.is_error())
                {
                    return ERR_APPEND(input_nets_res.get_error(),
                                      "cannot generate functional candidates for " + sc->m_gates.front()->get_name() + ": failed to get subgrapg inputs for net " + o_net->get_name() + " with ID "
                                          + std::to_string(o_net->get_id()));
                }
                const auto input_net_count = input_nets_res.get().size();

                if (input_net_count > max_inputs)
                {
                    found_oversized_boolean_function = true;
                    break;
                }
            }

            if (found_oversized_boolean_function)
            {
                return OK({});
            }

            // TODO this should not happen, but exists as a safety meassure
            if (sc->m_gates != sc->ctx.m_gates)
            {
                return ERR("cannot generate functional candidates: candiate and context gates are not identical!");
            }

            const auto res = sc->ctx.populate_boolean_function_cache(output_nets);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "cannot generate functional candidates: failed to populate Boolean context cache");
            }

            for (const auto& type : config.m_types_to_check)
            {
                const auto it = std::find(all_checkable_candidate_types.begin(), all_checkable_candidate_types.end(), type);

                if (it == all_checkable_candidate_types.end())
                {
                    log_error("module_identification", "no candidate generation available for type {}", enum_to_string(type));
                    continue;
                }

                const auto start_pre_processing = std::chrono::steady_clock::now();
                auto new_candidates_res         = FunctionalCandidate::create_candidates(sc, config.m_max_control_signals, sc->ctx, type, config.m_known_registers);
                const u64 duration              = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_pre_processing).count();

                if (new_candidates_res.is_error())
                {
                    return ERR(new_candidates_res.get_error().get());
                }
                auto new_candidates = new_candidates_res.get();

                std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, u64>>>> new_entries = {};
                new_entries["PRE_PROCESSING"][enum_to_string(type)]["CANDIDATE_CREATION"]["DURATION"] += duration;
                new_entries["PRE_PROCESSING"][enum_to_string(type)]["CANDIDATE_CREATION"]["AMOUNT"] += new_candidates.size();
                stats.add_stat(sc->base_candidate, new_entries);

                for (auto& nc : new_candidates)
                {
                    result.push_back(nc);
                }
            }

            return OK(result);
        }

        namespace
        {
            // TODO this is a mess of multi threading and needs cleaning up badly at some point
            void work(std::vector<StructuralCandidate*>& structural_candidates_work_queque,
                      std::vector<std::unique_ptr<StructuralCandidate>>& structural_candidates,
                      std::vector<std::unique_ptr<StructuralCandidate>>& done_structural_candidates,
                      std::atomic<u32>& structural_workload,
                      std::deque<std::pair<std::unique_ptr<StructuralCandidate>, std::vector<FunctionalCandidate>>>& functional_candidates,
                      std::map<BaseCandidate*, std::vector<VerifiedCandidate>>& verified_candidates,
                      const Configuration& config,
                      Statistics& stats,
                      std::vector<std::string>& status,
                      std::vector<std::string>& candidate_info,
                      std::mutex& thread_sync,
                      const u32 thread_idx)
            {
                std::map<BaseCandidate*, std::vector<VerifiedCandidate>> verified_candidates_cache;

                const auto work_structural =
                    [&config, &structural_candidates_work_queque, &structural_candidates, &structural_workload, &functional_candidates, &stats, &status, &candidate_info, &thread_sync, &thread_idx]()
                    -> bool {
                    // check for structural OverarchingCandidate workloads
                    if (!structural_candidates_work_queque.empty())
                    {
                        // const auto sc = structural_candidates_work_queque.back();
                        structural_candidates_work_queque.pop_back();

                        auto unique_sc = std::move(structural_candidates.back());
                        structural_candidates.pop_back();

                        thread_sync.unlock();

                        candidate_info.at(thread_idx) = "S_" + unique_sc->base_candidate->m_gates.front()->get_name();

                        auto new_functional_candidates_res = generate_functional_candidates(unique_sc.get(), config, stats);
                        if (new_functional_candidates_res.is_error())
                        {
                            log_error("module_identification",
                                      "failed to generate functional candidates for carry chain {}: {}",
                                      unique_sc.get()->m_gates.front()->get_name(),
                                      new_functional_candidates_res.get_error().get());
                            return true;
                        }
                        std::vector<FunctionalCandidate> new_functional_candidates = new_functional_candidates_res.get();

                        for (const auto& fc : new_functional_candidates)
                        {
                            const auto _bfs = unique_sc.get()->ctx.get_boolean_functions(fc.m_output_nets, fc.m_control_mapping);
                        }

                        structural_workload -= 1;
                        if (!new_functional_candidates.empty())
                        {
                            thread_sync.lock();
                            status.at(thread_idx) = "LOCKING STRUCTUAL at " + std::to_string(thread_idx);
                            functional_candidates.push_back({std::move(unique_sc), new_functional_candidates});
                            thread_sync.unlock();
                        }

                        return true;
                    }

                    return false;
                };

                const auto work_functional = [&config, &functional_candidates, &done_structural_candidates, &verified_candidates_cache, &candidate_info, &thread_sync, &thread_idx, &stats]() -> bool {
                    // check for  functional OverarchingCandidate workloads
                    if (!functional_candidates.empty())
                    {
                        auto& [sc, f_candidates] = functional_candidates.front();

                        auto sc_ptr = functional_candidates.front().first.get();
                        auto bc     = sc->base_candidate;
                        auto fc     = f_candidates.back();

                        if (f_candidates.size() == 1)
                        {
                            // if this was the last functional candidate from this structural candidate pop the pair from the deque
                            done_structural_candidates.push_back(std::move(sc));
                            sc_ptr = done_structural_candidates.back().get();

                            functional_candidates.pop_front();
                        }
                        else
                        {
                            // ... otherwise just pop one candidate from the vector
                            f_candidates.pop_back();
                        }
                        thread_sync.unlock();

                        candidate_info.at(thread_idx) = "F_" + fc.m_base_gates.front()->get_name();

                        const auto output_functions_res = sc_ptr->ctx.get_boolean_functions_const(fc.m_output_nets, fc.m_control_mapping);
                        if (output_functions_res.is_error())
                        {
                            log_error("module_identification", "cannot check candidate: failed to get Boolean output functions before check.\n {}", output_functions_res.get_error().get());
                        }
                        const auto output_functions = output_functions_res.get();

                        const auto res = fc.check(output_functions, config.m_known_registers);
                        if (res.is_error())
                        {
                            log_error("module_identification",
                                      "failed to check current overaching candidate at carry chain {} of type {}:\n{}",
                                      fc.m_gates.front()->get_name(),
                                      enum_to_string(fc.m_candidate_type),
                                      res.get_error().get());
                            return true;
                        }
                        stats.add_stat(bc, fc);

                        auto vc         = res.get();
                        vc.m_base_gates = bc->m_gates;

                        if (vc.is_verified())
                        {
                            verified_candidates_cache[bc].push_back(vc);
                        }
                        return true;
                    }

                    return false;
                };

                status.at(thread_idx) = "INIT";
                while (true)
                {
                    // getting workload
                    candidate_info.at(thread_idx) = "NONE";
                    status.at(thread_idx)         = "LOCKED";
                    thread_sync.lock();

                    // all work is done, now return results
                    if ((structural_workload == 0) && functional_candidates.empty())
                    {
                        for (auto [bc, vc] : verified_candidates_cache)
                        {
                            verified_candidates.at(bc).insert(verified_candidates.at(bc).end(), vc.begin(), vc.end());
                        }
                        thread_sync.unlock();

                        status.at(thread_idx) = "FINISHED";
                        return;
                    }

                    if (config.m_multithreading_priority == MultithreadingPriority::time_priority)
                    {
                        status.at(thread_idx) = "RUNNING STRUCTURAL";
                        if (work_structural())
                        {
                            continue;
                        }

                        status.at(thread_idx) = "RUNNING FUNCTIONAL";
                        if (work_functional())
                        {
                            continue;
                        }
                    }
                    else
                    {
                        status.at(thread_idx) = "RUNNING FUNCTIONAL";
                        if (work_functional())
                        {
                            continue;
                        }

                        status.at(thread_idx) = "RUNNING STRUCTURAL";
                        if (work_structural())
                        {
                            continue;
                        }
                    }

                    thread_sync.unlock();

                    status.at(thread_idx) = "WAITING";
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
            }

            hal::Result<Result>
                execute_on_structural_candidates(std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>>& base_to_structural_candidates,
                                                 const Configuration& config)
            {
                Netlist* nl = config.m_netlist;

                // contains a list of verified candidates for each base candidate
                std::map<BaseCandidate*, std::vector<VerifiedCandidate>> verified_candidates;

                std::vector<StructuralCandidate*> structural_candidates_work_queque;
                std::vector<std::unique_ptr<StructuralCandidate>> structural_candidates;
                std::vector<std::unique_ptr<StructuralCandidate>> done_structural_candidates;
                std::deque<std::pair<std::unique_ptr<StructuralCandidate>, std::vector<FunctionalCandidate>>> functional_candidates;

                for (auto& [base_cand, struct_cands] : base_to_structural_candidates)
                {
                    // initialize verified candidate list with pointer of base candidates
                    verified_candidates.insert(std::make_pair(base_cand.get(), std::vector<VerifiedCandidate>()));

                    // create a vector with all base candidates
                    for (auto& sc : struct_cands)
                    {
                        structural_candidates_work_queque.push_back(sc.get());
                        structural_candidates.push_back(std::move(sc));
                    }
                }

                std::atomic<u32> structural_workload = structural_candidates.size();

                if (structural_candidates.empty())
                {
                    Result(nl, std::vector<std::pair<BaseCandidate, VerifiedCandidate>>());
                }

                const u32 num_threads = std::min(config.m_max_thread_count, std::thread::hardware_concurrency() - 1);

                log_info("module_identification", "running with {} threads and {} multithreading priority", num_threads, fmt::underlying(config.m_multithreading_priority));

                auto stats = Statistics();

                // create hal GND and VCC nets as they might be needed for the operand creation
                const auto gnd_res = NetlistModificationDecorator(*nl).create_gnd_net();
                if (gnd_res.is_error())
                {
                    log_error("module_identification", "failed to create GND net: {}", gnd_res.get_error().get());
                }

                const auto vcc_res = NetlistModificationDecorator(*nl).create_vcc_net();
                if (vcc_res.is_error())
                {
                    log_error("module_identification", "failed to create VCC net: {}", vcc_res.get_error().get());
                }

                // creating workloads
                log_info("module_identification", "running checks for {} possible candidates...", structural_candidates.size());
                std::vector<std::thread> workers;
                std::vector<std::string> status         = {num_threads + 1, "NOT STARTED"};
                std::vector<std::string> candidate_info = {num_threads + 1, "NONE"};

                std::mutex thread_sync;

                for (u32 i = 0; i < num_threads; ++i)
                {
                    workers.emplace_back([&, i]() {
                        return work(structural_candidates_work_queque,
                                    structural_candidates,
                                    done_structural_candidates,
                                    structural_workload,
                                    functional_candidates,
                                    verified_candidates,
                                    config,
                                    stats,
                                    status,
                                    candidate_info,
                                    thread_sync,
                                    i);
                    });
                }

#ifdef PRINT_THREAD_INFO
                while ((structural_workload != 0) || !functional_candidates.empty())
                {
                    std::cout << "WORKLOAD: " << structural_workload << std::endl;

                    std::map<std::string, u32> status_collection;
                    for (u32 i = 0; i < num_threads; i++)
                    {
                        status_collection[status.at(i)] += 1;
                    }

                    std::cout << "Thread status: " << std::endl;
                    for (const auto& [sn, sc] : status_collection)
                    {
                        std::cout << "\t" << sn << ": " << sc << std::endl;
                    }

                    std::map<std::string, u32> candidate_collection;
                    for (u32 i = 0; i < num_threads; i++)
                    {
                        candidate_collection[candidate_info.at(i)] += 1;
                    }

                    if (candidate_collection.size() < 10)
                    {
                        std::cout << "Candidate Info: " << std::endl;
                        for (const auto& [sn, sc] : candidate_collection)
                        {
                            std::cout << "\t" << sn << ": " << sc << std::endl;
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
#endif

                // wait for threads to finish
                for (auto& worker : workers)
                {
                    worker.join();
                }

                // if (config.s_progress_indicator_function)
                // {
                //     // TODO use string formatter to make this pretty
                //     config.s_progress_indicator_function(100, "module identification finished\ndoing postprocessing");
                // }

                // post processing
                std::map<std::string, u32> type_counter;
                std::vector<std::vector<hal::Gate*>> known_registers = config.m_known_registers;
                std::vector<std::pair<BaseCandidate, VerifiedCandidate>> result_input;

                log_info("module_identification", "Done with module identification, now doing post processing");

                const auto start_post_processing = std::chrono::steady_clock::now();
                for (auto& [bc, vc] : verified_candidates)
                {
                    auto post_processing_result = post_processing(vc, nl, known_registers);

                    if (!post_processing_result.is_verified())
                    {
                        post_processing_result = VerifiedCandidate({}, {}, {}, {}, {}, {}, bc->m_gates, {}, {});
                    }

                    result_input.push_back(std::make_pair(*bc, post_processing_result));
                }
                const auto duration_post_processing = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_post_processing);
                log_info("module_identification", "Post processing took {} ms", duration_post_processing.count());

                return OK(Result(nl, result_input, stats.to_json()));
            }
        }    // namespace

        hal::Result<Result> execute(const Configuration& config)
        {
            const Netlist* nl = config.m_netlist;

            log_info("module_identification",
                     "Executing on netlist {} / {} / {}  and gatelib {}",
                     nl->get_top_module()->get_name(),
                     nl->get_design_name(),
                     nl->get_device_name(),
                     nl->get_gate_library()->get_name());

            auto res = generate_structural_candidates(nl, config);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "cannot execute plugin: failed structural candidate generation");
            }

            auto base_to_structural_candidates = res.get();
            return execute_on_structural_candidates(base_to_structural_candidates, config);
        }

        hal::Result<Result> execute_on_gates(const std::vector<Gate*>& gates, const Configuration& config)
        {
            if (gates.empty())
            {
                return ERR("cannot execute plugin: provided gates are empty");
            }

            std::unique_ptr<BaseCandidate> bc = std::make_unique<BaseCandidate>(gates);

            std::vector<std::unique_ptr<StructuralCandidate>> structural_vector;
            structural_vector.emplace_back(std::make_unique<StructuralCandidate>(bc.get(), gates));
            std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> base_to_structural_candidates;
            base_to_structural_candidates.emplace_back(std::make_pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>(std::move(bc), std::move(structural_vector)));

            return execute_on_structural_candidates(base_to_structural_candidates, config);
        }
    }    // namespace module_identification
}    // namespace hal
