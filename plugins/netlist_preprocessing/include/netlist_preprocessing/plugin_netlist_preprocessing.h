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

#include "hal_core/plugin_system/plugin_interface_base.h"

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
         * Replaces pins connected to GND/VCC with constants and simplifies the booleanfunction of a LUT but recomputing the INIT string.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of simplified INIT strings on success, an error otherwise.
         */
        static Result<u32> simplify_lut_inits(Netlist* nl);
    };
}    // namespace hal
