#include "dataflow_analysis/pre_processing/pre_processing.h"

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
                // void remove_buffers(NetlistAbstraction& netlist_abstr)
                // {
                //     auto buf_gates = netlist_abstr.nl->get_gates([](auto g) {
                //         const GateType* gt = g->get_type();
                //         return gt->get_name().find("BUF") != std::string::npos && gt->get_input_pins().size() == 1 && gt->get_output_pins().size() == 1;
                //     });
                //     log_info("dataflow", "removing {} buffers", buf_gates.size());
                //     for (const auto& g : buf_gates)
                //     {
                //         auto in_net  = *(g->get_fan_in_nets().begin());
                //         auto out_net = *(g->get_fan_out_nets().begin());
                //         auto dsts    = out_net->get_destinations();
                //         for (const auto& dst : dsts)
                //         {
                //             out_net->remove_destination(dst->get_gate(), dst->get_pin());
                //             in_net->add_destination(dst->get_gate(), dst->get_pin());
                //         }
                //         netlist_abstr.nl->delete_net(out_net);
                //         netlist_abstr.nl->delete_gate(g);
                //     }
                // }

                // void merge_duplicated_logic_cones(NetlistAbstraction& netlist_abstr)
                // {
                //     measure_block_time("merge duplicated logic cones");

                //     auto all_sequential_gates = netlist_abstr.all_sequential_gates;
                //     auto all_gates            = netlist_abstr.nl->get_gates();
                //     std::sort(all_gates.begin(), all_gates.end());

                //     std::vector<Gate*> all_combinational_gates;
                //     all_combinational_gates.reserve(all_gates.size() - all_sequential_gates.size());
                //     std::set_difference(all_gates.begin(), all_gates.end(), all_sequential_gates.begin(), all_sequential_gates.end(), std::back_inserter(all_combinational_gates));

                //     std::unordered_map<Gate*, std::tuple<std::vector<std::string>, std::vector<BooleanFunction::Value>>> characteristic_of_gate;

                //     log_info("dataflow", "computing boolean functions...");
                //     {
                //         measure_block_time("computing boolean functions");

                //         // allocate values for all the gates so that each thread can access it without changing the container
                //         for (auto gate : all_combinational_gates)
                //         {
                //             characteristic_of_gate[gate] = {};
                //         }

                //         utils::parallel_for_each(all_combinational_gates, [&characteristic_of_gate](auto& g) { characteristic_of_gate.at(g) = compute_merge_characteristic_of_gate(g); });
                //     }

                //     log_info("dataflow", "merging gates...");
                //     u32 gates_removed = 0;
                //     measure_block_time("merging gates");
                //     bool changes = true;
                //     while (changes)
                //     {
                //         changes = false;

                //         // map from gate we will keep (key) to set of gates that have exactly the same function and will be removed
                //         std::map<std::tuple<std::vector<std::string>, std::vector<BooleanFunction::Value>>, std::unordered_set<Gate*>> duplicate_gates;
                //         for (auto it : characteristic_of_gate)
                //         {
                //             duplicate_gates[it.second].insert(it.first);
                //         }

                //         // delete all duplicate gates
                //         for (const auto& [function, gate_set] : duplicate_gates)
                //         {
                //             if (gate_set.size() > 1)
                //             {
                //                 changes      = true;
                //                 auto it      = gate_set.begin();
                //                 Gate* gate_1 = *it;
                //                 it++;
                //                 std::vector<std::string> out_pins = gate_1->get_type()->get_output_pins();

                //                 std::unordered_set<Gate*> affected_gates;
                //                 for (; it != gate_set.end(); ++it)
                //                 {
                //                     auto gate_2 = *it;

                //                     for (auto out_pin : out_pins)
                //                     {
                //                         auto merge_net  = gate_1->get_fan_out_net(out_pin);
                //                         auto remove_net = gate_2->get_fan_out_net(out_pin);
                //                         if (remove_net != nullptr)
                //                         {
                //                             if (merge_net == nullptr)
                //                             {
                //                                 remove_net->remove_source(gate_2, out_pin);
                //                                 remove_net->add_source(gate_1, out_pin);
                //                             }
                //                             else
                //                             {
                //                                 for (auto dst : remove_net->get_destinations())
                //                                 {
                //                                     remove_net->remove_destination(dst->get_gate(), dst->get_pin());
                //                                     merge_net->add_destination(dst->get_gate(), dst->get_pin());
                //                                     affected_gates.insert(dst->get_gate());
                //                                 }
                //                                 netlist_abstr.nl->delete_net(remove_net);
                //                             }
                //                         }
                //                     }
                //                     characteristic_of_gate.erase(gate_2);
                //                     netlist_abstr.nl->delete_gate(gate_2);
                //                     gates_removed++;
                //                 }

                //                 for (auto affected_gate : affected_gates)
                //                 {
                //                     if (auto char_it = characteristic_of_gate.find(affected_gate); char_it != characteristic_of_gate.end())
                //                     {
                //                         char_it->second = compute_merge_characteristic_of_gate(affected_gate);
                //                     }
                //                 }
                //             }
                //         }
                //     }
                //     log_info("dataflow", "merged {} gates into others, {} gates left", gates_removed, all_gates.size() - gates_removed);
                // }

                void identify_all_sequential_gates(NetlistAbstraction& netlist_abstr)
                {
                    // TODO currently only accepts FFs
                    log_info("dataflow", "identifying sequential gates");
                    netlist_abstr.all_sequential_gates = netlist_abstr.nl->get_gates([&](auto g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    std::sort(netlist_abstr.all_sequential_gates.begin(), netlist_abstr.all_sequential_gates.end(), [](const Gate* g1, const Gate* g2) { return g1->get_id() < g2->get_id(); });
                    log_info("dataflow", "  #gates: {}", netlist_abstr.nl->get_gates().size());
                    log_info("dataflow", "  #sequential gates: {}", netlist_abstr.all_sequential_gates.size());
                }

                void identify_all_control_signals(NetlistAbstraction& netlist_abstr)
                {
                    auto begin_time = std::chrono::high_resolution_clock::now();
                    log_info("dataflow", "identifying control signals");
                    for (auto sg : netlist_abstr.all_sequential_gates)
                    {
                        std::vector<u32> fingerprint;
                        auto id = sg->get_id();
                        sg->get_name();

                        for (auto net : netlist_utils::get_nets_at_pins(
                                 sg, sg->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::clock; })))
                        {
                            netlist_abstr.gate_to_clock_signals[id].insert(net->get_id());
                            fingerprint.push_back(net->get_id());
                        }

                        for (auto net : netlist_utils::get_nets_at_pins(
                                 sg, sg->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::enable; })))
                        {
                            netlist_abstr.gate_to_enable_signals[id].insert(net->get_id());
                            fingerprint.push_back(net->get_id());
                        }

                        for (auto net : netlist_utils::get_nets_at_pins(
                                 sg, sg->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::reset; })))
                        {
                            netlist_abstr.gate_to_reset_signals[id].insert(net->get_id());
                            fingerprint.push_back(net->get_id());
                        }

                        for (auto net :
                             netlist_utils::get_nets_at_pins(sg, sg->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::set; })))
                        {
                            netlist_abstr.gate_to_set_signals[id].insert(net->get_id());
                            fingerprint.push_back(net->get_id());
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

            NetlistAbstraction run(Netlist* netlist, bool register_stage_identification)
            {
                log_info("dataflow", "pre-processing netlist...");
                measure_block_time("pre-processing");
                NetlistAbstraction netlist_abstr(netlist);
                //remove_buffers(netlist_abstr);
                identify_all_sequential_gates(netlist_abstr);
                //merge_duplicated_logic_cones(netlist_abstr);
                identify_all_control_signals(netlist_abstr);
                identify_all_succesors_predecessors_ffs_of_all_ffs(netlist_abstr);

                if (register_stage_identification)
                {
                    identify_register_stages(netlist_abstr);
                }

                return netlist_abstr;
            }
        }    // namespace pre_processing
    }        // namespace dataflow
}    // namespace hal
