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

#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

namespace hal
{
    // TODO tests
    class StateComponent : public GateTypeComponent
    {
    public:
        /**
         * Construct a new StateComponent with given child component and the internal state identifiers.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] state_identifier - The identifier of the internal state.
         * @param[in] neg_state_identifier - The identifier of the negated internal state.
         */
        StateComponent(std::unique_ptr<GateTypeComponent> component, const std::string& state_identifier, const std::string& neg_state_identifier);

        /**
         * Get the type of the gate type component.
         * 
         * @returns The type of the gate type component.
         */
        ComponentType get_type() const override;

        /**
         * Check whether a component is a StateComponent.
         * 
         * @param[in] component - The component to check.
         * @returns True if component is a StateComponent, false otherwise.
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
         * Get the identifier of the internal state.
         * 
         * @returns The identifier of the internal state.
         */
        const std::string& get_state_identifier() const;

        /**
         * Get the identifier of the internal state.
         * 
         * @param[in] state_identifier - The identifier of the internal state.
         */
        void set_state_identifier(const std::string& state_identifier);

        /**
         * Get the identifier of the negated internal state.
         * 
         * @returns The identifier of the negated internal state.
         */
        const std::string& get_neg_state_identifier() const;

        /**
         * Get the identifier of the negated internal state.
         * 
         * @param[in] neg_state_identifier - The identifier of the negated internal state.
         */
        void set_neg_state_identifier(const std::string& neg_state_identifier);

    private:
        static constexpr ComponentType m_type          = ComponentType::state;
        std::unique_ptr<GateTypeComponent> m_component = nullptr;

        std::string m_state_identifier;
        std::string m_neg_state_identifier;
    };
}    // namespace hal