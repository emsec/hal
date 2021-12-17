#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"

namespace hal
{
    RAMPortComponent::RAMPortComponent(std::unique_ptr<GateTypeComponent> component,
                                       const std::string& data_group,
                                       const std::string& addr_group,
                                       const BooleanFunction& clock_bf,
                                       const BooleanFunction& enable_bf,
                                       bool is_write)
        : m_component(std::move(component)), m_data_group(data_group), m_addr_group(addr_group), m_clock_bf(clock_bf.clone()), m_enable_bf(enable_bf.clone()), m_is_write(is_write)
    {
    }

    RAMPortComponent::ComponentType RAMPortComponent::get_type() const
    {
        return m_type;
    }

    bool RAMPortComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::vector<GateTypeComponent*> RAMPortComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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

    const std::string& RAMPortComponent::get_data_group() const
    {
        return m_data_group;
    }

    void RAMPortComponent::set_data_group(const std::string& data_group)
    {
        m_data_group = data_group;
    }

    const std::string& RAMPortComponent::get_address_group() const
    {
        return m_addr_group;
    }

    void RAMPortComponent::set_address_group(const std::string& addr_group)
    {
        m_addr_group = addr_group;
    }

    const BooleanFunction& RAMPortComponent::get_clock_function() const
    {
        return m_clock_bf;
    }

    void RAMPortComponent::set_clock_function(const BooleanFunction& clock_bf)
    {
        m_clock_bf = clock_bf.clone();
    }

    const BooleanFunction& RAMPortComponent::get_enable_function() const
    {
        return m_enable_bf;
    }

    void RAMPortComponent::set_enable_function(const BooleanFunction& enable_bf)
    {
        m_enable_bf = enable_bf.clone();
    }

    bool RAMPortComponent::is_write_port() const
    {
        return m_is_write;
    }

    void RAMPortComponent::set_write_port(bool is_write)
    {
        m_is_write = is_write;
    }
}    // namespace hal