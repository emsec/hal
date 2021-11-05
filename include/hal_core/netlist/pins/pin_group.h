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
    class GatePin;

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
         * Get the name of the pin group.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_name() const;

        /**
         * Get the (ordered) pins of the pin groups.
         * 
         * @returns The ordered pins.
         */
        std::vector<T*> get_pins() const;

        /**
         * Get the pin specified by the given index.
         * 
         * @param[in] index - The index of the pin within the pin group.
         * @returns The pin.
         */
        T* get_pin(u32 index) const;

        /**
         * Get the pin specified by the given name.
         * 
         * @param[in] name - The name of the pin.
         * @returns The pin.
         */
        T* get_pin(const std::string& name) const;

        /**
         * Get the index within the pin group of the given pin.
         * 
         * @param[in] pin - The pin.
         * @returns The index of the pin.
         */
        i32 get_index(const T* pin) const;

        /**
         * Get the index within the pin group of the pin specified by the given name.
         * 
         * @param[in] name - The name of the pin.
         * @returns The index of the pin.
         */
        i32 get_index(const std::string& name) const;

        /**
         * Check whether the pin order of a pin group comprising n pins is ascending (from 0 to n-1) or descending (from n-1 to 0).
         * 
         * @returns True for ascending bit order, false otherwise.
         */
        bool is_ascending() const;

        /**
         * Get the start index of the pin group.
         * Commonly, pin groups start at index 0, but this may not always be the case.
         * Note that the start index for a pin group comprising n pins is 0 independent of whether it is ascending or descending.
         * 
         * @returns The start index. 
         */
        u32 get_start_index() const;

        /**
         * Check whether the pin group is empty, i.e., contains no pins.
         * 
         * @returns True if the pin group is empty, false otherwise.
         */
        bool empty() const;

        /**
         * Get the size, i.e., the number of pins, of the pin group.
         * 
         * @returns The size of the pin group.
         */
        size_t size() const;

    private:
        friend GateType;
        friend Module;

        std::string m_name;
        std::list<T*> m_pins;
        std::unordered_map<std::string, T*> m_pin_name_map;
        u32 m_start_index;
        u32 m_next_index;
        bool m_ascending = true;

        PinGroup(const PinGroup&) = delete;
        PinGroup(PinGroup&&)      = delete;
        PinGroup& operator=(const PinGroup&) = delete;
        PinGroup& operator=(PinGroup&&) = delete;

        /**
         * Construct a new pin group from its name, pin order, and start index.
         * 
         * @param[in] name - The name of the pin group.
         * @param[in] ascending - True for ascending pin order (from 0 to n-1), false otherwise (from n-1 to 0).
         * @param[in] start_index - The start index of the pin group.
         */
        PinGroup(const std::string& name, bool ascending = false, u32 start_index = 0);

        /**
         * Assign a pin to the pin group.
         * 
         * @param[in] pin - The pin to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_pin(T* pin);

        /**
         * Move a pin to another index within the pin group.
         * 
         * @param[in] pin - The pin to move.
         * @param[in] new_index - The index to move the pin to.
         * @returns True on success, false otherwise.
         */
        bool move_pin(T* pin, u32 new_index);

        /**
         * Remove a pin from the pin group.
         * 
         * @param[in] pin - The pin to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_pin(T* pin);
    };
}    // namespace hal