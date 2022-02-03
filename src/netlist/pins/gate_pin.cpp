#include "hal_core/netlist/pins/gate_pin.h"

namespace hal
{
    GatePin::GatePin(const u32 id, const std::string& name, PinDirection direction, PinType type) : BasePin<GatePin>(id, name, direction, type)
    {
    }
}    // namespace hal