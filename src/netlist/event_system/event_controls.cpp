#include "netlist/event_system/event_controls.h"

#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/module_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/netlist_event_handler.h"

namespace hal
{
    namespace event_controls
    {
        void enable_all(bool flag)
        {
            netlist_event_handler::enable(flag);
            gate_event_handler::enable(flag);
            net_event_handler::enable(flag);
            module_event_handler::enable(flag);
        }
    }    // namespace event_controls
}    // namespace hal
