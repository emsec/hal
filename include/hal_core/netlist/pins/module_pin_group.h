#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/pins/gate_pin_group.h"

#include <list>
#include <string>

namespace hal
{
    class ModulePin;

    class ModulePinGroup : GatePinGroup
    {
    public:
        ModulePinGroup(const std::string& name, u32 start_index, i32 count_direction);
        bool move_pin(ModulePin* pin, u32 index);

    private:
        ModulePinGroup(const ModulePinGroup&) = delete;
        ModulePinGroup(ModulePinGroup&&)      = delete;
        ModulePinGroup& operator=(const ModulePinGroup&) = delete;
        ModulePinGroup& operator=(ModulePinGroup&&) = delete;
    };
}    // namespace hal