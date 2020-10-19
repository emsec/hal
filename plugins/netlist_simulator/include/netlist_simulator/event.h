#pragma once

#include "hal_core/netlist/net.h"
#include "netlist_simulator/signal_value.h"

namespace hal
{
    struct Event
    {
        Net* affected_net;
        SignalValue new_value;
        u64 time;
        u64 id;

        /**
         * Tests whether two events are equal.
         *
         * @param[in] other - Event to compare to.
         * @returns True when both events are equal, false otherwise.
         */
        bool operator==(const Event& other) const
        {
            return affected_net == other.affected_net && new_value == other.new_value && time == other.time;
        }

        /**
         * Tests whether one event happened before the other.
         *
         * @param[in] other - Event to compare to.
         * @returns True when this event happened before the other, false otherwise.
         */
        bool operator<(const Event& other) const
        {
            if (time != other.time)
            {
                return time < other.time;
            }
            return id < other.id;
        }
    };
}    // namespace hal
