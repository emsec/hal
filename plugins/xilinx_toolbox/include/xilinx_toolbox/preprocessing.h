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

namespace hal
{
    class Netlist;

    namespace xilinx_toolbox
    {
        /**
         * @brief Split LUTs with two outputs into two separate LUT gates.
         * 
         * Replaces `LUT6_2` with a `LUT6` and a `LUT5` gate if the respective outputs of the `LUT6_2` are actually used, i.e., connected to other gates.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @returns The number of split `LUT6_2` gates on success, an error otherwise.
         */
        Result<u32> split_luts(Netlist* nl);

        /**
         * @brief Split shift register primitives and replaces them with equivalent flip-flops chains.
         * 
         * Currently only implemented for gate type `SRL16E`.
         * 
         * @param[in] nl - The netlist to operate on. 
         * @return The number of split shift registers on success, an error otherwise.
         */
        Result<u32> split_shift_registers(Netlist* nl);

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