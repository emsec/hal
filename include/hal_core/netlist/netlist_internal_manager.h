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
#include "hal_core/netlist/event_system/event_handler.h"

#include <map>
#include <vector>

namespace hal
{
    // forward declaration
    class Netlist;
    class Gate;
    class GateType;
    class Net;
    class Module;
    class Endpoint;
    class Grouping;
    class BooleanFunction;

    /**
     * @ingroup netlist
     */
    class NETLIST_API NetlistInternalManager
    {
        friend class Netlist;
        friend class Module;
        friend class Net;
        friend class Gate;
        friend class Grouping;

    private:
        Netlist* m_netlist;
        EventHandler* m_event_handler;

        explicit NetlistInternalManager(Netlist* nl, EventHandler* eh);
        ~NetlistInternalManager() = default;

        // gate functions
        Gate* create_gate(u32 id, GateType* gt, const std::string& name, i32 x, i32 y);
        bool delete_gate(Gate* gate);
        bool is_gate_type_invalid(GateType* gt) const;

        // net functions
        Net* create_net(u32 id, const std::string& name);
        bool delete_net(Net* net);
        Endpoint* net_add_source(Net* net, Gate* gate, const std::string& pin);
        bool net_remove_source(Net* net, Endpoint* ep);
        Endpoint* net_add_destination(Net* net, Gate* gate, const std::string& pin);
        bool net_remove_destination(Net* net, Endpoint* ep);

        // module functions
        Module* create_module(u32 id, Module* parent, const std::string& name);
        bool delete_module(Module* module);
        bool module_assign_gate(Module* m, Gate* g);

        // grouping functions
        Grouping* create_grouping(u32 id, const std::string name);
        bool delete_grouping(Grouping* grouping);
        bool grouping_assign_gate(Grouping* grouping, Gate* gate, bool force = false);
        bool grouping_remove_gate(Grouping* grouping, Gate* gate);
        bool grouping_assign_net(Grouping* grouping, Net* net, bool force = false);
        bool grouping_remove_net(Grouping* grouping, Net* net);
        bool grouping_assign_module(Grouping* grouping, Module* module, bool force = false);
        bool grouping_remove_module(Grouping* grouping, Module* module);

        // caches
        void clear_caches();
        mutable std::map<std::pair<std::vector<std::string>, u64>, BooleanFunction> m_lut_function_cache;
    };
}    // namespace hal
