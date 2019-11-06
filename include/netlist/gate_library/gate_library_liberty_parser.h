//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "pragma_once.h"

#ifndef __HAL_GATE_LIBRARY_LIBERTY_PARSER_H
#define __HAL_GATE_LIBRARY_LIBERTY_PARSER_H

#include "def.h"

#include "netlist/boolean_function.h"

class bdd;
class gate_library;

namespace gate_library_liberty_parser
{
    /*
     * Parses a gate library from the liberty file format.
     * In order to also support lookup tables (LUTs) the following extension is allowed:
     * 
     * lut(<function name>) {
     *     data_category = <category>;
     *     data_key = <key>;
     *     direction = <"ascending" or "descending">;
     * }
     * 
     * <category> and <key> refer to the location where the LUT configuration string is stored, for example "generic" and "init".
     * direction describes whether the first configuration bit is the output for inputs 000... (ascending) or 111... (descending).
     * 
     * @param[in] ss - The string stream containing the liberty data.
     * @returns - The parsed gate library.
     */
    std::shared_ptr<gate_library> parse(std::stringstream& ss);

}    // namespace gate_library_liberty_parser

#endif    //__HAL_GATE_LIBRARY_LIBERTY_PARSER_H
