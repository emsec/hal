#include "verilator_simulator/verilator_simulator.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <filesystem>
#include <sstream>
#include <vector>

namespace hal {

std::set<GateType*> get_gate_gate_types_from_netlist(const Netlist* nl)
{
    std::set<GateType*> gate_types;

    for (const auto& gate : nl->get_gates()) {
        gate_types.emplace(gate->get_type());
    }

    return gate_types;
}

std::vector<std::string> get_parameters_for_gate(const GateType* gt)
{
    std::vector<std::string> parameters;
    // insert gate specific function
    if (gt->has_property(hal::GateTypeProperty::lut)) {
        u32 lut_size = gt->get_input_pins().size();
        u32 init_len = 1 << lut_size;

        std::stringstream parameter;
        parameter << "parameter [" << init_len << ":0]"
                  << " INIT = " << init_len << "'h" << std::setfill('0') << std::setw(init_len / 4) << 0 << ";";

        parameters.push_back(parameter.str());

    } else if (gt->has_property(hal::GateTypeProperty::ff) || gt->has_property(hal::GateTypeProperty::latch)) {
        std::stringstream parameter;
        parameter << "parameter [0:0]"
                  << " INIT = 1'b0;";
        parameters.push_back(parameter.str());
    }

    return parameters;
}

std::string get_prologue_for_gate_type(const GateType* gt)
{
    std::stringstream prologue;

    std::vector<std::string> input_pins = gt->get_input_pins();
    std::vector<std::string> output_pins = gt->get_output_pins();

    prologue << "`timescale 1 ps/1 ps" << std::endl;

    prologue << "module " << gt->get_name() << std::endl;

    // some gates have parameters, which we need to insert

    std::vector<std::string> parameters = get_parameters_for_gate(gt);
    if (!parameters.empty()) {
        prologue << "#(" << std::endl;

        for (const auto& parameter : parameters) {
            prologue << "\t" << parameter << std::endl;
        }
        prologue << ")" << std::endl;
    }

    prologue << "(" << std::endl;

    for (const auto& input_pin : input_pins) {
        prologue << "\tinput wire " << input_pin << "," << std::endl;
    }
    for (const auto& output_pin : output_pins) {
        prologue << "\toutput wire " << output_pin << "," << std::endl;
    }

    prologue.seekp(-2, prologue.cur); // remove the additional colon and space

    prologue << std::endl;
    prologue << ");" << std::endl;
    prologue << std::endl;

    return prologue.str();
}

std::string get_function_for_lut(const GateType* gt)
{
    std::stringstream function;
    u32 lut_size = gt->get_input_pins().size();
    u32 init_len = 1 << lut_size;

    std::vector<std::string> input_pins = gt->get_input_pins();
    std::reverse(input_pins.begin(), input_pins.end()); // needs to be reverted due to acces in INIT string

    std::vector<std::string> output_pins = gt->get_output_pins();
    if (output_pins.size() > 1) {
        log_error("verilator_simulator", "currently only supporting LUTs with one output, aborting");
        return function.str();
    }

    function << "// lut size: " << lut_size << std::endl;
    function << "// init len: " << init_len << std::endl;

    function << "wire [" << lut_size - 1 << ":0] lut_lookup = {";
    for (const auto& input_pin : input_pins) {
        function << input_pin << ", ";
    }
    function.seekp(-2, function.cur); // remove the additional colon and space
    function << "};" << std::endl;

    for (const auto& output_pin : output_pins){
        function << "assign " << output_pin << " = INIT[lut_lookup];" << std::endl;
    }

    return function.str();
}

std::string get_function_for_combinational_gate(const GateType* gt)
{
    std::stringstream function;

    for (const auto& [output_pin, bf] : gt->get_boolean_functions()) {
        function << "assign " << output_pin << " = " << bf.to_string() << ";" << std::endl;
    }

    return function.str();
}

std::string get_function_for_ff(const GateType* gt)
{
    std::stringstream function;

    return function.str();
}

std::string get_function_for_latch(const GateType* gt)
{
    std::stringstream function;

    return function.str();
}

std::string get_function_for_gate(const GateType* gt)
{
    std::stringstream gate_description;
    // insert gate specific function
    if (gt->has_property(hal::GateTypeProperty::lut)) {

        gate_description << get_function_for_lut(gt) << std::endl;

    } else if (gt->has_property(hal::GateTypeProperty::combinational)) {

        gate_description << get_function_for_combinational_gate(gt) << std::endl;

    } else if (gt->has_property(hal::GateTypeProperty::ff)) {

        gate_description << get_function_for_ff(gt) << std::endl;

    } else if (gt->has_property(hal::GateTypeProperty::latch)) {

        gate_description << get_function_for_latch(gt) << std::endl;
    }
    return gate_description.str();
}

std::string get_epilogue_for_gate_type(const GateType* gt)
{
    std::stringstream epilogue;

    epilogue << "endmodule" << std::endl;

    return epilogue.str();
}

namespace verilator_simulator {
    bool convert_gate_library_to_verilog(const Netlist* nl)
    {
        log_info("verilator_simulator", "converting {} to verilog for simulation", nl->get_gate_library()->get_name());

        auto gl = nl->get_gate_library();

        std::set<GateType*> gate_types = get_gate_gate_types_from_netlist(nl);

        std::filesystem::path tmp_folder = "/tmp/hal/gate_types/";

        for (const auto& gate_type : gate_types) {

            log_info("verilator_simulator", "creating verilog simulation model for {}", gate_type->get_name());

            std::stringstream gate_description;

            // insert prologue
            gate_description << get_prologue_for_gate_type(gate_type) << std::endl;

            // get function of gate
            std::string gate_function = get_function_for_gate(gate_type);

            if (gate_function.empty()) {
                log_info("verilator_simulator", "unimplemented reached for gate type: '{}', cannot create simulation model...", gate_type->get_name());
                return false;
            }
            gate_description << gate_function << std::endl;

            // insert epilogue
            gate_description << get_epilogue_for_gate_type(gate_type) << std::endl;

            log_info("verilator_simulator", "verilog file: \n{}", gate_description.str());
        }

        return true;
    }

} // namespace verilator_simulator
}