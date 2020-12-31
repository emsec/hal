#include "hal_core/netlist/gate_library/gate_type.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    const std::unordered_map<GateType::BaseType, std::string> GateType::m_base_type_to_string         = {{BaseType::combinational, "combinational"},
                                                                                                 {BaseType::ff, "ff"},
                                                                                                 {BaseType::latch, "latch"},
                                                                                                 {BaseType::lut, "lut"},
                                                                                                 {BaseType::ram, "ram"},
                                                                                                 {BaseType::io, "io"},
                                                                                                 {BaseType::buffer, "buffer"}};
    const std::unordered_map<GateType::PinDirection, std::string> GateType::m_pin_direction_to_string = {{PinDirection::input, "input"},
                                                                                                         {PinDirection::output, "output"},
                                                                                                         {PinDirection::inout, "inout"},
                                                                                                         {PinDirection::internal, "internal"}};
    const std::unordered_map<GateType::PinType, std::string> GateType::m_pin_type_to_string           = {{PinType::none, "none"},
                                                                                               {PinType::power, "power"},
                                                                                               {PinType::ground, "ground"},
                                                                                               {PinType::lut, "lut"},
                                                                                               {PinType::state, "state"},
                                                                                               {PinType::neg_state, "neg_state"},
                                                                                               {PinType::clock, "clock"},
                                                                                               {PinType::enable, "enable"},
                                                                                               {PinType::set, "set"},
                                                                                               {PinType::reset, "reset"},
                                                                                               {PinType::data, "data"},
                                                                                               {PinType::address, "address"},
                                                                                               {PinType::io_pad, "io_pad"}};

    const std::unordered_map<GateType::PinDirection, std::unordered_set<GateType::PinType>> GateType::m_direction_to_types = {
        {PinDirection::input, {PinType::none, PinType::power, PinType::ground, PinType::clock, PinType::enable, PinType::set, PinType::reset, PinType::data, PinType::address, PinType::io_pad}},
        {PinDirection::output, {PinType::none, PinType::lut, PinType::state, PinType::neg_state, PinType::data, PinType::address, PinType::io_pad}},
        {PinDirection::inout, {PinType::none, PinType::io_pad}},
        {PinDirection::internal, {PinType::none}}};

    GateType::GateType(GateLibrary* gate_library, u32 id, const std::string& name, BaseType base_type)
    {
        m_gate_library = gate_library;
        m_id           = id;
        m_name         = name;
        m_base_type    = base_type;
    }

    u32 GateType::get_id() const
    {
        return m_id;
    }

    std::string GateType::get_name() const
    {
        return m_name;
    }

    GateType::BaseType GateType::get_base_type() const
    {
        return m_base_type;
    }

    GateLibrary* GateType::get_gate_library() const
    {
        return m_gate_library;
    }

    std::string GateType::to_string() const
    {
        return m_name;
    }

    std::ostream& operator<<(std::ostream& os, const GateType& gt)
    {
        return os << gt.to_string();
    }

    std::ostream& operator<<(std::ostream& os, GateType::BaseType base_type)
    {
        return os << GateType::m_base_type_to_string.at(base_type);
    }

    std::ostream& operator<<(std::ostream& os, GateType::PinDirection direction)
    {
        return os << GateType::m_pin_direction_to_string.at(direction);
    }

    std::ostream& operator<<(std::ostream& os, GateType::PinType pin_type)
    {
        return os << GateType::m_pin_type_to_string.at(pin_type);
    }

    bool GateType::operator==(const GateType& other) const
    {
        return ((m_id == other.get_id()) && (m_gate_library == other.get_gate_library()));
    }

    bool GateType::operator!=(const GateType& other) const
    {
        return !(*this == other);
    }

    void GateType::add_input_pin(std::string pin)
    {
        m_pin_to_direction[pin] = PinDirection::input;
        m_direction_to_pins[PinDirection::input].push_back(pin);
        m_pin_to_type[pin] = PinType::none;
        m_type_to_pins[PinType::none].push_back(pin);
    }

    void GateType::add_input_pins(const std::vector<std::string>& pins)
    {
        for (const auto& pin : pins)
        {
            m_pin_to_direction[pin] = PinDirection::input;
            m_direction_to_pins[PinDirection::input].push_back(pin);
            m_pin_to_type[pin] = PinType::none;
            m_type_to_pins[PinType::none].push_back(pin);
        }
    }

    std::vector<std::string> GateType::get_input_pins() const
    {
        if (const auto it = m_direction_to_pins.find(PinDirection::input); it != m_direction_to_pins.end())
        {
            return it->second;
        }
        else
        {
            return {};
        }
    }

    void GateType::add_output_pin(std::string pin)
    {
        m_pin_to_direction[pin] = PinDirection::output;
        m_direction_to_pins[PinDirection::output].push_back(pin);
        m_pin_to_type[pin] = PinType::none;
        m_type_to_pins[PinType::none].push_back(pin);
    }

    void GateType::add_output_pins(const std::vector<std::string>& pins)
    {
        for (const auto& pin : pins)
        {
            m_pin_to_direction[pin] = PinDirection::output;
            m_direction_to_pins[PinDirection::output].push_back(pin);
            m_pin_to_type[pin] = PinType::none;
            m_type_to_pins[PinType::none].push_back(pin);
        }
    }

    std::vector<std::string> GateType::get_output_pins() const
    {
        if (const auto it = m_direction_to_pins.find(PinDirection::output); it != m_direction_to_pins.end())
        {
            return it->second;
        }
        else
        {
            return {};
        }
    }

    void GateType::assign_input_pin_group(const std::string& group_name, const std::map<u32, std::string>& index_to_pin)
    {
        m_input_pin_groups.emplace(group_name, index_to_pin);
    }

    void GateType::assign_input_pin_groups(const std::unordered_map<std::string, std::map<u32, std::string>>& pin_groups)
    {
        m_input_pin_groups.insert(pin_groups.begin(), pin_groups.end());
    }

    std::unordered_map<std::string, std::map<u32, std::string>> GateType::get_input_pin_groups() const
    {
        return m_input_pin_groups;
    }

    void GateType::assign_output_pin_group(const std::string& group_name, const std::map<u32, std::string>& index_to_pin)
    {
        m_output_pin_groups.emplace(group_name, index_to_pin);
    }

    void GateType::assign_output_pin_groups(const std::unordered_map<std::string, std::map<u32, std::string>>& pin_groups)
    {
        m_output_pin_groups.insert(pin_groups.begin(), pin_groups.end());
    }

    std::unordered_map<std::string, std::map<u32, std::string>> GateType::get_output_pin_groups() const
    {
        return m_output_pin_groups;
    }

    std::unordered_map<std::string, GateType::PinDirection> GateType::get_pin_directions() const
    {
        return m_pin_to_direction;
    }

    bool GateType::assign_pin_type(const std::string& pin, PinType pin_type)
    {
        for (const auto& [direction, pins] : m_direction_to_pins)
        {
            if (std::find(pins.begin(), pins.end(), pin) != pins.end())
            {
                if (const auto it = m_direction_to_types.find(direction); it != m_direction_to_types.end())
                {
                    if (it->second.find(pin_type) != it->second.end())
                    {
                        PinType current_type = m_pin_to_type.at(pin);
                        m_type_to_pins.at(current_type).erase(std::find(m_type_to_pins.at(current_type).begin(), m_type_to_pins.at(current_type).end(), pin));
                        m_pin_to_type[pin] = pin_type;
                        m_type_to_pins[pin_type].push_back(pin);
                        return true;
                    }
                }
            }
        }

        log_error("gate_library", "could not assign type '{}' to pin '{}' of gate type '{}'.", m_pin_type_to_string.at(pin_type), pin, m_name);
        return false;
    }

    GateType::PinType GateType::get_pin_type(const std::string& pin) const
    {
        if (const auto it = m_pin_to_type.find(pin); it != m_pin_to_type.end())
        {
            return it->second;
        }

        return PinType::none;
    }

    std::unordered_map<std::string, GateType::PinType> GateType::get_pin_types() const
    {
        return m_pin_to_type;
    }

    std::vector<std::string> GateType::get_pins_of_type(PinType pin_type) const
    {
        if (const auto it = m_type_to_pins.find(pin_type); it != m_type_to_pins.end())
        {
            return it->second;
        }

        return {};
    }

    void GateType::add_boolean_function(std::string pin_name, BooleanFunction bf)
    {
        m_functions.emplace(pin_name, bf);
    }

    void GateType::add_boolean_functions(const std::unordered_map<std::string, BooleanFunction>& functions)
    {
        m_functions.insert(functions.begin(), functions.end());
    }

    std::unordered_map<std::string, BooleanFunction> GateType::get_boolean_functions() const
    {
        return m_functions;
    }

    void GateType::set_clear_preset_behavior(ClearPresetBehavior cp1, ClearPresetBehavior cp2)
    {
        m_clear_preset_behavior = {cp1, cp2};
    }

    std::pair<GateType::ClearPresetBehavior, GateType::ClearPresetBehavior> GateType::get_clear_preset_behavior() const
    {
        return m_clear_preset_behavior;
    }

    void GateType::set_config_data_category(const std::string& category)
    {
        m_config_data_category = category;
    }

    std::string GateType::get_config_data_category() const
    {
        return m_config_data_category;
    }

    void GateType::set_config_data_identifier(const std::string& identifier)
    {
        m_config_data_identifier = identifier;
    }

    std::string GateType::get_config_data_identifier() const
    {
        return m_config_data_identifier;
    }

    void GateType::set_lut_init_ascending(bool ascending)
    {
        m_ascending = ascending;
    }

    bool GateType::is_lut_init_ascending() const
    {
        return m_ascending;
    }
}    // namespace hal
