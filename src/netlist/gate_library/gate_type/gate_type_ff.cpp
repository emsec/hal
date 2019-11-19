#include "netlist/gate_library/gate_type/gate_type_ff.h"

gate_type_ff::gate_type_ff(const std::string& name) : gate_type(name)
{
    m_base_type = base_type::ff;
    m_ascending = true;
}

bool gate_type_ff::doCompare(const gate_type& other) const
{
    bool equal             = false;
    const gate_type_ff* gt = dynamic_cast<const gate_type_ff*>(&other);

    if (gt)
    {
        equal = m_state_pins == gt->get_state_output_pins();
        equal &= m_inverted_state_pins == gt->get_inverted_state_output_pins();
        equal &= m_special_behavior == gt->get_special_behavior();
        equal &= m_data_category == gt->get_data_category();
        equal &= m_data_identifier == gt->get_data_identifier();
        equal &= m_ascending == gt->is_ascending_order();
    }

    return equal;
}

void gate_type_ff::add_state_output_pin(std::string output_pin_name)
{
    m_state_pins.insert(output_pin_name);
}

void gate_type_ff::add_inverted_state_output_pin(std::string output_pin_name)
{
    m_inverted_state_pins.insert(output_pin_name);
}

void gate_type_ff::set_special_behavior(special_behavior sb1, special_behavior sb2)
{
    m_special_behavior = {sb1, sb2};
}

void gate_type_ff::set_data_category(const std::string& category)
{
    m_data_category = category;
}

void gate_type_ff::set_data_identifier(const std::string& identifier)
{
    m_data_identifier = identifier;
}

void gate_type_ff::set_data_ascending_order(bool ascending)
{
    m_ascending = ascending;
}

std::unordered_set<std::string> gate_type_ff::get_state_output_pins() const
{
    return m_state_pins;
}

std::unordered_set<std::string> gate_type_ff::get_inverted_state_output_pins() const
{
    return m_inverted_state_pins;
}

std::pair<gate_type::special_behavior, gate_type::special_behavior> gate_type_ff::get_special_behavior() const
{
    return m_special_behavior;
}

std::string gate_type_ff::get_data_category() const
{
    return m_data_category;
}

std::string gate_type_ff::get_data_identifier() const
{
    return m_data_identifier;
}

bool gate_type_ff::is_ascending_order() const
{
    return m_ascending;
}
