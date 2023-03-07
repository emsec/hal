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
#include "hal_core/utilities/result.h"
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
    class NETLIST_API VHDLParser : public NetlistParser
    {
    public:
        VHDLParser()  = default;
        ~VHDLParser() = default;

        /**
         * Parse a VHDL netlist into an internal intermediate format.
         *
         * @param[in] file_path - Path to the VHDL netlist file.
         * @returns True on success, false otherwise.
         */
        Result<std::monostate> parse(const std::filesystem::path& file_path) override;

        /**
         * Instantiate the parsed VHDL netlist using the specified gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        Result<std::unique_ptr<Netlist>> instantiate(const GateLibrary* gate_library) override;

    private:
        using ci_string           = core_strings::CaseInsensitiveString;
        using identifier_t        = ci_string;
        using ranged_identifier_t = std::pair<ci_string, std::vector<std::vector<u32>>>;
        using numeral_t           = std::vector<BooleanFunction::Value>;
        using empty_t             = std::monostate;
        using assignment_t        = std::variant<identifier_t, ranged_identifier_t, numeral_t, empty_t>;

        struct VhdlDataEntry
        {
            u32 m_line_number;
            std::string m_name;
            std::string m_type  = "unknown";
            std::string m_value = "";
        };

        struct VhdlSignal
        {
            ci_string m_name;
            std::vector<std::vector<u32>> m_ranges;
            std::vector<VhdlDataEntry> m_attributes;
            std::vector<ci_string> m_expanded_names;
        };

        struct VhdlPort
        {
            ci_string m_identifier;
            PinDirection m_direction;
            std::vector<std::vector<u32>> m_ranges;
            std::vector<ci_string> m_expanded_identifiers;
        };

        struct VhdlPortAssignment
        {
            std::optional<assignment_t> m_port;
            std::vector<assignment_t> m_assignment;
        };

        struct VhdlAssignment
        {
            std::vector<assignment_t> m_variable;
            std::vector<assignment_t> m_assignment;
        };

        struct VhdlInstance
        {
            ci_string m_name;
            ci_string m_type;
            bool m_is_entity = false;
            std::vector<VhdlPortAssignment> m_port_assignments;
            std::vector<VhdlDataEntry> m_generics;
            std::vector<VhdlDataEntry> m_attributes;
            std::vector<std::pair<ci_string, ci_string>> m_expanded_port_assignments;
        };

        struct VhdlEntity
        {
        public:
            VhdlEntity()  = default;
            ~VhdlEntity() = default;

            /**
         * Check whether an module is considered smaller than another module.
         *
         * @param[in] other - The module to compare against.
         * @returns True if the module is smaller than 'other', false otherwise.
         */
            bool operator<(const VhdlEntity& other) const
            {
                return m_name < other.m_name;
            }

            // module information
            ci_string m_name;
            u32 m_line_number;
            std::vector<VhdlDataEntry> m_attributes;    // module attributes

            // ports
            std::vector<std::unique_ptr<VhdlPort>> m_ports;
            std::map<ci_string, VhdlPort*> m_ports_by_identifier;
            std::set<ci_string> m_expanded_port_identifiers;

            // signals
            std::vector<std::unique_ptr<VhdlSignal>> m_signals;
            std::map<ci_string, VhdlSignal*> m_signals_by_name;

            // assignments
            std::vector<VhdlAssignment> m_assignments;
            std::vector<std::pair<ci_string, ci_string>> m_expanded_assignments;

            // instances
            std::vector<std::unique_ptr<VhdlInstance>> m_instances;
            std::map<ci_string, VhdlInstance*> m_instances_by_name;
        };

        enum class AttributeTarget
        {
            ENTITY,
            INSTANCE,
            SIGNAL
        };

        using attribute_buffer_t = std::map<AttributeTarget, std::map<ci_string, VhdlDataEntry>>;

        std::stringstream m_fs;
        std::filesystem::path m_path;

        // temporary netlist
        Netlist* m_netlist = nullptr;

        // all entities of the netlist
        std::vector<std::unique_ptr<VhdlEntity>> m_entities;
        std::unordered_map<ci_string, VhdlEntity*> m_entities_by_name;
        ci_string m_last_entity;

        // std::unordered_map<ci_string, VhdlEntity> m_entities;
        // ci_string m_last_entity;

        // token stream of entire input file
        TokenStream<ci_string> m_token_stream;

        // some caching
        std::unordered_map<ci_string, GateType*> m_gate_types;
        std::unordered_map<ci_string, GateType*> m_vcc_gate_types;
        std::unordered_map<ci_string, GateType*> m_gnd_gate_types;
        std::unordered_map<Net*, std::vector<std::pair<Module*, u32>>> m_module_port_by_net;
        std::unordered_map<Module*, std::vector<std::pair<std::string, Net*>>> m_module_ports;
        attribute_buffer_t m_attribute_buffer;
        std::unordered_map<ci_string, ci_string> m_attribute_types;

        // library prefixes
        std::set<ci_string> m_libraries;

        // unique aliases
        std::unordered_map<ci_string, u32> m_signal_name_occurrences;
        std::unordered_map<ci_string, u32> m_instance_name_occurrences;

        // nets
        Net* m_zero_net;
        Net* m_one_net;
        std::unordered_map<ci_string, Net*> m_net_by_name;
        std::vector<std::pair<ci_string, ci_string>> m_nets_to_merge;

        // parse HDL into intermediate format
        void tokenize();
        Result<std::monostate> parse_tokens();
        void parse_library();
        Result<std::monostate> parse_entity();
        Result<std::monostate> parse_port_definitons(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_attribute();
        Result<std::monostate> parse_architecture();
        Result<std::monostate> parse_architecture_header(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_signal_definition(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_architecture_body(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_assignment(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_instance(VhdlEntity* vhdl_entity);
        Result<std::monostate> parse_port_assign(VhdlInstance* instance);
        Result<std::monostate> parse_generic_assign(VhdlInstance* instance);
        Result<std::monostate> assign_attributes(VhdlEntity* vhdl_entity);

        // construct netlist from intermediate format
        Result<std::monostate> construct_netlist(VhdlEntity* top_entity);
        Result<Module*> instantiate_entity(const ci_string& instance_name, VhdlEntity* vhdl_entity, Module* parent, const std::unordered_map<ci_string, ci_string>& parent_module_assignments);

        // helper functions
        ci_string get_unique_alias(std::unordered_map<ci_string, u32>& name_occurrences, const ci_string& name) const;
        std::vector<u32> parse_range(TokenStream<ci_string>& range_stream) const;
        Result<std::vector<std::vector<u32>>> parse_signal_ranges(TokenStream<ci_string>& signal_stream) const;
        void expand_ranges_recursively(std::vector<ci_string>& expanded_names, const ci_string& current_name, const std::vector<std::vector<u32>>& ranges, u32 dimension) const;
        std::vector<ci_string> expand_ranges(const ci_string& name, const std::vector<std::vector<u32>>& ranges) const;
        Result<std::vector<BooleanFunction::Value>> get_binary_vector(std::string value) const;
        Result<std::string> get_hex_from_literal(const Token<ci_string>& value_token) const;
        Result<std::vector<assignment_t>> parse_assignment_expression(TokenStream<ci_string>&& stream) const;
        Result<std::vector<ci_string>> expand_assignment_expression(VhdlEntity* vhdl_entity, const std::vector<assignment_t>& vars) const;
    };
}    // namespace hal
