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

#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class PLUGIN_API NetlistPreprocessingPlugin : public BasePluginInterface
    {
    public:
        /**
         * Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /**
         * Removes all LUT fan-in endpoints that do not correspond to a variable within the Boolean function that determines the output of a gate.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @returns The number of removed LUT endpoints on success, an error otherwise.
         */
        static Result<u32> remove_unused_lut_inputs(Netlist* nl);

        /**
         * Removes buffer gates from the netlist and connect their fan-in to their fan-out nets.
         * Considers all combinational gates and takes their inputs into account.
         * For example, a 2-input AND gate with one input being connected to constant `1` will also be removed.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @returns The number of removed buffers on success, an error otherwise.
         */
        static Result<u32> remove_buffers(Netlist* nl);

        /**
         * Removes redundant gates from the netlist, i.e., gates that are functionally equivalent and are connected to the same input nets.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of removed gates on success, an error otherwise.
         */
        static Result<u32> remove_redundant_logic(Netlist* nl);

        /**
         * Removes gates which outputs are all unconnected and not a global output net.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of removed gates on success, an error otherwise.
         */
        static Result<u32> remove_unconnected_gates(Netlist* nl);

        /**
         * Remove nets which have no source and not destination.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of removed nets on success, an error otherwise.
         */
        static Result<u32> remove_unconnected_nets(Netlist* nl);

        /**
         * Replaces pins connected to GND/VCC with constants and simplifies the boolean function of a LUT by recomputing the INIT string.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of simplified INIT strings on success, an error otherwise.
         */
        static Result<u32> simplify_lut_inits(Netlist* nl);

        /**
         * Builds the Boolean function of each output pin of the gate and constructs a gate tree implementing it.
         * Afterwards the original output net is connected to the built gate tree and the gate is deleted if the 'delete_gate' flag is set.
         * 
         * For the decomposition we currently only support the base operands AND, OR, INVERT, XOR.
         * The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate - The gate to decompose.
         * @param[in] delete_gate - Determines whether the original gate gets deleted by the function, defaults to true,
         * @return Ok on success, an error otherwise.
         */
        static Result<std::monostate> decompose_gate(Netlist* nl, Gate* g, const bool delete_gate = true);

        /**
         * Decomposes each gate of the specified type by building the Boolean function for each output pin of the gate and contructing a gate tree implementing it.
         * Afterwards the original gate is deleted and the output net is connected to the built gate tree.
         * 
         * For the decomposition we currently only support the base operands AND, OR, INVERT, XOR.
         * The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate_types - The gate types that should be decomposed.
         * @return Ok and the number of decomposed gates on success, an error otherwise.
         */
        static Result<u32> decompose_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types);

        /**
         * Tries to reconstruct a name and index for each flip flop that was part of a multibit wire in the verilog code.
         * This is NOT a general netlist reverse engineering algorithm and ONLY works on synthesized netlists with names annotated by the synthesizer.
         * This function mainly focuses netlists synthesized with yosys since yosys names the output wires of the flip flops but not the gate it self.
         * We try to reconstruct name and index for each flip flop based on the name of its output nets.
         * 
         * The reconstructed indexed identifiers get annoated to the flip flop in the gate data container.
         * 
         * @param[in] nl - The netlist to operate on.
         * return OK and the number of reconstructed names on success, an error otherwise.
        */
        static Result<u32> reconstruct_indexed_ff_identifiers(Netlist* nl);

        /**
         * Parses a design exchange format file and extracts the coordinated of a placed design for each component/gate.
         * The extracted coordinates get annotated to the gates.
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] def_file - Path to the def file.
         * return OK on success, an error otherwise.
        */
        static Result<std::monostate> parse_def_file(Netlist* nl, const std::filesystem::path& def_file);
    };
}    // namespace hal
