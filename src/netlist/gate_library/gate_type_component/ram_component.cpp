#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"

namespace hal
{
    RAMComponent::RAMComponent(std::unique_ptr<GateTypeComponent> component) : m_component(std::move(component))
    {
    }

    RAMComponent::ComponentType RAMComponent::get_type() const
    {
        return m_type;
    }

    bool RAMComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> RAMComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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
}    // namespace hal