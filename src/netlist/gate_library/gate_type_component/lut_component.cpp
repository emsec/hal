#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

namespace hal
{
    GateTypeComponent::ComponentType LUTComponent::get_type() const
    {
        return ComponentType::lut;
    }

    std::set<GateTypeComponent*> LUTComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const
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