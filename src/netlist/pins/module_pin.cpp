#include "hal_core/netlist/pins/module_pin.h"

namespace hal
{
    ModulePin::ModulePin(const std::string& name, Net* net, PinDirection direction, PinType type) : GatePin(name, direction, type), m_net(net)
    {
    }

    Net* ModulePin::get_net() const
    {
        return m_net;
    }

    const std::pair<PinGroup<ModulePin>*, u32>& ModulePin::get_group() const
    {
        return m_group;
    }
}    // namespace hal