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
 * @file vhdl_elaboration.h
 * @brief The elaboration of a VHDL syntax tree into a netlist IR design.
 */

#pragma once

#include "hal_core/netlist/netlist_ir/netlist_ir.h"
#include "hal_core/utilities/result.h"
#include "vhdl_parser/vhdl_ast.h"

namespace hal
{
    namespace vhdl
    {
        /**
         * Decode an integer literal: `12`, `1_000`, `16#FF#`, `2#1010#E1`, `1e3`.
         *
         * @param[in] text - The literal as written.
         * @returns The value on success, an error for a real or an invalid literal.
         */
        Result<i64> parse_integer(const std::string& text);

        /**
         * Expand a bit string literal to one character per bit, MSB first: `X"AB"` gives `10101011`, `B"01_1"` gives
         * `011`, `D"10"` gives `1010`, `8X"F"` gives `00001111`, `X"Z-"` gives `zzzz----`. Letters other than `0` and
         * `1` are kept in lower case, one per bit of the digit.
         *
         * @param[in] text - The literal as written, including the quotes.
         * @returns The bits on success, an error for an invalid digit or a width that does not hold the digits.
         */
        Result<std::string> expand_bit_string(const std::string& text);

        /**
         * Turn a parsed file into a design: every entity that has an architecture becomes an IR module with its ports
         * and signals expanded to bits; concurrent signal assignments become aliases; instantiations become instances
         * of gates or of the file's modules; generics and attributes become typed values. Basic identifiers resolve
         * without regard to case and keep the spelling of their declaration, extended identifiers are exact. Ranges and
         * generic values are evaluated as constant expressions with the entity's generic defaults and the constants of
         * the file. An entity or component without an architecture in the file is a gate type.
         *
         * @param[in] file - The parsed file.
         * @returns The design on success, an error naming the unit, item and location otherwise.
         */
        Result<netlist_ir::Design> elaborate(const ast::SourceFile& file);
    }    // namespace vhdl
}    // namespace hal
