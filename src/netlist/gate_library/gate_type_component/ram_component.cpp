#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"

namespace hal
{
    RAMComponent::RAMComponent(std::unique_ptr<GateTypeComponent> component, const u32 bit_size) : m_component(std::move(component)), m_bit_size(bit_size)
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

    std::vector<GateTypeComponent*> RAMComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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

    u32 RAMComponent::get_bit_size() const
    {
        return m_bit_size;
    }

    void RAMComponent::set_bit_size(const u32 bit_size)
    {
        m_bit_size = bit_size;
    }
}    // namespace hal