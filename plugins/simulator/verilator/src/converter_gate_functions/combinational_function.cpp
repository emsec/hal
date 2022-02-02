
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

                for (const auto& [output_pin, bf] : gt->get_boolean_functions())
                {
                    BooleanFunction bf_copy = bf;
                    function << "assign ";

                    if (std::string group = gt->get_pin_group(output_pin); !group.empty())
                    {
                        function << group << "[" << gt->get_index_in_group_of_pin(group, output_pin) << "]";
                    }
                    else
                    {
                        function << output_pin;
                    }

                    for (const std::string& var : bf_copy.get_variable_names())
                    {
                        if (std::string group = gt->get_pin_group(var); !group.empty())
                        {
                            std::string pin = group + "[" + std::to_string(gt->get_index_in_group_of_pin(group, var)) + "]";
                            bf_copy = bf_copy.substitute(var, pin);
                        }
                    }

                    function << " = " << bf_copy.to_string() << ";" << std::endl;
                }

                return function.str();
            }
        }    // namespace converter
    }        // namespace verilator
}    // namespace hal
