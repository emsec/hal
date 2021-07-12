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
            ram_port /**< RAM port component type. */
        };

        // factory methods
        static std::unique_ptr<GateTypeComponent> create_lut_component(std::unique_ptr<GateTypeComponent> component, bool init_ascending);
        static std::unique_ptr<GateTypeComponent> create_ff_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& next_state_bf, const BooleanFunction& clock_bf);
        static std::unique_ptr<GateTypeComponent> create_latch_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& data_in_bf, const BooleanFunction& enable_bf);
        static std::unique_ptr<GateTypeComponent> create_ram_component(std::unique_ptr<GateTypeComponent> component);
        static std::unique_ptr<GateTypeComponent> create_mac_component();
        static std::unique_ptr<GateTypeComponent> create_init_component(const std::string& init_category, const std::string& init_identifier);
        static std::unique_ptr<GateTypeComponent> create_ram_port_component();

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
         * Get the sub-components of the gate type component.
         * A user-defined filter may be applied to the result set, but is disabled by default.
         * 
         * @param[in] filter - The user-defined filter function applied to all candidate components.
         * @returns The sub-components of the gate type component.
         */
        virtual std::set<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const = 0;

        /**
         * Get a single sub-component of the gate type component.
         * A user-defined filter may be applied to the result set, but is disabled by default.
         * If more no or than one components match the filter condition, a nullptr is returned.
         * 
         * @param[in] filter - The user-defined filter function applied to all candidate components.
         * @returns The sub-component of the gate type component or a nullptr.
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
    std::vector<std::string> EnumStrings<GateTypeComponent::ComponentType>::data;
}    // namespace hal