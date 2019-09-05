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

    struct module_definition
    {
        std::vector<file_line> ports;
        std::vector<file_line> wires;
        std::vector<file_line> assigns;
        std::vector<file_line> instances;
    };

    struct module
    {
        std::string name;
        u32 line_number;
        module_definition definition;
        std::vector<std::pair<std::string, std::string>> ports;
    };

    std::map<std::string, module> m_modules;

    // prepare hdl
    void remove_comments(std::string& line, bool& multi_line_comment, bool& multi_line_property);

    // parse the hdl into an intermediate format
    bool parse_module(module& m);
    bool parse_ports(module& m);
    bool parse_wires(module& m);
    bool parse_assigns(module& m);
    bool parse_instances(module& m);
};

#endif /* __HAL_HDL_PARSER_VERILOG_H__ */
