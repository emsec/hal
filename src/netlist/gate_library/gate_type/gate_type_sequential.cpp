#include "hal_core/netlist/gate_library/gate_type/gate_type_sequential.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    GateTypeSequential::GateTypeSequential(const std::string& name, BaseType bt) : GateType(name)
    {
        m_base_type = bt;
        assert(m_base_type == BaseType::ff || m_base_type == BaseType::latch);
    }

    void GateTypeSequential::add_state_pin(const std::string& pin_name)
    {
        if (std::find(m_output_pins.begin(), m_output_pins.end(), pin_name) == m_output_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring state output pin assignment", pin_name, m_name);
            return;
        }

        m_state_pins.insert(pin_name);
    }

    std::unordered_set<std::string> GateTypeSequential::get_state_pins() const
    {
        return m_state_pins;
    }

    void GateTypeSequential::add_negated_state_pin(const std::string& pin_name)
    {
        if (std::find(m_output_pins.begin(), m_output_pins.end(), pin_name) == m_output_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring negated state output pin assignment", pin_name, m_name);
            return;
        }

        m_negated_state_pins.insert(pin_name);
    }

    std::unordered_set<std::string> GateTypeSequential::get_negated_state_pins() const
    {
        return m_negated_state_pins;
    }

    void GateTypeSequential::add_clock_pin(const std::string& pin_name)
    {
        if (std::find(m_input_pins.begin(), m_input_pins.end(), pin_name) == m_input_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an input pin, ignoring clock input pin assignment", pin_name, m_name);
            return;
        }

        m_clock_pins.insert(pin_name);
    }

    std::unordered_set<std::string> GateTypeSequential::get_clock_pins() const
    {
        return m_clock_pins;
    }

    void GateTypeSequential::add_enable_pin(const std::string& pin_name)
    {
        if (std::find(m_input_pins.begin(), m_input_pins.end(), pin_name) == m_input_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an input pin, ignoring enable input pin assignment", pin_name, m_name);
            return;
        }

        m_enable_pins.insert(pin_name);
    }

    std::unordered_set<std::string> GateTypeSequential::get_enable_pins() const
    {
        return m_enable_pins;
    }

    void GateTypeSequential::add_reset_pin(const std::string& pin_name)
    {
        if (std::find(m_input_pins.begin(), m_input_pins.end(), pin_name) == m_input_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an input pin, ignoring reset input pin assignment", pin_name, m_name);
            return;
        }

        m_reset_pins.insert(pin_name);
    }

    std::unordered_set<std::string> GateTypeSequential::get_reset_pins() const
    {
        return m_reset_pins;
    }

    void GateTypeSequential::add_set_pin(const std::string& pin_name)
    {
        if (std::find(m_input_pins.begin(), m_input_pins.end(), pin_name) == m_input_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an input pin, ignoring set input pin assignment", pin_name, m_name);
            return;
        }

        m_set_pins.insert(pin_name);
    }

    std::unordered_set<std::string> GateTypeSequential::get_set_pins() const
    {
        return m_set_pins;
    }

    void GateTypeSequential::add_data_pin(const std::string& pin_name)
    {
        if (std::find(m_input_pins.begin(), m_input_pins.end(), pin_name) == m_input_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an input pin, ignoring data input pin assignment", pin_name, m_name);
            return;
        }

        m_data_pins.insert(pin_name);
    }

    std::unordered_set<std::string> GateTypeSequential::get_data_pins() const
    {
        return m_data_pins;
    }

    void GateTypeSequential::set_clear_preset_behavior(ClearPresetBehavior cp1, ClearPresetBehavior cp2)
    {
        m_clear_preset_behavior = {cp1, cp2};
    }

    std::pair<GateTypeSequential::ClearPresetBehavior, GateTypeSequential::ClearPresetBehavior> GateTypeSequential::get_clear_preset_behavior() const
    {
        return m_clear_preset_behavior;
    }

    void GateTypeSequential::set_init_data_category(const std::string& category)
    {
        m_init_data_category = category;
    }

    std::string GateTypeSequential::get_init_data_category() const
    {
        return m_init_data_category;
    }

    void GateTypeSequential::set_init_data_identifier(const std::string& identifier)
    {
        m_init_data_identifier = identifier;
    }

    std::string GateTypeSequential::get_init_data_identifier() const
    {
        return m_init_data_identifier;
    }
}    // namespace hal
