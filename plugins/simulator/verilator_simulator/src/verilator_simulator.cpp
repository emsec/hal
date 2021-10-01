#include "verilator_simulator/verilator_simulator.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
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
        parameter << "parameter [" << init_len - 1 << ":0]"
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
    std::vector<std::string> output_pins = gt->get_output_pins();

    // check if LUTComponent exists, if not abort
    if (LUTComponent* lut_component = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }); lut_component != nullptr) {
        if (!lut_component->is_init_ascending()) {
            std::reverse(input_pins.begin(), input_pins.end()); // needs to be reverted due to access in INIT string
        }
    } else {
        log_error("verilator_simulator", "cannot get LUTComponent, aborting...");
        return function.str();
    }

    if (output_pins.size() > 1) {
        log_error("verilator_simulator", "unsupported reached: currently only supporting LUTs with one output, split them into two and set the INIT string correctly :) ! aborting...");
        return function.str();
    }

    function << "wire [" << lut_size - 1 << ":0] lut_lookup = {";
    for (const auto& input_pin : input_pins) {
        function << input_pin << ", ";
    }
    function.seekp(-2, function.cur); // remove the additional colon and space
    function << "};" << std::endl;

    for (const auto& output_pin : output_pins) {
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

    std::vector<std::string> output_pins = gt->get_output_pins();
    if (output_pins.size() > 1) {
        log_error("verilator_simulator", "unsupported reached: currently only supporting FFs with one output! aborting...");
        return std::string();
    }

    function << "reg Q_reg;" << std::endl;
    function << "reg nQ_reg;" << std::endl;
    function << "wire new_clock;" << std::endl;
    function << std::endl;

    function << "initial begin" << std::endl;
    function << "\tQ_reg = INIT;" << std::endl;
    function << "\tnQ_reg = not(INIT);" << std::endl;
    function << "end" << std::endl;

    function << std::endl;

    std::unordered_set<std::string> clock_pins = gt->get_pins_of_type(hal::PinType::clock);
    if (clock_pins.size() != 1) {
        log_error("verilator_simulator", "unsupported reached: currently only supporting FFs with one clock! aborting...");
        return std::string();
    }

    if (FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); }); ff_component != nullptr) {

        auto clock_function = ff_component->get_clock_function();
        if (clock_function.is_empty()) {
            log_error("verilator_simulator", "gate: {} has no clock function in gate_library. you need to set one! aborting...", gt->get_name());
            return std::string();
        }

        function << "assign new_clock = " << clock_function << ";" << std::endl;
        function << "always @(posedge new_clock";

        bool reset_async = false;
        bool set_async = false;

        if (!ff_component->get_async_reset_function().is_empty()) {
            reset_async = true;
            auto reset_function = ff_component->get_async_reset_function();
            for (const auto& var : reset_function.get_variables()) {
                function << " or posedge " << var;
            }
        }

        if (!ff_component->get_async_set_function().is_empty()) {

            set_async = true;
            auto set_function = ff_component->get_async_set_function();
            for (const auto& var : set_function.get_variables()) {
                function << " or posedge " << var;
            }
        }
        function << ")" << std::endl;
        function << "begin" << std::endl;

        bool if_used = false;

        if (reset_async && set_async) {
            auto [behav_state, behave_neg_state] = ff_component->get_async_set_reset_behavior();

            function << (if_used ? "\telse if (" : "\tif (") << ff_component->get_async_reset_function().to_string() << ")" << std::endl;

            switch (behav_state) {
            case AsyncSetResetBehavior::L:
                /* code */
                break;

            default:
                break;
            }

            if_used = true;

            // TODO: insert case when FF has both async RESET and SET
            log_error("verilator_simulator", "unsupported reached: FFs with both async reset and set not supported");
            return std::string();
        }
        if (reset_async) {
            function << (if_used ? "\telse if (" : "\tif (") << ff_component->get_async_reset_function().to_string() << ")" << std::endl;
            function << "\t\tQ_reg <= 1'b0;" << std::endl;
            if_used = true;
        }
        if (set_async) {
            function << (if_used ? "\telse if (" : "\tif (") << ff_component->get_async_set_function().to_string() << ")" << std::endl;
            function << "\t\tQ_reg <= 1'b1;" << std::endl;
            if_used = true;
        }

        function << (if_used ? "\telse\n" : "");
        function << (if_used ? "\t" : "") << "\tQ_reg <= " << ff_component->get_next_state_function() << ";" << std::endl;
        function << "end" << std::endl;

        for (const auto& output_pin : output_pins) {
            function << "assign " << output_pin << " = ";
            if (gt->get_pin_type(output_pin) == hal::PinType::state) {
                function << "Q_reg";
            } else if (gt->get_pin_type(output_pin) == hal::PinType::neg_state) {
                function << "!Q_reg";
            } else {
                log_error("verilator_simulator", "unsupported reached: FF has weird outpin '{}' for gate '{}', aborting...", output_pin, gt->get_name());
                return std::string();
            }
            function << ";" << std::endl;
        }

    } else {
        log_error("verilator_simulator", "cannot get FFComponent, aborting...");
        return function.str();
    }

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
                log_info("verilator_simulator", "unimplemented reached: gate type: '{}', cannot create simulation model...", gate_type->get_name());
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