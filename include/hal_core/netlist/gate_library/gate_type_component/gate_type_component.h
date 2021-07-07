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
    class GateTypeComponent
    {
    public:
        enum class ComponentType
        {
            lut,
            ff,
            latch,
            ram,
            mac,
            init,
            ram_port
        };

        // factory methods
        static std::unique_ptr<GateTypeComponent> create_lut_component(std::unique_ptr<GateTypeComponent> component, bool init_ascending);
        static std::unique_ptr<GateTypeComponent> create_ff_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& next_state_bf, const BooleanFunction& clock_bf);
        static std::unique_ptr<GateTypeComponent> create_latch_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& data_in_bf, const BooleanFunction& enable_bf);
        static std::unique_ptr<GateTypeComponent> create_ram_component(std::vector<std::unique_ptr<GateTypeComponent>> components);
        static std::unique_ptr<GateTypeComponent> create_mac_component();
        static std::unique_ptr<GateTypeComponent> create_init_component(const std::string& init_category, const std::string& init_identifier);
        static std::unique_ptr<GateTypeComponent> create_ram_port_component();

        virtual ComponentType get_type() const = 0;

        template<typename T>
        T* convert_to()
        {
            if (T::is_class_of(this))
            {
                return static_cast<T*>(this);
            }

            return nullptr;
        }

        virtual std::set<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const = 0;
        GateTypeComponent* get_component(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const;

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