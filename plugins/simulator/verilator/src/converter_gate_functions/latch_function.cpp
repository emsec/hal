#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
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
            std::string get_function_for_latch(const GateType* gt)
            {
                std::stringstream function;

                if (LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* c) { return LatchComponent::is_class_of(c); }); latch_component != nullptr)
                {
                    std::vector<std::string> output_pins = gt->get_output_pins();
                    // TODO: JULIAN
                    // std::string internal_state           = latch_component->get_state_identifier();
                    // std::string internal_negated_state   = latch_component->get_neg_state_identifier();
                    std::string internal_state         = "IQ";
                    std::string internal_negated_state = "IQN";

                    function << "reg " << internal_state << ";" << std::endl;
                    function << "reg " << internal_negated_state << ";" << std::endl;
                    function << std::endl;

                    if (InitComponent* init_component = latch_component->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                        init_component != nullptr)
                    {
                        function << "initial begin" << std::endl;
                        function << "\t" << internal_state << " = INIT;" << std::endl;
                        function << "\t" << internal_negated_state << " = !(INIT);" << std::endl;
                        function << "end" << std::endl;
                    }
                    function << std::endl;

                    auto async_reset_function = latch_component->get_async_reset_function();
                    auto async_set_function   = latch_component->get_async_set_function();
                    auto enable_function      = latch_component->get_enable_function();
                    auto data_function        = latch_component->get_data_in_function();

                    bool reset_async = false;
                    bool set_async   = false;
                    bool enable      = false;

                    // create new signals for clock, async reset/set that already have function in them
                    if (!enable_function.is_empty())
                    {
                        enable = true;
                        function << "wire new_enable;" << std::endl;
                        function << "assign new_enable = " << enable_function.to_string(verilog_function_printer) << ";" << std::endl;
                        function << std::endl;
                    }
                    if (!async_reset_function.is_empty())
                    {
                        reset_async = true;
                        function << "wire new_async_reset;" << std::endl;
                        function << "assign new_async_reset = " << async_reset_function.to_string(verilog_function_printer) << ";" << std::endl;
                        function << std::endl;
                    }
                    if (!async_set_function.is_empty())
                    {
                        set_async = true;
                        function << "wire new_async_set;" << std::endl;
                        function << "assign new_async_set = " << async_set_function.to_string(verilog_function_printer) << ";" << std::endl;
                        function << std::endl;
                    }

                    function << std::endl;

                    // begin process
                    if (enable)
                    {
                        function << "always @(new_enable";

                        if (reset_async)
                        {
                            function << " or new_async_reset";
                        }

                        if (set_async)
                        {
                            function << " or new_async_set";
                        }
                    }
                    else if (reset_async)
                    {
                        function << "always @(new_async_set";

                        if (set_async)
                        {
                            function << " or new_async_set";
                        }
                    }
                    else if (set_async)
                    {
                        function << "always @(new_async_set";
                    }
                    else
                    {
                        log_error("verilator", "gate '{}' has no signals in process, please check gate lib, aborting...", gt->get_name());
                        return std::string();
                    }

                    function << ")" << std::endl;
                    function << "begin" << std::endl;

                    bool if_used = false;

                    if (reset_async && set_async)
                    {
                        auto [behav_state, behave_neg_state] = latch_component->get_async_set_reset_behavior();

                        function << (if_used ? "\telse if (" : "\tif (") << async_reset_function.to_string(verilog_function_printer) << " & " << async_set_function << ") begin" << std::endl;

                        switch (behav_state)
                        {
                            case AsyncSetResetBehavior::L:
                                function << "\t\t" << internal_state << " <= 1'b0;" << std::endl;
                                break;
                            case AsyncSetResetBehavior::H:
                                function << "\t\t" << internal_state << " <= 1'b1;" << std::endl;
                                break;
                            case AsyncSetResetBehavior::T:
                                function << "\t\t" << internal_state << " <= !(" << internal_state << ");" << std::endl;
                                break;
                            case AsyncSetResetBehavior::N:
                                function << "\t\t" << internal_state << " <= " << internal_state << ";" << std::endl;
                                break;
                            case AsyncSetResetBehavior::X:
                                function << "\t\t" << internal_state << " <= 1'bX;" << std::endl;
                                break;

                            default:
                                log_error("verilator", "unimplemented reached: found weird AsyncSetResetBehaviour for gate: {}", gt->get_name());
                                return std::string();
                                break;
                        }

                        switch (behave_neg_state)
                        {
                            case AsyncSetResetBehavior::L:
                                function << "\t\t" << internal_negated_state << " <= 1'b0;" << std::endl;
                                break;
                            case AsyncSetResetBehavior::H:
                                function << "\t\t" << internal_negated_state << " <= 1'b1;" << std::endl;
                                break;
                            case AsyncSetResetBehavior::T:
                                function << "\t\t" << internal_negated_state << " <= !(" << internal_negated_state << ");" << std::endl;
                                break;
                            case AsyncSetResetBehavior::N:
                                function << "\t\t" << internal_negated_state << " <= " << internal_negated_state << ";" << std::endl;
                                break;
                            case AsyncSetResetBehavior::X:
                                function << "\t\t" << internal_negated_state << " <= 1'bX;" << std::endl;
                                break;

                            default:
                                log_error("verilator", "unimplemented reached: found weird AsyncSetResetBehaviour for gate: {}", gt->get_name());
                                return std::string();
                                break;
                        }
                        function << "\tend" << std::endl;

                        if_used = true;
                    }
                    if (reset_async)
                    {
                        function << (if_used ? "\telse if (" : "\tif (") << async_reset_function.to_string(verilog_function_printer) << ") begin" << std::endl;
                        function << "\t\t" << internal_state << " <= 1'b0;" << std::endl;
                        function << "\t\t" << internal_negated_state << " <= 1'b1;" << std::endl;
                        function << "\tend" << std::endl;
                        if_used = true;
                    }
                    if (set_async)
                    {
                        function << (if_used ? "\telse if (" : "\tif (") << async_set_function.to_string(verilog_function_printer) << ") begin" << std::endl;
                        function << "\t\t" << internal_state << " <= 1'b1;" << std::endl;
                        function << "\t\t" << internal_negated_state << " <= 1'b0;" << std::endl;
                        function << "\tend" << std::endl;

                        if_used = true;
                    }

                    if (enable)
                    {
                        function << (if_used ? "\telse if (" : "\tif (") << enable_function.to_string(verilog_function_printer) << ") begin" << std::endl;
                        function << (if_used ? "\t" : "") << "\t" << internal_state << " <= " << data_function << ";" << std::endl;
                        function << (if_used ? "\t" : "") << "\t" << internal_negated_state << " <= !(" << data_function << ");" << std::endl;
                        function << "\tend" << std::endl;

                        // unncessary but for completeness
                        function << "\telse begin" << std::endl;
                        function << (if_used ? "\t" : "") << "\t" << internal_state << " <= " << internal_state << ";" << std::endl;
                        function << (if_used ? "\t" : "") << "\t" << internal_negated_state << " <= " << internal_negated_state << ";" << std::endl;
                        function << "\tend" << std::endl;
                        if_used = true;
                    }
                    else
                    {
                        // else case if no enable for latch is present

                        if (data_function.is_empty())
                        {
                            function << "\telse begin" << std::endl;
                            function << (if_used ? "\t" : "") << "\t" << internal_state << " <= " << internal_state << ";" << std::endl;
                            function << (if_used ? "\t" : "") << "\t" << internal_negated_state << " <= " << internal_negated_state << ";" << std::endl;
                            function << "\tend" << std::endl;
                        }
                        else
                        {
                            function << (if_used ? "\telse begin\n" : "");
                            function << (if_used ? "\t" : "") << "\t" << internal_state << " <= " << data_function << ";" << std::endl;
                            function << (if_used ? "\t" : "") << "\t" << internal_negated_state << " <= !(" << data_function << ");" << std::endl;
                            function << "\tend" << std::endl;
                        }
                    }

                    function << "end" << std::endl;

                    // set output wires
                    for (const auto& output_pin : output_pins)
                    {
                        function << "assign " << output_pin << " = ";
                        if (gt->get_pin_type(output_pin) == hal::PinType::state)
                        {
                            function << "" << internal_state << "";
                        }
                        else if (gt->get_pin_type(output_pin) == hal::PinType::neg_state)
                        {
                            function << "" << internal_negated_state << "";
                        }
                        else
                        {
                            log_error("verilator", "unsupported reached: Latch has weird outpin '{}' for gate '{}', aborting...", output_pin, gt->get_name());
                            return std::string();
                        }
                        function << ";" << std::endl;
                    }
                }
                else
                {
                    log_error("verilator", "cannot get FFComponent, aborting...");
                    return function.str();
                }

                return function.str();
            }
        }    // namespace converter
    }        // namespace verilator
}    // namespace hal
