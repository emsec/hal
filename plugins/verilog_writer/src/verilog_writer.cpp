#include "verilog_writer/verilog_writer.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <fstream>

namespace hal
{
    bool VerilogWriter::write(Netlist* netlist, const std::filesystem::path& file_path)
    {
        std::stringstream res_stream;

        // TODO make sure that all identifiers are unique (signals within a module, instances within a module, module identifiers)
        write_module_declaration(res_stream, netlist->get_top_module());

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

    bool VerilogWriter::write_module_declaration(std::stringstream& res_stream, const Module* module) const
    {
        // TODO implement
        return true;
    }

    bool VerilogWriter::write_gate_instance(std::stringstream& res_stream, const Gate* gate) const
    {
        const GateType* gate_type = gate->get_type();

        res_stream << "\t" << gate_type->get_name();
        if (!write_generic_assignments(res_stream, gate))
        {
            return false;
        }

        res_stream << " \\" << gate->get_name() << " (\n";

        // collect all endpoints (i.e., pins that are actually in use)
        std::unordered_map<std::string, Net*> endpoints;
        for (const Endpoint* ep : gate->get_fan_in_endpoints())
        {
            endpoints[ep->get_pin()] = ep->get_net();
        }

        for (const Endpoint* ep : gate->get_fan_out_endpoints())
        {
            endpoints[ep->get_pin()] = ep->get_net();
        }

        // extract pin assignments (in order, respecting pin groups)
        std::vector<std::pair<std::string, std::vector<Net*>>> pin_assignments;
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
                std::vector<Net*> nets;
                for (const auto& [index, pin] : gate_type->get_pins_of_group(pin_group))
                {
                    visited_pins.insert(pin);

                    if (const auto ep_it = endpoints.find(pin); ep_it != endpoints.end())
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
                if (const auto ep_it = endpoints.find(pin); ep_it != endpoints.end())
                {
                    pin_assignments.push_back(std::make_pair(pin, std::vector<Net*>({ep_it->second})));
                }
            }
        }

        // write pin assignments
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

            res_stream << "\t\t.\\" << pin << " (";
            if (nets.size() > 1)
            {
                res_stream << "{";
                for (auto net_it = nets.begin(); net_it != nets.end();)
                {
                    const Net* net = *net_it;
                    if (net != nullptr)
                    {
                        res_stream << "\\" << (*net_it)->get_name() << " ";
                    }
                    else
                    {
                        // unconnected pin of a group with at least one connection
                        res_stream << "1'bz";
                    }

                    if (++net_it != nets.end())
                    {
                        res_stream << ",";
                    }
                }
                res_stream << "}";
            }
            else
            {
                res_stream << "\\" << nets.front()->get_name() << " ";
            }

            res_stream << ")";
        }

        res_stream << std::endl << "\t);" << std::endl;
    }

    bool VerilogWriter::write_module_instance(std::stringstream& res_stream, const Module* module) const
    {
        // TODO implement
        return true;
    }

    bool VerilogWriter::write_generic_assignments(std::stringstream& res_stream, const DataContainer* container) const
    {
        const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& data = container->get_data_map();

        static const std::set<std::string> valid_types = {"string", "integer", "floating_point", "bit_value", "bit_vector"};

        bool first_generic = true;

        for (const auto& [first, second] : data)
        {
            const auto& [category, key] = first;
            const auto& [type, value]   = second;

            if (category != "generic" || valid_types.find(type) == valid_types.end())
            {
                continue;
            }

            if (first_generic)
            {
                res_stream << " #(" << std::endl;
                first_generic = false;
            }
            else
            {
                res_stream << "," << std::endl;
            }

            if (type == "string")
            {
                res_stream << "\t\t." << key << "(\"" << value << "\")";
            }
            else if (type == "integer" || type == "floating_point")
            {
                res_stream << "\t\t." << key << "(" << value << ")";
            }
            else if (type == "floating_point")
            {
                res_stream << "\t\t." << key << "(\"" << value << "\")";
            }
            else if (type == "bit_value")
            {
                res_stream << "\t\t." << key << "(1'b" << value << ")";
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
                res_stream << "\t\t." << key << "(" << len << "'h" << value << ")";
            }
        }

        res_stream << std::endl << "\t);" << std::endl;

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
}    // namespace hal
