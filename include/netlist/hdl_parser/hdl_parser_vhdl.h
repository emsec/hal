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
class HDL_PARSER_API hdl_parser_vhdl : public hdl_parser<core_strings::case_insensitive_string>
{
public:
    /**
     * @param[in] stream - The string stream filled with the hdl code.
     */
    explicit hdl_parser_vhdl(std::stringstream& stream);

    ~hdl_parser_vhdl() = default;

    /**
     * Deserializes a netlist in VHDL format from the internal string stream into a netlist object.
     *
     * @param[in] gate_library - The gate library used in the serialized file.
     * @returns The deserialized netlist.
     */
    bool parse() override;

private:
    enum class attribute_target_class
    {
        ENTITY,
        INSTANCE,
        SIGNAL
    };

    using attribute_buffer_t = std::map<attribute_target_class, std::map<core_strings::case_insensitive_string, std::tuple<u32, std::string, std::string, std::string>>>;
    attribute_buffer_t m_attribute_buffer;

    std::set<core_strings::case_insensitive_string> m_libraries;
    std::map<core_strings::case_insensitive_string, core_strings::case_insensitive_string> m_attribute_types;

    token_stream<core_strings::case_insensitive_string> m_token_stream;

    bool tokenize();
    bool parse_tokens();

    // parse HDL into intermediate format
    bool parse_library();
    bool parse_entity();
    bool parse_port_definitons(entity& e);
    bool parse_attribute();
    bool parse_architecture();
    bool parse_architecture_header(entity& e);
    bool parse_signal_definition(entity& e);
    bool parse_architecture_body(entity& e);
    bool parse_assign(entity& e);
    bool parse_instance(entity& e);
    bool parse_port_assign(entity& e, instance& inst);
    bool parse_generic_assign(instance& inst);
    bool assign_attributes(entity& e);

    // helper functions
    std::vector<u32> parse_range(token_stream<core_strings::case_insensitive_string>& range_str);
    std::optional<std::vector<std::vector<u32>>> parse_signal_ranges(token_stream<core_strings::case_insensitive_string>& signal_str);
    std::optional<std::pair<std::vector<signal>, i32>> get_assignment_signals(entity& e, token_stream<core_strings::case_insensitive_string>& signal_str, bool is_left_half, bool is_port_assignment);
    core_strings::case_insensitive_string get_bin_from_literal(const token<core_strings::case_insensitive_string>& value_token);
    core_strings::case_insensitive_string get_hex_from_literal(const token<core_strings::case_insensitive_string>& value_token);
};
