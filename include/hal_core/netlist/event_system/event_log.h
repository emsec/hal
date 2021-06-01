//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"

#include "hal_core/netlist/event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"


/**
 * @ingroup events
 */

namespace hal
{
    namespace event_log
    {
         /**
         * dump gate events to hal log
         * @param[in] event - gate event
         * @param[in] gate - pointer to gate
         * @param[in] associated_data - additional data
         */
        NETLIST_API void handle_gate_event(GateEvent::event event, Gate* gate, u32 associated_data);

        /**
        * dump net events to hal log
        * @param[in] event - net event
        * @param[in] net - pointer to net
        * @param[in] associated_data - additional data
        */
        NETLIST_API void handle_net_event(NetEvent::event event, Net* net, u32 associated_data);

        /**
        * dump netlist events to hal log
        * @param[in] event - netlist event
        * @param[in] netlist - pointer to netlist
        * @param[in] associated_data - additional data
        */
        NETLIST_API void handle_netlist_event(NetlistEvent::event event, Netlist* netlist, u32 associated_data);

        /**
        * dump grouping events to hal log
        * @param[in] event - grouping event
        * @param[in] grp - pointer to grouping
        * @param[in] associated_data - additional data
        */
        NETLIST_API void handle_grouping_event(GroupingEvent::event event, Grouping* grp, u32 associated_data);

        /**
        * dump module events to hal log
        * @param[in] event - module event
        * @param[in] module - pointer to module
        * @param[in] associated_data - additional data
        */
        NETLIST_API void handle_module_event(ModuleEvent::event event, Module* module, u32 associated_data);

        /**
         * enable event log
         * @param[in] enable - enable logging for all event types
         */
        NETLIST_API void enable_event_log(bool enable);

    }    // namespace event_log
}    // namespace hal
