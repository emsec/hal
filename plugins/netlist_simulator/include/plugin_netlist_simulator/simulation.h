#pragma once

#include "netlist/net.h"
#include "plugin_netlist_simulator/event.h"

namespace hal
{
    class Simulation
    {
    public:
        SignalValue get_net_value(Net* net, u64 nanoseconds);

        // WARNING: unchecked inputs, use for testing only
        void add_event(const Event& ev);

        // WARNING: use for testing only
        std::unordered_map<Net*, std::vector<Event>> get_events() const;

        /*
        * Generates vcd
        * Returns string of vcd that can be written to file
        */
        std::string generate_vcd();

        /*
        * Generates vcd and groups nets with same name and consecutive ending.
        * Returns string of vcd that can be written to file
        */
        std::string generate_vcd_with_bus();

    private:
        friend class NetlistSimulator;
        std::unordered_map<Net*, std::vector<Event>> m_events;
    };

}    // namespace hal
