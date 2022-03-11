#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace hal
{
    class Module;
    class GateType;

    /**
     * A group of pins made up of a name, the pins, a pin order, and a start index.
     * 
     * @ingroup pins
     */
    template<class T>
    class PinGroup
    {
    public:
        /**
         * Construct a new pin group from its name, pin order, and start index.
         * 
         * @param[in] id - The ID of the pin group.
         * @param[in] name - The name of the pin group.
         * @param[in] direction - The direction of the pin group.
         * @param[in] type - The type of the pin group.
         * @param[in] ascending - True for ascending pin order (from 0 to n-1), false otherwise (from n-1 to 0).
         * @param[in] start_index - The start index of the pin group.
         */
        PinGroup(const u32 id, const std::string& name, PinDirection direction, PinType type, bool ascending = false, u32 start_index = 0)
            : m_id(id), m_name(name), m_direction(direction), m_type(type), m_ascending(ascending), m_start_index(start_index), m_next_index(start_index)
        {
        }

        ~PinGroup() = default;

        /**
         * Check whether two pin groups are equal.
         *
         * @param[in] other - The pin group to compare against.
         * @returns True if both pin groups are equal, false otherwise.
         */
        bool operator==(const PinGroup<T>& other) const
        {
            if (m_id != other.get_id() || m_name != other.get_name() || m_direction != other.get_direction() || m_type != other.get_type() || m_start_index != other.get_start_index()
                || m_ascending != other.is_ascending())
            {
                return false;
            }

            std::vector<T*> other_pins = other.get_pins();
            if (m_pins.size() != other_pins.size())
            {
                return false;
            }

            auto this_it  = m_pins.begin();
            auto other_it = other_pins.begin();
            while (this_it != m_pins.end() && other_it != other_pins.end())
            {
                if (**this_it++ != **other_it++)
                {
                    return false;
                }
            }

            return true;
        }

        /**
         * Check whether two pin groups are unequal.
         *
         * @param[in] other - The pin group to compare against.
         * @returns True if both pin groups are unequal, false otherwise.
         */
        bool operator!=(const PinGroup<T>& other) const
        {
            return !operator==(other);
        }

        /**
         * Get the ID of the pin group. The ID is unique within an entity, e.g., a module or a gate type.
         * 
         * @return The ID of the pin group.
         */
        u32 get_id() const
        {
            return m_id;
        }

        /**
         * Set the name of the pin group.
         * 
         * @param[in] name - The name of the pin group.
         */
        void set_name(const std::string& name)
        {
            m_name = name;
        }

        /**
         * Get the name of the pin group.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_name() const
        {
            return m_name;
        }

        /**
         * Set the type of the pin group.
         * 
         * @param[in] type - The pin group type.
         */
        void set_type(PinType type)
        {
            m_type = type;
        }

        /**
         * Get the type of the pin group.
         * 
         * @returns The type of the pin group.
         */
        PinType get_type() const
        {
            return m_type;
        }

        /**
         * Set the direction of the pin group.
         * 
         * @param[in] direction - The direction of the pin group.
         */
        void set_direction(PinDirection direction)
        {
            m_direction = direction;
        }

        /**
         * Get the direction of the pin group.
         * 
         * @returns The direction of the pin group.
         */
        PinDirection get_direction() const
        {
            return m_direction;
        }

        /**
         * Get the (ordered) pins of the pin groups.
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.ondition.
         * 
         * @param[in] filter - An optional filter.
         * @returns The ordered pins.
         */
        std::vector<T*> get_pins(const std::function<bool(T*)>& filter = nullptr) const
        {
            if (!filter)
            {
                return std::vector<T*>(m_pins.begin(), m_pins.end());
            }
            else
            {
                std::vector<T*> res;
                for (T* pin : m_pins)
                {
                    if (filter(pin))
                    {
                        res.push_back(pin);
                    }
                }
                return res;
            }
        }

        /**
         * Get the pin specified by the given index.
         * 
         * @param[in] index - The index of the pin within the pin group.
         * @returns The pin on success, an error message otherwise.
         */
        Result<T*> get_pin_at_index(u32 index) const
        {
            if (index >= m_start_index && index < m_next_index)
            {
                auto it = m_pins.begin();
                std::advance(it, index - m_start_index);
                return OK(*it);
            }
            return ERR("no pin exists at index " + std::to_string(index) + " within pin group '" + m_name + "'");
        }

        /**
         * Get the index within the pin group of the given pin.
         * 
         * @param[in] pin - The pin.
         * @returns The index of the pin on success, an error message otherwise.
         */
        Result<u32> get_index(const T* pin) const
        {
            if (pin == nullptr)
            {
                return ERR("'nullptr' provided as pin when trying to retrieve index within pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (pin->m_group.first != this)
            {
                return ERR("provided pin '" + pin->get_name() + "' does not belong to pin group '" + m_name + "'");
            }

            return OK(pin->m_group.second);
        }

        /**
         * Check whether the pin order of a pin group comprising n pins is ascending (from 0 to n-1) or descending (from n-1 to 0).
         * 
         * @returns True for ascending bit order, false otherwise.
         */
        bool is_ascending() const
        {
            return m_ascending;
        }

        /**
         * Get the start index of the pin group.
         * Commonly, pin groups start at index 0, but this may not always be the case.
         * Note that the start index for a pin group comprising n pins is 0 independent of whether it is ascending or descending.
         * 
         * @returns The start index. 
         */
        u32 get_start_index() const
        {
            return m_start_index;
        }

        /**
         * Check whether the pin group is empty, i.e., contains no pins.
         * 
         * @returns True if the pin group is empty, false otherwise.
         */
        bool empty() const
        {
            return m_pins.empty();
        }

        /**
         * Get the size, i.e., the number of pins, of the pin group.
         * 
         * @returns The size of the pin group.
         */
        size_t size() const
        {
            return m_pins.size();
        }

        /**
         * Assign a pin to the pin group.
         * 
         * @param[in] pin - The pin to assign.
         * @returns Ok on success, an error message otherwise.
         */
        Result<std::monostate> assign_pin(T* pin)
        {
            if (pin == nullptr)
            {
                return ERR("'nullptr' given instead of a pin when trying to assign a pin to pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            u32 index = m_next_index++;
            m_pins.push_back(pin);
            m_pin_id_map[pin->get_id()] = pin;
            pin->m_group                = std::make_pair(this, index);
            return OK({});
        }

        /**
         * Move a pin to another index within the pin group.
         * 
         * @param[in] pin - The pin to move.
         * @param[in] new_index - The index to move the pin to.
         * @returns Ok on success, an error message otherwise.
         */
        Result<std::monostate> move_pin(T* pin, u32 new_index)
        {
            if (pin == nullptr)
            {
                return ERR("'nullptr' given instead of a pin when trying to move pin within pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (pin->m_group.first != this)
            {
                return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (new_index >= m_start_index && new_index < m_next_index)
            {
                u32 old_index = pin->m_group.second;
                if (old_index == new_index)
                {
                    return OK({});
                }

                i32 direction = (old_index > new_index) ? 1 : -1;
                m_pins.erase(std::next(m_pins.begin(), old_index - m_start_index));
                auto it = std::next(m_pins.begin(), new_index - m_start_index);
                it      = m_pins.insert(it, pin);
                for (u32 i = new_index; i != old_index; i += direction)
                {
                    std::advance(it, direction);
                    std::get<1>((*it)->m_group) += direction;
                }
                std::get<1>(pin->m_group) = new_index;

                return OK({});
            }
            else
            {
                return ERR("new index (" + std::to_string(new_index) + ") for pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " is not between start index ("
                           + std::to_string(m_start_index) + ") and end index(" + std::to_string(m_next_index - 1) + ") of pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }
        }

        /**
         * Remove a pin from the pin group.
         * 
         * @param[in] pin - The pin to remove.
         * @returns Ok on success, an error message otherwise.
         */
        Result<std::monostate> remove_pin(T* pin)
        {
            if (pin == nullptr)
            {
                return ERR("'nullptr' given instead of a pin when trying to remove pin from pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (pin->m_group.first != this)
            {
                return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            u32 index    = pin->m_group.second;
            pin->m_group = std::make_pair(nullptr, 0);
            auto it      = std::next(m_pins.begin(), index - m_start_index);
            it           = m_pins.erase(it);
            m_pin_id_map.erase(pin->get_id());
            for (; it != m_pins.end(); it++)
            {
                std::get<1>((*it)->m_group)--;
            }
            m_next_index--;

            return OK({});
        }

    private:
        u32 m_id;
        std::string m_name;
        PinDirection m_direction;
        PinType m_type;
        std::list<T*> m_pins;
        std::unordered_map<u32, T*> m_pin_id_map;
        bool m_ascending;
        u32 m_start_index;
        u32 m_next_index;

        PinGroup(const PinGroup&) = delete;
        PinGroup(PinGroup&&)      = delete;
        PinGroup& operator=(const PinGroup&) = delete;
        PinGroup& operator=(PinGroup&&) = delete;
    };
}    // namespace hal