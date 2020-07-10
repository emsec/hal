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

#include "core/program_options.h"
#include "def.h"
#include "netlist/hdl_parser/hdl_parser.h"

#include <set>
#include <string>
#include <tuple>

namespace hal
{
    /** forward declaration */
    class Netlist;

    /**
     * @ingroup hdl_parsers
     */
    namespace HDLParserDispatcher
    {
        /**
         * Returns the command line interface options of the hdl parser dispatcher
         *
         * @returns The options.
         */
        ProgramOptions get_cli_options();

        /**
         * Returns the command language options for the parse() function.
         *
         * @returns The language options.
         */
        std::set<std::string> get_gui_option();

        /**
         * Returns the netlist for the file and specified command line options.
         *
         * @param[in] hdl_file - The netlist file.
         * @param[in] args - The command line options.
         * @returns The netlist representation of the hdl code or a nullpointer on error.
         */
        std::shared_ptr<Netlist> parse(const std::filesystem::path& hdl_file, const ProgramArguments& args);

        /**
         * Returns the netlist for a file, parsed with a defined parser_name and gate library.
         *
         * @param[in] hdl_file - The netlist file.
         * @param[in] parser_name - The name of the parser to use, e.g. vhdl, verilog...
         * @param[in] gate_library - The gate library used in the file.
         * @returns The netlist representation of the hdl code or a nullpointer on error.
         */
        std::shared_ptr<Netlist> parse(const std::filesystem::path& hdl_file, std::string parser_name = "", const std::shared_ptr<GateLibrary>& gate_library = nullptr);

        /**
         * Tries to match the HDL file with each of the preloaded gate libraries and returns a vector of netlist objects for which the matching was successful.
         *
         * @param[in] hdl_file - The netlist file.
         * @param[in] parser_name - The name of the parser to use, e.g. vhdl, verilog...
         * @returns A vector containing one netlist per matching gate library.
         */
        std::vector<std::shared_ptr<Netlist>> parse_all(const std::filesystem::path& hdl_file, std::string parser_name = "");
    }    // namespace HDLParserDispatcher
}    // namespace hal
