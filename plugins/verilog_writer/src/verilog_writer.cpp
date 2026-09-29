#include "verilog_writer/verilog_writer.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <fstream>

namespace hal
{
    Result<std::monostate> VerilogWriter::write(Netlist* netlist, const std::filesystem::path& file_path)
    {
        std::stringstream res_stream;

        if (netlist == nullptr)
        {
            return ERR("could not write netlist to Verilog file '" + file_path.string() + "': netlist is a 'nullptr'");
        }

        if (file_path.empty())
        {
            return ERR("could not write netlist to Verilog file '" + file_path.string() + "': file path is empty");
        }

        // get modules in hierarchical order (bottom-up)
        std::vector<Module*> ordered_modules;
        {
            const std::vector<Module*> modules = netlist->get_modules();
            std::unordered_set<Module*> modules_set(modules.begin(), modules.end());

            while (!modules_set.empty())
            {
                for (auto it = modules_set.begin(); it != modules_set.end();)
                {
                    std::vector<Module*> submodules = (*it)->get_submodules();
                    if (submodules.empty() || std::all_of(submodules.begin(), submodules.end(), [modules_set](Module* submod) { return modules_set.find(submod) == modules_set.end(); }))
                    {
                        ordered_modules.push_back(*it);
                        it = modules_set.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
            }

            assert(ordered_modules.back()->is_top_module() == true);
        }

        std::unordered_map<const Module*, std::string> module_aliases;
        std::unordered_map<std::string, u32> module_identifier_occurrences;
        for (Module* mod : ordered_modules)
        {
            if (auto res = write_module_declaration(res_stream, mod, module_aliases, module_identifier_occurrences); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not write netlist to Verilog file '" + file_path.string() + "': failed to write module declaration");
            }
            res_stream << std::endl;
        }

        // write to file
        std::ofstream file;
        file.open(file_path.string(), std::ofstream::out);
        if (!file.is_open())
        {
            return ERR("could not write netlist to Verilog file '" + file_path.string() + "': failed to open file");
        }
        file << "`timescale 1 ps/1 ps" << std::endl;
        file << res_stream.str();
        file.close();

        return OK({});
    }

    Result<std::monostate> VerilogWriter::write_module_declaration(std::stringstream& res_stream,
                                                                   const Module* module,
                                                                   std::unordered_map<const Module*, std::string>& module_type_aliases,
                                                                   std::unordered_map<std::string, u32>& module_type_occurrences) const
    {
        // deal with empty modules
        if (module->get_gates(nullptr, true).empty() && !module->is_top_module())
        {
            return OK({});
        }

        // deal with unspecified module type
        std::string module_type = module->get_type();
        if (module_type.empty())
        {
            module_type = module->get_name();
            if (!module->is_top_module())
            {
                module_type += "_type";
            }
        }
        module_type_aliases[module] = get_unique_alias(module_type_occurrences, module_type);

        if (auto res = write_attributes(res_stream, module, ""); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not write declaration of module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write attributes");
        }
        if (const std::string& design_name = module->get_netlist()->get_design_name(); module_type_aliases.at(module) == "top_module" && !design_name.empty())
        {
            res_stream << "module " << escape(design_name);
        }
        else
        {
            res_stream << "module " << escape(module_type_aliases.at(module));
        }

        std::unordered_map<const DataContainer*, std::string> aliases;
        std::unordered_map<std::string, u32> identifier_occurrences;

        bool first_port = true;
        std::stringstream tmp_stream;

        res_stream << "(";
        for (const auto* pin : module->get_pins())
        {
            Net* net = pin->get_net();
            if (first_port)
            {
                first_port = false;
            }
            else
            {
                res_stream << ",";
            }

            aliases[net] = escape(get_unique_alias(identifier_occurrences, pin->get_name()));

            res_stream << aliases.at(net);
            if (auto res = write_attributes(tmp_stream, net, "    "); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not write declaration of module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write attributes of port net '" + net->get_name() + "'");
            }
            tmp_stream << "    " << enum_to_string(pin->get_direction()) << " " << aliases.at(net) << ";" << std::endl;
        }

        res_stream << ");" << std::endl;
        res_stream << tmp_stream.str();

        // module parameters: the generics of the typed store
        for (const auto& [key, entry] : module->get_parameters(Parameter::Source::Generic))
        {
            const auto& [declaration, value] = entry;
            res_stream << "    parameter " << escape(key) << " = ";
            if (auto res = write_parameter_value(res_stream, declaration, value); res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "could not write declaration of module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write parameter value");
            }
            res_stream << ";" << std::endl;
        }

        std::unordered_set<Net*> port_nets       = module->get_input_nets();
        std::unordered_set<Net*> output_nets_tmp = module->get_output_nets();
        port_nets.reserve(output_nets_tmp.size());
        port_nets.insert(output_nets_tmp.begin(), output_nets_tmp.end());

        for (Net* net : module->get_nets())
        {
            if (port_nets.find(net) != port_nets.end())
            {
                continue;
            }

            if (aliases.find(net) == aliases.end())
            {
                auto net_alias = escape(get_unique_alias(identifier_occurrences, net->get_name()));
                aliases[net]   = net_alias;

                if (auto res = write_attributes(res_stream, net, "    "); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not write declaration of module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write attributes of net '" + net->get_name() + "'");
                }
                res_stream << "    wire " << net_alias;

                if (net->is_vcc_net() && net->get_num_of_sources() == 0)
                {
                    res_stream << " = 1'b1";
                }
                else if (net->is_gnd_net() && net->get_num_of_sources() == 0)
                {
                    res_stream << " = 1'b0";
                }

                res_stream << ";" << std::endl;
            }
        }

        // write gate instances
        for (const Gate* gate : module->get_gates())
        {
            res_stream << std::endl;
            if (auto res = write_gate_instance(res_stream, gate, aliases, identifier_occurrences); res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "could not write declaration of module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write gate '" + gate->get_name()
                                      + "' with ID " + std::to_string(gate->get_id()));
            }
        }

        // write module instances
        for (const Module* sub_module : module->get_submodules())
        {
            res_stream << std::endl;
            if (auto res = write_module_instance(res_stream, sub_module, aliases, identifier_occurrences, module_type_aliases); res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "could not write declaration of module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write sub-module '"
                                      + sub_module->get_name() + "' with ID " + std::to_string(sub_module->get_id()));
            }
        }

        res_stream << "endmodule" << std::endl;

        return OK({});
    }

    Result<std::monostate> VerilogWriter::write_gate_instance(std::stringstream& res_stream,
                                                              const Gate* gate,
                                                              std::unordered_map<const DataContainer*, std::string>& aliases,
                                                              std::unordered_map<std::string, u32>& identifier_occurrences) const
    {
        const GateType* gate_type = gate->get_type();

        if (auto res = write_attributes(res_stream, gate, "    "); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not write gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to write attributes");
        }
        res_stream << "    " << escape(gate_type->get_name());
        if (auto res = write_parameter_assignments(res_stream, gate); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not write gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to write parameter assignments");
        }
        aliases[gate] = escape(get_unique_alias(identifier_occurrences, gate->get_name()));
        res_stream << " " << aliases.at(gate);

        // collect all endpoints (i.e., pins that are actually in use)
        std::unordered_map<const GatePin*, const Net*> connections;
        for (const Endpoint* ep : gate->get_fan_in_endpoints())
        {
            connections[ep->get_pin()] = ep->get_net();
        }

        for (const Endpoint* ep : gate->get_fan_out_endpoints())
        {
            connections[ep->get_pin()] = ep->get_net();
        }

        // extract pin assignments (in order, respecting pin groups)
        std::vector<std::pair<std::string, std::vector<const Net*>>> pin_assignments;
        for (const PinGroup<GatePin>* pin_group : gate_type->get_pin_groups())
        {
            std::vector<const Net*> nets;
            for (const GatePin* pin : pin_group->get_pins())
            {
                if (const auto ep_it = connections.find(pin); ep_it != connections.end())
                {
                    nets.push_back(ep_it->second);
                }
                else
                {
                    nets.push_back(nullptr);
                }
            }

            // only append if at least one pin of the group is connected
            if (std::any_of(nets.begin(), nets.end(), [](const Net* net) { return net != nullptr; }))
            {
                pin_assignments.push_back(std::make_pair(pin_group->get_name(), nets));
            }
        }

        if (auto res = write_pin_assignments(res_stream, pin_assignments, aliases); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not write gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to write pin assignments");
        }

        res_stream << ";" << std::endl;

        return OK({});
    }

    Result<std::monostate> VerilogWriter::write_module_instance(std::stringstream& res_stream,
                                                                const Module* module,
                                                                std::unordered_map<const DataContainer*, std::string>& aliases,
                                                                std::unordered_map<std::string, u32>& identifier_occurrences,
                                                                std::unordered_map<const Module*, std::string>& module_type_aliases) const
    {
        if (auto res = write_attributes(res_stream, module, "    "); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not write sub-module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write attributes");
        }
        res_stream << "    " << escape(module_type_aliases.at(module));
        if (auto res = write_parameter_assignments(res_stream, module); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not write sub-module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write parameter assignments");
        }
        aliases[module] = escape(get_unique_alias(identifier_occurrences, module->get_name()));
        res_stream << " " << aliases.at(module);

        // extract port assignments
        std::vector<std::pair<std::string, std::vector<const Net*>>> port_assignments;

        for (const ModulePin* pin : module->get_pins())
        {
            port_assignments.push_back(std::make_pair(pin->get_name(), std::vector<const Net*>({pin->get_net()})));
        }

        if (auto res = write_pin_assignments(res_stream, port_assignments, aliases); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not write sub-module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()) + ": failed to write pin assignments");
        }

        res_stream << ";" << std::endl;

        return OK({});
    }

    Result<std::monostate> VerilogWriter::write_parameter_assignments(std::stringstream& res_stream, const DataContainer* container) const
    {
        bool first_parameter = true;
        for (const auto& [key, entry] : container->get_parameters(Parameter::Source::Generic))
        {
            const auto& [declaration, value] = entry;
            if (first_parameter)
            {
                res_stream << " #(" << std::endl;
                first_parameter = false;
            }
            else
            {
                res_stream << "," << std::endl;
            }

            res_stream << "        ." << escape(key) << "(";

            if (auto res = write_parameter_value(res_stream, declaration, value); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not write parameter assignments: failed to write parameter value '" + value + "' of '" + key + "'");
            }

            res_stream << ")";
        }

        if (!first_parameter)
        {
            res_stream << std::endl << "    )";
        }

        return OK({});
    }

    Result<std::monostate> VerilogWriter::write_attributes(std::stringstream& res_stream, const DataContainer* container, const std::string& indent) const
    {
        const auto attributes = container->get_parameters(Parameter::Source::Attribute);
        if (attributes.empty())
        {
            return OK({});
        }
        res_stream << indent << "(* ";
        bool first = true;
        for (const auto& [key, entry] : attributes)
        {
            const auto& [declaration, value] = entry;
            if (!first)
            {
                res_stream << ", ";
            }
            first = false;
            res_stream << escape(key) << " = ";
            if (auto res = write_parameter_value(res_stream, declaration, value); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not write attributes: failed to write attribute value '" + value + "' of '" + key + "'");
            }
        }
        res_stream << " *)" << std::endl;
        return OK({});
    }

    Result<std::monostate> VerilogWriter::write_pin_assignments(std::stringstream& res_stream,
                                                                const std::vector<std::pair<std::string, std::vector<const Net*>>>& pin_assignments,
                                                                std::unordered_map<const DataContainer*, std::string>& aliases) const
    {
        static u32 unused_signal_counter = 0;

        res_stream << " (" << std::endl;
        bool first_pin = true;
        for (const auto& [pin, nets] : pin_assignments)
        {
            if (first_pin)
            {
                first_pin = false;
            }
            else
            {
                res_stream << "," << std::endl;
            }

            res_stream << "        ." << escape(pin) << "(";
            if (nets.size() > 1)
            {
                res_stream << "{" << std::endl << "            ";
            }

            bool first_net = true;
            for (auto it = nets.begin(); it != nets.end(); it++)
            {
                const Net* net = *it;

                if (!first_net)
                {
                    res_stream << "," << std::endl << "            ";
                }
                first_net = false;

                if (net != nullptr)
                {
                    if (const auto alias_it = aliases.find(net); alias_it != aliases.end())
                    {
                        res_stream << alias_it->second;
                    }
                    else
                    {
                        return ERR("could not write pin assignments: no alias for net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + " found");
                    }
                }
                else
                {
                    // unconnected pin of a group with at least one connection
                    res_stream << "HAL_UNUSED_SIGNAL_" + std::to_string(unused_signal_counter++);
                }
            }

            if (nets.size() > 1)
            {
                res_stream << std::endl << "        }";
            }

            res_stream << ")";
        }

        res_stream << std::endl << "    )";

        return OK({});
    }

    Result<std::monostate> VerilogWriter::write_parameter_value(std::stringstream& res_stream, const Parameter& declaration, const std::string& value) const
    {
        switch (declaration.get_type())
        {
            case Parameter::Type::String:
            case Parameter::Type::Time:
            case Parameter::Type::Enum:
                // Verilog has no time or enumeration literal in a parameter; the text survives as a string
                res_stream << "\"" << value << "\"";
                return OK({});
            case Parameter::Type::Integer:
            case Parameter::Type::Float:
                res_stream << value;
                return OK({});
            case Parameter::Type::Boolean:
                res_stream << "1'b" << (value == "true" ? "1" : "0");
                return OK({});
            case Parameter::Type::BitVector: {
                const std::string digits = (value.size() > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) ? value.substr(2) : value;
                res_stream << declaration.get_size() << "'h" << digits;
                return OK({});
            }
            case Parameter::Type::LogicVector: {
                const std::string bits = (value.size() > 2 && value[0] == '0' && (value[1] == 'b' || value[1] == 'B')) ? value.substr(2) : value;
                res_stream << declaration.get_size() << "'b" << bits;
                return OK({});
            }
        }
        return ERR("could not write parameter value '" + value + "': unknown type");
    }

    std::string VerilogWriter::get_unique_alias(std::unordered_map<std::string, u32>& name_occurrences, const std::string& name) const
    {
        name_occurrences[name]++;

        // if the name only appears once, we don't have to suffix it
        if (name_occurrences[name] < 2)
        {
            return name;
        }

        // otherwise, add a unique string to the name
        return name + "__[" + std::to_string(name_occurrences[name]) + "]__";
    }

    std::string VerilogWriter::escape(const std::string& s) const
    {
        if (s.empty())
        {
            return "";
        }

        const char first = s.at(0);
        if (!(first >= 'a' && first <= 'z') && !(first >= 'A' && first <= 'Z') && first != '_')
        {
            return "\\" + s + " ";
        }
        else if (std::any_of(s.begin(), s.end(), [](const char c) { return (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9') && c != '_' && c != '$'); }))
        {
            return "\\" + s + " ";
        }

        return s;
    }
}    // namespace hal
