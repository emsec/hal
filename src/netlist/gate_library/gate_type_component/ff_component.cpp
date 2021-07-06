#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

#include "hal_core/netlist/gate_library/gate_type.h"

namespace hal
{
    FFComponent::FFComponent(std::unique_ptr<GateTypeComponent> component, GateType* gate_type, const BooleanFunction& next_state_bf, const BooleanFunction& clock_bf)
        : m_component(std::move(component))
    {
        this->set_next_state_function(gate_type, next_state_bf);
        this->set_clock_function(gate_type, clock_bf);
    }

    FFComponent::ComponentType FFComponent::get_type() const
    {
        return m_type;
    }

    bool FFComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
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

    BooleanFunction FFComponent::get_next_state_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("next_state");
    }

    void FFComponent::set_next_state_function(GateType* gate_type, const BooleanFunction& next_state_bf)
    {
        gate_type->add_boolean_function("next_state", next_state_bf);
    }

    BooleanFunction FFComponent::get_clock_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("clocked_on");
    }

    void FFComponent::set_clock_function(GateType* gate_type, const BooleanFunction& clock_bf)
    {
        gate_type->add_boolean_function("clocked_on", clock_bf);
    }

    BooleanFunction FFComponent::get_async_reset_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("async_reset_on");
    }

    void FFComponent::set_async_reset_function(GateType* gate_type, const BooleanFunction& async_reset_bf)
    {
        gate_type->add_boolean_function("async_reset_on", async_reset_bf);
    }

    BooleanFunction FFComponent::get_async_set_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("async_set_on");
    }

    void FFComponent::set_async_set_function(GateType* gate_type, const BooleanFunction& async_set_bf)
    {
        gate_type->add_boolean_function("async_set_on", async_set_bf);
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