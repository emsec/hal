#include "plugin_netlist_simulator/simulation.h"

namespace hal
{
    SignalValue Simulation::get_net_value(Net* net, u64 nanoseconds)
    {
        auto it = m_events.find(net);

        SignalValue result = SignalValue::X;
        if (it != m_events.end())
        {
            for (auto& e : it->second)
            {
                if (e.time > nanoseconds)
                {
                    break;
                }
                result = e.new_value;
            }
        }
        return result;
    }

    void Simulation::add_event(const Event& ev)
    {
        m_events[ev.affected_net].push_back(ev);
    }

    std::unordered_map<Net*, std::vector<Event>> Simulation::get_events() const
    {
        return m_events;
    }

    std::string Simulation::generate_vcd()
    {
        return "";
    }

    std::string Simulation::generate_vcd_with_bus()
    {
        return "";
    }
}    // namespace hal
