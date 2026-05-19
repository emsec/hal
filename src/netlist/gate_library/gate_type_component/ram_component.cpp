#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"

#include <memory>

namespace hal
{
    RAMComponent::RAMComponent(std::unique_ptr<GateTypeComponent> component, const u32 bit_size, std::vector<std::string> init_identifiers)
        : m_component(std::move(component)), m_bit_size(bit_size), m_init_identifiers(std::move(init_identifiers))
    {
    }

    std::unique_ptr<RAMComponent> RAMComponent::create(std::unique_ptr<GateTypeComponent> component, const u32 bit_size, const std::vector<std::string>& init_identifiers)
    {
        return std::unique_ptr<RAMComponent>(new RAMComponent(std::move(component), bit_size, init_identifiers));
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

    const std::vector<std::string>& RAMComponent::get_init_identifiers() const
    {
        return m_init_identifiers;
    }

    void RAMComponent::set_init_identifiers(const std::vector<std::string>& init_identifiers)
    {
        m_init_identifiers = init_identifiers;
    }
}    // namespace hal