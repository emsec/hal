#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"

namespace hal
{
    StateComponent::StateComponent(std::unique_ptr<GateTypeComponent> component, const std::string& state_identifier, const std::string& neg_state_identifier)
        : m_component(std::move(component)), m_state_identifier(state_identifier), m_neg_state_identifier(neg_state_identifier)
    {
    }

    StateComponent::ComponentType StateComponent::get_type() const
    {
        return m_type;
    }

    bool StateComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::vector<GateTypeComponent*> StateComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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

    const std::string& StateComponent::get_state_identifier() const
    {
        return m_state_identifier;
    }

    void StateComponent::set_state_identifier(const std::string& state_identifier)
    {
        m_state_identifier = state_identifier;
    }

    const std::string& StateComponent::get_neg_state_identifier() const
    {
        return m_neg_state_identifier;
    }

    void StateComponent::set_neg_state_identifier(const std::string& neg_state_identifier)
    {
        m_neg_state_identifier = neg_state_identifier;
    }

}    // namespace hal