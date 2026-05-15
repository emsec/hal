#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

#include "hal_core/utilities/log.h"

#include <iomanip>
#include <limits>
#include <sstream>

namespace hal
{
    LUTComponent::LUTComponent(bool init_ascending) : m_init_ascending(init_ascending)
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

    void LUTComponent::add_output_pin_config(const std::string& pin_name, const std::string& init_identifier, u32 bit_offset, u32 bit_count)
    {
        if (bit_count == 0 || (bit_count & (bit_count - 1)) != 0)
        {
            log_error("lut_component", "cannot add output pin config for pin '{}': bit_count {} must be a non-zero power of two.", pin_name, bit_count);
            return;
        }
        m_output_pin_configs[pin_name] = {init_identifier, bit_offset, bit_count};
    }

    void LUTComponent::add_output_pin_config(const GatePin* pin, const std::string& init_identifier, u32 bit_offset, u32 bit_count)
    {
        if (pin == nullptr)
        {
            log_error("lut_component", "cannot add output pin config: pin is nullptr.");
            return;
        }
        add_output_pin_config(pin->get_name(), init_identifier, bit_offset, bit_count);
    }

    bool LUTComponent::remove_output_pin_config(const std::string& pin_name)
    {
        return m_output_pin_configs.erase(pin_name) > 0;
    }

    bool LUTComponent::remove_output_pin_config(const GatePin* pin)
    {
        if (pin == nullptr)
        {
            log_error("lut_component", "cannot remove output pin config: pin is nullptr.");
            return false;
        }
        return remove_output_pin_config(pin->get_name());
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
        if (bit_count == 0)
        {
            return ERR("bit_count must be non-zero");
        }

        if (full_hex.empty())
        {
            return ERR("INIT string is empty");
        }

        const size_t min_chars = (static_cast<size_t>(bit_offset) + bit_count + 3) / 4;
        if (full_hex.size() < min_chars)
        {
            return ERR("INIT string '" + full_hex + "' is too short: need at least " + std::to_string(min_chars) + " hex char(s) to cover slice [" + std::to_string(bit_offset) + ", "
                       + std::to_string(bit_offset + bit_count) + "), got " + std::to_string(full_hex.size()));
        }

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
            return ERR("bit_count must be non-zero");
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

        const int width = full_hex.empty() ? static_cast<int>((bit_offset + bit_count + 3) / 4) : static_cast<int>(full_hex.size());

        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << spliced;
        return OK(ss.str());
    }
}    // namespace hal