#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "verilator/verilator.h"

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>
#include <cctype>

namespace hal
{

    namespace verilator
    {
        namespace converter
        {

            bool convert_gate_library_to_verilog(const Netlist* nl, const std::filesystem::path verilator_sim_path, const std::filesystem::path model_path)
            {
                log_info("verilator", "converting {} to verilog for simulation", nl->get_gate_library()->get_name());

                std::unordered_map<GateType*,std::string> gate_type_names = get_gate_gate_types_from_netlist(nl);

                // get preset simulation gate models if path given
                std::filesystem::path gate_definitions_path = verilator_sim_path / "gate_definitions/";
                std::filesystem::create_directory(gate_definitions_path);
                std::set<std::string> provided_models;
                if (!model_path.empty())
                {
                    provided_models = get_provided_models(model_path, gate_definitions_path);
                }

                // create all remaining gate type simulation models
                for (auto& it : gate_type_names)
                {
                    // check if model has been given
                    if (provided_models.find(it.first->get_name()) != provided_models.end())
                    {
                        log_debug("verilator", "using provided model for gate: {}", it.first->get_name());
                        continue;
                    }

                    bool replace_name = false;
                    if (it.second == it.first->get_name())
                        log_info("verilator", "creating verilog simulation model for '{}'", it.first->get_name());
                    else
                    {
                        log_info("verilator", "creating verilog simulation model for '{}' which corresponds to gate type '{}'",
                                 it.second, it.first->get_name());
                        replace_name = true;
                    }
                    std::stringstream gate_description;

                    // insert prologue
                    gate_description << get_prologue_for_gate_type(it.first) << std::endl;

                    // get function of gate
                    std::string gate_function = get_function_for_gate(it.first);
                    if (gate_function.empty())
                    {
                        log_error("verilator", "unimplemented reached: gate type: '{}', cannot create simulation model...", it.first->get_name());
                        return false;
                    }
                    gate_description << gate_function << std::endl;

                    // insert epilogue
                    gate_description << get_epilogue_for_gate_type() << std::endl;

                    // write file
                    std::ofstream gate_file(gate_definitions_path / (it.second + ".v"));
                    if (replace_name)
                    {
                        std::string buffer =  gate_description.str();
                        size_t pos = 0;
                        for (;;)
                        {
                            pos = buffer.find(it.first->get_name());
                            if (pos == std::string::npos) break;
                            buffer.replace(pos, it.first->get_name().size(), it.second);
                            pos += it.second.size();
                        }
                        gate_file << buffer;
                    }
                    else
                        gate_file << gate_description.str();
                    gate_file.close();
                }

                return true;
            }

            std::set<std::string> get_provided_models(const std::filesystem::path model_path, const std::filesystem::path gate_definition_path)
            {
                std::set<std::string> supported_gate_types;
                if (!std::filesystem::exists(model_path))
                {
                    log_warning("verilator", "provided_models path '{}' does not exist", model_path.string());
                    return supported_gate_types;
                }
                for (const auto& entry : std::filesystem::directory_iterator(model_path))
                {
                    std::string file = entry.path().filename();

                    if (entry.path().extension() != ".v")
                    {
                        log_info("verilator", "not reading {} from simulation path, since it is not verilog (no .v extension)", file);
                        continue;
                    }

                    file = utils::replace(file, std::string(".v"), std::string(""));
                    supported_gate_types.insert(file);

                    // copy gate lib to verilator folder
                    std::filesystem::copy(entry.path(), gate_definition_path);
                }

                if (!supported_gate_types.empty())
                {
                    log_info("verilator", "using provided gate simulation models for: ");
                    for (const auto& gate : supported_gate_types)
                    {
                        log_info("verilator", "\t{}", gate);
                    }
                }
                else
                {
                    log_info("verilator", "no gate types were provided");
                }

                return supported_gate_types;
            }

            std::unordered_map<GateType*,std::string> get_gate_gate_types_from_netlist(const Netlist* nl)
            {
                std::unordered_map<GateType*,std::string> gate_types;

                for (const auto& gate : nl->get_gates())
                {
                    gate_types.emplace(std::make_pair<GateType*,std::string>(gate->get_type(),get_name_for_gate_type(gate->get_type())));
                }

                return gate_types;
            }

            std::string get_name_for_gate_type(const GateType* gt)
            {
                std::stringstream retval;
                for (char cc : gt->get_name())
                {
                    if (isalnum(cc))
                    {
                        if (isdigit(cc) && retval.str().empty())
                           retval << 'G';
                        retval << cc;
                    }
                    else if (cc == '_')
                        retval << cc;
                    else
                    {
                        retval << '_' << std::hex << static_cast<unsigned int>(cc) << "_";
                    }
                }
                return retval.str();
            }

            std::vector<std::string> get_parameters_for_gate(const GateType* gt)
            {
                std::vector<std::string> parameters;
                // insert gate specific function
                if (gt->has_property(hal::GateTypeProperty::c_lut))
                {
                    u32 lut_size             = gt->get_input_pins().size();
                    u32 init_len             = 1 << lut_size;
                    bool lut_init_descending = false;

                    if (LUTComponent* lut_component = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }); lut_component != nullptr)
                    {
                        if (!lut_component->is_init_ascending())
                        {
                            lut_init_descending = true;
                        }
                    }
                    else
                    {
                        log_error("verilator", "cannot get LUTComponent, aborting...");
                        return std::vector<std::string>();
                    }

                    std::stringstream parameter;

                    InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                    
                    if (init_component == nullptr)
                    {
                        log_error("verilator", "Could not get init component for gate type {}!", gt->get_name());
                    }

                    if (lut_init_descending)
                    {
                        parameter << "parameter [" << init_len - 1 << ":0]"
                                  <<  " " << init_component->get_init_identifiers().front() << " = " << init_len << "'h" << std::setfill('0') << std::setw(init_len / 4) << 0 << ",";
                    }
                    else
                    {
                        parameter << "parameter [0:" << init_len - 1 << "]"
                                  <<  " " << init_component->get_init_identifiers().front() << " = " << init_len << "'h" << std::setfill('0') << std::setw(init_len / 4) << 0 << ",";
                    }

                    parameters.push_back(parameter.str());
                }
                else if (gt->has_property(hal::GateTypeProperty::ff) || gt->has_property(hal::GateTypeProperty::latch))
                {
                    if (InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); }); init_component != nullptr)
                    {
                        std::stringstream parameter;
                        parameter << "parameter [0:0]"
                                  << " " << init_component->get_init_identifiers().front() <<  "= 1'b0,";
                        parameters.push_back(parameter.str());
                    }
                }

                return parameters;
            }

            std::string get_prologue_for_gate_type(const GateType* gt)
            {
                std::stringstream prologue;

                prologue << "`timescale 1 ps/1 ps" << std::endl;

                prologue << "module " << gt->get_name() << std::endl;

                // some gates have parameters, which we need to insert

                std::vector<std::string> parameters = get_parameters_for_gate(gt);

                // add parameters to all gates
                if (!parameters.empty())
                {
                    prologue << "#(" << std::endl;
                    std::string parameter_str;
                    for (const auto& parameter : parameters)
                    {
                        parameter_str += "\t" + parameter + "\n";
                    }
                    parameter_str.erase(parameter_str.find_last_of(","));
                    prologue << parameter_str << std::endl;
                    prologue << ")" << std::endl;
                }

                // in and outputs of module
                prologue << "(" << std::endl;

                std::unordered_set<GatePin*> visited_pins;
                for (const auto pin : gt->get_pins())
                {
                    // check if pin was contained in a group that has already been dealt with
                    if (visited_pins.find(pin) != visited_pins.end())
                    {
                        continue;
                    }

                    PinDirection direction = pin->get_direction();
                    prologue << "\t" << enum_to_string(direction) << " ";

                    if (const auto* pin_group = pin->get_group().first; pin_group != nullptr)
                    {
                        const auto group_pins = pin_group->get_pins();
                        prologue << " [" << std::to_string(pin_group->get_index(group_pins.back()).get()) << ":" << std::to_string(pin_group->get_index(group_pins.front()).get()) << "] "
                                 << pin_group->get_name() << "," << std::endl;

                        for (const auto pin : group_pins)
                        {
                            visited_pins.insert(pin);
                        }
                    }
                    else
                    {
                        // append all connected pins
                        prologue << pin->get_name() << "," << std::endl;
                    }
                }

                prologue.seekp(-2, prologue.cur);    // remove the additional colon and space

                prologue << std::endl;
                prologue << ");" << std::endl;
                prologue << std::endl;

                return prologue.str();
            }

            Result<std::string> verilog_function_printer(const BooleanFunction::Node& node, std::vector<std::string>&& operands)
            {
                if (node.get_arity() != operands.size())
                {
                    return ERR("node arity of " + std::to_string(node.get_arity()) + " does not match number of operands of " + std::to_string(operands.size()));
                }

                switch (node.type)
                {
                    case BooleanFunction::NodeType::Constant: {
                        std::string str;
                        for (const auto& value : node.constant)
                        {
                            str = enum_to_string(value) + str;
                        }
                        return OK(std::to_string(node.constant.size()) + "'b" + str);
                    }
                    case BooleanFunction::NodeType::Index:
                        return OK(std::to_string(node.index));
                    case BooleanFunction::NodeType::Variable:
                        return OK(node.variable);
                    case BooleanFunction::NodeType::And:
                        return OK("(" + operands[0] + " & " + operands[1] + ")");
                    case BooleanFunction::NodeType::Not:
                        return OK("(! " + operands[0] + ")");
                    case BooleanFunction::NodeType::Or:
                        return OK("(" + operands[0] + " | " + operands[1] + ")");
                    case BooleanFunction::NodeType::Xor:
                        return OK("(" + operands[0] + " ^ " + operands[1] + ")");
                    default:
                        return ERR("unsupported node type '" + std::to_string(node.type) + "'");
                }
            }

            std::string get_function_for_gate(const GateType* gt)
            {
                std::stringstream gate_description;
                // insert gate specific function
                if (gt->has_property(hal::GateTypeProperty::c_lut))
                {
                    gate_description << verilator::converter::get_function_for_lut(gt) << std::endl;
                }
                else if (gt->has_property(hal::GateTypeProperty::combinational))
                {
                    gate_description << verilator::converter::get_function_for_combinational_gate(gt) << std::endl;
                }
                else if (gt->has_property(hal::GateTypeProperty::ff))
                {
                    gate_description << verilator::converter::get_function_for_ff(gt) << std::endl;
                }
                else if (gt->has_property(hal::GateTypeProperty::latch))
                {
                    gate_description << verilator::converter::get_function_for_latch(gt) << std::endl;
                }
                else if (gt->has_property(hal::GateTypeProperty::c_buffer))
                {
                    gate_description << verilator::converter::get_function_for_combinational_gate(gt) << std::endl;
                }
                return gate_description.str();
            }

            std::string get_epilogue_for_gate_type()
            {
                std::stringstream epilogue;

                epilogue << "endmodule" << std::endl;

                return epilogue.str();
            }
        }    // namespace converter

    }    // namespace verilator
}    // namespace hal
