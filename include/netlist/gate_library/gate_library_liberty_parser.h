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

#ifndef __HAL_GATE_LIBRARY_LIBERTY_PARSER_H
#define __HAL_GATE_LIBRARY_LIBERTY_PARSER_H

#include "def.h"

namespace gate_library_liberty_parser
{
    struct statement
    {
        statement(std::weak_ptr<statement> p_parent, bool p_is_group, std::string p_name, std::string p_value) : parent(p_parent), is_group(p_is_group), name(p_name), value(p_value)
        {
        }

        std::weak_ptr<statement> parent;
        bool is_group;
        std::string name;
        std::string value;
        std::vector<std::shared_ptr<statement>> statements;
    };

    struct pin
    {
        pin(std::string p_name) : name(p_name)
        {
        }

        std::string name;
        std::string direction;
        std::string function;
        std::string three_state;
        std::string x_function;
    };

    struct cell
    {
        cell(std::string p_name) : name(p_name)
        {
        }

        std::string name;
        bool is_ff    = false;
        bool is_latch = false;
        std::vector<pin> pins;
        std::string clocked_on;
        std::string enable;
        std::string next_state;
        std::string data_in;
        std::string clear;
        std::string preset;
        std::string clear_preset_var1;
        std::string clear_preset_var2;
    };

    struct intermediate_library
    {
        std::string name;
        std::vector<cell> cells;
    };

    std::shared_ptr<gate_library> parse(std::stringstream& ss);
    std::shared_ptr<statement> get_statements(std::stringstream& ss);
    std::shared_ptr<intermediate_library> get_intermediate_library(std::shared_ptr<statement> root);

    std::shared_ptr<gate_library> get_gate_library(std::shared_ptr<intermediate_library> inter_lib);

    void remove_comments(std::string& line, bool& multi_line_comment);

}    // namespace gate_library_liberty_parser

#endif    //__HAL_GATE_LIBRARY_LIBERTY_PARSER_H