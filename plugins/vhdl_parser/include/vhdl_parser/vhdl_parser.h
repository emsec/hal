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


/**
 * @file vhdl_parser.h
 * @brief The VHDL netlist parser built on the netlist IR.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/netlist_ir/instantiate.h"
#include "hal_core/netlist/netlist_ir/netlist_ir.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"

#include <filesystem>
#include <optional>

namespace hal
{
    namespace vhdl
    {
        /**
         * Read a structural VHDL file into a design: lexing, parsing and elaboration.
         *
         * @param[in] file - The path of the file.
         * @returns The design on success, an error otherwise.
         */
        Result<netlist_ir::Design> parse_to_ir(const std::filesystem::path& file);

        /**
         * Read structural VHDL text into a design.
         *
         * @param[in] text - The text.
         * @param[in] file - The path to report in messages.
         * @returns The design on success, an error otherwise.
         */
        Result<netlist_ir::Design> parse_text_to_ir(const std::string& text, const std::filesystem::path& file = "<string>");
    }    // namespace vhdl

    /**
     * @class VHDLParser
     * @brief The VHDL netlist parser: reads a file into a netlist IR design and instantiates it against a gate library.
     */
    class NETLIST_API VHDLParser : public NetlistParser
    {
    public:
        VHDLParser()  = default;
        ~VHDLParser() = default;

        /**
         * Parse a VHDL netlist into the netlist IR.
         *
         * @param[in] file_path - Path to the VHDL netlist file.
         * @returns Ok on success, an error otherwise.
         */
        Result<std::monostate> parse(const std::filesystem::path& file_path) override;

        /**
         * Instantiate the parsed design as a netlist using the given gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns The netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> instantiate(const GateLibrary* gate_library) override;

        /**
         * Get the design of the last successful `parse`.
         *
         * @returns The design, or nothing if no file has been parsed.
         */
        const std::optional<netlist_ir::Design>& get_design() const;

        /**
         * Set the options that `instantiate` uses.
         *
         * @param[in] options - The options.
         */
        void set_instantiation_options(const netlist_ir::InstantiationOptions& options);

    private:
        std::optional<netlist_ir::Design> m_design;
        netlist_ir::InstantiationOptions m_options;
    };
}    // namespace hal
