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
#include "netlist/gate_library/gate_type/gate_type.h"

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
    /**
     * Constructor for a latch gate type.
     * 
     * @param[in] name - The name of the latch gate type.
     */
    gate_type_latch(const std::string& name);
    ~gate_type_latch() override = default;

    /**
     * Sets the boolean function describing the data input of the latch.
     * 
     * @param[in] data_in_f - The boolean function.
     */
    void set_data_in_function(const boolean_function& data_in_f);

    /**
     * Sets the boolean function describing the enable input of the latch.
     * 
     * @param[in] enable_f - The boolean function.
     */
    void set_enable_function(const boolean_function& enable_f);

    /**
     * Sets the boolean function describing the set behavior of the latch.
     * 
     * @param[in] set_f - The boolean function.
     */
    void set_set_function(const boolean_function& set_f);

    /**
     * Sets the boolean function describing the reset behavior of the latch.
     * 
     * @param[in] reset_f - The boolean function.
     */
    void set_reset_function(const boolean_function& reset_f);

    /**
     * Sets a flag indicating that the output of the given output pin shall be inverted.
     * Used in combination with the data_in function.
     * 
     * @param[in] output_pin - Name of the output pin.
     * @param[in] inverted - True if inverted, false otherwise.
     */
    void set_output_pin_inverted(const std::string& output_pin, bool inverted);

    /**
     * Sets the behavior that describes the internal state of the latch when both set and reset are active.
     * May be one of the following:
     *  - U: not specified for this gate type
     *  - L: set internal state to 0
     *  - H: set internal state to 1
     *  - N: keep current internal state
     *  - T: toggle internal state
     *  - X: undefined behavior
     * 
     * @param[in] sb - The value specifying the behavior.
     */
    void set_special_behavior1(special_behavior sb);

    /**
     * Sets the behavior that describes the inverted internal state of the latch when both set and reset are active.
     * May be one of the following:
     *  - U: not specified for this gate type
     *  - L: set inverted internal state to 0
     *  - H: set inverted internal state to 1
     *  - N: keep current inverted internal state
     *  - T: toggle inverted internal state
     *  - X: undefined behavior
     * 
     * @param[in] sb - The value specifying the behavior.
     */
    void set_special_behavior2(special_behavior sb);

    /**
     * Returns the boolean function describing the data input of the latch.
     * 
     * @returns The boolean function.
     */
    boolean_function get_data_in_function() const;

    /**
     * Returns the boolean function describing the enable input of the latch.
     * 
     * @returns The boolean function.
     */
    boolean_function get_enable_function() const;

    /**
     * Returns the boolean function describing the set behavior of the latch.
     * 
     * @returns The boolean function.
     */
    boolean_function get_set_function() const;

    /**
     * Returns the boolean function describing the reset behavior of the latch.
     * 
     * @returns The boolean function.
     */
    boolean_function get_reset_function() const;

    /**
     * Returns a set of output pin names that shall use the inverted value of the data_in function.
     * 
     * @returns The set containing the output pin names.
     */
    std::set<std::string> get_inverted_output_pins() const;

    /**
     * Returns the behavior of the internal state and the inverted internal state when both set and reset are active.
     * May be one of the following:
     *  - U: not specified for this gate type
     *  - L: set (inverted) internal state to 0
     *  - H: set (inverted) internal state to 1
     *  - N: keep current (inverted) internal state
     *  - T: toggle (inverted) internal state
     *  - X: undefined behavior
     * 
     * @returns The boolean function.
     */
    std::pair<special_behavior, special_behavior> get_special_behavior() const;

private:
    bool doCompare(const gate_type& other) const;

    boolean_function m_data_in_f;
    boolean_function m_enable_f;
    boolean_function m_set_f;
    boolean_function m_reset_f;
    std::set<std::string> m_inverted_output_pins;
    std::pair<special_behavior, special_behavior> m_special_behavior;
};
#endif    //__HAL_GATE_TYPE_LATCH_H
