#pragma once

#include "hal_core/netlist/net.h"
#include "netlist_simulator/signal_value.h"

namespace hal
{
    struct Event
    {
        /**
         * The net affected by the event.
         */
        Net* affected_net;

        /**
         * The new value caused by the event.
         */
        SignalValue new_value;

        /**
         * The time of the event.
         */
        u64 time;

        /**
         * The unique ID of the event.
         */
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
