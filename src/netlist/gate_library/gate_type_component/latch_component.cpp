#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"

#include "hal_core/netlist/gate_library/gate_type.h"

namespace hal
{
    LatchComponent::LatchComponent(const BooleanFunction& data_in_bf, const BooleanFunction& enable_bf) : m_data_in_bf(data_in_bf), m_enable_bf(enable_bf)
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

    std::set<GateTypeComponent*> LatchComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        UNUSED(filter);
        return {};
    }

    BooleanFunction LatchComponent::get_data_in_function() const
    {
        return m_data_in_bf;
    }

    void LatchComponent::set_data_in_function(const BooleanFunction& data_in_bf)
    {
        m_data_in_bf = data_in_bf;
    }

    BooleanFunction LatchComponent::get_enable_function() const
    {
        return m_enable_bf;
    }

    void LatchComponent::set_enable_function(const BooleanFunction& enable_bf)
    {
        m_enable_bf = enable_bf;
    }

    BooleanFunction LatchComponent::get_async_reset_function() const
    {
        return m_async_reset_bf;
    }

    void LatchComponent::set_async_reset_function(const BooleanFunction& async_reset_bf)
    {
        m_async_reset_bf = async_reset_bf;
    }

    BooleanFunction LatchComponent::get_async_set_function() const
    {
        return m_async_set_bf;
    }

    void LatchComponent::set_async_set_function(const BooleanFunction& async_set_bf)
    {
        m_async_set_bf = async_set_bf;
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