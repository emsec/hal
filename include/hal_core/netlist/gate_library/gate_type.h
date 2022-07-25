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
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
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
        carry,         /**< Carry gate type. **/
        pll,           /**< PLL gate type. **/
        oscillator,    /**< Oscillator gate type. **/
    };

    template<>
    std::map<GateTypeProperty,std::string> EnumStrings<GateTypeProperty>::data;

    /**
     * A gate type contains information about its internals such as input and output pins as well as its Boolean functions.
     *
     * @ingroup gate_lib
     */
    class NETLIST_API GateType
    {
    public:
        /**
         * TODO test
         * Get all components matching the filter condition (if provided) as a vector. 
         * Returns an empty vector if (i) the gate type does not contain any components or (ii) no component matches the filter condition.
         * 
         * @param[in] filter - The filter applied to all candidate components, disabled by default.
         * @returns The components.
         */
        std::vector<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const;

        /**
         * TODO test
         * Get a single component matching the filter condition (if provided).
         * Returns a nullptr if (i) the gate type does not contain any components, (ii) multiple components match the filter condition, or (iii) no component matches the filter condition.
         * 
         * @param[in] filter - The filter applied to all candidate components.
         * @returns The component or a nullptr.
         */
        GateTypeComponent* get_component(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const;

        /**
         * TODO test
         * Get a single component and convert it to a component of the type specified by the template parameter.
         * A user-defined filter may be applied to the result set, but is disabled by default.
         * If more no or than one components match the filter condition, a nullptr is returned.
         * A check is performed to determine whether the conversion is legal and a nullptr is returned in case it is not.
         * 
         * @param[in] filter - The user-defined filter function applied to all candidate components.
         * @returns The component converted to the target type or a nullptr.
         */
        template<typename T>
        T* get_component_as(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
        {
            GateTypeComponent* component = this->get_component(filter);
            if (component != nullptr)
            {
                return component->convert_to<T>();
            }

            return nullptr;
        }

        /**
         * TODO test
         * Check if the gate type contains a component of the specified type.
         * 
         * @param[in] type - The component type to check for.
         * @returns True if the gate type contains a component of the speciifed type, false otherwise.
         */
        bool has_component_of_type(const GateTypeComponent::ComponentType type) const;

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
         * TODO test
         * Assign a new property to the gate type.
         * 
         * @param[in] property - The property to assign.
         */
        void assign_property(const GateTypeProperty property);

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
         * @param[in] type - The pin type to be assigned.
         * @returns True on success, false otherwise.
         */
        bool add_pin(const std::string& pin, PinDirection direction, PinType type = PinType::none);

        /**
         * Add a vector of pin of the specified direction and type to the gate type.
         *
         * @param[in] pins - The pins.
         * @param[in] direction - The pin direction to be assigned.
         * @param[in] type - The pin type to be assigned.
         * @returns True on success, false otherwise.
         */
        bool add_pins(const std::vector<std::string>& pins, PinDirection direction, PinType type = PinType::none);

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
         * @param[in] type - The pin type to be assigned.
         * @returns True on success, false otherwise.
         */
        bool assign_pin_type(const std::string& pin, PinType type);

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
         * @param[in] type - The pin type.
         * @returns A set of pins.
         */
        std::unordered_set<std::string> get_pins_of_type(PinType type) const;

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
        const std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>>& get_pin_groups() const;

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
         * Get the index of the given pin within the specified group.
         * 
         * @param[in] group - The name of the pin group. 
         * @param[in] pin - The name of the pin.
         * @return u32 
         */
        i32 get_index_in_group_of_pin(const std::string& group, const std::string& pin) const;

        /**
         * Add a Boolean function with the specified name to the gate type.
         *
         * @param[in] name - The name of the Boolean function.
         * @param[in] function - The Boolean function.
         */
        void add_boolean_function(const std::string& name, const BooleanFunction& function);

        /**
         * Add multiple Boolean functions to the gate type.
         *
         * @param[in] functions - A map from Boolean function names to Boolean functions.
         */
        void add_boolean_functions(const std::unordered_map<std::string, BooleanFunction>& functions);

        /**
         * TODO test
         * Get the Boolean function specified by name.
         * If no Boolean function matches the name, an empty function is returned.
         * 
         * @param[in] function_name - The name of the Boolean function.
         * @returns The specified Boolean function.
         */
        const BooleanFunction get_boolean_function(const std::string& function_name) const;

        /**
         * Get all Boolean functions of the gate type.
         *
         * @returns A map from Boolean function names to Boolean functions.
         */
        const std::unordered_map<std::string, BooleanFunction>& get_boolean_functions() const;

    private:
        friend class GateLibrary;

        GateLibrary* m_gate_library;
        u32 m_id;
        std::string m_name;
        std::set<GateTypeProperty> m_properties;
        std::unique_ptr<GateTypeComponent> m_component;

        // pins
        std::vector<std::string> m_pins;
        std::unordered_set<std::string> m_pins_set;

        // pin directions
        std::unordered_map<std::string, PinDirection> m_pin_to_direction;
        std::unordered_map<PinDirection, std::unordered_set<std::string>> m_direction_to_pins;

        // pin types
        std::unordered_map<std::string, PinType> m_pin_to_type;
        std::unordered_map<PinType, std::unordered_set<std::string>> m_type_to_pins;

        // pin groups
        std::unordered_map<std::string, std::string> m_pin_to_group;
        std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> m_pin_groups;
        std::unordered_map<std::string, std::unordered_map<u32, std::string>> m_pin_group_indices;

        // Boolean functions
        std::unordered_map<std::string, BooleanFunction> m_functions;

        GateType(GateLibrary* gate_library, u32 id, const std::string& name, std::set<GateTypeProperty> properties, std::unique_ptr<GateTypeComponent> component = nullptr);

        GateType(const GateType&) = delete;
        GateType& operator=(const GateType&) = delete;
    };
}    // namespace hal
