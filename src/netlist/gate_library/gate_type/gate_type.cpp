#include "netlist/gate_library/gate_type/gate_type.h"

gate_type::gate_type(const std::string& name)
{
    static u32 next_id = 1;

    m_id        = next_id++;
    m_name      = name;
    m_base_type = base_type::combinatorial;
}

u32 gate_type::get_id() const
{
    return m_id;
}

std::string gate_type::to_string() const
{
    return m_name;
}

std::ostream& operator<<(std::ostream& os, const gate_type& gt)
{
    return os << gt.to_string();
}

bool gate_type::operator==(const gate_type& other) const
{
    bool equal = m_name == other.get_name();
    equal &= m_base_type == other.get_base_type();
    equal &= m_input_pins == other.get_input_pins();
    equal &= m_output_pins == other.get_output_pins();
    equal &= m_functions == other.get_boolean_functions();
    equal &= this->do_compare(other);

    return equal;
}

bool gate_type::operator!=(const gate_type& other) const
{
    return !(*this == other);
}

bool gate_type::do_compare(const gate_type& other) const
{
    UNUSED(other);
    return true;
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

void gate_type::add_boolean_function(std::string pin_name, boolean_function bf)
{
    m_functions.emplace(pin_name, bf);
}

std::string gate_type::get_name() const
{
    return m_name;
}

gate_type::base_type gate_type::get_base_type() const
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

std::unordered_map<std::string, boolean_function> gate_type::get_boolean_functions() const
{
    return m_functions;
}
