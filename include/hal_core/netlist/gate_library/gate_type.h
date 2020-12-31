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
#include <unordered_set>

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
         * Defines the base type of a gate type.
         */
        enum class BaseType
        {
            combinational, /**< Represents a combinational gate type. **/
            lut,           /**< Represents a combinational LUT gate type. **/
            ff,            /**< Represents a sequential FF gate type. **/
            latch,         /**< Represents a sequential latch gate type. **/
            ram            /**< Represents a sequential RAM gate type. **/
        };

        /**
         * Defines the direction of a pin.
         */
        enum class PinDirection
        {
            input,   /**< Input pin. **/
            output,  /**< Output pin. **/
            inout,   /**< Inout pin. **/
            internal /**< Internal pin. **/
        };

        /**
         * Defines the type of a pin.
         */
        enum class PinType
        {
            none,      /**< Default pin. **/
            power,     /**< Power pin. **/
            ground,    /**< Ground pin. **/
            lut,       /**< Pin that generates output from LUT initialization string. **/
            state,     /**< Pin that generates output from internal state. **/
            neg_state, /**< Pin that generates output from negated internal state. **/
            clock,     /**< Clock pin. **/
            enable,    /**< Enable pin. **/
            set,       /**< Set/preset pin. **/
            reset,     /**< Reset/clear pin. **/
            data,      /**< Data pin. **/
            address    /**< Address pin. **/
        };

        /**
         * Defines the behavior of the gate type in case both clear and preset are active at the same time.
         */
        enum class ClearPresetBehavior
        {
            U = 0, /**< Default value when no behavior is specified. **/
            L = 1, /**< Set the internal state to \p 0. **/
            H = 2, /**< Set the internal state to \p 1. **/
            N = 3, /**< Do not change the internal state. **/
            T = 4, /**< Toggle, i.e., invert the internal state. **/
            X = 5  /**< Set the internal state to \p X. **/
        };

        /**
         * Construct a new gate type by specifying its name and base type.
         *
         * @param[in] name - The name of the gate type.
         * @param[in] base_type - The base type.
         */
        GateType(const std::string& name, BaseType base_type);
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
         * Get a string describing the given gate type.
         *
         * @returns A string describing the gate type.
         */
        std::string to_string() const;

        /**
         * Insert the gate type object to an output stream.
         *
         * @param[in] os - The output stream.
         * @param[in] gate_type - The gate type.
         * @returns An output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, const GateType& gate_type);

        /**
         * Insert the base type string representation to an output stream.
         *
         * @param[in] os - The output stream.
         * @param[in] base_type - The base type.
         * @returns An output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, BaseType base_type);

        /**
         * Insert the pin direction string representation to an output stream.
         *
         * @param[in] os - The output stream.
         * @param[in] direction - The pin direction.
         * @returns An output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, PinDirection direction);

        /**
         * Insert the pin type string representation to an output stream.
         *
         * @param[in] os - The output stream.
         * @param[in] direction - The pin type.
         * @returns An output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, PinType pin_type);

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
         * @param[in] pin - The name of the input pin to add.
         */
        void add_input_pin(std::string pin);

        /**
         * Add a vector of input pins to the gate type.
         *
         * @param[in] pins - The vector of names of input pins to add.
         */
        void add_input_pins(const std::vector<std::string>& pins);

        /**
         * Get a vector of input pins of the gate type.
         *
         * @returns A vector of input pin names of the gate type.
         */
        std::vector<std::string> get_input_pins() const;

        /**
         * Add an output pin to the gate type.
         *
         * @param[in] pin - The name of the output pin to add.
         */
        void add_output_pin(std::string pin);

        /**
         * Add a vector of output pins to the gate type.
         *
         * @param[in] pins - The vector of names of output pins to add.
         */
        void add_output_pins(const std::vector<std::string>& pins);

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
         * Assign a pin type to the given pin. The pin must have been added to the gate type beforehand.
         * 
         * @param[in] pin - The pin.
         * @param[in] pin_type - The pin type to be assigned.
         * @returns True on success, false otherwise.
         */
        bool assign_pin_type(const std::string& pin, PinType pin_type);

        /**
         * Get the pin type of the given pin. If the pin does not exist, the default type 'none' will be returned.
         * 
         * @param[in] pin - The pin.
         * @returns The pin type.
         */
        PinType get_pin_type(const std::string& pin) const;

        /**
         * Get the pin types of all pins as a map.
         * 
         * @returns A map from pin to pin type.
         */
        std::unordered_map<std::string, PinType> get_pin_types() const;

        /** 
         * Get all pins of the specified pin type.
         * 
         * @param[in] pin_type - The pin type.
         * @returns A vector of pins.
         */
        std::vector<std::string> get_pins_of_type(PinType pin_type) const;

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

        /**
         * Set the behavior that describes the internal state when both clear and preset are active at the same time.
         *
         * @param[in] cp1 - The value specifying the behavior for the internal state.
         * @param[in] cp2 - The value specifying the behavior for the negated internal state.
         */
        void set_clear_preset_behavior(ClearPresetBehavior cp1, ClearPresetBehavior cp2);

        /**
         * Get the behavior of the internal state and the negated internal state when both clear and preset are active at the same time.
         *
         * @returns The values specifying the behavior for the internal and negated internal state.
         */
        std::pair<ClearPresetBehavior, ClearPresetBehavior> get_clear_preset_behavior() const;

        /**
         * Set the category in which to find the configuration data associated with this gate type.
         *
         * @param[in] category - The data category.
         */
        void set_config_data_category(const std::string& category);

        /**
         * Get the category in which to find the configuration data associated with this gate type.
         *
         * @returns The data category.
         */
        std::string get_config_data_category() const;

        /**
         * Set the identifier used to specify the configuration data associated with this gate type.
         *
         * @param[in] identifier - The data identifier.
         */
        void set_config_data_identifier(const std::string& identifier);

        /**
         * Get the identifier used to specify the configuration data associated with this gate type.
         *
         * @returns The data identifier.
         */
        std::string get_config_data_identifier() const;

        /**
         * For LUT gate types, set the bit-order of the initialization string.
         *
         * @param[in] ascending - True if ascending bit-order, false otherwise.
         */
        void set_lut_init_ascending(bool ascending);

        /**
         * For LUT gate types, get the bit-order of the initialization string.
         *
         * @returns True if ascending bit-order, false otherwise.
         */
        bool is_lut_init_ascending() const;

    private:
        u32 m_id;
        std::string m_name;
        BaseType m_base_type;

        static const std::unordered_map<BaseType, std::string> m_base_type_to_string;
        static const std::unordered_map<PinDirection, std::string> m_pin_direction_to_string;
        static const std::unordered_map<PinType, std::string> m_pin_type_to_string;

        std::unordered_map<std::string, PinDirection> m_pin_to_direction;
        std::unordered_map<PinDirection, std::vector<std::string>> m_direction_to_pins;

        std::unordered_map<std::string, PinType> m_pin_to_type;
        std::unordered_map<PinType, std::vector<std::string>> m_type_to_pins;
        static const std::unordered_map<PinDirection, std::unordered_set<PinType>> m_direction_to_types;

        std::unordered_map<std::string, std::map<u32, std::string>> m_input_pin_groups;
        std::unordered_map<std::string, std::map<u32, std::string>> m_output_pin_groups;

        std::unordered_map<std::string, BooleanFunction> m_functions;

        std::pair<ClearPresetBehavior, ClearPresetBehavior> m_clear_preset_behavior;

        std::string m_config_data_category = "";
        std::string m_config_data_identifier = "";
        bool m_ascending = true;

        GateType(const GateType&) = delete;               // disable copy-constructor
        GateType& operator=(const GateType&) = delete;    // disable copy-assignment
    };
}    // namespace hal
