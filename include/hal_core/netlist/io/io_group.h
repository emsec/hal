#pragma once

#include "hal_core/defines.h"

#include <list>
#include <string>

namespace hal
{
    class IOPin;

    class IOGroup
    {
    public:
        IOGroup(const std::string& name, u32 start_index, i32 count_direction);

        bool assign_pin(IOPin* pin);
        bool move_pin(IOPin* pin, u32 index);
        bool remove_pin(IOPin* pin);
        const std::list<IOPin*>& get_pins() const;
        IOPin* get_pin(u32 index) const;
        u32 get_index(IOPin* pin) const;

        u32 get_start_index() const;
        u32 get_current_index() const;
        i32 get_count_direction() const;

    private:
        IOGroup(const IOGroup&) = delete;
        IOGroup(IOGroup&&)      = delete;
        IOGroup& operator=(const IOGroup&) = delete;
        IOGroup& operator=(IOGroup&&) = delete;

        std::string m_name;
        std::list<IOPin*> m_pins;
        u32 m_start_index;
        u32 m_current_index;
        i32 m_count_direction;
    };
}    // namespace hal