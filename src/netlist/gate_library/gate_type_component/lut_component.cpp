#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

namespace hal
{
    LUTComponent::LUTComponent(std::unique_ptr<GateTypeComponent> component, bool init_ascending) : m_component(std::move(component)), m_init_ascending(init_ascending)
    {
    }

    LUTComponent::ComponentType LUTComponent::get_type() const
    {
        return m_type;
    }

    bool LUTComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::set<GateTypeComponent*> LUTComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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

    bool LUTComponent::is_init_ascending() const
    {
        return m_init_ascending;
    }

    void LUTComponent::set_init_ascending(bool ascending)
    {
        m_init_ascending = ascending;
    }
}    // namespace hal