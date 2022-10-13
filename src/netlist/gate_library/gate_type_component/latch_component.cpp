#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"

namespace hal
{
    LatchComponent::LatchComponent(std::unique_ptr<GateTypeComponent> component) : m_component(std::move(component))
    {
    }

    LatchComponent::ComponentType LatchComponent::get_type() const
    {
        return m_type;
    }

    bool LatchComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::vector<GateTypeComponent*> LatchComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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

    BooleanFunction LatchComponent::get_data_in_function() const
    {
        return m_data_in_bf.clone();
    }

    void LatchComponent::set_data_in_function(const BooleanFunction& data_in_bf)
    {
        m_data_in_bf = data_in_bf.clone();
    }

    BooleanFunction LatchComponent::get_enable_function() const
    {
        return m_enable_bf.clone();
    }

    void LatchComponent::set_enable_function(const BooleanFunction& enable_bf)
    {
        m_enable_bf = enable_bf.clone();
    }

    BooleanFunction LatchComponent::get_async_reset_function() const
    {
        return m_async_reset_bf.clone();
    }

    void LatchComponent::set_async_reset_function(const BooleanFunction& async_reset_bf)
    {
        m_async_reset_bf = async_reset_bf.clone();
    }

    BooleanFunction LatchComponent::get_async_set_function() const
    {
        return m_async_set_bf.clone();
    }

    void LatchComponent::set_async_set_function(const BooleanFunction& async_set_bf)
    {
        m_async_set_bf = async_set_bf.clone();
    }

    const std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior>& LatchComponent::get_async_set_reset_behavior() const
    {
        return m_async_set_reset_behavior;
    }

    void LatchComponent::set_async_set_reset_behavior(AsyncSetResetBehavior behav_state, AsyncSetResetBehavior behav_neg_state)
    {
        m_async_set_reset_behavior = std::make_pair(behav_state, behav_neg_state);
    }

}    // namespace hal