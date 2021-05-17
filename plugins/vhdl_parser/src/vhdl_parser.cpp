#include "vhdl_parser/vhdl_parser.h"

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
    bool VHDLParser::parse(const std::filesystem::path& file_path)
    {
        m_entities.clear();
        m_attribute_buffer.clear();
        m_attribute_types.clear();

        {
            std::ifstream ifs;
            ifs.open(file_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                log_error("vhdl_parser", "unable to open '{}'.", file_path.string());
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
        catch (TokenStream<ci_string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                log_error("vhdl_parser", "{} near line {}.", e.message, e.line_number);
            }
            else
            {
                log_error("vhdl_parser", "{}.", e.message);
            }
            return false;
        }

        // expand module port identifiers, signals, and assignments
        for (auto& [entity_name, entity] : m_entities)
        {
            // expand port identifiers
            for (ci_string& port_identifier : entity.m_port_names)
            {
                if (const auto ranges_it = entity.m_port_ranges.find(port_identifier); ranges_it != entity.m_port_ranges.end())
                {
                    entity.m_expanded_port_names[port_identifier] = expand_ranges(port_identifier, ranges_it->second);
                }
                else
                {
                    entity.m_expanded_port_names[port_identifier] = std::vector<ci_string>({port_identifier});
                }
            }

            // expand signals
            for (ci_string& signal_identifier : entity.m_signals)
            {
                if (const auto ranges_it = entity.m_signal_ranges.find(signal_identifier); ranges_it != entity.m_signal_ranges.end())
                {
                    entity.m_expanded_signals[signal_identifier] = expand_ranges(signal_identifier, ranges_it->second);
                }
                else
                {
                    entity.m_expanded_signals[signal_identifier] = std::vector<ci_string>({signal_identifier});
                }
            }

            // expand assignments
            for (auto& [left_stream, right_stream] : entity.m_assignments)
            {
                std::vector<ci_string> left_signals  = expand_assignment_signal(entity, left_stream, false);
                std::vector<ci_string> right_signals = expand_assignment_signal(entity, right_stream, true);

                u32 left_size  = left_signals.size();
                u32 right_size = right_signals.size();
                if (left_size != right_size)
                {
                    log_error("vhdl_parser", "assignment width mismatch in entity '{}'.", entity_name);
                    return false;
                }
                else
                {
                    for (u32 i = 0; i < right_size; i++)
                    {
                        entity.m_expanded_assignments.push_back(std::make_pair(left_signals.at(i), right_signals.at(i)));
                    }
                }
            }
        }

        // expand entity port assignments
        for (auto& [entity_name, entity] : m_entities)
        {
            for (const ci_string& instance_name : entity.m_instances)
            {
                const ci_string& instance_type = entity.m_instance_types.at(instance_name);
                if (auto entity_it = m_entities.find(instance_type); entity_it != m_entities.end())
                {
                    if (auto inst_it = entity.m_instance_assignments.find(instance_name); inst_it != entity.m_instance_assignments.end())
                    {
                        for (auto& [port_stream, assignment_stream] : inst_it->second)
                        {
                            ci_string port_name = port_stream.consume().string;
                            std::vector<ci_string> left_port;

                            if (port_stream.consume("("))
                            {
                                std::vector<std::vector<u32>> ranges;
                                u32 closing_pos = port_stream.find_next(")");
                                do
                                {
                                    TokenStream<ci_string> range_stream = port_stream.extract_until(",", closing_pos);
                                    ranges.emplace_back(parse_range(range_stream));

                                } while (port_stream.consume(",", false));
                                port_stream.consume(")", true);

                                left_port = expand_ranges(port_name, ranges);
                            }
                            else
                            {
                                left_port = entity_it->second.m_expanded_port_names.at(port_name);
                            }

                            const std::vector<ci_string> right_port = expand_assignment_signal(entity, assignment_stream, false);

                            if (right_port.size() != left_port.size())
                            {
                                log_error("vhdl_parser", "port assignment width mismatch at instance '{}' of entity '{}' within entity '{}'.", instance_name, instance_type, entity_name);
                                return false;
                            }

                            for (u32 i = 0; i < left_port.size(); i++)
                            {
                                entity.m_expanded_entity_assignments[instance_name].push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    std::unique_ptr<Netlist> VHDLParser::instantiate(const GateLibrary* gate_library)
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
        if (m_entities.empty())
        {
            log_error("vhdl_parser", "file did not contain any modules.");
            return nullptr;
        }

        m_gate_types.clear();
        m_gnd_gate_types.clear();
        m_vcc_gate_types.clear();
        m_instance_name_occurrences.clear();
        m_signal_name_occurrences.clear();
        m_net_by_name.clear();
        m_nets_to_merge.clear();
        m_module_ports.clear();

        // buffer gate types
        for (const auto& [gt_name, gt] : gate_library->get_gate_types())
        {
            m_gate_types[core_strings::convert_string<std::string, ci_string>(gt_name)] = gt;
        }
        for (const auto& [gt_name, gt] : gate_library->get_gnd_gate_types())
        {
            m_gnd_gate_types[core_strings::convert_string<std::string, ci_string>(gt_name)] = gt;
        }
        for (const auto& [gt_name, gt] : gate_library->get_vcc_gate_types())
        {
            m_vcc_gate_types[core_strings::convert_string<std::string, ci_string>(gt_name)] = gt;
        }

        // create const 0 and const 1 net, will be removed if unused
        m_zero_net = m_netlist->create_net("'0'");
        if (m_zero_net == nullptr)
        {
            return nullptr;
        }
        m_net_by_name[core_strings::convert_string<std::string, ci_string>(m_zero_net->get_name())] = m_zero_net;

        m_one_net = m_netlist->create_net("'1'");
        if (m_one_net == nullptr)
        {
            return nullptr;
        }
        m_net_by_name[core_strings::convert_string<std::string, ci_string>(m_one_net->get_name())] = m_one_net;

        // construct the netlist with the last module being considered the top module
        VHDLEntity& top_entity = m_entities.at(m_last_entity);
        if (!construct_netlist(top_entity))
        {
            return nullptr;
        }

        // add global GND gate if required by any instance
        if (m_netlist->get_gnd_gates().empty())
        {
            if (!m_zero_net->get_destinations().empty())
            {
                GateType* gnd_type           = m_gnd_gate_types.begin()->second;
                const std::string output_pin = *gnd_type->get_pins_of_direction(PinDirection::output).begin();
                Gate* gnd                    = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");

                if (!m_netlist->mark_gnd_gate(gnd))
                {
                    return nullptr;
                }

                if (!m_zero_net->add_source(gnd, output_pin))
                {
                    return nullptr;
                }
            }
            else
            {
                m_netlist->delete_net(m_zero_net);
            }
        }

        // add global VCC gate if required by any instance
        if (m_netlist->get_vcc_gates().empty())
        {
            if (!m_one_net->get_destinations().empty())
            {
                GateType* vcc_type           = m_vcc_gate_types.begin()->second;
                const std::string output_pin = *vcc_type->get_pins_of_direction(PinDirection::output).begin();
                Gate* vcc                    = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");

                if (!m_netlist->mark_vcc_gate(vcc))
                {
                    return nullptr;
                }

                if (!m_one_net->add_source(vcc, output_pin))
                {
                    return nullptr;
                }
            }
            else
            {
                m_netlist->delete_net(m_one_net);
            }
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

        return result;
    }

    // ###########################################################################
    // ###########          Parse HDL into Intermediate Format          ##########
    // ###########################################################################

    bool VHDLParser::tokenize()
    {
        std::vector<Token<core_strings::CaseInsensitiveString>> parsed_tokens;
        const std::string delimiters = ",(): ;=><&";
        core_strings::CaseInsensitiveString current_token;
        u32 line_number = 0;

        std::string line;
        bool in_string = false;
        bool escaped   = false;

        while (std::getline(m_fs, line))
        {
            line_number++;
            if (line.find("--") != std::string::npos)
            {
                line = line.substr(0, line.find("--"));
            }
            for (char c : utils::trim(line))
            {
                if (in_string == false && c == '\\')
                {
                    escaped = !escaped;
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

                if (delimiters.find(c) == std::string::npos || escaped || in_string)
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
                        if (c == '=' && parsed_tokens.back() == "<")
                        {
                            parsed_tokens.back() = "<=";
                            continue;
                        }
                        else if (c == '=' && parsed_tokens.back() == ":")
                        {
                            parsed_tokens.back() = ":=";
                            continue;
                        }
                        else if (c == '>' && parsed_tokens.back() == "=")
                        {
                            parsed_tokens.back() = "=>";
                            continue;
                        }
                    }

                    if (!std::isspace(c))
                    {
                        parsed_tokens.emplace_back(line_number, core_strings::CaseInsensitiveString(1, c));
                    }
                }
            }
            if (!current_token.empty())
            {
                parsed_tokens.emplace_back(line_number, current_token);
                current_token.clear();
            }
        }
        m_token_stream = TokenStream(parsed_tokens, {"("}, {")"});
        return true;
    }

    bool VHDLParser::parse_tokens()
    {
        while (m_token_stream.remaining() > 0)
        {
            if (m_token_stream.peek() == "library" || m_token_stream.peek() == "use")
            {
                if (!parse_library())
                {
                    return false;
                }
            }
            else if (m_token_stream.peek() == "entity")
            {
                if (!parse_entity())
                {
                    return false;
                }
            }
            else if (m_token_stream.peek() == "architecture")
            {
                if (!parse_architecture())
                {
                    return false;
                }
            }
            else
            {
                log_error("vhdl_parser", "unexpected token '{}' in global scope in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
                return false;
            }
        }

        return true;
    }

    bool VHDLParser::parse_library()
    {
        if (m_token_stream.peek() == "use")
        {
            m_token_stream.consume("use", true);
            auto lib = m_token_stream.consume().string;
            m_token_stream.consume(";", true);

            // remove specific import like ".all" but keep the "."
            lib = utils::trim(lib.substr(0, lib.rfind(".") + 1));
            m_libraries.insert(lib);
        }
        else
        {
            m_token_stream.consume_until(";");
            m_token_stream.consume(";", true);
        }
        return true;
    }

    bool VHDLParser::parse_entity()
    {
        m_token_stream.consume("entity", true);
        const u32 line_number       = m_token_stream.peek().number;
        const ci_string entity_name = m_token_stream.consume().string;

        // verify entity name
        if (m_entities.find(entity_name) != m_entities.end())
        {
            log_error("vhdl_parser", "an entity with the name '{}' does already exist (see line {} and line {})", entity_name, line_number, m_entities.at(entity_name).m_line_number);
            return false;
        }

        m_token_stream.consume("is", true);
        VHDLEntity entity;
        entity.m_line_number = line_number;
        entity.m_name        = entity_name;

        m_attribute_buffer.clear();

        Token<ci_string> next_token = m_token_stream.peek();
        while (next_token != "end")
        {
            if (next_token == "generic")
            {
                m_token_stream.consume_until(";");
                m_token_stream.consume(";", true);
            }
            else if (next_token == "port")
            {
                if (!parse_port_definitons(entity))
                {
                    return false;
                }
            }
            else if (next_token == "attribute")
            {
                if (!parse_attribute())
                {
                    return false;
                }
            }
            else
            {
                log_error("vhdl_parser", "unexpected token '{}' in entity defintion in line {}", next_token.string, next_token.number);
                return false;
            }

            next_token = m_token_stream.peek();
        }

        m_token_stream.consume("end", true);
        m_token_stream.consume();
        m_token_stream.consume(";", true);

        // add to collection of entities
        m_entities.emplace(entity_name, entity);
        m_last_entity = entity_name;

        return true;
    }

    bool VHDLParser::parse_port_definitons(VHDLEntity& entity)
    {
        // default port assignments are not supported
        m_token_stream.consume("port", true);
        m_token_stream.consume("(", true);
        auto port_def_stream = m_token_stream.extract_until(")");

        while (port_def_stream.remaining() > 0)
        {
            std::vector<core_strings::CaseInsensitiveString> port_names;
            // std::set<signal> signals;

            const auto line_number = port_def_stream.peek().number;

            // extract names
            do
            {
                port_names.push_back(port_def_stream.consume().string);
            } while (port_def_stream.consume(",", false));

            port_def_stream.consume(":", true);

            // extract direction
            PinDirection direction;
            const ci_string direction_str = port_def_stream.consume().string;
            if (direction_str == "in")
            {
                direction = PinDirection::input;
            }
            else if (direction_str == "out")
            {
                direction = PinDirection::output;
            }
            else if (direction_str == "inout")
            {
                direction = PinDirection::inout;
            }
            else
            {
                log_error("vhdl_parser", "invalid direction '{}' for port declaration in line {}", direction_str, line_number);
                return false;
            }

            // extract ranges
            TokenStream<ci_string> port_stream = port_def_stream.extract_until(";");
            std::vector<std::vector<u32>> ranges;
            if (auto ranges_opt = parse_signal_ranges(port_stream); !ranges_opt.has_value())
            {
                // error already printed in subfunction
                return false;
            }
            else
            {
                ranges = ranges_opt.value();
            }

            port_def_stream.consume(";", port_def_stream.remaining() > 0);    // last entry has no semicolon, so no throw in that case

            for (const ci_string& port_name : port_names)
            {
                entity.m_port_names.push_back(port_name);
                entity.m_port_names_set.insert(port_name);
                entity.m_port_directions[port_name] = direction;
                if (!ranges.empty())
                {
                    entity.m_port_ranges[port_name] = ranges;
                }
            }
        }

        m_token_stream.consume(")", true);
        m_token_stream.consume(";", true);

        return true;
    }

    bool VHDLParser::parse_attribute()
    {
        const u32 line_number = m_token_stream.peek().number;

        m_token_stream.consume("attribute", true);
        const ci_string attribute_name = m_token_stream.consume().string;

        if (m_token_stream.peek() == ":")
        {
            m_token_stream.consume(":", true);
            m_attribute_types[attribute_name] = m_token_stream.join_until(";", " ");
            m_token_stream.consume(";", true);
        }
        else if (m_token_stream.peek() == "of" && m_token_stream.peek(2) == ":")
        {
            AttributeTarget target_class;
            m_token_stream.consume("of", true);
            const ci_string attribute_target = m_token_stream.consume().string;
            m_token_stream.consume(":", true);
            const ci_string attribute_class = m_token_stream.consume().string;
            m_token_stream.consume("is", true);
            ci_string attribute_value = m_token_stream.join_until(";", " ").string;
            m_token_stream.consume(";", true);
            ci_string attribute_type;

            if (attribute_value[0] == '\"' && attribute_value.back() == '\"')
            {
                attribute_value = attribute_value.substr(1, attribute_value.size() - 2);
            }

            if (const auto type_it = m_attribute_types.find(attribute_name); type_it == m_attribute_types.end())
            {
                log_warning("vhdl_parser", "attribute {} has unknown base type in line {}.", attribute_name, line_number);
                attribute_type = "unknown";
            }
            else
            {
                attribute_type = type_it->second;
            }

            if (attribute_class == "entity")
            {
                target_class = AttributeTarget::ENTITY;
            }
            else if (attribute_class == "label")
            {
                target_class = AttributeTarget::INSTANCE;
            }
            else if (attribute_class == "signal")
            {
                target_class = AttributeTarget::SIGNAL;
            }
            else
            {
                log_warning("vhdl_parser", "unsupported attribute class '{}' in line {}, ignoring attribute.", attribute_class, line_number);
                return true;
            }

            m_attribute_buffer[target_class].emplace(attribute_target,
                                                     std::make_tuple(line_number,
                                                                     core_strings::convert_string<ci_string, std::string>(attribute_name),
                                                                     core_strings::convert_string<ci_string, std::string>(attribute_type),
                                                                     core_strings::convert_string<ci_string, std::string>(attribute_value)));
        }
        else
        {
            log_error("hdl_parser", "malformed attribute defintion in line {}.", line_number);
            return false;
        }

        return true;
    }

    bool VHDLParser::parse_architecture()
    {
        m_token_stream.consume("architecture", true);
        m_token_stream.consume();
        m_token_stream.consume("of", true);

        const ci_string entity_name = m_token_stream.consume().string;

        if (const auto it = m_entities.find(entity_name); it == m_entities.end())
        {
            log_error("vhdl_parser", "architecture refers to entity '{}', but no such entity exists.", entity_name);
            return false;
        }
        else
        {
            VHDLEntity& entity = it->second;

            m_token_stream.consume("is", true);

            return parse_architecture_header(entity) && parse_architecture_body(entity) && assign_attributes(entity);
        }
    }

    bool VHDLParser::parse_architecture_header(VHDLEntity& entity)
    {
        auto next_token = m_token_stream.peek();
        while (next_token != "begin")
        {
            if (next_token == "signal")
            {
                if (!parse_signal_definition(entity))
                {
                    return false;
                }
            }
            else if (next_token == "component")
            {
                // components are ignored
                m_token_stream.consume("component", true);
                const ci_string component_name = m_token_stream.consume().string;
                m_token_stream.consume_until("end");
                m_token_stream.consume("end", true);
                m_token_stream.consume("component", true);
                if (m_token_stream.peek() != ";")
                {
                    m_token_stream.consume(component_name, true);    // optional repetition of component name
                }
                m_token_stream.consume(";", true);
            }
            else if (next_token == "attribute")
            {
                if (!parse_attribute())
                {
                    return false;
                }
            }
            else
            {
                log_error("vhdl_parser", "unexpected token '{}' in architecture header in line {}.", next_token.string, next_token.number);
                return false;
            }

            next_token = m_token_stream.peek();
        }

        return true;
    }

    bool VHDLParser::parse_signal_definition(VHDLEntity& entity)
    {
        m_token_stream.consume("signal", true);

        // extract names
        std::vector<ci_string> signal_names;
        do
        {
            signal_names.push_back(m_token_stream.consume().string);
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(":", true);

        // extract bounds
        TokenStream<ci_string> signal_stream = m_token_stream.extract_until(";");
        std::vector<std::vector<u32>> ranges;
        if (auto ranges_opt = parse_signal_ranges(signal_stream); !ranges_opt.has_value())
        {
            // error already printed in subfunction
            return false;
        }
        else
        {
            ranges = ranges_opt.value();
        }

        m_token_stream.consume(";", true);

        for (const auto& signal_name : signal_names)
        {
            entity.m_signals.push_back(signal_name);
            entity.m_signals_set.insert(signal_name);
            if (!ranges.empty())
            {
                entity.m_signal_ranges[signal_name] = ranges;
            }
        }

        return true;
    }

    bool VHDLParser::parse_architecture_body(VHDLEntity& entity)
    {
        m_token_stream.consume("begin", true);

        while (m_token_stream.peek() != "end")
        {
            // new instance found
            if (m_token_stream.peek(1) == ":")
            {
                if (!parse_instance(entity))
                {
                    return false;
                }
            }
            // not in instance -> has to be a direct assignment
            else if (m_token_stream.find_next("<=") < m_token_stream.find_next(";"))
            {
                if (!parse_assignment(entity))
                {
                    return false;
                }
            }
            else
            {
                log_error("vhdl_parser", "unexpected token '{}' in architecture body in line {}.", m_token_stream.peek().string, m_token_stream.peek().number);
                return false;
            }
        }

        m_token_stream.consume("end", true);
        m_token_stream.consume();
        m_token_stream.consume(";", true);

        return true;
    }

    bool VHDLParser::parse_assignment(VHDLEntity& entity)
    {
        TokenStream<ci_string> left_stream = m_token_stream.extract_until("<=");
        m_token_stream.consume("<=", true);
        TokenStream<ci_string> right_stream = m_token_stream.extract_until(";");
        m_token_stream.consume(";", true);

        entity.m_assignments.push_back(std::make_pair(left_stream, right_stream));

        return true;
    }

    bool VHDLParser::parse_instance(VHDLEntity& entity)
    {
        const u32 line_number         = m_token_stream.peek().number;
        const ci_string instance_name = m_token_stream.consume();
        ci_string instance_type;
        m_token_stream.consume(":", true);

        // remove prefix from type
        if (m_token_stream.peek() == "entity")
        {
            m_token_stream.consume("entity", true);
            instance_type = m_token_stream.consume();
            if (const size_t pos = instance_type.find('.'); pos != std::string::npos)
            {
                instance_type = instance_type.substr(pos + 1);
            }
            if (m_entities.find(instance_type) == m_entities.end())
            {
                log_error("vhdl_parser", "trying to instantiate unknown entity '{}' in line {}.", instance_type, line_number);
                return false;
            }
        }
        else if (m_token_stream.peek() == "component")
        {
            m_token_stream.consume("component", true);
            instance_type = m_token_stream.consume();
        }
        else
        {
            instance_type = m_token_stream.consume();
            ci_string prefix;

            // find longest matching library prefix
            for (const auto& lib : m_libraries)
            {
                if (lib.size() > prefix.size() && utils::starts_with(instance_type, lib))
                {
                    prefix = lib;
                }
            }

            // remove prefix
            if (!prefix.empty())
            {
                instance_type = instance_type.substr(prefix.size());
            }
        }

        entity.m_instances.push_back(instance_name);
        entity.m_instances_set.insert(instance_name);
        entity.m_instance_types[instance_name] = instance_type;

        if (m_token_stream.consume("generic"))
        {
            if (!parse_generic_assign(entity, instance_name))
            {
                return false;
            }
        }

        if (m_token_stream.peek() == "port")
        {
            if (!parse_port_assign(entity, instance_name))
            {
                return false;
            }
        }

        m_token_stream.consume(";", true);

        return true;
    }

    bool VHDLParser::parse_port_assign(VHDLEntity& entity, const ci_string& instance_name)
    {
        m_token_stream.consume("port", true);
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        TokenStream<ci_string> port_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (port_stream.remaining() > 0)
        {
            TokenStream<ci_string> left_stream = port_stream.extract_until("=>");
            port_stream.consume("=>", true);
            TokenStream<ci_string> right_stream = port_stream.extract_until(",");
            port_stream.consume(",", port_stream.remaining() > 0);    // last entry has no comma

            if (!right_stream.consume("open"))
            {
                entity.m_instance_assignments[instance_name].push_back(std::make_pair(left_stream, right_stream));
            }
        }

        return true;
    }

    bool VHDLParser::parse_generic_assign(VHDLEntity& entity, const ci_string& instance_name)
    {
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        TokenStream<ci_string> generic_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (generic_stream.remaining() > 0)
        {
            std::string value, data_type;

            const u32 line_number = generic_stream.peek().number;
            const std::string lhs = core_strings::convert_string<ci_string, std::string>(generic_stream.join_until("=>", "").string);
            generic_stream.consume("=>", true);
            const Token<ci_string> rhs = generic_stream.join_until(",", "");
            generic_stream.consume(",", generic_stream.remaining() > 0);    // last entry has no comma

            // determine data type
            if ((rhs == "true") || (rhs == "false"))
            {
                value     = core_strings::convert_string<ci_string, std::string>(rhs);
                data_type = "boolean";
            }
            else if (utils::is_integer(rhs.string))
            {
                value     = core_strings::convert_string<ci_string, std::string>(rhs);
                data_type = "integer";
            }
            else if (utils::is_floating_point(rhs.string))
            {
                value     = core_strings::convert_string<ci_string, std::string>(rhs);
                data_type = "floating_point";
            }
            else if (utils::ends_with(rhs.string, ci_string("s")) || utils::ends_with(rhs.string, ci_string("sec")) || utils::ends_with(rhs.string, ci_string("min"))
                     || utils::ends_with(rhs.string, ci_string("hr")))
            {
                value     = core_strings::convert_string<ci_string, std::string>(rhs);
                data_type = "time";
            }
            else if (rhs.string.at(0) == '\"' && rhs.string.back() == '\"')
            {
                value     = core_strings::convert_string<ci_string, std::string>(rhs.string.substr(1, rhs.string.size() - 2));
                data_type = "string";
            }
            else if (rhs.string.at(0) == '\'' && rhs.string.at(2) == '\'')
            {
                value     = core_strings::convert_string<ci_string, std::string>(rhs.string.substr(1, 1));
                data_type = "bit_value";
            }
            else if (rhs.string.at(1) == '\"' && rhs.string.back() == '\"')
            {
                value = core_strings::convert_string<ci_string, std::string>(get_hex_from_literal(rhs));
                if (value.empty())
                {
                    return false;
                }

                data_type = "bit_vector";
            }
            else
            {
                log_error("vhdl_parser", "cannot identify data type of generic map value '{}' in instance '{}' in line {}.", rhs.string, instance_name, line_number);
                return false;
            }
            entity.m_instance_generic_assignments[instance_name].push_back(std::make_tuple(lhs, data_type, value));
        }

        return true;
    }

    bool VHDLParser::assign_attributes(VHDLEntity& entity)
    {
        for (const auto& [target_class, attributes] : m_attribute_buffer)
        {
            // entity attributes
            if (target_class == AttributeTarget::ENTITY)
            {
                for (const auto& [target, attribute] : attributes)
                {
                    if (entity.m_name != target)
                    {
                        log_error("vhdl_parser", "invalid attribute target '{}' within entity '{}' in line {}.", target, entity.m_name, std::get<0>(attribute));
                        return false;
                    }
                    else
                    {
                        entity.m_attributes.push_back(std::make_tuple(std::get<1>(attribute), std::get<2>(attribute), std::get<3>(attribute)));
                    }
                }
            }
            // instance attributes
            else if (target_class == AttributeTarget::INSTANCE)
            {
                for (const auto& [target, attribute] : attributes)
                {
                    if (const auto instance_it = entity.m_instances_set.find(target); instance_it == entity.m_instances_set.end())
                    {
                        log_error("vhdl_parser", "invalid attribute target '{}' within entity '{}' in line {}.", target, entity.m_name, std::get<0>(attribute));
                        return false;
                    }
                    else
                    {
                        entity.m_instance_attributes[*instance_it].push_back(std::make_tuple(std::get<1>(attribute), std::get<2>(attribute), std::get<3>(attribute)));
                    }
                }
            }
            // signal attributes
            else if (target_class == AttributeTarget::SIGNAL)
            {
                for (const auto& [target, attribute] : attributes)
                {
                    if (const auto signal_it = entity.m_signals_set.find(target); signal_it != entity.m_signals_set.end())
                    {
                        entity.m_signal_attributes[*signal_it].push_back(std::make_tuple(std::get<1>(attribute), std::get<2>(attribute), std::get<3>(attribute)));
                    }
                    else if (const auto port_it = entity.m_port_names_set.find(target); port_it != entity.m_port_names_set.end())
                    {
                        entity.m_port_attributes[*port_it].push_back(std::make_tuple(std::get<1>(attribute), std::get<2>(attribute), std::get<3>(attribute)));
                    }
                    else
                    {
                        log_error("vhdl_parser", "invalid attribute target '{}' within entity '{}' in line {}.", target, entity.m_name, std::get<0>(attribute));
                        return false;
                    }
                }
            }
        }

        return true;
    }

    // ###########################################################################
    // ###########      Assemble Netlist from Intermediate Format       ##########
    // ###########################################################################

    bool VHDLParser::construct_netlist(VHDLEntity& top_entity)
    {
        m_netlist->set_design_name(core_strings::convert_string<ci_string, std::string>(top_entity.m_name));

        std::unordered_map<ci_string, u32> instantiation_count;

        // preparations for alias: count the occurences of all names
        std::queue<VHDLEntity*> q;
        q.push(&top_entity);

        // top entity instance will be named after its entity, so take into account for aliases
        m_instance_name_occurrences["top_module"]++;

        // global input/output signals will be named after ports, so take into account for aliases
        for (const ci_string& port_name : top_entity.m_port_names)
        {
            m_signal_name_occurrences[port_name]++;
        }

        while (!q.empty())
        {
            VHDLEntity* entity = q.front();
            q.pop();

            instantiation_count[entity->m_name]++;

            for (const ci_string& signal_identifier : entity->m_signals)
            {
                m_signal_name_occurrences[signal_identifier]++;
            }

            for (const ci_string& instance_identifier : entity->m_instances)
            {
                m_instance_name_occurrences[instance_identifier]++;

                if (const auto it = m_entities.find(entity->m_instance_types.at(instance_identifier)); it != m_entities.end())
                {
                    q.push(&(it->second));
                }
            }
        }

        for (auto& [entity_name, entity] : m_entities)
        {
            // detect unused entities
            if (instantiation_count[entity_name] == 0)
            {
                log_warning("vhdl_parser", "module '{}' has been defined in the netlist but is not instantiated.", entity_name);
                continue;
            }

            // clear gate assignments for every initialization attempt
            entity.m_expanded_gate_assignments.clear();

            // expand gate pin assignments
            for (const ci_string& instance_name : entity.m_instances)
            {
                const ci_string& instance_type = entity.m_instance_types.at(instance_name);
                if (const auto gate_type_it = m_gate_types.find(instance_type); gate_type_it != m_gate_types.end())
                {
                    // cache pin groups
                    std::unordered_map<ci_string, std::vector<ci_string>> pin_groups;
                    for (const auto& [group_name, pins] : gate_type_it->second->get_pin_groups())
                    {
                        for (const auto& pin : pins)
                        {
                            pin_groups[core_strings::convert_string<std::string, ci_string>(group_name)].push_back(core_strings::convert_string<std::string, ci_string>(pin.second));
                        }
                    }

                    if (auto inst_it = entity.m_instance_assignments.find(instance_name); inst_it != entity.m_instance_assignments.end())
                    {
                        for (auto [pin_stream, assignment_stream] : inst_it->second)
                        {
                            ci_string pin_name = pin_stream.consume().string;
                            std::vector<ci_string> left_port;
                            if (const auto group_it = pin_groups.find(pin_name); group_it != pin_groups.end())
                            {
                                if (pin_stream.consume("("))
                                {
                                    std::vector<std::vector<u32>> ranges;
                                    u32 closing_pos = pin_stream.find_next(")");
                                    do
                                    {
                                        TokenStream<ci_string> range_stream = pin_stream.extract_until(",", closing_pos);
                                        ranges.emplace_back(parse_range(range_stream));

                                    } while (pin_stream.consume(",", false));
                                    pin_stream.consume(")", true);

                                    left_port = expand_ranges(pin_name, ranges);
                                }
                                else
                                {
                                    left_port = group_it->second;
                                }
                            }
                            else
                            {
                                left_port.push_back(pin_name);
                            }
                            const std::vector<ci_string> right_port = expand_assignment_signal(entity, assignment_stream, false);

                            if (right_port.size() != left_port.size())
                            {
                                log_error("vhdl_parser", "pin assignment width mismatch at instance '{}' of gate type '{}' within entity '{}'.", instance_name, instance_type, entity_name);
                                return false;
                            }

                            for (u32 i = 0; i < left_port.size(); i++)
                            {
                                entity.m_expanded_gate_assignments[instance_name].push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                            }
                        }
                    }
                }
            }
        }

        // for the top module, generate global i/o signals for all ports
        std::unordered_map<ci_string, ci_string> top_assignments;
        for (const auto& [port_identifier, expanded_port_identifiers] : top_entity.m_expanded_port_names)
        {
            PinDirection direction = top_entity.m_port_directions.at(port_identifier);

            for (const ci_string& expanded_identifier : expanded_port_identifiers)
            {
                Net* global_port_net = m_netlist->create_net(core_strings::convert_string<ci_string, std::string>(expanded_identifier));
                if (global_port_net == nullptr)
                {
                    log_error("vhdl_parser", "could not create global port net '{}'.", expanded_identifier);
                    return false;
                }

                m_net_by_name[expanded_identifier] = global_port_net;

                // assign global port nets to ports of top module
                top_assignments[expanded_identifier] = expanded_identifier;

                if (direction == PinDirection::input || direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_input_net())
                    {
                        log_error("vhdl_parser", "could not mark global port net '{}' as global input.", expanded_identifier);
                        return false;
                    }
                }

                if (direction == PinDirection::output || direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_output_net())
                    {
                        log_error("vhdl_parser", "could not mark global port net '{}' as global output.", expanded_identifier);
                        return false;
                    }
                }
            }
        }

        if (!instantiate_entity("top_module", top_entity, nullptr, top_assignments))
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
                            log_warning("vhdl_parser",
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
                log_error("vhdl_parser", "cyclic dependency between signals detected, cannot parse netlist.");
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

    Module* VHDLParser::instantiate_entity(const ci_string& instance_identifier, VHDLEntity& vhdl_entity, Module* parent, const std::unordered_map<ci_string, ci_string>& parent_module_assignments)
    {
        std::unordered_map<ci_string, ci_string> signal_alias;
        std::unordered_map<ci_string, ci_string> instance_alias;

        // TODO check parent module assignments for port aliases

        instance_alias[instance_identifier] = get_unique_alias(m_instance_name_occurrences, instance_identifier);

        // create netlist module
        Module* module;
        if (parent == nullptr)
        {
            module = m_netlist->get_top_module();
            module->set_name(core_strings::convert_string<ci_string, std::string>(instance_alias.at(instance_identifier)));
        }
        else
        {
            module = m_netlist->create_module(core_strings::convert_string<ci_string, std::string>(instance_alias.at(instance_identifier)), parent);
        }

        ci_string instance_type = vhdl_entity.m_name;
        if (module == nullptr)
        {
            log_error("vhdl_parser", "could not instantiate instance '{}' of module '{}'.", instance_identifier, instance_type);
            return nullptr;
        }
        module->set_type(core_strings::convert_string<ci_string, std::string>(instance_type));

        // assign entity-level attributes
        for (const std::tuple<std::string, std::string, std::string>& attribute : vhdl_entity.m_attributes)
        {
            if (!module->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
            {
                log_warning("vhdl_parser",
                            "could not set attribute ({}, {}, {}) for instance '{}' type '{}'.",
                            std::get<0>(attribute),
                            std::get<1>(attribute),
                            std::get<2>(attribute),
                            instance_identifier,
                            instance_type);
            }
        }

        // assign module port names and attributes
        for (const auto& [port_identifier, expanded_port_identifiers] : vhdl_entity.m_expanded_port_names)
        {
            PinDirection direction = vhdl_entity.m_port_directions.at(port_identifier);

            for (const ci_string& expanded_identifier : expanded_port_identifiers)
            {
                if (const auto it = parent_module_assignments.find(expanded_identifier); it != parent_module_assignments.end())
                {
                    Net* port_net            = m_net_by_name.at(it->second);
                    m_module_ports[port_net] = std::make_tuple(direction, core_strings::convert_string<ci_string, std::string>(expanded_identifier), module);

                    // assign port attributes
                    if (const auto port_attr_it = vhdl_entity.m_port_attributes.find(port_identifier); port_attr_it != vhdl_entity.m_port_attributes.end())
                    {
                        for (const std::tuple<std::string, std::string, std::string>& attribute : port_attr_it->second)
                        {
                            if (!port_net->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
                            {
                                log_warning("vhdl_parser",
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
        for (const auto& [signal_identifier, expanded_signal_identifiers] : vhdl_entity.m_expanded_signals)
        {
            for (const ci_string& expanded_identifier : expanded_signal_identifiers)
            {
                signal_alias[expanded_identifier] = get_unique_alias(m_signal_name_occurrences, expanded_identifier);

                // create new net for the signal
                Net* signal_net = m_netlist->create_net(core_strings::convert_string<ci_string, std::string>(signal_alias.at(expanded_identifier)));
                if (signal_net == nullptr)
                {
                    log_error(
                        "vhdl_parser", "could not instantiate net '{}' of instance '{}' of type '{}'.", expanded_identifier, instance_identifier, instance_type, instance_identifier, instance_type);
                    return nullptr;
                }

                m_net_by_name[signal_alias.at(expanded_identifier)] = signal_net;

                // assign signal attributes
                if (const auto signal_attr_it = vhdl_entity.m_signal_attributes.find(signal_identifier); signal_attr_it != vhdl_entity.m_port_attributes.end())
                {
                    for (const std::tuple<std::string, std::string, std::string>& attribute : signal_attr_it->second)
                    {
                        if (!signal_net->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
                        {
                            log_warning("vhdl_parser",
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
        for (const auto& [left_expanded_signal, right_expanded_signal] : vhdl_entity.m_expanded_assignments)
        {
            ci_string a = left_expanded_signal;
            ci_string b = right_expanded_signal;

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
                log_error("vhdl_parser", "cannot find alias for net '{}' of instance '{}' of type '{}'.", a, instance_identifier, instance_type);
                return nullptr;
            }

            if (const auto parent_it = parent_module_assignments.find(b); parent_it != parent_module_assignments.end())
            {
                b = parent_it->second;
            }
            else if (const auto alias_it = signal_alias.find(b); alias_it != signal_alias.end())
            {
                b = alias_it->second;
            }
            else if (b == "'Z'" || b == "'X'")
            {
                continue;
            }
            else if (b != "'0'" && b != "'1'")
            {
                log_error("vhdl_parser", "cannot find alias for net '{}' of instance '{}' of type '{}'.", b, instance_identifier, instance_type);
                return nullptr;
            }

            m_nets_to_merge[b].push_back(a);
        }

        // process instances i.e. gates or other entities
        for (const ci_string& inst_identifier : vhdl_entity.m_instances)
        {
            const ci_string& inst_type = vhdl_entity.m_instance_types.at(inst_identifier);

            // will later hold either module or gate, so attributes can be assigned properly
            DataContainer* container;

            // assign actual signal names to ports
            std::unordered_map<ci_string, ci_string> instance_assignments;

            // if the instance is another entity, recursively instantiate it
            if (auto entity_it = m_entities.find(inst_type); entity_it != m_entities.end())
            {
                if (const auto inst_it = vhdl_entity.m_expanded_entity_assignments.find(inst_identifier); inst_it != vhdl_entity.m_expanded_entity_assignments.end())
                {
                    // expand port assignments
                    for (const auto& [port, assignment] : inst_it->second)
                    {
                        if (const auto it = parent_module_assignments.find(assignment); it != parent_module_assignments.end())
                        {
                            instance_assignments[port] = it->second;
                        }
                        else
                        {
                            if (const auto alias_it = signal_alias.find(assignment); alias_it != signal_alias.end())
                            {
                                instance_assignments[port] = alias_it->second;
                            }
                            else if (assignment == "'0'" || assignment == "'1'")
                            {
                                instance_assignments[port] = assignment;
                            }
                            else if (assignment != "'Z'" && assignment != "'X'")
                            {
                                log_error("vhdl_parser", "port assignment \"{} = {}\" is invalid for instance '{}' of type '{}'", port, assignment, inst_identifier, inst_type);
                                return nullptr;
                            }
                        }
                    }
                }

                container = instantiate_entity(inst_identifier, entity_it->second, module, instance_assignments);
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

                Gate* new_gate = m_netlist->create_gate(gate_type_it->second, core_strings::convert_string<ci_string, std::string>(instance_alias.at(inst_identifier)));
                if (new_gate == nullptr)
                {
                    log_error("vhdl_parser", "could not instantiate gate '{}' within instance '{}' of type '{}'.", inst_identifier, instance_identifier, instance_type);
                    return nullptr;
                }

                if (!module->is_top_module())
                {
                    module->assign_gate(new_gate);
                }

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

                if (const auto inst_it = vhdl_entity.m_expanded_gate_assignments.find(inst_identifier); inst_it != vhdl_entity.m_expanded_gate_assignments.end())
                {
                    // cache pin directions
                    std::unordered_map<ci_string, PinDirection> pin_to_direction;
                    for (const auto& [pin, direction] : gate_type_it->second->get_pin_directions())
                    {
                        pin_to_direction[core_strings::convert_string<std::string, ci_string>(pin)] = direction;
                    }

                    // expand pin assignments
                    for (const auto& [pin, assignment] : inst_it->second)
                    {
                        ci_string signal;
                        if (const auto parent_it = parent_module_assignments.find(assignment); parent_it != parent_module_assignments.end())
                        {
                            signal = parent_it->second;
                        }
                        else if (const auto alias_it = signal_alias.find(assignment); alias_it != signal_alias.end())
                        {
                            signal = alias_it->second;
                        }
                        else if (assignment == "'0'" || assignment == "'1'")
                        {
                            signal = assignment;
                        }
                        else if (assignment == "'Z'" || assignment == "'X'")
                        {
                            continue;
                        }
                        else
                        {
                            log_error("vhdl_parser", "pin assignment \"{} = {}\" is invalid for instance '{}' of type '{}'.", pin, assignment, inst_identifier, inst_type);
                            return nullptr;
                        }

                        // get the respective net for the assignment
                        if (const auto net_it = m_net_by_name.find(signal); net_it == m_net_by_name.end())
                        {
                            log_error("vhdl_parser", "signal '{}' of instance '{}' of type '{}' has not been declared.", signal, inst_identifier, inst_type);
                            return nullptr;
                        }
                        else
                        {
                            Net* current_net = net_it->second;

                            // add net src/dst by pin types
                            bool is_input  = false;
                            bool is_output = false;

                            if (const auto it = pin_to_direction.find(pin); it != pin_to_direction.end())
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
                                log_error("vhdl_parser", "undefined pin '{}' for gate '{}' of type '{}'.", pin, new_gate->get_name(), new_gate->get_type()->get_name());
                                return nullptr;
                            }

                            if (is_output && !current_net->add_source(new_gate, core_strings::convert_string<ci_string, std::string>(pin)))
                            {
                                return nullptr;
                            }

                            if (is_input && !current_net->add_destination(new_gate, core_strings::convert_string<ci_string, std::string>(pin)))
                            {
                                return nullptr;
                            }
                        }
                    }
                }
            }
            else
            {
                log_error("vhdl_parser", "could not find gate type '{}' in gate library '{}'.", inst_type, m_netlist->get_gate_library()->get_name());
                return nullptr;
            }

            // assign instance attributes
            if (const auto& attr_it = vhdl_entity.m_instance_attributes.find(inst_identifier); attr_it != vhdl_entity.m_instance_attributes.end())
            {
                for (const auto& attribute : attr_it->second)
                {
                    if (!container->set_data("attribute", std::get<0>(attribute), std::get<1>(attribute), std::get<2>(attribute)))
                    {
                        log_warning("vhdl_parser",
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
            }

            // process generics
            if (const auto& gen_it = vhdl_entity.m_instance_generic_assignments.find(inst_identifier); gen_it != vhdl_entity.m_instance_generic_assignments.end())
            {
                for (const auto& generic : gen_it->second)
                {
                    if (!container->set_data("generic", std::get<0>(generic), std::get<1>(generic), std::get<2>(generic)))
                    {
                        log_warning("vhdl_parser",
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
        }

        return module;
    }

    // ###########################################################################
    // ###################          Helper Functions          ####################
    // ###########################################################################

    std::vector<u32> VHDLParser::parse_range(TokenStream<ci_string>& range_stream) const
    {
        if (range_stream.remaining() == 1)
        {
            return {(u32)std::stoi(core_strings::convert_string<ci_string, std::string>(range_stream.consume().string))};
        }

        int direction = 1;
        const int end = std::stoi(core_strings::convert_string<ci_string, std::string>(range_stream.consume().string));

        if (range_stream.peek() == "downto")
        {
            range_stream.consume("downto");
        }
        else
        {
            range_stream.consume("to", true);
            direction = -1;
        }

        const int start = std::stoi(core_strings::convert_string<ci_string, std::string>(range_stream.consume().string));

        std::vector<u32> res;
        for (int i = start; i != end + direction; i += direction)
        {
            res.push_back((u32)i);
        }
        return res;
    }

    const static std::map<core_strings::CaseInsensitiveString, size_t> id_to_dim = {{"std_logic_vector", 1}, {"std_logic_vector2", 2}, {"std_logic_vector3", 3}};

    std::optional<std::vector<std::vector<u32>>> VHDLParser::parse_signal_ranges(TokenStream<ci_string>& signal_stream) const
    {
        std::vector<std::vector<u32>> ranges;
        const u32 line_number = signal_stream.peek().number;

        const Token<ci_string> type_name = signal_stream.consume();
        if (type_name == "std_logic")
        {
            return ranges;
        }

        signal_stream.consume("(", true);
        TokenStream<ci_string> signal_bounds_stream = signal_stream.extract_until(")");

        // process ranges
        do
        {
            TokenStream<ci_string> bound_stream = signal_bounds_stream.extract_until(",");
            ranges.emplace_back(parse_range(bound_stream));
        } while (signal_bounds_stream.consume(","));

        signal_stream.consume(")", true);

        if (id_to_dim.find(type_name) != id_to_dim.end())
        {
            const size_t dimension = id_to_dim.at(type_name);

            if (ranges.size() != dimension)
            {
                log_error("vhdl_parser", "dimension-bound mismatch in line {} : expected {}, got {}", line_number, dimension, ranges.size());
                return std::nullopt;
            }
        }
        else
        {
            log_error("vhdl_parser", "type name {} is invalid in line {}", type_name.string, line_number);
            return std::nullopt;
        }

        return ranges;
    }

    std::vector<VHDLParser::ci_string> VHDLParser::expand_assignment_signal(VHDLEntity& entity, TokenStream<ci_string>& signal_stream, bool is_left)
    {
        // PARSE ASSIGNMENT
        //   assignment can currently be one of the following:
        //   (1) NAME
        //   (2) NUMBER
        //   (3) NAME(INDEX1, INDEX2, ...)
        //   (4) NAME(BEGIN_INDEX1 to/downto END_INDEX1, BEGIN_INDEX2 to/downto END_INDEX2, ...)
        //   (5) ((1 - 4), (1 - 4), ...)

        std::vector<ci_string> result;
        std::vector<TokenStream<ci_string>> parts;

        // (5) ((1 - 4), (1 - 4), ...)
        if (!is_left)
        {
            if (signal_stream.consume("("))
            {
                do
                {
                    parts.push_back(signal_stream.extract_until(","));
                } while (signal_stream.consume(",", false));

                signal_stream.consume(")", true);
            }
            else
            {
                parts.push_back(signal_stream);
            }
        }
        else
        {
            if (signal_stream.find_next(",") != TokenStream<ci_string>::END_OF_STREAM)
            {
                log_error("vhdl_parser", "aggregation is not allowed at this position in line {}.", signal_stream.peek().number);
                return {};
            }
            parts.push_back(signal_stream);
        }

        for (auto it = parts.rbegin(); it != parts.rend(); it++)
        {
            TokenStream<ci_string>& part_stream = *it;

            const Token<ci_string> signal_name_token = part_stream.consume();
            const u32 line_number                    = signal_name_token.number;
            ci_string signal_name                    = signal_name_token.string;

            // (2) NUMBER
            if (utils::starts_with(signal_name, core_strings::CaseInsensitiveString("\"")) || utils::starts_with(signal_name, core_strings::CaseInsensitiveString("b\""))
                || utils::starts_with(signal_name, core_strings::CaseInsensitiveString("o\"")) || utils::starts_with(signal_name, core_strings::CaseInsensitiveString("x\"")))
            {
                if (is_left)
                {
                    log_error("vhdl_parser", "numeric value {} not allowed at this position in line {}.", signal_name, line_number);
                    return {};
                }

                std::vector<ci_string> binary_vector = get_bin_from_literal(signal_name_token);
                if (binary_vector.empty())
                {
                    return {};
                }
                result.insert(result.end(), binary_vector.begin(), binary_vector.end());
            }
            else if (signal_name == "'0'" || signal_name == "'1'" || signal_name == "'Z'" || signal_name == "'X'")
            {
                if (is_left)
                {
                    log_error("vhdl_parser", "numeric value {} not allowed at this position in line {}.", signal_name, line_number);
                    return {};
                }

                result.push_back(signal_name);
            }
            else
            {
                std::vector<std::vector<u32>> ranges;

                // (3) NAME(INDEX1, INDEX2, ...)
                // (4) NAME(BEGIN_INDEX1 to/downto END_INDEX1, BEGIN_INDEX2 to/downto END_INDEX2, ...)
                if (part_stream.consume("("))
                {
                    u32 closing_pos = part_stream.find_next(")");
                    do
                    {
                        TokenStream<ci_string> range_stream = part_stream.extract_until(",", closing_pos);
                        ranges.emplace_back(parse_range(range_stream));

                    } while (part_stream.consume(",", false));
                    part_stream.consume(")", true);
                }
                else
                {
                    // (1) NAME
                    std::vector<std::vector<u32>> reference_ranges;
                    if (const auto signal_ranges_it = entity.m_signal_ranges.find(signal_name); signal_ranges_it != entity.m_signal_ranges.end())
                    {
                        reference_ranges = signal_ranges_it->second;
                    }
                    else if (const auto port_ranges_it = entity.m_port_ranges.find(signal_name); port_ranges_it != entity.m_port_ranges.end())
                    {
                        reference_ranges = port_ranges_it->second;
                    }

                    ranges = reference_ranges;
                }

                if (!ranges.empty())
                {
                    std::vector<ci_string> expanded_signal = expand_ranges(signal_name, ranges);
                    result.insert(result.end(), expanded_signal.begin(), expanded_signal.end());
                }
                else
                {
                    result.push_back(signal_name);
                }
            }
        }

        return result;
    }

    static const std::map<char, std::vector<core_strings::CaseInsensitiveString>> oct_to_bin = {{'0', {"'0'", "'0'", "'0'"}},
                                                                                                {'1', {"'1'", "'0'", "'0'"}},
                                                                                                {'2', {"'0'", "'1'", "'0'"}},
                                                                                                {'3', {"'1'", "'1'", "'0'"}},
                                                                                                {'4', {"'0'", "'0'", "'1'"}},
                                                                                                {'5', {"'1'", "'0'", "'1'"}},
                                                                                                {'6', {"'0'", "'1'", "'1'"}},
                                                                                                {'7', {"'1'", "'1'", "'1'"}}};
    static const std::map<char, std::vector<core_strings::CaseInsensitiveString>> hex_to_bin = {{'0', {"'0'", "'0'", "'0'", "'0'"}},
                                                                                                {'1', {"'1'", "'0'", "'0'", "'0'"}},
                                                                                                {'2', {"'0'", "'1'", "'0'", "'0'"}},
                                                                                                {'3', {"'1'", "'1'", "'0'", "'0'"}},
                                                                                                {'4', {"'0'", "'0'", "'1'", "'0'"}},
                                                                                                {'5', {"'1'", "'0'", "'1'", "'0'"}},
                                                                                                {'6', {"'0'", "'1'", "'1'", "'0'"}},
                                                                                                {'7', {"'1'", "'1'", "'1'", "'0'"}},
                                                                                                {'8', {"'0'", "'0'", "'0'", "'1'"}},
                                                                                                {'9', {"'1'", "'0'", "'0'", "'1'"}},
                                                                                                {'A', {"'0'", "'1'", "'0'", "'1'"}},
                                                                                                {'B', {"'1'", "'1'", "'0'", "'1'"}},
                                                                                                {'C', {"'0'", "'0'", "'1'", "'1'"}},
                                                                                                {'D', {"'1'", "'0'", "'1'", "'1'"}},
                                                                                                {'E', {"'0'", "'1'", "'1'", "'1'"}},
                                                                                                {'F', {"'1'", "'1'", "'1'", "'1'"}}};

    std::vector<VHDLParser::ci_string> VHDLParser::get_bin_from_literal(const Token<ci_string>& value_token) const
    {
        const u32 line_number = value_token.number;
        const ci_string value = utils::to_upper(utils::replace(value_token.string, ci_string("_"), ci_string("")));

        char prefix;
        ci_string number;
        std::vector<ci_string> result;

        if (value.at(0) != '\"')
        {
            prefix = value.at(0);
            number = value.substr(2, value.rfind('\"') - 2);
        }
        else
        {
            prefix = 'B';
            number = value.substr(1, value.rfind('\"') - 1);
        }

        // parse number literal
        switch (prefix)
        {
            case 'B': {
                for (auto it = number.rbegin(); it != number.rend(); it++)
                {
                    const char c = *it;
                    if (c == '0' || c == '1' || c == 'Z' || c == 'X')
                    {
                        result.push_back("'" + ci_string(1, c) + "'");
                    }
                    else
                    {
                        log_error("vhdl_parser", "invalid character within binary number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }
                break;
            }

            case 'O': {
                for (auto it = number.rbegin(); it != number.rend(); it++)
                {
                    const char c = *it;
                    if (c >= '0' && c <= '7')
                    {
                        const std::vector<ci_string>& bits = oct_to_bin.at(c);
                        result.insert(result.end(), bits.begin(), bits.end());
                    }
                    else
                    {
                        log_error("vhdl_parser", "invalid character within octal number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }
                break;
            }

            case 'D': {
                u64 tmp_val = 0;

                for (const auto& c : number)
                {
                    if (c >= '0' && c <= '9')
                    {
                        tmp_val = (tmp_val * 10) + (c - '0');
                    }
                    else
                    {
                        log_error("vhdl_parser", "invalid character within octal number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }

                do
                {
                    result.push_back(((tmp_val & 1) == 1) ? "'1'" : "'0'");
                    tmp_val >>= 1;
                } while (tmp_val != 0);
                break;
            }

            case 'X': {
                for (auto it = number.rbegin(); it != number.rend(); it++)
                {
                    const char c = *it;
                    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))
                    {
                        const std::vector<ci_string>& bits = hex_to_bin.at(c);
                        result.insert(result.end(), bits.begin(), bits.end());
                    }
                    else
                    {
                        log_error("vhdl_parser", "invalid character within hexadecimal number literal {} in line {}.", value, line_number);
                        return {};
                    }
                }
                break;
            }

            default: {
                log_error("vhdl_parser", "invalid base '{}' within number literal {} in line {}.", prefix, value, line_number);
                return {};
            }
        }

        return result;
    }

    VHDLParser::ci_string VHDLParser::get_hex_from_literal(const Token<ci_string>& value_token) const
    {
        const u32 line_number = value_token.number;
        const ci_string value = utils::to_upper(utils::replace(value_token.string, ci_string("_"), ci_string("")));

        char prefix;
        ci_string number;
        ci_string res;
        std::stringstream res_ss;

        if (value[0] != '\"')
        {
            prefix = value[0];
            number = value.substr(2, value.rfind('\"') - 2);
        }
        else
        {
            prefix = 'B';
            number = value.substr(1, value.rfind('\"') - 1);
        }

        // select base
        switch (prefix)
        {
            case 'B': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '1'); }))
                {
                    log_error("vhdl_parser", "invalid character within binary number literal {} in line {}.", value, line_number);
                    return "";
                }

                res_ss << std::uppercase << std::hex << stoull(core_strings::convert_string<ci_string, std::string>(number), 0, 2);
                res = ci_string(res_ss.str().data());
                break;
            }

            case 'O': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '7'); }))
                {
                    log_error("vhdl_parser", "invalid character within octal number literal {} in line {}.", value, line_number);
                    return "";
                }

                res_ss << std::uppercase << std::hex << stoull(core_strings::convert_string<ci_string, std::string>(number), 0, 8);
                res = ci_string(res_ss.str().data());
                break;
            }

            case 'D': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9'); }))
                {
                    log_error("vhdl_parser", "invalid character within decimal number literal {} in line {}.", value, line_number);
                    return "";
                }

                res_ss << std::uppercase << std::hex << stoull(core_strings::convert_string<ci_string, std::string>(number), 0, 10);
                res = ci_string(res_ss.str().data());
                break;
            }

            case 'X': {
                for (const auto& c : number)
                {
                    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))
                    {
                        res += c;
                    }
                    else
                    {
                        log_error("vhdl_parser", "invalid character within hexadecimal number literal {} in line {}.", value, line_number);
                        return "";
                    }
                }

                break;
            }

            default: {
                log_error("vhdl_parser", "invalid base '{}' within number literal {} in line {}.", prefix, value, line_number);
                res = "";
                break;
            }
        }

        return res;
    }

    std::vector<VHDLParser::ci_string> VHDLParser::expand_ranges(const ci_string& name, const std::vector<std::vector<u32>>& ranges) const
    {
        std::vector<ci_string> res;

        expand_ranges_recursively(res, name, ranges, 0);

        return res;
    }

    void VHDLParser::expand_ranges_recursively(std::vector<ci_string>& expanded_names, const ci_string& current_name, const std::vector<std::vector<u32>>& ranges, u32 dimension) const
    {
        // expand signal recursively
        if (ranges.size() > dimension)
        {
            for (const u32 index : ranges[dimension])
            {
                expand_ranges_recursively(expanded_names, current_name + "(" + core_strings::convert_string<std::string, ci_string>(std::to_string(index)) + ")", ranges, dimension + 1);
            }
        }
        else
        {
            // last dimension
            expanded_names.push_back(current_name);
        }
    }

    VHDLParser::ci_string VHDLParser::get_unique_alias(std::unordered_map<ci_string, u32>& name_occurrences, const ci_string& name) const
    {
        // if the name only appears once, we don't have to suffix it
        if (name_occurrences[name] < 2)
        {
            return name;
        }

        name_occurrences[name]++;

        // otherwise, add a unique string to the name
        return name + "__[" + core_strings::convert_string<std::string, ci_string>(std::to_string(name_occurrences[name])) + "]__";
    }
}    // namespace hal
