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
    namespace hdl_parser_manager
    {
        /**
         * Returns the command line interface options of the hdl parser manager
         *
         * @returns The options.
         */
        NETLIST_API ProgramOptions get_cli_options();

        /**
         * Registers a new HDL parser for a selection of file types.
         * If parsers for some of the extensions already exist, they are not changed, only the new ones are registered.
         *
         * @param[in] parser - The parser to register.
         * @param[in] supported_file_extensions - The file extensions this parser can process.
         */
        NETLIST_API void register_parser(HDLParser* parser, const std::vector<std::string>& supported_file_extensions);

        /**
         * Unregisters a specific parser.
         *
         * @param[in] parser - The parser to unregister.
         */
        NETLIST_API void unregister_parser(HDLParser* parser);

        /**
         * Returns the netlist for the file and specified command line options.
         *
         * @param[in] file_name - The input file.
         * @param[in] args - The command line options.
         * @returns The netlist representation of the hdl code or a nullpointer on error.
         */
        NETLIST_API std::shared_ptr<Netlist> parse(const std::filesystem::path& file_name, const ProgramArguments& args);

        /**
         * Returns the netlist for a file, parsed with a defined gate library.
         *
         * @param[in] file_name - The input file.
         * @param[in] gate_library - The gate library used in the file.
         * @returns The netlist representation of the hdl code or a nullpointer on error.
         */
        NETLIST_API std::shared_ptr<Netlist> parse(const std::filesystem::path& file_name, const std::shared_ptr<GateLibrary>& gate_library);
    }    // namespace hdl_parser_manager
}    // namespace hal
