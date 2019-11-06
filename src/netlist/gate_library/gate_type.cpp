#include "netlist/gate_library/gate_type.h"

gate_type::gate_type(const std::string& name)
{
    m_name      = name;
    m_base_type = combinatorial;
}

std::string gate_type::to_string(const gate_type& gt)
{
    return gt.get_name();
}

std::ostream& operator<<(std::ostream& os, const gate_type& gt)
{
    return os << gate_type::to_string(gt);
}

bool gate_type::operator==(const gate_type& other) const
{
    bool equal = m_name == other.get_name();
    equal &= m_base_type == other.get_base_type();
    equal &= m_input_pins == other.get_input_pins();
    equal &= m_output_pins == other.get_output_pins();
    equal &= m_functions == other.get_boolean_functions();

    return equal;
}

bool gate_type::operator!=(const gate_type& other) const
{
    return !(*this == other);
}

void gate_type::set_base_type(base_type_t base_type)
{
    m_base_type = base_type;
}

void gate_type::add_input_pin(std::string input_pin)
{
    m_input_pins.push_back(input_pin);
}

void gate_type::add_input_pins(const std::vector<std::string>& input_pins)
{
    m_input_pins.insert(m_input_pins.end(), input_pins.begin(), input_pins.end());
}

void gate_type::add_output_pin(std::string output_pin)
{
    m_output_pins.push_back(output_pin);
}

void gate_type::add_output_pins(const std::vector<std::string>& output_pins)
{
    m_output_pins.insert(m_output_pins.end(), output_pins.begin(), output_pins.end());
}

void gate_type::add_boolean_function(std::string name, boolean_function bf)
{
    m_functions.emplace(name, bf);
}

void gate_type::add_boolean_function_map(const std::map<std::string, const boolean_function>& boolean_function_map)
{
    m_functions.insert(boolean_function_map.begin(), boolean_function_map.end());
}

std::string gate_type::get_name() const
{
    return m_name;
}

gate_type::base_type_t gate_type::get_base_type() const
{
    return m_base_type;
}

std::vector<std::string> gate_type::get_input_pins() const
{
    return m_input_pins;
}

std::vector<std::string> gate_type::get_output_pins() const
{
    return m_output_pins;
}

std::map<std::string, boolean_function> gate_type::get_boolean_functions() const
{
    return m_functions;
}
