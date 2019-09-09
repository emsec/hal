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
#ifndef __HAL_HDL_PARSER_VERILOG_OLD_H__
#define __HAL_HDL_PARSER_VERILOG_OLD_H__

#include "def.h"
#include "hdl_parser/hdl_parser.h"
#include "netlist/net.h"
#include <map>

/**
 * @ingroup hdl_parsers
 */
class HDL_PARSER_API hdl_parser_verilog_old : public hdl_parser
{
public:
    /**
     * @param[in] stream - The string stream filled with the hdl code.
     */
    explicit hdl_parser_verilog_old(std::stringstream& stream);

    /** destructor (= default) */
    ~hdl_parser_verilog_old() = default;

    /**
     * Deserializes a netlist in Verilog format from the internal string stream into a netlist object.
     *
     * @param[in] gate_library - The gate library used in the serialized file.
     * @returns The deserialized netlist.
     */
    std::shared_ptr<netlist> parse(const std::string& gate_library) override;

private:
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

    void remove_comments(std::string& buffer, bool& multiline_comment, bool& multiline_property);

    bool parse_module(const std::string& token, const int line);

    bool parse_architecture(const std::string& token, const std::string& identifier, const int line);

    bool parse_instance(const std::string& token, const std::string& identifier, const int line);

    bool parse_assign(const std::string& token, const int line);

    std::vector<std::string> parse_net(const std::string& token, const int line);

    std::vector<std::string> parse_net_single(const std::string& token, const int line);

    std::vector<std::string> parse_pin(std::shared_ptr<gate>& new_gate, const std::string& token, const int line);

    std::vector<int> get_vector_bounds(const std::string& s);

    int get_idx_of_last_vector_bound(const std::string& s);

    std::string get_hex_from_number_literal(const std::string& value);

    std::vector<std::string> get_binary_string_from_number_literal(const std::string& value, const int line);

    bool connect_net_to_pin(const std::string& net_name, std::shared_ptr<gate>& new_gate, const std::string& pin_name, const int line);

    bool is_numeric(const std::string& token);
};

#endif /* __HAL_HDL_PARSER_VERILOG_OLD_H__ */
