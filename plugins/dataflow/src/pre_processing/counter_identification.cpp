#include "pre_processing/counter_identification.h"

#include "common/netlist_abstraction.h"
#include "core/log.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "utils/utils.h"

#include <list>
#include <queue>
#include <unordered_map>

namespace hal
{
    namespace pre_processing
    {
        namespace
        {
            std::vector<Gate*> find_chain(NetlistAbstraction& netlist_abstr, Gate* sg)
            {
                std::vector<Gate*> chain;
                chain.push_back(sg);

                std::queue<Gate*> gate_queue;

                // loop to itself? if not return
                auto successors_of_start_gate = netlist_abstr.gate_to_successors.at(sg->get_id());
                if (successors_of_start_gate.find(sg->get_id()) == successors_of_start_gate.end())
                {
                    return std::vector<Gate*>();
                }

                std::set<u32> filtered_successors;

                // first check: keep only successors that have a loop to themselfes
                for (const auto successor_of_start_gate : successors_of_start_gate)
                {
                    // first check: check if all form loop
                    auto successors_of_successor = netlist_abstr.gate_to_successors.at(successor_of_start_gate);
                    if (successors_of_successor.find(successor_of_start_gate) != successors_of_successor.end())
                    {
                        filtered_successors.emplace(successor_of_start_gate);
                    }
                }

                if (sg->get_name().find("ISR") == std::string::npos)
                {
                    return std::vector<Gate*>();
                }

                if (filtered_successors.empty())
                {
                    return std::vector<Gate*>();
                }

                log_info("dataflow", "processing gate: {}", sg->get_name());

                log_info("dataflow", "\tstill got {} successors", filtered_successors.size());

                // second check: build levels. every FF has to reach the next ones
                std::map<u32, std::vector<u32>> level_to_successor_ffs;
                u32 max_level = filtered_successors.size();
                for (const auto successor_of_start_gate : filtered_successors)
                {
                    auto gate_to_check           = netlist_abstr.nl->get_gate_by_id(successor_of_start_gate);
                    auto successors_of_successor = netlist_abstr.gate_to_successors.at(successor_of_start_gate);
                }
                for (const auto successor_of_start_gate : filtered_successors)
                {
                    log_info("dataflow", "\t{}", netlist_abstr.nl->get_gate_by_id(successor_of_start_gate)->get_name());
                }

                return chain;
            }

            void find_counters_wordrev(NetlistAbstraction& netlist_abstr)
            {
                auto total_begin_time = std::chrono::high_resolution_clock::now();
                auto begin_time       = std::chrono::high_resolution_clock::now();

                std::set<std::vector<Gate*>> candidates;

                for (const auto& sg : netlist_abstr.all_sequential_gates)
                {
                    bool do_search = true;
                    for (const auto& candidate : candidates)
                    {
                        auto it = std::find(candidate.begin(), candidate.end(), sg);
                        if (it != candidate.end())
                        {
                            do_search = false;
                        }
                    }
                    if (do_search)
                    {
                        auto chain = find_chain(netlist_abstr, sg);
                        if (chain.size() <= 3)
                        {
                            continue;
                        }
                        bool replace = false;

                        for (const auto& chain_sg : chain)
                        {
                            for (const auto& candidate : candidates)
                            {
                                auto it = std::find(candidate.begin(), candidate.end(), chain_sg);
                                if (it != candidate.end())
                                {
                                    if (chain.size() > candidate.size())
                                    {
                                        replace = true;
                                    }
                                }
                                if (replace)
                                {
                                    candidates.erase(candidate);
                                    break;
                                }
                            }
                            if (replace)
                            {
                                break;
                            }
                        }

                        candidates.insert(chain);
                    }
                }

                for (const auto& candidate : candidates)
                {
                    log_info("dataflow", "shift_register: {}", candidate.size());

                    for (const auto candidate_gate : candidate)
                    {
                        log_info("dataflow", "\t{}", candidate_gate->get_name());
                    }
                }

                log_info("dataflow", "pre_processing_pass 'identify_counters' executed in {:3.2f}s", seconds_since(total_begin_time));
            }

        }    // namespace

        void identify_counters(NetlistAbstraction& netlist_abstr)
        {
            // find_counters_wordrev(netlist_abstr);
            // exit(0);
        }
    }    // namespace pre_processing
}    // namespace hal
