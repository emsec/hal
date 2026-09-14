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
 * @file verilog_parser.h
 * @brief The Verilog netlist parser: a file goes through the preprocessor, the lexer, the syntax parser and the
 * elaboration into a netlist IR design, which is then instantiated against a gate library.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/netlist_ir/instantiate.h"
#include "hal_core/netlist/netlist_ir/netlist_ir.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"
#include "hal_core/utilities/result.h"

#include <filesystem>
#include <optional>

namespace hal
{
    namespace verilog
    {
        /**
         * Read a structural Verilog file into a netlist IR design.
         *
         * @param[in] file - The path of the file.
         * @returns The design on success, an error with the location of the problem otherwise.
         */
        Result<netlist_ir::Design> parse_to_ir(const std::filesystem::path& file);

        /**
         * Read structural Verilog text into a netlist IR design.
         *
         * @param[in] text - The text.
         * @param[in] file - The path to report in messages and to resolve `include against.
         * @returns The design on success, an error with the location of the problem otherwise.
         */
        Result<netlist_ir::Design> parse_text_to_ir(const std::string& text, const std::filesystem::path& file = "<string>");
    }    // namespace verilog

    /**
     * The Verilog netlist parser.
     *
     * @ingroup netlist_parser
     */
    class NETLIST_API VerilogParser : public NetlistParser
    {
    public:
        VerilogParser()  = default;
        ~VerilogParser() = default;

        /**
         * Parse a Verilog file into the netlist IR.
         *
         * @param[in] file_path - Path to the Verilog file.
         * @returns Ok on success, an error otherwise.
         */
        Result<std::monostate> parse(const std::filesystem::path& file_path) override;

        /**
         * Instantiate the parsed design against the given gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns The netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> instantiate(const GateLibrary* gate_library) override;

        /**
         * Get the design the last successful `parse` produced.
         *
         * @returns The design, or `std::nullopt` if nothing was parsed.
         */
        const std::optional<netlist_ir::Design>& get_design() const;

        /**
         * Set the options for the instantiation; the defaults are what a netlist parser should do.
         *
         * @param[in] options - The options.
         */
        void set_instantiation_options(const netlist_ir::InstantiationOptions& options);

    private:
        std::optional<netlist_ir::Design> m_design;
        netlist_ir::InstantiationOptions m_options;
    };
}    // namespace hal
