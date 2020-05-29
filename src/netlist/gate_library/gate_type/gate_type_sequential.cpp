#include "netlist/gate_library/gate_type/gate_type_sequential.h"

#include "core/log.h"

gate_type_sequential::gate_type_sequential(const std::string& name, base_type bt) : gate_type(name)
{
    m_base_type = bt;
    assert(m_base_type == base_type::ff || m_base_type == base_type::latch);
}

void gate_type_sequential::add_state_output_pin(std::string pin_name)
{
    if (const auto& it = std::find(m_input_pins.begin(), m_input_pins.end(), pin_name); it != m_input_pins.end())
    {
        log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring state output pin assignment", pin_name, m_name);
        return;
    }

    m_state_pins.insert(pin_name);
}

void gate_type_sequential::add_inverted_state_output_pin(std::string pin_name)
{
    if (const auto& it = std::find(m_input_pins.begin(), m_input_pins.end(), pin_name); it != m_input_pins.end())
    {
        log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring state output pin assignment", pin_name, m_name);
        return;
    }

    m_inverted_state_pins.insert(pin_name);
}

void gate_type_sequential::set_set_reset_behavior(set_reset_behavior sb1, set_reset_behavior sb2)
{
    m_set_reset_behavior = {sb1, sb2};
}

void gate_type_sequential::set_init_data_category(const std::string& category)
{
    m_init_data_category = category;
}

void gate_type_sequential::set_init_data_identifier(const std::string& identifier)
{
    m_init_data_identifier = identifier;
}

std::unordered_set<std::string> gate_type_sequential::get_state_output_pins() const
{
    return m_state_pins;
}

std::unordered_set<std::string> gate_type_sequential::get_inverted_state_output_pins() const
{
    return m_inverted_state_pins;
}

std::pair<gate_type_sequential::set_reset_behavior, gate_type_sequential::set_reset_behavior> gate_type_sequential::get_set_reset_behavior() const
{
    return m_set_reset_behavior;
}

std::string gate_type_sequential::get_init_data_category() const
{
    return m_init_data_category;
}

std::string gate_type_sequential::get_init_data_identifier() const
{
    return m_init_data_identifier;
}
