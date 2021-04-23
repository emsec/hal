#include "verilog_parser/verilog_parser.h"

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <fstream>
#include <iomanip>
#include <queue>

namespace hal
{
    bool VerilogParser::parse(const std::filesystem::path& file_path)
    {
        m_path = file_path;

        {
            std::ifstream ifs;
            ifs.open(m_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                log_error("verilog_parser", "unable to open '{}'.", m_path.string());
                return false;
            }
            m_fs << ifs.rdbuf();
            ifs.close();
        }

        // tokenize file
        if (!tokenize())
        {
            return false;
        }

        // parse tokens into intermediate format
        try
        {
            if (!parse_tokens())
            {
                return false;
            }
        }
        catch (TokenStream<std::string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                log_error("verilog_parser", "{} near line {}.", e.message, e.line_number);
            }
            else
            {
                log_error("verilog_parser", "{}.", e.message);
            }
            return false;
        }

        // expand module port identifiers, signals, and assignments
        for (auto& [module_name, module] : m_modules)
        {
            // expand port iodentifiers
            for (std::string& port_identifier : module.m_port_identifiers)
            {
                if (const auto port_expr_it = module.m_port_ident_to_expr.find(port_identifier); port_expr_it != module.m_port_ident_to_expr.end())
                {
                    if (const auto ranges_it = module.m_port_ranges.find(port_expr_it->second); ranges_it != module.m_port_ranges.end())
                    {
                        module.m_expanded_port_identifiers[port_identifier] = expand_ranges(port_identifier, ranges_it->second);
                        std::vector<std::string> expanded_port_expressions  = expand_ranges(port_expr_it->second, ranges_it->second);

                        for (u32 i = 0; i < expanded_port_expressions.size(); i++)
                        {
                            module.m_expanded_port_ident_to_expr[module.m_expanded_port_identifiers[port_identifier][i]] = expanded_port_expressions[i];
                        }
                    }
                    else
                    {
                        module.m_expanded_port_identifiers[port_identifier]   = std::vector<std::string>({port_identifier});
                        module.m_expanded_port_ident_to_expr[port_identifier] = port_expr_it->second;
                    }
                }
                else
                {
                    if (const auto ranges_it = module.m_port_ranges.find(port_identifier); ranges_it != module.m_port_ranges.end())
                    {
                        module.m_expanded_port_identifiers[port_identifier] = expand_ranges(port_identifier, ranges_it->second);
                    }
                    else
                    {
                        module.m_expanded_port_identifiers[port_identifier] = std::vector<std::string>({port_identifier});
                    }
                }
            }

            // expand signals
            for (std::string& signal_identifier : module.m_signals)
            {
                if (const auto ranges_it = module.m_signal_ranges.find(signal_identifier); ranges_it != module.m_signal_ranges.end())
                {
                    module.m_expanded_signals[signal_identifier] = expand_ranges(signal_identifier, ranges_it->second);
                }
                else
                {
                    module.m_expanded_signals[signal_identifier] = std::vector<std::string>({signal_identifier});
                }
            }

            // expand assignments
            for (auto& [left_stream, right_stream] : module.m_assignments)
            {
                std::vector<std::string> left_signals  = get_expanded_assignment_signal(module, left_stream, false);
                std::vector<std::string> right_signals = get_expanded_assignment_signal(module, left_stream, false);

                u32 left_size  = left_signals.size();
                u32 right_size = right_signals.size();
                if (left_size <= right_size)
                {
                    // cut off redundant bits
                    for (u32 i = 0; i < left_size; i++)
                    {
                        module.m_expanded_assignments.push_back(std::make_pair(left_signals.at(i), right_signals.at(i)));
                    }
                }
                else
                {
                    for (u32 i = 0; i < right_size; i++)
                    {
                        module.m_expanded_assignments.push_back(std::make_pair(left_signals.at(i), right_signals.at(i)));
                    }

                    // implicit "0"
                    for (u32 i = 0; i < left_size - right_size; i++)
                    {
                        module.m_expanded_assignments.push_back(std::make_pair(left_signals.at(i), "0"));
                    }
                }
            }
        }

        return true;
    }

    std::unique_ptr<Netlist> VerilogParser::instantiate(const GateLibrary* gate_library)
    {
        // create empty netlist
        std::unique_ptr<Netlist> result = netlist_factory::create_netlist(gate_library);
        m_netlist                       = result.get();

        if (m_netlist == nullptr)
        {
            // error printed in subfunction
            return nullptr;
        }

        // any entities in netlist?
        if (m_modules.empty())
        {
            log_error("verilog_parser", "file did not contain any modules.");
            return nullptr;
        }

        // buffer gate types
        m_gate_types     = gate_library->get_gate_types();
        m_gnd_gate_types = gate_library->get_gnd_gate_types();
        m_vcc_gate_types = gate_library->get_vcc_gate_types();

        // create const 0 and const 1 net, will be removed if unused
        m_zero_net = m_netlist->create_net("'0'");
        if (m_zero_net == nullptr)
        {
            return nullptr;
        }
        m_net_by_name[m_zero_net->get_name()] = m_zero_net;

        m_one_net = m_netlist->create_net("'1'");
        if (m_one_net == nullptr)
        {
            return nullptr;
        }
        m_net_by_name[m_one_net->get_name()] = m_one_net;

        // construct the netlist with the last module being considered the top module
        VerilogModule& top_module = m_modules.at(m_last_module);
        if (!construct_netlist(top_module))
        {
            return nullptr;
        }

        // add global GND gate if required by any instance
        // TODO add check whether GND gate already exists
        if (!m_zero_net->get_destinations().empty())
        {
            const auto gnd_type   = m_gnd_gate_types.begin()->second;
            const auto output_pin = gnd_type->get_output_pins().at(0);
            const auto gnd        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");

            if (!m_netlist->mark_gnd_gate(gnd))
            {
                return nullptr;
            }

            if (auto gnd_net = m_net_by_name.find("'0'")->second; !gnd_net->add_source(gnd, output_pin))
            {
                return nullptr;
            }
        }
        else
        {
            m_netlist->delete_net(m_zero_net);
        }

        // add global VCC gate if required by any instance
        // TODO add check whether VCC gate already exists
        if (!m_one_net->get_destinations().empty())
        {
            const auto vcc_type   = m_vcc_gate_types.begin()->second;
            const auto output_pin = vcc_type->get_output_pins().at(0);
            const auto vcc        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");

            if (!m_netlist->mark_vcc_gate(vcc))
            {
                return nullptr;
            }

            if (auto vcc_net = m_net_by_name.find("'1'")->second; !vcc_net->add_source(vcc, output_pin))
            {
                return nullptr;
            }
        }
        else
        {
            m_netlist->delete_net(m_one_net);
        }

        // delete unused nets
        for (auto net : m_netlist->get_nets())
        {
            const bool no_source      = net->get_num_of_sources() == 0 && !net->is_global_input_net();
            const bool no_destination = net->get_num_of_destinations() == 0 && !net->is_global_output_net();
            if (no_source && no_destination)
            {
                m_netlist->delete_net(net);
            }
        }

        // TODO load gate coordinates

        m_gate_types.clear();
        m_gnd_gate_types.clear();
        m_vcc_gate_types.clear();
        m_instance_name_occurrences.clear();
        m_signal_name_occurrences.clear();
        m_module_ports.clear();

        return result;
    }

    // ###########################################################################
    // ###########          Parse HDL into Intermediate Format          ##########
    // ###########################################################################

    bool VerilogParser::tokenize()
    {
        const std::string delimiters = "`,()[]{}\\#*: ;=.";
        std::string current_token;
        u32 line_number = 0;

        std::string line;
        bool in_string          = false;
        bool escaped            = false;
        bool multi_line_comment = false;

        std::vector<Token<std::string>> parsed_tokens;
        while (std::getline(m_fs, line))
        {
            line_number++;
            this->remove_comments(line, multi_line_comment);

            for (char c : line)
            {
                if (in_string == false && c == '\\')
                {
                    escaped = true;
                    continue;
                }
                else if (escaped && std::isspace(c))
                {
                    escaped = false;
                    continue;
                }
                else if (!escaped && c == '"')
                {
                    in_string = !in_string;
                }

                if ((!std::isspace(c) && delimiters.find(c) == std::string::npos) || escaped || in_string)
                {
                    current_token += c;
                }
                else
                {
                    if (!current_token.empty())
                    {
                        if (parsed_tokens.size() > 1 && utils::is_digits(parsed_tokens.at(parsed_tokens.size() - 2).string) && parsed_tokens.at(parsed_tokens.size() - 1) == "."
                            && utils::is_digits(current_token))
                        {
                            parsed_tokens.pop_back();
                            parsed_tokens.back() += "." + current_token;
                        }
                        else
                        {
                            parsed_tokens.emplace_back(line_number, current_token);
                        }
                        current_token.clear();
                    }

                    if (!parsed_tokens.empty())
                    {
                        if (c == '(' && parsed_tokens.back() == "#")
                        {
                            parsed_tokens.back() = "#(";
                            continue;
                        }
                        else if (c == '*' && parsed_tokens.back() == "(")
                        {
                            parsed_tokens.back() = "(*";
                            continue;
                        }
                        else if (c == ')' && parsed_tokens.back() == "*")
                        {
                            parsed_tokens.back() = "*)";
                            continue;
                        }
                    }

                    if (!std::isspace(c))
                    {
                        parsed_tokens.emplace_back(line_number, std::string(1, c));
                    }
                }
            }
            if (!current_token.empty())
            {
                parsed_tokens.emplace_back(line_number, current_token);
                current_token.clear();
            }
        }

        m_token_stream = TokenStream(parsed_tokens, {"(", "["}, {")", "]"});
        return true;
    }

    bool VerilogParser::parse_tokens()
    {
        std::map<std::string, std::string> attributes;

        while (m_token_stream.remaining() > 0)
        {
            if (m_token_stream.peek() == "(*")
            {
                if (!parse_attribute(attributes))
                {
                    return false;
                }
            }
            else if (m_token_stream.peek() == "`")
            {
                m_token_stream.consume_current_line();
                log_warning("verilog_parser", "cannot parse compiler directives.");
            }
            else
            {
                if (!parse_module(attributes))
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool VerilogParser::parse_module(std::map<std::string, std::string>& attributes)
    {
        std::set<std::string> port_names;
        std::map<std::string, std::string> internal_attributes;

        m_token_stream.consume("module", true);
        const u32 line_number         = m_token_stream.peek().number;
        const std::string module_name = m_token_stream.consume();

        // verify entity name
        if (m_modules.find(module_name) != m_modules.end())
        {
            log_error("verilog_parser", "a module with the name '{}' does already exist (see line {} and line {})", module_name, line_number, m_modules.at(module_name).m_line_number);
            return false;
        }

        VerilogModule module;
        module.m_line_number = line_number;
        module.m_name        = module_name;

        // parse parameter list
        if (m_token_stream.consume("#("))
        {
            m_token_stream.consume_until(")");
            m_token_stream.consume(")", true);
            log_warning("verilog_parser", "cannot parse parameter list provided for module '{}'.", module_name);
        }

        // parse port (declaration) list
        m_token_stream.consume("(", true);
        Token<std::string> next_token = m_token_stream.peek();
        if (next_token == "input" || next_token == "output" || next_token == "inout")
        {
            if (!parse_port_declaration_list(module))
            {
                return false;
            }
        }
        else
        {
            if (!parse_port_list(module))
            {
                return false;
            }
        }

        m_token_stream.consume(";", true);

        next_token = m_token_stream.peek();
        while (next_token != "endmodule")
        {
            if (next_token == "input" || next_token == "output" || next_token == "inout")
            {
                if (!parse_port_definition(module, internal_attributes))
                {
                    return false;
                }
            }
            else if (next_token == "wire")
            {
                if (!parse_signal_definition(module, internal_attributes))
                {
                    return false;
                }
            }
            else if (next_token == "assign")
            {
                if (!parse_assignment(module))
                {
                    return false;
                }
            }
            else if (next_token == "(*")
            {
                if (!parse_attribute(internal_attributes))
                {
                    return false;
                }
            }
            else
            {
                if (!parse_instance(module, internal_attributes))
                {
                    return false;
                }
            }

            next_token = m_token_stream.peek();
        }

        m_token_stream.consume("endmodule", true);

        // assign attributes to entity
        if (!attributes.empty())
        {
            for (const auto& [attribute_name, attribute_value] : attributes)
            {
                module.m_attributes.push_back(std::make_tuple(attribute_name, "unknown", attribute_value));
            }

            attributes.clear();
        }

        // add to collection of entities
        m_modules.emplace(module_name, module);
        m_last_module = module_name;

        return true;
    }

    bool VerilogParser::parse_port_list(VerilogModule& module)
    {
        TokenStream<std::string> ports_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (ports_stream.remaining() > 0)
        {
            Token<std::string> next_token = ports_stream.consume();
            std::string port_identifier;

            if (next_token == ".")
            {
                port_identifier = ports_stream.consume().string;
                module.m_port_identifiers.push_back(port_identifier);

                ports_stream.consume("(", true);

                module.m_port_ident_to_expr[port_identifier] = ports_stream.consume().string;

                ports_stream.consume(")", true);
            }
            else
            {
                port_identifier = next_token.string;
                module.m_port_identifiers.push_back(port_identifier);
                module.m_port_expressions.insert(port_identifier);
            }
            ports_stream.consume(",", ports_stream.remaining() > 0);
        }

        return true;
    }

    bool VerilogParser::parse_port_declaration_list(VerilogModule& module)
    {
        TokenStream<std::string> ports_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (ports_stream.remaining() > 0)
        {
            // direction
            const Token<std::string> direction_token = ports_stream.consume();
            PinDirection direction                   = enum_from_string<PinDirection>(direction_token.string, PinDirection::none);
            if (direction == PinDirection::none || direction == PinDirection::internal)
            {
                log_error("verilog_parser", "invalid direction '{}' for port declaration in line {}", direction_token.string, direction_token.number);
                return false;
            }

            // ranges
            std::vector<std::vector<u32>> ranges;
            while (ports_stream.consume("["))
            {
                const std::vector<u32> range = parse_range(ports_stream);
                ports_stream.consume("]", true);

                ranges.emplace_back(range);
            }

            // port expressions
            do
            {
                const Token<std::string> next_token = ports_stream.peek();
                if (next_token == "input" || next_token == "output" || next_token == "inout")
                {
                    break;
                }
                ports_stream.consume();

                std::string port_identifier = next_token.string;
                module.m_port_identifiers.push_back(port_identifier);
                module.m_port_expressions.insert(port_identifier);
                module.m_port_directions[port_identifier] = direction;
                if (!ranges.empty())
                {
                    module.m_port_ranges[port_identifier] = ranges;
                }
            } while (ports_stream.consume(",", ports_stream.remaining() > 0));
        }

        return true;
    }

    bool VerilogParser::parse_port_definition(VerilogModule& module, std::map<std::string, std::string>& attributes)
    {
        // port direction
        const Token<std::string> direction_token = m_token_stream.consume();
        PinDirection direction                   = enum_from_string<PinDirection>(direction_token.string, PinDirection::none);
        if (direction == PinDirection::none || direction == PinDirection::internal)
        {
            log_error("verilog_parser", "invalid direction '{}' for port declaration in line {}", direction_token.string, direction_token.number);
            return false;
        }

        // ranges
        std::vector<std::vector<u32>> ranges;
        while (m_token_stream.consume("["))
        {
            const std::vector<u32> range = parse_range(m_token_stream);
            m_token_stream.consume("]", true);

            ranges.emplace_back(range);
        }

        // port expressions
        do
        {
            Token<std::string> port_expression_token = m_token_stream.consume();
            std::string port_expression              = port_expression_token.string;

            if (module.m_port_expressions.find(port_expression) == module.m_port_expressions.end())
            {
                log_error("verilog_parser", "a port with name '{}' does not exist for module '{}' in line {}.", port_expression, module.m_name, port_expression_token.number);
                return false;
            }

            module.m_port_directions[port_expression] = direction;
            if (!ranges.empty())
            {
                module.m_port_ranges[port_expression] = ranges;
            }
            for (const auto& [attribute_name, attribute_value] : attributes)
            {
                module.m_port_attributes[port_expression].push_back(std::make_tuple(attribute_name, "unknown", attribute_value));
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(";", true);
        attributes.clear();

        return true;
    }

    bool VerilogParser::parse_signal_definition(VerilogModule& module, std::map<std::string, std::string>& attributes)
    {
        m_token_stream.consume("wire", true);

        // extract bounds
        std::vector<std::vector<u32>> ranges;
        while (m_token_stream.consume("["))
        {
            const std::vector<u32> range = parse_range(m_token_stream);
            m_token_stream.consume("]", true);

            ranges.emplace_back(range);
        }

        // extract names
        do
        {
            std::string signal_name = m_token_stream.consume().string;
            module.m_signals.push_back(signal_name);
            if (!ranges.empty())
            {
                module.m_signal_ranges[signal_name] = ranges;
            }
            for (const auto& [attribute_name, attribute_value] : attributes)
            {
                module.m_signal_attributes[signal_name].push_back(std::make_tuple(attribute_name, "unknown", attribute_value));
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(";", true);
        attributes.clear();

        return true;
    }

    bool VerilogParser::parse_assignment(VerilogModule& module)
    {
        m_token_stream.consume("assign", true);
        TokenStream<std::string> left_stream = m_token_stream.extract_until("=");
        m_token_stream.consume("=", true);
        TokenStream<std::string> right_stream = m_token_stream.extract_until(";");
        m_token_stream.consume(";", true);

        module.m_assignments.push_back(std::make_pair(left_stream, right_stream));

        return true;
    }

    bool VerilogParser::parse_attribute(std::map<std::string, std::string>& attributes)
    {
        m_token_stream.consume("(*", true);

        // extract attributes
        do
        {
            const std::string attribute_name = m_token_stream.consume().string;
            std::string attribute_value;

            // attribute value specified?
            if (m_token_stream.consume("="))
            {
                attribute_value = m_token_stream.consume();

                // remove "
                if (attribute_value[0] == '\"' && attribute_value.back() == '\"')
                {
                    attribute_value = attribute_value.substr(1, attribute_value.size() - 2);
                }
            }

            attributes.emplace(attribute_name, attribute_value);

        } while (m_token_stream.consume(",", false));

        m_token_stream.consume("*)", true);

        return true;
    }

    bool VerilogParser::parse_instance(VerilogModule& module, std::map<std::string, std::string>& attributes)
    {
        const std::string instance_type = m_token_stream.consume().string;

        // parse generics map
        std::vector<std::tuple<std::string, std::string, std::string>> generics;
        if (m_token_stream.consume("#("))
        {
            generics = parse_generic_assign();
        }

        // parse instance name
        const std::string instance_name = m_token_stream.consume().string;
        module.m_instances.push_back(instance_name);
        module.m_instance_types[instance_name] = instance_type;

        if (!generics.empty())
        {
            module.m_instance_generic_assignments[instance_name] = generics;
        }

        // parse port map
        if (!parse_port_assign(module, instance_name))
        {
            return false;
        }

        // assign attributes to instance
        for (const auto& [attribute_name, attribute_value] : attributes)
        {
            module.m_instance_attributes[instance_name].push_back(std::make_tuple(attribute_name, "unknown", attribute_value));
        }

        attributes.clear();

        return true;
    }

    bool VerilogParser::parse_port_assign(VerilogModule& module, const std::string& instance_name)
    {
        m_token_stream.consume("(", true);

        do
        {
            if (m_token_stream.consume(".", false))
            {
                Token<std::string> left_token = m_token_stream.consume();
                m_token_stream.consume("(", true);
                TokenStream<std::string> right_stream = m_token_stream.extract_until(")");
                m_token_stream.consume(")", true);

                if (right_stream.size() != 0)
                {
                    module.m_instance_port_assignments[instance_name].push_back(std::make_pair(left_token, right_stream));
                }
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(")", true);
        m_token_stream.consume(";", true);

        return true;
    }

    std::vector<std::tuple<std::string, std::string, std::string>> VerilogParser::parse_generic_assign()
    {
        std::vector<std::tuple<std::string, std::string, std::string>> generics;

        do
        {
            if (m_token_stream.consume(".", false))
            {
                const Token<std::string> lhs = m_token_stream.join_until("(", "");
                m_token_stream.consume("(", true);
                const Token<std::string> rhs = m_token_stream.join_until(")", "");
                m_token_stream.consume(")", true);

                bool skip = false;
                std::string value, data_type;
                if (utils::is_integer(rhs.string))
                {
                    value     = rhs;
                    data_type = "integer";
                }
                else if (utils::is_floating_point(rhs.string))
                {
                    value     = rhs;
                    data_type = "floating_point";
                }
                else if (rhs.string[0] == '\"' && rhs.string.back() == '\"')
                {
                    value     = rhs.string.substr(1, rhs.string.size() - 2);
                    data_type = "string";
                }
                else if (isdigit(rhs.string[0]) || rhs.string[0] == '\'')
                {
                    value = get_hex_from_literal(rhs);
                    if (value.empty())
                    {
                        skip = true;
                    }

                    if (value.size() == 1)
                    {
                        data_type = "bit_value";
                    }
                    else
                    {
                        data_type = "bit_vector";
                    }
                }
                else
                {
                    log_warning("verilog_parser", "cannot identify data type of generic map value '{}' in line {}", rhs.string, rhs.number);
                    skip = true;
                }

                if (!skip)
                {
                    generics.push_back(std::make_tuple(lhs.string, data_type, value));
                }
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(")", true);

        return generics;
    }

    // ###########################################################################
    // ###########      Assemble Netlist from Intermediate Format       ##########
    // ###########################################################################

    bool VerilogParser::construct_netlist(VerilogModule& top_module)
    {
        m_netlist->set_design_name(top_module.m_name);

        std::unordered_map<std::string, u32> instantiation_count;

        // preparations for alias: count the occurences of all names
        std::queue<VerilogModule*> q;
        q.push(&top_module);

        // top entity instance will be named after its entity, so take into account for aliases
        m_instance_name_occurrences["top_module"]++;

        // global input/output signals will be named after ports, so take into account for aliases
        for (const std::string& port_identifier : top_module.m_port_identifiers)
        {
            m_signal_name_occurrences[port_identifier]++;
        }

        while (!q.empty())
        {
            VerilogModule* module = q.front();
            q.pop();

            instantiation_count[module->m_name]++;

            for (const std::string& signal_identifier : module->m_signals)
            {
                m_signal_name_occurrences[signal_identifier]++;
            }

            for (const auto& instance_identifier : module->m_instances)
            {
                m_instance_name_occurrences[instance_identifier]++;

                if (const auto it = m_modules.find(module->m_instance_types.at(instance_identifier)); it != m_modules.end())
                {
                    q.push(&(it->second));
                }
            }
        }

        // detect unused entities
        for (const auto& module : m_modules)
        {
            if (instantiation_count[module.first] == 0)
            {
                log_warning("verilog_parser", "module '{}' has been defined in the netlist but is not instantiated.", module.first);
            }
        }

        // for the top module, generate global i/o signals for all ports
        std::map<std::string, std::string> top_assignments;
        for (const auto& [port_identifier, expanded_port_identifiers] : top_module.m_expanded_port_identifiers)
        {
            PinDirection direction;
            if (const auto port_expr_it = top_module.m_port_ident_to_expr.find(port_identifier); port_expr_it != top_module.m_port_ident_to_expr.end())
            {
                direction = top_module.m_port_directions.at(port_expr_it->second);
            }
            else
            {
                direction = top_module.m_port_directions.at(port_identifier);
            }

            for (const auto& expanded_identifier : expanded_port_identifiers)
            {
                Net* global_port_net = m_netlist->create_net(expanded_identifier);
                if (global_port_net == nullptr)
                {
                    log_error("verilog_parser", "could not create global port net '{}'.", expanded_identifier);
                    return false;
                }

                m_net_by_name[expanded_identifier] = global_port_net;

                // assign global port nets to ports of top module
                top_assignments[expanded_identifier] = expanded_identifier;

                if (direction == PinDirection::input || direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_input_net())
                    {
                        log_error("verilog_parser", "could not mark global port net '{}' as global input.", expanded_identifier);
                        return false;
                    }
                }

                if (direction == PinDirection::output || direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_output_net())
                    {
                        log_error("verilog_parser", "could not mark global port net '{}' as global output.", expanded_identifier);
                        return false;
                    }
                }
            }
        }

        if (!instantiate_module("top_module", top_module, nullptr, top_assignments))
        {
            // error printed in subfunction
            return false;
        }

        // merge nets without gates in between them
        while (!m_nets_to_merge.empty())
        {
            // master = net that other nets are merged into
            // slave = net to merge into master and then delete
            bool progress_made = false;

            for (const auto& [master, merge_set] : m_nets_to_merge)
            {
                // check if none of the slaves is itself a master
                bool is_master = false;
                for (const auto& slave : merge_set)
                {
                    if (m_nets_to_merge.find(slave) != m_nets_to_merge.end())
                    {
                        is_master = true;
                        break;
                    }
                }

                if (is_master)
                {
                    continue;
                }

                auto master_net = m_net_by_name.at(master);
                for (const auto& slave : merge_set)
                {
                    auto slave_net = m_net_by_name.at(slave);

                    // merge sources
                    if (slave_net->is_global_input_net())
                    {
                        master_net->mark_global_input_net();
                    }

                    for (auto src : slave_net->get_sources())
                    {
                        Gate* src_gate      = src->get_gate();
                        std::string src_pin = src->get_pin();

                        slave_net->remove_source(src);

                        if (!master_net->is_a_source(src_gate, src_pin))
                        {
                            master_net->add_source(src_gate, src_pin);
                        }
                    }

                    // merge destinations
                    if (slave_net->is_global_output_net())
                    {
                        master_net->mark_global_output_net();
                    }

                    for (auto dst : slave_net->get_destinations())
                    {
                        Gate* dst_gate      = dst->get_gate();
                        std::string dst_pin = dst->get_pin();

                        slave_net->remove_destination(dst);

                        if (!master_net->is_a_destination(dst_gate, dst_pin))
                        {
                            master_net->add_destination(dst_gate, dst_pin);
                        }
                    }

                    // merge generics and attributes
                    for (const auto it : slave_net->get_data_map())
                    {
                        if (!master_net->set_data(std::get<0>(it.first), std::get<1>(it.first), std::get<0>(it.second), std::get<1>(it.second)))
                        {
                            log_warning("verilog_parser",
                                        "unable to transfer data from slave net '{}' with ID {} to master net '{}' with ID {}.",
                                        slave_net->get_name(),
                                        slave_net->get_id(),
                                        master_net->get_name(),
                                        master_net->get_id());
                        }
                    }

                    // update module ports
                    if (const auto it = m_module_ports.find(slave_net); it != m_module_ports.end())
                    {
                        m_module_ports[master_net] = it->second;
                        m_module_ports.erase(it);
                    }

                    // make sure to keep module ports up to date
                    m_netlist->delete_net(slave_net);
                    m_net_by_name.erase(slave);
                }

                m_nets_to_merge.erase(master);
                progress_made = true;
                break;
            }

            if (!progress_made)
            {
                log_error("hdl_parser", "cyclic dependency between signals detected, cannot parse netlist.");
                return false;
            }
        }

        // assign module ports
        for (const auto& [net, port_info] : m_module_ports)
        {
            const PinDirection direction = std::get<0>(port_info);
            const std::string& port_name = std::get<1>(port_info);
            Module* module               = std::get<2>(port_info);

            if (direction == PinDirection::input || direction == PinDirection::inout)
            {
                module->set_input_port_name(net, port_name);
            }

            if (direction == PinDirection::output || direction == PinDirection::inout)
            {
                module->set_output_port_name(net, port_name);
            }
        }

        return true;
    }

    Module* VerilogParser::instantiate_module(const std::string& instance_identifier,
                                                     VerilogModule& verilog_module,
                                                     Module* parent,
                                                     const std::map<std::string, std::string>& parent_module_assignments)
    {
        std::unordered_map<std::string, std::string> signal_alias;
        std::unordered_map<std::string, std::string> instance_alias;

        // const auto& e              = m_entities.at(entity_inst_type);
        // const auto& entity_signals = e.get_signals();

        // TODO check parent module assignments for port aliases

        instance_alias[instance_identifier] = get_unique_alias(m_instance_name_occurrences, instance_identifier);

        // create netlist module
        Module* module;
        if (parent == nullptr)
        {
            module = m_netlist->get_top_module();
            module->set_name(instance_alias.at(instance_identifier));
        }
        else
        {
            module = m_netlist->create_module(instance_alias.at(instance_identifier), parent);
        }

        std::string instance_type = verilog_module.m_name;
        if (module == nullptr)
        {
            log_error("verilog_parser", "could not instantiate instance '{}' of module '{}'.", instance_identifier, instance_type);
            return nullptr;
        }
        module->set_type(instance_type);

        // assign entity-level attributes
        for (const std::tuple<std::string, std::string, std::string>& attribute : verilog_module.m_attributes)
        {
            if (!module->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
            {
                log_warning("verilog_parser",
                            "could not set attribute ({}, {}, {}) for instance '{}' type '{}'.",
                            std::get<0>(attribute),
                            std::get<1>(attribute),
                            std::get<2>(attribute),
                            instance_identifier,
                            instance_type);
            }
        }

        // assign module port names and attributes
        for (const auto& [port_identifier, expanded_port_identifiers] : verilog_module.m_expanded_port_identifiers)
        {
            std::string internal_identifier;
            if (const auto port_expr_it = verilog_module.m_port_ident_to_expr.find(port_identifier); port_expr_it != verilog_module.m_port_ident_to_expr.end())
            {
                internal_identifier = port_expr_it->second;
            }
            else
            {
                internal_identifier = port_identifier;
            }

            PinDirection direction = verilog_module.m_port_directions.at(internal_identifier);

            for (const auto& expanded_identifier : expanded_port_identifiers)
            {
                if (const auto it = parent_module_assignments.find(expanded_identifier); it != parent_module_assignments.end())
                {
                    Net* port_net            = m_net_by_name.at(it->second);
                    m_module_ports[port_net] = std::make_tuple(direction, expanded_identifier, module);

                    // assign port attributes
                    if (const auto port_attr_it = verilog_module.m_port_attributes.find(internal_identifier); port_attr_it != verilog_module.m_port_attributes.end())
                    {
                        for (const std::tuple<std::string, std::string, std::string>& attribute : port_attr_it->second)
                        {
                            if (!port_net->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
                            {
                                log_warning("verilog_parser",
                                            "could not set attribute ({}, {}, {}) for port '{}' of instance '{}' of type '{}'.",
                                            std::get<0>(attribute),
                                            std::get<1>(attribute),
                                            std::get<2>(attribute),
                                            port_identifier,
                                            instance_identifier,
                                            instance_type);
                            }
                        }
                    }
                }
            }
        }

        // create internal signals
        for (const auto& [signal_identifier, expanded_signal_identifiers] : verilog_module.m_expanded_signals)
        {
            for (const std::string& expanded_identifier : expanded_signal_identifiers)
            {
                signal_alias[expanded_identifier] = get_unique_alias(m_signal_name_occurrences, expanded_identifier);

                // create new net for the signal
                Net* signal_net = m_netlist->create_net(signal_alias.at(expanded_identifier));
                if (signal_net == nullptr)
                {
                    log_error(
                        "verilog_parser", "could not instantiate net '{}' of instance '{}' of type '{}'.", expanded_identifier, instance_identifier, instance_type, instance_identifier, instance_type);
                    return nullptr;
                }

                m_net_by_name[signal_alias.at(expanded_identifier)] = signal_net;

                // assign signal attributes
                if (const auto signal_attr_it = verilog_module.m_signal_attributes.find(signal_identifier); signal_attr_it != verilog_module.m_port_attributes.end())
                {
                    for (const std::tuple<std::string, std::string, std::string>& attribute : signal_attr_it->second)
                    {
                        if (!signal_net->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
                        {
                            log_warning("verilog_parser",
                                        "could not set attribute ({}, {}, {}) for net '{}' of instance '{}' of type '{}'.",
                                        std::get<0>(attribute),
                                        std::get<1>(attribute),
                                        std::get<2>(attribute),
                                        signal_identifier,
                                        instance_identifier,
                                        instance_type);
                        }
                    }
                }
            }
        }

        // schedule assigned nets for merging
        for (const auto& [left_expanded_signal, right_expanded_signal] : verilog_module.m_expanded_assignments)
        {
            std::string a = left_expanded_signal;
            std::string b = right_expanded_signal;

            if (const auto parent_it = parent_module_assignments.find(a); parent_it != parent_module_assignments.end())
            {
                a = parent_it->second;
            }
            else if (const auto alias_it = signal_alias.find(a); alias_it != signal_alias.end())
            {
                a = alias_it->second;
            }
            else
            {
                log_warning("verilog_parser", "cannot find alias for net '{}' of instance '{}' of type '{}'.", a, instance_identifier, instance_type);
            }

            if (const auto parent_it = parent_module_assignments.find(b); parent_it != parent_module_assignments.end())
            {
                b = parent_it->second;
            }
            else if (const auto alias_it = signal_alias.find(b); alias_it != signal_alias.end())
            {
                b = alias_it->second;
            }
            else if (b != "'0'" && b != "'1'" && b != "'Z'")
            {
                log_warning("verilog_parser", "cannot find alias for net '{}' of instance '{}' of type '{}'.", b, instance_identifier, instance_type);
            }

            m_nets_to_merge[b].push_back(a);
        }

        // process instances i.e. gates or other entities
        for (const std::string& inst_identifier : verilog_module.m_instances)
        {
            const std::string& inst_type = verilog_module.m_instance_types.at(inst_identifier);

            // will later hold either module or gate, so attributes can be assigned properly
            DataContainer* container;

            // assign actual signal names to ports
            std::map<std::string, std::string> instance_assignments;

            // if the instance is another entity, recursively instantiate it
            if (auto module_it = m_modules.find(inst_type); module_it != m_modules.end())
            {
                // expand port assignments
                for (auto& [port, assignments] : verilog_module.m_instance_port_assignments.at(inst_identifier))
                {
                    const std::vector<std::string>& left_port = module_it->second.m_expanded_port_identifiers.at(port);
                    const std::vector<std::string> right_port = get_expanded_assignment_signal(verilog_module, assignments, true);

                    u32 max_size;
                    if (right_port.size() <= left_port.size())
                    {
                        max_size = right_port.size();
                    }
                    else
                    {
                        max_size = left_port.size();
                    }

                    for (u32 i = 0; i < max_size; i++)
                    {
                        const std::string& rhs = right_port.at(i);
                        const std::string& lhs = left_port.at(i);
                        if (const auto it = parent_module_assignments.find(rhs); it != parent_module_assignments.end())
                        {
                            instance_assignments[lhs] = it->second;
                        }
                        else
                        {
                            if (const auto alias_it = signal_alias.find(rhs); alias_it != signal_alias.end())
                            {
                                instance_assignments[lhs] = alias_it->second;
                            }
                            else if (rhs == "'0'" || rhs == "'1'" || rhs == "'Z'")
                            {
                                instance_assignments[lhs] = rhs;
                            }
                            else
                            {
                                log_error("verilog_parser", "port assignment \"{} = {}\" is invalid for instance '{}' of type '{}'", lhs, rhs, inst_identifier, inst_type);
                                return nullptr;
                            }
                        }
                    }
                }

                container = instantiate_module(inst_identifier, module_it->second, module, instance_assignments);
                if (container == nullptr)
                {
                    return nullptr;
                }
            }
            // otherwise it has to be an element from the gate library
            else if (const auto gate_type_it = m_gate_types.find(inst_type); gate_type_it != m_gate_types.end())
            {
                // create the new gate
                instance_alias[inst_identifier] = get_unique_alias(m_instance_name_occurrences, inst_identifier);

                Gate* new_gate = m_netlist->create_gate(gate_type_it->second, instance_alias.at(inst_identifier));
                if (new_gate == nullptr)
                {
                    log_error("verilog_parser", "could not instantiate gate '{}' within instance '{}' of type '{}'.", inst_identifier, instance_identifier, instance_type);
                    return nullptr;
                }

                module->assign_gate(new_gate);
                container = new_gate;

                // if gate is of a GND or VCC gate type, mark it as such
                if (m_vcc_gate_types.find(inst_type) != m_vcc_gate_types.end() && !new_gate->mark_vcc_gate())
                {
                    return nullptr;
                }
                if (m_gnd_gate_types.find(inst_type) != m_gnd_gate_types.end() && !new_gate->mark_gnd_gate())
                {
                    return nullptr;
                }

                // cache pin types
                std::unordered_map<std::string, PinDirection> pin_to_direction = gate_type_it->second->get_pin_directions();
                std::unordered_map<std::string, std::vector<std::string>> pin_groups;
                for (const auto& [group_name, pins] : gate_type_it->second->get_pin_groups())
                {
                    for (const auto& [index, pin] : pins)
                    {
                        pin_groups[group_name].push_back(pin + "(" + std::to_string(index) + ")");
                    }
                }

                // expand port assignments
                for (auto& [port, assignments] : verilog_module.m_instance_port_assignments.at(inst_identifier))
                {
                    std::vector<std::string> left_port;
                    if (const auto group_it = pin_groups.find(port); group_it != pin_groups.end())
                    {
                        left_port = group_it->second;
                    }
                    else
                    {
                        left_port.push_back(port);
                    }
                    const std::vector<std::string> right_port = get_expanded_assignment_signal(verilog_module, assignments, true);

                    u32 max_size;
                    if (right_port.size() <= left_port.size())
                    {
                        max_size = right_port.size();
                    }
                    else
                    {
                        max_size = left_port.size();
                    }

                    for (u32 i = 0; i < max_size; i++)
                    {
                        const std::string& rhs = right_port.at(i);
                        const std::string& lhs = left_port.at(i);
                        if (const auto it = parent_module_assignments.find(rhs); it != parent_module_assignments.end())
                        {
                            instance_assignments[lhs] = it->second;
                        }
                        else
                        {
                            if (const auto alias_it = signal_alias.find(rhs); alias_it != signal_alias.end())
                            {
                                instance_assignments[lhs] = alias_it->second;
                            }
                            else if (rhs == "'0'" || rhs == "'1'" || rhs == "'Z'")
                            {
                                instance_assignments[lhs] = rhs;
                            }
                            else
                            {
                                log_error("verilog_parser", "pin assignment \"{} = {}\" is invalid for instance '{}' of type '{}'", lhs, rhs, inst_identifier, inst_type);
                                return nullptr;
                            }
                        }
                    }
                }

                // check for port
                for (const auto& [port, assignment] : instance_assignments)
                {
                    // get the respective net for the assignment
                    if (const auto net_it = m_net_by_name.find(assignment); net_it == m_net_by_name.end())
                    {
                        log_error("verilog_parser", "signal '{}' of instance '{}' of type '{}' has not been declared.", assignment, inst_identifier, inst_type);
                        return nullptr;
                    }
                    else
                    {
                        auto current_net = net_it->second;

                        // add net src/dst by pin types
                        bool is_input  = false;
                        bool is_output = false;

                        if (const auto it = pin_to_direction.find(port); it != pin_to_direction.end())
                        {
                            if (it->second == PinDirection::input || it->second == PinDirection::inout)
                            {
                                is_input = true;
                            }

                            if (it->second == PinDirection::output || it->second == PinDirection::inout)
                            {
                                is_output = true;
                            }
                        }

                        if (!is_input && !is_output)
                        {
                            log_error("verilog_parser", "undefined pin '{}' for gate '{}' of type '{}'.", port, new_gate->get_name(), new_gate->get_type()->get_name());
                            return nullptr;
                        }

                        if (is_output && !current_net->add_source(new_gate, port))
                        {
                            return nullptr;
                        }

                        if (is_input && !current_net->add_destination(new_gate, port))
                        {
                            return nullptr;
                        }
                    }
                }
            }
            else
            {
                log_error("verilog_parser", "could not find gate type '{}' in gate library '{}'.", inst_type, m_netlist->get_gate_library()->get_name());
                return nullptr;
            }

            // assign instance attributes
            for (const auto& attribute : verilog_module.m_instance_attributes.at(inst_identifier))
            {
                if (!container->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
                {
                    log_warning("verilog_parser",
                                "could not set attribute ({}, {}, {}) for instance '{}' of type '{}' within instance '{}' of type '{}'.",
                                std::get<0>(attribute),
                                std::get<1>(attribute),
                                std::get<2>(attribute),
                                inst_identifier,
                                inst_type,
                                instance_identifier,
                                instance_type);
                }
            }

            // process generics
            for (const auto& generic : verilog_module.m_instance_generic_assignments.at(inst_identifier))
            {
                if (!container->set_data("generic", std::get<0>(generic), std::get<1>(generic), std::get<2>(generic)))
                {
                    log_warning("verilog_parser",
                                "could not set generic ({}, {}, {}) for instance '{}' of type '{}' within instance '{}' of type '{}'.",
                                std::get<0>(generic),
                                std::get<1>(generic),
                                std::get<2>(generic),
                                inst_identifier,
                                inst_type,
                                instance_identifier,
                                instance_type);
                }
            }
        }

        return module;
    }

    // ###########################################################################
    // ###################          Helper Functions          ####################
    // ###########################################################################

    void VerilogParser::remove_comments(std::string& line, bool& multi_line_comment) const
    {
        bool repeat = true;

        while (repeat)
        {
            repeat = false;

            // skip empty lines
            if (line.empty())
            {
                break;
            }

            const size_t single_line_comment_begin = line.find("//");
            const size_t multi_line_comment_begin  = line.find("/*");
            const size_t multi_line_comment_end    = line.find("*/");

            std::string begin = "";
            std::string end   = "";

            if (multi_line_comment == true)
            {
                if (multi_line_comment_end != std::string::npos)
                {
                    // multi-line comment ends in current line
                    multi_line_comment = false;
                    line               = line.substr(multi_line_comment_end + 2);
                    repeat             = true;
                }
                else
                {
                    // current line entirely within multi-line comment
                    line = "";
                    break;
                }
            }
            else
            {
                if (single_line_comment_begin != std::string::npos)
                {
                    if (multi_line_comment_begin == std::string::npos || (multi_line_comment_begin != std::string::npos && multi_line_comment_begin > single_line_comment_begin))
                    {
                        // single-line comment
                        line   = line.substr(0, single_line_comment_begin);
                        repeat = true;
                    }
                }
                else if (multi_line_comment_begin != std::string::npos)
                {
                    if (multi_line_comment_end != std::string::npos)
                    {
                        // multi-line comment entirely in current line
                        line   = line.substr(0, multi_line_comment_begin) + line.substr(multi_line_comment_end + 2);
                        repeat = true;
                    }
                    else
                    {
                        // multi-line comment starts in current line
                        multi_line_comment = true;
                        line               = line.substr(0, multi_line_comment_begin);
                    }
                }
            }
        }
    }

    std::vector<u32> VerilogParser::parse_range(TokenStream<std::string>& range_str) const
    {
        if (range_str.remaining() == 1)
        {
            return {(u32)std::stoi(range_str.consume().string)};
        }

        // MSB to LSB
        const int end = std::stoi(range_str.consume().string);
        range_str.consume(":", true);
        const int start = std::stoi(range_str.consume().string);

        const int direction = (start <= end) ? 1 : -1;

        std::vector<u32> res;
        for (int i = start; i != end + direction; i += direction)
        {
            res.push_back((u32)i);
        }
        return res;
    }

    std::vector<std::string> VerilogParser::get_expanded_assignment_signal(VerilogModule& module, TokenStream<std::string>& signal_str, bool allow_numerics)
    {
        // PARSE ASSIGNMENT
        //   assignment can currently be one of the following:
        //   (1) NAME *single-dimensional*
        //   (2) NAME *multi-dimensional*
        //   (3) NUMBER
        //   (4) NAME[INDEX1][INDEX2]...
        //   (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
        //   (6) {(1 - 5), (1 - 5), ...}

        std::vector<std::string> res;
        std::vector<TokenStream<std::string>> parts;

        // always go LSB to MSB (from right to left)

        // (6) {(1) - (5), (1) - (5), ...}
        if (signal_str.peek() == "{")
        {
            signal_str.consume("{", true);

            TokenStream<std::string> assignment_list_str = signal_str.extract_until("}");
            signal_str.consume("}", true);

            do
            {
                parts.push_back(assignment_list_str.extract_until(","));
            } while (assignment_list_str.consume(",", false));
        }
        else
        {
            parts.push_back(signal_str);
        }

        for (auto it = parts.rbegin(); it != parts.rend(); it++)
        {
            TokenStream<std::string>& part_stream = *it;

            const Token<std::string> signal_name_token = part_stream.consume();
            const u32 line_number                      = signal_name_token.number;
            std::string signal_name                    = signal_name_token.string;

            // (3) NUMBER
            if (isdigit(signal_name[0]) || signal_name[0] == '\'')
            {
                if (!allow_numerics)
                {
                    log_error("verilog_parser", "numeric value {} in line {} is invalid.", signal_name, line_number);
                    return {};
                }

                std::vector<std::string> binary_vector = get_bin_from_literal(signal_name_token);
                if (binary_vector.empty())
                {
                    return {};
                }
                res.insert(res.end(), binary_vector.begin(), binary_vector.end());
            }
            else
            {
                std::vector<std::vector<u32>> ranges;

                // any bounds specified?
                if (part_stream.consume("["))
                {
                    // (4) NAME[INDEX1][INDEX2]...
                    // (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...

                    do
                    {
                        TokenStream<std::string> range_str = part_stream.extract_until("]");
                        ranges.emplace_back(parse_range(range_str));
                        part_stream.consume("]", true);
                    } while (part_stream.consume("[", false));
                }
                else
                {
                    // (1) NAME *single-dimensional*
                    // (2) NAME *multi-dimensional*

                    std::vector<std::vector<u32>> reference_ranges;
                    if (const auto signal_ranges_it = module.m_signal_ranges.find(signal_name); signal_ranges_it != module.m_signal_ranges.end())
                    {
                        reference_ranges = signal_ranges_it->second;
                    }
                    else if (const auto port_ranges_it = module.m_port_ranges.find(signal_name); port_ranges_it != module.m_port_ranges.end())
                    {
                        reference_ranges = port_ranges_it->second;
                    }

                    ranges = reference_ranges;
                }

                if (!ranges.empty())
                {
                    std::vector<std::string> expanded_signal = expand_ranges(signal_name, ranges);
                    res.insert(res.end(), expanded_signal.begin(), expanded_signal.end());
                }
                else
                {
                    res.push_back(signal_name);
                }
            }

            // create new signal for assign
        }

        return res;
    }

    static const std::map<char, std::vector<std::string>> oct_to_bin = {{'0', {"0", "0", "0"}},
                                                                        {'1', {"1", "0", "0"}},
                                                                        {'2', {"0", "1", "0"}},
                                                                        {'3', {"1", "1", "0"}},
                                                                        {'4', {"0", "0", "1"}},
                                                                        {'5', {"1", "0", "1"}},
                                                                        {'6', {"0", "1", "1"}},
                                                                        {'7', {"1", "1", "1"}}};
    static const std::map<char, std::vector<std::string>> hex_to_bin = {{'0', {"0", "0", "0", "0"}},
                                                                        {'1', {"1", "0", "0", "0"}},
                                                                        {'2', {"0", "1", "0", "0"}},
                                                                        {'3', {"1", "1", "0", "0"}},
                                                                        {'4', {"0", "0", "1", "0"}},
                                                                        {'5', {"1", "0", "1", "0"}},
                                                                        {'6', {"0", "1", "1", "0"}},
                                                                        {'7', {"1", "1", "1", "0"}},
                                                                        {'8', {"0", "0", "0", "1"}},
                                                                        {'9', {"1", "0", "0", "1"}},
                                                                        {'a', {"0", "1", "0", "1"}},
                                                                        {'b', {"1", "1", "0", "1"}},
                                                                        {'c', {"0", "0", "1", "1"}},
                                                                        {'d', {"1", "0", "1", "1"}},
                                                                        {'e', {"0", "1", "1", "1"}},
                                                                        {'f', {"1", "1", "1", "1"}}};

    // TODO add high-impedance support
    std::vector<std::string> VerilogParser::get_bin_from_literal(const Token<std::string>& value_token) const
    {
        const u32 line_number   = value_token.number;
        const std::string value = utils::to_lower(utils::replace(value_token.string, std::string("_"), std::string("")));

        i32 len = -1;
        std::string prefix;
        std::string number;
        std::vector<std::string> res;

        // base specified?
        if (value.find('\'') == std::string::npos)
        {
            prefix = "d";
            number = value;
        }
        else
        {
            if (value.at(0) != '\'')
            {
                len = std::stoi(value.substr(0, value.find('\'')));
            }
            prefix = value.substr(value.find('\'') + 1, 1);
            number = value.substr(value.find('\'') + 2);
        }

        // select base
        switch (prefix.at(0))
        {
            case 'b': {
                for (auto it = number.rbegin(); it != number.rend(); it++)
                {
                    const char c = *it;
                    if (c >= '0' && c <= '1')
                    {
                        res.push_back(std::string(1, c));
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within binary number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }
                break;
            }

            case 'o':
                for (auto it = number.rbegin(); it != number.rend(); it++)
                {
                    const char c = *it;
                    if (c >= '0' && c <= '7')
                    {
                        const std::vector<std::string>& bits = oct_to_bin.at(c);
                        res.insert(res.end(), bits.begin(), bits.end());
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within octal number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }
                break;

            case 'd': {
                u64 tmp_val = 0;

                for (const char c : number)
                {
                    if (c >= '0' && c <= '9')
                    {
                        tmp_val = (tmp_val * 10) + (c - '0');
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within decimal number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }

                do
                {
                    res.push_back(((tmp_val & 1) == 1) ? "1" : "0");
                    tmp_val >>= 1;
                } while (tmp_val != 0);
                break;
            }

            case 'h': {
                for (auto it = number.rbegin(); it != number.rend(); it++)
                {
                    const char c = *it;
                    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                    {
                        const std::vector<std::string>& bits = hex_to_bin.at(c);
                        res.insert(res.end(), bits.begin(), bits.end());
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within hexadecimal number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }
                break;
            }

            default: {
                log_error("verilog_parser", "invalid base '{}' within number literal {} in line {}.", prefix, value, line_number);
                return {};
            }
        }

        if (len != -1)
        {
            // fill with '0'
            for (i32 i = 0; i < len - (i32)res.size(); i++)
            {
                res.push_back("0");
            }
        }

        return res;
    }

    std::string VerilogParser::get_hex_from_literal(const Token<std::string>& value_token) const
    {
        const u32 line_number   = value_token.number;
        const std::string value = utils::to_lower(utils::replace(value_token.string, std::string("_"), std::string("")));

        i32 len = -1;
        std::string prefix;
        std::string number;
        u32 base;

        // base specified?
        if (value.find('\'') == std::string::npos)
        {
            prefix = "d";
            number = value;
        }
        else
        {
            if (value.at(0) != '\'')
            {
                len = std::stoi(value.substr(0, value.find('\'')));
            }
            prefix = value.substr(value.find('\'') + 1, 1);
            number = value.substr(value.find('\'') + 2);
        }

        // select base
        switch (prefix.at(0))
        {
            case 'b': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '1'); }))
                {
                    log_error("verilog_parser", "invalid character within binary number literal {} in line {}", value, line_number);
                    return "";
                }

                base = 2;
                break;
            }

            case 'o': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '7'); }))
                {
                    log_error("verilog_parser", "invalid character within octal number literal {} in line {}", value, line_number);
                    return "";
                }

                base = 8;
                break;
            }

            case 'd': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9'); }))
                {
                    log_error("verilog_parser", "invalid character within decimal number literal {} in line {}", value, line_number);
                    return "";
                }

                base = 10;
                break;
            }

            case 'h': {
                std::string res;

                for (const char c : number)
                {
                    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                    {
                        res += c;
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within hexadecimal number literal {} in line {}", value, line_number);
                        return "";
                    }
                }

                return res;
            }

            default: {
                log_error("verilog_parser", "invalid base '{}' within number literal {} in line {}", prefix, value, line_number);
                return "";
            }
        }

        std::stringstream ss;
        if (len != -1)
        {
            // fill with '0'
            ss << std::setfill('0') << std::setw((len + 3) / 4) << std::hex << stoull(number, 0, base);
        }
        else
        {
            ss << std::hex << stoull(number, 0, base);
        }
        return ss.str();
    }

    bool VerilogParser::is_in_bounds(const std::vector<std::pair<i32, i32>>& bounds, const std::vector<std::pair<i32, i32>>& reference_bounds) const
    {
        if (bounds.size() != reference_bounds.size())
        {
            return false;
        }

        for (u32 i = 0; i < bounds.size(); i++)
        {
            i32 ref_max, ref_min;
            if (reference_bounds[i].first < reference_bounds[i].second)
            {
                ref_min = reference_bounds[i].first;
                ref_max = reference_bounds[i].second;
            }
            else
            {
                ref_min = reference_bounds[i].second;
                ref_max = reference_bounds[i].first;
            }

            if (!(((ref_min <= bounds[i].first) && (bounds[i].first <= ref_max)) && ((ref_min <= bounds[i].second) && (bounds[i].second <= ref_max))))
            {
                return false;
            }
        }

        return true;
    }

    std::vector<std::string> VerilogParser::expand_ranges(const std::string& name, const std::vector<std::vector<u32>>& ranges) const
    {
        std::vector<std::string> res;

        expand_ranges_recursively(res, name, ranges, 0);

        return res;
    }

    void VerilogParser::expand_ranges_recursively(std::vector<std::string>& expanded_names, const std::string& current_name, const std::vector<std::vector<u32>>& ranges, u32 dimension) const
    {
        // expand signal recursively
        if (ranges.size() > dimension)
        {
            for (const u32 index : ranges[dimension])
            {
                expand_ranges_recursively(expanded_names, current_name + "(" + core_strings::convert_string<std::string, std::string>(std::to_string(index)) + ")", ranges, dimension + 1);
            }
        }
        else
        {
            // last dimension
            expanded_names.push_back(current_name);
        }
    }

    std::string VerilogParser::get_unique_alias(std::unordered_map<std::string, u32>& name_occurrences, const std::string& name) const
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
