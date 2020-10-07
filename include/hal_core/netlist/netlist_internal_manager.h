//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

        explicit NetlistInternalManager(Netlist* nl);

        ~NetlistInternalManager() = default;

        // gate functions
        Gate* create_gate(u32 id, const GateType* gt, const std::string& name, float x, float y);
        bool delete_gate(Gate* gate);
        bool is_gate_type_invalid(const GateType* gt) const;

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
        bool module_remove_gate(Module* m, Gate* g);

        // grouping functions
        Grouping* create_grouping(u32 id, const std::string name);
        bool delete_grouping(Grouping* grouping);

        // caches
        void clear_caches();
        mutable std::map<std::pair<std::vector<std::string>, u64>, BooleanFunction> m_lut_function_cache;
    };
}    // namespace hal
