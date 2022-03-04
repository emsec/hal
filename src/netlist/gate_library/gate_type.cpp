#include "hal_core/netlist/gate_library/gate_type.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    template<>
    std::map<GateTypeProperty, std::string> EnumStrings<GateTypeProperty>::data = {{GateTypeProperty::combinational, "combinational"},
                                                                                   {GateTypeProperty::sequential, "sequential"},
                                                                                   {GateTypeProperty::power, "power"},
                                                                                   {GateTypeProperty::ground, "ground"},
                                                                                   {GateTypeProperty::lut, "lut"},
                                                                                   {GateTypeProperty::ff, "ff"},
                                                                                   {GateTypeProperty::latch, "latch"},
                                                                                   {GateTypeProperty::ram, "ram"},
                                                                                   {GateTypeProperty::io, "io"},
                                                                                   {GateTypeProperty::dsp, "dsp"},
                                                                                   {GateTypeProperty::mux, "mux"},
                                                                                   {GateTypeProperty::buffer, "buffer"},
                                                                                   {GateTypeProperty::carry, "carry"},
                                                                                   {GateTypeProperty::pll, "pll"}};

    GateType::GateType(GateLibrary* gate_library, u32 id, const std::string& name, std::set<GateTypeProperty> properties, std::unique_ptr<GateTypeComponent> component)
        : m_gate_library(gate_library), m_id(id), m_name(name), m_properties(properties), m_component(std::move(component))
    {
    }

    std::vector<GateTypeComponent*> GateType::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        if (m_component != nullptr)
        {
            std::vector<GateTypeComponent*> res = m_component->get_components(filter);
            if (filter)
            {
                if (filter(m_component.get()))
                {
                    res.push_back(m_component.get());
                }
            }
            else
            {
                res.push_back(m_component.get());
            }

            return res;
        }

        return {};
    }

    GateTypeComponent* GateType::get_component(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        std::vector<GateTypeComponent*> components = this->get_components(filter);

        if (components.size() == 1)
        {
            return *components.begin();
        }

        return nullptr;
    }

    bool GateType::has_component_of_type(const GateTypeComponent::ComponentType type) const
    {
        return !this->get_components([type](const GateTypeComponent* component) { return component->get_type() == type; }).empty();
    }

    u32 GateType::get_id() const
    {
        return m_id;
    }

    const std::string& GateType::get_name() const
    {
        return m_name;
    }

    void GateType::assign_property(const GateTypeProperty property)
    {
        m_properties.insert(property);
    }

    std::set<GateTypeProperty> GateType::get_properties() const
    {
        return m_properties;
    }

    bool GateType::has_property(GateTypeProperty property) const
    {
        return m_properties.find(property) != m_properties.end();
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

    bool GateType::operator==(const GateType& other) const
    {
        return ((m_id == other.get_id()) && (m_gate_library == other.get_gate_library()));
    }

    bool GateType::operator!=(const GateType& other) const
    {
        return !(*this == other);
    }

    void GateType::add_input_pin(const std::string& pin)
    {
        add_pin(pin, PinDirection::input);
    }

    void GateType::add_input_pins(const std::vector<std::string>& pins)
    {
        add_pins(pins, PinDirection::input);
    }

    std::vector<std::string> GateType::get_input_pins() const
    {
        std::vector<std::string> res;

        for (const auto& pin : m_pins)
        {
            PinDirection direction = m_pin_to_direction.at(pin);
            if (direction == PinDirection::input || direction == PinDirection::inout)
            {
                res.push_back(pin);
            }
        }

        return res;
    }

    void GateType::add_output_pin(const std::string& pin)
    {
        add_pin(pin, PinDirection::output);
    }

    void GateType::add_output_pins(const std::vector<std::string>& pins)
    {
        add_pins(pins, PinDirection::output);
    }

    std::vector<std::string> GateType::get_output_pins() const
    {
        std::vector<std::string> res;

        for (const auto& pin : m_pins)
        {
            PinDirection direction = m_pin_to_direction.at(pin);
            if (direction == PinDirection::output || direction == PinDirection::inout)
            {
                res.push_back(pin);
            }
        }

        return res;
    }

    u32 GateType::get_unique_pin_id()
    {
        if (!m_free_pin_ids.empty())
        {
            return *(m_free_pin_ids.begin());
        }
        while (m_used_pin_ids.find(m_next_pin_id) != m_used_pin_ids.end())
        {
            m_next_pin_id++;
        }
        return m_next_pin_id;
    }

    u32 GateType::get_unique_pin_group_id()
    {
        if (!m_free_pin_group_ids.empty())
        {
            return *(m_free_pin_group_ids.begin());
        }
        while (m_used_pin_group_ids.find(m_next_pin_group_id) != m_used_pin_group_ids.end())
        {
            m_next_pin_group_id++;
        }
        return m_next_pin_group_id;
    }

    Result<GatePin*> GateType::create_pin(const u32 id, const std::string& name, PinDirection direction, PinType type)
    {
        if (name.empty())
        {
            return ERR("could not create pin for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": empty string passed as name");
        }
        if (id == 0)
        {
            return ERR("could not create pin '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": ID 0 is invalid");
        }
        if (m_used_pin_ids.find(id) != m_used_pin_ids.end())
        {
            return ERR("could not create pin '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": ID " + std::to_string(id) + " is already taken");
        }
        if (direction == PinDirection::none || direction == PinDirection::internal)
        {
            return ERR("could not create pin '" + name + "' for gate type '" + m_name + "' with " + std::to_string(m_id) + ": direction '" + enum_to_string(direction) + "' is invalid");
        }

        // create pin
        std::unique_ptr<GatePin> pin_owner(new GatePin(id, name, direction, type));
        GatePin* pin = pin_owner.get();
        m_pins_new.push_back(std::move(pin_owner));
        m_pins_map[id] = pin;

        // mark pin ID as used
        if (auto free_id_it = m_free_pin_ids.find(id); free_id_it != m_free_pin_ids.end())
        {
            m_free_pin_ids.erase(free_id_it);
        }
        m_used_pin_ids.insert(id);

        if (auto res = create_pin_group(name, {pin}, direction, type); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not create pin '" + name + "' for gate type '" + m_name + "' with " + std::to_string(m_id) + ": failed to create pin group");
        }

        return OK(pin);
    }

    Result<GatePin*> GateType::create_pin(const std::string& name, PinDirection direction, PinType type)
    {
        return create_pin(get_unique_pin_id(), name, direction, type);
    }

    std::vector<GatePin*> GateType::get_pins_new(const std::function<bool(GatePin*)>& filter) const
    {
        std::vector<GatePin*> res;
        if (!filter)
        {
            res.reserve(m_pins.size());
            for (const auto& group : m_pin_groups_new)
            {
                std::vector<GatePin*> pins = group->get_pins();
                res.insert(res.end(), pins.begin(), pins.end());
            }
        }
        else
        {
            for (PinGroup<GatePin>* group : m_pin_groups_ordered)
            {
                for (GatePin* pin : group->get_pins())
                {
                    if (filter(pin))
                    {
                        res.push_back(pin);
                    }
                }
            }
        }
        return res;
    }

    Result<GatePin*> GateType::get_pin_by_id(const u32 id) const
    {
        if (id == 0)
        {
            return ERR("could not get pin by ID for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": ID 0 is invalid");
        }

        if (const auto it = m_pins_map.find(id); it != m_pins_map.end())
        {
            return OK(it->second);
        }

        return ERR("could not get pin by ID for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": no pin with ID " + std::to_string(id) + " exists");
    }

    Result<PinGroup<GatePin>*>
        GateType::create_pin_group(const u32 id, const std::string& name, const std::vector<GatePin*> pins, PinDirection direction, PinType type, bool ascending, u32 start_index)
    {
        if (name.empty())
        {
            return ERR("could not create pin group for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": empty string passed as name");
        }
        if (id == 0)
        {
            return ERR("could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": ID 0 is invalid");
        }
        if (m_used_pin_group_ids.find(id) != m_used_pin_group_ids.end())
        {
            return ERR("could not create pin group'" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": ID " + std::to_string(id) + " is already taken");
        }

        // create pin group
        std::unique_ptr<PinGroup<GatePin>> pin_group_owner(new PinGroup<GatePin>(id, name, direction, type, ascending, start_index));
        PinGroup<GatePin>* pin_group = pin_group_owner.get();
        m_pin_groups_new.push_back(std::move(pin_group_owner));
        m_pin_groups_ordered.push_back(pin_group);
        m_pin_groups_map[id] = pin_group;

        // mark pin group ID as used
        if (auto free_id_it = m_free_pin_group_ids.find(id); free_id_it != m_free_pin_group_ids.end())
        {
            m_free_pin_group_ids.erase(free_id_it);
        }
        m_used_pin_group_ids.insert(id);

        for (auto pin : pins)
        {
            if (pin == nullptr)
            {
                return ERR("could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": pin is a 'nullptr'");
            }

            if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
            {
                return ERR("could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": pin '" + pin->get_name() + "' with ID "
                           + std::to_string(pin->get_id()) + " does not belong to gate type");
            }

            if (PinGroup<GatePin>* pg = pin->get_group().first; pg != nullptr)
            {
                // remove from old group and delete old group if empty
                if (auto res = pg->remove_pin(pin); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": unable to remove pin '" + pin->get_name()
                                          + "' with ID " + std::to_string(pin->get_id()) + " from pin group '" + pg->get_name() + "' with ID " + std::to_string(pin->get_id()));
                }

                if (pg->empty())
                {
                    // erase pin group
                    u32 del_id = pin_group->get_id();
                    m_pin_groups_map.erase(del_id);
                    m_pin_groups_ordered.erase(std::find(m_pin_groups_ordered.begin(), m_pin_groups_ordered.end(), pin_group));
                    m_pin_groups_new.erase(std::find_if(m_pin_groups_new.begin(), m_pin_groups_new.end(), [pin_group](const auto& group) { return group.get() == pin_group; }));

                    // free pin group ID
                    m_free_pin_group_ids.insert(del_id);
                    m_used_pin_group_ids.erase(del_id);
                }
            }

            if (auto res = pin_group->assign_pin(pin); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": failed to assign pin to pin group");
            }
        }

        return OK(pin_group);
    }

    Result<PinGroup<GatePin>*> GateType::create_pin_group(const std::string& name, const std::vector<GatePin*> pins, PinDirection direction, PinType type, bool ascending, u32 start_index)
    {
        return create_pin_group(get_unique_pin_group_id(), name, pins, direction, type, ascending, start_index);
    }

    std::vector<PinGroup<GatePin>*> GateType::get_pin_groups_new(const std::function<bool(PinGroup<GatePin>*)>& filter) const
    {
        std::vector<PinGroup<GatePin>*> res;
        if (!filter)
        {
            res.reserve(m_pin_groups_ordered.size());
            res.insert(res.end(), m_pin_groups_ordered.begin(), m_pin_groups_ordered.end());
        }
        else
        {
            for (PinGroup<GatePin>* group : m_pin_groups_ordered)
            {
                if (filter(group))
                {
                    res.push_back(group);
                }
            }
        }
        return res;
    }

    Result<PinGroup<GatePin>*> GateType::get_pin_group_by_id(const u32 id) const
    {
        if (id == 0)
        {
            return ERR("could not get pin by ID: ID 0 is invalid");
        }

        if (const auto it = m_pin_groups_map.find(id); it != m_pin_groups_map.end())
        {
            return OK(it->second);
        }

        return ERR("could not get pin by ID: gate type '" + m_name + "' with ID " + std::to_string(m_id) + " contains no pin with ID " + std::to_string(id));
    }

    bool GateType::add_pin(const std::string& pin, PinDirection direction, PinType pin_type)
    {
        if (m_pins_set.find(pin) != m_pins_set.end())
        {
            log_error("gate_library", "pin '{}' could not be added to gate type '{}' since a pin with the same name does already exist.", pin, m_name);
            return false;
        }

        m_pins.push_back(pin);
        m_pins_set.insert(pin);
        m_pin_to_direction[pin] = direction;
        m_direction_to_pins[direction].insert(pin);
        if (!assign_pin_type(pin, pin_type))
        {
            return false;
        }

        return true;
    }

    bool GateType::add_pins(const std::vector<std::string>& pins, PinDirection direction, PinType pin_type)
    {
        bool success = true;

        for (const auto& pin : pins)
        {
            success &= add_pin(pin, direction, pin_type);
        }

        return success;
    }

    const std::vector<std::string>& GateType::get_pins() const
    {
        return m_pins;
    }

    PinDirection GateType::get_pin_direction(const std::string& pin) const
    {
        if (const auto it = m_pin_to_direction.find(pin); it != m_pin_to_direction.end())
        {
            return it->second;
        }

        return PinDirection::none;
    }

    const std::unordered_map<std::string, PinDirection>& GateType::get_pin_directions() const
    {
        return m_pin_to_direction;
    }

    std::unordered_set<std::string> GateType::get_pins_of_direction(PinDirection direction) const
    {
        if (const auto it = m_direction_to_pins.find(direction); it != m_direction_to_pins.end())
        {
            return it->second;
        }

        return {};
    }

    bool GateType::assign_pin_type(const std::string& pin, PinType pin_type)
    {
        if (m_pins_set.find(pin) != m_pins_set.end())
        {
            if (const auto it = m_pin_to_type.find(pin); it != m_pin_to_type.end())
            {
                m_type_to_pins.at(it->second).erase(pin);
            }
            m_pin_to_type[pin] = pin_type;
            m_type_to_pins[pin_type].insert(pin);
            return true;
        }

        log_error("gate_library", "could not assign type '{}' to pin '{}' of gate type '{}'.", enum_to_string<PinType>(pin_type), pin, m_name);
        return false;
    }

    PinType GateType::get_pin_type(const std::string& pin) const
    {
        if (const auto it = m_pin_to_type.find(pin); it != m_pin_to_type.end())
        {
            return it->second;
        }

        return PinType::none;
    }

    const std::unordered_map<std::string, PinType>& GateType::get_pin_types() const
    {
        return m_pin_to_type;
    }

    std::unordered_set<std::string> GateType::get_pins_of_type(PinType pin_type) const
    {
        if (const auto it = m_type_to_pins.find(pin_type); it != m_type_to_pins.end())
        {
            return it->second;
        }

        return {};
    }

    bool GateType::assign_pin_group(const std::string& group, const std::vector<std::pair<u32, std::string>>& pins)
    {
        if (m_pin_groups.find(group) != m_pin_groups.end())
        {
            log_error("gate_library", "pin group '{}' could not be added to gate type '{}' since a pin group with the same name already exists.", group, m_name);
            return false;
        }

        for (const auto& pin : pins)
        {
            if (m_pins_set.find(pin.second) == m_pins_set.end())
            {
                log_error("gate_library", "could not assign pin '{}' to group '{}' of gate type '{}'.", pin.second, group, m_name);
                return false;
            }
        }

        m_pin_groups[group] = pins;

        std::unordered_map<u32, std::string> index_to_pin;
        for (const auto& [index, pin] : pins)
        {
            index_to_pin[index] = pin;
            m_pin_to_group[pin] = group;
        }

        m_pin_group_indices[group] = index_to_pin;
        return true;
    }

    std::string GateType::get_pin_group(const std::string& pin) const
    {
        if (const auto it = m_pin_to_group.find(pin); it != m_pin_to_group.end())
        {
            return it->second;
        }

        return "";
    }

    const std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>>& GateType::get_pin_groups() const
    {
        return m_pin_groups;
    }

    std::vector<std::pair<u32, std::string>> GateType::get_pins_of_group(const std::string& group) const
    {
        if (const auto it = m_pin_groups.find(group); it != m_pin_groups.end())
        {
            return it->second;
        }

        log_error("gate_library", "pin group with name '{}' does not exist.", group);
        return {};
    }

    std::string GateType::get_pin_of_group_at_index(const std::string& group, const u32 index) const
    {
        if (const auto group_it = m_pin_group_indices.find(group); group_it != m_pin_group_indices.end())
        {
            const std::unordered_map<u32, std::string>& index_to_pin = group_it->second;
            if (const auto index_it = index_to_pin.find(index); index_it != index_to_pin.end())
            {
                return index_it->second;
            }

            log_error("gate_library", "pin group with name '{}' does not have a pin with index {}.", group, index);
            return "";
        }

        log_error("gate_library", "pin group with name '{}' does not exist.", group);
        return "";
    }

    i32 GateType::get_index_in_group_of_pin(const std::string& group, const std::string& pin) const
    {
        if (const auto group_it = m_pin_groups.find(group); group_it != m_pin_groups.end())
        {
            const std::vector<std::pair<u32, std::string>>& pin_indices = group_it->second;
            if (const auto index_it = std::find_if(pin_indices.begin(), pin_indices.end(), [pin](const std::pair<u32, std::string>& pin_index) { return pin == pin_index.second; });
                index_it != pin_indices.end())
            {
                return index_it->first;
            }

            log_error("gate_library", "pin group with name '{}' does not have a pin with name {}.", group, pin);
            return -1;
        }

        log_error("gate_library", "pin group with name '{}' does not exist.", group);
        return -1;
    }

    void GateType::add_boolean_function(const std::string& pin_name, const BooleanFunction& bf)
    {
        m_functions.emplace(pin_name, bf.clone());
    }

    void GateType::add_boolean_functions(const std::unordered_map<std::string, BooleanFunction>& functions)
    {
        for (const auto& [name, function] : functions)
        {
            m_functions.insert({name, function.clone()});
        }
    }

    const BooleanFunction GateType::get_boolean_function(const std::string& function_name) const
    {
        if (const auto it = m_functions.find(function_name); it != m_functions.end())
        {
            return std::get<1>(*it).clone();
        }

        return BooleanFunction();
    }

    const std::unordered_map<std::string, BooleanFunction>& GateType::get_boolean_functions() const
    {
        return m_functions;
    }
}    // namespace hal
