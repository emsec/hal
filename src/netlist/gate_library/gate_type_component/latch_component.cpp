#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"

#include "hal_core/netlist/gate_library/gate_type.h"

namespace hal
{
    LatchComponent::LatchComponent(std::unique_ptr<GateTypeComponent> component, GateType* gate_type, const BooleanFunction& data_in_bf, const BooleanFunction& enable_bf)
        : m_component(std::move(component))
    {
        this->set_data_in_function(gate_type, data_in_bf);
        this->set_enable_function(gate_type, enable_bf);
    }

    LatchComponent::ComponentType LatchComponent::get_type() const
    {
        return m_type;
    }

    bool LatchComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> LatchComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
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

    BooleanFunction LatchComponent::get_data_in_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("data_in");
    }

    void LatchComponent::set_data_in_function(GateType* gate_type, const BooleanFunction& data_in_bf)
    {
        gate_type->add_boolean_function("data_in", data_in_bf);
    }

    BooleanFunction LatchComponent::get_enable_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("enabled_on");
    }

    void LatchComponent::set_enable_function(GateType* gate_type, const BooleanFunction& enable_bf)
    {
        gate_type->add_boolean_function("enabled_on", enable_bf);
    }

    BooleanFunction LatchComponent::get_async_reset_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("async_reset_on");
    }

    void LatchComponent::set_async_reset_function(GateType* gate_type, const BooleanFunction& async_reset_bf)
    {
        gate_type->add_boolean_function("async_reset_on", async_reset_bf);
    }

    BooleanFunction LatchComponent::get_async_set_function(const GateType* gate_type) const
    {
        return gate_type->get_boolean_function("async_set_on");
    }

    void LatchComponent::set_async_set_function(GateType* gate_type, const BooleanFunction& async_set_bf)
    {
        gate_type->add_boolean_function("async_set_on", async_set_bf);
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