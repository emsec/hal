//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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
#include "hal_core/utilities/enums.h"

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    class GateLibrary;

    /**
     * A set of available properties for a gate type.
     */
    enum class GateTypeProperty
    {
        combinational, /**< Combinational gate type. **/
        sequential,    /**< Sequential gate type. **/
        power,         /**< Power gate type. **/
        ground,        /**< Ground gate type. **/
        lut,           /**< LUT gate type. **/
        ff,            /**< Flip-flop gate type. **/
        latch,         /**< Latch gate type. **/
        ram,           /**< RAM gate type. **/
        io,            /**< IO gate type. **/
        dsp,           /**< DSP gate type. **/
        mux,           /**< MUX gate type. **/
        buffer,        /**< Buffer gate type. **/
        carry          /**< Carry gate type. **/
    };

    template<>
    std::vector<std::string> EnumStrings<GateTypeProperty>::data;

    /**
     * Defines the direction of a pin.
     */
    enum class PinDirection
    {
        none,    /**< Invalid pin. **/
        input,   /**< Input pin. **/
        output,  /**< Output pin. **/
        inout,   /**< Inout pin. **/
        internal /**< Internal pin. **/
    };

    template<>
    std::vector<std::string> EnumStrings<PinDirection>::data;

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
        address,   /**< Address pin. **/
        io_pad,    /**< IO pad pin. **/
        select     /**< Select pin. **/
    };

    template<>
    std::vector<std::string> EnumStrings<PinType>::data;

    /**
     * A gate type contains information about its internals such as input and output pins as well as its Boolean functions.
     *
     * @ingroup gate_lib
     */
    class NETLIST_API GateType
    {
    public:
        /**
         * Defines the behavior of the gate type in case both clear and preset are active at the same time.
         */
        enum class ClearPresetBehavior
        {
            L,    /**< Set the internal state to \p 0. **/
            H,    /**< Set the internal state to \p 1. **/
            N,    /**< Do not change the internal state. **/
            T,    /**< Toggle, i.e., invert the internal state. **/
            X,    /**< Set the internal state to \p X. **/
            undef /**< Invalid behavior, used by default. **/
        };

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
        const std::string& get_name() const;

        /**
         * Get the properties assigned to the gate type.
         *
         * @returns The properties of the gate type.
         */
        std::set<GateTypeProperty> get_properties() const;

        /**
         * Check whether the gate type has the specified property.
         *
         * @param[in] property - The property to check for.
         * @returns True if the gate type has the specified property, false otherwise.
         */
        bool has_property(GateTypeProperty property) const;

        /**
         * Get the gate library this gate type is associated with.
         *
         * @returns The gate library.
         */
        GateLibrary* get_gate_library() const;

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
        void add_input_pin(const std::string& pin);

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
        void add_output_pin(const std::string& pin);

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
         * Add a pin of the specified direction and type to the gate type.
         *
         * @param[in] pin - The pin.
         * @param[in] direction - The pin direction to be assigned.
         * @param[in] pin_type - The pin type to be assigned.
         * @returns True on success, false otherwise.
         */
        bool add_pin(const std::string& pin, PinDirection direction, PinType pin_type = PinType::none);

        /**
         * Add a vector of pin of the specified direction and type to the gate type.
         *
         * @param[in] pins - The pins.
         * @param[in] direction - The pin direction to be assigned.
         * @param[in] pin_type - The pin type to be assigned.
         * @returns True on success, false otherwise.
         */
        bool add_pins(const std::vector<std::string>& pins, PinDirection direction, PinType pin_type = PinType::none);

        /**
         * Get all pins belonging to the gate type.
         *
         * @returns A vector of pins.
         */
        const std::vector<std::string>& get_pins() const;

        /**
         * Get the pin direction of the given pin. The user has to make sure that the pin exists before calling this function. If the pin does not exist, the direction 'internal' will be returned.
         *
         * @param[in] pin - The pin.
         * @returns The pin direction.
         */
        PinDirection get_pin_direction(const std::string& pin) const;

        /**
         * Get the pin directions of all pins as a map.
         *
         * @returns A map from pin to pin direction.
         */
        const std::unordered_map<std::string, PinDirection>& get_pin_directions() const;

        /**
         * Get all pins of the specified pin direction.
         *
         * @param[in] direction - The pin direction.
         * @returns A set of pins.
         */
        std::unordered_set<std::string> get_pins_of_direction(PinDirection direction) const;

        /**
         * Assign a pin type to the given pin. The pin must have been added to the gate type beforehand.
         *
         * @param[in] pin - The pin.
         * @param[in] pin_type - The pin type to be assigned.
         * @returns True on success, false otherwise.
         */
        bool assign_pin_type(const std::string& pin, PinType pin_type);

        /**
         * Get the pin type of the given pin. The user has to make sure that the pin exists before calling this function. If the pin does not exist, the type 'none' will be returned.
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
        const std::unordered_map<std::string, PinType>& get_pin_types() const;

        /**
         * Get all pins of the specified pin type.
         *
         * @param[in] pin_type - The pin type.
         * @returns A set of pins.
         */
        std::unordered_set<std::string> get_pins_of_type(PinType pin_type) const;

        /**
         * Assign existing pins to a pin group.
         *
         * @param[in] group - The name of the pin group.
         * @param[in] pins - The pins to be added to the group including their indices.
         * @returns True on success, false otherwise.
         */
        bool assign_pin_group(const std::string& group, const std::vector<std::pair<u32, std::string>>& pins);

        /**
         * Get the pin type of the given pin. The user has to make sure that the pin exists before calling this function. If the pin is not in a group or the does not exist, an empty string will be returned.
         *
         * @param[in] pin - The pin.
         * @returns The pin group.
         */
        std::string get_pin_group(const std::string& pin) const;

        /**
         * Get all pin groups of the gate type.
         *
         * @returns A map from pin group names to the pins of each group including their indices.
         */
        std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> get_pin_groups() const;

        /**
         * Get all pins of the specified pin group including their indices.
         *
         * @param[in] group - The name of the pin group.
         * @returns The pins including their indices.
         */
        std::vector<std::pair<u32, std::string>> get_pins_of_group(const std::string& group) const;

        /**
         * Get the pin at the specified index of the given group.
         *
         * @param[in] group - The name of the pin group.
         * @param[in] index - The index of the pin.
         * @returns The pin.
         */
        std::string get_pin_of_group_at_index(const std::string& group, const u32 index) const;

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
        const std::unordered_map<std::string, BooleanFunction>& get_boolean_functions() const;

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
        const std::pair<ClearPresetBehavior, ClearPresetBehavior>& get_clear_preset_behavior() const;

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
        const std::string& get_config_data_category() const;

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
        const std::string& get_config_data_identifier() const;

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
        friend class GateLibrary;

        GateLibrary* m_gate_library;
        u32 m_id;
        std::string m_name;
        std::set<GateTypeProperty> m_properties;

        // pins
        std::vector<std::string> m_pins;
        std::unordered_set<std::string> m_pins_set;

        // pin direction
        std::unordered_map<std::string, PinDirection> m_pin_to_direction;
        std::unordered_map<PinDirection, std::unordered_set<std::string>> m_direction_to_pins;

        // pin type
        std::unordered_map<std::string, PinType> m_pin_to_type;
        std::unordered_map<PinType, std::unordered_set<std::string>> m_type_to_pins;
        static const std::unordered_map<PinDirection, std::unordered_set<PinType>> m_direction_to_types;

        // pin groups
        std::unordered_map<std::string, std::string> m_pin_to_group;
        std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> m_pin_groups;
        std::unordered_map<std::string, std::unordered_map<u32, std::string>> m_pin_group_indices;

        // Boolean functions
        std::unordered_map<std::string, BooleanFunction> m_functions;

        // sequential and LUT stuff
        std::pair<ClearPresetBehavior, ClearPresetBehavior> m_clear_preset_behavior = {ClearPresetBehavior::undef, ClearPresetBehavior::undef};
        std::string m_config_data_category                                          = "";
        std::string m_config_data_identifier                                        = "";
        bool m_ascending                                                            = true;

        GateType(GateLibrary* gate_library, u32 id, const std::string& name, std::set<GateTypeProperty> properties);

        GateType(const GateType&) = delete;
        GateType& operator=(const GateType&) = delete;
    };

    template<>
    std::vector<std::string> EnumStrings<GateType::ClearPresetBehavior>::data;
}    // namespace hal
