#include "netlist/gate_library/gate_library.h"

gate_library::gate_library(const std::string& name) : m_name(name)
{
}

std::string gate_library::get_name() const
{
    return m_name;
}

void gate_library::add_gate_type(gate_type gt)
{
    m_gate_type_map[gt.get_name()] = gt;

    auto out_pins = gt.get_output_pins();

    if ((gt.get_input_pins.empty() == true) && (out_pins.size() == 1))
    {
        auto bf = gt.get_boolean_function(out_pins[0]);

        if (bf.is_constant_one())
        {
            m_global_vcc_gate_types.insert(&gt);
        }
        else if (bf.is_constant_zero())
        {
            m_global_gnd_gate_types.insert(&gt);
        }
    }
}

const gate_type& gate_library::get_gate_type(std::string name)
{
    return m_gate_type_map[name];
}

std::vector<const gate_type> gate_library::get_gate_types()
{
    std::vector<const gate_type> gate_types;

    for (auto [name, gt] : m_gate_type_map)
    {
        gate_types.push_back(gt);
    }

    return gate_types;
}

std::vector<const gate_type> gate_library::get_global_vcc_gate_types()
{
    std::vector<const gate_type> gate_types;

    for (auto gt : m_global_vcc_gate_types)
    {
        gate_types.push_back(*gt);
    }

    return gate_types;
}

std::vector<const gate_type> gate_library::get_global_gnd_gate_types()
{
    std::vector<const gate_type> gate_types;

    for (auto gt : m_global_gnd_gate_types)
    {
        gate_types.push_back(*gt);
    }

    return gate_types;
}

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
