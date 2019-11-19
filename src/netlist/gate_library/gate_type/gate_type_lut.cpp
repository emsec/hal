#include "netlist/gate_library/gate_type/gate_type_lut.h"

gate_type_lut::gate_type_lut(const std::string& name) : gate_type(name)
{
    m_base_type = base_type::lut;
    m_ascending = true;
}

bool gate_type_lut::doCompare(const gate_type& other) const
{
    bool equal              = false;
    const gate_type_lut* gt = dynamic_cast<const gate_type_lut*>(&other);

    if (gt)
    {
        equal = m_data_category == gt->get_data_category();
        equal &= m_data_identifier == gt->get_data_identifier();
        equal &= m_ascending == gt->is_ascending_order();
    }

    return equal;
}

void gate_type_lut::add_output_from_init_string_pin(const std::string& output_pin_name)
{
    m_output_from_init_string_pins.insert(output_pin_name);
}

void gate_type_lut::set_data_category(const std::string& category)
{
    m_data_category = category;
}

void gate_type_lut::set_data_identifier(const std::string& identifier)
{
    m_data_identifier = identifier;
}

void gate_type_lut::set_data_ascending_order(bool ascending)
{
    m_ascending = ascending;
}

std::unordered_set<std::string> gate_type_lut::get_output_from_init_string_pins() const
{
    return m_output_from_init_string_pins;
}

std::string gate_type_lut::get_data_category() const
{
    return m_data_category;
}

std::string gate_type_lut::get_data_identifier() const
{
    return m_data_identifier;
}

bool gate_type_lut::is_ascending_order() const
{
    return m_ascending;
}
