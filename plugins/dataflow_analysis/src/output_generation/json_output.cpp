#include "dataflow_analysis/output_generation/json_output.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace hal
{
    namespace dataflow
    {
        namespace json_output
        {
            void save_state_to_json(u32 iteration,
                                    const NetlistAbstraction& netlist_abstr,
                                    const processing::Result& processing_result,
                                    const evaluation::Result& eval_result,
                                    bool with_gates,
                                    nlohmann::json& j)
            {
                // log_info("dataflow", "start save_state_to_json");
                // auto begin_time = std::chrono::high_resolution_clock::now();

                std::string round_id = "voting_" + std::to_string(iteration);

                u32 state_counter = -1;
                for (const auto& grouping : processing_result.groupings)
                {
                    const auto& state    = grouping.second;
                    std::string state_id = "state_" + std::to_string(++state_counter);

                    if (with_gates)
                    {
                        u32 group_counter = -1;
                        for (const auto& it : state->gates_of_group)
                        {
                            std::string group_name = "group_" + std::to_string(++group_counter);

                            std::vector<std::string> gate_names;
                            gate_names.reserve(it.second.size());
                            std::transform(
                                it.second.begin(), it.second.end(), std::back_inserter(gate_names), [&](auto gate_id) { return state->netlist_abstr.nl->get_gate_by_id(gate_id)->get_name(); });

                            j[netlist_abstr.nl->get_design_name()][round_id][state_id][group_name] = (gate_names);
                        }
                    }
                }

                const auto& state    = eval_result.merged_result;
                std::string state_id = "result";

                if (with_gates)
                {
                    u32 group_counter = -1;
                    for (const auto& it : state->gates_of_group)
                    {
                        std::string group_name = "group_" + std::to_string(++group_counter);

                        std::vector<std::string> gate_names;
                        gate_names.reserve(it.second.size());
                        std::transform(it.second.begin(), it.second.end(), std::back_inserter(gate_names), [&](auto gate_id) { return state->netlist_abstr.nl->get_gate_by_id(gate_id)->get_name(); });

                        j[netlist_abstr.nl->get_design_name()][round_id][state_id][group_name] = (gate_names);
                    }
                }

                // log_info("dataflow", "save_state_to_json executed in {:3.2f}s", seconds_since(begin_time));
            }
        }    // namespace json_output
    }        // namespace dataflow
}    // namespace hal
