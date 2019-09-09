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
#include "hdl_parser/hdl_parser.h"

#include <map>
#include <netlist/module.h>
#include <netlist/net.h>
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
    struct file_line
    {
        u32 number;
        std::string text;
    };

    struct entity_definition
    {
        std::vector<file_line> ports;
        std::vector<file_line> wires;
        std::vector<file_line> assigns;
        std::vector<file_line> instances;
    };

    struct instance
    {
        std::string name;
        std::string type;
        std::vector<std::pair<std::string, std::string>> ports;
        std::vector<std::pair<std::string, std::string>> generics;
    };

    struct entity
    {
        std::string name;
        u32 line_number;
        entity_definition definition;
        std::vector<std::pair<std::string, std::string>> ports;
        std::vector<std::string> signals;
        std::vector<instance> instances;
        std::map<std::string, std::string> direct_assignments;
        std::map<std::string, std::vector<std::string>> expanded_signal_names;
    };

    std::map<std::string, std::shared_ptr<net>> m_net_by_name;
    std::shared_ptr<net> m_zero_net;
    std::shared_ptr<net> m_one_net;
    std::map<std::string, u32> m_name_occurrences;
    std::map<std::string, u32> m_current_instance_index;
    std::map<std::string, entity> m_entities;
    std::map<std::string, std::string> m_attribute_types;
    std::map<std::string, std::vector<std::string>> m_nets_to_merge;

    // parse the hdl into an intermediate format
    bool parse_entity(entity& e);
    bool parse_ports(entity& e);
    bool parse_wires(entity& e);
    bool parse_assigns(entity& e);
    bool parse_instances(entity& e);

    // build the netlist from the intermediate format
    bool build_netlist(const std::string& top_module);
    std::shared_ptr<module> instantiate(const entity& e, std::shared_ptr<module> parent, std::map<std::string, std::string> port_assignments);

    // helper functions
    void remove_comments(std::string& line, bool& multi_line_comment, bool& multi_line_property);
    std::map<std::string, std::vector<std::string>> get_expanded_wire_signals(const std::string& line);
    std::map<std::string, std::string> get_port_assignments(const std::string& port, const std::string& assignment, entity& e);
    std::string get_bin_from_number_literal(const std::string& v);
    std::string get_unique_alias(const std::string& name);
};

#endif /* __HAL_HDL_PARSER_VERILOG_H__ */
