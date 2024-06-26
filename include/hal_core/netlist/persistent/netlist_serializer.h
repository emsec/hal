// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"

namespace hal
{
    /* forward declaration */
    class Netlist;

    class GateLibrary;

    /**
     * @file
     *
     * \namespace netlist_serializer
     * @ingroup persistent
     */

    namespace netlist_serializer
    {

        /**
         * Serializes a netlist into a `.hal` file.
         *
         * @param[in] netlist - The netlist to serialize.
         * @param[in] hal_file - The path to the `.hal` file.
         * @returns `true` on success, `false` otherwise.
         */
        NETLIST_API bool serialize_to_file(const Netlist* netlist, const std::filesystem::path& hal_file);

        /**
         * Deserializes a netlist from a `.hal` file using the provided gate library.
         * If no gate library is provided, a gate library path must be specified within the `.hal` file.
         *
         * @param[in] hal_file - The path to the `.hal` file.
         * @param[in] gate_lib - The gate library. Defaults to a `nullptr`.
         * @returns The deserialized netlist on success, a `nullptr` otherwise.
         */
        NETLIST_API std::unique_ptr<Netlist> deserialize_from_file(const std::filesystem::path& hal_file, GateLibrary* gate_lib = nullptr);

        /**
         * Deserializes a string which contains a netlist in HAL-(JSON)-format using the provided gate library.
         * If no gate library is provided, a gate library path must be specified within the string.
         *
         * @param[in] hal_string - The string containing the netlist in HAL-(JSON)-format.
         * @param[in] gate_lib - The gate library. Defaults to a `nullptr`.
         * @returns The deserialized netlist on success, a `nullptr` otherwise.
         */
        NETLIST_API std::unique_ptr<Netlist> deserialize_from_string(const std::string& hal_string, GateLibrary* gate_lib = nullptr);
    }    // namespace netlist_serializer
}    // namespace hal
