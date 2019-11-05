#include "netlist/gate_library/gate_library.h"

gate_library::gate_library(const std::string& name) : m_name(name)
{
}

std::string gate_library::get_name() const
{
    return m_name;
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
            m_global_vcc_gate_types.emplace(gt->get_name(), gt);
        }
        else if (bf.is_constant_zero())
        {
            m_global_gnd_gate_types.emplace(gt->get_name(), gt);
        }
    }
}

bool gate_library::add_global_gnd_gate_type(const std::string& gt_name)
{
    if (m_gate_type_map.find(gt_name) != m_gate_type_map.end())
    {
        m_global_gnd_gate_types.emplace(gt_name, m_gate_type_map.at(gt_name));
        return true;
    }

    return false;
}

bool gate_library::add_global_vcc_gate_type(const std::string& gt_name)
{
    if (m_gate_type_map.find(gt_name) != m_gate_type_map.end())
    {
        m_global_vcc_gate_types.emplace(gt_name, m_gate_type_map.at(gt_name));
        return true;
    }

    return false;
}

const std::map<std::string, std::shared_ptr<const gate_type>>& gate_library::get_gate_types()
{
    return m_gate_type_map;
}

const std::map<std::string, std::shared_ptr<const gate_type>>& gate_library::get_global_vcc_gate_types()
{
    return m_global_vcc_gate_types;
}

const std::map<std::string, std::shared_ptr<const gate_type>>& gate_library::get_global_gnd_gate_types()
{
    return m_global_gnd_gate_types;
}

std::vector<std::string>* gate_library::get_vhdl_includes()
{
    return &m_vhdl_includes;
}
