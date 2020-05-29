#include "netlist/gate_library/gate_type/gate_type_lut.h"

#include "core/log.h"

gate_type_lut::gate_type_lut(const std::string& name) : gate_type(name)
{
    m_base_type = base_type::lut;
    m_ascending = true;
}

void gate_type_lut::add_output_from_init_string_pin(const std::string& pin_name)
{
    if (const auto& it = std::find(m_input_pins.begin(), m_input_pins.end(), pin_name); it != m_input_pins.end())
    {
        log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring output from INIT string pin assignment", pin_name, m_name);
        return;
    }

    m_output_from_init_string_pins.insert(pin_name);
}

void gate_type_lut::set_config_data_category(const std::string& category)
{
    m_config_data_category = category;
}

void gate_type_lut::set_config_data_identifier(const std::string& identifier)
{
    m_config_data_identifier = identifier;
}

void gate_type_lut::set_config_data_ascending_order(bool ascending)
{
    m_ascending = ascending;
}

std::unordered_set<std::string> gate_type_lut::get_output_from_init_string_pins() const
{
    return m_output_from_init_string_pins;
}

std::string gate_type_lut::get_config_data_category() const
{
    return m_config_data_category;
}

std::string gate_type_lut::get_config_data_identifier() const
{
    return m_config_data_identifier;
}

bool gate_type_lut::is_config_data_ascending_order() const
{
    return m_ascending;
}
