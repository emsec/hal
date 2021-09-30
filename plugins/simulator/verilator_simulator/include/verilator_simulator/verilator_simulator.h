#pragma once

#include "hal_core/defines.h"

namespace hal {
class Netlist;
class Gate;
class Net;

namespace verilator_simulator {
    bool convert_gate_library_to_verilog(const Netlist* nl);
} // namespace verilator_simulator

} // namespace hal
