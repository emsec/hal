#include "hal_core/netlist/netlist_utils.h"

#include "hal_core/utilities/log.h"
#include "netlist/boolean_function.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "utils/utils.h"

#include <queue>

namespace hal
{
    namespace
    {
        struct
        {
            std::unordered_map<u32, BooleanFunction> functions;
        } _cache;

        static BooleanFunction get_function_of_gate(const Gate* gate)
        {
            if (auto it = _cache.functions.find(gate->get_id()); it != _cache.functions.end())
            {
                return it->second;
            }
            else
            {
                BooleanFunction bf = gate->get_boolean_function();
                for (auto const& input_pin : gate->get_input_pins())
                {
                    const auto& input_net = gate->get_fan_in_net(input_pin);
                    bf                    = bf.substitute(input_pin, std::to_string(input_net->get_id()));
                }
                _cache.functions.emplace(gate->get_id(), bf);
                return bf;
            }
        }
    }    // namespace

    BooleanFunction get_subgraph_function(const std::set<const Gate*> subgraph_gates, const Net* output_net)
    {
        /* check validity of subgraph_gates */
        if (subgraph_gates.empty())
        {
            log_error("verification", "parameter 'subgraph_gates' is empty");
            return BooleanFunction();
        }
        if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](auto& g) { return g == nullptr; }))
        {
            log_error("verification", "parameter 'subgraph_gates' contains a nullptr");
            return BooleanFunction();
        }
        if (output_net->get_num_of_sources() != 1)
        {
            log_error("verification", "target net has 0 or more than 1 sources.");
            return BooleanFunction();
        }
        auto nl         = (*subgraph_gates.begin())->get_netlist();
        auto start_gate = output_net->get_sources()[0].get_gate();
        auto result     = get_function_of_gate(start_gate);

        std::queue<Net*> q;
        for (auto& n : start_gate->get_fan_in_nets())
        {
            q.push(n);
        }
        while (!q.empty())
        {
            auto n = q.front();
            q.pop();
            if (n->get_num_of_sources() != 1)
            {
                log_error("verification", "net has 0 or more than 1 sources. not expanding the function here");
                continue;
            }

            auto src_gate = n->get_sources()[0].get_gate();

            // log_info("test", "processing gate {}", src_gate->get_name());
            if (subgraph_gates.find(src_gate) != subgraph_gates.end())
            {
                result = result.substitute(std::to_string(n->get_id()), get_function_of_gate(src_gate));
                //log_info("test", result.to_string());
                for (auto& sn : src_gate->get_fan_in_nets())
                {
                    q.push(sn);
                }
            }
        }
        return result;
    }

}    // namespace hal