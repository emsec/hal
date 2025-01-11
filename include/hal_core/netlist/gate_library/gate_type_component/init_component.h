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

#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

namespace hal
{
    class InitComponent : public GateTypeComponent
    {
    public:
        /**
         * Construct a new InitComponent with given child component, the category and a vector of identifiers pointing to the initialization data.
         * 
         * @param[in] init_category - The data category.
         * @param[in] init_identifiers - The data identifiers.
         */
        InitComponent(const std::string& init_category, const std::vector<std::string>& init_identifiers);

        /**
         * Get the type of the gate type component.
         * 
         * @returns The type of the gate type component.
         */
        ComponentType get_type() const override;

        /**
         * Check whether a component is an InitComponent.
         * 
         * @param[in] component - The component to check.
         * @returns True if component is an InitComponent, false otherwise.
         */
        static bool is_class_of(const GateTypeComponent* component);

        /**
         * Get the sub-components of the gate type component.
         * A user-defined filter may be applied to the result vector, but is disabled by default.
         * 
         * @param[in] filter - The user-defined filter function applied to all candidate components.
         * @returns The sub-components of the gate type component.
         */
        std::vector<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const override;

        /**
         * Get the category in which to find the initialization data.
         *
         * @returns The data category.
         */
        const std::string& get_init_category() const;

        /**
         * Set the category in which to find the initialization data.
         *
         * @param[in] init_category - The data category.
         */
        void set_init_category(const std::string& init_category);

        /**
         * Get the vector of identifiers at which to find initialization data.
         *
         * @returns The data identifiers.
         */
        const std::vector<std::string>& get_init_identifiers() const;

        /**
         * Set the vector of identifiers at which to find initialization data.
         *
         * @param[in] init_identifiers - The data identifiers.
         */
        void set_init_identifiers(const std::vector<std::string>& init_identifiers);

    private:
        static constexpr ComponentType m_type = ComponentType::init;

        std::string m_init_category                 = "";
        std::vector<std::string> m_init_identifiers = {};
    };
}    // namespace hal