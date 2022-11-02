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
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"
#include "hal_core/utilities/special_strings.h"
#include "hal_core/utilities/token_stream.h"

#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace hal
{

    /**
     * @ingroup netlist_parser
     */
    class NETLIST_API VerilogParser : public NetlistParser
    {
    public:
        VerilogParser()  = default;
        ~VerilogParser() = default;

        /**
         * Parse a Verilog netlist into an internal intermediate format.
         *
         * @param[in] file_path - Path to the Verilog netlist file.
         * @returns True on success, false otherwise.
         */
        Result<std::monostate> parse(const std::filesystem::path& file_path) override;

        /**
         * Instantiate the parsed Verilog netlist using the specified gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        Result<std::unique_ptr<Netlist>> instantiate(const GateLibrary* gate_library) override;

    private:
        using identifier_t        = std::string;
        using ranged_identifier_t = std::pair<std::string, std::vector<std::vector<u32>>>;
        using numeral_t           = std::vector<BooleanFunction::Value>;
        using empty_t             = std::monostate;
        using assignment_t        = std::variant<identifier_t, ranged_identifier_t, numeral_t, empty_t>;

        struct VerilogDataEntry
        {
            std::string m_name;
            std::string m_type  = "unknown";
            std::string m_value = "";
        };

        struct VerilogSignal
        {
            std::string m_name;
            std::vector<std::vector<u32>> m_ranges;
            std::vector<VerilogDataEntry> m_attributes;
            std::vector<std::string> m_expanded_names;
        };

        struct VerilogPort
        {
            std::string m_identifier;
            std::string m_expression;
            PinDirection m_direction;
            std::vector<std::vector<u32>> m_ranges;
            std::vector<VerilogDataEntry> m_attributes;
            std::vector<std::string> m_expanded_identifiers;
        };

        struct VerilogPortAssignment
        {
            std::optional<std::string> m_port_name;
            std::vector<assignment_t> m_assignment;
        };

        struct VerilogAssignment
        {
            std::vector<assignment_t> m_variable;
            std::vector<assignment_t> m_assignment;
        };

        struct VerilogInstance
        {
            std::string m_name;
            std::string m_type;
            bool m_is_module = false;
            std::vector<VerilogPortAssignment> m_port_assignments;
            std::vector<VerilogDataEntry> m_parameters;
            std::vector<VerilogDataEntry> m_attributes;
            std::vector<std::pair<std::string, std::string>> m_expanded_port_assignments;
        };

        struct VerilogModule
        {
        public:
            VerilogModule()  = default;
            ~VerilogModule() = default;

            /**
             * Check whether an module is considered smaller than another module.
             *
             * @param[in] other - The module to compare against.
             * @returns True if the module is smaller than 'other', false otherwise.
             */
            bool operator<(const VerilogModule& other) const
            {
                return m_name < other.m_name;
            }

            // module information
            std::string m_name;
            u32 m_line_number;
            std::vector<VerilogDataEntry> m_attributes;    // module attributes

            // ports
            std::vector<std::unique_ptr<VerilogPort>> m_ports;
            std::map<std::string, VerilogPort*> m_ports_by_identifier;
            std::map<std::string, VerilogPort*> m_ports_by_expression;
            std::set<std::string> m_expanded_port_expressions;

            // signals
            std::vector<std::unique_ptr<VerilogSignal>> m_signals;
            std::map<std::string, VerilogSignal*> m_signals_by_name;

            // assignments
            std::vector<VerilogAssignment> m_assignments;
            std::vector<std::pair<std::string, std::string>> m_expanded_assignments;

            // instances
            std::vector<std::unique_ptr<VerilogInstance>> m_instances;
            std::map<std::string, VerilogInstance*> m_instances_by_name;
        };

        std::stringstream m_fs;
        std::filesystem::path m_path;

        // temporary netlist
        Netlist* m_netlist = nullptr;

        // all modules of the netlist
        std::vector<std::unique_ptr<VerilogModule>> m_modules;
        std::unordered_map<std::string, VerilogModule*> m_modules_by_name;
        std::string m_last_module;

        // token stream of entire input file
        TokenStream<std::string> m_token_stream;

        // some caching
        std::unordered_map<std::string, GateType*> m_gate_types;
        std::unordered_map<std::string, GateType*> m_vcc_gate_types;
        std::unordered_map<std::string, GateType*> m_gnd_gate_types;
        std::unordered_map<Net*, std::vector<std::pair<Module*, u32>>> m_module_port_by_net;
        std::unordered_map<Module*, std::vector<std::pair<std::string, Net*>>> m_module_ports;

        // unique aliases
        std::unordered_map<std::string, u32> m_signal_name_occurrences;
        std::unordered_map<std::string, u32> m_instance_name_occurrences;

        // nets
        Net* m_zero_net;
        Net* m_one_net;
        std::unordered_map<std::string, Net*> m_net_by_name;
        std::unordered_map<std::string, std::vector<std::string>> m_nets_to_merge;

        // parse HDL into intermediate format
        void tokenize();
        Result<std::monostate> parse_tokens();
        Result<std::monostate> parse_module(std::vector<VerilogDataEntry>& attributes);
        void parse_port_list(VerilogModule* module);
        Result<std::monostate> parse_port_declaration_list(VerilogModule* module);
        Result<std::monostate> parse_port_definition(VerilogModule* module, std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_signal_definition(VerilogModule* module, std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_assignment(VerilogModule* module);
        Result<std::monostate> parse_defparam(VerilogModule* module);
        void parse_attribute(std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_instance(VerilogModule* module, std::vector<VerilogDataEntry>& attributes);
        Result<std::monostate> parse_port_assign(VerilogInstance* instance);
        Result<std::vector<VerilogDataEntry>> parse_parameter_assign();

        // construct netlist from intermediate format
        Result<std::monostate> construct_netlist(VerilogModule* top_module);
        Result<Module*>
            instantiate_module(const std::string& instance_name, VerilogModule* verilog_module, Module* parent, const std::unordered_map<std::string, std::string>& parent_module_assignments);

        // helper functions
        std::string get_unique_alias(std::unordered_map<std::string, u32>& name_occurrences, const std::string& name) const;
        std::vector<u32> parse_range(TokenStream<std::string>& stream) const;
        void expand_ranges_recursively(std::vector<std::string>& expanded_names, const std::string& current_name, const std::vector<std::vector<u32>>& ranges, u32 dimension) const;
        std::vector<std::string> expand_ranges(const std::string& name, const std::vector<std::vector<u32>>& ranges) const;
        Result<std::vector<BooleanFunction::Value>> get_binary_vector(std::string value) const;
        Result<std::string> get_hex_from_literal(const Token<std::string>& value_token) const;
        Result<std::pair<std::string, std::string>> parse_parameter_value(const Token<std::string>& value_token) const;
        Result<std::vector<VerilogParser::assignment_t>> parse_assignment_expression(TokenStream<std::string>&& stream) const;
        std::vector<std::string> expand_assignment_expression(VerilogModule* verilog_module, const std::vector<assignment_t>& vars) const;
    };
}    // namespace hal
