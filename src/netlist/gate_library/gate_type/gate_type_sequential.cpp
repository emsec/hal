#include "netlist/gate_library/gate_type/gate_type_sequential.h"

gate_type_sequential::gate_type_sequential(const std::string& name, base_type bt) : gate_type(name)
{
    m_base_type = bt;
    assert(m_base_type == base_type::ff || m_base_type == base_type::latch);
}

bool gate_type_sequential::do_compare(const gate_type& other) const
{
    bool equal    = false;
    const auto gt = dynamic_cast<const gate_type_sequential*>(&other);

    if (gt)
    {
        equal = m_state_pins == gt->get_state_output_pins();
        equal &= m_inverted_state_pins == gt->get_inverted_state_output_pins();
        equal &= m_special_behavior == gt->get_special_behavior();
        equal &= m_init_data_category == gt->get_init_data_category();
        equal &= m_init_data_identifier == gt->get_init_data_identifier();
    }

    return equal;
}

void gate_type_sequential::add_state_output_pin(std::string output_pin_name)
{
    m_state_pins.insert(output_pin_name);
}

std::unordered_set<std::string> gate_type_sequential::get_state_output_pins() const
{
    return m_state_pins;
}

void gate_type_sequential::add_inverted_state_output_pin(std::string output_pin_name)
{
    m_inverted_state_pins.insert(output_pin_name);
}

std::unordered_set<std::string> gate_type_sequential::get_inverted_state_output_pins() const
{
    return m_inverted_state_pins;
}

void gate_type_sequential::set_special_behavior(special_behavior sb1, special_behavior sb2)
{
    m_special_behavior = {sb1, sb2};
}

std::pair<gate_type::special_behavior, gate_type::special_behavior> gate_type_sequential::get_special_behavior() const
{
    return m_special_behavior;
}

void gate_type_sequential::set_init_data_category(const std::string& category)
{
    m_init_data_category = category;
}

std::string gate_type_sequential::get_init_data_category() const
{
    return m_init_data_category;
}

void gate_type_sequential::set_init_data_identifier(const std::string& identifier)
{
    m_init_data_identifier = identifier;
}

std::string gate_type_sequential::get_init_data_identifier() const
{
    return m_init_data_identifier;
}
