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
 * @file verilog_elaboration.h
 * @brief Turns the syntax tree of a Verilog file into a netlist IR design.
 */

#pragma once

#include "hal_core/netlist/netlist_ir/netlist_ir.h"
#include "hal_core/utilities/result.h"
#include "verilog_parser/verilog_ast.h"

#include <string>

namespace hal
{
    namespace verilog
    {
        /**
         * A Verilog number, decoded.
         */
        struct Number
        {
            bool sized     = false; /**< The width was written, as in `4'b1010`. */
            bool based     = false; /**< A base was written; a plain decimal has none. */
            bool is_signed = false;
            bool is_real   = false; /**< `1.5` or `1e3`; `bits` is then empty. */
            u32 width      = 0;     /**< The number of bits, the written width or the width of the digits. */
            std::string bits;       /**< One character per bit, MSB first: `0`, `1`, `x`, or `z`. */
            std::string text;       /**< The number as written. */

            /**
             * Check whether every bit is `0` or `1`.
             */
            bool is_defined() const;

            /**
             * The value as an unsigned integer, if it has no `x` or `z` bit and at most 64 bits.
             */
            Result<u64> to_u64() const;

            /**
             * The bits as a hexadecimal string with `0x` prefix and without leading zeros, e.g. `0xAB`; only when `is_defined()`.
             */
            std::string to_hex() const;
        };

        /**
         * Decode a Verilog number as the lexer gives it.
         *
         * @param[in] text - `12`, `4'b10_1x`, `'hff`, `'0`, `1.5`, ...
         * @returns The number on success, an error for an invalid digit or a width that does not hold the digits.
         */
        Result<Number> parse_number(const std::string& text);

        /**
         * Turn a parsed file into a design: every module becomes an IR module with its ports and signals expanded to
         * bits; header port expressions, assignments, `wire a = b` initializers and `supply0`/`supply1` nets become
         * aliases; instantiations become instances of gates or of the file's modules; parameters and attributes become
         * typed values. Ranges and parameter values are evaluated as constant integer expressions with the module's
         * own parameter defaults. A module marked `(* top = 1 *)` becomes the design's top.
         *
         * @param[in] file - The parsed file.
         * @returns The design on success, an error naming the module, item and location otherwise.
         */
        Result<netlist_ir::Design> elaborate(const ast::SourceFile& file);
    }    // namespace verilog
}    // namespace hal
