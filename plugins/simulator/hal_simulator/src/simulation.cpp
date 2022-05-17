#include "netlist_simulator/simulation.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    BooleanFunction::Value Simulation::get_net_value(const Net* net, u64 time) const
    {
        BooleanFunction::Value result = BooleanFunction::Value::X;

        if (auto it = m_events.find(net); it != m_events.end())
        {
            for (const WaveEvent& e : it->second)
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

    void Simulation::add_event(const WaveEvent &event)
    {
        m_events[event.affected_net].push_back(event);
    }

    std::unordered_map<const Net*, std::vector<WaveEvent>> Simulation::get_events() const
    {
        return m_events;
    }

    std::vector<WaveEvent> Simulation::get_events_by_net_id(u32 netId, bool* found) const
    {
        for (auto it=m_events.begin(); it!=m_events.end(); ++it)
            if (it->first->get_id() == netId)
            {
                if (found) *found = true;
                return it->second;
            }
        if (found) *found = false;
        return std::vector<WaveEvent>();
    }

}    // namespace hal
