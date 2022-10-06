
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "verilator/verilator.h"

#include <filesystem>
#include <sstream>
#include <vector>

namespace hal
{
    namespace verilator
    {
        namespace converter
        {
            std::string get_function_for_combinational_gate(const GateType* gt)
            {
                std::stringstream function;

                for (const auto& [output_pin_name, bf] : gt->get_boolean_functions())
                {
                    BooleanFunction bf_copy = bf;
                    function << "assign ";

                    const GatePin* output_pin = gt->get_pin_by_name(output_pin_name);
                    if (output_pin == nullptr)
                    {
                        continue;
                    }

                    const auto group = output_pin->get_group();
                    if (group.first->size() > 1)
                    {
                        function << group.first->get_name() << "[" << group.second << "]";
                    }
                    else
                    {
                        function << output_pin->get_name();
                    }

                    for (const std::string& var : bf_copy.get_variable_names())
                    {
                        if (const auto* pin = gt->get_pin_by_name(var); pin != nullptr)
                        {
                            const auto pin_group = pin->get_group();
                            std::string pin_str  = pin_group.first->get_name() + "[" + std::to_string(pin_group.second) + "]";
                            bf_copy              = bf_copy.substitute(var, pin_str);
                        }
                    }

                    function << " = " << bf_copy.to_string(verilog_function_printer) << ";" << std::endl;
                }

                return function.str();
            }
        }    // namespace converter
    }        // namespace verilator
}    // namespace hal
