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
 * @file preprocessing.h 
 * @brief This file contains functions specifically designed to preprocess Xilinx FPGA netlists.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <vector>

namespace hal
{
    class Gate;
    class Netlist;

    namespace xilinx_toolbox
    {
        /**
         * @brief Split a single `LUT6_2` gate into up to two separate LUT gates.
         *
         * Creates replacement gates depending on which outputs are connected:
         * - `O6` → `LUT6` using all 64 bits of the INIT string and all 6 inputs.
         * - `O5` → `LUT5` using bits [0, 31] of the INIT string and inputs I0-I4 (I5 is excluded, as it only selects the INIT half).
         *
         * The original `LUT6_2` gate is always deleted. The original INIT string is stored on each replacement gate under `xilinx_preprocessing_information/original_init`.
         *
         * @param[in] g - The `LUT6_2` gate to split.
         * @param[in] create_module - If `true`, a new module named after the original gate is created as a child of its parent module and all replacement gates are placed into it. If `false` (default), replacement gates inherit the parent module of the original gate.
         * @returns An empty OK result on success, an error if `g` is not of type `LUT6_2`, if the INIT string is malformed, or if the gate cannot be deleted.
         */
        Result<std::monostate> split_lut(Gate* g, bool create_module = false);

        /**
         * @brief Split a set of `LUT6_2` gates into separate LUT gates.
         *
         * Calls `split_lut` for each gate in `gates`. Gates that cannot be split are skipped with a
         * warning; processing always continues with the remaining gates.
         *
         * @param[in] gates - The `LUT6_2` gates to split.
         * @param[in] create_module - If `true`, each split gate's replacements are placed into a new module named after the original gate. If `false` (default), replacements inherit the parent module of the original gate.
         * @returns The number of successfully split gates.
         */
        Result<u32> split_luts(const std::vector<Gate*>& gates, bool create_module = false);

        /**
         * @brief Split all `LUT6_2` gates in the netlist into separate LUT gates.
         *
         * Finds all gates of type `LUT6_2` in the netlist and calls `split_luts(gates)`.
         *
         * @param[in] nl - The netlist to operate on.
         * @param[in] create_module - If `true`, each split gate's replacements are placed into a new module named after the original gate. If `false` (default), replacements inherit the parent module of the original gate.
         * @returns The number of successfully split `LUT6_2` gates.
         */
        Result<u32> split_luts(Netlist* nl, bool create_module = false);

        /**
         * @brief Split a single shift register primitive into an equivalent chain of `FDCE` flip-flops.
         *
         * Supported gate types: `SRL16`, `SRL16E`, `SRLC16E`, `SRLC32E`.
         *
         * Every address pin must be connected and driven by a constant (GND/VCC) net; an error is
         * returned otherwise.  The number of flip-flops created equals `select_value + 1`, where
         * `select_value` is the binary value encoded by the address pins (A0 = bit 0).  When the
         * cascade output (`Q15` for `SRLC16E`, `Q31` for `SRLC32E`) has downstream consumers, all
         * stages up to the maximum depth (15 or 31) are materialised so that the last flip-flop
         * correctly drives the cascade net.
         *
         * The original gate is always deleted on success.
         *
         * @param[in] g - The shift register gate to split.
         * @param[in] create_module - If `true`, a new module named after the original gate is created as a child of its parent module and all replacement flip-flops are placed into it. If `false` (default), replacement flip-flops inherit the parent module of the original gate.
         * @returns An empty OK result on success, an error if `g` is not a supported shift register
         *          type, if any address pin is unconnected or not driven by a constant (GND/VCC) net,
         *          or if the gate cannot be deleted.
         */
        Result<std::monostate> split_shift_register(Gate* g, bool create_module = false);

        /**
         * @brief Split a set of shift register primitives into equivalent `FDCE` flip-flop chains.
         *
         * Supported gate types: `SRL16`, `SRL16E`, `SRLC16E`, `SRLC32E`.
         *
         * Calls `split_shift_register` for each gate in `gates`. Gates that cannot be split (e.g.
         * because an address pin is not driven by a constant net) are skipped with a warning;
         * processing always continues with the remaining gates.
         *
         * @param[in] gates - The shift register gates to split.
         * @param[in] create_module - If `true`, each split gate's replacements are placed into a new module named after the original gate. If `false` (default), replacements inherit the parent module of the original gate.
         * @returns The number of successfully split gates.
         */
        Result<u32> split_shift_registers(const std::vector<Gate*>& gates, bool create_module = false);

        /**
         * @brief Split all shift register primitives in the netlist into `FDCE` flip-flop chains.
         *
         * Supported gate types: `SRL16`, `SRL16E`, `SRLC16E`, `SRLC32E`.
         *
         * Finds all gates of one of the supported types in the netlist and calls
         * `split_shift_registers(gates)`.
         *
         * @param[in] nl - The netlist to operate on.
         * @param[in] create_module - If `true`, each split gate's replacements are placed into a new module named after the original gate. If `false` (default), replacements inherit the parent module of the original gate.
         * @returns The number of successfully split shift register gates.
         */
        Result<u32> split_shift_registers(Netlist* nl, bool create_module = false);

        /**
         * @brief Parse an `.xdc` file and extract the position LOC and BEL data of each gate.
         * 
         * Translates the coordinates extracted from the `.xdc` file into integer values.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @param[in] xdc_file - The path to the `.xdc` file.
         * @return Ok() on success, an error otherwise.
         */
        Result<std::monostate> parse_xdc_file(Netlist* nl, const std::filesystem::path& xdc_file);
    }    // namespace xilinx_toolbox
}    // namespace hal