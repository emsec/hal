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
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/utilities/token_stream.h"

#include <filesystem>
#include <map>
#include <optional>
#include <unordered_map>

namespace hal
{
    /**
     * @ingroup netlist
     */
    class NETLIST_API LibertyParser : public GateLibraryParser
    {
    public:
        LibertyParser()  = default;
        ~LibertyParser() = default;

        /**
         * Deserializes a gate library in Liberty format from the string stream into a gate library object.
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
         * @param[in] file_path - Path to the file containing the gate library definition.
         * @returns The gate library on success, an error otherwise.
         */
        Result<std::unique_ptr<GateLibrary>> parse(const std::filesystem::path& file_path) override;

    private:
        struct type_group
        {
            u32 line_number;
            std::string name;
            bool ascending  = false;
            u32 start_index = 0;
            u32 width;
        };

        struct pin_group
        {
            u32 line_number;
            std::vector<std::string> pin_names;
            PinDirection direction = PinDirection::none;
            PinType type           = PinType::none;
            std::string function;
            std::string x_function;
            std::string z_function;
            bool clock  = false;
            bool power  = false;
            bool ground = false;
        };

        struct bus_group
        {
            u32 line_number;
            std::string name;
            PinDirection direction = PinDirection::none;
            std::vector<std::string> pin_names;
            std::vector<pin_group> pins;
            bool ascending  = false;
            u32 start_index = 0;
            std::unordered_map<u32, std::string> index_to_pin;
        };

        struct ff_group
        {
            u32 line_number;
            std::string state1, state2;
            std::string clocked_on;
            std::string next_state;
            std::string clear;
            std::string preset;
            AsyncSetResetBehavior special_behavior_var1 = AsyncSetResetBehavior::undef;
            AsyncSetResetBehavior special_behavior_var2 = AsyncSetResetBehavior::undef;
        };

        struct latch_group
        {
            u32 line_number;
            std::string state1, state2;
            std::string enable;
            std::string data_in;
            std::string clear;
            std::string preset;
            AsyncSetResetBehavior special_behavior_var1 = AsyncSetResetBehavior::undef;
            AsyncSetResetBehavior special_behavior_var2 = AsyncSetResetBehavior::undef;
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
            std::set<GateTypeProperty> properties;
            std::optional<ff_group> ff;
            std::optional<latch_group> latch;
            std::optional<lut_group> lut;
            std::vector<pin_group> pins;
            std::map<std::string, bus_group> buses;
            std::set<std::string> pin_names;
        };

        std::unique_ptr<GateLibrary> m_gate_lib;
        std::stringstream m_fs;
        std::filesystem::path m_path;

        TokenStream<std::string> m_token_stream;
        std::map<std::string, type_group> m_bus_types;
        std::set<std::string> m_cell_names;

        void tokenize();
        Result<std::monostate> parse_tokens();

        Result<cell_group> parse_cell(TokenStream<std::string>& library_stream);
        Result<type_group> parse_type(TokenStream<std::string>& str);
        Result<pin_group> parse_pin(TokenStream<std::string>& str, cell_group& cell, PinDirection direction = PinDirection::none, const std::string& external_pin_name = "");
        Result<pin_group> parse_pg_pin(TokenStream<std::string>& str, cell_group& cell);
        Result<bus_group> parse_bus(TokenStream<std::string>& str, cell_group& cell);
        Result<ff_group> parse_ff(TokenStream<std::string>& str);
        Result<latch_group> parse_latch(TokenStream<std::string>& str);
        Result<std::monostate> construct_gate_type(cell_group&& cell);

        void remove_comments(std::string& line, bool& multi_line_comment);
        std::vector<std::string> tokenize_function(const std::string& function);
        std::map<std::string, std::string> expand_bus_function(const std::map<std::string, bus_group>& buses, const std::vector<std::string>& pin_names, const std::string& function);
        std::string prepare_pin_function(const std::map<std::string, bus_group>& buses, const std::string& function);
        Result<std::unordered_map<std::string, BooleanFunction>> construct_bus_functions(const cell_group& cell);
    };
}    // namespace hal
