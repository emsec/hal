#include "netlist/gate_library/gate_type/gate_type_sequential.h"

#include "core/log.h"

namespace hal
{
    GateTypeSequential::GateTypeSequential(const std::string& name, BaseType bt) : GateType(name)
    {
        m_base_type = bt;
        assert(m_base_type == BaseType::ff || m_base_type == BaseType::latch);
    }

    void GateTypeSequential::add_state_output_pin(std::string pin_name)
    {
        if (std::find(m_output_pins.begin(), m_output_pins.end(), pin_name) == m_output_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring state output pin assignment", pin_name, m_name);
            return;
        }

        m_state_pins.insert(pin_name);
    }

    void GateTypeSequential::add_inverted_state_output_pin(std::string pin_name)
    {
        if (std::find(m_output_pins.begin(), m_output_pins.end(), pin_name) == m_output_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring inverted state output pin assignment", pin_name, m_name);
            return;
        }

        m_inverted_state_pins.insert(pin_name);
    }

    void GateTypeSequential::add_clock_pin(std::string pin_name)
    {
        if (std::find(m_input_pins.begin(), m_input_pins.end(), pin_name) == m_input_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an input pin, ignoring clock input pin assignment", pin_name, m_name);
            return;
        }

        m_clock_pins.insert(pin_name);
    }

    void GateTypeSequential::set_set_reset_behavior(SetResetBehavior sb1, SetResetBehavior sb2)
    {
        m_set_reset_behavior = {sb1, sb2};
    }

    void GateTypeSequential::set_init_data_category(const std::string& category)
    {
        m_init_data_category = category;
    }

    void GateTypeSequential::set_init_data_identifier(const std::string& identifier)
    {
        m_init_data_identifier = identifier;
    }

    std::unordered_set<std::string> GateTypeSequential::get_state_output_pins() const
    {
        return m_state_pins;
    }

    std::unordered_set<std::string> GateTypeSequential::get_inverted_state_output_pins() const
    {
        return m_inverted_state_pins;
    }

    std::unordered_set<std::string> GateTypeSequential::get_clock_pins() const
    {
        return m_clock_pins;
    }

    std::pair<GateTypeSequential::SetResetBehavior, GateTypeSequential::SetResetBehavior> GateTypeSequential::get_set_reset_behavior() const
    {
        return m_set_reset_behavior;
    }

    std::string GateTypeSequential::get_init_data_category() const
    {
        return m_init_data_category;
    }

    std::string GateTypeSequential::get_init_data_identifier() const
    {
        return m_init_data_identifier;
    }
}    // namespace hal
