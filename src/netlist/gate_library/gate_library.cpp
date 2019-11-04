#include "netlist/gate_library/gate_library.h"

gate_library::gate_library(const std::string& name) : m_name(name)
{
}

std::string gate_library::get_name() const
{
    return m_name;
}

bool gate_library::operator==(const gate_library& rhs) const
{
    return (m_name == rhs.get_name());
}

bool gate_library::operator==(const std::string& rhs) const
{
    return (m_name == rhs);
}

bool gate_library::operator!=(const gate_library& rhs) const
{
    return (m_name != rhs.get_name());
}

bool gate_library::operator!=(const std::string& rhs) const
{
    return (m_name != rhs);
}

void gate_library::add_gate_type(std::shared_ptr<const gate_type> gt)
{
    m_gate_type_map.emplace(gt->get_name(), gt);

    auto out_pins = gt->get_output_pins();

    if ((gt->get_input_pins().empty() == true) && (out_pins.size() == 1))
    {
        auto bf = gt->get_boolean_functions().at(out_pins[0]);

        if (bf.is_constant_one())
        {
            m_global_vcc_gate_types.insert(gt);
        }
        else if (bf.is_constant_zero())
        {
            m_global_gnd_gate_types.insert(gt);
        }
    }
}

bool gate_library::add_global_gnd_gate_type(const std::string& gt_name)
{
    try
    {
        m_global_gnd_gate_types.insert(m_gate_type_map.at(gt_name));
        return true;
    }
    catch (const std::out_of_range& e)
    {
        return false;
    }
}

bool gate_library::add_global_vcc_gate_type(const std::string& gt_name)
{
    try
    {
        m_global_vcc_gate_types.insert(m_gate_type_map.at(gt_name));
        return true;
    }
    catch (const std::out_of_range& e)
    {
        return false;
    }
}

const std::map<std::string, std::shared_ptr<const gate_type>>& gate_library::get_gate_types()
{
    return m_gate_type_map;
}

const std::vector<std::shared_ptr<const gate_type>>& gate_library::get_global_vcc_gate_types()
{
    std::vector<std::shared_ptr<const gate_type>> gate_types;

    for (auto gt : m_global_vcc_gate_types)
    {
        gate_types.push_back(gt);
    }

    return gate_types;
}

const std::vector<std::shared_ptr<const gate_type>>& gate_library::get_global_gnd_gate_types()
{
    std::vector<std::shared_ptr<const gate_type>> gate_types;

    for (auto gt : m_global_gnd_gate_types)
    {
        gate_types.push_back(gt);
    }

    return gate_types;
}

// TODO remove stuff below
std::set<std::string>* gate_library::get_input_pin_types()
{
    return &m_input_pin_type;
}

std::set<std::string>* gate_library::get_output_pin_types()
{
    return &m_output_pin_type;
}

std::set<std::string>* gate_library::get_inout_pin_types()
{
    return &m_inout_pin_type;
}

std::map<std::string, std::vector<std::string>>* gate_library::get_gate_type_map_to_input_pin_types()
{
    return &m_gate_type_to_input_pin_types;
}

std::map<std::string, std::vector<std::string>>* gate_library::get_gate_type_map_to_output_pin_types()
{
    return &m_gate_type_to_output_pin_types;
}

std::map<std::string, std::vector<std::string>>* gate_library::get_gate_type_map_to_inout_pin_types()
{
    return &m_gate_type_to_inout_pin_types;
}

std::vector<std::string>* gate_library::get_vhdl_includes()
{
    return &m_vhdl_includes;
}
