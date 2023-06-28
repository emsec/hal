#include "dataflow_analysis/pre_processing/pre_processing.h"

#include "dataflow_analysis/api/configuration.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/pre_processing/register_stage_identification.h"
#include "dataflow_analysis/utils/parallel_for_each.h"
#include "dataflow_analysis/utils/progress_printer.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/netlist/gate.h"
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
                void identify_all_sequential_gates(NetlistAbstraction& netlist_abstr)
                {
                    // TODO currently only accepts FFs
                    log_info("dataflow", "identifying sequential gates");
                    netlist_abstr.all_sequential_gates = netlist_abstr.nl->get_gates([&](auto g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    std::sort(netlist_abstr.all_sequential_gates.begin(), netlist_abstr.all_sequential_gates.end(), [](const Gate* g1, const Gate* g2) { return g1->get_id() < g2->get_id(); });
                    log_info("dataflow", "  #gates: {}", netlist_abstr.nl->get_gates().size());
                    log_info("dataflow", "  #sequential gates: {}", netlist_abstr.all_sequential_gates.size());
                }

                void identify_all_control_signals(const dataflow::Configuration& config, NetlistAbstraction& netlist_abstr)
                {
                    auto begin_time = std::chrono::high_resolution_clock::now();
                    log_info("dataflow", "identifying control signals");
                    for (auto sg : netlist_abstr.all_sequential_gates)
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

                /* get all successor/predecessor FFs of all FFs */
                void identify_all_succesors_predecessors_ffs_of_all_ffs(NetlistAbstraction& netlist_abstr)
                {
                    log_info("dataflow", "identifying successors and predecessors of sequential gates...");
                    measure_block_time("identifying successors and predecessors of sequential gates") ProgressPrinter progress_bar;
                    float cnt = 0;

                    std::unordered_map<u32, std::vector<Gate*>> cache;

                    for (const auto& single_ff : netlist_abstr.all_sequential_gates)
                    {
                        cnt++;
                        progress_bar.print_progress(cnt / netlist_abstr.all_sequential_gates.size());
                        // create sets even if there are no successors
                        if (netlist_abstr.gate_to_successors.find(single_ff->get_id()) == netlist_abstr.gate_to_successors.end())
                        {
                            netlist_abstr.gate_to_successors[single_ff->get_id()] = std::unordered_set<u32>();
                        }
                        if (netlist_abstr.gate_to_predecessors.find(single_ff->get_id()) == netlist_abstr.gate_to_predecessors.end())
                        {
                            netlist_abstr.gate_to_predecessors[single_ff->get_id()] = std::unordered_set<u32>();
                        }
                        for (const auto& suc : netlist_utils::get_next_sequential_gates(single_ff, true, cache))
                        {
                            netlist_abstr.gate_to_successors[single_ff->get_id()].insert(suc->get_id());
                            netlist_abstr.gate_to_predecessors[suc->get_id()].insert(single_ff->get_id());
                        }
                    }
                    progress_bar.clear();
                }
            }    // namespace

            NetlistAbstraction run(const dataflow::Configuration& config)
            {
                log_info("dataflow", "pre-processing netlist...");
                measure_block_time("pre-processing");
                NetlistAbstraction netlist_abstr(config.netlist);
                //remove_buffers(netlist_abstr);
                identify_all_sequential_gates(netlist_abstr);
                //merge_duplicated_logic_cones(netlist_abstr);
                identify_all_control_signals(config, netlist_abstr);
                identify_all_succesors_predecessors_ffs_of_all_ffs(netlist_abstr);

                if (config.enable_register_stages)
                {
                    identify_register_stages(netlist_abstr);
                }

                return netlist_abstr;
            }
        }    // namespace pre_processing
    }        // namespace dataflow
}    // namespace hal
