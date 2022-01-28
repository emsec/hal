#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
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
            std::string get_function_for_ff(const GateType* gt)
            {
                std::stringstream function;

                if (FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); }); ff_component != nullptr)
                {
                    StateComponent* state_component;
                    if (state_component = ff_component->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); }); state_component == nullptr)
                    {
                        log_error("verilator", "gate: {} has no state information in gate_library. you need to set one! aborting...", gt->get_name());
                        return std::string();
                    }

                    std::vector<std::string> output_pins = gt->get_output_pins();
                    std::string internal_state           = state_component->get_state_identifier();
                    std::string internal_negated_state   = state_component->get_neg_state_identifier();

                    function << "reg " << internal_state << ";" << std::endl;
                    function << "reg " << internal_negated_state << ";" << std::endl;
                    function << std::endl;
                    function << "wire clock___internal___;" << std::endl;
                    function << std::endl;

                    if (InitComponent* init_component = ff_component->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                        init_component != nullptr)
                    {
                        function << "initial begin" << std::endl;
                        function << "\t" << internal_state << " = INIT;" << std::endl;
                        function << "\t" << internal_negated_state << " = !(INIT);" << std::endl;
                        function << "end" << std::endl;
                    }

                    function << std::endl;

                    std::unordered_set<std::string> clock_pins = gt->get_pins_of_type(hal::PinType::clock);
                    if (clock_pins.size() != 1)
                    {
                        log_error("verilator", "unsupported reached: currently only supporting FFs with one clock! aborting...");
                        return std::string();
                    }
                    BooleanFunction clock_function       = ff_component->get_clock_function();
                    BooleanFunction async_reset_function = ff_component->get_async_reset_function();
                    BooleanFunction async_set_function   = ff_component->get_async_set_function();

                    bool reset_async = false;
                    bool set_async   = false;

                    if (clock_function.is_empty())
                    {
                        log_error("verilator", "gate: {} has no clock function in gate_library. you need to set one! aborting...", gt->get_name());
                        return std::string();
                    }

                    // create new signals for clock, async reset/set that already have function in them
                    function << "assign clock___internal___ = " << clock_function << ";" << std::endl;
                    if (!async_reset_function.is_empty())
                    {
                        reset_async = true;
                        function << "wire async_reset___internal___;" << std::endl;
                        function << "assign async_reset___internal___ = " << async_reset_function.to_string() << ";" << std::endl;
                        function << std::endl;
                    }
                    if (!async_set_function.is_empty())
                    {
                        set_async = true;
                        function << "wire async_set___internal___;" << std::endl;
                        function << "assign async_set___internal___ = " << async_set_function.to_string() << ";" << std::endl;
                        function << std::endl;
                    }

                    function << std::endl;

                    // begin process
                    function << "always @(posedge clock___internal___";
                    if (reset_async)
                    {
                        function << " or posedge async_reset___internal___";
                    }

                    if (set_async)
                    {
                        function << " or posedge async_set___internal___";
                    }
                    function << ")" << std::endl;
                    function << "begin" << std::endl;

                    bool if_used = false;

                    if (reset_async && set_async)
                    {
                        auto [behav_state, behave_neg_state] = ff_component->get_async_set_reset_behavior();

                        function << (if_used ? "\telse if (" : "\tif (") << async_reset_function.to_string() << " & " << async_set_function << ") begin" << std::endl;

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
                        function << (if_used ? "\telse if (" : "\tif (") << async_reset_function.to_string() << ") begin" << std::endl;
                        function << "\t\t" << internal_state << " <= 1'b0;" << std::endl;
                        function << "\t\t" << internal_negated_state << " <= 1'b1;" << std::endl;
                        function << "\tend" << std::endl;
                        if_used = true;
                    }
                    if (set_async)
                    {
                        function << (if_used ? "\telse if (" : "\tif (") << async_set_function.to_string() << ") begin" << std::endl;
                        function << "\t\t" << internal_state << " <= 1'b1;" << std::endl;
                        function << "\t\t" << internal_negated_state << " <= 1'b0;" << std::endl;
                        function << "\tend" << std::endl;

                        if_used = true;
                    }

                    // else case of normal case if DFF
                    function << (if_used ? "\telse begin\n" : "");
                    function << (if_used ? "\t" : "") << "\t" << internal_state << " <= " << ff_component->get_next_state_function() << ";" << std::endl;
                    function << (if_used ? "\t" : "") << "\t" << internal_negated_state << " <= !(" << ff_component->get_next_state_function() << ");" << std::endl;
                    function << (if_used ? "\tend\n" : "");
                    function << "end" << std::endl;

                    // set output wires
                    for (const auto& output_pin : output_pins)
                    {
                        function << "assign " << output_pin << " = ";
                        if (gt->get_pin_type(output_pin) == hal::PinType::state)
                        {
                            function << internal_state;
                        }
                        else if (gt->get_pin_type(output_pin) == hal::PinType::neg_state)
                        {
                            function << internal_negated_state;
                        }
                        else
                        {
                            log_error("verilator", "unsupported reached: FF has weird outpin '{}' for gate '{}', aborting...", output_pin, gt->get_name());
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
