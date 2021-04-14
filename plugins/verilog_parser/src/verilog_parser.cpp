#include "verilog_parser/verilog_parser.h"

#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <fstream>
#include <iomanip>

namespace hal
{
    // ###########################################################################
    // ###########          Parse HDL into intermediate format          ##########
    // ###########################################################################

    bool NetlistParserVerilog::parse(const std::filesystem::path& file_path)
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

        return true;
    }

    std::unique_ptr<Netlist> NetlistParserVerilog::instantiate(const GateLibrary* gate_library)
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

        // TODO implement

        return result;
    }

    bool NetlistParserVerilog::tokenize()
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

    bool NetlistParserVerilog::parse_tokens()
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

    bool NetlistParserVerilog::parse_module(std::map<std::string, std::string>& attributes)
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

    bool NetlistParserVerilog::parse_port_list(VerilogModule& module)
    {
        TokenStream<std::string> ports_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (ports_stream.remaining() > 0)
        {
            Token<std::string> next_token = ports_stream.consume();
            std::string port_identifier;

            // TODO add support for unnamed ports
            if (next_token == ".")
            {
                port_identifier = ports_stream.consume().string;
                module.m_port_identifiers.push_back(port_identifier);

                ports_stream.consume("(", true);

                // TODO add support for ranges
                next_token = ports_stream.consume();
                if (next_token == "{")
                {
                    do
                    {
                        std::string signal_identifier = ports_stream.consume().string;
                        module.m_port_ident_to_expr[port_identifier].push_back(signal_identifier);
                    } while (ports_stream.consume(",", false) == true);
                    ports_stream.consume("}", true);
                }
                else
                {
                    module.m_port_ident_to_expr[port_identifier].push_back(next_token.string);
                }

                ports_stream.consume(")", true);
            }
            else
            {
                port_identifier = next_token.string;
                module.m_port_identifiers.push_back(port_identifier);
                module.m_port_expressions.insert(port_identifier);
                module.m_port_ident_to_expr[port_identifier].push_back(port_identifier);
            }
            ports_stream.consume(",", ports_stream.remaining() > 0);
        }

        return true;
    }

    bool NetlistParserVerilog::parse_port_declaration_list(VerilogModule& module)
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
                module.m_port_ident_to_expr[port_identifier].push_back(port_identifier);
                module.m_port_directions[port_identifier] = direction;
                module.m_port_ranges[port_identifier]     = ranges;
            } while (ports_stream.consume(",", ports_stream.remaining() > 0));
        }

        return true;
    }

    bool NetlistParserVerilog::parse_port_definition(VerilogModule& module, std::map<std::string, std::string>& attributes)
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
            module.m_port_ranges[port_expression]     = ranges;
            for (const auto& [attribute_name, attribute_value] : attributes)
            {
                module.m_port_attributes[port_expression].push_back(std::make_tuple(attribute_name, "unknown", attribute_value));
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(";", true);
        attributes.clear();

        return true;
    }

    bool NetlistParserVerilog::parse_signal_definition(VerilogModule& module, std::map<std::string, std::string>& attributes)
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
            module.m_signal_ranges[signal_name] = ranges;
            for (const auto& [attribute_name, attribute_value] : attributes)
            {
                module.m_signal_attributes[signal_name].push_back(std::make_tuple(attribute_name, "unknown", attribute_value));
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(";", true);
        attributes.clear();

        return true;
    }

    bool NetlistParserVerilog::parse_assignment(VerilogModule& module)
    {
        m_token_stream.consume("assign", true);
        TokenStream<std::string> left_stream = m_token_stream.extract_until("=");
        m_token_stream.consume("=", true);
        TokenStream<std::string> right_stream = m_token_stream.extract_until(";");
        m_token_stream.consume(";", true);

        module.m_assignments.push_back(std::make_pair(left_stream, right_stream));

        return true;
    }

    bool NetlistParserVerilog::parse_attribute(std::map<std::string, std::string>& attributes)
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

    bool NetlistParserVerilog::parse_instance(VerilogModule& module, std::map<std::string, std::string>& attributes)
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

    bool NetlistParserVerilog::parse_port_assign(VerilogModule& module, const std::string& instance_name)
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

    std::vector<std::tuple<std::string, std::string, std::string>> NetlistParserVerilog::parse_generic_assign()
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
    // ###################          Helper functions          ####################
    // ###########################################################################

    void NetlistParserVerilog::remove_comments(std::string& line, bool& multi_line_comment)
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

    std::vector<u32> NetlistParserVerilog::parse_range(TokenStream<std::string>& range_str)
    {
        if (range_str.remaining() == 1)
        {
            return {(u32)std::stoi(range_str.consume().string)};
        }

        const int start = std::stoi(range_str.consume().string);
        range_str.consume(":", true);
        const int end = std::stoi(range_str.consume().string);

        const int direction = (start <= end) ? 1 : -1;

        std::vector<u32> res;
        for (int i = start; i != end + direction; i += direction)
        {
            res.push_back((u32)i);
        }
        return res;
    }

    // std::map<std::string, VerilogSignal> NetlistParserVerilog::parse_signal_list()
    // {
    //     std::map<std::string, VerilogSignal> signals;
    //     std::vector<std::vector<u32>> ranges;

    //     TokenStream<std::string> signal_str = m_token_stream.extract_until(";");
    //     m_token_stream.consume(";", true);

    //     // extract bounds
    //     while (signal_str.consume("["))
    //     {
    //         const std::vector<u32> range = parse_range(signal_str);
    //         signal_str.consume("]", true);

    //         ranges.emplace_back(range);
    //     }

    //     // extract names
    //     do
    //     {
    //         const Token<std::string> signal_name = signal_str.consume();

    //         VerilogSignal s(signal_name.number, signal_name.string, ranges);
    //         signals.emplace(signal_name, s);
    //     } while (signal_str.consume(",", false));

    //     return signals;
    // }

    // std::optional<std::pair<std::vector<VerilogSignal>, i32>> NetlistParserVerilog::get_assignment_signals(VerilogModule& e, TokenStream<std::string>& signal_str, bool allow_numerics)
    // {
    //     // PARSE ASSIGNMENT
    //     //   assignment can currently be one of the following:
    //     //   (1) NAME *single-dimensional*
    //     //   (2) NAME *multi-dimensional*
    //     //   (3) NUMBER
    //     //   (4) NAME[INDEX1][INDEX2]...
    //     //   (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
    //     //   (6) {(1 - 5), (1 - 5), ...}

    //     std::vector<VerilogSignal> result;
    //     std::vector<TokenStream<std::string>> parts;
    //     i32 size = 0;

    //     // (6) {(1) - (5), (1) - (5), ...}
    //     if (signal_str.peek() == "{")
    //     {
    //         signal_str.consume("{", true);

    //         TokenStream<std::string> assignment_list_str = signal_str.extract_until("}");
    //         signal_str.consume("}", true);

    //         do
    //         {
    //             parts.push_back(assignment_list_str.extract_until(","));
    //         } while (assignment_list_str.consume(",", false));
    //     }
    //     else
    //     {
    //         parts.push_back(signal_str);
    //     }

    //     for (TokenStream<std::string>& part_stream : parts)
    //     {
    //         const Token<std::string> signal_name_token = part_stream.consume();
    //         const u32 line_number                      = signal_name_token.number;
    //         std::string signal_name                    = signal_name_token.string;
    //         std::vector<std::vector<u32>> ranges;
    //         bool is_binary = false;

    //         // (3) NUMBER
    //         if (isdigit(signal_name[0]) || signal_name[0] == '\'')
    //         {
    //             if (!allow_numerics)
    //             {
    //                 log_error("verilog_parser", "numeric value {} not allowed at this position in line {}", signal_name, line_number);
    //                 return std::nullopt;
    //             }

    //             signal_name = get_bin_from_literal(signal_name_token);
    //             if (signal_name.empty())
    //             {
    //                 return std::nullopt;
    //             }

    //             ranges    = {};
    //             is_binary = true;
    //         }
    //         else
    //         {
    //             std::vector<std::vector<u32>> reference_ranges;

    //             const std::map<std::string, VerilogSignal>& signals                        = e.get_signals();
    //             const std::map<std::string, std::pair<PinDirection, VerilogSignal>>& ports = e.get_ports();
    //             if (const auto signal_it = signals.find(signal_name); signal_it != signals.end())
    //             {
    //                 reference_ranges = signal_it->second.get_ranges();
    //             }
    //             else if (const auto port_it = ports.find(signal_name); port_it != ports.end())
    //             {
    //                 reference_ranges = port_it->second.second.get_ranges();
    //             }
    //             else
    //             {
    //                 log_error("verilog_parser", "signal name '{}' is invalid in assignment in line {}", signal_name, line_number);
    //                 return std::nullopt;
    //             }

    //             // any bounds specified?
    //             if (part_stream.consume("["))
    //             {
    //                 // (4) NAME[INDEX1][INDEX2]...
    //                 // (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
    //                 do
    //                 {
    //                     TokenStream<std::string> range_str = part_stream.extract_until("]");
    //                     ranges.emplace_back(parse_range(range_str));
    //                     part_stream.consume("]", true);
    //                 } while (part_stream.consume("[", false));
    //             }
    //             else
    //             {
    //                 // (1) NAME *single-dimensional*
    //                 // (2) NAME *multi-dimensional*
    //                 ranges = reference_ranges;
    //             }
    //         }

    //         // create new signal for assign
    //         VerilogSignal s(line_number, signal_name, ranges, is_binary);
    //         size += s.get_size();
    //         result.push_back(s);
    //     }

    //     return std::make_pair(result, size);
    // }

    static const std::map<char, std::string> oct_to_bin = {{'0', "000"}, {'1', "001"}, {'2', "010"}, {'3', "011"}, {'4', "100"}, {'5', "101"}, {'6', "110"}, {'7', "111"}};
    static const std::map<char, std::string> hex_to_bin = {{'0', "0000"},
                                                           {'1', "0001"},
                                                           {'2', "0010"},
                                                           {'3', "0011"},
                                                           {'4', "0100"},
                                                           {'5', "0101"},
                                                           {'6', "0110"},
                                                           {'7', "0111"},
                                                           {'8', "1000"},
                                                           {'9', "1001"},
                                                           {'a', "1010"},
                                                           {'b', "1011"},
                                                           {'c', "1100"},
                                                           {'d', "1101"},
                                                           {'e', "1110"},
                                                           {'f', "1111"}};

    std::string NetlistParserVerilog::get_bin_from_literal(const Token<std::string>& value_token)
    {
        const u32 line_number   = value_token.number;
        const std::string value = utils::to_lower(utils::replace(value_token.string, std::string("_"), std::string("")));

        i32 len = -1;
        std::string prefix;
        std::string number;
        std::string res;

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
                for (const char c : number)
                {
                    if (c >= '0' && c <= '1')
                    {
                        res += c;
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within binary number literal {} in line {}", value, line_number);
                        return "";
                    }
                }
                break;
            }

            case 'o':
                for (const char c : number)
                {
                    if (c >= '0' && c <= '7')
                    {
                        res += oct_to_bin.at(c);
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within octal number literal {} in line {}", value, line_number);
                        return "";
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
                        log_error("verilog_parser", "invalid character within decimal number literal {} in line {}", value, line_number);
                        return "";
                    }
                }

                do
                {
                    res = (((tmp_val & 1) == 1) ? "1" : "0") + res;
                    tmp_val >>= 1;
                } while (tmp_val != 0);
                break;
            }

            case 'h': {
                for (const char c : number)
                {
                    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                    {
                        res += hex_to_bin.at(c);
                    }
                    else
                    {
                        log_error("verilog_parser", "invalid character within hexadecimal number literal {} in line {}", value, line_number);
                        return "";
                    }
                }
                break;
            }

            default: {
                log_error("verilog_parser", "invalid base '{}' within number literal {} in line {}", prefix, value, line_number);
                return "";
            }
        }

        if (len != -1)
        {
            // fill with '0'
            for (i32 i = 0; i < len - (i32)res.size(); i++)
            {
                res = "0" + res;
            }
        }

        return res;
    }

    std::string NetlistParserVerilog::get_hex_from_literal(const Token<std::string>& value_token)
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

    bool NetlistParserVerilog::is_in_bounds(const std::vector<std::pair<i32, i32>>& bounds, const std::vector<std::pair<i32, i32>>& reference_bounds) const
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

    // std::vector<std::string> NetlistParserVerilog::expand_binary_signal(const VerilogSignal& s) const
    // {
    //     std::vector<std::string> res;

    //     for (const char bin_value : s.get_name())
    //     {
    //         res.push_back("'" + std::string(1, bin_value) + "'");
    //     }

    //     return res;
    // }

    // std::vector<std::string> NetlistParserVerilog::expand_signal(const VerilogSignal& s) const
    // {
    //     std::vector<std::string> res;

    //     expand_signal_recursively(res, s.get_name(), s.get_ranges(), 0);

    //     return res;
    // }

    // void NetlistParserVerilog::expand_signal_recursively(std::vector<std::string>& expanded_signal, const std::string& current_signal, const std::vector<std::vector<u32>>& ranges, u32 dimension) const
    // {
    //     // expand signal recursively
    //     if (ranges.size() > dimension)
    //     {
    //         for (const u32 index : ranges[dimension])
    //         {
    //             expand_signal_recursively(expanded_signal, current_signal + "(" + core_strings::convert_string<std::string, std::string>(std::to_string(index)) + ")", ranges, dimension + 1);
    //         }
    //     }
    //     else
    //     {
    //         // last dimension
    //         expanded_signal.push_back(current_signal);
    //     }
    // }

    // std::vector<std::string> NetlistParserVerilog::expand_signal_vector(const std::vector<VerilogSignal>& signals, bool allow_binary) const
    // {
    //     std::vector<std::string> res;

    //     for (const VerilogSignal& s : signals)
    //     {
    //         std::vector<std::string> expanded;

    //         if (allow_binary && s.is_binary())
    //         {
    //             expanded = expand_binary_signal(s);
    //         }
    //         else
    //         {
    //             expanded = expand_signal(s);
    //         }

    //         res.insert(res.begin(), expanded.begin(), expanded.end());
    //     }

    //     return res;
    // }
}    // namespace hal
