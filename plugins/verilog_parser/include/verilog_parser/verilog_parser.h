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
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"
#include "hal_core/utilities/special_strings.h"
#include "hal_core/utilities/token_stream.h"
#include "verilog_parser/verilog_module.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace hal
{
    /**
     * @ingroup hdl_parsers
     */
    class NETLIST_API NetlistParserVerilog : public NetlistParser
    {
    public:
        NetlistParserVerilog()  = default;
        ~NetlistParserVerilog() = default;

        /**
         * Parse a Verilog netlist into an internal intermediate format.
         *
         * @param[in] file_path - Path to the Verilog netlist file.
         * @returns True on success, false otherwise.
         */
        bool parse(const std::filesystem::path& file_path);

        /**
         * Instantiate the parsed Verilog netlist using the specified gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        std::unique_ptr<Netlist> instantiate(const GateLibrary* gate_library);

    private:
        std::filesystem::path m_path;
        std::stringstream m_fs;

        // temporarily stores the netlist during serialization
        Netlist* m_netlist = nullptr;

        // map of all entities
        std::unordered_map<std::string, VerilogModule> m_modules;
        std::string m_last_module;

        TokenStream<std::string> m_token_stream;

        bool tokenize();
        bool parse_tokens();

        // parse HDL into intermediate format
        bool parse_module(std::map<std::string, std::string>& attributes);
        bool parse_port_list(VerilogModule& module);
        bool parse_port_declaration_list(VerilogModule& module);
        bool parse_port_definition(VerilogModule& module, std::map<std::string, std::string>& attributes);
        bool parse_signal_definition(VerilogModule& module, std::map<std::string, std::string>& attributes);
        bool parse_assignment(VerilogModule& module);
        bool parse_attribute(std::map<std::string, std::string>& attributes);
        bool parse_instance(VerilogModule& module, std::map<std::string, std::string>& attributes);
        bool parse_port_assign(VerilogModule& module, const std::string& instance_name);
        std::vector<std::tuple<std::string, std::string, std::string>> parse_generic_assign();

        // helper functions
        void remove_comments(std::string& line, bool& multi_line_comment);
        std::vector<u32> parse_range(TokenStream<std::string>& range_str);
        // std::optional<std::pair<std::vector<VerilogSignal>, i32>> get_assignment_signals(VerilogModule& module, TokenStream<std::string>& VerilogSignal_str, bool allow_numerics);
        std::string get_bin_from_literal(const Token<std::string>& value_token);
        std::string get_hex_from_literal(const Token<std::string>& value_token);
        bool is_in_bounds(const std::vector<std::pair<i32, i32>>& bounds, const std::vector<std::pair<i32, i32>>& reference_bounds) const;

        // signal expansion
        // std::vector<std::string> expand_binary_signal(const VerilogSignal& s) const;
        // std::vector<std::string> expand_signal(const VerilogSignal& s) const;
        // void expand_signal_recursively(std::vector<std::string>& expanded_VerilogSignal, const std::string& current_VerilogSignal, const std::vector<std::vector<u32>>& ranges, u32 dimension) const;
        // std::vector<std::string> expand_signal_vector(const std::vector<VerilogSignal>& VerilogSignals, bool allow_binary) const;
    };
}    // namespace hal
