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
#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "netlist/gate_library/gate_type/gate_type.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"

#include <optional>
#include <unordered_map>

namespace hal
{
    /**
     * @ingroup netlist
     */
    class NETLIST_API gate_library_parser_liberty : public gate_library_parser
    {
    public:
        /**
         * @param[in] stream - The string stream filled with gate library definition.
         */
        explicit gate_library_parser_liberty(const std::filesystem::path& file_path, std::stringstream& file_content);

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
        enum class pin_direction
        {
            UNKNOWN,
            IN,
            OUT,
            INOUT
        };

        struct type_group
        {
            u32 line_number;
            std::string name;
            std::vector<u32> range;
        };

        struct pin_group
        {
            u32 line_number;
            std::vector<std::string> pin_names;
            pin_direction direction = pin_direction::UNKNOWN;
            std::string function;
            std::string x_function;
            std::string z_function;
        };

        struct bus_group
        {
            u32 line_number;
            std::string name;
            pin_direction direction = pin_direction::UNKNOWN;
            std::vector<std::string> pin_names;
            std::vector<pin_group> pins;
            std::map<u32, std::string> index_to_pin_name;
        };

        struct ff_group
        {
            u32 line_number;
            std::string state1, state2;
            std::string clocked_on;
            std::string next_state;
            std::string clear;
            std::string preset;
            gate_type_sequential::set_reset_behavior special_behavior_var1 = gate_type_sequential::set_reset_behavior::U;
            gate_type_sequential::set_reset_behavior special_behavior_var2 = gate_type_sequential::set_reset_behavior::U;
            std::string data_category;
            std::string data_identifier;
        };

        struct latch_group
        {
            u32 line_number;
            std::string state1, state2;
            std::string enable;
            std::string data_in;
            std::string clear;
            std::string preset;
            gate_type_sequential::set_reset_behavior special_behavior_var1 = gate_type_sequential::set_reset_behavior::U;
            gate_type_sequential::set_reset_behavior special_behavior_var2 = gate_type_sequential::set_reset_behavior::U;
        };

        struct lut_group
        {
            u32 line_number;
            std::string name;
            std::string data_category;
            std::string data_identifier;
            std::string data_direction;
        };

        struct cell_group
        {
            u32 line_number;
            std::string name;
            gate_type::base_type type = gate_type::base_type::combinatorial;
            std::map<std::string, bus_group> buses;
            ff_group ff;
            latch_group latch;
            lut_group lut;
            std::vector<pin_group> pins;
            std::set<std::string> pin_names;
            std::map<std::string, std::string> special_functions;
        };

        TokenStream<std::string> m_token_stream;
        std::map<std::string, type_group> m_bus_types;
        std::set<std::string> m_cell_names;

        bool tokenize();
        bool parse_tokens();

        std::optional<cell_group> parse_cell(TokenStream<std::string>& library_stream);
        std::optional<type_group> parse_type(TokenStream<std::string>& str);
        std::optional<pin_group> parse_pin(TokenStream<std::string>& str, cell_group& cell, pin_direction direction = pin_direction::UNKNOWN, const std::string& external_pin_name = "");
        std::optional<bus_group> parse_bus(TokenStream<std::string>& str, cell_group& cell);
        std::optional<ff_group> parse_ff(TokenStream<std::string>& str);
        std::optional<latch_group> parse_latch(TokenStream<std::string>& str);
        std::optional<lut_group> parse_lut(TokenStream<std::string>& str);
        std::shared_ptr<gate_type> construct_gate_type(cell_group& cell);

        void remove_comments(std::string& line, bool& multi_line_comment);
        std::vector<std::string> tokenize_function(const std::string& function);
        std::map<std::string, std::string> expand_bus_function(const std::map<std::string, bus_group>& buses, const std::vector<std::string>& pin_names, const std::string& function);
        std::string prepare_pin_function(const std::map<std::string, bus_group>& buses, const std::string& function);
        std::map<std::string, boolean_function> construct_bus_functions(const cell_group& cell, const std::vector<std::string>& input_pins);
    };
}    // namespace hal
