#pragma once

#include "hal_core/defines.h"

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace hal
{
    class Module;
    class GatePin;

    template<class T>
    class PinGroup
    {
    public:
        PinGroup(const std::string& name, bool ascending = true, u32 start_index = 0);
        ~PinGroup() = default;

        std::vector<T*> get_pins() const;
        T* get_pin(u32 index) const;
        T* get_pin(const std::string& name) const;
        bool move_pin(T* pin, u32 new_index);
        i32 get_index(const T* pin) const;
        i32 get_index(const std::string& name) const;
        bool is_ascending() const;

    private:
        friend Module;

        std::string m_name;
        std::list<T*> m_pins;
        std::unordered_map<std::string, T*> m_name_to_pin;
        u32 m_start_index;
        u32 m_next_index;
        bool m_ascending = true;

        PinGroup(const PinGroup&) = delete;
        PinGroup(PinGroup&&)      = delete;
        PinGroup& operator=(const PinGroup&) = delete;
        PinGroup& operator=(PinGroup&&) = delete;
        bool assign_pin(T* pin);
        bool remove_pin(T* pin);
    };
}    // namespace hal