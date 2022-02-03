#include "hal_core/netlist/pins/module_pin.h"

namespace hal
{
    ModulePin::ModulePin(const u32 id, const std::string& name, Net* net, PinDirection direction, PinType type) : BasePin(id, name, direction, type), m_net(net)
    {
    }

    bool ModulePin::operator==(const ModulePin& other) const
    {
        return m_net->get_id() == other.get_net()->get_id() && BasePin::operator==(other);
    }

    bool ModulePin::operator!=(const ModulePin& other) const
    {
        return !operator==(other);
    }

    Net* ModulePin::get_net() const
    {
        return m_net;
    }
}    // namespace hal