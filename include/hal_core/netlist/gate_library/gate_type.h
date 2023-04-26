// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/gate_type_property.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/netlist/pins/pin_group.h"
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
     * A gate type contains information about its internals such as input and output pins as well as its Boolean functions.
     *
     * @ingroup gate_lib
     */
    class NETLIST_API GateType
    {
    public:
        /**
         * Get all components matching the filter condition (if provided) as a vector. 
         * Returns an empty vector if (i) the gate type does not contain any components or (ii) no component matches the filter condition.
         * 
         * @param[in] filter - The filter applied to all candidate components, disabled by default.
         * @returns The components.
         */
        std::vector<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const;

        /**
         * Get a single component matching the filter condition (if provided).
         * Returns a nullptr if (i) the gate type does not contain any components, (ii) multiple components match the filter condition, or (iii) no component matches the filter condition.
         * 
         * @param[in] filter - The filter applied to all candidate components.
         * @returns The component or a nullptr.
         */
        GateTypeComponent* get_component(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const;

        /**
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
         * Get the properties assigned to the gate type as list
         * where the most significant property stands at first position.
         *
         * @return The properties of the gate type.
         */
        std::vector<GateTypeProperty> get_property_list() const;

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
         * Get a spare pin ID.<br>
         * The value of 0 is reserved and represents an invalid ID.
         * 
         * @returns The pin ID.
         */
        u32 get_unique_pin_id();

        /**
         * Get a spare pin group ID.<br>
         * The value of 0 is reserved and represents an invalid ID.
         * 
         * @returns The pin group ID.
         */
        u32 get_unique_pin_group_id();

        /**
         * Create a gate pin with the specified name.
         * 
         * @param[in] id - The ID of the pin.
         * @param[in] name - The name of the pin.
         * @param[in] direction - The direction of the pin.
         * @param[in] type - The type of the pin. Defaults to `PinType::none`.
         * @param[in] create_group - Set `true` to automatically assign the pin to a new pin group, `false` otherwise. Defaults to `true`.
         * @returns The gate pin on success, an error message otherwise.
         */
        Result<GatePin*> create_pin(const u32 id, const std::string& name, PinDirection direction, PinType type = PinType::none, bool create_group = true);

        /**
         * Create a gate pin with the specified name.
         * The ID of the pin is set automatically.
         * 
         * @param[in] name - The name of the pin.
         * @param[in] direction - The direction of the pin.
         * @param[in] type - The type of the pin. Defaults to `PinType::none`.
         * @param[in] create_group - Set `true` to create a pin group for the pin, `false` otherwise. Defaulrs to `false`. 
         * @returns The gate pin on success, an error message otherwise.
         */
        Result<GatePin*> create_pin(const std::string& name, PinDirection direction, PinType type = PinType::none, bool create_group = true);

        /**
         * Get an ordered vector of all pins of the gate type.
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         * 
         * @param[in] filter - An optional filter.
         * @returns An ordered vector of pins.
         */
        std::vector<GatePin*> get_pins(const std::function<bool(GatePin*)>& filter = nullptr) const;

        /**
         * Get an ordered vector of the names of all pins of the gate type.
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         * 
         * @param[in] filter - An optional filter.
         * @returns An ordered vector of pin names.
         */
        std::vector<std::string> get_pin_names(const std::function<bool(GatePin*)>& filter = nullptr) const;

        /**
         * Get an ordered vector of all input pins of the gate type (including inout pins).
         * 
         * @returns An ordered vector of input pins.
         */
        std::vector<GatePin*> get_input_pins() const;

        /**
         * Get an ordered vector of the names of all input pins of the gate type (including inout pins).
         * 
         * @returns An ordered vector of input pin names.
         */
        std::vector<std::string> get_input_pin_names() const;

        /**
         * Get an ordered vector of all output pins of the gate type (including inout pins).
         * 
         * @returns An ordered vector of output pins.
         */
        std::vector<GatePin*> get_output_pins() const;

        /**
         * Get an ordered vector of the names of all output pins of the gate type (including inout pins).
         * 
         * @returns An ordered vector of output pin names.
         */
        std::vector<std::string> get_output_pin_names() const;

        /**
         * Get the pin corresponding to the given ID.
         * 
         * @param[in] id - The ID of the pin.
         * @returns The pin on success, a `nullptr` otherwise.
         */
        GatePin* get_pin_by_id(const u32 id) const;

        /**
         * Get the pin corresponding to the given name.
         * 
         * @param[in] name - The name of the pin.
         * @returns The pin on success, a `nullptr` otherwise.
         */
        GatePin* get_pin_by_name(const std::string& name) const;

        /** 
         * Create a pin group with the given name.
         * 
         * @param[in] id - The ID of the pin group.
         * @param[in] name - The name of the pin group.
         * @param[in] pins - The pins to be assigned to the pin group. Defaults to an empty vector.
         * @param[in] direction - The direction of the pin group. Defaults to `PinDirection::none`.
         * @param[in] type - The type of the pin group. Defaults to `PinType::none`.
         * @param[in] ascending - Set `true` for ascending pin order (from 0 to n-1), `false` otherwise (from n-1 to 0). Defaults to `false`.
         * @param[in] start_index - The start index of the pin group. Defaults to `0`.
         * @returns The pin group on success, an error message otherwise.
         */
        Result<PinGroup<GatePin>*> create_pin_group(const u32 id,
                                                    const std::string& name,
                                                    const std::vector<GatePin*> pins = {},
                                                    PinDirection direction           = PinDirection::none,
                                                    PinType type                     = PinType::none,
                                                    bool ascending                   = false,
                                                    u32 start_index                  = 0);

        /**
         * Create a pin group with the given name.
         * The ID of the pin group is set automatically.
         * 
         * @param[in] name - The name of the pin group.
         * @param[in] pins - The pins to be assigned to the pin group. Defaults to an empty vector.
         * @param[in] direction - The direction of the pin group. Defaults to `PinDirection::none`.
         * @param[in] type - The type of the pin group. Defaults to `PinType::none`.
         * @param[in] ascending - Set `true` for ascending pin order (from 0 to n-1), `false` otherwise (from n-1 to 0). Defaults to `false`.
         * @param[in] start_index - The start index of the pin group. Defaults to `0`.
         * @returns The pin group on success, an error message otherwise.
         */
        Result<PinGroup<GatePin>*> create_pin_group(const std::string& name,
                                                    const std::vector<GatePin*> pins = {},
                                                    PinDirection direction           = PinDirection::none,
                                                    PinType type                     = PinType::none,
                                                    bool ascending                   = false,
                                                    u32 start_index                  = 0);

        /**
         * Get an ordered vector of all pin groups of the gate type.
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         * 
         * @param[in] filter - An optional filter.
         * @returns An ordered vector of pin groups.
         */
        std::vector<PinGroup<GatePin>*> get_pin_groups(const std::function<bool(PinGroup<GatePin>*)>& filter = nullptr) const;

        /**
         * Get the pin group corresponding to the given ID.
         * 
         * @param[in] id - The ID of the pin group.
         * @returns The pin group on success, a `nullptr` otherwise.
         */
        PinGroup<GatePin>* get_pin_group_by_id(const u32 id) const;

        /**
         * Get the pin group corresponding to the given name.
         * 
         * @param[in] name - The name of the pin group.
         * @returns The pin group on success, a `nullptr` otherwise.
         */
        PinGroup<GatePin>* get_pin_group_by_name(const std::string& name) const;

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
         * @param[in] functions - A map from names to Boolean functions.
         */
        void add_boolean_functions(const std::unordered_map<std::string, BooleanFunction>& functions);

        /**
         * Get all Boolean functions of the gate type.
         *
         * @returns A map from names to Boolean functions.
         */
        const std::unordered_map<std::string, BooleanFunction>& get_boolean_functions() const;

        /**
         * Get the Boolean function specified by the given name.
         * This name can for example be an output pin of the gate or any other user-defined function name.
         * 
         * @param[in] name - The name of the Boolean function.
         * @returns The Boolean function on success, an empty Boolean function otherwise.
         */
        BooleanFunction get_boolean_function(const std::string& name) const;

        /**
         * Get the Boolean function corresponding to the given output pin.
         * If `pin` is a `nullptr`, the Boolean function of the first output pin is returned.
         * 
         * @param[in] pin - The pin.
         * @returns The Boolean function on success, an empty Boolean function otherwise.
         */
        BooleanFunction get_boolean_function(const GatePin* pin = nullptr) const;

    private:
        friend class GateLibrary;

        GateLibrary* m_gate_library;
        u32 m_id;
        std::string m_name;
        std::set<GateTypeProperty> m_properties;
        std::unique_ptr<GateTypeComponent> m_component;

        // pins
        u32 m_next_pin_id;
        std::set<u32> m_used_pin_ids;
        std::set<u32> m_free_pin_ids;
        u32 m_next_pin_group_id;
        std::set<u32> m_used_pin_group_ids;
        std::set<u32> m_free_pin_group_ids;

        std::vector<std::unique_ptr<GatePin>> m_pins;
        std::unordered_map<u32, GatePin*> m_pins_map;
        std::unordered_map<std::string, GatePin*> m_pin_names_map;
        std::vector<std::unique_ptr<PinGroup<GatePin>>> m_pin_groups;
        std::unordered_map<u32, PinGroup<GatePin>*> m_pin_groups_map;
        std::unordered_map<std::string, PinGroup<GatePin>*> m_pin_group_names_map;
        std::list<PinGroup<GatePin>*> m_pin_groups_ordered;

        // Boolean functions
        std::unordered_map<std::string, BooleanFunction> m_functions;

        GateType(GateLibrary* gate_library, u32 id, const std::string& name, std::set<GateTypeProperty> properties, std::unique_ptr<GateTypeComponent> component = nullptr);

        GateType(const GateType&)            = delete;
        GateType& operator=(const GateType&) = delete;
    };
}    // namespace hal
