#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
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
            std::string get_function_for_lut(const GateType* gt)
            {
                std::stringstream function;
                std::vector<GatePin*> input_pins  = gt->get_input_pins();
                std::vector<GatePin*> output_pins = gt->get_output_pins();
                u32 lut_size                      = input_pins.size();

                const LUTComponent* lut_component = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
                if (lut_component == nullptr)
                {
                    log_error("verilator", "cannot get LUTComponent, aborting...");
                    return function.str();
                }

                const auto& pin_cfgs = lut_component->get_output_pin_configs();
                if (pin_cfgs.empty())
                {
                    log_error("verilator", "LUT gate type '{}' has no output pin configs, aborting...", gt->get_name());
                    return function.str();
                }

                if (output_pins.size() > 1)
                {
                    log_error("verilator", "unsupported reached: currently only supporting LUTs with one output, split them into two and set the LUT_INIT string correctly :) ! aborting...");
                    return function.str();
                }

                const std::string& init_identifier = pin_cfgs.begin()->second.init_identifier;

                std::reverse(input_pins.begin(), input_pins.end());    // needs to be reverted due to access in LUT_INIT string
                function << "wire [" << lut_size - 1 << ":0] lut_lookup = {";
                for (const auto input_pin : input_pins)
                {
                    function << input_pin->get_name() << ", ";
                }
                function.seekp(-2, function.cur);    // remove the additional colon and space
                function << "};" << std::endl;

                for (const auto& output_pin : output_pins)
                {
                    function << "assign " << output_pin->get_name() << " = " << init_identifier << "[lut_lookup];" << std::endl;
                }

                return function.str();
            }

        }    // namespace converter
    }        // namespace verilator
}    // namespace hal
