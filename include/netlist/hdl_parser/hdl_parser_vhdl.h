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

#include "netlist/module.h"
#include "netlist/net.h"

#include "hdl_parser.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>

/**
 * @ingroup hdl_parsers
 */
class HDL_PARSER_API hdl_parser_vhdl : public hdl_parser
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
    std::shared_ptr<netlist> parse(const std::string& gate_library) override;

private:
    struct instance
    {
        u32 line_number;
        std::string name;
        std::string type;
        std::vector<std::pair<std::string, std::string>> generics;
        std::vector<std::pair<std::string, std::string>> ports;
    };

    struct entity
    {
        std::string name;
        u32 line_number;
        std::vector<std::pair<std::string, std::string>> ports;
        std::unordered_map<std::string, std::vector<std::string>> expanded_signal_names;
        std::unordered_map<std::string, std::set<std::tuple<std::string, std::string, std::string>>> entity_attributes;
        std::unordered_map<std::string, std::set<std::tuple<std::string, std::string, std::string>>> instance_attributes;
        std::unordered_map<std::string, std::set<std::tuple<std::string, std::string, std::string>>> signal_attributes;
        std::vector<std::string> signals;
        std::vector<instance> instances;
        std::unordered_map<std::string, std::string> direct_assignments;
    };

    token_stream m_token_stream;
    std::string m_last_entity;

    std::unordered_set<std::string> m_libraries;
    std::unordered_map<std::string, std::shared_ptr<net>> m_net_by_name;
    std::unordered_map<std::string, u32> m_name_occurrences;
    std::unordered_map<std::string, u32> m_current_instance_index;
    std::unordered_map<std::string, entity> m_entities;
    std::unordered_map<std::string, std::string> m_attribute_types;
    std::unordered_map<std::string, std::vector<std::string>> m_nets_to_merge;

    bool tokenize();
    bool parse_tokens();

    // parse the hdl into an intermediate format
    bool parse_library();
    bool parse_entity_definiton();
    bool parse_port_definiton(entity& e);
    bool parse_architecture();
    bool parse_architecture_header(entity& e);
    bool parse_architecture_body(entity& e);
    bool parse_instance(entity& e);

    bool parse_attribute(std::unordered_map<std::string, std::set<std::tuple<std::string, std::string, std::string>>>& mapping);

    // build the netlist from the intermediate format
    bool build_netlist(const std::string& top_module);
    std::shared_ptr<module> instantiate(const entity& e, std::shared_ptr<module> parent, std::unordered_map<std::string, std::string> port_assignments);

    // helper functions
    std::unordered_map<std::string, std::string> get_assignments(token_stream& lhs, token_stream& rhs);
    std::vector<std::string> get_vector_signals(const std::string& base_name, token_stream& type);
    std::string get_hex_from_number_literal(const std::string& value);
    std::string get_unique_alias(const std::string& name);
};
