#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"

namespace hal
{
    RAMComponent::RAMComponent(std::vector<std::unique_ptr<GateTypeComponent>> components) : m_components(std::move(components))
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

    std::set<GateTypeComponent*> RAMComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
    {
        if (!m_components.empty())
        {
            std::set<GateTypeComponent*> res;

            for (const auto& component : m_components)
            {
                std::set<GateTypeComponent*> sub_components = component->get_components(filter);
                res.insert(sub_components.begin(), sub_components.end());

                if (filter)
                {
                    if (filter(component.get()))
                    {
                        res.insert(component.get());
                    }
                }
                else
                {
                    res.insert(component.get());
                }
            }

            return res;
        }

        return {};
    }
}    // namespace hal