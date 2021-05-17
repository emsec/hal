//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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
#include "hal_core/netlist/netlist.h"

#include <filesystem>

namespace hal
{
    /* forward declaration*/
    class GateLibrary;

    /**
     * @ingroup netlist_parser
     */
    class NETLIST_API NetlistParser
    {
    public:
        NetlistParser()          = default;
        virtual ~NetlistParser() = default;

        /**
         * Parse a netlist into an internal intermediate format.
         *
         * @param[in] file_path - Path to the netlist file.
         * @returns True on success, false otherwise.
         */
        virtual bool parse(const std::filesystem::path& file_path) = 0;

        /**
         * Instantiate the parsed netlist using the specified gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        virtual std::unique_ptr<Netlist> instantiate(const GateLibrary* gate_library) = 0;

        /**
         * Parse and instantiate a netlist using the specified gate library.
         *
         * @param[in] file_path - Path to the netlist file.
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        std::unique_ptr<Netlist> parse_and_instantiate(const std::filesystem::path& file_path, const GateLibrary* gate_library)
        {
            if (parse(file_path))
            {
                return instantiate(gate_library);
            }
            return nullptr;
        }
    };
}    // namespace hal
