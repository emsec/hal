#include "dataflow_analysis/pre_processing/pre_processing.h"

#include "dataflow_analysis/api/configuration.h"
#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/pre_processing/register_stage_identification.h"
#include "dataflow_analysis/utils/parallel_for_each.h"
#include "dataflow_analysis/utils/progress_printer.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/utilities/log.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <queue>

namespace hal
{
    namespace dataflow
    {
        namespace pre_processing
        {
            namespace
            {
                void identify_all_target_gates(const dataflow::Configuration& config, NetlistAbstraction& netlist_abstr)
                {
                    log_info("dataflow", "identifying target gates");
                    netlist_abstr.target_gates = netlist_abstr.nl->get_gates([config](auto g) { return config.gate_types.find(g->get_type()) != config.gate_types.end(); });
                    std::sort(netlist_abstr.target_gates.begin(), netlist_abstr.target_gates.end(), [](const Gate* g1, const Gate* g2) { return g1->get_id() < g2->get_id(); });
                    log_info("dataflow", "  #gates: {}", netlist_abstr.nl->get_gates().size());
                    log_info("dataflow", "  #target gates: {}", netlist_abstr.target_gates.size());
                }

                void identify_all_control_signals(const dataflow::Configuration& config, NetlistAbstraction& netlist_abstr)
                {
                    auto begin_time = std::chrono::high_resolution_clock::now();
                    log_info("dataflow", "identifying control signals");
                    for (auto sg : netlist_abstr.target_gates)
                    {
                        std::vector<u32> fingerprint;
                        auto id = sg->get_id();
                        sg->get_name();

                        for (auto type : config.control_pin_types)
                        {
                            for (auto net :
                                 netlist_utils::get_nets_at_pins(sg, sg->get_type()->get_pins([type](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == type; })))
                            {
                                netlist_abstr.gate_to_control_signals[id][type].insert(net->get_id());
                                fingerprint.push_back(net->get_id());
                            }
                        }

                        netlist_abstr.gate_to_fingerprint[id] = fingerprint;
                    }

                    log_info("dataflow", "  done after {:3.2f}s", seconds_since(begin_time));
                }

                void identify_known_groups(const dataflow::Configuration& config, NetlistAbstraction& netlist_abstr, std::vector<std::vector<Gate*>>& known_target_groups)
                {
                    // for known gates, only check if they all match the target gate types; discard groups that do not
                    for (const auto& gates : config.known_gate_groups)
                    {
                        if (std::all_of(
                                gates.begin(), gates.end(), [&netlist_abstr](const Gate* g) { return netlist_abstr.gate_to_fingerprint.find(g->get_id()) != netlist_abstr.gate_to_fingerprint.end(); }))
                        {
                            known_target_groups.push_back(gates);
                        }
                        else
                        {
                            log_warning("dataflow",
                                        "known group containing gate '{}' with ID {} contains gates that are not of the target gate type, known group will be ignored...",
                                        gates.front()->get_name(),
                                        gates.front()->get_id());
                        }
                    }
                }

                /* get all successor/predecessor FFs of all FFs */
                void identify_successors_predecessors(const dataflow::Configuration& config, NetlistAbstraction& netlist_abstr)
                {
                    log_info("dataflow", "identifying successors and predecessors of sequential gates...");
                    measure_block_time("identifying successors and predecessors of sequential gates") ProgressPrinter progress_bar;
                    float cnt = 0;

                    // cache map of nets to group indices of known net groups
                    std::unordered_map<const Net*, u32> net_to_group_index;
                    for (u32 i = 0; i < config.known_net_groups.size(); i++)
                    {
                        const auto& net_group = config.known_net_groups.at(i);
                        if (net_group.size() < config.min_group_size)
                        {
                            continue;
                        }

                        for (const auto* net : net_group)
                        {
                            net_to_group_index[net] = i;
                        }
                    }

                    // find successors
                    std::unordered_map<const Net*, std::pair<std::unordered_set<Gate*>, std::unordered_set<u32>>> suc_cache;
                    std::unordered_map<const Net*, std::unordered_set<u32>> pred_cache;
                    for (const auto& gate : netlist_abstr.target_gates)
                    {
                        cnt++;
                        progress_bar.print_progress(cnt / netlist_abstr.target_gates.size());

                        // create sets even if there are no successors
                        if (netlist_abstr.gate_to_successors.find(gate->get_id()) == netlist_abstr.gate_to_successors.end())
                        {
                            netlist_abstr.gate_to_successors[gate->get_id()] = std::unordered_set<u32>();
                        }
                        if (netlist_abstr.gate_to_predecessors.find(gate->get_id()) == netlist_abstr.gate_to_predecessors.end())
                        {
                            netlist_abstr.gate_to_predecessors[gate->get_id()] = std::unordered_set<u32>();
                        }
                        if (netlist_abstr.gate_to_known_successor_groups.find(gate->get_id()) == netlist_abstr.gate_to_known_successor_groups.end())
                        {
                            netlist_abstr.gate_to_known_successor_groups[gate->get_id()] = std::unordered_set<u32>();
                        }
                        if (netlist_abstr.gate_to_known_predecessor_groups.find(gate->get_id()) == netlist_abstr.gate_to_known_predecessor_groups.end())
                        {
                            netlist_abstr.gate_to_known_predecessor_groups[gate->get_id()] = std::unordered_set<u32>();
                        }

                        const auto& start_fan_out_nets = gate->get_fan_out_nets();
                        std::vector<const Net*> stack(start_fan_out_nets.cbegin(), start_fan_out_nets.cend());    // init stack with fan-out of start gate
                        std::unordered_set<const Net*> visited;
                        std::vector<const Net*> previous;                                                         // will keep track of all predecessor nets of the current net
                        while (!stack.empty())
                        {
                            const Net* current = stack.back();    // do not pop last item yet

                            if (!previous.empty() && current == previous.back())
                            {
                                // will execute when coming back to a net along the path of predecessor nets (i.e., after finding a target gate or when unable to propagate further)
                                stack.pop_back();
                                previous.pop_back();
                                continue;
                            }

                            visited.insert(current);

                            if (const auto suc_cache_it = suc_cache.find(current); suc_cache_it != suc_cache.end())
                            {
                                auto& suc_cache_current           = std::get<1>(*suc_cache_it);
                                const auto& suc_cached_gates      = std::get<0>(suc_cache_current);
                                const auto& suc_cached_net_groups = std::get<1>(suc_cache_current);

                                // add cached target gates and known successor net groups to suc_cache of all predecessor nets
                                for (const auto* n : previous)
                                {
                                    auto& suc_cache_n = suc_cache[n];
                                    std::get<0>(suc_cache_n).insert(suc_cached_gates.cbegin(), suc_cached_gates.cend());
                                    std::get<1>(suc_cache_n).insert(suc_cached_net_groups.cbegin(), suc_cached_net_groups.cend());
                                }

                                // add cached net groups to known successor net groups of current gate
                                netlist_abstr.gate_to_known_successor_groups[gate->get_id()].insert(suc_cached_net_groups.cbegin(), suc_cached_net_groups.cend());

                                stack.pop_back();
                            }
                            else
                            {
                                if (const auto group_it = net_to_group_index.find(current); group_it != net_to_group_index.end())
                                {
                                    for (const auto* n : previous)
                                    {
                                        std::get<1>(suc_cache[n]).insert(group_it->second);
                                    }
                                    netlist_abstr.gate_to_known_successor_groups[gate->get_id()].insert(group_it->second);
                                }

                                bool added = false;
                                for (const auto* ep : current->get_destinations())
                                {
                                    auto* g = ep->get_gate();
                                    if (config.gate_types.find(g->get_type()) != config.gate_types.end())
                                    {
                                        // if target gate found, add to suc_cache of all nets along the predecessor path
                                        auto& suc_cache_current = suc_cache[current];
                                        std::get<0>(suc_cache_current).insert(g);
                                        for (const auto* n : previous)
                                        {
                                            std::get<0>(suc_cache[n]).insert(g);
                                        }
                                    }
                                    else
                                    {
                                        // propagate further by adding successors to stack
                                        for (const auto* n : g->get_fan_out_nets())
                                        {
                                            if (visited.find(n) == visited.end())
                                            {
                                                stack.push_back(n);
                                                added = true;
                                            }
                                        }
                                    }
                                }

                                if (added)
                                {
                                    // keep track of previous net whenever propagating further
                                    previous.push_back(current);
                                }
                                else
                                {
                                    // if no change to stack, go back
                                    stack.pop_back();
                                }
                            }
                        }

                        const auto& start_fan_in_nets = gate->get_fan_in_nets();
                        stack                         = std::vector<const Net*>(start_fan_in_nets.begin(), start_fan_in_nets.end());
                        visited.clear();
                        previous.clear();
                        while (!stack.empty())
                        {
                            const Net* current = stack.back();    // do not pop last item yet

                            if (!previous.empty() && current == previous.back())
                            {
                                // will execute when coming back to a net along the path of predecessor nets (i.e., after finding a target gate or when unable to propagate further)
                                stack.pop_back();
                                previous.pop_back();
                                continue;
                            }

                            visited.insert(current);

                            if (const auto pred_cache_it = pred_cache.find(current); pred_cache_it != pred_cache.end())
                            {
                                auto& pred_cached_net_groups = std::get<1>(*pred_cache_it);

                                // add cached known predecessor net groups to cache of all predecessor nets
                                for (const auto* n : previous)
                                {
                                    pred_cache[n].insert(pred_cached_net_groups.cbegin(), pred_cached_net_groups.cend());
                                }

                                // add cached net groups to known predecessor net groups of current gate
                                netlist_abstr.gate_to_known_predecessor_groups[gate->get_id()].insert(pred_cached_net_groups.cbegin(), pred_cached_net_groups.cend());

                                stack.pop_back();
                            }
                            else
                            {
                                if (const auto group_it = net_to_group_index.find(current); group_it != net_to_group_index.end())
                                {
                                    for (const auto* n : previous)
                                    {
                                        pred_cache[n].insert(group_it->second);
                                    }
                                    netlist_abstr.gate_to_known_predecessor_groups[gate->get_id()].insert(group_it->second);
                                }

                                bool added = false;
                                for (const auto* ep : current->get_sources())
                                {
                                    auto* g = ep->get_gate();
                                    if (config.gate_types.find(g->get_type()) == config.gate_types.end())
                                    {
                                        // propagate further by adding predecessors to stack
                                        for (const auto* n : g->get_fan_in_nets())
                                        {
                                            if (visited.find(n) == visited.end())
                                            {
                                                stack.push_back(n);
                                                added = true;
                                            }
                                        }
                                    }
                                }

                                if (added)
                                {
                                    // keep track of previous net whenever propagating further
                                    previous.push_back(current);
                                }
                                else
                                {
                                    // if no change to stack, go back
                                    stack.pop_back();
                                }
                            }
                        }

                        // collect successor target gates by getting cache of all fan-out nets of start gate
                        std::unordered_set<Gate*> next_target_gates;
                        for (const auto* n : start_fan_out_nets)
                        {
                            if (const auto it = suc_cache.find(n); it != suc_cache.end())
                            {
                                const auto& cached_gates = std::get<0>(std::get<1>(*it));
                                next_target_gates.insert(cached_gates.cbegin(), cached_gates.cend());
                            }
                        }

                        for (const auto& suc : next_target_gates)
                        {
                            netlist_abstr.gate_to_successors[gate->get_id()].insert(suc->get_id());
                            netlist_abstr.gate_to_predecessors[suc->get_id()].insert(gate->get_id());
                        }
                    }
                    progress_bar.clear();
                }
            }    // namespace

            NetlistAbstraction run(const dataflow::Configuration& config, std::shared_ptr<dataflow::Grouping>& initial_grouping)
            {
                log_info("dataflow", "pre-processing netlist...");
                measure_block_time("pre-processing");
                NetlistAbstraction netlist_abstr(config.netlist);
                std::vector<std::vector<Gate*>> known_target_groups;
                std::vector<std::vector<Net*>> known_net_groups;
                identify_all_target_gates(config, netlist_abstr);
                identify_all_control_signals(config, netlist_abstr);
                identify_known_groups(config, netlist_abstr, known_target_groups);
                identify_successors_predecessors(config, netlist_abstr);

                if (config.enable_stages)
                {
                    identify_register_stages(netlist_abstr);
                }

                initial_grouping = std::make_shared<dataflow::Grouping>(netlist_abstr, known_target_groups);

                return netlist_abstr;
            }
        }    // namespace pre_processing
    }        // namespace dataflow
}    // namespace hal
