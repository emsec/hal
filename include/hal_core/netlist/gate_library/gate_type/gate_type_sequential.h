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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type.h"

#include <cassert>
#include <string>
#include <unordered_set>

namespace hal
{
    /**
     * A sequential gate type contains information about its internals such as input and output pins as well as its Boolean functions.<br>
     * In addition to the standard gate type functionality, it provides mechanisms to specify outputs that depend on the internal state of the sequential gate as well as clock pins.
     *
     * @ingroup gate_lib
     */
    class GateTypeSequential : public GateType
    {
    public:
        /**
         * Defines the behavior of the gate type in case both set and reset are active at the same time.
         */
        enum class SetResetBehavior
        {
            U = 0, /**< Default value when no behavior is specified. **/
            L = 1, /**< Set the internal state to \p 0. **/
            H = 2, /**< Set the internal state to \p 1. **/
            N = 3, /**< Do not change the internal state. **/
            T = 4, /**< Toggle, i.e., invert the internal state. **/
            X = 5  /**< Set the internal state to \p X. **/
        };

        /**
         * Construct a new LUT gate type by specifying its name and base type.
         * The base type must be either \p ff or \p latch .
         *
         * @param[in] name - The name of the sequential gate type.
         * @param[in] base_type - The base type of the sequential gate type.
         */
        GateTypeSequential(const std::string& name, BaseType base_type);
        ~GateTypeSequential() override = default;

        /**
         * Add an existing output pin to the collection of output pins that generate their output from the internal state of the gate.<br>
         * The pin has to be declared as an output pin beforehand.
         *
         * @param[in] pin_name - The name of the output pin to add.
         */
        void add_state_output_pin(const std::string& pin_name);

        /**
         * Get the output pins that use the internal state of the gate to generate their output.
         *
         * @returns The set of output pin names.
         */
        std::unordered_set<std::string> get_state_output_pins() const;

        /**
         * Add an existing output pin to the collection of output pins that generate their output from the inverted internal state of the gate.<br>
         * The pin has to be declared as an output pin beforehand.
         *
         * @param[in] pin_name - The name of the output pin to add.
         */
        void add_inverted_state_output_pin(const std::string& pin_name);

        /**
         * Get the output pins that use the inverted internal state of the gate to generate their output.
         *
         * @returns The set of output pin names.
         */
        std::unordered_set<std::string> get_inverted_state_output_pins() const;

        /**
         * Add an existing input pin to the collection of input pins that are connected to a clock signal.<br>
         * The pin has to be declared as an input pin beforehand.
         *
         * @param[in] pin_name - The name of the input pin to add.
         */
        void add_clock_pin(const std::string& pin_name);

        /**
         * Get the input pins that that are connected to a clock signal.
         *
         * @returns The set of input pin names.
         */
        std::unordered_set<std::string> get_clock_pins() const;

        /**
         * Set the behavior that describes the internal state when both set and reset are active at the same time.
         *
         * @param[in] sb1 - The value specifying the behavior for the internal state.
         * @param[in] sb2 - The value specifying the behavior for the inverted internal state.
         */
        void set_set_reset_behavior(SetResetBehavior sb1, SetResetBehavior sb2);

        /**
         * Get the behavior of the internal state and the inverted internal state when both set and reset are active at the same time.
         *
         * @returns The values specifying the behavior for the internal and inverted internal state.
         */
        std::pair<SetResetBehavior, SetResetBehavior> get_set_reset_behavior() const;

        /**
         * Set the data category in which to find the initialization value.
         *
         * @param[in] category - The data category.
         */
        void set_init_data_category(const std::string& category);

        /**
         * Get the data category in which to find the initialization value.
         *
         * @returns The data category.
         */
        std::string get_init_data_category() const;

        /**
         * Set the data identifier used to specify the initialization value.
         *
         * @param[in] identifier - The data identifier.
         */
        void set_init_data_identifier(const std::string& identifier);

        /**
         * Get the data identifier used to specify the initialization value.
         *
         * @returns The data identifier.
         */
        std::string get_init_data_identifier() const;

    private:
        // set of pins that use the internal state or inverted internal state respectively as output
        std::unordered_set<std::string> m_state_pins;
        std::unordered_set<std::string> m_inverted_state_pins;
        std::unordered_set<std::string> m_clock_pins;

        // behavior when both set and reset are active
        std::pair<SetResetBehavior, SetResetBehavior> m_set_reset_behavior;

        std::string m_init_data_category;
        std::string m_init_data_identifier;
    };
}    // namespace hal
