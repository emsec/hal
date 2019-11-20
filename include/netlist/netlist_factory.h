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

/* forward declaration */
class netlist;

/**
 * @file
 */

/**
 * \namespace netlist_factory
 * @ingroup netlist
 */
namespace netlist_factory
{
    /**
     * Creates a new netlist for a specific gate library.
     *
     * @param[in] gate_library_name - Name of hardware gate library.
     * @returns The new netlist.
     */
    NETLIST_API std::shared_ptr<netlist> create_netlist(const std::string& gate_library_name);

    /**
     * Creates a new netlist for a specific file.
     *
     * @param[in] hdl_file - Name of the hdl file.
     * @param[in] language - Programming language used in \p file_name.
     * @param[in] gate_library_name - Name of hardware gate library.
     * @returns The new netlist.
     */
    NETLIST_API std::shared_ptr<netlist> load_netlist(const hal::path& hdl_file, const std::string& language, const std::string& gate_library_name);

    /**
     * Creates a new netlist for a specific '.hal' file.
     *
     * @param[in] hal_file - Name of the '.hal' file.
     * file.
     * @returns The new netlist.
     */
    NETLIST_API std::shared_ptr<netlist> load_netlist(const hal::path& hal_file);

    /**
     * Creates a new netlist entirely from program options.<br>
     * Invokes parsers or serializers as needed.
     *
     * @param[in] args - Command line options.
     * @returns The new netlist.
     */
    NETLIST_API std::shared_ptr<netlist> load_netlist(const program_arguments& args);
}    // namespace netlist_factory
