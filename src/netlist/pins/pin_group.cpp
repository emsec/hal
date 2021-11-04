#include "hal_core/netlist/pins/pin_group.h"

#include "hal_core/netlist/pins/gate_pin.h"

namespace hal
{
    template<class T>
    PinGroup<T>::PinGroup(const std::string& name, bool ascending, u32 start_index) : m_name(name), m_ascending(m_ascending), m_start_index(start_index), m_next_index(start_index)
    {
    }

    template<class T>
    std::vector<T*> PinGroup<T>::get_pins() const
    {
        return std::vector<T*>(m_pins.begin(), m_pins.end());
    }

    template<class T>
    T* PinGroup<T>::get_pin(u32 index) const
    {
        if (index >= m_start_index && index < m_next_index)
        {
            auto it = m_pins.begin();
            std::advance(it, index - m_start_index);
            return *it;
        }
        return nullptr;
    }

    template<class T>
    T* PinGroup<T>::get_pin(const std::string& name) const
    {
        if (const auto it = m_name_to_pin.find(name); it != m_name_to_pin.end())
        {
            return it->second;
        }
        return nullptr;
    }

    template<class T>
    bool PinGroup<T>::move_pin(T* pin, u32 new_index)
    {
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

    template<class T>
    i32 PinGroup<T>::get_index(const T* pin) const
    {
        if (pin != nullptr && pin->m_group.first == this)
        {
            return pin->m_group.second;
        }
        return -1;
    }

    template<class T>
    i32 PinGroup<T>::get_index(const std::string& name) const
    {
        if (const T* pin = get_pin(name); pin != nullptr)
        {
            return get_index(pin);
        }
        return -1;
    }

    template<class T>
    bool PinGroup<T>::is_ascending() const
    {
        return m_ascending;
    }

    template<class T>
    bool PinGroup<T>::assign_pin(T* pin)
    {
        if (m_name_to_pin.find(pin->m_name) != m_name_to_pin.end() || pin->m_group.first != nullptr)
        {
            return false;
        }

        u32 index = m_next_index++;
        m_pins.push_back(pin);
        m_name_to_pin[pin->get_name()] = pin;
        pin->m_group                   = std::make_pair(this, index);
        return true;
    }

    template<class T>
    bool PinGroup<T>::remove_pin(T* pin)
    {
        // TODO this needs to happen externally to properly take care of moving the pin into a single-bit pin group
        if (pin == nullptr || pin->m_group.first != this)
        {
            return false;
        }

        u32 index    = pin->m_group.second;
        pin->m_group = std::make_pair(nullptr, 0);
        auto it      = std::next(m_pins.begin(), pin->m_group.second);
        it           = m_pins.erase(it);
        for (; it != m_pins.end(); it++)
        {
            std::get<1>((*it)->m_group)--;
        }
        m_next_index--;

        return true;
    }
}    // namespace hal