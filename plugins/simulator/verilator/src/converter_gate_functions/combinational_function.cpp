
#include "verilator/verilator.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <filesystem>
#include <sstream>
#include <vector>

namespace hal {
namespace verilator {
    namespace converter {
        std::string get_function_for_combinational_gate(const GateType* gt)
        {
            std::stringstream function;

            for (const auto& [output_pin, bf] : gt->get_boolean_functions()) {
                function << "assign " << output_pin << " = " << bf.to_string() << ";" << std::endl;
            }

            return function.str();
        }
    }
}
}
