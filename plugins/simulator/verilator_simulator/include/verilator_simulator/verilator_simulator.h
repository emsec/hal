#pragma once

#include "hal_core/defines.h"

#include <set>
#include <vector>

namespace hal {
class Netlist;
class Gate;
class GateType;
class Net;

namespace verilator_simulator {
    namespace converter {
        bool convert_gate_library_to_verilog(const Netlist* nl);

        std::set<GateType*> get_gate_gate_types_from_netlist(const Netlist* nl);

        std::vector<std::string> get_parameters_for_gate(const GateType* gt);

        std::string get_prologue_for_gate_type(const GateType* gt);

        // gate functionality
        std::string get_function_for_gate(const GateType* gt);
        std::string get_function_for_lut(const GateType* gt);
        std::string get_function_for_combinational_gate(const GateType* gt);
        std::string get_function_for_ff(const GateType* gt);
        std::string get_function_for_latch(const GateType* gt);

        std::string get_epilogue_for_gate_type(const GateType* gt);

    }
} // namespace verilator_simulator

} // namespace hal
