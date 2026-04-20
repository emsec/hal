#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

#include "hal_core/utilities/log.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>

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

    Result<std::string> LUTComponent::extract_init_slice(const std::string& full_hex, u32 bit_offset, u32 bit_count)
    {
        if (bit_count == 0 || full_hex.empty())
            return OK(full_hex);

        u64 full_val = 0;
        try
        {
            full_val = std::stoull(full_hex, nullptr, 16);
        }
        catch (const std::invalid_argument&)
        {
            return ERR("INIT string '" + full_hex + "' is not a valid hex value");
        }
        catch (const std::out_of_range&)
        {
            return ERR("INIT string '" + full_hex + "' is out of range for u64");
        }

        const u64 mask   = (bit_count >= 64) ? std::numeric_limits<u64>::max() : ((1ULL << bit_count) - 1);
        const u64 sliced = (full_val >> bit_offset) & mask;

        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(static_cast<int>((bit_count + 3) / 4)) << sliced;
        return OK(ss.str());
    }

    Result<std::string> LUTComponent::splice_init_slice(const std::string& full_hex, const std::string& slice_hex, u32 bit_offset, u32 bit_count)
    {
        if (bit_count == 0)
        {
            std::string upper = slice_hex;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            return OK(upper);
        }

        u64 existing = 0;
        if (!full_hex.empty())
        {
            try
            {
                existing = std::stoull(full_hex, nullptr, 16);
            }
            catch (const std::invalid_argument&)
            {
                return ERR("full INIT string '" + full_hex + "' is not a valid hex value");
            }
            catch (const std::out_of_range&)
            {
                return ERR("full INIT string '" + full_hex + "' is out of range for u64");
            }
        }

        u64 new_slice = 0;
        try
        {
            new_slice = std::stoull(slice_hex, nullptr, 16);
        }
        catch (const std::invalid_argument&)
        {
            return ERR("slice value '" + slice_hex + "' is not a valid hex value");
        }
        catch (const std::out_of_range&)
        {
            return ERR("slice value '" + slice_hex + "' is out of range for u64");
        }

        const u64 mask    = (bit_count >= 64) ? std::numeric_limits<u64>::max() : ((1ULL << bit_count) - 1);
        const u64 spliced = (existing & ~(mask << bit_offset)) | ((new_slice & mask) << bit_offset);

        const int width = full_hex.empty()
                              ? static_cast<int>((bit_offset + bit_count + 3) / 4)
                              : static_cast<int>(full_hex.size());

        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << spliced;
        return OK(ss.str());
    }
}    // namespace hal