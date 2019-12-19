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

#pragma once

#include "def.h"

#include "core/program_options.h"

/** forward declaration */
class netlist;
class hdl_writer;

#include <set>
#include <string>
#include <tuple>

/**
 * @ingroup hdl_writers
 */
namespace hdl_writer_dispatcher
{
    /**
     * Returns the command line interface options of the hdl writer dispatcher
     * @returns The options.
     */
    program_options get_cli_options();

    /**
     * Writes the netlist into a file specified in command line options.
     * @param[in] g - The netlist.
     * @param[in] args - The command line options.
     * @returns True on success.
     */
    bool write(std::shared_ptr<netlist> g, const program_arguments& args);

    /**
     * Writes the netlist into a file with a defined format
     * @param[in] g - The netlist.
     * @param[in] format - The target format of the file, e.g. vhdl, verilog...
     * @param[in] file_name - The input file.
     * @returns True on success.
     */
    bool write(std::shared_ptr<netlist> g, const std::string& format, const hal::path& file_name);
}    // namespace hdl_writer_dispatcher
