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

#include <map>
#include <string>
#include <unordered_map>

namespace hal
{
    /**
     * A gate type contains information about its internals such as input and output pins as well as its Boolean functions.
     *
     * @ingroup gate_lib
     */
    class NETLIST_API GateType
    {
    public:
        /**
         * Represents the base type of a gate type.
         */
        enum class BaseType
        {
            combinatorial, /**< Represents a combinatorial gate type. **/
            lut,           /**< Represents a combinatorial LUT gate type. **/
            ff,            /**< Represents a sequential FF gate type. **/
            latch          /**< Represents a sequential latch gate type. **/
        };

        /**
         * Construct a new gate type by specifying its name.
         *
         * @param[in] name - The name of the gate type.
         */
        GateType(const std::string& name);
        virtual ~GateType() = default;

        /**
         * Get the unique ID of the gate type.
         *
         * @returns The unique ID of the gate type.
         */
        u32 get_id() const;

        /**
         * Get the name of the gate type.
         *
         * @returns The name of the gate type.
         */
        std::string get_name() const;

        /**
         * Get the base type of the gate type, which can be either combinatorial, lut, ff, or latch.
         *
         * @returns The base type of the gate type.
         */
        BaseType get_base_type() const;

        /**
         * Get a string describing the given gate type object.
         *
         * @returns A string describing the gate type.
         */
        std::string to_string() const;

        /**
         * Insert the gate type object to an output stream.
         *
         * @param[in] os - The output stream to insert the gate type object into.
         * @param[in] gt - The gate type object.
         * @returns An output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, const GateType& gt);

        /**
         * Check whether two gate types are equal.
         *
         * @param[in] other - The gate type to compare against.
         * @returns True if both gate types are equal, false otherwise.
         */
        bool operator==(const GateType& other) const;

        /**
         * Check whether two gate types are unequal.
         *
         * @param[in] other - The gate type object to compare to.
         * @returns True if both gate types are unequal, false otherwise.
         */
        bool operator!=(const GateType& other) const;

        /**
         * Add an input pin to the gate type.
         *
         * @param[in] pin_name - The name of the input pin to add.
         */
        void add_input_pin(std::string pin_name);

        /**
         * Add a vector of input pins to the gate type.
         *
         * @param[in] pin_names - The vector of names of input pins to add.
         */
        void add_input_pins(const std::vector<std::string>& pin_names);

        /**
         * Get a vector of input pins of the gate type.
         *
         * @returns A vector of input pin names of the gate type.
         */
        std::vector<std::string> get_input_pins() const;

        /**
         * Add an output pin to the gate type.
         *
         * @param[in] pin_name - The name of the output pin to add.
         */
        void add_output_pin(std::string pin_name);

        /**
         * Add a vector of output pins to the gate type.
         *
         * @param[in] pin_names - The vector of names of output pins to add.
         */
        void add_output_pins(const std::vector<std::string>& pin_names);

        /**
         * Get a vector of output pins of the gate type.
         *
         * @returns A vector of output pins of the gate type.
         */
        std::vector<std::string> get_output_pins() const;

        /**
         * Assign existing input pins to a input pin group.
         *
         * @param[in] group_name - The name of the input pin group.
         * @param[in] index_to_pin - A map from pin index to input pin name.
         */
        void assign_input_pin_group(const std::string& group_name, const std::map<u32, std::string>& index_to_pin);

        /**
         * Assign existing input pins to multiple input pin groups.
         *
         * @param[in] pin_groups - A map from pin group names to a map from pin indices to pin names.
         */
        void assign_input_pin_groups(const std::unordered_map<std::string, std::map<u32, std::string>>& pin_groups);

        /**
         * Get all input pin groups of the gate type.
         *
         * @returns A map from pin group names to a map from pin indices to pin names.
         */
        std::unordered_map<std::string, std::map<u32, std::string>> get_input_pin_groups() const;

        /**
         * Assign existing output pins to a output pin group.
         *
         * @param[in] group_name - The name of the output pin group.
         * @param[in] index_to_pin - A map from pin index to output pin name.
         */
        void assign_output_pin_group(const std::string& group_name, const std::map<u32, std::string>& index_to_pin);

        /**
         * Assign existing output pins to multiple output pin groups.
         *
         * @param[in] pin_groups - A map from pin group names to a map from pin indices to pin names.
         */
        void assign_output_pin_groups(const std::unordered_map<std::string, std::map<u32, std::string>>& pin_groups);

        /**
         * Get all output pin groups of the gate type.
         *
         * @returns A map from pin group names to a map from pin indices to pin names.
         */
        std::unordered_map<std::string, std::map<u32, std::string>> get_output_pin_groups() const;

        /**
         * Add a Boolean function with the specified name to the gate type.
         *
         * @param[in] name - The name of the Boolean function.
         * @param[in] function - The Boolean function.
         */
        void add_boolean_function(std::string name, BooleanFunction function);

        /**
         * Add multiple Boolean functions to the gate type.
         *
         * @param[in] functions - A map from Boolean function names to Boolean functions.
         */
        void add_boolean_functions(const std::unordered_map<std::string, BooleanFunction>& functions);

        /**
         * Get all Boolean functions of the gate type.
         *
         * @returns A map from Boolean function names to Boolean functions.
         */
        std::unordered_map<std::string, BooleanFunction> get_boolean_functions() const;

    protected:
        u32 m_id;
        std::string m_name;
        BaseType m_base_type;

        std::vector<std::string> m_input_pins;
        std::vector<std::string> m_output_pins;

        std::unordered_map<std::string, std::map<u32, std::string>> m_input_pin_groups;
        std::unordered_map<std::string, std::map<u32, std::string>> m_output_pin_groups;

        std::unordered_map<std::string, BooleanFunction> m_functions;

    private:
        GateType(const GateType&) = delete;               // disable copy-constructor
        GateType& operator=(const GateType&) = delete;    // disable copy-assignment
    };
}    // namespace hal
