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
#include <utility>
#include <netlist/net.h>

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

    struct component
    {
        std::string name;
        std::vector<std::pair<std::string, std::string>> generics;
        std::vector<std::pair<std::string, std::string>> ports;
    };

    struct entity
    {
        std::string name;
        entity_definition definition;
        std::vector<std::pair<std::string, std::string>> ports;                                 // name, direction
        std::map<std::string, std::tuple<std::string, std::string, std::string>> attributes;    // map from name to rest
        std::vector<std::string> signals;
        std::vector<component> components;
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

    std::map<std::string, std::string> m_attribute_types;

    bool add_entity_definition(const std::vector<std::tuple<int, std::string>>& entity);

    bool parse_architecture(const std::vector<std::tuple<int, std::string>>& architecture);

    bool parse_instances(const std::vector<std::tuple<int, std::string>>& instances);

    bool parse_instance(std::string instance);

    std::string get_hex_from_number_literal(const std::string& value);

    std::vector<std::string> get_port_assignments(const std::string& line);
    std::vector<std::string> get_vector_signals(const std::string& name, const std::string& type);

    std::map<std::string, entity> m_entities;

    bool parse_header(const std::vector<file_line>& header);
    bool parse_libraries(const std::vector<file_line>& header);

    bool parse_entity(entity& e);
    bool parse_entity_header(entity& e);
    bool parse_architecture_header(entity& e);
    bool parse_architecture_body(entity& e);
};

#endif /* __HAL_HDL_PARSER_VHDL_H__ */
