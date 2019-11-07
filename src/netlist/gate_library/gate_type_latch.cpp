#include "netlist/gate_library/gate_type_latch.h"

gate_type_latch::gate_type_latch(const std::string& name) : gate_type(name)
{
    m_base_type = base_type::latch;
}

bool gate_type_latch::doCompare(const gate_type& other) const
{
    bool equal                = false;
    const gate_type_latch* gt = dynamic_cast<const gate_type_latch*>(&other);

    if (gt)
    {
        equal = m_data_in_f == gt->get_data_in_function();
        equal &= m_enable_f == gt->get_enable_function();
        equal &= m_set_f == gt->get_set_function();
        equal &= m_reset_f == gt->get_reset_function();
        equal &= m_inverted_output_pins == gt->get_inverted_output_pins();
        equal &= m_special_behavior == gt->get_special_behavior();
    }

    return equal;
}

void gate_type_latch::set_data_in_function(const boolean_function& data_in_f)
{
    m_data_in_f = data_in_f;
}
void gate_type_latch::set_enable_function(const boolean_function& enable_f)
{
    m_enable_f = enable_f;
}

void gate_type_latch::set_set_function(const boolean_function& set_f)
{
    m_set_f = set_f;
}

void gate_type_latch::set_reset_function(const boolean_function& reset_f)
{
    m_reset_f = reset_f;
}

void gate_type_latch::set_output_pin_inverted(const std::string& output_pin, bool inverted)
{
    if (inverted)
    {
        m_inverted_output_pins.insert(output_pin);
    }
    else
    {
        m_inverted_output_pins.erase(output_pin);
    }
}

void gate_type_latch::set_special_behavior1(special_behavior sb)
{
    m_special_behavior.first = sb;
}

void gate_type_latch::set_special_behavior2(special_behavior sb)
{
    m_special_behavior.second = sb;
}

boolean_function gate_type_latch::get_data_in_function() const
{
    return m_data_in_f;
}

boolean_function gate_type_latch::get_enable_function() const
{
    return m_enable_f;
}

boolean_function gate_type_latch::get_set_function() const
{
    return m_set_f;
}

boolean_function gate_type_latch::get_reset_function() const
{
    return m_reset_f;
}

std::set<std::string> gate_type_latch::get_inverted_output_pins() const
{
    return m_inverted_output_pins;
}

std::pair<gate_type_latch::special_behavior, gate_type_latch::special_behavior> gate_type_latch::get_special_behavior() const
{
    return m_special_behavior;
}
