#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{
    FFComponent::FFComponent(std::unique_ptr<GateTypeComponent> component,
                             const BooleanFunction& next_state_bf,
                             const BooleanFunction& clock_bf,
                             const BooleanFunction& async_reset_bf,
                             const BooleanFunction& async_set_bf,
                             const AsyncSetResetBehavior behav_state,
                             const AsyncSetResetBehavior behav_neg_state)
        : m_component(std::move(component)), m_next_state_bf(next_state_bf.clone()), m_clock_bf(clock_bf.clone()), m_async_reset_bf(async_reset_bf.clone()), m_async_set_bf(async_set_bf.clone()),
          m_async_set_reset_behavior(std::make_pair(behav_state, behav_neg_state))
    {
    }

    FFComponent::ComponentType FFComponent::get_type() const
    {
        return m_type;
    }

    bool FFComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::vector<GateTypeComponent*> FFComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        if (m_component != nullptr)
        {
            std::vector<GateTypeComponent*> res = m_component->get_components(filter);
            if (filter)
            {
                if (filter(m_component.get()))
                {
                    res.push_back(m_component.get());
                }
            }
            else
            {
                res.push_back(m_component.get());
            }

            return res;
        }

        return {};
    }

    BooleanFunction FFComponent::get_next_state_function() const
    {
        return m_next_state_bf.clone();
    }

    void FFComponent::set_next_state_function(const BooleanFunction& next_state_bf)
    {
        m_next_state_bf = next_state_bf.clone();
    }

    BooleanFunction FFComponent::get_clock_function() const
    {
        return m_clock_bf.clone();
    }

    void FFComponent::set_clock_function(const BooleanFunction& clock_bf)
    {
        m_clock_bf = clock_bf.clone();
    }

    BooleanFunction FFComponent::get_async_reset_function() const
    {
        return m_async_reset_bf.clone();
    }

    void FFComponent::set_async_reset_function(const BooleanFunction& async_reset_bf)
    {
        m_async_reset_bf = async_reset_bf.clone();
    }

    BooleanFunction FFComponent::get_async_set_function() const
    {
        return m_async_set_bf.clone();
    }

    void FFComponent::set_async_set_function(const BooleanFunction& async_set_bf)
    {
        m_async_set_bf = async_set_bf.clone();
    }

    const std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior>& FFComponent::get_async_set_reset_behavior() const
    {
        return m_async_set_reset_behavior;
    }

    void FFComponent::set_async_set_reset_behavior(AsyncSetResetBehavior behav_state, AsyncSetResetBehavior behav_neg_state)
    {
        m_async_set_reset_behavior = std::make_pair(behav_state, behav_neg_state);
    }

}    // namespace hal