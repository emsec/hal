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
#ifndef __HAL_HDL_PARSER_VHDL_H__
#define __HAL_HDL_PARSER_VHDL_H__

#include "def.h"
#include "hdl_parser/hdl_parser.h"

#include <map>
#include <netlist/net.h>
#include <netlist/module.h>
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
    struct file_line
    {
        u32 number;
        std::string text;
    };

    struct entity_definition
    {
        std::vector<file_line> entity_header;
        std::vector<file_line> architecture_header;
        std::vector<file_line> architecture_body;
    };

    struct instance
    {
        std::string name;
        std::string type;
        std::vector<std::pair<std::string, std::string>> generics;
        std::vector<std::pair<std::string, std::string>> ports;

        std::shared_ptr<module> netlist_module;
    };

    struct entity
    {
        std::string name;
        u32 line_number;
        entity_definition definition;
        std::vector<std::pair<std::string, std::string>> ports;
        std::map<std::string, std::vector<std::string>> expanded_signal_names;
        std::map<std::string, std::set<std::tuple<std::string, std::string, std::string>>> entity_attributes;
        std::map<std::string, std::set<std::tuple<std::string, std::string, std::string>>> instance_attributes;
        std::map<std::string, std::set<std::tuple<std::string, std::string, std::string>>> signal_attributes;
        std::vector<std::string> signals;
        std::vector<instance> instances;
        std::map<std::string, std::string> direct_assignments;
    };

    /** libraries used */
    std::set<std::string> m_libraries;

    /** stores the architecture name of the design */
    std::string m_architecture_name;

    /** stores the net names of the design */
    std::map<std::string, std::shared_ptr<net>> m_net;

    /** stores the global input net names */
    std::set<std::string> m_global_input_net;

    /** stores the global output net names */
    std::set<std::string> m_global_output_net;

    /** stores the net names that have to be replace (keyword: assign) */
    std::map<std::string, std::string> m_replace_net_name;

    std::string get_hex_from_number_literal(const std::string& value);

    std::map<std::string, std::string> get_port_assignments(const std::string& port, const std::string& assignment);
    std::vector<std::string> get_vector_signals(const std::string& name, const std::string& type);

    std::map<std::string, entity> m_entities;
    std::map<std::string, std::string> m_attribute_types;

    bool parse_header(const std::vector<file_line>& header);
    bool parse_libraries(const std::vector<file_line>& header);
    bool parse_components(const std::vector<std::vector<file_line>>& components);

    bool parse_entity(entity& e);
    void parse_attribute(std::map<std::string, std::set<std::tuple<std::string, std::string, std::string>>>& mapping, const file_line& line);
    bool parse_entity_header(entity& e);
    bool parse_architecture_header(entity& e);
    bool parse_architecture_body(entity& e);
    bool parse_instance(entity& e, const std::vector<file_line>& lines);

    std::map<std::string, u32> m_name_instances;
    std::map<std::string, u32> m_instance_count;

    bool build_netlist(const std::string& top_module);
    std::shared_ptr<module> instantiate(const entity& e, std::shared_ptr<module> parent, const std::map<std::string, std::string>& port_assignments);

    std::string get_unique_alias(const entity& e, const std::string& name);

    std::shared_ptr<net> m_zero_net;
    std::shared_ptr<net> m_one_net;
};

#endif /* __HAL_HDL_PARSER_VHDL_H__ */
