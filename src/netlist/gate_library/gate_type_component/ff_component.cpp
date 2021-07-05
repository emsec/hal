#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

#include "hal_core/netlist/gate_library/gate_type.h"

namespace hal
{
    GateTypeComponent::ComponentType FFComponent::get_type() const
    {
        return ComponentType::ff;
    }

    std::set<GateTypeComponent*> FFComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        if (m_component != nullptr)
        {
            std::set<GateTypeComponent*> res = m_component->get_components(filter);
            if (filter)
            {
                if (filter(m_component.get()))
                {
                    res.insert(m_component.get());
                }
            }
            else
            {
                res.insert(m_component.get());
            }

            return res;
        }

        return {};
    }

    BooleanFunction FFComponent::get_clock_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("clocked_on");
    }

    void FFComponent::set_clock_function(GateType* gate_type, const BooleanFunction& clk_bf)
    {
        gate_type->add_boolean_function("clocked_on", clk_bf);
    }

    BooleanFunction FFComponent::get_enable_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("enabled_on");
    }

    void FFComponent::set_enable_function(GateType* gate_type, const BooleanFunction& en_bf)
    {
        gate_type->add_boolean_function("enabled_on", en_bf);
    }

    BooleanFunction FFComponent::get_async_reset_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("async_reset_on");
    }

    void FFComponent::set_async_reset_function(GateType* gate_type, const BooleanFunction& rst_bf)
    {
        gate_type->add_boolean_function("async_reset_on", rst_bf);
    }

    BooleanFunction FFComponent::get_async_set_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("async_set_on");
    }

    void FFComponent::set_async_set_function(GateType* gate_type, const BooleanFunction& set_bf)
    {
        gate_type->add_boolean_function("async_set_on", set_bf);
    }

    const std::pair<FFComponent::AsyncSetResetBehavior, FFComponent::AsyncSetResetBehavior>& FFComponent::get_async_set_reset_behavior() const
    {
        return m_async_set_reset_behavior;
    }

    void FFComponent::set_async_set_reset_behavior(AsyncSetResetBehavior behav_state, AsyncSetResetBehavior behav_neg_state)
    {
        m_async_set_reset_behavior = std::make_pair(behav_state, behav_neg_state);
    }

}    // namespace hal