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
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"
#include "hal_core/utilities/special_strings.h"
#include "hal_core/utilities/token_stream.h"
#include "vhdl_parser/vhdl_entity.h"

#include <optional>
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
        bool parse(const std::filesystem::path& file_path) override;

        /**
         * Instantiate the parsed VHDL netlist using the specified gate library.
         *
         * @param[in] gate_library - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        std::unique_ptr<Netlist> instantiate(const GateLibrary* gate_library) override;

    private:
        enum class AttributeTarget
        {
            ENTITY,
            INSTANCE,
            SIGNAL
        };

        using ci_string          = core_strings::CaseInsensitiveString;
        using attribute_buffer_t = std::map<AttributeTarget, std::map<ci_string, std::tuple<u32, std::string, std::string, std::string>>>;

        std::stringstream m_fs;

        // temporary netlist
        Netlist* m_netlist = nullptr;

        // all modules of the netlist
        std::unordered_map<ci_string, VHDLEntity> m_entities;
        ci_string m_last_entity;

        // token stream of entire input file
        TokenStream<ci_string> m_token_stream;

        // some caching
        std::unordered_map<ci_string, GateType*> m_gate_types;
        std::unordered_map<ci_string, GateType*> m_vcc_gate_types;
        std::unordered_map<ci_string, GateType*> m_gnd_gate_types;
        std::unordered_map<Net*, std::tuple<PinDirection, std::string, Module*>> m_module_ports;
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
        std::unordered_map<ci_string, std::vector<ci_string>> m_nets_to_merge;

        // parse HDL into intermediate format
        bool tokenize();
        bool parse_tokens();
        bool parse_library();
        bool parse_entity();
        bool parse_port_definitons(VHDLEntity& entity);
        bool parse_attribute();
        bool parse_architecture();
        bool parse_architecture_header(VHDLEntity& entity);
        bool parse_signal_definition(VHDLEntity& entity);
        bool parse_architecture_body(VHDLEntity& entity);
        bool parse_assignment(VHDLEntity& entity);
        bool parse_instance(VHDLEntity& entity);
        bool parse_port_assign(VHDLEntity& entity, const ci_string& instance_name);
        bool parse_generic_assign(VHDLEntity& entity, const ci_string& instance_name);
        bool assign_attributes(VHDLEntity& entity);

        // construct netlist from intermediate format
        bool construct_netlist(VHDLEntity& top_entity);
        Module* instantiate_entity(const ci_string& instance_name, VHDLEntity& vhdl_entity, Module* parent, const std::unordered_map<ci_string, ci_string>& parent_module_assignments);

        // helper functions
        std::vector<u32> parse_range(TokenStream<ci_string>& range_stream) const;
        std::optional<std::vector<std::vector<u32>>> parse_signal_ranges(TokenStream<ci_string>& signal_stream) const;

        std::vector<ci_string> get_bin_from_literal(const Token<ci_string>& value_token) const;
        ci_string get_hex_from_literal(const Token<ci_string>& value_token) const;
        bool is_in_bounds(const std::vector<std::pair<i32, i32>>& bounds, const std::vector<std::pair<i32, i32>>& reference_bounds) const;
        std::vector<ci_string> expand_assignment_signal(VHDLEntity& entity, TokenStream<ci_string>& signal_stream, bool is_left);
        std::vector<ci_string> expand_ranges(const ci_string& name, const std::vector<std::vector<u32>>& ranges) const;
        void expand_ranges_recursively(std::vector<ci_string>& expanded_names, const ci_string& current_name, const std::vector<std::vector<u32>>& ranges, u32 dimension) const;
        ci_string get_unique_alias(std::unordered_map<ci_string, u32>& name_occurrences, const ci_string& name) const;
    };
}    // namespace hal
