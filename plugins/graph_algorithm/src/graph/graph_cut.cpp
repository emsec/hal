#include "graph_algorithm/plugin_graph_algorithm.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    std::vector<std::set<Gate*>> GraphAlgorithmPlugin::get_graph_cut(Netlist* const g, Gate* current_gate, const u32 depth, const std::set<std::string> terminal_gate_type)
    {
        if (g == nullptr)
        {
            log_error(this->get_name(), "parameter 'g' is nullptr.");
            return std::vector<std::set<Gate*>>();
        }
        if (current_gate == nullptr)
        {
            log_error(this->get_name(), "parameter 'gate' is nullptr.");
            return std::vector<std::set<Gate*>>();
        }
        if (depth == std::numeric_limits<u32>::max() && terminal_gate_type.empty())
        {
            log_error(this->get_name(), "parameter 'depth' is 0 and no terminal gate type defined.");
            return std::vector<std::set<Gate*>>();
        }

        std::vector<std::set<Gate*>> result;
        result.push_back({current_gate});

        if (depth == 1)
        {
            return result;
        }

        for (u32 i = 1; i < depth; i++)
        {
            std::set<Gate*> previous_state = result.back(), next_state = std::set<Gate*>();
            for (const auto& it : previous_state)
            {
                for (const auto& predecessor : it->get_predecessors())
                {
                    if (terminal_gate_type.find(predecessor->get_gate()->get_type()->get_name()) == terminal_gate_type.end())
                    {
                        next_state.insert(predecessor->get_gate());
                    }
                }
            }
            if (next_state.empty())
            {
                return result;
            }
            else
            {
                result.push_back(next_state);
            }
        }
        return result;
    }
}    // namespace hal
