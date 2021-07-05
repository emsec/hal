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
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

namespace hal
{
    class GateType;

    class FFComponent : public GateTypeComponent
    {
    public:
        /**
         * Defines the behavior of the gate type in case both clear and preset are active at the same time.
         */
        enum class AsyncSetResetBehavior
        {
            L,    /**< Set the internal state to \p 0. **/
            H,    /**< Set the internal state to \p 1. **/
            N,    /**< Do not change the internal state. **/
            T,    /**< Toggle, i.e., invert the internal state. **/
            X,    /**< Set the internal state to \p X. **/
            undef /**< Invalid behavior, used by default. **/
        };

        FFComponent(std::unique_ptr<GateTypeComponent> component) : m_component(std::move(component))
        {
        }

        ComponentType get_type() const override;

        std::set<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const override;

        BooleanFunction get_clock_function(const GateType* gate_type) const;
        void set_clock_function(GateType* gate_type, const BooleanFunction& clk_bf);

        BooleanFunction get_enable_function(const GateType* gate_type) const;
        void set_enable_function(GateType* gate_type, const BooleanFunction& en_bf);

        BooleanFunction get_async_reset_function(const GateType* gate_type) const;
        void set_async_reset_function(GateType* gate_type, const BooleanFunction& rst_bf);

        BooleanFunction get_async_set_function(const GateType* gate_type) const;
        void set_async_set_function(GateType* gate_type, const BooleanFunction& set_bf);

        const std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior>& get_async_set_reset_behavior() const;
        void set_async_set_reset_behavior(AsyncSetResetBehavior behav_state, AsyncSetResetBehavior behav_neg_state);

    private:
        static constexpr ComponentType m_type = ComponentType::lut;
        std::unique_ptr<GateTypeComponent> m_component;

        std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> m_async_set_reset_behavior = {AsyncSetResetBehavior::undef, AsyncSetResetBehavior::undef};
    };
}    // namespace hal