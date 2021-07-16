#include "netlist_simulator/netlist_simulator.h"

namespace hal
{
    bool NetlistSimulator::SimulationGateCombinational::simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)
    {
        UNUSED(simulation);

        // compute delay, currently just a placeholder
        u64 delay = 0;

        for (auto out_net : output_nets)
        {
            BooleanFunction::Value result = functions[out_net](input_values);

            new_events[std::make_pair(out_net, event.time + delay)] = result;
        }

        return true;
    }
}    // namespace hal