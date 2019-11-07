#include "netlist/gate_library/gate_type_ff.h"

gate_type_ff::gate_type_ff(const std::string& name) : gate_type(name)
{
    m_base_type = base_type::ff;
}

bool gate_type_ff::doCompare(const gate_type& other) const
{
    bool equal             = false;
    const gate_type_ff* gt = dynamic_cast<const gate_type_ff*>(&other);

    if (gt)
    {
        equal = m_next_state_f == gt->get_next_state_function();
        equal &= m_clock_f == gt->get_clock_function();
        equal &= m_set_f == gt->get_set_function();
        equal &= m_reset_f == gt->get_reset_function();
        equal &= m_inverted_output_pins == gt->get_inverted_output_pins();
        equal &= m_special_behavior == gt->get_special_behavior();
    }

    return equal;
}

void gate_type_ff::set_next_state_function(const boolean_function& next_state_f)
{
    m_next_state_f = next_state_f;
}

void gate_type_ff::set_clock_function(const boolean_function& clock_f)
{
    m_clock_f = clock_f;
}

void gate_type_ff::set_set_function(const boolean_function& set_f)
{
    m_set_f = set_f;
}

void gate_type_ff::set_reset_function(const boolean_function& reset_f)
{
    m_reset_f = reset_f;
}

void gate_type_ff::set_output_pin_inverted(const std::string& output_pin, bool inverted)
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

void gate_type_ff::set_special_behavior1(special_behavior sb)
{
    m_special_behavior.first = sb;
}

void gate_type_ff::set_special_behavior2(special_behavior sb)
{
    m_special_behavior.second = sb;
}

boolean_function gate_type_ff::get_next_state_function() const
{
    return m_next_state_f;
}

boolean_function gate_type_ff::get_clock_function() const
{
    return m_clock_f;
}

boolean_function gate_type_ff::get_set_function() const
{
    return m_set_f;
}

boolean_function gate_type_ff::get_reset_function() const
{
    return m_reset_f;
}

std::set<std::string> gate_type_ff::get_inverted_output_pins() const
{
    return m_inverted_output_pins;
}

std::pair<gate_type_ff::special_behavior, gate_type_ff::special_behavior> gate_type_ff::get_special_behavior() const
{
    return m_special_behavior;
}
