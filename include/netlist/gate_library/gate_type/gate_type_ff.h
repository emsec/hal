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
#include "netlist/gate_library/gate_type/gate_type.h"

#include <string>
#include <unordered_set>

/**
 *  gate type class
 *
 * @ingroup netlist
 */
class gate_type_ff : public gate_type
{
public:
    /**
     * Constructor for a flipflop gate type.
     * 
     * @param[in] name - The name of the flipflop gate type.
     */
    gate_type_ff(const std::string& name);
    ~gate_type_ff() override = default;

    /**
     * Adds an output pin to the collection of output pins that generate their output from the next_state function.
     * 
     * @param[in] output_pin_name - Name of the output pin.
     */
    void add_state_output_pin(std::string output_pin_name);

    /**
     * Adds an output pin to the collection of output pins that generate their output from the inverted next_state function.
     * 
     * @param[in] output_pin_name - Name of the output pin.
     */
    void add_inverted_state_output_pin(std::string output_pin_name);

    /**
     * Sets the behavior that describes the internal state of the flipflop when both set and reset are active.
     * Each may be one of the following:
     *  - U: not specified for this gate type
     *  - L: set internal state to 0
     *  - H: set internal state to 1
     *  - N: keep current internal state
     *  - T: toggle internal state
     *  - X: undefined behavior
     * 
     * @param[in] sb1 - The value specifying the behavior for the internal state.
     * @param[in] sb2 - The value specifying the behaviorfor the inverted internal state.
     */
    void set_special_behavior(special_behavior sb1, special_behavior sb2);

    /**
     * Describes in what part of the gate definition to find the INIT string, e.g., "generic".
     * 
     * @param[in] category - The category as a string.
     */
    void set_data_category(const std::string& category);

    /**
     * Describes the identifier used to specify the INIT string.
     * 
     * @param[in] identifier - The identifier as a string.
     */
    void set_data_identifier(const std::string& identifier);

    /**
     * Describes the bit-order of the INIT string.
     * 
     * @param[in] ascending - True if ascending bit-order, false otherwise.
     */
    void set_data_ascending_order(bool ascending);

    /**
     * Returns the output pins that use the next_state function to generate their output.
     * 
     * @returns The set of output pin names.
     */
    std::unordered_set<std::string> get_state_output_pins() const;

    /**
     * Returns the output pins that use the inverted next_state function to generate their output.
     * 
     * @returns The set of output pin names.
     */
    std::unordered_set<std::string> get_inverted_state_output_pins() const;

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

    /**
     * Returns the string describing in what part of the gate definition to find the INIT string, e.g., "generic".
     * 
     * @returns The string describing the category.
     */
    std::string get_data_category() const;

    /**
     * Returns the string describing the identifier used to specify the INIT string.
     * 
     * @returns The identifier as a string.
     */
    std::string get_data_identifier() const;

    /**
     * Returns the bit-order of the INIT string.
     * 
     * @returns True if ascending bit-order, false otherwise.
     */
    bool is_ascending_order() const;

private:
    bool doCompare(const gate_type& other) const;

    // set of pins that use the internal state or inverted internal state respectively as output
    std::unordered_set<std::string> m_state_pins, m_inverted_state_pins;

    // behavior when both set and reset are active
    std::pair<special_behavior, special_behavior> m_special_behavior;

    std::string m_data_category;
    std::string m_data_identifier;
    bool m_ascending;
};
#endif    //__HAL_GATE_TYPE_FF_H
