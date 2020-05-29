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

#pragma once

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_type/gate_type.h"

#include <cassert>
#include <string>
#include <unordered_set>

/**
 * Sequential gate type class containing information about the internals of a specific sequential gate type.
 *
 * @ingroup netlist
 */
class gate_type_sequential : public gate_type
{
public:
    enum class set_reset_behavior
    {
        U = 0,    // not set
        L = 1,
        H = 2,
        N = 3,
        T = 4,
        X = 5
    };

    /**
     * Constructor for a sequential gate type.
     *
     * @param[in] name - The name of the sequential gate type.
     * @param[in] bt - The base type of the sequential gate type.
     */
    gate_type_sequential(const std::string& name, base_type bt);
    ~gate_type_sequential() override = default;

    /**
     * Add an output pin to the collection of output pins that generate their output from the next_state function.
     * The pin has to be declared as an output pin beforehand.
     *
     * @param[in] output_pin - Name of the output pin.
     */
    void add_state_output_pin(std::string pin_name);

    /**
     * Add an output pin to the collection of output pins that generate their output from the inverted next_state function.
     * The pin has to be declared as an output pin beforehand.
     *
     * @param[in] output_pin - Name of the output pin.
     */
    void add_inverted_state_output_pin(std::string pin_name);

    /**
     * Set the behavior that describes the internal state when both set and reset are active.
     * Each may be one of the following:
     *  - U: not specified for this gate type
     *  - L: set internal state to 0
     *  - H: set internal state to 1
     *  - N: keep current internal state
     *  - T: toggle internal state
     *  - X: undefined behavior
     *
     * @param[in] sb1 - The value specifying the behavior for the internal state.
     * @param[in] sb2 - The value specifying the behavior for the inverted internal state.
     */
    void set_set_reset_behavior(set_reset_behavior sb1, set_reset_behavior sb2);

    /**
     * Set the category in which to find the INIT string.
     *
     * @param[in] category - The category as a string.
     */
    void set_init_data_category(const std::string& category);

    /**
     * Set the identifier used to specify the INIT string.
     *
     * @param[in] identifier - The identifier as a string.
     */
    void set_init_data_identifier(const std::string& identifier);

    /**
     * Get the output pins that use the next_state function to generate their output.
     *
     * @returns The set of output pin names.
     */
    std::unordered_set<std::string> get_state_output_pins() const;

    /**
     * Get the output pins that use the inverted next_state function to generate their output.
     *
     * @returns The set of output pin names.
     */
    std::unordered_set<std::string> get_inverted_state_output_pins() const;

    /**
     * Get the behavior of the internal state and the inverted internal state when both set and reset are active.
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
    std::pair<set_reset_behavior, set_reset_behavior> get_set_reset_behavior() const;

    /**
     * Get the category in which to find the INIT string.
     *
     * @returns The string describing the category.
     */
    std::string get_init_data_category() const;

    /**
     * Get the identifier used to specify the INIT string.
     *
     * @returns The identifier as a string.
     */
    std::string get_init_data_identifier() const;

private:
    // set of pins that use the internal state or inverted internal state respectively as output
    std::unordered_set<std::string> m_state_pins, m_inverted_state_pins;

    // behavior when both set and reset are active
    std::pair<set_reset_behavior, set_reset_behavior> m_set_reset_behavior;

    std::string m_init_data_category;
    std::string m_init_data_identifier;
};
