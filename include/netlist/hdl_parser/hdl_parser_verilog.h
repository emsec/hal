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

#include "core/token_stream.h"
#include "def.h"
#include "hdl_parser.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

/**
 * @ingroup hdl_parsers
 */
class HDL_PARSER_API hdl_parser_verilog : public hdl_parser<std::string>
{
public:
    /**
     * @param[in] stream - The string stream filled with the hdl code.
     */
    explicit hdl_parser_verilog(std::stringstream& stream);

    ~hdl_parser_verilog() = default;

    /**
     * Deserializes a netlist in Verilog format from the internal string stream into a netlist object.
     *
     * @param[in] gate_library - The gate library used in the serialized file.
     * @returns The deserialized netlist.
     */
    bool parse() override;

private:
    token_stream<std::string> m_token_stream;

    bool tokenize();
    bool parse_tokens();

    // parse HDL into intermediate format
    bool parse_entity(std::map<std::string, std::string>& attributes);
    void parse_port_list(std::set<std::string>& port_names);
    bool parse_port_definition(entity& e, const std::set<std::string>& port_names, std::map<std::string, std::string>& attributes);
    bool parse_signal_definition(entity& e, std::map<std::string, std::string>& attributes);
    bool parse_assign(entity& e);
    bool parse_attribute(std::map<std::string, std::string>& attributes);
    bool parse_instance(entity& e, std::map<std::string, std::string>& attributes);
    bool parse_port_assign(entity& e, instance& inst);
    bool parse_generic_assign(instance& inst);

    // helper functions
    void remove_comments(std::string& line, bool& multi_line_comment);
    std::vector<u32> parse_range(token_stream<std::string>& range_str);
    std::map<std::string, signal> parse_signal_list();
    std::optional<std::pair<std::vector<signal>, i32>> get_assignment_signals(entity& e, token_stream<std::string>& signal_str, bool allow_numerics);
    std::string get_bin_from_literal(const token<std::string>& value_token);
    std::string get_hex_from_literal(const token<std::string>& value_token);
    bool is_in_bounds(const std::vector<std::pair<i32, i32>>& bounds, const std::vector<std::pair<i32, i32>>& reference_bounds) const;
};
