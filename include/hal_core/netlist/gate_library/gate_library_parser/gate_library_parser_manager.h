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
     * @ingroup gate_lib_parser
     */
    namespace gate_library_parser_manager
    {
        using ParserFactory = std::function<std::unique_ptr<GateLibraryParser>()>;

        /**
         * Registers a new gate library parser for a selection of file types.
         * If parsers for some of the extensions already exist, they are not changed, only the new ones are registered.
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
         * Loads a gate library file.
         *
         * @param[in] path - the file to load.
         * @param[in] reload_if_existing - If true, reloads all libraries that are already loaded.
         * @returns Pointer to the loaded gate library or nullptr on error.
         */
        NETLIST_API GateLibrary* load_file(std::filesystem::path path, bool reload_if_existing = false);

        /**
         * Loads all gate libraries which are available.
         *
         * @param[in] reload_if_existing - If true, reloads all libraries that are already loaded.
         */
        NETLIST_API void load_all(bool reload_if_existing = false);

        /**
         * Get a gate library object by file name.
         * If no library with the given name is already loaded this function will attempt to load the file.
         *
         * @param[in] file_name - file name of the gate library.
         * @returns Pointer to the gate library object or nullptr on error.
         */
        NETLIST_API GateLibrary* get_gate_library(const std::string& file_name);

        /**
         * Get a gate library object by library name.
         * If no library with the given name is loaded this function returns nullptr.
         *
         * @param[in] lib_name - name of the gate library.
         * @returns Pointer to the gate library object or nullptr on error.
         */
        NETLIST_API GateLibrary* get_gate_library_by_name(const std::string& lib_name);

        /**
         * Get all loaded gate libraries.
         *
         * @returns A vector of pointers to the gate library objects.
         */
        NETLIST_API std::vector<GateLibrary*> get_gate_libraries();

    }    // namespace gate_library_parser_manager
}    // namespace hal
