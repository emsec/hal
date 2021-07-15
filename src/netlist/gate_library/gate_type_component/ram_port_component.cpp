#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"

namespace hal
{
    RAMPortComponent::RAMPortComponent(std::unique_ptr<GateTypeComponent> component) : m_component(std::move(component))
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

    std::set<GateTypeComponent*> RAMPortComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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

    const std::string& RAMPortComponent::get_write_data_group() const
    {
        return m_write_data_group;
    }

    void RAMPortComponent::set_write_data_group(const std::string& write_data_group)
    {
        m_write_data_group = write_data_group;
    }

    const std::string& RAMPortComponent::get_read_data_group() const
    {
        return m_read_data_group;
    }

    void RAMPortComponent::set_read_data_group(const std::string& read_data_group)
    {
        m_read_data_group = read_data_group;
    }

    const std::string& RAMPortComponent::get_write_address_group() const
    {
        return m_write_addr_group;
    }

    void RAMPortComponent::set_write_address_group(const std::string& write_addr_group)
    {
        m_write_addr_group = write_addr_group;
    }

    const std::string& RAMPortComponent::get_read_address_group() const
    {
        return m_read_addr_group;
    }

    void RAMPortComponent::set_read_address_group(const std::string& read_addr_group)
    {
        m_read_addr_group = read_addr_group;
    }

    const BooleanFunction& RAMPortComponent::get_write_clock_function() const
    {
        return m_write_clock_bf;
    }

    void RAMPortComponent::set_write_clock_function(const BooleanFunction& write_clock_bf)
    {
        m_write_clock_bf = write_clock_bf;
    }

    const BooleanFunction& RAMPortComponent::get_read_clock_function() const
    {
        return m_read_clock_bf;
    }

    void RAMPortComponent::set_read_clock_function(const BooleanFunction& read_clock_bf)
    {
        m_read_clock_bf = read_clock_bf;
    }

    const BooleanFunction& RAMPortComponent::get_write_enable_function() const
    {
        return m_write_enable_bf;
    }

    void RAMPortComponent::set_write_enable_function(const BooleanFunction& write_enable_bf)
    {
        m_write_enable_bf = write_enable_bf;
    }

    const BooleanFunction& RAMPortComponent::get_read_enable_function() const
    {
        return m_read_enable_bf;
    }

    void RAMPortComponent::set_read_enable_function(const BooleanFunction& read_enable_bf)
    {
        m_read_enable_bf = read_enable_bf;
    }
}    // namespace hal