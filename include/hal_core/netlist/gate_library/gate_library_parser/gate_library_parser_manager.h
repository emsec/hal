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

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace hal
{
    class GateLibrary;

    /**
     * TODO Python binding & documentation
     * @ingroup gate_lib_parser
     */
    namespace gate_library_parser_manager
    {
        using ParserFactory = std::function<std::unique_ptr<GateLibraryParser>()>;

        /**
         * Registers a new gate library parser for a selection of file types.<br>
         * If parsers for some of the extensions already exist, they remain changed and only new ones are registered.
         *
         * @param[in] name - The name of the parser.
         * @param[in] parser_factory - A factory function that constructs a new parser instance.
         * @param[in] supported_file_extensions - The file extensions this parser can process.
         */
        NETLIST_API void register_parser(const std::string& name, const ParserFactory& parser_factory, const std::vector<std::string>& supported_file_extensions);

        /**
         * Unregisters a specific parser.
         *
         * @param[in] name - The name of the parser.
         */
        NETLIST_API void unregister_parser(const std::string& name);

        /**
         * Parses the gate library file depending on its file extension.
         * 
         * @param[in] file_path - The input path.
         * @returns The gate library.
         */
        NETLIST_API std::unique_ptr<GateLibrary> parse(std::filesystem::path file_path);
    }    // namespace gate_library_parser_manager
}    // namespace hal
