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
#include "hal_core/utilities/result.h"
#include "nlohmann/json.hpp"

#include <map>
#include <vector>

namespace hal
{
    class Gate;
    class Netlist;
    class GateType;
    class GateLibrary;
    class Module;
    class Net;

    enum class PinDirection;

    namespace netlist_preprocessing
    {
        /**
         * Removes all LUT fan-in endpoints that do not correspond to a variable within the Boolean function that determines the output of a gate.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @returns OK() and the number of removed LUT endpoints on success, an error otherwise.
         */
        Result<u32> remove_unused_lut_inputs(Netlist* nl);

        /**
         * Removes buffer gates from the netlist and connect their fan-in to their fan-out nets.
         * Considers all combinational gates and takes their inputs into account.
         * For example, a 2-input AND gate with one input being connected to constant `1` will also be removed.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @returns OK() and the number of removed buffers on success, an error otherwise.
         */
        Result<u32> remove_buffers(Netlist* nl);

        /**
         * Removes redundant gates from the netlist, i.e., gates that are functionally equivalent and are connected to the same input nets.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] filter - Optional filter to fine-tune which gates are being replaced. Default to a `nullptr`.
         * @return OK() and the number of removed gates on success, an error otherwise.
         */
        Result<u32> remove_redundant_gates(Netlist* nl, const std::function<bool(const Gate*)>& filter = nullptr);

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
        Result<u32> remove_redundant_loops(Netlist* nl);

        /**
         * Removes redundant logic trees made up of combinational gates.
         * If two trees compute the exact same function even if implemented with different gates we will disconnect one of the trees and afterwards clean up all dangling gates and nets. 
         * Parts of this function can be sped up using threads.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] num_threads - The number of threads to use. 
         * @return OK() and the number of disconnected net on success, an error otherwise.
         */
        Result<u32> remove_redundant_logic_trees(Netlist* nl, const u32 num_threads = 1);

        /**
         * Removes gates for which all fan-out nets do not have a destination and are not global output nets.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed gates on success, an error otherwise.
         */
        Result<u32> remove_unconnected_gates(Netlist* nl);

        /**
         * Removes nets who have neither a source, nor a destination.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed nets on success, an error otherwise.
         */
        Result<u32> remove_unconnected_nets(Netlist* nl);

        /**
         * Calls remove_unconnected_gates / remove_unconnected_nets until there are no further changes.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of removed nets and gates on success, an error otherwise.
         */
        Result<u32> remove_unconnected_looped(Netlist* nl);

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
        Result<u32> manual_mux_optimizations(Netlist* nl, GateLibrary* mux_inv_gl);

        /**
         * Builds for all gate output nets the Boolean function and substitutes all variables connected to vcc/gnd nets with the respective boolean value.
         * If the function simplifies to a  boolean constant cut the connection to the nets destinations and directly connect it to vcc/gnd. 
         * 
         * @param[in] nl - The netlist to operate on.
         * @return OK() and the number rerouted destinations on success, an error otherwise.
         */
        Result<u32> propagate_constants(Netlist* nl);

        /**
         * Removes two consecutive inverters and reconnects the input of the first inverter to the output of the second one.
         * If the first inverter has additional successors, only the second inverter is deleted.
         * 
         * @param[in] nl - The netlist to operate on.
         * @returns OK() and the number of removed inverter gates on success, an error otherwise.
         */
        Result<u32> remove_consecutive_inverters(Netlist* nl);

        /**
         * Replaces pins connected to GND/VCC with constants and simplifies the Boolean function of a LUT by recomputing the INIT string.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return OK() and the number of simplified INIT strings on success, an error otherwise.
         */
        Result<u32> simplify_lut_inits(Netlist* nl);

        /**
         * Represents an identifier with an associated index and additional metadata, used for reconstructing and annotating names and indices
         * for flip flops in synthesized netlists based on input and output net names as well as gate names.
         * 
         * This struct is designed specifically for use with synthesized netlists. By analyzing net and gate names, we attempt to reconstruct a  
         * multi bit word and index for each flip flop.
         * 
         * The reconstructed identifiers, stored as `indexed_identifier` instances, are added to the gate data container in the netlist.
         * 
         * Members:
         * - identifier: The reconstructed name of the flip flop.
         * - index: The index number associated with the identifier, if part of a multi-bit signal.
         * - origin: The original source or scope of the identifier.
         * - pin: The specific pin associated with the identifier.
         * - direction: The direction of the pin (e.g., INPUT, OUTPUT, INOUT).
         * - distance: The distance or offset, representing additional structural information.
         */
        struct indexed_identifier
        {
            indexed_identifier();
            indexed_identifier(const std::string& identifier, const u32 index, const std::string& origin, const std::string& pin, const PinDirection& direction, const u32 distance);

            std::string identifier; /**< The reconstructed name of the multi-bit words. */
            u32 index;              /**< The index associated with the identifier, used for multi-bit signals. */
            std::string origin;     /**< The origin or source of the identifier within the netlist (either "gate_name" or "net_name"). */
            std::string pin;        /**< The pin name associated with this identifier. */
            PinDirection direction; /**< Direction of the pin. */
            u32 distance;           /**< Distance to merged net which name this index was derived from. */

            // Overload < operator for strict weak ordering
            bool operator<(const indexed_identifier& other) const;
        };

        // Serialization function for indexed_identifier as a list of values
        void to_json(nlohmann::json& j, const indexed_identifier& id);

        // Deserialization function for indexed_identifier from a list of values
        void from_json(const nlohmann::json& j, indexed_identifier& id);

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
        Result<u32> reconstruct_indexed_ff_identifiers(Netlist* nl);

        /**
         *  Tries to reconstruct top module pin groups via indexed pin names.
         *  This should really be done by the verilog parser, but this is at the moment not the case.
         * 
         * @param[in] nl - The netlist to operate on
         * 
         * @return OK() and the number of reconstructed pin groups on success.
        */
        Result<u32> reconstruct_top_module_pin_groups(Netlist* nl);

        /**
         * Parses a design exchange format file and extracts the coordinates of a placed design for each component/gate.
         * The extracted coordinates get annotated to the gates.
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] def_file - Path to the def file.
         * @return OK() on success, an error otherwise.
        */
        Result<std::monostate> parse_def_file(Netlist* nl, const std::filesystem::path& def_file);

        /**
         * Create modules from large gates like RAMs and DSPs with the option to concat mutliple gate pingroups to larger consecutive pin groups
         * 
         * TODO: document paramaters
         */
        Result<std::vector<Module*>> create_multi_bit_gate_modules(Netlist* nl, const std::map<std::string, std::map<std::string, std::vector<std::string>>>& concatenated_pin_groups);

        /**
         * TODO: document
        */
        Result<std::vector<Net*>> create_nets_at_unconnected_pins(Netlist* nl);

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
        Result<u32> unify_ff_outputs(Netlist* nl, const std::vector<Gate*>& ffs = {}, GateType* inverter_type = nullptr);
    }    // namespace netlist_preprocessing
}    // namespace hal