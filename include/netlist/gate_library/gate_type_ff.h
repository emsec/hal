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

#ifndef __HAL_GATE_TYPE_FF_H
#define __HAL_GATE_TYPE_FF_H

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_type.h"

#include <map>
#include <string>

/**
 *  gate type class
 *
 * @ingroup netlist
 */
class gate_type_ff : public gate_type
{
public:
    enum class special_behavior
    {
        L,
        H,
        N,
        T,
        X
    };

    gate_type_ff(const std::string& name);
    ~gate_type_ff() override = default;

    void set_next_state_function(const boolean_function& next_state_f);
    void set_clock_function(const boolean_function& clock_f);
    void set_set_function(const boolean_function& set_f);
    void set_reset_function(const boolean_function& reset_f);
    void set_output_pin_inverted(const std::string& output_pin, bool inverted);
    void set_special_behavior1(special_behavior sb);
    void set_special_behavior2(special_behavior sb);

    boolean_function get_next_state_function() const;
    boolean_function get_clock_function() const;
    boolean_function get_set_function() const;
    boolean_function get_reset_function() const;
    std::set<std::string> get_inverted_output_pins() const;
    std::pair<special_behavior, special_behavior> get_special_behavior() const;

private:
    bool doCompare(const gate_type& other) const;

    boolean_function m_next_state_f;
    boolean_function m_clock_f;
    boolean_function m_set_f;
    boolean_function m_reset_f;
    std::set<std::string> m_inverted_output_pins;
    std::pair<special_behavior, special_behavior> m_special_behavior;
};
#endif    //__HAL_GATE_TYPE_LUT_H
