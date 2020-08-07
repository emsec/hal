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

#include <set>
#include <string>
#include <tuple>

namespace hal
{
    /** forward declaration */
    class Netlist;
    class HDLWriter;

    /**
     * @ingroup hdl_writers
     */
    namespace hdl_writer_manager
    {
        /**
         * Returns the command line interface options of the hdl writer dispatcher
         * @returns The options.
         */
        ProgramOptions get_cli_options();

        /**
         * Registers a new HDL writer for a selection of file types.
         * If writers for some of the extensions already exist, they are not changed, only the new ones are registered.
         *
         * @param[in] writer - The writer to register.
         * @param[in] supported_file_extensions - The file extensions this writer can process.
         */
        NETLIST_API void register_writer(HDLWriter* writer, const std::vector<std::string>& supported_file_extensions);

        /**
         * Unregisters a specific writer.
         *
         * @param[in] writer - The writer to unregister.
         */
        NETLIST_API void unregister_writer(HDLWriter* writer);

        /**
         * Writes the netlist into a file specified in command line options.
         * @param[in] netlist - The netlist.
         * @param[in] args - The command line options.
         * @returns True on success.
         */
        bool write(Netlist* netlist, const ProgramArguments& args);

        /**
         * Writes the netlist into a file with a defined format
         * @param[in] netlist - The netlist.
         * @param[in] file_name - The output file name.
         * @returns True on success.
         */
        bool write(Netlist* netlist, const std::filesystem::path& file_name);

        /**
         * Writes the netlist into a stringstream with a defined format
         * @param[in] netlist - The netlist.
         * @param[in] type_extension - The file extension of the output format
         * @param[out] stream - The output stream.
         * @returns True on success.
         */
        bool write(Netlist* netlist, const std::string& type_extension, std::stringstream& stream);
    }    // namespace hdl_writer_manager
}    // namespace hal
