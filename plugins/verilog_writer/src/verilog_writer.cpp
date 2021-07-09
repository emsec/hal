#include "verilog_writer/verilog_writer.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <fstream>

namespace hal
{
    const std::set<std::string> VerilogWriter::valid_types = {"string", "integer", "floating_point", "bit_value", "bit_vector"};

    bool VerilogWriter::write(Netlist* netlist, const std::filesystem::path& file_path)
    {
        std::stringstream res_stream;

        // get modules in hierarchical order (bottom-up)
        std::vector<const Module*> ordered_modules;
        {
            const std::vector<Module*> modules = netlist->get_modules();
            std::unordered_set<const Module*> modules_set(modules.begin(), modules.end());

            while (!modules_set.empty())
            {
                for (auto it = modules_set.begin(); it != modules_set.end();)
                {
                    std::vector<Module*> submodules = (*it)->get_submodules();
                    if (submodules.empty() || std::all_of(submodules.begin(), submodules.end(), [modules_set](const Module* submod) { return modules_set.find(submod) == modules_set.end(); }))
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

        // TODO take care of 1 and 0 nets (probably rework their handling within the core to supply a "is_gnd_net" and "is_vcc_net" function)

        std::unordered_map<const Module*, std::string> module_aliases;
        std::unordered_map<std::string, u32> module_identifier_occurrences;
        for (const Module* mod : ordered_modules)
        {
            if (!write_module_declaration(res_stream, mod, module_aliases, module_identifier_occurrences))
            {
                return false;
            }
            res_stream << std::endl;
        }

        // write to file
        std::ofstream file;
        file.open(file_path.string(), std::ofstream::out);
        if (!file.is_open())
        {
            log_error("verilog_writer", "unable to open '{}'.", file_path.string());
            return false;
        }
        file << res_stream.str();
        file.close();

        return true;
    }

    bool VerilogWriter::write_module_declaration(std::stringstream& res_stream,
                                                 const Module* module,
                                                 std::unordered_map<const Module*, std::string>& module_type_aliases,
                                                 std::unordered_map<std::string, u32>& module_type_occurrences) const
    {
        // deal with empty modules
        if (module->get_gates(nullptr, true).empty() && !module->is_top_module())
        {
            return true;
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
        res_stream << "module " << escape(module_type_aliases.at(module));

        std::unordered_map<const DataContainer*, std::string> aliases;
        std::unordered_map<std::string, u32> identifier_occurrences;

        bool first_port = true;
        std::stringstream tmp_stream;

        res_stream << "(";
        for (const auto& [net, port] : module->get_input_port_names())
        {
            if (first_port)
            {
                first_port = false;
            }
            else
            {
                res_stream << ",";
            }

            aliases[net] = escape(get_unique_alias(identifier_occurrences, port));

            res_stream << aliases.at(net);

            tmp_stream << "    input " << aliases.at(net) << ";" << std::endl;
        }

        for (const auto& [net, port] : module->get_output_port_names())
        {
            if (first_port)
            {
                first_port = false;
            }
            else
            {
                res_stream << ",";
            }

            aliases[net] = escape(get_unique_alias(identifier_occurrences, port));

            res_stream << aliases.at(net);

            tmp_stream << "    output " << aliases.at(net) << ";" << std::endl;
        }

        res_stream << ");" << std::endl;
        res_stream << tmp_stream.str();

        {
            // module parameters
            const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& data = module->get_data_map();

            for (const auto& [first, second] : data)
            {
                const auto& [category, key] = first;
                const auto& [type, value]   = second;

                if (category != "generic" || valid_types.find(type) == valid_types.end())
                {
                    continue;
                }

                res_stream << "    parameter " << escape(key) << " = ";
                if (!write_parameter_value(res_stream, type, value))
                {
                    return false;
                }
                res_stream << ";" << std::endl;
            }
        }

        for (Net* net : module->get_nets())
        {
            if (aliases.find(net) == aliases.end())
            {
                aliases[net] = escape(get_unique_alias(identifier_occurrences, net->get_name()));
                res_stream << "    wire " << aliases.at(net) << ";" << std::endl;
            }
        }

        // write gate instances
        for (const Gate* gate : module->get_gates())
        {
            res_stream << std::endl;
            if (!write_gate_instance(res_stream, gate, aliases, identifier_occurrences))
            {
                return false;
            }
        }

        // write module instances
        for (const Module* sub_module : module->get_submodules())
        {
            res_stream << std::endl;
            if (!write_module_instance(res_stream, sub_module, aliases, identifier_occurrences, module_type_aliases))
            {
                return false;
            }
        }

        res_stream << "endmodule" << std::endl;

        return true;
    }

    bool VerilogWriter::write_gate_instance(std::stringstream& res_stream,
                                            const Gate* gate,
                                            std::unordered_map<const DataContainer*, std::string>& aliases,
                                            std::unordered_map<std::string, u32>& identifier_occurrences) const
    {
        const GateType* gate_type = gate->get_type();

        res_stream << "    " << escape(gate_type->get_name());
        if (!write_parameter_assignments(res_stream, gate))
        {
            return false;
        }
        aliases[gate] = escape(get_unique_alias(identifier_occurrences, gate->get_name()));
        res_stream << " " << aliases.at(gate);

        // collect all endpoints (i.e., pins that are actually in use)
        std::unordered_map<std::string, Net*> connections;
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
        std::unordered_set<std::string> visited_pins;
        for (const std::string& pin : gate_type->get_pins())
        {
            // check if pin was contained in a group that has already been dealt with
            if (visited_pins.find(pin) != visited_pins.end())
            {
                continue;
            }

            if (std::string pin_group = gate_type->get_pin_group(pin); !pin_group.empty())
            {
                // if pin is of pin group, deal with the entire group at once (i.e., collect all connected nets)
                std::vector<const Net*> nets;
                for (const auto& [index, group_pin] : gate_type->get_pins_of_group(pin_group))
                {
                    visited_pins.insert(group_pin);

                    if (const auto ep_it = connections.find(group_pin); ep_it != connections.end())
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
                    pin_assignments.push_back(std::make_pair(pin_group, nets));
                }
            }
            else
            {
                // append all connected pins
                if (const auto ep_it = connections.find(pin); ep_it != connections.end())
                {
                    pin_assignments.push_back(std::make_pair(pin, std::vector<const Net*>({ep_it->second})));
                }
            }
        }

        if (!write_pin_assignments(res_stream, pin_assignments, aliases))
        {
            return false;
        }

        res_stream << ";" << std::endl;

        return true;
    }

    bool VerilogWriter::write_module_instance(std::stringstream& res_stream,
                                              const Module* module,
                                              std::unordered_map<const DataContainer*, std::string>& aliases,
                                              std::unordered_map<std::string, u32>& identifier_occurrences,
                                              std::unordered_map<const Module*, std::string>& module_type_aliases) const
    {
        res_stream << "    " << escape(module_type_aliases.at(module));
        if (!write_parameter_assignments(res_stream, module))
        {
            return false;
        }
        aliases[module] = escape(get_unique_alias(identifier_occurrences, module->get_name()));
        res_stream << " " << aliases.at(module);

        // extract port assignments
        std::vector<std::pair<std::string, std::vector<const Net*>>> port_assignments;

        for (const auto& [net, port] : module->get_input_port_names())
        {
            port_assignments.push_back(std::make_pair(port, std::vector<const Net*>({net})));
        }

        for (const auto& [net, port] : module->get_output_port_names())
        {
            port_assignments.push_back(std::make_pair(port, std::vector<const Net*>({net})));
        }

        if (!write_pin_assignments(res_stream, port_assignments, aliases))
        {
            return false;
        }

        res_stream << ";" << std::endl;

        return true;
    }

    bool VerilogWriter::write_parameter_assignments(std::stringstream& res_stream, const DataContainer* container) const
    {
        const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& data = container->get_data_map();

        bool first_parameter = true;
        for (const auto& [first, second] : data)
        {
            const auto& [category, key] = first;
            const auto& [type, value]   = second;

            if (category != "generic" || valid_types.find(type) == valid_types.end())
            {
                continue;
            }

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

            if (!write_parameter_value(res_stream, type, value))
            {
                return false;
            }

            res_stream << ")";
        }

        if (!first_parameter)
        {
            res_stream << std::endl << "    )";
        }

        return true;
    }

    bool VerilogWriter::write_pin_assignments(std::stringstream& res_stream,
                                              const std::vector<std::pair<std::string, std::vector<const Net*>>>& pin_assignments,
                                              std::unordered_map<const DataContainer*, std::string>& aliases) const
    {
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
                res_stream << "{";
            }

            bool first_net = true;
            for (auto it = nets.rbegin(); it != nets.rend(); it++)
            {
                const Net* net = *it;

                if (net != nullptr)
                {
                    if (first_net)
                    {
                        first_net = false;
                    }
                    else
                    {
                        res_stream << "," << std::endl;
                    }

                    if (const auto alias_it = aliases.find(net); alias_it != aliases.end())
                    {
                        res_stream << alias_it->second;
                    }
                    else
                    {
                        log_error("verilog_writer", "no alias for net '{}' with ID {} found.", net->get_name(), net->get_id());
                        return false;
                    }
                }
                else
                {
                    // unconnected pin of a group with at least one connection
                    res_stream << "1'bz";
                }
            }

            if (nets.size() > 1)
            {
                res_stream << "}";
            }

            res_stream << ")";
        }

        res_stream << std::endl << "    )";

        return true;
    }

    bool VerilogWriter::write_parameter_value(std::stringstream& res_stream, const std::string& type, const std::string& value) const
    {
        if (type == "string")
        {
            res_stream << "\"" << value << "\"";
        }
        else if (type == "integer" || type == "floating_point")
        {
            res_stream << value;
        }
        else if (type == "bit_value")
        {
            res_stream << "1'b" << value;
        }
        else if (type == "bit_vector")
        {
            u32 len = value.size() * 4;
            if (value.at(0) == '0' || value.at(0) == '1')
            {
                len -= 3;
            }
            else if (value.at(0) == '2' || value.at(0) == '3')
            {
                len -= 2;
            }
            else if (value.at(0) >= '4' && value.at(0) <= '7')
            {
                len -= 1;
            }
            res_stream << len << "'h" << value;
        }
        else
        {
            return false;
        }

        return true;
    }

    std::string VerilogWriter::get_unique_alias(std::unordered_map<std::string, u32>& name_occurrences, const std::string& name) const
    {
        // if the name only appears once, we don't have to suffix it
        if (name_occurrences[name] < 2)
        {
            return name;
        }

        name_occurrences[name]++;

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
