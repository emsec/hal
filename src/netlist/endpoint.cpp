#include "netlist/endpoint.h"

namespace hal
{
    Endpoint::Endpoint(Gate* gate, const std::string& pin, bool is_a_destination) : m_gate(gate), m_pin(pin), m_is_a_destination(is_a_destination)
    {
    }

    bool Endpoint::operator<(const Endpoint& rhs) const
    {
        if (this->m_gate != rhs.m_gate)
        {
            return (this->m_gate < rhs.m_gate);
        }
        if (this->m_pin != rhs.m_pin)
        {
            return (this->m_pin < rhs.m_pin);
        }
        return (this->m_is_a_destination < rhs.m_is_a_destination);
    }

    bool Endpoint::operator==(const Endpoint& rhs) const
    {
        return (this->m_gate == rhs.m_gate) && (this->m_pin == rhs.m_pin) && (this->m_is_a_destination == rhs.m_is_a_destination);
    }

    bool Endpoint::operator!=(const Endpoint& rhs) const
    {
        return !(*this == rhs);
    }

    Gate* Endpoint::get_gate() const
    {
        return m_gate;
    }

    const std::string& Endpoint::get_pin() const
    {
        return m_pin;
    }

    bool Endpoint::is_destination_pin() const
    {
        return m_is_a_destination;
    }

    bool Endpoint::is_source_pin() const
    {
        return !m_is_a_destination;
    }
}
