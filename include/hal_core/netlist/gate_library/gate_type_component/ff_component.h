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
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

namespace hal
{
    class FFComponent : public GateTypeComponent
    {
    public:
        /**
         * Construct a new FFComponent with given child component and the Boolean functions describing the next state and the clock signal.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] next_state_bf - The function describing the internal state.
         * @param[in] clock_bf - The function describing the clock input.
         */
        FFComponent(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& next_state_bf, const BooleanFunction& clock_bf);

        /**
         * Get the type of the gate type component.
         * 
         * @returns The type of the gate type component.
         */
        ComponentType get_type() const override;

        /**
         * Check whether a component is a FFComponent.
         * 
         * @param[in] component - The component to check.
         * @returns True if component is a FFComponent, false otherwise.
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
         * Get the Boolean function describing the next internal state of the flip-flop.
         * 
         * @returns The function describing the internal state.
         */
        BooleanFunction get_next_state_function() const;

        /**
         * Set the Boolean function describing the next internal state of the flip-flop.
         * 
         * @param[in] next_state_bf - The function describing the internal state.
         */
        void set_next_state_function(const BooleanFunction& next_state_bf);

        /**
         * Get the Boolean function describing the clock input of the flip-flop.
         * 
         * @returns The function describing the clock input.
         */
        BooleanFunction get_clock_function() const;

        /**
         * Set the Boolean function describing the clock input of the flip-flop.
         * 
         * @param[in] clock_bf - The function describing the clock input.
         */
        void set_clock_function(const BooleanFunction& clock_bf);

        /**
         * Get the Boolean function describing the asynchronous reset behavior of the flip-flop.
         * Returns an empty function if asynchronous reset is not supported by the flip-flop.
         * 
         * @returns The function describing the asynchronous reset behavior.
         */
        BooleanFunction get_async_reset_function() const;

        /**
         * Set the Boolean function describing the asynchronous reset behavior of the flip-flop.
         * 
         * @param[in] async_reset_bf - The function describing the asynchronous reset behavior.
         */
        void set_async_reset_function(const BooleanFunction& async_reset_bf);

        /**
         * Get the Boolean function describing the asynchronous set behavior of the flip-flop.
         * Returns an empty function if asynchronous set is not supported by the flip-flop.
         * 
         * @returns The function describing the asynchronous set behavior.
         */
        BooleanFunction get_async_set_function() const;

        /**
         * Set the Boolean function describing the asynchronous set behavior of the flip-flop.
         * 
         * @param[in] async_set_bf - The function describing the asynchronous set behavior.
         */
        void set_async_set_function(const BooleanFunction& async_set_bf);

        /**
         * Get the behavior of the internal state and the negated internal state when both asynchronous set and reset are active at the same time.
         * 
         * @returns The values specifying the behavior for the internal and negated internal state.
         */
        const std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior>& get_async_set_reset_behavior() const;

        /**
         * Set the behavior of the internal state and the negated internal state when both asynchronous set and reset are active at the same time.
         * 
         * @param[in] behav_state - The behavior of the internal state.
         * @param[in] behav_neg_state - The behavior of the negated internal state.
         */
        void set_async_set_reset_behavior(const AsyncSetResetBehavior behav_state, const AsyncSetResetBehavior behav_neg_state);

    private:
        static constexpr ComponentType m_type          = ComponentType::ff;
        std::unique_ptr<GateTypeComponent> m_component = nullptr;

        BooleanFunction m_next_state_bf                                                    = BooleanFunction();
        BooleanFunction m_clock_bf                                                         = BooleanFunction();
        BooleanFunction m_async_reset_bf                                                   = BooleanFunction();
        BooleanFunction m_async_set_bf                                                     = BooleanFunction();
        std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> m_async_set_reset_behavior = {AsyncSetResetBehavior::undef, AsyncSetResetBehavior::undef};
    };
}    // namespace hal