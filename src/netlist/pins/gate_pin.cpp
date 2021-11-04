#include "hal_core/netlist/pins/gate_pin.h"

namespace hal
{
    GatePin::GatePin(const std::string& name, PinDirection direction, PinType type) : m_name(name), m_direction(direction), m_type(type)
    {
    }

    const std::string& GatePin::get_name() const
    {
        return m_name;
    }

    PinDirection GatePin::get_direction() const
    {
        return m_direction;
    }

    PinType GatePin::get_type() const
    {
        return m_type;
    }

    const std::pair<PinGroup<GatePin>*, u32>& GatePin::get_group() const
    {
        return m_group;
    }
}    // namespace hal