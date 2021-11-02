#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/pins/gate_pin.h"

#include <string>

namespace hal
{
    class ModulePin : GatePin
    {
    public:
        ModulePin(DataContainer* parent, const std::string& name, Net* net, PinDirection direction, PinType type = PinType::none);

        Net* get_net() const;

    private:
        friend Module;

        ModulePin(const ModulePin&) = delete;
        ModulePin(ModulePin&&)      = delete;
        ModulePin& operator=(const ModulePin&) = delete;
        ModulePin& operator=(ModulePin&&) = delete;

        Net* m_net;
    };
}    // namespace hal