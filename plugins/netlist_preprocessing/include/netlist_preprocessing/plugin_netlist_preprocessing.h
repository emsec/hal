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
         * @returns OK() and the number of removed LUT endpoints on success, an error otherwise.
         */
        static Result<u32> remove_unused_lut_inputs(Netlist* nl);

        /**
         * Removes buffer gates from the netlist and connect their fan-in to their fan-out nets.
         * Considers all combinational gates and takes their inputs into account.
         * For example, a 2-input AND gate with one input being connected to constant `1` will also be removed.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @returns OK() and the number of removed buffers on success, an error otherwise.
         */
        static Result<u32> remove_buffers(Netlist* nl);

        /**
         * Removes redundant gates from the netlist, i.e., gates that are functionally equivalent and are connected to the same input nets.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed gates on success, an error otherwise.
         */
        static Result<u32> remove_redundant_gates(Netlist* nl);

        /**
         * Removes redundant sequential feedback loops.
         * Sometimes flip-flops and some of their combinational fan-in form a feedback loop where the flip-flop input depends on its own output.
         * For optimization, some synthesizers create multiple equivalent instances of these feedback loops.
         * To simplify structural analysis, this function removes the redundant flip-flop gate of the loop from the netlist.
         * Other preprocessing functions can then take care of the remaining combination gates of the loop.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed gates on success, an error otherwise.
         */
        static Result<u32> remove_redundant_loops(Netlist* nl);

        /**
         * Removes redundant logic trees made up of combinational gates.
         * If two trees compute the exact same function even if implemented with different gates we will disconnect one of the trees and afterwards clean up all dangling gates and nets. 
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of disconnected net on success, an error otherwise.
         */
        static Result<u32> remove_redundant_logic_trees(Netlist* nl);

        /**
         * Removes gates for which all fan-out nets do not have a destination and are not global output nets.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed gates on success, an error otherwise.
         */
        static Result<u32> remove_unconnected_gates(Netlist* nl);

        /**
         * Removes nets who have neither a source, nor a destination.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed nets on success, an error otherwise.
         */
        static Result<u32> remove_unconnected_nets(Netlist* nl);

        /**
         * Calls remove_unconnected_gates / remove_unconnected_nets until there are no further changes.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed nets and gates on success, an error otherwise.
         */
        static Result<u32> remove_unconnected_looped(Netlist* nl);

        /**
         * Apply manually implemented optimizations to the netlist centered around muxes.
         * Currently implemented optimizations include:
         *  - removing inverters incase there are inverter gates in front and behind every data input and output of the mux
         *  - optimizing and therefore unifying possible inverters preceding the select signals by resynthesizing
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] mux_inv_gl - A gate library only containing mux and inverter gates used for resynthesis.
         * @return OK() and the difference in the total number of gates caused by these optimizations.
         */
        static Result<u32> manual_mux_optimizations(Netlist* nl, GateLibrary* mux_inv_gl);

        /**
         * Builds for all gate output nets the Boolean function and substitutes all variables connected to vcc/gnd nets with the respective boolean value.
         * If the function simplifies to a static boolean constant cut the connection to the nets destinations and directly connect it to vcc/gnd. 
         * 
         * @param[in] nl - The netlist to operate on.
         * @return OK() and the number rerouted destinations on success, an error otherwise.
         */
        static Result<u32> propagate_constants(Netlist* nl);

        /**
         * Removes two consecutive inverters and reconnects the input of the first inverter to the output of the second one.
         * If the first inverter has additional successors, only the second inverter is deleted.
         * 
         * @param[in] nl - The netlist to operate on.
         * @returns OK() and the number of removed inverter gates on success, an error otherwise.
         */
        static Result<u32> remove_consecutive_inverters(Netlist* nl);

        /**
         * Replaces pins connected to GND/VCC with constants and simplifies the Boolean function of a LUT by recomputing the INIT string.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of simplified INIT strings on success, an error otherwise.
         */
        static Result<u32> simplify_lut_inits(Netlist* nl);

        /**
         * Builds the Boolean function of each output pin of the gate and constructs a small netlist computing the same function.
         * Afterwards the original gate ist replaced by this netlist and the gate is deleted if the `delete_gate` flag is set.
         * 
         * For the decomposition we currently only support the base operands AND, OR, INVERT, XOR.
         * The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate - The gate to decompose.
         * @param[in] delete_gate - Determines whether the original gate gets deleted by the function, defaults to true.
         * @return OK() on success, an error otherwise.
         */
        static Result<std::monostate> decompose_gate(Netlist* nl, Gate* g, const bool delete_gate = true);

        /**
         * Decomposes each gate of the specified type by building the Boolean function for each output pin of the gate types and constructing a small netlist implementing these.
         * Afterwards the original gates are deleted and replaced by the corresponding netlists.
         * 
         * For the decomposition we currently only support the base operands AND, OR, INVERT, XOR.
         * The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate_types - The gate types that should be decomposed.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        static Result<u32> decompose_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types);

        /**
         * Build the Boolean function of the gate and resynthesize a functional description of that function with a logic synthesizer.
         * Afterwards the original gate is replaced by the technology mapped netlist produced by the synthesizer.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] g -  The gate to resynthesize.
         * @param[in] target_lib -  Gate library containing the gates used for technology mapping.
         * @param[in] genlib_path - Path to file containing the target library in genlib format.
         * @param[in] delete_gate - Determines whether the original gate gets deleted by the function, defaults to true.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        static Result<std::monostate> resynthesize_gate(Netlist* nl, Gate* g, GateLibrary* target_lib, const std::filesystem::path& genlib_path, const bool delete_gate);

        /**
         * Build the Boolean function for each gate and resynthesize a functional description of that function with a logic synthesizer.
         * Afterwards all the original gates are replaced by the technology mapped netlists produced by the synthesizer.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gates -  The gates to resynthesize.
         * @param[in] target_lib -  Gate library containing the gates used for technology mapping.
         * @param[in] genlib_path - Path to file containing the target library in genlib format.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        static Result<u32> resynthesize_gates(Netlist* nl, const std::vector<Gate*>& gates, GateLibrary* target_lib);

        /**
         * Build the Boolean functions of all gates of the specified types and resynthesize a functional description of those functions with a logic synthesizer.
         * Afterwards the original gates are replaced by the technology mapped netlists produced by the synthesizer.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] gate_types - The gate types specifying which gates should be resynthesized.
         * @param[in] target_lib -  Gate library containing the gates used for technology mapping.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        static Result<u32> resynthesize_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl);

        /**
         * Build a Verilog description of a subgraph of gates and synthesize a new technology mapped netlist of the whole subgraph with a logic synthesizer.
         * Afterwards the original subgraph is replaced by the technology mapped netlist produced by the synthesizer.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] subgraph -  The subgraph to resynthesize.
         * @param[in] target_gl -  Gate library containing the gates used for technology mapping.
         * @return OK() and the number of decomposed gates on success, an error otherwise.
         */
        static Result<u32> resynthesize_subgraph(Netlist* nl, const std::vector<Gate*>& subgraph, GateLibrary* target_gl);

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
        static Result<u32> resynthesize_subgraph_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl);

        /**
         * Tries to reconstruct a name and index for each flip flop that was part of a multi-bit wire in the verilog code.
         * This is NOT a general netlist reverse engineering algorithm and ONLY works on synthesized netlists with names annotated by the synthesizer.
         * This function mainly focuses netlists synthesized with yosys since yosys names the output wires of the flip flops but not the gate it self.
         * We try to reconstruct name and index for each flip flop based on the name of its output nets.
         * 
         * The reconstructed indexed identifiers get annotated to the flip flop in the gate data container.
         * 
         * @param[in] nl - The netlist to operate on.
         * @return OK() and the number of reconstructed names on success, an error otherwise.
        */
        static Result<u32> reconstruct_indexed_ff_identifiers(Netlist* nl);

        /**
         *  Tries to reconstruct top module pin groups via indexed pin names.
         *  This should really be done by the verilog parser, but this is at the moment not the case.
         * 
         * @param[in] nl - The netlist to operate on
         * 
         * @return OK() and the number of reconstructed pin groups on success.
        */
        static Result<u32> reconstruct_top_module_pin_groups(Netlist* nl);

        /**
         * Parses a design exchange format file and extracts the coordinates of a placed design for each component/gate.
         * The extracted coordinates get annotated to the gates.
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] def_file - Path to the def file.
         * @return OK() on success, an error otherwise.
        */
        static Result<std::monostate> parse_def_file(Netlist* nl, const std::filesystem::path& def_file);

        /**
         * Create modules from large gates like RAMs and DSPs with the option to concat mutliple gate pingroups to larger consecutive pin groups
         * 
         * TODO: document paramaters
         */
        static Result<std::vector<Module*>> create_multi_bit_gate_modules(Netlist* nl, const std::map<std::string, std::map<std::string, std::vector<std::string>>>& concatenated_pin_groups);

        /**
         * Iterates all flip-flops of the netlist or specified by the user.
         * If a flip-flop has a `state` and a `neg_state` output, a new inverter gate is created and connected to the `state` output net as an additional destination.
         * Finally, the `neg_state` output net is disconnected from the `neg_state` pin and re-connected to the new inverter gate's output. 
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] ffs - The flip-flops to operate on. Defaults to an empty vector, in which case all flip-flops of the netlist are considered.
         * @param[in] inverter_type - The inverter gate type to use. Defaults to a `nullptr`, in which case the first inverter type found in the gate library is used.
         * @returns OK() and the number of rerouted `neg_state` outputs on success, an error otherwise.
         */
        static Result<u32> unify_ff_outputs(Netlist* nl, const std::vector<Gate*>& ffs = {}, GateType* inverter_type = nullptr);
    };
}    // namespace hal
