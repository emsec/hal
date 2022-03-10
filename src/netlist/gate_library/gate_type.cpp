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
        m_pins.push_back(std::move(pin_owner));
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

    std::vector<GatePin*> GateType::get_pins(const std::function<bool(GatePin*)>& filter) const
    {
        std::vector<GatePin*> res;
        if (!filter)
        {
            res.reserve(m_pins.size());
            for (const auto& group : m_pin_groups)
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

    std::vector<std::string> GateType::get_pin_names(const std::function<bool(GatePin*)>& filter) const
    {
        std::vector<std::string> res;
        if (!filter)
        {
            res.reserve(m_pins.size());
            for (const auto& group : m_pin_groups)
            {
                std::vector<GatePin*> pins = group->get_pins();
                for (const auto pin : group->get_pins())
                {
                    res.push_back(pin->get_name());
                }
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
                        res.push_back(pin->get_name());
                    }
                }
            }
        }
        return res;
    }

    std::vector<GatePin*> GateType::get_input_pins() const
    {
        return get_pins([](const GatePin* pin) {
            PinDirection direction = pin->get_direction();
            return direction == PinDirection::input || direction == PinDirection::inout;
        });
    }

    std::vector<std::string> GateType::get_input_pin_names() const
    {
        return get_pin_names([](const GatePin* pin) {
            PinDirection direction = pin->get_direction();
            return direction == PinDirection::input || direction == PinDirection::inout;
        });
    }

    std::vector<GatePin*> GateType::get_output_pins() const
    {
        return get_pins([](const GatePin* pin) {
            PinDirection direction = pin->get_direction();
            return direction == PinDirection::output || direction == PinDirection::inout;
        });
    }

    std::vector<std::string> GateType::get_output_pin_names() const
    {
        return get_pin_names([](const GatePin* pin) {
            PinDirection direction = pin->get_direction();
            return direction == PinDirection::output || direction == PinDirection::inout;
        });
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

    Result<GatePin*> GateType::get_pin_by_name(const std::string& name) const
    {
        if (auto pins = get_pins_by_name(name); pins.is_error())
        {
            return ERR(pins.get_error());
        }
        else
        {
            auto res = pins.get();
            if (res.empty())
            {
                return ERR("could not get pin by name '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": no pin matches the given name");
            }
            else if (res.size() > 1)
            {
                return ERR("could not get pin by name '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": multiple pins matche the given name");
            }
            else
            {
                return OK(res.front());
            }
        }
    }

    Result<std::vector<GatePin*>> GateType::get_pins_by_name(const std::string& name) const
    {
        if (name.empty())
        {
            return ERR("could not get pins by name for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": empty string provided as name");
        }

        std::vector<GatePin*> res;
        for (const auto& pin : m_pins)
        {
            if (pin->get_name() == name)
            {
                res.push_back(pin.get());
            }
        }

        return OK(res);
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
        m_pin_groups.push_back(std::move(pin_group_owner));
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
                    m_pin_groups.erase(std::find_if(m_pin_groups.begin(), m_pin_groups.end(), [pin_group](const auto& group) { return group.get() == pin_group; }));

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

    std::vector<PinGroup<GatePin>*> GateType::get_pin_groups(const std::function<bool(PinGroup<GatePin>*)>& filter) const
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

    Result<PinGroup<GatePin>*> GateType::get_pin_group_by_name(const std::string& name) const
    {
        if (auto pin_groups = get_pin_groups_by_name(name); pin_groups.is_error())
        {
            return ERR(pin_groups.get_error());
        }
        else
        {
            auto res = pin_groups.get();
            if (res.empty())
            {
                return ERR("could not get pin group by name '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": no pin group matches the given name");
            }
            else if (res.size() > 1)
            {
                return ERR("could not get pin group by name '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": multiple pin groups matche the given name");
            }
            else
            {
                return OK(res.front());
            }
        }
    }

    Result<std::vector<PinGroup<GatePin>*>> GateType::get_pin_groups_by_name(const std::string& name) const
    {
        if (name.empty())
        {
            return ERR("could not get pin groups by name for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": empty string provided as name");
        }

        std::vector<PinGroup<GatePin>*> res;
        for (const auto& pin_group : m_pin_groups)
        {
            if (pin_group->get_name() == name)
            {
                res.push_back(pin_group.get());
            }
        }

        return OK(res);
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

    Result<BooleanFunction> GateType::get_boolean_function(const std::string& name) const
    {
        if (name.empty())
        {
            return ERR("could not get Boolean function of gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": empty string provided as name");
        }

        if (const auto it = m_functions.find(name); it != m_functions.end())
        {
            return OK(std::get<1>(*it).clone());
        }

        return ERR("could not get Boolean function with name '" + name + "' at gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": no function with that name exists");
    }

    Result<BooleanFunction> GateType::get_boolean_function(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            auto output_pins = get_output_pins();
            if (output_pins.empty())
            {
                return ERR("could not get Boolean function of gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": gate type has no output pins");
            }
            pin = output_pins.at(0);
        }

        if (auto res = get_boolean_function(pin->get_name()); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            return res;
        }

        return ERR("could not get Boolean function of pin '" + pin->get_name() + "' at gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": no function with that name exists");
    }

    const std::unordered_map<std::string, BooleanFunction>& GateType::get_boolean_functions() const
    {
        return m_functions;
    }
}    // namespace hal
