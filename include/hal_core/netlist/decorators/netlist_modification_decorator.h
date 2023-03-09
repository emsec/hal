// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class NETLIST_API NetlistModificationDecorator
    {
    public:
        /**
         * Construct new NetlistModificationDecorator object.
         * 
         * @param[in] netlist - The netlist to operate on.
         */
        NetlistModificationDecorator(Netlist& netlist);

        /**
         * Delete all modules in the  netlist except the top module.
         * An optional filter can be specified to delete only modules fulfilling a certain condition.
         * 
         * @param[in] filter - An optional filter to be applied to the modules before deletion.
         */
        Result<std::monostate> delete_modules(const std::function<bool(const Module*)>& filter = nullptr);

        /**
         * Replace the given gate with a gate of the specified gate type.
         * A map from old to new pins must be provided in order to correctly connect the gates inputs and outputs.
         * A pin can be omitted if no connection at that pin is desired.
         * 
         * @param[in] gate - The gate to be replaced.
         * @param[in] target_type - The gate type of the replacement gate.
         * @param[in] pin_map - A map from old to new pins.
         * @returns The new gate on success, an error otherwise.
         */
        Result<Gate*> replace_gate(Gate* gate, GateType* target_type, const std::map<GatePin*, GatePin*>& pin_map);

        /**
         * Connects two gates through the specified pins.
         * If both pins are not yet connected to a net, a new net is created to connect both pins.
         * If one of the pins is already connected to a net, that net is connected to the other pin.
         * If both pins are already connected to a net, an error is returned.
         * 
         * @param[in] src_gate - The source gate.
         * @param[in] src_pin - The output pin of the source gate.
         * @param[in] dst_gate - The destination gate.
         * @param[in] dst_pin - The input pin of the destination gate.
         * @returns The connecting net on success, an error otherwise.
         */
        Result<Net*> connect_gates(Gate* src_gate, GatePin* src_pin, Gate* dst_gate, GatePin* dst_pin);

        /**
         * Connects (and thereby merges) two nets.
         * All properties of the slave net are transfered to the master net and the slave net is subsequently deleted.
         * 
         * @param[in] master_net - The net that receives all properties from the slave net. 
         * @param[in] slave_net - The net that transfers all properties to the master net and is subsequently deleted.
         * @returns The merged net on success, an error otherwise.
         */
        Result<Net*> connect_nets(Net* master_net, Net* slave_net);

    private:
        Netlist& m_netlist;
    };
}    // namespace hal