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

#ifndef __HAL_GATE_TYPE_LATCH_H
#define __HAL_GATE_TYPE_LATCH_H

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_type.h"

#include <map>
#include <string>

/**
 *  gate type class
 *
 * @ingroup netlist
 */
class gate_type_latch : public gate_type
{
public:
    enum special_behavior_t
    {
        L,
        H,
        N,
        T,
        X
    };

    using gate_type::gate_type;

    void set_enable_function(boolean_function enable_f);
    void set_set_function(boolean_function set_f);
    void set_reset_function(boolean_function reset_f);
    void set_output_pin_inverted(std::string output_pin, bool inverted);
    void set_special_behavior(std::pair<special_behavior_t, special_behavior_t> sb);

    boolean_function get_enable_function() const;
    boolean_function get_set_function() const;
    boolean_function get_reset_function() const;
    std::map<std::string, bool> get_output_pin_inverted() const;
    std::pair<special_behavior_t, special_behavior_t> get_special_behavior() const;

private:
    bool doCompare(const gate_type& other) const;

    boolean_function m_enable_f;
    boolean_function m_set_f;
    boolean_function m_reset_f;
    std::map<std::string, bool> m_output_pin_inverted;
    std::pair<special_behavior_t, special_behavior_t> m_special_behavior;
};
#endif    //__HAL_GATE_TYPE_LUT_H
