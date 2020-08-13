#pragma once

#include "netlist/net.h"
#include "plugin_netlist_simulator/signal_value.h"

namespace hal
{
    struct Event
    {
        Net* affected_net;
        SignalValue new_value;
        u64 time;
        u64 id;

        bool operator==(const Event& other) const
        {
            return affected_net == other.affected_net && new_value == other.new_value && time == other.time;
        }
    };
}    // namespace hal
