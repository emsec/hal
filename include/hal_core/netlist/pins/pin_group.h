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

    // TODO template???
    class PinGroup
    {
    public:
        PinGroup(const std::string& name, bool ascending = true, u32 start_index = 0);
        ~PinGroup() = default;

        std::vector<GatePin*> get_pins() const;
        GatePin* get_pin(u32 index) const;
        GatePin* get_pin(const std::string& name) const;
        bool move_pin(GatePin* pin, u32 new_index);
        bool remove_pin(GatePin* pin);
        i32 get_index(const GatePin* pin) const;
        i32 get_index(const std::string& name) const;
        bool is_ascending() const;

    private:
        friend Module;

        PinGroup(const PinGroup&) = delete;
        PinGroup(PinGroup&&)      = delete;
        PinGroup& operator=(const PinGroup&) = delete;
        PinGroup& operator=(PinGroup&&) = delete;

        std::string m_name;
        std::list<GatePin*> m_pins;
        std::unordered_map<std::string, GatePin*> m_name_to_pin;
        u32 m_start_index;
        u32 m_next_index;
        bool m_ascending = true;

        bool assign_pin(GatePin* pin);
    };
}    // namespace hal