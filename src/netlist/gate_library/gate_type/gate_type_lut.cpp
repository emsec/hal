#include "netlist/gate_library/gate_type/gate_type_lut.h"

#include "core/log.h"

namespace hal
{
    GateTypeLut::GateTypeLut(const std::string& name) : GateType(name)
    {
        m_base_type = BaseType::lut;
        m_ascending = true;
    }

    void GateTypeLut::add_output_from_init_string_pin(const std::string& pin_name)
    {
        if (const auto& it = std::find(m_input_pins.begin(), m_input_pins.end(), pin_name); it != m_input_pins.end())
        {
            log_warning("gate_type", "pin '{}' of gate type '{}' is not an output pin, ignoring output from INIT string pin assignment", pin_name, m_name);
            return;
        }

        m_output_from_init_string_pins.insert(pin_name);
    }

    void GateTypeLut::set_config_data_category(const std::string& category)
    {
        m_config_data_category = category;
    }

    void GateTypeLut::set_config_data_identifier(const std::string& identifier)
    {
        m_config_data_identifier = identifier;
    }

    void GateTypeLut::set_config_data_ascending_order(bool ascending)
    {
        m_ascending = ascending;
    }

    std::unordered_set<std::string> GateTypeLut::get_output_from_init_string_pins() const
    {
        return m_output_from_init_string_pins;
    }

    std::string GateTypeLut::get_config_data_category() const
    {
        return m_config_data_category;
    }

    std::string GateTypeLut::get_config_data_identifier() const
    {
        return m_config_data_identifier;
    }

    bool GateTypeLut::is_config_data_ascending_order() const
    {
        return m_ascending;
    }
}    // namespace hal
