#include "netlist_simulator/simulation.h"

#include "hal_core/netlist/net.h"

namespace hal
{
    SignalValue Simulation::get_net_value(Net* net, u64 time)
    {
        auto it = m_events.find(net);

        SignalValue result = SignalValue::X;

        if (it != m_events.end())
        {
            for (auto& e : it->second)
            {
                if (e.time > time)
                {
                    break;
                }
                result = e.new_value;
            }
        }
        return result;
    }

    void Simulation::add_event(const Event& event)
    {
        m_events[event.affected_net].push_back(event);
    }

    std::unordered_map<Net*, std::vector<Event>> Simulation::get_events() const
    {
        return m_events;
    }
}    // namespace hal
