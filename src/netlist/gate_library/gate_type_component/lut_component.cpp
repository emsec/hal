#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <sstream>

namespace hal
{
    LUTComponent::LUTComponent(std::unordered_map<std::string, LUTOutputConfig> configs) : m_output_pin_configs(std::move(configs))
    {
    }

    std::unique_ptr<LUTComponent> LUTComponent::create(std::unordered_map<std::string, LUTOutputConfig> configs)
    {
        for (auto it = configs.begin(); it != configs.end();)
        {
            const u32 bc = it->second.bit_count;
            if (bc == 0 || (bc & (bc - 1)) != 0)
            {
                log_error("lut_component", "cannot add output pin config for pin '{}': bit_count {} must be a non-zero power of two — skipping.", it->first, bc);
                it = configs.erase(it);
            }
            else if (it->second.input_pins.empty())
            {
                log_error("lut_component", "cannot add output pin config for pin '{}': input_pins must be non-empty — skipping.", it->first);
                it = configs.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return std::unique_ptr<LUTComponent>(new LUTComponent(std::move(configs)));
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

    namespace
    {
        std::string lut_param_read(const Gate* gate, const std::string& key)
        {
            if (!gate->has_parameter(key))
            {
                return "";
            }
            const std::string pv = gate->get_parameter_value(key).get();
            return (pv.size() >= 2 && pv[0] == '0' && pv[1] == 'x') ? pv.substr(2) : pv;
        }
    }    // namespace

    Result<std::string> LUTComponent::get_init_string(const Gate* gate, const std::string& pin_name) const
    {
        if (gate == nullptr)
        {
            return ERR("gate is nullptr");
        }
        const GatePin* pin = gate->get_type()->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            return ERR("pin '" + pin_name + "' does not exist on gate type '" + gate->get_type()->get_name() + "'");
        }
        return get_init_string(gate, pin);
    }

    Result<std::string> LUTComponent::get_init_string(const Gate* gate, const GatePin* pin) const
    {
        if (gate == nullptr)
        {
            return ERR("gate is nullptr");
        }
        if (pin == nullptr)
        {
            return ERR("pin is nullptr");
        }
        if (pin->get_type() != PinType::lut)
        {
            return ERR("pin '" + pin->get_name() + "' is not of type PinType::lut");
        }
        const PinDirection dir = pin->get_direction();
        if (dir != PinDirection::output && dir != PinDirection::internal)
        {
            return ERR("pin '" + pin->get_name() + "' must have direction output or internal");
        }
        const LUTOutputConfig* cfg = get_output_pin_config(pin->get_name());
        if (cfg == nullptr)
        {
            return ERR("pin '" + pin->get_name() + "' has no INIT config");
        }
        const std::string full_str = lut_param_read(gate, cfg->init_identifier);
        auto res                   = extract_init_slice(full_str, cfg->bit_offset, cfg->bit_count);
        if (res.is_error())
        {
            return ERR("could not get INIT string for gate '" + gate->get_name() + "': " + res.get_error().get());
        }
        return OK(res.get());
    }

    Result<std::monostate> LUTComponent::set_init_string(Gate* gate, const std::string& pin_name, const std::string& hex) const
    {
        if (gate == nullptr)
        {
            return ERR("gate is nullptr");
        }
        const GatePin* pin = gate->get_type()->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            return ERR("pin '" + pin_name + "' does not exist on gate type '" + gate->get_type()->get_name() + "'");
        }
        return set_init_string(gate, pin, hex);
    }

    Result<std::monostate> LUTComponent::set_init_string(Gate* gate, const GatePin* pin, const std::string& hex) const
    {
        if (gate == nullptr)
        {
            return ERR("gate is nullptr");
        }
        if (pin == nullptr)
        {
            return ERR("pin is nullptr");
        }
        if (pin->get_type() != PinType::lut)
        {
            return ERR("pin '" + pin->get_name() + "' is not of type PinType::lut");
        }
        const PinDirection dir = pin->get_direction();
        if (dir != PinDirection::output && dir != PinDirection::internal)
        {
            return ERR("pin '" + pin->get_name() + "' must have direction output or internal");
        }
        const LUTOutputConfig* cfg = get_output_pin_config(pin->get_name());
        if (cfg == nullptr)
        {
            return ERR("pin '" + pin->get_name() + "' has no INIT config");
        }
        if (hex.empty() || !std::all_of(hex.begin(), hex.end(), [](unsigned char c) { return std::isxdigit(c); }))
        {
            return ERR("'" + hex + "' is not a valid hex string");
        }
        const size_t expected_chars = (static_cast<size_t>(cfg->bit_count) + 3) / 4;
        if (hex.size() != expected_chars)
        {
            return ERR("hex string '" + hex + "' has wrong length: expected " + std::to_string(expected_chars) + " characters for " + std::to_string(cfg->bit_count) + " bits");
        }

        const std::string existing_str = lut_param_read(gate, cfg->init_identifier);
        auto res                       = splice_init_slice(existing_str, hex, cfg->bit_offset, cfg->bit_count);
        if (res.is_error())
        {
            return ERR("could not set INIT string for gate '" + gate->get_name() + "': " + res.get_error().get());
        }

        const std::string spliced_str = res.get();
        const std::string value       = "0x" + spliced_str;
        const size_t nbits            = spliced_str.size() * 4;
        auto param_res                = Parameter::BitVector(cfg->init_identifier, static_cast<u16>(nbits), "");
        if (param_res.is_error())
        {
            return ERR("could not create parameter: " + param_res.get_error().get());
        }
        gate->set_parameter(param_res.get(), value);
        return OK({});
    }
}    // namespace hal
