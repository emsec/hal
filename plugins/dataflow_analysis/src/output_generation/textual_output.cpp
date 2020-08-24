#include "dataflow_analysis/output_generation/textual_output.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

#include <fstream>
#include <iomanip>

namespace hal
{
    namespace textual_output
    {
        void write_register_output(const std::shared_ptr<Grouping>& state, const std::string m_path, const std::string file_name)
        {
            std::string m_path_textual_ouptut = m_path + file_name;
            std::ofstream result_out(m_path_textual_ouptut, std::ofstream::out);

            result_out << "State:";
            result_out << "\n\n";

            for (const auto& [group_id, gates] : state->gates_of_group)
            {
                result_out << "ID:" << group_id << ", ";
                result_out << "Size:" << gates.size() << ", ";
                result_out << "RS: {" << utils::join(", ", state->get_register_stage_intersect_of_group(group_id)) << "}, ";
                result_out << "CLK: {" << utils::join(", ", state->get_clock_signals_of_group(group_id)) << "}, ";
                result_out << "CS: {" << utils::join(", ", state->get_control_signals_of_group(group_id)) << "}, ";
                result_out << "R: {" << utils::join(", ", state->get_reset_signals_of_group(group_id)) << "}" << std::endl;

                auto unsorted_successors = state->get_successor_groups_of_group(group_id);
                result_out << "  Successors:   {" + utils::join(", ", std::set<u32>(unsorted_successors.begin(), unsorted_successors.end())) << "}" << std::endl;

                auto unsorted_predecessors = state->get_predecessor_groups_of_group(group_id);
                result_out << "  Predecessors: {" + utils::join(", ", std::set<u32>(unsorted_predecessors.begin(), unsorted_predecessors.end())) << "}" << std::endl;

                std::unordered_map<u32, std::vector<std::string>> texts;
                std::unordered_map<u32, u32> text_max_lengths;

                for (const auto& single_ff : gates)
                {
                    auto name = state->netlist_abstr.nl->get_gate_by_id(single_ff)->get_name() + ", ";
                    if (state->netlist_abstr.yosys)
                    {
                        auto net = state->netlist_abstr.nl->get_gate_by_id(single_ff)->get_fan_out_net("Q");
                        name     = state->netlist_abstr.nl->get_gate_by_id(single_ff)->get_name() + ", ";
                        if (net != nullptr)
                        {
                            name = net->get_name();
                        }
                    }
                    auto type          = "type: " + state->netlist_abstr.nl->get_gate_by_id(single_ff)->get_type()->get_name() + ", ";
                    auto id            = "id: " + std::to_string(state->netlist_abstr.nl->get_gate_by_id(single_ff)->get_id()) + ", ";
                    std::string stages = "RS: ";
                    if (auto it = state->netlist_abstr.gate_to_register_stages.find(single_ff); it != state->netlist_abstr.gate_to_register_stages.end())
                    {
                        stages += "{" + utils::join(", ", std::set<u32>(it->second.begin(), it->second.end())) + "}";
                    }

                    std::vector<std::string> data = {name, type, id, stages};
                    for (u32 i = 0; i < data.size(); ++i)
                    {
                        text_max_lengths[i] = std::max(text_max_lengths[i], (u32)data[i].size());
                    }
                    texts.emplace(single_ff, data);
                }

                for (const auto& single_ff : std::set<u32>(gates.begin(), gates.end()))
                {
                    auto& data = texts[single_ff];
                    for (u32 i = 0; i < data.size(); ++i)
                    {
                        result_out << std::setw(text_max_lengths[i]) << std::left << data[i];
                    }
                    result_out << std::endl;
                }
                result_out << "\n";
            }
            result_out.close();
        }
    }    // namespace textual_output
}    // namespace hal
