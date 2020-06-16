#include "netlist/gate_library/gate_type/gate_type.h"

#include "core/log.h"

namespace hal
{
    GateType::GateType(const std::string& name)
    {
        static u32 next_id = 1;

        m_id        = next_id++;
        m_name      = name;
        m_base_type = BaseType::combinatorial;
    }

    u32 GateType::get_id() const
    {
        return m_id;
    }

    std::string GateType::to_string() const
    {
        return m_name;
    }

    std::ostream& operator<<(std::ostream& os, const GateType& gt)
    {
        return os << gt.to_string();
    }

    bool GateType::operator==(const GateType& other) const
    {
        return m_id == other.get_id();
    }

    bool GateType::operator!=(const GateType& other) const
    {
        return !(*this == other);
    }

    void GateType::add_input_pin(std::string pin_name)
    {
        if (const auto& it = std::find(m_input_pins.begin(), m_input_pins.end(), pin_name); it != m_input_pins.end())
        {
            log_warning("gate_type", "input pin '{}' does already exist for gate type '{}', ignoring pin", pin_name, m_name);
            return;
        }

        m_input_pins.push_back(pin_name);
    }

    void GateType::add_input_pins(const std::vector<std::string>& pin_names)
    {
        for (const auto& pin_name : pin_names)
        {
            if (const auto& it = std::find(m_input_pins.begin(), m_input_pins.end(), pin_name); it != m_input_pins.end())
            {
                log_warning("gate_type", "input pin '{}' does already exist for gate type '{}', ignoring pin", pin_name, m_name);
                continue;
            }

            m_input_pins.push_back(pin_name);
        }
    }

    void GateType::add_output_pin(std::string pin_name)
    {
        if (const auto& it = std::find(m_output_pins.begin(), m_output_pins.end(), pin_name); it != m_output_pins.end())
        {
            log_warning("gate_type", "output pin '{}' does already exist for gate type '{}', ignoring pin", pin_name, m_name);
            return;
        }

        m_output_pins.push_back(pin_name);
    }

    void GateType::add_output_pins(const std::vector<std::string>& pin_names)
    {
        for (const auto& pin_name : pin_names)
        {
            if (const auto& it = std::find(m_output_pins.begin(), m_output_pins.end(), pin_name); it != m_output_pins.end())
            {
                log_warning("gate_type", "output pin '{}' does already exist for gate type '{}', ignoring pin", pin_name, m_name);
                continue;
            }

            m_output_pins.push_back(pin_name);
        }
    }

    void GateType::assign_input_pin_group(const std::string& group_name, const std::map<u32, std::string>& index_to_pin)
    {
        if (const auto& it = m_input_pin_groups.find(group_name); it != m_input_pin_groups.end())
        {
            log_warning("gate_type", "input pin group '{}' does already exist for gate type '{}', ignoring group", group_name, m_name);
            return;
        }

        for (const auto& pin : index_to_pin)
        {
            if (std::find(m_input_pins.begin(), m_input_pins.end(), pin.second) == m_input_pins.end())
            {
                log_warning("gate_type", "input pin '{}' does not yet exist within pin group '{}' for gate type '{}', ignoring group", pin.first, group_name, m_name);
                return;
            }
        }

        m_input_pin_groups.emplace(group_name, index_to_pin);
    }

    void GateType::assign_input_pin_groups(const std::map<std::string, std::map<u32, std::string>>& pin_groups)
    {
        for (const auto& [group_name, index_to_pin] : pin_groups)
        {
            assign_input_pin_group(group_name, index_to_pin);
        }
    }

    void GateType::assign_output_pin_group(const std::string& group_name, const std::map<u32, std::string>& index_to_pin)
    {
        if (const auto& it = m_output_pin_groups.find(group_name); it != m_output_pin_groups.end())
        {
            log_warning("gate_type", "output pin group '{}' does already exist for gate type '{}', ignoring group", group_name, m_name);
            return;
        }

        for (const auto& pin : index_to_pin)
        {
            if (std::find(m_output_pins.begin(), m_output_pins.end(), pin.second) == m_output_pins.end())
            {
                log_warning("gate_type", "output pin '{}' does not yet exist within pin group '{}' for gate type '{}', ignoring group", pin.first, group_name, m_name);
                return;
            }
        }

        m_output_pin_groups.emplace(group_name, index_to_pin);
    }

    void GateType::assign_output_pin_groups(const std::map<std::string, std::map<u32, std::string>>& pin_groups)
    {
        for (const auto& [group_name, index_to_pin] : pin_groups)
        {
            assign_output_pin_group(group_name, index_to_pin);
        }
    }

    void GateType::add_boolean_function(std::string pin_name, BooleanFunction bf)
    {
        m_functions.emplace(pin_name, bf);
    }

    void GateType::add_boolean_functions(const std::map<std::string, BooleanFunction>& functions)
    {
        m_functions.insert(functions.begin(), functions.end());
    }

    std::string GateType::get_name() const
    {
        return m_name;
    }

    GateType::BaseType GateType::get_base_type() const
    {
        return m_base_type;
    }

    std::vector<std::string> GateType::get_input_pins() const
    {
        return m_input_pins;
    }

    std::map<std::string, std::map<u32, std::string>> GateType::get_input_pin_groups() const
    {
        return m_input_pin_groups;
    }

    std::vector<std::string> GateType::get_output_pins() const
    {
        return m_output_pins;
    }

    std::map<std::string, std::map<u32, std::string>> GateType::get_output_pin_groups() const
    {
        return m_output_pin_groups;
    }

    std::unordered_map<std::string, BooleanFunction> GateType::get_boolean_functions() const
    {
        return m_functions;
    }
}    // namespace hal
