#include "hal_core/netlist/pins/gate_pin.h"

namespace hal
{
    GatePin::GatePin(const std::string& name, PinDirection direction, PinType type) : BasePin<GatePin>(name, direction, type)
    {
    }
}    // namespace hal