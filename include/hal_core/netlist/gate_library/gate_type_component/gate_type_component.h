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

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"

#include <functional>
#include <set>

namespace hal
{
    /**
     * A component defining additional functionality of a gate type.
     */
    class GateTypeComponent
    {
    public:
        virtual ~GateTypeComponent() = default;

        /**
         * The type of a gate type component.
         */
        enum class ComponentType
        {
            lut,     /**< LUT component type. */
            ff,      /**< Flip-flop component type. */
            latch,   /**< Latch component type. */
            ram,     /**< RAM component type. */
            mac,     /**< MAC component type. */
            init,    /**< Initialization component type. */
            state,   /**< State component type. */
            ram_port /**< RAM port component type. */
        };

        /**
         * Create a new LUTComponent with given child component and bit-order.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] init_ascending - True if ascending bit-order, false otherwise.
         * @returns The LUTComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_lut_component(std::unique_ptr<GateTypeComponent> component, bool init_ascending);

        /**
         * Create a new FFComponent with given child component and tzhe Boolean functions describing the next state and the clock signal.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] next_state_bf - The function describing the internal state.
         * @param[in] clock_bf - The function describing the clock input.
         * @returns The FFComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_ff_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& next_state_bf, const BooleanFunction& clock_bf);

        /**
         * Create a new LatchComponent with given child component and the Boolean functions describing the data input and the enable signal.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @returns The LatchComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_latch_component(std::unique_ptr<GateTypeComponent> component);

        /**
         * Create a new RAMComponent with given child component.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] bit_size - The size of the RAM in bits.
         * @returns The RAMComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_ram_component(std::unique_ptr<GateTypeComponent> component, const u32 bit_size);

        /**
         * Create a new MACComponent.
         * 
         * @returns The MACComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_mac_component();

        /**
         * Create a new InitComponent with given child component, the category and a vector of identifiers pointing to the initialization data.
         * 
         * @param[in] init_category - The data category.
         * @param[in] init_identifiers - The data identifiers.
         * @returns The InitComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_init_component(const std::string& init_category, const std::vector<std::string>& init_identifiers);

        /**
         * Create a new StateComponent with given child component and the internal state identifiers.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] state_identifier - The identifier of the internal state.
         * @param[in] neg_state_identifier - The identifier of the negated internal state.
         * @returns The StateComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_state_component(std::unique_ptr<GateTypeComponent> component, const std::string& state_identifier, const std::string& neg_state_identifier);

        /**
         * Create a new RAMPortComponent with given child component, the write/read data/address pin groups, and the write/read clock/enable functions.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] data_group - The name of the read or write data pin group.
         * @param[in] addr_group - The name of the read or write address pin group.
         * @param[in] clock_bf - The read or write clock's Boolean function.
         * @param[in] enable_bf - The read or write enable's Boolean function.
         * @param[in] is_write - Set true for write port, false for read port.
         * @returns The RAMPortComponent.
         */
        static std::unique_ptr<GateTypeComponent> create_ram_port_component(std::unique_ptr<GateTypeComponent> component,
                                                                            const std::string& data_group,
                                                                            const std::string& addr_group,
                                                                            const BooleanFunction& clock_bf,
                                                                            const BooleanFunction& enable_bf,
                                                                            bool is_write);

        /**
         * Get the type of the gate type component.
         * 
         * @returns The type of the gate type component.
         */
        virtual ComponentType get_type() const = 0;

        /**
         * Convert the gate type component to a component of the type specified by the template parameter.
         * A check is performed to determine whether the conversion is legal and a nullptr is returned in case it is not.
         * 
         * @returns The component converted to the target type or a nullptr.
         */
        template<typename T>
        T* convert_to()
        {
            if (T::is_class_of(this))
            {
                return static_cast<T*>(this);
            }

            return nullptr;
        }

        /**
         * @copydoc GateTypeComponent::convert_to
         */
        template<typename T>
        const T* convert_to() const
        {
            if (T::is_class_of(this))
            {
                return static_cast<const T*>(this);
            }

            return nullptr;
        }

        /**
         * Get all components matching the filter condition (if provided) as a vector. 
         * Returns an empty vector if (i) the gate type does not contain any components or (ii) no component matches the filter condition.
         * 
         * @param[in] filter - The filter applied to all candidate components, disabled by default.
         * @returns The components.
         */
        virtual std::vector<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const = 0;

        /**
         * Get a single component matching the filter condition (if provided).
         * Returns a nullptr if (i) the gate type does not contain any components, (ii) multiple components match the filter condition, or (iii) no component matches the filter condition.
         * 
         * @param[in] filter - The filter applied to all candidate components.
         * @returns The component or a nullptr.
         */
        GateTypeComponent* get_component(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const;

        /**
         * Get a single sub-component of the gate type component and convert it to a component of the type specified by the template parameter.
         * A user-defined filter may be applied to the result set, but is disabled by default.
         * If more no or than one components match the filter condition, a nullptr is returned.
         * A check is performed to determine whether the conversion is legal and a nullptr is returned in case it is not.
         * 
         * @param[in] filter - The user-defined filter function applied to all candidate components.
         * @returns The sub-component of the gate type component converted to the target type or a nullptr.
         */
        template<typename T>
        T* get_component_as(const std::function<bool(const GateTypeComponent*)>& filter = nullptr)
        {
            GateTypeComponent* component = this->get_component(filter);
            if (component != nullptr)
            {
                return component->convert_to<T>();
            }

            return nullptr;
        }
    };

    template<>
    std::map<GateTypeComponent::ComponentType, std::string> EnumStrings<GateTypeComponent::ComponentType>::data;
}    // namespace hal