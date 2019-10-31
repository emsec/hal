#include "gate_type.h"

gate_type::gate_type(const std::string& name, base_type_t base_type)
{
    m_name      = name;
    m_base_type = base_type;
}

void gate_type::add_input_pins(const std::vector<std::string>& input_pins)
{
    m_input_pins.insert(m_input_pins.end(), input_pins.begin(), input_pins.end());
}

void gate_type::add_output_pins(const std::vector<std::string>& output_pins)
{
    m_output_pins.insert(m_output_pins.end(), output_pins.begin(), output_pins.end());
}

void gate_type::add_boolean_function_map(const std::map<std::string, boolean_function>& boolean_function_map)
{
    m_boolean_function_map.insert(boolean_function_map.begin(), boolean_function_map.end());
}

const std::string& gate_type::get_name()
{
    return m_name;
}

std::vector<std::string> gate_type::get_input_pins()
{
    return m_input_pins;
}

std::vector<std::string> gate_type::get_output_pins()
{
    return m_output_pins;
}

const boolean_function& gate_type::get_boolean_function(const std::string& name)
{
    return m_boolean_function_map[name];
}