// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/utilities/log.h"
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
         * @param[in] ascending - Set `true` for ascending pin order (from 0 to n-1), `false` otherwise (from n-1 to 0). Defaults to `true`.
         * @param[in] start_index - The start index of the pin group. Defaults to `0`.
         * @param[in] ordered - Set `true` if the pin group features an inherent order, `false` otherwise. Defaults to `false`.
         */
        PinGroup(const u32 id, const std::string& name, PinDirection direction, PinType type, bool ascending = true, u32 start_index = 0, bool ordered = false)
            : m_id(id), m_name(name), m_direction(direction), m_type(type), m_ascending(ascending), m_lowest_index(start_index), m_highest_index(start_index), m_ordered(ordered)
        {
        }

        ~PinGroup() = default;

        /**
         * Check whether two pin groups are equal.
         *
         * @param[in] other - The pin group to compare against.
         * @returns `true` if both pin groups are equal, `false` otherwise.
         */
        bool operator==(const PinGroup<T>& other) const
        {
            if (m_id != other.get_id() || m_name != other.get_name() || m_direction != other.get_direction() || m_type != other.get_type() || m_lowest_index != other.get_lowest_index()
                || m_highest_index != other.get_highest_index() || m_ascending != other.is_ascending() || m_ordered != other.is_ordered())
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
         * @returns `true` if both pin groups are unequal, `false` otherwise.
         */
        bool operator!=(const PinGroup<T>& other) const
        {
            return !operator==(other);
        }

        /**
         * Hash function for python binding.
         *
         * @return Pybind11 compatible hash.
         */
        ssize_t get_hash() const
        {
            return (uintptr_t)this;
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
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
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
        Result<T*> get_pin_at_index(i32 index) const
        {
            if (index >= m_lowest_index && index <= m_highest_index)
            {
                auto it = m_pins.begin();
                if (m_ascending)
                {
                    std::advance(it, index - m_lowest_index);
                }
                else
                {
                    std::advance(it, m_highest_index - index);
                }
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
        Result<i32> get_index(const T* pin) const
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
         * Check whether the pin order of a pin group comprising n pins is ascending, e.g., from index `0` to `n-1`.
         * 
         * @returns `true` for ascending bit order, `false` otherwise.
         */
        bool is_ascending() const
        {
            return m_ascending;
        }

        /**
         * Check whether the pin order of a pin group comprising n pins is descending, e.g., from index `n-1` to `0`.
         * 
         * @returns `true` for descending bit order, `false` otherwise.
         */
        bool is_descending() const
        {
            return !m_ascending;
        }

        /**
         * Get the numerically lowest index of the pin group.
         *
         * @returns The lowest index.
         */
        i32 get_lowest_index() const
        {
            return m_lowest_index;
        }

        /**
         * Get the numerically highest index of the pin group.
         *
         * @returns The highest index.
         */
        i32 get_highest_index() const
        {
            return m_highest_index;
        }

        /**
         * Get the start index of the pin group.
         * For ascending pin groups, this index equals the lowest index of the pin group.
         * For descending pin groups, it is equal to the highest index of the pin group.
         * 
         * @returns The start index. 
         */
        i32 get_start_index() const
        {
            return m_ascending ? m_lowest_index : m_highest_index;
        }

        /**
         * Check whether the pin group features an inherent order.
         * 
         * @returns `true` if the pin group is inherently ordered, `false` otherwise.
         */
        bool is_ordered() const
        {
            return m_ordered;
        }

        /**
         * Set whether the pin group features an inherent order.
         * 
         * @param[in] ordered - Set `true` if the pin group is inherently ordered, `false` otherwise. Defaults to `true`.
         */
        void set_ordered(bool ordered = true)
        {
            m_ordered = ordered;
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
         * The pin will automatically assigned `highest_index+1` as its index, independent of whether the group is ascending or descending.
         * The `highest_index` of the group will be incremented by 1.
         * For ascending groups, the pin will be inserted at the end of the list of pins.
         * For descending groups, it will be inserted at the beginning of the list of pins. 
         * 
         * @param[in] pin - The pin to assign.
         * @returns true on success, false otherwise.
         */
        bool assign_pin(T* pin)
        {
            if (pin == nullptr)
            {
                log_warning("pin_group", "'nullptr' given instead of a pin when trying to assign a pin to pin group '{}' with ID {}", m_name, m_id);
                return false;
            }

            i32 index = m_pins.empty() ? m_highest_index : ++m_highest_index;

            if (m_ascending)
            {
                m_pins.push_back(pin);
            }
            else
            {
                m_pins.push_front(pin);
            }
            pin->m_group = std::make_pair(this, index);
            return true;
        }

        /**
         * Move a pin to another index within the pin group.
         * The indices of all pins between the old index and the new index will be affected as well, because they are incremented or decremented accordingly.
         * 
         * @param[in] pin - The pin to move.
         * @param[in] new_index - The index to move the pin to.
         * @returns Ok on success, an error message otherwise.
         */
        Result<std::monostate> move_pin(T* pin, i32 new_index)
        {
            if (pin == nullptr)
            {
                return ERR("'nullptr' given instead of a pin when trying to move pin within pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (pin->m_group.first != this)
            {
                return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (new_index >= m_lowest_index && new_index <= m_highest_index)
            {
                if (m_ascending)
                {
                    i32 old_index = pin->m_group.second;
                    if (old_index == new_index)
                    {
                        return OK({});
                    }

                    i32 direction = (old_index > new_index) ? 1 : -1;

                    // move pin within vector
                    m_pins.erase(std::next(m_pins.begin(), old_index - m_lowest_index));
                    auto it = std::next(m_pins.begin(), new_index - m_lowest_index);
                    it      = m_pins.insert(it, pin);

                    // update indices; 'it' now points to inserted pin
                    for (i32 i = new_index; i != old_index; i += direction)
                    {
                        std::advance(it, direction);
                        std::get<1>((*it)->m_group) += direction;
                    }
                    std::get<1>(pin->m_group) = new_index;
                }
                else
                {
                    i32 old_index = pin->m_group.second;
                    if (old_index == new_index)
                    {
                        return OK({});
                    }

                    i32 direction = (old_index < new_index) ? 1 : -1;

                    // move pin within vector
                    m_pins.erase(std::next(m_pins.begin(), m_highest_index - old_index));
                    auto it = std::next(m_pins.begin(), m_highest_index - new_index);
                    it      = m_pins.insert(it, pin);

                    // update indices; 'it' now points to inserted pin
                    for (i32 i = new_index; i != old_index; i -= direction)
                    {
                        std::advance(it, direction);
                        std::get<1>((*it)->m_group) -= direction;
                    }
                    std::get<1>(pin->m_group) = new_index;
                }
            }
            else
            {
                return ERR("new index (" + std::to_string(new_index) + ") for pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " is not between lowest index ("
                           + std::to_string(m_lowest_index) + ") and highest index (" + std::to_string(m_highest_index - 1) + ") of pin group '" + m_name + "' with ID " + std::to_string(m_id));
            }

            return OK({});
        }

        /**
         * Remove a pin from the pin group.
         * The indices of all pins with a higher index than the removed pin will be reduced by 1.
         * The `highest_index` will also be reduced by 1.
         * 
         * @param[in] pin - The pin to remove.
         * @returns `true` on success, `false` otherwise.
         */
        bool remove_pin(T* pin)
        {
            if (pin == nullptr)
            {
                log_warning("pin_group", "'nullptr' given instead of a pin when trying to remove pin from pin group '{}' with ID {}.", m_name, m_id);
                return false;
            }

            if (pin->m_group.first != this)
            {
                log_warning("pin_group", "pin '{}' with ID {} does not belong to pin group '{}' with ID {}.", pin->get_name(), pin->get_id(), m_name, m_id);
                return false;
            }

            i32 index    = pin->m_group.second;
            pin->m_group = std::make_pair(nullptr, 0);

            // iterate until at pin to be removed, reduce all indices by 1 on the way
            // start at highest indices (rbegin() for ascending and begin() for descending)
            if (m_ascending)
            {
                auto it = std::next(m_pins.begin(), index - m_lowest_index);
                it      = m_pins.erase(it);
                for (; it != m_pins.end(); it++)
                {
                    std::get<1>((*it)->m_group)--;
                }
            }
            else
            {
                auto it = m_pins.begin();
                for (int i = m_highest_index; i > index; i--)
                {
                    std::get<1>((*(it++))->m_group)--;
                }
                m_pins.erase(it);
            }

            m_highest_index = m_pins.empty() ? m_lowest_index : --m_highest_index;

            return true;
        }

        /**
         * Check whether the pin group contains the given pin.
         * 
         * @param[in] pin - The pin to check.
         * @returns `true` if the pin group contains the pin, `false` otherwise.
         */
        bool contains_pin(T* pin)
        {
            if (pin == nullptr)
            {
                return false;
            }

            if (pin->m_group.first != this)
            {
                return false;
            }

            return true;
        }

    private:
        u32 m_id;
        std::string m_name;
        PinDirection m_direction;
        PinType m_type;
        std::list<T*> m_pins;
        bool m_ascending;
        i32 m_lowest_index;
        i32 m_highest_index;
        bool m_ordered;

        PinGroup(const PinGroup&)            = delete;
        PinGroup(PinGroup&&)                 = delete;
        PinGroup& operator=(const PinGroup&) = delete;
        PinGroup& operator=(PinGroup&&)      = delete;
    };
}    // namespace hal
