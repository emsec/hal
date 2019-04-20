#include "gate_decorator_system/gate_decorator.h"

#include "netlist/gate.h"

#include <assert.h>

gate_decorator::gate_decorator(std::shared_ptr<gate> const gate)
{
    assert(gate != nullptr);
    m_gate = gate;
}

std::shared_ptr<gate> gate_decorator::get_gate()
{
    return m_gate;
}
