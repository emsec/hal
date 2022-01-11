#include "verilator/verilator.h"

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

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>

namespace hal {

namespace verilator {
    namespace converter {

        bool convert_gate_library_to_verilog(const Netlist* nl, const std::filesystem::path verilator_sim_path, const std::filesystem::path model_path)
        {
            log_info("verilator", "converting {} to verilog for simulation", nl->get_gate_library()->get_name());

            std::set<GateType*> gate_types = get_gate_gate_types_from_netlist(nl);

            // get preset simulation gate models if path given
            std::filesystem::path gate_definitions_path = verilator_sim_path / "gate_definitions/";
            std::filesystem::create_directory(gate_definitions_path);
            std::set<std::string> provided_models;
            if (!model_path.empty()) {
                provided_models = get_provided_models(model_path, gate_definitions_path);
            }

            // create all remaining gate type simulation models
            for (const auto& gate_type : gate_types) {

                // check if model has been given
                if (provided_models.find(gate_type->get_name()) != provided_models.end()) {
                    log_debug("verilator", "using provided model for gate: {}", gate_type->get_name());
                    continue;
                }

                log_info("verilator", "creating verilog simulation model for {}", gate_type->get_name());
                std::stringstream gate_description;

                // insert prologue
                gate_description << get_prologue_for_gate_type(gate_type) << std::endl;

                // get function of gate
                std::string gate_function = get_function_for_gate(gate_type);
                if (gate_function.empty()) {
                    log_error("verilator", "unimplemented reached: gate type: '{}', cannot create simulation model...", gate_type->get_name());
                    return false;
                }
                gate_description << gate_function << std::endl;

                // insert epilogue
                gate_description << get_epilogue_for_gate_type(gate_type) << std::endl;

                // write file
                std::ofstream gate_file(gate_definitions_path / (gate_type->get_name() + ".v"));
                gate_file << gate_description.str();
                gate_file.close();
            }

            return true;
        }

        std::set<std::string> get_provided_models(const std::filesystem::path model_path, const std::filesystem::path gate_definition_path)
        {
            std::set<std::string> supported_gate_types;
            for (const auto& entry : std::filesystem::directory_iterator(model_path)) {
                std::string file = entry.path().filename();

                if (entry.path().extension() != ".v") {
                    log_info("verilator", "not reading {} from simulation path, since it is not verilog (no .v extension)", file);
                    continue;
                }

                file = utils::replace(file, std::string(".v"), std::string(""));
                supported_gate_types.insert(file);

                // copy gate lib to verilator folder
                std::filesystem::copy(model_path / entry, gate_definition_path);
            }

            if (!supported_gate_types.empty()) {

                log_info("verilator", "using provided gate simulation models for: ");
                for (const auto& gate : supported_gate_types) {
                    log_info("verilator", "\t{}", gate);
                }
            } else {
                log_info("verilator", "no gate types were provided");
            }

            return supported_gate_types;
        }

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
                bool lut_init_descending = false;

                if (LUTComponent* lut_component = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }); lut_component != nullptr) {
                    if (!lut_component->is_init_ascending()) {
                        lut_init_descending = true;
                    }
                } else {
                    log_error("verilator", "cannot get LUTComponent, aborting...");
                    return std::vector<std::string>();
                }

                std::stringstream parameter;

                if (lut_init_descending) {
                    parameter << "parameter [" << init_len - 1 << ":0]"
                              << " INIT = " << init_len << "'h" << std::setfill('0') << std::setw(init_len / 4) << 0 << ",";
                } else {
                    parameter << "parameter [0:" << init_len - 1 << "]"
                              << " INIT = " << init_len << "'h" << std::setfill('0') << std::setw(init_len / 4) << 0 << ",";
                }

                parameters.push_back(parameter.str());

            } else if (gt->has_property(hal::GateTypeProperty::ff) || gt->has_property(hal::GateTypeProperty::latch)) {
                if (InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); }); init_component != nullptr) {

                    std::stringstream parameter;
                    parameter << "parameter [0:0]"
                              << " INIT = 1'b0,";
                    parameters.push_back(parameter.str());
                }
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

            // add parameters to all gates
            if (!parameters.empty()) {
                prologue << "#(" << std::endl;
                std::string parameter_str;
                for (const auto& parameter : parameters) {
                    parameter_str += "\t" + parameter + "\n";
                }
                parameter_str.erase(parameter_str.find_last_of(","));
                prologue << parameter_str << std::endl;
                prologue << ")" << std::endl;
            }

            // in and outputs of module
            prologue << "(" << std::endl;

            for (const auto& input_pin : input_pins) {
                prologue << "\tinput " << input_pin << "," << std::endl;
            }
            for (const auto& output_pin : output_pins) {
                prologue << "\toutput " << output_pin << "," << std::endl;
            }

            prologue.seekp(-2, prologue.cur); // remove the additional colon and space

            prologue << std::endl;
            prologue << ");" << std::endl;
            prologue << std::endl;

            return prologue.str();
        }

        std::string get_function_for_gate(const GateType* gt)
        {
            std::stringstream gate_description;
            // insert gate specific function
            if (gt->has_property(hal::GateTypeProperty::lut)) {

                gate_description << verilator::converter::get_function_for_lut(gt) << std::endl;

            } else if (gt->has_property(hal::GateTypeProperty::combinational)) {

                gate_description << verilator::converter::get_function_for_combinational_gate(gt) << std::endl;

            } else if (gt->has_property(hal::GateTypeProperty::ff)) {

                gate_description << verilator::converter::get_function_for_ff(gt) << std::endl;

            } else if (gt->has_property(hal::GateTypeProperty::latch)) {

                gate_description << verilator::converter::get_function_for_latch(gt) << std::endl;
            }
            return gate_description.str();
        }

        std::string get_epilogue_for_gate_type(const GateType* gt)
        {
            std::stringstream epilogue;

            epilogue << "endmodule" << std::endl;

            return epilogue.str();
        }
    }

} // namespace verilator
}