#include "netlist/gate_library/gate_type/gate_type_latch.h"

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
        equal = m_state_pins == gt->get_state_output_pins();
        equal &= m_inverted_state_pins == gt->get_inverted_state_output_pins();
        equal &= m_special_behavior == gt->get_special_behavior();
    }

    return equal;
}

void gate_type_latch::add_state_output_pin(std::string output_pin_name)
{
    m_state_pins.insert(output_pin_name);
}

void gate_type_latch::add_inverted_state_output_pin(std::string output_pin_name)
{
    m_inverted_state_pins.insert(output_pin_name);
}

void gate_type_latch::set_special_behavior(special_behavior sb1, special_behavior sb2)
{
    m_special_behavior = {sb1, sb2};
}

std::unordered_set<std::string> gate_type_latch::get_state_output_pins() const
{
    return m_state_pins;
}

std::unordered_set<std::string> gate_type_latch::get_inverted_state_output_pins() const
{
    return m_inverted_state_pins;
}

std::pair<gate_type::special_behavior, gate_type::special_behavior> gate_type_latch::get_special_behavior() const
{
    return m_special_behavior;
}
