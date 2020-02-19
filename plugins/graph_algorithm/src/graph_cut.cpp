#include "plugin_graph_algorithm.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/netlist.h"

std::vector<std::set<std::shared_ptr<gate>>>
    plugin_graph_algorithm::get_graph_cut(std::shared_ptr<netlist> const g, std::shared_ptr<gate> const current_gate, const u32 depth, const std::set<std::string> terminal_gate_type)
{
    if (g == nullptr)
    {
        log_error(this->get_name(), "parameter 'g' is nullptr.");
        return std::vector<std::set<std::shared_ptr<gate>>>();
    }
    if (current_gate == nullptr)
    {
        log_error(this->get_name(), "parameter 'gate' is nullptr.");
        return std::vector<std::set<std::shared_ptr<gate>>>();
    }
    if (depth == std::numeric_limits<u32>::max() && terminal_gate_type.empty())
    {
        log_error(this->get_name(), "parameter 'depth' is 0 and no terminal gate type defined.");
        return std::vector<std::set<std::shared_ptr<gate>>>();
    }

    std::vector<std::set<std::shared_ptr<gate>>> result;
    result.push_back({current_gate});

    if (depth == 1)
    {
        return result;
    }

    for (u32 i = 1; i < depth; i++)
    {
        std::set<std::shared_ptr<gate>> previous_state = result.back(), next_state = std::set<std::shared_ptr<gate>>();
        for (const auto& it : previous_state)
        {
            for (const auto& predecessor : it->get_predecessors())
            {
                if (terminal_gate_type.find(predecessor.get_gate()->get_type()->get_name()) == terminal_gate_type.end())
                {
                    next_state.insert(predecessor.get_gate());
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
