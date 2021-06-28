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

#include <set>

namespace hal
{
    class GateTypeComponent
    {
    public:
        enum class ComponentType
        {
            none = 0,
            init,
            lut,
            ff,
            latch,
            ram
        };

        // factory methods
        static std::unique_ptr<GateTypeComponent> create_init_component(const std::string& init_category, const std::string& init_identifier);
        static std::unique_ptr<GateTypeComponent> create_lut_component(std::unique_ptr<GateTypeComponent> component, bool init_ascending);

        virtual ComponentType get_type() const = 0;

        static bool is_class_of(const const GateTypeComponent* component)
        {
            return component->get_type() == m_type;
        }

        template<typename T>
        const T* get_as() const
        {
            if (T::is_class_of(this))
            {
                return static_cast<const T*>(this);
            }

            return nullptr;
        }

        virtual std::set<const GateTypeComponent*> get_components() const = 0;

    protected:
        static constexpr ComponentType m_type = ComponentType::none;
    };

}    // namespace hal