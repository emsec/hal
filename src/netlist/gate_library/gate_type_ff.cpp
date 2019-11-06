#include "netlist/gate_library/gate_type_ff.h"

bool gate_type_ff::doCompare(const gate_type& other) const
{
    bool equal             = false;
    const gate_type_ff* gt = dynamic_cast<const gate_type_ff*>(&other);

    if (gt)
    {
        equal &= m_clock_f == gt->get_clock_function();
        equal &= m_set_f == gt->get_set_function();
        equal &= m_reset_f == gt->get_reset_function();
        equal &= m_output_pin_inverted == gt->get_output_pin_inverted();
        equal &= m_special_behavior == gt->get_special_behavior();
    }

    return equal;
}

void gate_type_ff::set_clock_function(boolean_function clock_f)
{
    m_clock_f = clock_f;
}

void gate_type_ff::set_set_function(boolean_function set_f)
{
    m_set_f = set_f;
}

void gate_type_ff::set_reset_function(boolean_function reset_f)
{
    m_reset_f = reset_f;
}

void gate_type_ff::set_output_pin_inverted(std::string output_pin, bool inverted)
{
    m_output_pin_inverted[output_pin] = inverted;
}

void gate_type_ff::set_special_behavior(std::pair<special_behavior_t, special_behavior_t> special_behavior)
{
    m_special_behavior = special_behavior;
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

std::map<std::string, bool> gate_type_ff::get_output_pin_inverted() const
{
    return m_output_pin_inverted;
}

std::pair<gate_type_ff::special_behavior_t, gate_type_ff::special_behavior_t> gate_type_ff::get_special_behavior() const
{
    return m_special_behavior;
}