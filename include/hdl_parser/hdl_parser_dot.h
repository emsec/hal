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
#ifndef __HAL_HDL_PARSER_DOT_H__
#define __HAL_HDL_PARSER_DOT_H__

#include "def.h"
#include "hdl_parser/hdl_parser.h"

#include <map>

/* forward declaration */
class net;

/**
 * @ingroup hdl_parsers
 */
class HDL_PARSER_API hdl_parser_dot : public hdl_parser
{
public:
    /**
     * @param[in] stream - The string stream filled with the hdl code.
     */
    explicit hdl_parser_dot(std::stringstream& stream);

    /** Destructor (default) */
    ~hdl_parser_dot() = default;

    /**
     * Deserializes a netlist in dot format from the internal string stream into a netlist object.
     *
     * @param[in] gate_library - The gate library used in the serialized file.
     * @returns The deserialized netlist.
     */
    std::shared_ptr<netlist> parse(const std::string& gate_library) override;

private:
    /** stores the net names of the design */
    std::map<std::string, net*> m_net;

    bool parse_header(const std::string& header);

    bool parse_gates(const std::string& gates);

    bool parse_edges(const std::string& edges);
};

#endif /* __HAL_HDL_PARSER_DOT_H__ */
