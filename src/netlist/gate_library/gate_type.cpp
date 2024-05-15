#include "hal_core/netlist/gate_library/gate_type.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    GateType::GateType(GateLibrary* gate_library, u32 id, const std::string& name, std::set<GateTypeProperty> properties, std::unique_ptr<GateTypeComponent> component)
        : m_gate_library(gate_library), m_id(id), m_name(name), m_properties(properties), m_component(std::move(component))
    {
        m_next_pin_id       = 1;
        m_next_pin_group_id = 1;
    }

    ssize_t GateType::get_hash() const
    {
        return (uintptr_t)this;
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

    std::vector<GateTypeProperty> GateType::get_property_list() const
    {
        std::vector<GateTypeProperty> ctype_properties;
        std::vector<GateTypeProperty> other_properties;
        for (GateTypeProperty gtp : m_properties)
        {
            if (gtp >= GateTypeProperty::c_buffer)
                ctype_properties.push_back(gtp);
            else
                other_properties.push_back(gtp);
        }
        if (!other_properties.empty())
            ctype_properties.insert(ctype_properties.end(), other_properties.begin(), other_properties.end());
        return ctype_properties;
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

    std::ostream& operator<<(std::ostream& os, const GateType& gate_type)
    {
        return os << gate_type.to_string();
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

    Result<GatePin*> GateType::create_pin(const u32 id, const std::string& name, PinDirection direction, PinType type, bool create_group)
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
        if (m_pin_names_map.find(name) != m_pin_names_map.end())
        {
            return ERR("could not create pin '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": name '" + name + "' is already taken");
        }
        if (direction == PinDirection::none)
        {
            return ERR("could not create pin '" + name + "' for gate type '" + m_name + "' with " + std::to_string(m_id) + ": direction '" + enum_to_string(direction) + "' is invalid");
        }

        // create pin
        std::unique_ptr<GatePin> pin_owner(new GatePin(id, name, direction, type));
        GatePin* pin = pin_owner.get();
        m_pins.push_back(std::move(pin_owner));
        m_pins_map[id]        = pin;
        m_pin_names_map[name] = pin;

        // mark pin ID as used
        if (auto free_id_it = m_free_pin_ids.find(id); free_id_it != m_free_pin_ids.end())
        {
            m_free_pin_ids.erase(free_id_it);
        }
        m_used_pin_ids.insert(id);

        if (create_group)
        {
            if (auto res = create_pin_group(name, {pin}, direction, type); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not create pin '" + name + "' for gate type '" + m_name + "' with " + std::to_string(m_id) + ": failed to create pin group");
            }
        }

        return OK(pin);
    }

    Result<GatePin*> GateType::create_pin(const std::string& name, PinDirection direction, PinType type, bool create_group)
    {
        return create_pin(get_unique_pin_id(), name, direction, type, create_group);
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

    GatePin* GateType::get_pin_by_id(const u32 id) const
    {
        if (id == 0)
        {
            log_warning("gate_library", "could not get pin by ID for gate type '{}' with ID {}: ID 0 is invalid", m_name, std::to_string(m_id));
            return nullptr;
        }

        if (const auto it = m_pins_map.find(id); it != m_pins_map.end())
        {
            return it->second;
        }

        log_warning("gate_library", "could not get pin by ID for gate type '{}' with ID {}: no pin with ID {} exists", m_name, std::to_string(m_id), std::to_string(id));
        return nullptr;
    }

    GatePin* GateType::get_pin_by_name(const std::string& name) const
    {
        if (name.empty())
        {
            log_warning("gate_library", "could not get pin by name for gate type '{}' with ID {}: empty string provided as name", m_name, std::to_string(m_id));
            return nullptr;
        }

        if (const auto it = m_pin_names_map.find(name); it != m_pin_names_map.end())
        {
            return it->second;
        }

        log_warning("gate_library", "could not get pin by name for gate type '{}' with ID {}: no pin with name {} exists", m_name, std::to_string(m_id), name);
        return nullptr;
    }

    Result<PinGroup<GatePin>*> GateType::create_pin_group_internal(const u32 id, const std::string& name, PinDirection direction, PinType type, bool ascending, u32 start_index)
    {
        // some sanity checks
        if (id == 0)
        {
            return ERR("could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": ID 0 is invalid");
        }
        if (m_used_pin_group_ids.find(id) != m_used_pin_group_ids.end())
        {
            return ERR("could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": ID " + std::to_string(id) + " is already taken");
        }
        if (m_pin_group_names_map.find(name) != m_pin_group_names_map.end())
        {
            return ERR("could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": name '" + name + "' is already taken");
        }

        // create pin group
        std::unique_ptr<PinGroup<GatePin>> pin_group_owner(new PinGroup<GatePin>(id, name, direction, type, ascending, start_index));
        PinGroup<GatePin>* pin_group = pin_group_owner.get();
        m_pin_groups.push_back(std::move(pin_group_owner));
        m_pin_groups_ordered.push_back(pin_group);
        m_pin_groups_map[id]        = pin_group;
        m_pin_group_names_map[name] = pin_group;

        // mark pin group ID as used
        if (auto free_id_it = m_free_pin_group_ids.find(id); free_id_it != m_free_pin_group_ids.end())
        {
            m_free_pin_group_ids.erase(free_id_it);
        }
        m_used_pin_group_ids.insert(id);

        return OK(pin_group);
    }

    Result<PinGroup<GatePin>*> GateType::create_pin_group(const u32 id,
                                                          const std::string& name,
                                                          const std::vector<GatePin*> pins,
                                                          PinDirection direction,
                                                          PinType type,
                                                          bool ascending,
                                                          i32 start_index,
                                                          bool delete_empty_groups)
    {
        if (name.empty())
        {
            return ERR("could not create pin group for gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": empty string passed as name");
        }

        PinGroup<GatePin>* pin_group;
        if (!ascending && !pins.empty())
        {
            // compensate for shifting the start index
            start_index -= (pins.size() - 1);
        }

        if (auto res = create_pin_group_internal(id, name, direction, type, ascending, start_index); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not create pin group '" + name + "' for gate type '" + m_name + "' with ID " + std::to_string(m_id));
        }
        else
        {
            pin_group = res.get();
        }

        if (ascending)
        {
            for (auto it = pins.begin(); it != pins.end(); ++it)
                if (auto res = assign_pin_to_group(pin_group, *it, delete_empty_groups); res.is_error())
                {
                    assert(delete_pin_group(pin_group));
                    return ERR(res.get_error());
                }
        }
        else
        {
            for (auto it = pins.rbegin(); it != pins.rend(); ++it)
                if (auto res = assign_pin_to_group(pin_group, *it, delete_empty_groups); res.is_error())
                {
                    assert(delete_pin_group(pin_group));
                    return ERR(res.get_error());
                }
        }

        return OK(pin_group);
    }

    Result<PinGroup<GatePin>*>
        GateType::create_pin_group(const std::string& name, const std::vector<GatePin*> pins, PinDirection direction, PinType type, bool ascending, i32 start_index, bool delete_empty_groups)
    {
        return create_pin_group(get_unique_pin_group_id(), name, pins, direction, type, ascending, start_index, delete_empty_groups);
    }

    bool GateType::delete_pin_group_internal(PinGroup<GatePin>* pin_group)
    {
        // some sanity checks
        if (pin_group == nullptr)
        {
            log_warning("gate", "could not delete pin group of gate type '{}' with ID {}: pin group is a 'nullptr'", m_name, m_id);
            return false;
        }
        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            log_warning(
                "gate", "could not delete pin group '{}' with ID {} of gate type '{}' with ID {}: pin group does not belong to gate type", pin_group->get_name(), pin_group->get_id(), m_name, m_id);
        }

        // erase pin group
        u32 del_id                  = pin_group->get_id();
        const std::string& del_name = pin_group->get_name();
        m_pin_groups_map.erase(del_id);
        m_pin_group_names_map.erase(del_name);
        m_pin_groups_ordered.erase(std::find(m_pin_groups_ordered.begin(), m_pin_groups_ordered.end(), pin_group));
        m_pin_groups.erase(std::find_if(m_pin_groups.begin(), m_pin_groups.end(), [pin_group](const auto& pg) { return pg.get() == pin_group; }));

        // free pin group ID
        m_free_pin_group_ids.insert(del_id);
        m_used_pin_group_ids.erase(del_id);

        return true;
    }

    bool GateType::delete_pin_group(PinGroup<GatePin>* pin_group)
    {
        if (pin_group == nullptr)
        {
            log_warning("gate", "could not delete pin group from gate type '{}' with ID {}: pin group is a 'nullptr'", m_name, m_id);
            return false;
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            log_warning(
                "gate", "could not delete pin group '{}' with ID {} from gate type '{}' with ID {}: pin group does not belong to gate type", pin_group->get_name(), pin_group->get_id(), m_name, m_id);
            return false;
        }

        bool removed_pins = false;

        std::vector<GatePin*> pins_copy = pin_group->get_pins();
        for (auto* pin : pins_copy)
        {
            removed_pins = true;
            if (auto res = create_pin_group(pin->get_name(), {pin}, pin->get_direction(), pin->get_type(), true, 0, false); res.is_error())
            {
                log_warning("gate", "{}", res.get_error().get());
                return false;
            }
        }

        if (!delete_pin_group_internal(pin_group))
        {
            return false;
        }

        return true;
    }

    Result<std::monostate> GateType::assign_pin_to_group(PinGroup<GatePin>* pin_group, GatePin* pin, bool delete_empty_groups)
    {
        if (pin_group == nullptr)
        {
            return ERR("could not assign pin to pin group of gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": pin group is a 'nullptr'");
        }

        if (pin == nullptr)
        {
            return ERR("could not assign pin to pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " of gate type '" + m_name + "' with ID "
                       + std::to_string(m_id) + ": pin is a 'nullptr'");
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("could not assign pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " to pin group '" + pin_group->get_name() + "' with ID "
                       + std::to_string(pin_group->get_id()) + " of gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": pin group does not belong to gate type");
        }

        if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
        {
            return ERR("could not assign pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " to pin group '" + pin_group->get_name() + "' with ID "
                       + std::to_string(pin_group->get_id()) + " of gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": pin does not belong to gate type");
        }

        if (PinGroup<GatePin>* pg = pin->get_group().first; pg != nullptr)
        {
            // remove from old group and potentially delete old group if empty
            if (!pg->remove_pin(pin))
            {
                return ERR("could not assign pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " to pin group '" + pin_group->get_name() + "' with ID "
                           + std::to_string(pin_group->get_id()) + " of gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": unable to remove pin from pin group '" + pg->get_name()
                           + "' with ID " + std::to_string(pg->get_id()));
            }

            if (delete_empty_groups && pg->empty())
            {
                if (!delete_pin_group_internal(pg))
                {
                    return ERR("could not assign pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " to pin group '" + pin_group->get_name() + "' with ID "
                               + std::to_string(pin_group->get_id()) + " of gate type '" + m_name + "' with ID " + std::to_string(m_id) + ": unable to delete pin group '" + pg->get_name()
                               + "' with ID " + std::to_string(pg->get_id()));
                }
            }
        }

        if (!pin_group->assign_pin(pin))
        {
            return ERR("could not assign pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " to pin group '" + pin_group->get_name() + "' with ID "
                       + std::to_string(pin_group->get_id()) + " of gate type '" + m_name + "' with ID " + std::to_string(m_id));
        }

        return OK({});
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

    PinGroup<GatePin>* GateType::get_pin_group_by_id(const u32 id) const
    {
        if (id == 0)
        {
            log_warning("gate_library", "could not get pin group by ID for gate type '{}' with ID {}: ID 0 is invalid", m_name, std::to_string(m_id));
            return nullptr;
        }

        if (const auto it = m_pin_groups_map.find(id); it != m_pin_groups_map.end())
        {
            return it->second;
        }

        log_warning("gate_library", "could not get pin group by ID for gate type '{}' with ID {}: no pin group with ID {} exists", m_name, std::to_string(m_id), std::to_string(id));
        return nullptr;
    }

    PinGroup<GatePin>* GateType::get_pin_group_by_name(const std::string& name) const
    {
        if (name.empty())
        {
            log_warning("gate_library", "could not get pin group by name for gate type '{}' with ID {}: empty string provided as name", m_name, std::to_string(m_id));
            return nullptr;
        }

        if (const auto it = m_pin_group_names_map.find(name); it != m_pin_group_names_map.end())
        {
            return it->second;
        }

        log_debug("gate_library", "could not get pin group by name for gate type '{}' with ID {}: no pin group with name '{}' exists", m_name, std::to_string(m_id), name);
        return nullptr;
    }

    bool GateType::set_pin_group_name(PinGroup<GatePin>* pin_group, const std::string& new_name)
    {
        if (pin_group == nullptr)
        {
            log_warning("gate_type", "could not set name for pin group of gate type '{}' with ID {}: pin group is a 'nullptr'", m_name, m_id);
            return false;
        }

        if (new_name.empty())
        {
            log_warning("gate_type",
                        "could not set name for pin group '{}' with ID {} of gate type '{}' with ID {}: empty string passed as new name",
                        pin_group->get_name(),
                        pin_group->get_id(),
                        m_name,
                        m_id);
            return false;
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            log_warning("gate_type",
                        "could not set name for pin group '{}' with ID {} of gate type '{}' with ID {}: pin group does not belong to gate type",
                        pin_group->get_name(),
                        pin_group->get_id(),
                        m_name,
                        m_id);
            return false;
        }

        if (m_pin_group_names_map.find(new_name) != m_pin_group_names_map.end())
        {
            log_warning("gate_type",
                        "could not set name for pin group '{}' with ID {} of gate type '{}' with ID {}: a pin group with name '{}' already exists within the gate type",
                        pin_group->get_name(),
                        pin_group->get_id(),
                        m_name,
                        m_id,
                        new_name);
            return false;
        }

        if (const std::string& old_name = pin_group->get_name(); old_name != new_name)
        {
            m_pin_group_names_map.erase(old_name);
            pin_group->set_name(new_name);
            m_pin_group_names_map[new_name] = pin_group;
        }

        return true;
    }

    bool GateType::set_pin_group_type(PinGroup<GatePin>* pin_group, PinType new_type)
    {
        if (pin_group == nullptr)
        {
            log_warning("gate_type", "could not set type for pin group of gate type '{}' with ID {}: pin group is a 'nullptr'", m_name, m_id);
            return false;
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            log_warning("gate_type",
                        "could not set type for pin group '{}' with ID {} of gate type '{}' with ID {}: pin group does not belong to gate type",
                        pin_group->get_name(),
                        pin_group->get_id(),
                        m_name,
                        m_id);
            return false;
        }

        pin_group->set_type(new_type);
        return true;
    }

    bool GateType::set_pin_group_direction(PinGroup<GatePin>* pin_group, PinDirection new_direction)
    {
        if (pin_group == nullptr)
        {
            log_warning("gate_type", "could not set direction for pin group of gate type '{}' with ID {}: pin group is a 'nullptr'", m_name, m_id);
            return false;
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            log_warning("gate_type",
                        "could not set direction for pin group '{}' with ID {} of gate type '{}' with ID {}: pin group does not belong to gate type",
                        pin_group->get_name(),
                        pin_group->get_id(),
                        m_name,
                        m_id);
            return false;
        }

        pin_group->set_direction(new_direction);
        return true;
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

    BooleanFunction GateType::get_boolean_function(const std::string& name) const
    {
        if (name.empty())
        {
            log_warning("gate_library", "could not get Boolean function of gate type '{}' with ID {}: empty string provided as name", m_name, std::to_string(m_id));
            return BooleanFunction();
        }

        if (const auto it = m_functions.find(name); it != m_functions.end())
        {
            return std::get<1>(*it).clone();
        }

        log_warning("gate_library", "could not get Boolean function with name '{}' at gate type '{}' with ID {}: no function with that name exists", name, m_name, std::to_string(m_id));
        return BooleanFunction();
    }

    BooleanFunction GateType::get_boolean_function(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            auto output_pins = get_output_pins();
            if (output_pins.empty())
            {
                log_warning("gate_library", "could not get Boolean function of gate type '{}' with ID {}: gate type has no output pins", m_name, std::to_string(m_id));
                return BooleanFunction();
            }
            pin = output_pins.at(0);
        }
        return get_boolean_function(pin->get_name());
    }

    const std::unordered_map<std::string, BooleanFunction>& GateType::get_boolean_functions() const
    {
        return m_functions;
    }
}    // namespace hal
