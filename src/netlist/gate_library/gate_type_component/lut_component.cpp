#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

#include "hal_core/utilities/log.h"

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

    std::vector<GateTypeComponent*> LUTComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
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

    bool LUTComponent::is_init_ascending() const
    {
        return m_init_ascending;
    }

    void LUTComponent::set_init_ascending(bool ascending)
    {
        m_init_ascending = ascending;
    }

    void LUTComponent::set_output_pin_config(const std::string& pin_name, const std::string& init_identifier, u32 bit_offset, u32 bit_count)
    {
        if (bit_count != 0 && (bit_count & (bit_count - 1)) != 0)
        {
            log_error("lut_component", "cannot set output pin config for pin '{}': bit_count {} is not a power of two.", pin_name, bit_count);
            return;
        }
        m_output_pin_configs[pin_name] = {init_identifier, bit_offset, bit_count};
    }

    const LUTComponent::LUTOutputConfig* LUTComponent::get_output_pin_config(const std::string& pin_name) const
    {
        if (auto it = m_output_pin_configs.find(pin_name); it != m_output_pin_configs.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    const std::unordered_map<std::string, LUTComponent::LUTOutputConfig>& LUTComponent::get_output_pin_configs() const
    {
        return m_output_pin_configs;
    }
}    // namespace hal