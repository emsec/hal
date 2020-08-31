#include "hal_core/netlist/endpoint.h"

namespace hal
{
    Endpoint::Endpoint(Gate* gate, const std::string& pin, Net* net, bool is_a_destination) : m_gate(gate), m_pin(pin), m_net(net), m_is_a_destination(is_a_destination)
    {
    }

    Gate* Endpoint::get_gate() const
    {
        return m_gate;
    }

    std::string Endpoint::get_pin() const
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
