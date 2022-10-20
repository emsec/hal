#include "hal_core/netlist/endpoint.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/pins/gate_pin.h"

namespace hal
{
    Endpoint::Endpoint(Gate* gate, GatePin* pin, Net* net, bool is_a_destination) : m_gate(gate), m_pin(pin), m_net(net), m_is_a_destination(is_a_destination)
    {
    }

    bool Endpoint::operator==(const Endpoint& other) const
    {
        return *m_pin == *other.get_pin() && m_is_a_destination == other.is_destination_pin() && *m_gate == *other.get_gate() && *m_net == *other.get_net();
    }

    bool Endpoint::operator!=(const Endpoint& other) const
    {
        return !operator==(other);
    }

    Gate* Endpoint::get_gate() const
    {
        return m_gate;
    }

    GatePin* Endpoint::get_pin() const
    {
        return m_pin;
    }

    Net* Endpoint::get_net() const
    {
        return m_net;
    }

    bool Endpoint::is_destination_pin() const
    {
        return m_is_a_destination;
    }

    bool Endpoint::is_source_pin() const
    {
        return !m_is_a_destination;
    }
}    // namespace hal
