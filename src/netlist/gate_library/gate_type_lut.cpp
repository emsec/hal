#include "netlist/gate_library/gate_type_lut.h"

bool gate_type_lut::doCompare(const gate_type& other) const
{
    bool equal              = false;
    const gate_type_lut* gt = dynamic_cast<const gate_type_lut*>(&other);

    if (gt)
    {
        equal &= m_data_category == gt->get_data_category();
        equal &= m_data_key == gt->get_data_key();
        equal &= m_data_key == gt->get_data_key();
    }

    return equal;
}

void gate_type_lut::set_data_category(std::string data_category)
{
    m_data_category = data_category;
}

void gate_type_lut::set_data_key(std::string data_key)
{
    m_data_key = data_key;
}

void gate_type_lut::set_ascending(bool ascending)
{
    m_ascending = ascending;
}

std::string gate_type_lut::get_data_category() const
{
    return m_data_category;
}

std::string gate_type_lut::get_data_key() const
{
    return m_data_key;
}

bool gate_type_lut::is_ascending() const
{
    return m_ascending;
}