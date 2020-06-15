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
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_type/gate_type.h"

#include <sstream>

namespace hal
{
    /* forward declaration*/
    class netlist;

    /**
     * @ingroup netlist
     */
    class NETLIST_API gate_library_parser
    {
    public:
        /**
         * @param[in] stream - The string stream filled with gate library definition.
         */
        explicit gate_library_parser(const std::filesystem::path& file_path, std::stringstream& file_content);

        virtual ~gate_library_parser() = default;

        /**
         * Parses the gate library definition file.
         *
         * @returns The gate library or a nullptr on error.
         */
        virtual std::shared_ptr<gate_library> parse() = 0;

    protected:
        // stores the gate library
        std::shared_ptr<gate_library> m_gate_lib;

        // stores the input stream to the file
        std::stringstream& m_fs;

        // stores the path to the parsed file
        std::filesystem::path m_path;
    };
}    // namespace hal
