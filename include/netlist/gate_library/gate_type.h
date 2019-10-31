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

#ifndef __HAL_GATE_TYPE_H
#define __HAL_GATE_TYPE_H

#include "netlist/boolean_function.h"

#include <map>
#include <string>

/**
 *  gate type class
 *
 * @ingroup netlist
 */
class gate_type
{
public:
    enum base_type_t
    {
        combinatorial,
        lut,
        ff,
        latch
    };

    gate_type(const std::string& name, base_type_t base_type);

    void add_input_pins(const std::vector<std::string>& input_pins);
    void add_output_pins(const std::vector<std::string>& output_pins);
    void add_boolean_function_map(const std::map<std::string, boolean_function>& boolean_function_map);

    std::string get_name() const;
    std::vector<std::string> get_input_pins() const;
    std::vector<std::string> get_output_pins() const;
    const boolean_function& get_boolean_function(const std::string& name) const;

private:
    std::string m_name;
    base_type_t m_base_type;

    std::vector<std::string> m_input_pins;
    std::vector<std::string> m_output_pins;

    std::map<std::string, boolean_function> m_boolean_function_map;
};
#endif    //__HAL_GATE_TYPE_H
