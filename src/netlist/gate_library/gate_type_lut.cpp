#include "netlist/gate_library/gate_type_lut.h"

std::string gate_type_lut::to_string(const gate_type_lut& gt)
{
    return gt.get_name();
}

std::ostream& operator<<(std::ostream& os, const gate_type_lut& gt)
{
    return os << gate_type_lut::to_string(gt);
}

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

void gate_type_lut::set_direction(direction_t direction)
{
    m_direction = direction;
}

std::string gate_type_lut::get_data_category() const
{
    return m_data_category;
}

std::string gate_type_lut::get_data_key() const
{
    return m_data_key;
}

gate_type_lut::direction_t gate_type_lut::get_direction() const
{
    return m_direction;
}