#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "verilator/verilator.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace hal
{
    namespace verilator
    {
        namespace converter
        {
            std::string get_function_for_lut(const GateType* gt)
            {
                std::stringstream function;

                const LUTComponent* lut_component = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
                if (lut_component == nullptr)
                {
                    log_error("verilator", "cannot get LUTComponent for gate type '{}', aborting...", gt->get_name());
                    return {};
                }

                const auto& pin_cfgs = lut_component->get_output_pin_configs();
                if (pin_cfgs.empty())
                {
                    log_error("verilator", "LUT gate type '{}' has no output pin configs, aborting...", gt->get_name());
                    return {};
                }

                for (const auto& [pin_name, cfg] : pin_cfgs)
                {
                    if (cfg.input_pins.empty())
                    {
                        log_error("verilator", "LUT gate type '{}': output pin '{}' has no input pins configured, aborting...", gt->get_name(), pin_name);
                        return {};
                    }

                    // Sanitize pin name for use as a Verilog wire identifier
                    std::string safe_name = pin_name;
                    for (char& c : safe_name)
                    {
                        if (!std::isalnum(static_cast<unsigned char>(c)))
                        {
                            c = '_';
                        }
                    }
                    const std::string lookup_wire = "lut_lookup_" + safe_name;

                    // Reverse input order so that the LSB input lands at index 0,
                    // matching the INIT bit addressing convention used in get_lut_function.
                    std::vector<std::string> inputs = cfg.input_pins;
                    std::reverse(inputs.begin(), inputs.end());

                    function << "wire [" << inputs.size() - 1 << ":0] " << lookup_wire << " = {";
                    for (const auto& ipin : inputs)
                    {
                        function << ipin << ", ";
                    }
                    function.seekp(-2, function.cur);
                    function << "};\n";

                    function << "assign " << pin_name << " = " << cfg.init_identifier << "[" << lookup_wire;
                    if (cfg.bit_offset > 0)
                    {
                        function << " + " << cfg.bit_offset;
                    }
                    function << "];\n";
                }

                return function.str();
            }

        }    // namespace converter
    }    // namespace verilator
}    // namespace hal
