#pragma once

#include "hal_core/defines.h"

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
        ~PinGroup() = default;

        /**
         * Check whether two pin groups are equal.
         *
         * @param[in] other - The pin group to compare against.
         * @returns True if both pin groups are equal, false otherwise.
         */
        bool operator==(const PinGroup<T>& other) const
        {
            if (m_name != other.get_name() || m_start_index != other.get_start_index() || m_ascending != other.is_ascending())
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
         * Get the name of the pin group.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_name() const
        {
            return m_name;
        }

        /**
         * Get the (ordered) pins of the pin groups.
         * 
         * @returns The ordered pins.
         */
        std::vector<T*> get_pins() const
        {
            return std::vector<T*>(m_pins.begin(), m_pins.end());
        }

        /**
         * Get the pin specified by the given index.
         * 
         * @param[in] index - The index of the pin within the pin group.
         * @returns The pin on success, a nullptr otherwise.
         */
        T* get_pin(u32 index) const
        {
            if (index >= m_start_index && index < m_next_index)
            {
                auto it = m_pins.begin();
                std::advance(it, index - m_start_index);
                return *it;
            }
            return nullptr;
        }

        /**
         * Get the pin specified by the given name.
         * 
         * @param[in] name - The name of the pin.
         * @returns The pin on success, a nullptr otherwise.
         */
        T* get_pin(const std::string& name) const
        {
            if (const auto it = m_pin_name_map.find(name); it != m_pin_name_map.end())
            {
                return it->second;
            }
            return nullptr;
        }

        /**
         * Get the index within the pin group of the given pin.
         * 
         * @param[in] pin - The pin.
         * @returns The index of the pin on success, -1 otherwise.
         */
        i32 get_index(const T* pin) const
        {
            if (pin != nullptr && pin->m_group.first == this)
            {
                return pin->m_group.second;
            }
            return -1;
        }

        /**
         * Get the index within the pin group of the pin specified by the given name.
         * 
         * @param[in] name - The name of the pin.
         * @returns The index of the pin on success, -1 otherwise.
         */
        i32 get_index(const std::string& name) const
        {
            if (const T* pin = get_pin(name); pin != nullptr)
            {
                return get_index(pin);
            }
            return -1;
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
         * Get the direction of the pin group.
         * 
         * @returns The direction of the pin group.
         */
        PinDirection get_direction() const
        {
            return m_direction;
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

    private:
        friend GateType;
        friend Module;

        std::string m_name;
        PinDirection m_direction;
        PinType m_type;
        std::list<T*> m_pins;
        std::unordered_map<std::string, T*> m_pin_name_map;
        bool m_ascending;
        u32 m_start_index;
        u32 m_next_index;

        PinGroup(const PinGroup&) = delete;
        PinGroup(PinGroup&&)      = delete;
        PinGroup& operator=(const PinGroup&) = delete;
        PinGroup& operator=(PinGroup&&) = delete;

        /**
         * Construct a new pin group from its name, pin order, and start index.
         * 
         * @param[in] name - The name of the pin group.
         * @param[in] direction - The direction of the pin group.
         * @param[in] type - The type of the pin group.
         * @param[in] ascending - True for ascending pin order (from 0 to n-1), false otherwise (from n-1 to 0).
         * @param[in] start_index - The start index of the pin group.
         */
        PinGroup(const std::string& name, PinDirection direction, PinType type, bool ascending = false, u32 start_index = 0)
            : m_name(name), m_direction(direction), m_type(type), m_ascending(ascending), m_start_index(start_index), m_next_index(start_index)
        {
        }

        /**
         * Assign a pin to the pin group.
         * 
         * @param[in] pin - The pin to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_pin(T* pin)
        {
            if (pin == nullptr)
            {
                return false;
            }

            if (m_pin_name_map.find(pin->m_name) != m_pin_name_map.end())
            {
                // pin with same name already exists within group
                return false;
            }

            u32 index = m_next_index++;
            m_pins.push_back(pin);
            m_pin_name_map[pin->get_name()] = pin;
            pin->m_group                    = std::make_pair(this, index);
            return true;
        }

        /**
         * Move a pin to another index within the pin group.
         * 
         * @param[in] pin - The pin to move.
         * @param[in] new_index - The index to move the pin to.
         * @returns True on success, false otherwise.
         */
        bool move_pin(T* pin, u32 new_index)
        {
            if (pin == nullptr)
            {
                return false;
            }

            if (new_index >= m_start_index && new_index < m_next_index && pin->m_group.first == this)
            {
                u32 old_index = pin->m_group.second;
                if (old_index == new_index)
                {
                    return true;
                }

                i32 direction             = (old_index < new_index) ? -1 : 1;
                auto it                   = std::next(m_pins.begin(), new_index - m_start_index);
                it                        = m_pins.insert(it, pin);
                std::get<1>(pin->m_group) = new_index;
                for (u32 i = new_index; i != old_index; i += direction)
                {
                    ++it;
                    std::get<1>((*it)->m_group) += direction;
                }

                return true;
            }
            return false;
        }

        /**
         * Remove a pin from the pin group.
         * 
         * @param[in] pin - The pin to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_pin(T* pin)
        {
            if (pin == nullptr || pin->m_group.first != this)
            {
                return false;
            }

            u32 index    = pin->m_group.second;
            pin->m_group = std::make_pair(nullptr, 0);
            auto it      = std::next(m_pins.begin(), index);
            it           = m_pins.erase(it);
            for (; it != m_pins.end(); it++)
            {
                std::get<1>((*it)->m_group)--;
            }
            m_next_index--;

            return true;
        }
    };
}    // namespace hal