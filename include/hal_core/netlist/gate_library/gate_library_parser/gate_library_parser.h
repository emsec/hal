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
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/plugin_system/fac_extension_interface.h"

#include <sstream>

namespace hal
{
    /**
     * The gate library parser is the base class that needs to be inherited from all parsers that shall be registered with HAL.
     * 
     * @ingroup gate_lib_parser
     */
    class NETLIST_API GateLibraryParser
    {
    public:
        /**
         * The outcome of the most recent parse as reported by the parser.
         */
        enum class ParseStatus
        {
            NoError,         ///< The gate library was parsed successfully.
            NeedsUpgrade,    ///< The file format is outdated, it gets upgraded by parsing it again after `request_upgrade`.
            UnknownError     ///< Parsing failed for any other reason, or the parser does not report its status.
        };

        /**
         * Sets the parse status to `UnknownError` until the parser reports otherwise.
         */
        GateLibraryParser();

        /**
         * Withdraws a pending upgrade request, so that it applies to a single parse only.
         */
        virtual ~GateLibraryParser();

        /**
         * Parses the gate library definition file.
         *
         * @param[in] file_path - Path to the file containing the gate library definition.
         * @returns The gate library on success, an error otherwise.
         */
        virtual Result<std::unique_ptr<GateLibrary>> parse(const std::filesystem::path& file_path) = 0;

        /**
         * Get the outcome of the most recent parse, e.g., to find out why loading a gate library failed.
         *
         * If loading fails before any parser is involved, e.g., because the file does not exist, the status still refers
         * to an earlier parse.
         *
         * @returns The parse status.
         */
        static ParseStatus get_parse_status();

        /**
         * Allow the next parse to upgrade a gate library file in an outdated format.
         *
         * The upgrade replaces the file, so it must only be requested with the consent of the user. The request applies
         * to the next parse only and is withdrawn once that parse is done.
         */
        static void request_upgrade();

    protected:
        /**
         * Report the outcome of the current parse.
         *
         * @param[in] status - The parse status.
         */
        static void set_parse_status(ParseStatus status);

        /**
         * Check whether the current parse may upgrade a gate library file in an outdated format.
         *
         * @returns `true` if an upgrade was requested, `false` otherwise.
         */
        static bool is_upgrade_requested();
    };
}    // namespace hal
