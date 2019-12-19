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

#include "core/token_stream.h"

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "netlist/gate_library/gate_type/gate_type.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"

#include <unordered_map>

/**
 * @ingroup netlist
 */
class NETLIST_API gate_library_parser_liberty : public gate_library_parser
{
public:
    /**
     * @param[in] stream - The string stream filled with gate library definition.
     */
    explicit gate_library_parser_liberty(std::stringstream& stream);

    ~gate_library_parser_liberty() = default;

    /**
     * Deserializes a gate library in Liberty format from the internal string stream into a gate library object.
     * In order to also support lookup tables (LUTs) the following extension is allowed:
     *
     * lut(<function name>) {
     *     data_category = <category>;
     *     data_identifier = <identifier>;
     *     direction = <"ascending" or "descending">;
     * }
     *
     * <category> and <identifier> refer to the location where the LUT configuration string is stored, for example "generic" and "init".
     * direction describes whether the least significant bit of the configuration is the output for inputs 000... (ascending) or 111... (descending).
     *
     * @returns The deserialized gate library.
     */
    std::shared_ptr<gate_library> parse() override;

private:
    struct cell
    {
        std::string name;
        gate_type::base_type type;
        std::vector<std::string> input_pins, output_pins;
        std::unordered_map<std::string, token> functions;
        std::string next_state, clocked_on, reset, set;
        gate_type_sequential::set_reset_behavior special_behavior_var1, special_behavior_var2;
        std::string data_category, data_identifier, data_direction;
        std::string state1, state2;

        void clear()
        {
            name = "";
            type = gate_type::base_type::combinatorial;
            input_pins.clear();
            output_pins.clear();
            functions.clear();
            next_state            = "";
            clocked_on            = "";
            reset                 = "";
            set                   = "";
            special_behavior_var1 = gate_type_sequential::set_reset_behavior::U;
            special_behavior_var2 = gate_type_sequential::set_reset_behavior::U;
            data_category         = "";
            data_identifier       = "";
            data_direction        = "";
            state1                = "";
            state2                = "";
        }
    } m_current_cell;

    token_stream m_token_stream;

    bool tokenize();
    bool parse_tokens();

    bool parse_cell(token_stream& library_stream);
    bool parse_pin(token_stream& cell_stream);
    bool parse_ff(token_stream& cell_stream);
    bool parse_latch(token_stream& cell_stream);
    bool parse_lut(token_stream& cell_stream);
    std::shared_ptr<gate_type> construct_gate_type();

    void remove_comments(std::string& line, bool& multi_line_comment);
};
