#pragma once

#include "plugin_netlist_simulator/event.h"

#include <unordered_map>
#include <vector>

namespace hal
{
    class Net;

    class Simulation
    {
    public:
        /*
         * Get the signal value of a specific net at a specific point in time
         *
         * @param[in] net - The net to inspect
         * @param[in] picoseconds - The point in (simulation) time
         * @returns the net's signal value
         */
        SignalValue get_net_value(Net* net, u64 picoseconds);

        /*
         * Adds a custom event to the simulation.
         *
         * @param[in] ev - The event to add
         */
        void add_event(const Event& ev);

        /*
         * Get all events of the simulation.
         *
         * @returns a map from net to associated events for that net sorted by time
         */
        std::unordered_map<Net*, std::vector<Event>> get_events() const;

    private:
        friend class NetlistSimulator;
        std::unordered_map<Net*, std::vector<Event>> m_events;
    };

}    // namespace hal
