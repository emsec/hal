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

#include "def.h"
#include "netlist/netlist_constants.h"

// forward declaration
class netlist;
class gate;
class gate_type;
class net;
class module;
struct endpoint;

/**
 * @ingroup netlist
 */
class NETLIST_API netlist_internal_manager
{
    friend class netlist;
    friend class net;
    friend class module;

private:
    netlist* m_netlist;

    explicit netlist_internal_manager(netlist* nl);

    ~netlist_internal_manager() = default;

    // gate functions

    std::shared_ptr<gate> create_gate(u32 id, std::shared_ptr<const gate_type> gt, const std::string& name, float x, float y);
    bool delete_gate(std::shared_ptr<gate> gate);

    bool is_gate_type_invalid(std::shared_ptr<const gate_type> gt) const;

    // net functions

    std::shared_ptr<net> create_net(u32 id, const std::string& name);
    bool delete_net(std::shared_ptr<net> net);

    bool net_set_src(std::shared_ptr<net> const net, endpoint src);
    bool net_remove_src(std::shared_ptr<net> const net);
    bool net_add_dst(std::shared_ptr<net> const net, endpoint dst);
    bool net_remove_dst(std::shared_ptr<net> const net, endpoint dst);

    // module functions

    std::shared_ptr<module> create_module(u32 id, std::shared_ptr<module> parent, const std::string& name);
    bool delete_module(std::shared_ptr<module> module);

    bool module_assign_gate(std::shared_ptr<module> m, std::shared_ptr<gate> g);
    bool module_remove_gate(std::shared_ptr<module> m, std::shared_ptr<gate> g);
};
