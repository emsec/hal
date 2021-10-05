#include "verilator_simulator/verilator_simulator.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <filesystem>
#include <sstream>
#include <vector>

namespace hal {
namespace verilator_simulator {
    namespace converter {
        std::string get_function_for_ff(const GateType* gt)
        {
            std::stringstream function;

            if (FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); }); ff_component != nullptr) {

                std::vector<std::string> output_pins = gt->get_output_pins();

                function << "reg Q_reg;" << std::endl;
                function << "reg QN_reg;" << std::endl;
                function << std::endl;
                function << "wire new_clock;" << std::endl;
                function << std::endl;

                if (InitComponent* init_component = ff_component->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); }); init_component != nullptr) {

                    function << "initial begin" << std::endl;
                    function << "\tQ_reg = INIT;" << std::endl;
                    function << "\tQN_reg = not(INIT);" << std::endl;
                    function << "end" << std::endl;
                }

                function << std::endl;

                std::unordered_set<std::string> clock_pins = gt->get_pins_of_type(hal::PinType::clock);
                if (clock_pins.size() != 1) {
                    log_error("verilator_simulator", "unsupported reached: currently only supporting FFs with one clock! aborting...");
                    return std::string();
                }
                auto clock_function = ff_component->get_clock_function();
                auto async_reset_function = ff_component->get_async_reset_function();
                auto async_set_function = ff_component->get_async_set_function();

                bool reset_async = false;
                bool set_async = false;

                if (clock_function.is_empty()) {
                    log_error("verilator_simulator", "gate: {} has no clock function in gate_library. you need to set one! aborting...", gt->get_name());
                    return std::string();
                }

                // create new signals for clock, async reset/set that already have function in them
                function << "assign new_clock = " << clock_function << ";" << std::endl;
                if (!async_reset_function.is_empty()) {
                    reset_async = true;
                    function << "wire new_async_reset;" << std::endl;
                    function << "assign new_async_reset = " << async_reset_function.to_string() << ";" << std::endl;
                    function << std::endl;
                }
                if (!async_set_function.is_empty()) {
                    set_async = true;
                    function << "wire new_async_set;" << std::endl;
                    function << "assign new_async_set = " << async_set_function.to_string() << ";" << std::endl;
                    function << std::endl;
                }

                function << std::endl;

                // begin process
                function << "always @(posedge new_clock";
                if (reset_async) {
                    function << " or posedge new_async_reset";
                }

                if (set_async) {
                    function << " or posedge new_async_set";
                }
                function << ")" << std::endl;
                function << "begin" << std::endl;

                bool if_used = false;

                if (reset_async && set_async) {
                    auto [behav_state, behave_neg_state] = ff_component->get_async_set_reset_behavior();

                    function << (if_used ? "\telse if (" : "\tif (") << async_reset_function.to_string() << " & " << async_set_function << ") begin" << std::endl;

                    switch (behav_state) {
                    case AsyncSetResetBehavior::L:
                        function << "\t\tQ_reg <= 1'b0;" << std::endl;
                        break;
                    case AsyncSetResetBehavior::H:
                        function << "\t\tQ_reg <= 1'b1;" << std::endl;
                        break;
                    case AsyncSetResetBehavior::T:
                        function << "\t\tQ_reg <= not(Q_reg);" << std::endl;
                        break;
                    case AsyncSetResetBehavior::N:
                        function << "\t\tQ_reg <= Q_reg;" << std::endl;
                        break;
                    case AsyncSetResetBehavior::X:
                        function << "\t\tQ_reg <= 1'bX;" << std::endl;
                        break;

                    default:
                        log_error("verilator_simulator", "unimplemented reached: found weird AsyncSetResetBehaviour for gate: {}", gt->get_name());
                        return std::string();
                        break;
                    }

                    switch (behave_neg_state) {
                    case AsyncSetResetBehavior::L:
                        function << "\t\tQN_reg <= 1'b0;" << std::endl;
                        break;
                    case AsyncSetResetBehavior::H:
                        function << "\t\tQN_reg <= 1'b1;" << std::endl;
                        break;
                    case AsyncSetResetBehavior::T:
                        function << "\t\tQN_reg <= not(QN_reg);" << std::endl;
                        break;
                    case AsyncSetResetBehavior::N:
                        function << "\t\tQN_reg <= QN_reg;" << std::endl;
                        break;
                    case AsyncSetResetBehavior::X:
                        function << "\t\tQN_reg <= 1'bX;" << std::endl;
                        break;

                    default:
                        log_error("verilator_simulator", "unimplemented reached: found weird AsyncSetResetBehaviour for gate: {}", gt->get_name());
                        return std::string();
                        break;
                    }
                    function << "\tend" << std::endl;

                    if_used = true;
                }
                if (reset_async) {
                    function << (if_used ? "\telse if (" : "\tif (") << async_reset_function.to_string() << ") begin" << std::endl;
                    function << "\t\tQ_reg <= 1'b0;" << std::endl;
                    function << "\t\tQN_reg <= 1'b1;" << std::endl;
                    function << "\tend" << std::endl;
                    if_used = true;
                }
                if (set_async) {
                    function << (if_used ? "\telse if (" : "\tif (") << async_set_function.to_string() << ") begin" << std::endl;
                    function << "\t\tQ_reg <= 1'b1;" << std::endl;
                    function << "\t\tQN_reg <= 1'b0;" << std::endl;
                    function << "\tend" << std::endl;

                    if_used = true;
                }

                // else case of normal case if DFF
                function << (if_used ? "\telse begin\n" : "");
                function << (if_used ? "\t" : "") << "\tQ_reg <= " << ff_component->get_next_state_function() << ";" << std::endl;
                function << (if_used ? "\t" : "") << "\tQN_reg <= not(" << ff_component->get_next_state_function() << ");" << std::endl;
                function << "\tend" << std::endl;
                function << "end" << std::endl;

                // set output wires
                for (const auto& output_pin : output_pins) {
                    function << "assign " << output_pin << " = ";
                    if (gt->get_pin_type(output_pin) == hal::PinType::state) {
                        function << "Q_reg";
                    } else if (gt->get_pin_type(output_pin) == hal::PinType::neg_state) {
                        function << "QN_reg";
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
    }
}
}
