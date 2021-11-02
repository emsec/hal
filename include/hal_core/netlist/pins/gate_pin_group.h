#pragma once

#include "hal_core/defines.h"

#include <list>
#include <string>

namespace hal
{
    class GatePin;

    class GatePinGroup
    {
    public:
        GatePinGroup(const std::string& name, u32 start_index, i32 count_direction);

        bool assign_pin(GatePin* pin);
        const std::list<GatePin*>& get_pins() const;
        GatePin* get_pin(u32 index) const;
        u32 get_index(GatePin* pin) const;

        u32 get_start_index() const;
        u32 get_current_index() const;
        i32 get_count_direction() const;

    private:
        GatePinGroup(const GatePinGroup&) = delete;
        GatePinGroup(GatePinGroup&&)      = delete;
        GatePinGroup& operator=(const GatePinGroup&) = delete;
        GatePinGroup& operator=(GatePinGroup&&) = delete;

    protected:
        std::string m_name;
        std::list<GatePin*> m_pins;
        u32 m_start_index;
        u32 m_current_index;
        i32 m_count_direction;
    };
}    // namespace hal