
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

/**
 * @file resynthesis.h 
 * @brief This file contains functions to decompose or re-synthesize combinational parts of a gate-level netlist.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class Netlist;
    class Gate;
    class GateType;
    class GateLibrary;

    namespace resynthesis
    {
        /**
         * Decompose a combinational gate into a small circuit of AND, OR, XOR, and INVERT gates.
         * For each output pin, the resolved Boolean function (only dependent on input pins) is determined.
         * All these Boolean functions are then converted into a netlist using the previously mentioned primitive gates.
         * The target gate is then replaced in the original netlist with the circuit that was just generated.
         * The target gate is only deleted if `delete_gate` is set to `true`.
         * Gate replacement will fail if the gate library of the netlist does not contain suitable AND, OR, XOR, and INVERT gate types.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate - The gate to decompose.
         * @param[in] delete_gate - Set `true` to delete the original gate, `false` to keep it in the netlist. Defaults to `true`.
         * @return OK() on success, an error otherwise.
         */
        Result<std::monostate> decompose_gate(Netlist* nl, Gate* gate, const bool delete_gate = true);

        /**
         * Decompose all combinational gates of the specified types into small circuits of AND, OR, XOR, and INVERT gates.
         * For all output pins of each gate, the resolved Boolean function (only dependent on input pins) is determined.
         * All these Boolean functions are then converted into a circuit using the previously mentioned primitive gates.
         * The target gates are then replaced (and thereby deleted) in the original netlist with the circuit that was just generated.
         * Gate replacement will fail if the gate library of the netlist does not contain suitable AND, OR, XOR, and INVERT gate types.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate_types - The gate types to be decomposed.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        Result<u32> decompose_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types);

        /**
         * Re-synthesize a combinational gate by calling Yosys on a functional description of the gate using a reduced gate library.
         * For each output pin, the resolved Boolean function (only dependent on input pins) is determined.
         * All these Boolean functions are then written to an HDL file that is functionally equivalent to the target gate.
         * This file is fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
         * The provided gate library should be a subset of the gate library that was used to parse the netlist.
         * The target gate is then replaced in the original netlist with the circuit that was just generated.
         * The target gate is only deleted if `delete_gate` is set to `true`.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate -  The gate to re-synthesize.
         * @param[in] target_gl - The gate library that is a subset of the gate library used to parse the netlist.
         * @param[in] delete_gate - Set `true` to delete the original gate, `false` to keep it in the netlist. Defaults to `true`.
         * @return OK() on success, an error otherwise.
         */
        Result<std::monostate> resynthesize_gate(Netlist* nl, Gate* gate, GateLibrary* target_gl, const bool delete_gate = true);

        /**
         * Re-synthesize all specified combinational gates by calling Yosys on a functional description of the gates using a reduced gate library.
         * For all output pins of each gate, the resolved Boolean function (only dependent on input pins) is determined.
         * All these Boolean functions are then written to an HDL file that is functionally equivalent to the target gates.
         * This file is fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
         * The provided gate library should be a subset of the gate library that was used to parse the netlist.
         * The target gates are then replaced in the original netlist with the circuit that was just generated.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gates -  The gates to re-synthesize.
         * @param[in] target_gl - The gate library that is a subset of the gate library used to parse the netlist.
         * @return OK() and the number of re-synthesized gates on success, an error otherwise.
         */
        Result<u32> resynthesize_gates(Netlist* nl, const std::vector<Gate*>& gates, GateLibrary* target_gl);

        /**
         * Re-synthesize all combinational gates of the specified types by calling Yosys on a functional description of the gates using a reduced gate library.
         * For all output pins of each gate, the resolved Boolean function (only dependent on input pins) is determined.
         * All these Boolean functions are then written to an HDL file that is functionally equivalent to the target gates.
         * This file is fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
         * The provided gate library should be a subset of the gate library that was used to parse the netlist.
         * The target gates are then replaced in the original netlist with the circuit that was just generated.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate_types - The gate types to be re-synthesized.
         * @param[in] target_gl - The gate library that is a subset of the gate library used to parse the netlist.
         * @return OK() and the number of re-synthesized gates on success, an error otherwise.
         */
        Result<u32> resynthesize_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl);

        // TODO update docs below

        /**
         * Build a Verilog description of a subgraph of gates and synthesize a new technology mapped netlist of the whole subgraph with a logic synthesizer.
         * Afterwards the original subgraph is replaced by the technology mapped netlist produced by the synthesizer.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] subgraph -  The subgraph to resynthesize.
         * @param[in] target_gl -  Gate library containing the gates used for technology mapping.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        Result<u32> resynthesize_subgraph(Netlist* nl, const std::vector<Gate*>& subgraph, GateLibrary* target_gl);

        /**
         * Build a verilog description of the subgraph consisting of all the gates of the specified types. 
         * Then synthesize a new technology mapped netlist of the whole subgraph with a logic synthesizer.
         * Afterwards the original subgraph is replaced by the technology mapped netlist produced by the synthesizer.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate_types -  The gate types specifying which gates should be part of the subgraph.
         * @param[in] target_gl -  Gate library containing the gates used for technology mapping.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        Result<u32> resynthesize_subgraph_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl);
    }    // namespace resynthesis
}    // namespace hal
