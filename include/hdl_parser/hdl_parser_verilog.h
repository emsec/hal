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

#include "pragma_once.h"
#ifndef __HAL_HDL_PARSER_VERILOG_H__
#define __HAL_HDL_PARSER_VERILOG_H__

#include "def.h"

#include "core/token_stream.h"

#include "netlist/module.h"
#include "netlist/net.h"

#include "hdl_parser/hdl_parser.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>

/**
 * @ingroup hdl_parsers
 */
class HDL_PARSER_API hdl_parser_verilog : public hdl_parser
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
    std::shared_ptr<netlist> parse(const std::string& gate_library) override;

private:
    struct instance
    {
        std::string name;
        std::string type;
        std::vector<std::pair<token_stream, token_stream>> port_streams;
        std::vector<std::pair<token_stream, token_stream>> generic_streams;
        std::vector<std::pair<std::string, std::string>> ports;
        std::vector<std::pair<std::string, std::string>> generics;
    };

    struct entity
    {
        std::string name;
        u32 line_number;
        std::unordered_set<std::string> port_names;
        std::unordered_map<std::string, std::pair<std::string, std::vector<std::string>>> ports_expanded;
        std::vector<std::string> signals_expanded;
        std::vector<instance> instances;
        std::unordered_map<std::string, std::string> direct_assignments;
        std::unordered_map<std::string, std::vector<std::string>> expanded_signal_names;
    };

    token_stream m_token_stream;
    std::string m_last_entity;
    std::unordered_map<std::string, std::vector<std::string>> m_gate_to_pin_map;

    std::map<std::string, std::shared_ptr<net>> m_net_by_name;
    std::shared_ptr<net> m_zero_net;
    std::shared_ptr<net> m_one_net;
    std::unordered_map<std::string, u32> m_name_occurrences;
    std::unordered_map<std::string, u32> m_current_instance_index;
    std::unordered_map<std::string, entity> m_entities;
    std::unordered_map<std::string, std::vector<std::string>> m_nets_to_merge;

    bool tokenize();
    bool parse_tokens();

    // parse the hdl into an intermediate format
    bool parse_entity_definiton();
    bool parse_port_list(entity& e);
    bool parse_port_definition(entity& e);
    bool parse_signal_definition(entity& e);
    bool parse_assign(entity& e);
    bool parse_instance(entity& e);
    bool connect_instances();

    // build the netlist from the intermediate format
    bool build_netlist(const std::string& top_module);
    std::shared_ptr<module> instantiate(const entity& e, std::shared_ptr<module> parent, std::unordered_map<std::string, std::string> port_assignments);

    // helper functions
    void remove_comments(std::string& line, bool& multi_line_comment, bool& multi_line_property);
    void expand_signal(std::vector<std::string>& expanded_signal, std::string current_signal, std::vector<std::pair<i32, i32>> bounds, u32 dimension);
    std::unordered_map<std::string, std::vector<std::string>> get_expanded_signals(token_stream& signal_str);
    std::vector<std::string> get_assignment_signals(token_stream& signal_str, entity& e);
    std::vector<std::string> get_port_signals(token_stream& port_str, const std::string& instance_type);
    std::string get_number_from_literal(const std::string& v, const u32 base);
    std::string get_unique_alias(const std::string& name);
};

#endif /* __HAL_HDL_PARSER_VERILOG_H__ */
