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
 * @file instantiate.h
 * @brief Instantiation of a netlist IR design into a netlist, against a gate library.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/netlist_ir/netlist_ir.h"
#include "hal_core/utilities/result.h"

#include <memory>
#include <string>

namespace hal
{
    class GateLibrary;
    class Netlist;

    namespace netlist_ir
    {
        /**
         * Settings for the instantiation of a design.
         */
        struct NETLIST_API InstantiationOptions
        {
            /** Create a net for every signal, even for one that nothing drives or reads. */
            bool keep_unconnected_signals = false;

            /** What separates the instance path from the name when a name has to be prefixed to stay unique. */
            std::string instance_name_separator = "/";

            /** The gate type to drive the constant `0` with; empty selects the first GND type of the gate library. */
            std::string gnd_gate_type;

            /** The gate type to drive the constant `1` with; empty selects the first VCC type of the gate library. */
            std::string vcc_gate_type;

            /** The name of the top module of the netlist; the design name is the top module's type regardless. */
            std::string top_module_name = "top_module";
        };

        /**
         * Instantiate a design against a gate library.
         *
         * The design is validated first. Its hierarchy is walked from the top module; every alias, every connection
         * to a module port and every constant is resolved with a union-find before anything is created, so that each
         * net is created exactly once and nothing is merged afterwards. A net is created for every class of bits that
         * connects to a gate pin or is a port of the top module; other signals produce no net unless the options say
         * so, and their attributes go with them (Yosys annotates every wire, used or not). Gate types and pins are
         * resolved against the library by exact name first and then by a unique case-insensitive match. Parameters
         * and attributes land in the typed stores of the created objects; a parameter that the gate type declares
         * takes the gate type's declaration.
         *
         * A net is named after the port or signal bits it consists of: a port of the top module comes first, then the
         * shortest instance path, then the receiving side of an assignment, then the earliest declaration. Gates,
         * modules and nets carry their plain name if it is unique across the netlist and are prefixed with the name
         * of the module they belong to otherwise, as the netlist parsers have always done.
         *
         * @param[in] design - The design.
         * @param[in] gate_library - The gate library to resolve gate types against.
         * @param[in] options - The settings.
         * @returns The netlist on success, an error naming the instance and location otherwise.
         */
        NETLIST_API Result<std::unique_ptr<Netlist>> instantiate(const Design& design, const GateLibrary* gate_library, const InstantiationOptions& options = {});
    }    // namespace netlist_ir
}    // namespace hal
