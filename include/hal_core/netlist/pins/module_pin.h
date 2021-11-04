#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <string>

namespace hal
{
    class ModulePin : GatePin
    {
    public:
        ModulePin(const std::string& name, Net* net, PinDirection direction, PinType type = PinType::none);

        Net* get_net() const;
        const std::pair<PinGroup<ModulePin>*, u32>& get_group() const;

    private:
        friend PinGroup<ModulePin>;

        ModulePin(const ModulePin&) = delete;
        ModulePin(ModulePin&&)      = delete;
        ModulePin& operator=(const ModulePin&) = delete;
        ModulePin& operator=(ModulePin&&) = delete;

        Net* m_net;
        std::pair<PinGroup<ModulePin>*, u32> m_group = {nullptr, 0};
    };
}    // namespace hal