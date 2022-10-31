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
    namespace
    {
        std::vector<u32> parse_range(TokenStream<std::string>& stream)
        {
            if (stream.remaining() == 1)
            {
                return {(u32)std::stoi(stream.consume().string)};
            }

            // MSB to LSB
            const int end = std::stoi(stream.consume().string);
            stream.consume(":", true);
            const int start = std::stoi(stream.consume().string);

            const int direction = (start <= end) ? 1 : -1;

            std::vector<u32> result;
            for (int i = start; i != end + direction; i += direction)
            {
                result.push_back((u32)i);
            }
            return result;
        }

        void expand_ranges_recursively(std::vector<std::string>& expanded_names, const std::string& current_name, const std::vector<std::vector<u32>>& ranges, u32 dimension)
        {
            // expand signal recursively
            if (ranges.size() > dimension)
            {
                for (const u32 index : ranges[dimension])
                {
                    expand_ranges_recursively(expanded_names, current_name + "(" + std::to_string(index) + ")", ranges, dimension + 1);
                }
            }
            else
            {
                // last dimension
                expanded_names.push_back(current_name);
            }
        }

        std::vector<std::string> expand_ranges(const std::string& name, const std::vector<std::vector<u32>>& ranges)
        {
            std::vector<std::string> res;

            expand_ranges_recursively(res, name, ranges, 0);

            return res;
        }

        static const std::map<char, BooleanFunction::Value> bin_map = {{'0', BooleanFunction::Value::ZERO},
                                                                       {'1', BooleanFunction::Value::ONE},
                                                                       {'X', BooleanFunction::Value::X},
                                                                       {'Z', BooleanFunction::Value::Z}};

        static const std::map<char, std::vector<BooleanFunction::Value>> oct_map = {{'0', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO}},
                                                                                    {'1', {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO}},
                                                                                    {'2', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO}},
                                                                                    {'3', {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO}},
                                                                                    {'4', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
                                                                                    {'5', {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
                                                                                    {'6', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE}},
                                                                                    {'7', {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE}},
                                                                                    {'X', {BooleanFunction::Value::X, BooleanFunction::Value::X, BooleanFunction::Value::X}},
                                                                                    {'Z', {BooleanFunction::Value::Z, BooleanFunction::Value::Z, BooleanFunction::Value::Z}}};

        static const std::map<char, std::vector<BooleanFunction::Value>> hex_map = {
            {'0', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO}},
            {'1', {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO}},
            {'2', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO}},
            {'3', {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO}},
            {'4', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO}},
            {'5', {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO}},
            {'6', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO}},
            {'7', {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO}},
            {'8', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {'9', {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {'A', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {'B', {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {'C', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE}},
            {'D', {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE}},
            {'E', {BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE}},
            {'F', {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE}},
            {'X', {BooleanFunction::Value::X, BooleanFunction::Value::X, BooleanFunction::Value::X, BooleanFunction::Value::X}},
            {'Z', {BooleanFunction::Value::Z, BooleanFunction::Value::Z, BooleanFunction::Value::Z, BooleanFunction::Value::Z}}};

        Result<std::vector<BooleanFunction::Value>> get_binary_vector(std::string value)
        {
            value = utils::to_upper(utils::replace(value, std::string("_"), std::string("")));

            i32 len = -1;
            std::string prefix;
            std::string number;
            std::vector<BooleanFunction::Value> result;

            // base specified?
            if (value.find('\'') == std::string::npos)
            {
                prefix = "D";
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
                case 'B': {
                    for (auto it = number.rbegin(); it != number.rend(); it++)
                    {
                        const char c = *it;
                        if (c == '0' || c == '1' || c == 'Z' || c == 'X')
                        {
                            result.push_back(bin_map.at(c));
                        }
                        else
                        {
                            return ERR("could not convert string to binary vector: invalid character within binary number literal '" + value + "'");
                        }
                    }
                    break;
                }

                case 'O':
                    for (auto it = number.rbegin(); it != number.rend(); it++)
                    {
                        const char c = *it;
                        if ((c >= '0' && c <= '7') || c == 'X' || c == 'Z')
                        {
                            const auto& bits = oct_map.at(c);
                            result.insert(result.end(), bits.begin(), bits.end());
                        }
                        else
                        {
                            return ERR("could not convert string to binary vector: invalid character within octal number literal '" + value + "'");
                        }
                    }
                    break;

                case 'D': {
                    u64 tmp_val = 0;

                    for (const char c : number)
                    {
                        if ((c >= '0' && c <= '9'))
                        {
                            tmp_val = (tmp_val * 10) + (c - '0');
                        }
                        else
                        {
                            return ERR("could not convert string to binary vector: invalid character within decimal number literal '" + value + "'");
                        }
                    }

                    do
                    {
                        result.push_back(((tmp_val & 1) == 1) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
                        tmp_val >>= 1;
                    } while (tmp_val != 0);
                    break;
                }

                case 'H': {
                    for (auto it = number.rbegin(); it != number.rend(); it++)
                    {
                        const char c = *it;
                        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || c == 'X' || c == 'Z')
                        {
                            const auto& bits = hex_map.at(c);
                            result.insert(result.end(), bits.begin(), bits.end());
                        }
                        else
                        {
                            return ERR("could not convert string to binary vector: invalid character within hexadecimal number literal '" + value + "'");
                        }
                    }
                    break;
                }

                default: {
                    return ERR("could not convert string to binary vector: invalid base '" + prefix + "' within number literal '" + value + "'");
                }
            }

            if (len != -1)
            {
                i32 result_size = result.size();

                if (len > result_size)
                {
                    // fill with '0'
                    for (i32 i = 0; i < (len - result_size); i++)
                    {
                        result.push_back(BooleanFunction::Value::ZERO);
                    }
                }
                else
                {
                    // drop trailing bits
                    for (i32 i = 0; i < (result_size - len); i++)
                    {
                        result.pop_back();
                    }
                }
            }

            return OK(result);
        }

        Result<std::string> get_hex_from_literal(const Token<std::string>& value_token)
        {
            const u32 line_number   = value_token.number;
            const std::string value = utils::to_upper(utils::replace(value_token.string, std::string("_"), std::string("")));

            i32 len = -1;
            std::string prefix;
            std::string number;
            u32 base;

            // base specified?
            if (value.find('\'') == std::string::npos)
            {
                prefix = "D";
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
                case 'B': {
                    if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '1'); }))
                    {
                        return ERR("could not convert token to hexadecimal string: invalid character within binary number literal '" + value + "' (line " + std::to_string(line_number) + ")");
                    }

                    base = 2;
                    break;
                }

                case 'O': {
                    if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '7'); }))
                    {
                        return ERR("could not convert token to hexadecimal string: invalid character within ocatl number literal '" + value + "' (line " + std::to_string(line_number) + ")");
                    }

                    base = 8;
                    break;
                }

                case 'D': {
                    if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9'); }))
                    {
                        return ERR("could not convert token to hexadecimal string: invalid character within decimal number literal '" + value + "' (line " + std::to_string(line_number) + ")");
                    }

                    base = 10;
                    break;
                }

                case 'H': {
                    std::string res;

                    for (const char c : number)
                    {
                        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))
                        {
                            res += c;
                        }
                        else
                        {
                            return ERR("could not convert token to hexadecimal string: invalid character within hexadecimal number literal '" + value + "' (line " + std::to_string(line_number) + ")");
                        }
                    }

                    return OK(res);
                }

                default: {
                    return ERR("could not convert token to hexadecimal string: invalid base '" + prefix + "' within number literal '" + value + "' (line " + std::to_string(line_number) + ")");
                }
            }

            std::stringstream ss;
            if (len != -1)
            {
                // fill with '0'
                ss << std::uppercase << std::setfill('0') << std::setw((len + 3) / 4) << std::hex << stoull(number, 0, base);
            }
            else
            {
                ss << std::uppercase << std::hex << stoull(number, 0, base);
            }
            return OK(ss.str());
        }

        Result<std::pair<std::string, std::string>> parse_parameter_value(const Token<std::string>& value_token)
        {
            std::pair<std::string, std::string> value;

            if (utils::is_integer(value_token.string))
            {
                value.first  = "integer";
                value.second = value_token.string;
            }
            else if (utils::is_floating_point(value_token.string))
            {
                value.first  = "floating_point";
                value.second = value_token.string;
            }
            else if (value_token.string[0] == '\"' && value_token.string.back() == '\"')
            {
                value.first  = "string";
                value.second = value_token.string.substr(1, value_token.string.size() - 2);
            }
            else if (isdigit(value_token.string[0]) || value_token.string[0] == '\'')
            {
                if (const auto res = get_hex_from_literal(value_token); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not parse parameter value: failed to convert '" + value_token.string + "' to hexadecimal value (line " + std::to_string(value_token.number) + ")");
                }
                else
                {
                    value.second = res.get();
                }

                if (value.second == "0" || value.second == "1")
                {
                    value.first = "bit_value";
                }
                else
                {
                    value.first = "bit_vector";
                }
            }
            else
            {
                return ERR("could not parse parameter value: failed to identify data type of parameter '" + value_token.string + "' (line " + std::to_string(value_token.number) + ")");
            }

            return OK(value);
        }

        Result<std::vector<assignment_t>> parse_assignment_expression(TokenStream<std::string>&& stream)
        {
            std::vector<TokenStream<std::string>> parts;

            if (stream.size() == 0)
            {
                return OK({});
            }

            if (stream.peek() == "{")
            {
                stream.consume("{", true);

                TokenStream<std::string> assignment_list_str = stream.extract_until("}");
                stream.consume("}", true);

                do
                {
                    parts.push_back(assignment_list_str.extract_until(","));
                } while (assignment_list_str.consume(",", false));
            }
            else
            {
                parts.push_back(stream);
            }

            std::vector<assignment_t> result;
            result.reserve(parts.size());

            for (auto it = parts.rbegin(); it != parts.rend(); it++)
            {
                TokenStream<std::string>& part_stream = *it;

                const Token<std::string> signal_name_token = part_stream.consume();
                std::string signal_name                    = signal_name_token.string;

                // (3) NUMBER
                if (isdigit(signal_name[0]) || signal_name[0] == '\'')
                {
                    if (auto res = get_binary_vector(signal_name_token); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse assignment expression: unable to convert token to binary vector");
                    }
                    else
                    {
                        result.push_back(std::move(res.get()));
                    }
                }
                else
                {
                    // any bounds specified?
                    if (part_stream.consume("["))
                    {
                        // (4) NAME[INDEX1][INDEX2]...
                        // (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...

                        std::vector<std::vector<u32>> ranges;
                        do
                        {
                            TokenStream<std::string> range_str = part_stream.extract_until("]");
                            ranges.emplace_back(parse_range(range_str));
                            part_stream.consume("]", true);
                        } while (part_stream.consume("[", false));

                        result.push_back(ranged_identifier_t({std::move(signal_name), std::move(ranges)}));
                    }
                    else
                    {
                        // (1) NAME *single-dimensional*
                        // (2) NAME *multi-dimensional*
                        result.push_back(std::move(signal_name));
                    }
                }
            }

            return OK(result);
        }

        std::vector<std::string> expand_assignment_expression(VerilogModule* verilog_module, const std::vector<assignment_t>& vars)
        {
            std::vector<std::string> result;
            for (const auto& var : vars)
            {
                if (const identifier_t* identifier = std::get_if<identifier_t>(&var); identifier != nullptr)
                {
                    std::vector<std::vector<u32>> ranges;

                    if (const auto signal_it = verilog_module->m_signals_by_name.find(*identifier); signal_it != verilog_module->m_signals_by_name.end())
                    {
                        ranges = signal_it->second->m_ranges;
                    }
                    else if (const auto port_it = verilog_module->m_ports_by_expression.find(*identifier); port_it != verilog_module->m_ports_by_expression.end())
                    {
                        ranges = port_it->second->m_ranges;
                    }

                    std::vector<std::string> expanded = expand_ranges(*identifier, ranges);
                    result.insert(result.end(), expanded.begin(), expanded.end());
                }
                else if (const ranged_identifier_t* ranged_identifier = std::get_if<ranged_identifier_t>(&var); ranged_identifier != nullptr)
                {
                    std::vector<std::string> expanded = expand_ranges(ranged_identifier->first, ranged_identifier->second);
                    result.insert(result.end(), expanded.begin(), expanded.end());
                }
                else if (const numeral_t* numeral = std::get_if<numeral_t>(&var); numeral != nullptr)
                {
                    for (auto value : *numeral)
                    {
                        result.push_back("'" + BooleanFunction::to_string(value) + "'");
                    }
                }
            }

            return result;
        }
    }    // namespace

    Result<std::monostate> VerilogParser::parse(const std::filesystem::path& file_path)
    {
        m_path = file_path;
        m_modules.clear();
        m_modules_by_name.clear();

        {
            std::ifstream ifs;
            ifs.open(file_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                return ERR("could not parse Verilog file '" + m_path.string() + "' : unable to open file");
            }
            m_fs << ifs.rdbuf();
            ifs.close();
        }

        // tokenize file
        tokenize();

        // parse tokens into intermediate format
        try
        {
            if (auto res = parse_tokens(); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse Verilog file '" + file_path.string() + "': unable to parse tokens");
            }
        }
        catch (TokenStream<std::string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                return ERR("could not parse Verilog file '" + m_path.string() + "': " + e.message + " (line " + std::to_string(e.line_number) + ")");
            }
            else
            {
                return ERR("could not parse Verilog file '" + m_path.string() + "': " + e.message);
            }
        }

        if (m_modules.empty())
        {
            return ERR("could not parse Verilog file '" + m_path.string() + "': does not contain any modules");
        }

        // expand module port identifiers, signals, and assignments
        for (auto& [module_name, verilog_module] : m_modules_by_name)
        {
            // expand port identifiers
            for (const auto& port : verilog_module->m_ports)
            {
                if (port->m_expression == port->m_identifier)
                {
                    if (!port->m_ranges.empty())
                    {
                        port->m_expanded_identifiers = expand_ranges(port->m_identifier, port->m_ranges);
                        verilog_module->m_expanded_port_expressions.insert(port->m_expanded_identifiers.begin(), port->m_expanded_identifiers.end());
                    }
                    else
                    {
                        port->m_expanded_identifiers = {port->m_identifier};
                        verilog_module->m_expanded_port_expressions.insert(port->m_identifier);
                    }
                }
                else
                {
                    if (!port->m_ranges.empty())
                    {
                        port->m_expanded_identifiers = expand_ranges(port->m_identifier, port->m_ranges);
                        auto expanded_expression     = expand_ranges(port->m_expression, port->m_ranges);
                        verilog_module->m_expanded_port_expressions.insert(expanded_expression.begin(), expanded_expression.end());
                    }
                    else
                    {
                        port->m_expanded_identifiers = {port->m_identifier};
                        verilog_module->m_expanded_port_expressions.insert(port->m_expression);
                    }
                }
            }

            // expand signals
            for (auto& signal : verilog_module->m_signals)
            {
                if (!signal->m_ranges.empty())
                {
                    signal->m_expanded_names = expand_ranges(signal->m_name, signal->m_ranges);
                }
                else
                {
                    signal->m_expanded_names = std::vector<std::string>({signal->m_name});
                }
            }

            // expand assignments
            for (auto& assignment : verilog_module->m_assignments)
            {
                const std::vector<std::string> left_signals  = expand_assignment_expression(verilog_module, assignment.m_variable);
                const std::vector<std::string> right_signals = expand_assignment_expression(verilog_module, assignment.m_assignment);
                if (left_signals.empty() || right_signals.empty())
                {
                    return ERR("could not parse Verilog file '" + m_path.string() + "': unable to expand assignments within module '" + verilog_module->m_name + "'");
                }

                u32 left_size  = left_signals.size();
                u32 right_size = right_signals.size();
                if (left_size <= right_size)
                {
                    // cut off redundant bits
                    for (u32 i = 0; i < left_size; i++)
                    {
                        verilog_module->m_expanded_assignments.push_back(std::make_pair(left_signals.at(i), right_signals.at(i)));
                    }
                }
                else
                {
                    for (u32 i = 0; i < right_size; i++)
                    {
                        verilog_module->m_expanded_assignments.push_back(std::make_pair(left_signals.at(i), right_signals.at(i)));
                    }

                    // implicit "0"
                    for (u32 i = 0; i < left_size - right_size; i++)
                    {
                        verilog_module->m_expanded_assignments.push_back(std::make_pair(left_signals.at(i + right_size), "'0'"));
                    }
                }
            }
        }

        // expand module port assignments
        for (auto& [module_name, verilog_module] : m_modules_by_name)
        {
            for (auto& instance : verilog_module->m_instances)
            {
                if (auto module_it = m_modules_by_name.find(instance->m_type); module_it != m_modules_by_name.end())
                {
                    instance->m_is_module = true;
                    if (!instance->m_port_assignments.empty())
                    {
                        // all port assignments by name
                        if (instance->m_port_assignments.front().m_port_name.has_value())
                        {
                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                const std::vector<std::string> right_port = expand_assignment_expression(verilog_module, port_assignment.m_assignment);
                                if (!right_port.empty())
                                {
                                    VerilogPort* port;
                                    if (const auto port_it = module_it->second->m_ports_by_identifier.find(port_assignment.m_port_name.value());
                                        port_it == module_it->second->m_ports_by_identifier.end())
                                    {
                                        return ERR("could not parse Verilog file '" + m_path.string() + "': unable to assign signal to port '" + port_assignment.m_port_name.value()
                                                   + "' as it is not a port of module '" + module_it->first + "'");
                                    }
                                    else
                                    {
                                        port = port_it->second;
                                    }
                                    const std::vector<std::string>& left_port = port->m_expanded_identifiers;
                                    if (left_port.empty())
                                    {
                                        return ERR("could not parse Verilog file '" + m_path.string() + "': unable to expand port assignment");
                                    }

                                    u32 max_size = right_port.size() <= left_port.size() ? right_port.size() : left_port.size();

                                    for (u32 i = 0; i < max_size; i++)
                                    {
                                        instance->m_expanded_port_assignments.push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                                    }
                                }
                            }
                        }
                        // all port assignments by order
                        else
                        {
                            std::vector<std::string> ports;
                            for (const auto& port : m_modules_by_name.at(instance->m_type)->m_ports)
                            {
                                ports.insert(ports.end(), port->m_expanded_identifiers.begin(), port->m_expanded_identifiers.end());
                            }

                            auto port_it = ports.begin();

                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                std::vector<std::string> right_port = expand_assignment_expression(verilog_module, port_assignment.m_assignment);
                                if (!right_port.empty())
                                {
                                    std::vector<std::string> left_port;

                                    for (u32 i = 0; i < right_port.size() && port_it != ports.end(); i++)
                                    {
                                        left_port.push_back(*port_it++);
                                    }

                                    u32 max_size = right_port.size() <= left_port.size() ? right_port.size() : left_port.size();

                                    for (u32 i = 0; i < max_size; i++)
                                    {
                                        instance->m_expanded_port_assignments.push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return OK({});
    }

    Result<std::unique_ptr<Netlist>> VerilogParser::instantiate(const GateLibrary* gate_library)
    {
        // create empty netlist
        std::unique_ptr<Netlist> result = netlist_factory::create_netlist(gate_library);
        m_netlist                       = result.get();
        if (m_netlist == nullptr)
        {
            return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create empty netlist");
        }

        m_gate_types.clear();
        m_gnd_gate_types.clear();
        m_vcc_gate_types.clear();
        m_instance_name_occurrences.clear();
        m_signal_name_occurrences.clear();
        m_net_by_name.clear();
        m_nets_to_merge.clear();
        m_module_ports.clear();
        for (const auto& verilog_module : m_modules)
        {
            for (const auto& instance : verilog_module->m_instances)
            {
                if (!instance->m_is_module)
                {
                    instance->m_expanded_port_assignments.clear();
                }
            }
        }

        // buffer gate types
        m_gate_types     = gate_library->get_gate_types();
        m_gnd_gate_types = gate_library->get_gnd_gate_types();
        m_vcc_gate_types = gate_library->get_vcc_gate_types();

        // create const 0 and const 1 net, will be removed if unused
        m_zero_net = m_netlist->create_net("'0'");
        if (m_zero_net == nullptr)
        {
            return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create zero net");
        }
        m_net_by_name[m_zero_net->get_name()] = m_zero_net;

        m_one_net = m_netlist->create_net("'1'");
        if (m_one_net == nullptr)
        {
            return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create one net");
        }
        m_net_by_name[m_one_net->get_name()] = m_one_net;

        // TODO: This tries to find the topmodule by searching for a module that is not referenced by any other module. This fails when there are multiple of those modules (for example with unused modules). There is also a top=1 flag that is set bz yosys for example that we could check first, before using this approach.
        std::map<std::string, u32> module_name_to_refereneces;
        for (const auto& [_name, module] : m_modules_by_name)
        {
            for (const auto& instance : module->m_instances)
            {
                if (const auto it = m_modules_by_name.find(instance->m_type); it != m_modules_by_name.end())
                {
                    module_name_to_refereneces[it->first]++;
                }
            }
        }

        std::vector<std::string> top_module_candidates;
        for (const auto& [name, module] : m_modules_by_name)
        {
            if (module_name_to_refereneces.find(name) == module_name_to_refereneces.end())
            {
                top_module_candidates.push_back(name);
            }
        }

        if (top_module_candidates.empty())
        {
            return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': unable to find any top module candidates");
        }

        if (top_module_candidates.size() > 1)
        {
            return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': found multiple modules as candidates for the top module");
        }

        // construct the netlist with the the top module
        VerilogModule* top_module = m_modules_by_name.at(top_module_candidates.front());

        if (const auto res = construct_netlist(top_module); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': unable to construct netlist");
        }

        // add global GND gate if required by any instance
        if (m_netlist->get_gnd_gates().empty())
        {
            if (!m_zero_net->get_destinations().empty())
            {
                GateType* gnd_type  = m_gnd_gate_types.begin()->second;
                GatePin* output_pin = gnd_type->get_output_pins().front();
                Gate* gnd           = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");

                if (!m_netlist->mark_gnd_gate(gnd))
                {
                    return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to mark GND gate");
                }

                if (m_zero_net->add_source(gnd, output_pin) == nullptr)
                {
                    return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to add source to GND gate");
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
                GateType* vcc_type  = m_vcc_gate_types.begin()->second;
                GatePin* output_pin = vcc_type->get_output_pins().front();
                Gate* vcc           = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");

                if (!m_netlist->mark_vcc_gate(vcc))
                {
                    return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to mark VCC gate");
                }

                if (m_one_net->add_source(vcc, output_pin) == nullptr)
                {
                    return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to add source to VCC gate");
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
            const u32 num_of_sources      = net->get_num_of_sources();
            const u32 num_of_destinations = net->get_num_of_destinations();
            const bool no_source          = num_of_sources == 0 && !(net->is_global_input_net() && num_of_destinations != 0);
            const bool no_destination     = num_of_destinations == 0 && !(net->is_global_output_net() && num_of_sources != 0);
            if (no_source && no_destination)
            {
                m_netlist->delete_net(net);
            }
        }

        m_netlist->load_gate_locations_from_data();

        return OK(std::move(result));
    }

    // ###########################################################################
    // ###########          Parse HDL into Intermediate Format          ##########
    // ###########################################################################

    void VerilogParser::tokenize()
    {
        const std::string delimiters = "`,()[]{}\\#*: ;=./";
        std::string current_token;
        u32 line_number = 0;

        std::string line;
        char prev_char  = 0;
        bool in_string  = false;
        bool escaped    = false;
        bool in_comment = false;

        std::vector<Token<std::string>> parsed_tokens;
        while (std::getline(m_fs, line))
        {
            line_number++;
            // this->remove_comments(line, multi_line_comment);

            for (char c : line)
            {
                // deal with comments
                if (in_comment)
                {
                    if (c == '/' && prev_char == '*')
                    {
                        in_comment = false;
                    }

                    prev_char = c;
                    continue;
                }

                // deal with escaping and strings
                if (!in_string && c == '\\')
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

                if (!in_comment && ((!std::isspace(c) && delimiters.find(c) == std::string::npos) || escaped || in_string))
                {
                    current_token += c;
                }
                else
                {
                    // deal with floats
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
                        // deal with multi-character tokens
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
                        // start a comment
                        else if (c == '/' && parsed_tokens.back() == "/")
                        {
                            parsed_tokens.pop_back();
                            break;
                        }
                        else if (c == '*' && parsed_tokens.back() == "/")
                        {
                            in_comment = true;
                            parsed_tokens.pop_back();
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
    }

    Result<std::monostate> VerilogParser::parse_tokens()
    {
        std::vector<VerilogDataEntry> attributes;
        u32 line_number;

        while (m_token_stream.remaining() > 0)
        {
            if (m_token_stream.peek() == "(*")
            {
                parse_attribute(attributes);
            }
            else if (m_token_stream.peek() == "`")
            {
                m_token_stream.consume_current_line();
                log_warning("verilog_parser", "could not parse compiler directives.");
            }
            else
            {
                line_number = m_token_stream.peek().number;
                if (auto res = parse_module(attributes); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens: unable to parse module (line " + std::to_string(line_number) + ")");
                }
            }
        }

        return OK({});
    }

    Result<std::monostate> VerilogParser::parse_module(std::vector<VerilogDataEntry>& attributes)
    {
        std::set<std::string> port_names;
        std::vector<VerilogDataEntry> internal_attributes;

        m_token_stream.consume("module", true);
        const u32 line_number         = m_token_stream.peek().number;
        const std::string module_name = m_token_stream.consume();

        // verify entity name
        if (const auto it = m_modules_by_name.find(module_name); it != m_modules_by_name.end())
        {
            return ERR("could not parse module '" + module_name + "' (line " + std::to_string(line_number) + "): a module with the same name already exists (line "
                       + std::to_string(it->second->m_line_number) + ")");
        }

        auto verilog_module               = std::make_unique<VerilogModule>();
        VerilogModule* verilog_module_raw = verilog_module.get();
        verilog_module_raw->m_line_number = line_number;
        verilog_module_raw->m_name        = module_name;

        // parse parameter list
        if (m_token_stream.consume("#("))
        {
            // TODO add support for parameter parsing
            m_token_stream.consume_until(")");
            m_token_stream.consume(")", true);
            log_warning("verilog_parser", "could not parse parameter list provided for module '{}'.", module_name);
        }

        // parse port (declaration) list
        m_token_stream.consume("(", true);
        Token<std::string> next_token = m_token_stream.peek();
        if (next_token == "input" || next_token == "output" || next_token == "inout")
        {
            if (auto res = parse_port_declaration_list(verilog_module_raw); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse module '" + module_name + "': unable to parse port declaration list (line " + std::to_string(line_number) + ")");
            }
        }
        else
        {
            parse_port_list(verilog_module_raw);
        }

        m_token_stream.consume(";", true);

        next_token = m_token_stream.peek();
        while (next_token != "endmodule")
        {
            if (next_token == "input" || next_token == "output" || next_token == "inout")
            {
                if (auto res = parse_port_definition(verilog_module_raw, internal_attributes); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse module '" + module_name + "': unable to parse port definition (line " + std::to_string(line_number) + ")");
                }
            }
            else if (next_token == "wire" || next_token == "tri")
            {
                if (auto res = parse_signal_definition(verilog_module_raw, internal_attributes); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse module '" + module_name + "': unable to parse signal definition (line " + std::to_string(line_number) + ")");
                }
            }
            else if (next_token == "parameter")
            {
                // TODO add support for parameter parsing
                m_token_stream.consume_until(";");
                m_token_stream.consume(";", true);
                log_warning("verilog_parser", "could not parse parameter provided for module '{}'.", module_name);
            }
            else if (next_token == "assign")
            {
                if (auto res = parse_assignment(verilog_module_raw); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse module '" + module_name + "': unable to parse assignment (line " + std::to_string(line_number) + ")");
                }
            }
            else if (next_token == "defparam")
            {
                if (auto res = parse_defparam(verilog_module_raw); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse module '" + module_name + "': unable to parse defparam (line " + std::to_string(line_number) + ")");
                }
            }
            else if (next_token == "(*")
            {
                parse_attribute(internal_attributes);
            }
            else
            {
                if (auto res = parse_instance(verilog_module_raw, internal_attributes); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse module '" + module_name + "': unable to parse instance (line " + std::to_string(line_number) + ")");
                }
            }

            next_token = m_token_stream.peek();
        }

        m_token_stream.consume("endmodule", true);

        // assign attributes to entity
        if (!attributes.empty())
        {
            verilog_module->m_attributes.insert(verilog_module->m_attributes.end(), attributes.begin(), attributes.end());
            attributes.clear();
        }

        // add to collection of entities
        m_modules.push_back(std::move(verilog_module));
        m_modules_by_name[module_name] = verilog_module_raw;
        m_last_module                  = module_name;

        return OK({});
    }

    void VerilogParser::parse_port_list(VerilogModule* verilog_module)
    {
        TokenStream<std::string> ports_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (ports_stream.remaining() > 0)
        {
            Token<std::string> next_token = ports_stream.consume();
            auto port                     = std::make_unique<VerilogPort>();

            if (next_token == ".")
            {
                port->m_identifier = ports_stream.consume().string;
                ports_stream.consume("(", true);
                port->m_expression = ports_stream.consume().string;
                ports_stream.consume(")", true);
            }
            else
            {
                port->m_identifier = next_token.string;
                port->m_expression = next_token.string;
            }

            verilog_module->m_ports_by_identifier[port->m_identifier] = port.get();
            verilog_module->m_ports_by_expression[port->m_expression] = port.get();
            verilog_module->m_ports.push_back(std::move(port));

            ports_stream.consume(",", ports_stream.remaining() > 0);
        }
    }

    Result<std::monostate> VerilogParser::parse_port_declaration_list(VerilogModule* verilog_module)
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
                return ERR("could not parse port declaration list: invalid direction '" + direction_token.string + "' (line " + std::to_string(direction_token.number) + ")");
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

                auto port          = std::make_unique<VerilogPort>();
                port->m_identifier = next_token.string;
                port->m_expression = next_token.string;
                port->m_direction  = direction;
                if (!ranges.empty())
                {
                    port->m_ranges = ranges;
                }

                verilog_module->m_ports_by_identifier[port->m_identifier] = port.get();
                verilog_module->m_ports_by_expression[port->m_expression] = port.get();
                verilog_module->m_ports.push_back(std::move(port));
            } while (ports_stream.consume(",", ports_stream.remaining() > 0));
        }

        return OK({});
    }

    Result<std::monostate> VerilogParser::parse_port_definition(VerilogModule* verilog_module, std::vector<VerilogDataEntry>& attributes)
    {
        // port direction
        const Token<std::string> direction_token = m_token_stream.consume();
        PinDirection direction                   = enum_from_string<PinDirection>(direction_token.string, PinDirection::none);
        if (direction == PinDirection::none || direction == PinDirection::internal)
        {
            return ERR("could not parse port definition: invalid direction '" + direction_token.string + "' (line " + std::to_string(direction_token.number) + ")");
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

            VerilogPort* port;
            if (const auto it = verilog_module->m_ports_by_expression.find(port_expression); it == verilog_module->m_ports_by_expression.end())
            {
                return ERR("could not parse port definition: a port with name '" + port_expression + "' does not exist for module '" + verilog_module->m_name + "' (line "
                           + std::to_string(direction_token.number) + ")");
            }
            else
            {
                port = it->second;
            }

            port->m_direction = direction;
            if (!ranges.empty())
            {
                port->m_ranges = ranges;
            }
            port->m_attributes.insert(port->m_attributes.end(), attributes.begin(), attributes.end());
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(";", true);
        attributes.clear();

        return OK({});
    }

    Result<std::monostate> VerilogParser::parse_signal_definition(VerilogModule* verilog_module, std::vector<VerilogDataEntry>& attributes)
    {
        // consume "wire" or "tri"
        u32 line_number = m_token_stream.consume().number;

        TokenStream<std::string> signal_stream = m_token_stream.extract_until(";");
        m_token_stream.consume(";", true);

        // extract bounds
        std::vector<std::vector<u32>> ranges;
        while (signal_stream.consume("["))
        {
            const std::vector<u32> range = parse_range(signal_stream);
            signal_stream.consume("]", true);

            ranges.emplace_back(range);
        }

        // extract names
        do
        {
            Token<std::string> signal_name = signal_stream.consume();
            if (signal_stream.remaining() > 0 && signal_stream.peek() == "=")
            {
                VerilogAssignment assignment;
                assignment.m_variable.push_back(signal_name);
                signal_stream.consume("=", true);
                if (auto res = parse_assignment_expression(signal_stream.extract_until(",")); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse signal definition: unable to parse assignment expression (line " + std::to_string(line_number) + ")");
                }
                else
                {
                    assignment.m_assignment = res.get();
                }
                verilog_module->m_assignments.push_back(std::move(assignment));
            }

            auto signal    = std::make_unique<VerilogSignal>();
            signal->m_name = signal_name.string;
            if (!ranges.empty())
            {
                signal->m_ranges = ranges;
            }
            signal->m_attributes.insert(signal->m_attributes.end(), attributes.begin(), attributes.end());
            verilog_module->m_signals_by_name[signal_name.string] = signal.get();
            verilog_module->m_signals.push_back(std::move(signal));

        } while (signal_stream.consume(",", false));

        attributes.clear();

        return OK({});
    }

    Result<std::monostate> VerilogParser::parse_assignment(VerilogModule* verilog_module)
    {
        m_token_stream.consume("assign", true);
        u32 line_number = m_token_stream.peek().number;
        VerilogAssignment assignment;

        if (auto res = parse_assignment_expression(m_token_stream.extract_until("=")); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not parse assignment: unable to parse assignment expression (line " + std::to_string(line_number) + ")");
        }
        else
        {
            assignment.m_variable = res.get();
        }
        m_token_stream.consume("=", true);

        if (auto res = parse_assignment_expression(m_token_stream.extract_until(";")); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not parse assignment: unable to parse assignment expression (line " + std::to_string(line_number) + ")");
        }
        else
        {
            assignment.m_assignment = res.get();
        }
        m_token_stream.consume(";", true);

        verilog_module->m_assignments.push_back(std::move(assignment));
        return OK({});
    }

    Result<std::monostate> VerilogParser::parse_defparam(VerilogModule* module)
    {
        m_token_stream.consume("defparam", true);
        std::string instance_name = m_token_stream.consume().string;
        m_token_stream.consume(".", true);

        if (const auto inst_it = module->m_instances_by_name.find(instance_name); inst_it != module->m_instances_by_name.end())
        {
            VerilogDataEntry param;
            param.m_name = m_token_stream.consume().string;
            m_token_stream.consume("=", true);

            if (const auto res = parse_parameter_value(m_token_stream.consume()); res.is_ok())
            {
                const auto value = res.get();
                param.m_type     = value.first;
                param.m_value    = value.second;
                inst_it->second->m_parameters.push_back(param);
            }
            else
            {
                log_warning("verilog_parser", "{}", res.get_error().get());
            }
        }
        else
        {
            m_token_stream.consume(";", true);
            return ERR("could not parse defparam: no instance with name '" + instance_name + "' exists within module '" + module->m_name + "'");
        }

        m_token_stream.consume(";", true);
        return OK({});
    }

    void VerilogParser::parse_attribute(std::vector<VerilogDataEntry>& attributes)
    {
        m_token_stream.consume("(*", true);

        // extract attributes
        do
        {
            VerilogDataEntry attribute;
            attribute.m_name = m_token_stream.consume().string;

            // attribute value specified?
            if (m_token_stream.consume("="))
            {
                attribute.m_value = m_token_stream.consume();

                // remove "
                if (attribute.m_value[0] == '\"' && attribute.m_value.back() == '\"')
                {
                    attribute.m_value = attribute.m_value.substr(1, attribute.m_value.size() - 2);
                }
            }

            attributes.push_back(std::move(attribute));

        } while (m_token_stream.consume(",", false));

        m_token_stream.consume("*)", true);
    }

    Result<std::monostate> VerilogParser::parse_instance(VerilogModule* verilog_module, std::vector<VerilogDataEntry>& attributes)
    {
        auto instance    = std::make_unique<VerilogInstance>();
        u32 line_number  = m_token_stream.peek().number;
        instance->m_type = m_token_stream.consume().string;

        // parse generics map
        if (m_token_stream.consume("#("))
        {
            if (auto res = parse_parameter_assign(); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse instance of type '" + instance->m_type + "': unable to parse parameter assignment (line " + std::to_string(line_number) + ")");
            }
            else
            {
                instance->m_parameters = res.get();
            }
        }

        // parse instance name
        instance->m_name = m_token_stream.consume().string;

        // parse port map
        if (auto res = parse_port_assign(instance.get()); res.is_error())
        {
            return ERR_APPEND(res.get_error(),
                              "could not parse instance '" + instance->m_name + "' of type '" + instance->m_type + "': unable to parse port assignment (line " + std::to_string(line_number) + ")");
        }

        // assign attributes to instance
        instance->m_attributes = attributes;
        attributes.clear();

        verilog_module->m_instances_by_name[instance->m_name] = instance.get();
        verilog_module->m_instances.push_back(std::move(instance));

        return OK({});
    }

    Result<std::monostate> VerilogParser::parse_port_assign(VerilogInstance* instance)
    {
        u32 line_number = m_token_stream.peek().number;
        m_token_stream.consume("(", true);
        u32 line_end = m_token_stream.find_next(";");
        if (m_token_stream.peek() == ".")
        {
            do
            {
                m_token_stream.consume(".");
                VerilogPortAssignment port_assignment;
                port_assignment.m_port_name = m_token_stream.consume().string;
                m_token_stream.consume("(", true);
                if (auto res = parse_assignment_expression(m_token_stream.extract_until(")")); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse port assignment: unable to parse assignment expression (line " + std::to_string(line_number) + ")");
                }
                else
                {
                    port_assignment.m_assignment = res.get();
                }
                m_token_stream.consume(")", true);
                if (port_assignment.m_assignment.empty())
                {
                    continue;
                }
                instance->m_port_assignments.push_back(std::move(port_assignment));
            } while (m_token_stream.consume(",", false));
        }
        else
        {
            do
            {
                VerilogPortAssignment port_assignment;
                if (auto res = parse_assignment_expression(m_token_stream.extract_until(",", line_end - 1)); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse port assignment: unable to parse assignment expression (line " + std::to_string(line_number) + ")");
                }
                else
                {
                    port_assignment.m_assignment = res.get();
                }
                if (port_assignment.m_assignment.empty())
                {
                    continue;
                }
                instance->m_port_assignments.push_back(std::move(port_assignment));
            } while (m_token_stream.consume(",", false));
        }

        m_token_stream.consume(")", true);
        m_token_stream.consume(";", true);

        return OK({});
    }

    Result<std::vector<VerilogDataEntry>> VerilogParser::parse_parameter_assign()
    {
        std::vector<VerilogDataEntry> generics;

        do
        {
            if (m_token_stream.consume(".", false))
            {
                const Token<std::string> lhs = m_token_stream.join_until("(", "");
                m_token_stream.consume("(", true);
                const Token<std::string> rhs = m_token_stream.join_until(")", "");
                m_token_stream.consume(")", true);

                if (const auto res = parse_parameter_value(rhs); res.is_ok())
                {
                    const auto value = res.get();
                    generics.push_back(VerilogDataEntry({lhs.string, value.first, value.second}));
                }
                else
                {
                    log_warning("verilog_parser", "{}", res.get_error().get());
                }
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(")", true);

        return OK(generics);
    }

    // ###########################################################################
    // ###########      Assemble Netlist from Intermediate Format       ##########
    // ###########################################################################

    Result<std::monostate> VerilogParser::construct_netlist(VerilogModule* top_module)
    {
        m_netlist->set_design_name(top_module->m_name);
        m_netlist->enable_automatic_net_checks(false);

        std::unordered_map<std::string, u32> instantiation_count;

        // preparations for alias: count the occurences of all names
        std::queue<VerilogModule*> q;
        q.push(top_module);

        // top entity instance will be named after its entity, so take into account for aliases
        m_instance_name_occurrences["top_module"]++;

        // global input/output signals will be named after ports, so take into account for aliases
        for (const auto& port : top_module->m_ports)
        {
            for (const auto& expanded_port_identifier : port->m_expanded_identifiers)
            {
                m_signal_name_occurrences[expanded_port_identifier]++;
            }
        }

        while (!q.empty())
        {
            VerilogModule* module = q.front();
            q.pop();

            instantiation_count[module->m_name]++;

            for (const auto& signal : module->m_signals)
            {
                for (const auto& expanded_name : signal->m_expanded_names)
                {
                    m_signal_name_occurrences[expanded_name]++;
                }
            }

            for (const auto& instance : module->m_instances)
            {
                m_instance_name_occurrences[instance->m_name]++;

                if (const auto it = m_modules_by_name.find(instance->m_type); it != m_modules_by_name.end())
                {
                    q.push(it->second);
                }
            }
        }

        for (auto& [module_name, verilog_module] : m_modules_by_name)
        {
            // detect unused modules
            if (instantiation_count[module_name] == 0)
            {
                log_warning("verilog_parser", "module '{}' has been defined in the netlist but is not instantiated.", module_name);
                continue;
            }

            // expand gate pin assignments
            for (const auto& instance : verilog_module->m_instances)
            {
                if (const auto gate_type_it = m_gate_types.find(instance->m_type); gate_type_it != m_gate_types.end())
                {
                    if (!instance->m_port_assignments.empty())
                    {
                        // all port assignments by name
                        if (instance->m_port_assignments.front().m_port_name.has_value())
                        {
                            // cache pin groups
                            std::unordered_map<std::string, std::vector<std::string>> pin_groups;
                            for (const auto pin_group : gate_type_it->second->get_pin_groups())
                            {
                                for (const auto pin : pin_group->get_pins())
                                {
                                    pin_groups[pin_group->get_name()].push_back(pin->get_name());
                                }
                            }

                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                std::vector<std::string> right_port = expand_assignment_expression(verilog_module, port_assignment.m_assignment);
                                if (!right_port.empty())
                                {
                                    std::vector<std::string> left_port;

                                    const auto& port_name = port_assignment.m_port_name.value();
                                    if (const auto group_it = pin_groups.find(port_name); group_it != pin_groups.end())
                                    {
                                        left_port = group_it->second;
                                    }
                                    else
                                    {
                                        left_port.push_back(port_name);
                                    }

                                    u32 max_size = right_port.size() <= left_port.size() ? right_port.size() : left_port.size();

                                    for (u32 i = 0; i < max_size; i++)
                                    {
                                        instance->m_expanded_port_assignments.push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                                    }
                                }
                            }
                        }
                        // all port assignments by order
                        else
                        {
                            // cache pins
                            std::vector<std::string> pins = gate_type_it->second->get_pin_names();
                            auto pin_it                   = pins.begin();

                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                std::vector<std::string> right_port = expand_assignment_expression(verilog_module, port_assignment.m_assignment);
                                if (!right_port.empty())
                                {
                                    std::vector<std::string> left_port;

                                    for (u32 i = 0; i < right_port.size() && pin_it != pins.end(); i++)
                                    {
                                        left_port.push_back(*pin_it++);
                                    }

                                    u32 max_size = right_port.size() <= left_port.size() ? right_port.size() : left_port.size();

                                    for (u32 i = 0; i < max_size; i++)
                                    {
                                        instance->m_expanded_port_assignments.push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // for the top module, generate global i/o signals for all ports
        std::unordered_map<std::string, std::string> top_assignments;
        for (const auto& port : top_module->m_ports)
        {
            for (const auto& expanded_port_identifier : port->m_expanded_identifiers)
            {
                Net* global_port_net = m_netlist->create_net(expanded_port_identifier);
                if (global_port_net == nullptr)
                {
                    return ERR("could not construct netlist: failed to create global I/O net '" + expanded_port_identifier + "'");
                }

                m_net_by_name[expanded_port_identifier] = global_port_net;

                // assign global port nets to ports of top module
                top_assignments[expanded_port_identifier] = expanded_port_identifier;

                if (port->m_direction == PinDirection::input || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_input_net())
                    {
                        return ERR("could not construct netlist: failed to mark global I/O net '" + expanded_port_identifier + "' as global input");
                    }
                }

                if (port->m_direction == PinDirection::output || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_output_net())
                    {
                        return ERR("could not construct netlist: failed to mark global I/O net '" + expanded_port_identifier + "' as global output");
                    }
                }
            }
        }

        if (auto res = instantiate_module("top_module", top_module, nullptr, top_assignments); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not construct netlist: unable to instantiate top module");
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
                        Gate* src_gate   = src->get_gate();
                        GatePin* src_pin = src->get_pin();

                        if (!slave_net->remove_source(src))
                        {
                            return ERR("could not construct netlist: failed to remove source from net '" + slave_net->get_name() + "' with ID " + std::to_string(slave_net->get_id()));
                        }

                        if (!master_net->is_a_source(src_gate, src_pin))
                        {
                            if (!master_net->add_source(src_gate, src_pin))
                            {
                                return ERR("could not construct netlist: failed to add source to net '" + master_net->get_name() + "' with ID " + std::to_string(master_net->get_id()));
                            }
                        }
                    }

                    // merge destinations
                    if (slave_net->is_global_output_net())
                    {
                        master_net->mark_global_output_net();
                    }

                    for (auto dst : slave_net->get_destinations())
                    {
                        Gate* dst_gate   = dst->get_gate();
                        GatePin* dst_pin = dst->get_pin();

                        if (!slave_net->remove_destination(dst))
                        {
                            return ERR("could not construct netlist: failed to remove destination from net '" + slave_net->get_name() + "' with ID " + std::to_string(slave_net->get_id()));
                        }

                        if (!master_net->is_a_destination(dst_gate, dst_pin))
                        {
                            if (!master_net->add_destination(dst_gate, dst_pin))
                            {
                                return ERR("could not construct netlist: failed to add destination to net '" + master_net->get_name() + "' with ID " + std::to_string(master_net->get_id()));
                            }
                        }
                    }

                    // merge generics and attributes
                    for (const auto& [identifier, content] : slave_net->get_data_map())
                    {
                        if (!master_net->set_data(std::get<0>(identifier), std::get<1>(identifier), std::get<0>(content), std::get<1>(content)))
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
                    if (const auto it = m_module_port_by_net.find(slave_net); it != m_module_port_by_net.end())
                    {
                        for (auto [module, index] : it->second)
                        {
                            std::get<1>(m_module_ports.at(module).at(index)) = master_net;
                        }
                        m_module_port_by_net[master_net].insert(m_module_port_by_net[master_net].end(), it->second.begin(), it->second.end());
                        m_module_port_by_net.erase(it);
                    }

                    m_netlist->delete_net(slave_net);
                    m_net_by_name.erase(slave);
                }

                m_nets_to_merge.erase(master);
                progress_made = true;
                break;
            }

            if (!progress_made)
            {
                return ERR("could not construct netlist: cyclic dependency between signals detected");
            }
        }

        // update module nets, internal nets, input nets, and output nets
        for (Module* module : m_netlist->get_modules())
        {
            module->update_nets();
        }

        // assign module pins
        for (const auto& [module, ports] : m_module_ports)
        {
            for (const auto& [port_name, port_net] : ports)
            {
                if (port_net->get_num_of_sources() == 0 && port_net->get_num_of_destinations() == 0)
                {
                    continue;
                }

                if (auto res = module->create_pin(port_name, port_net); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not construct netlist: failed to create pin '" + port_name + "' at net '" + port_net->get_name() + "' with ID " + std::to_string(port_net->get_id())
                                          + " within module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()));
                    // TODO: The pin creation fails when there are unused ports that never get a net assigned to them (verliog...),
                    //       but this also happens when the net just passes through the module (since there is no gate inside the module with that net as either input or output net, the net does not get listed as module input or output)
                }
            }
        }

        for (Module* module : m_netlist->get_modules())
        {
            std::unordered_set<Net*> input_nets  = module->get_input_nets();
            std::unordered_set<Net*> output_nets = module->get_input_nets();

            if (!module->get_pin_by_net(m_one_net) && (input_nets.find(m_one_net) != input_nets.end() || output_nets.find(m_one_net) != input_nets.end()))
            {
                if (auto res = module->create_pin("'1'", m_one_net); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not construct netlist: failed to create pin '1' at net '" + m_one_net->get_name() + "' with ID " + std::to_string(m_one_net->get_id()) + "within module '"
                                          + module->get_name() + "' with ID " + std::to_string(module->get_id()));
                }
            }

            if (!module->get_pin_by_net(m_zero_net) && (input_nets.find(m_zero_net) != input_nets.end() || output_nets.find(m_zero_net) != input_nets.end()))
            {
                if (auto res = module->create_pin("'0'", m_zero_net); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not construct netlist: failed to create pin '0' at net '" + m_zero_net->get_name() + "' with ID " + std::to_string(m_zero_net->get_id())
                                          + "within module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()));
                }
            }
        }

        m_netlist->enable_automatic_net_checks(true);
        return OK({});
    }

    Result<Module*> VerilogParser::instantiate_module(const std::string& instance_identifier,
                                                      VerilogModule* verilog_module,
                                                      Module* parent,
                                                      const std::unordered_map<std::string, std::string>& parent_module_assignments)
    {
        std::unordered_map<std::string, std::string> signal_alias;
        std::unordered_map<std::string, std::string> instance_alias;

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

        std::string instance_type = verilog_module->m_name;
        if (module == nullptr)
        {
            return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to create module");
        }
        module->set_type(instance_type);

        // assign entity-level attributes
        for (const VerilogDataEntry& attribute : verilog_module->m_attributes)
        {
            if (!module->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
            {
                log_warning("verilog_parser",
                            "could not set attribute '{} = {}' of type '{}' for instance '{}' type '{}'.",
                            attribute.m_name,
                            attribute.m_value,
                            attribute.m_type,
                            instance_identifier,
                            instance_type);
            }
        }

        // assign module port names and attributes
        for (const auto& port : verilog_module->m_ports)
        {
            for (const auto& expanded_port_identifier : port->m_expanded_identifiers)
            {
                if (const auto it = parent_module_assignments.find(expanded_port_identifier); it != parent_module_assignments.end())
                {
                    Net* port_net = m_net_by_name.at(it->second);
                    m_module_ports[module].push_back(std::make_pair(expanded_port_identifier, port_net));
                    m_module_port_by_net[port_net].push_back(std::make_pair(module, m_module_ports[module].size() - 1));

                    // assign port attributes
                    for (const VerilogDataEntry& attribute : port->m_attributes)
                    {
                        if (!port_net->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
                        {
                            log_warning("verilog_parser",
                                        "could not set attribute '{} = {}' of type '{}' for port '{}' of instance '{}' of type '{}'.",
                                        attribute.m_name,
                                        attribute.m_value,
                                        attribute.m_type,
                                        expanded_port_identifier,
                                        instance_identifier,
                                        instance_type);
                        }
                    }
                }
            }
        }

        // create internal signals
        for (const auto& signal : verilog_module->m_signals)
        {
            for (const auto& expanded_name : signal->m_expanded_names)
            {
                signal_alias[expanded_name] = get_unique_alias(m_signal_name_occurrences, expanded_name);

                // create new net for the signal
                Net* signal_net = m_netlist->create_net(signal_alias.at(expanded_name));
                if (signal_net == nullptr)
                {
                    return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to create net '" + expanded_name + "'");
                }

                m_net_by_name[signal_alias.at(expanded_name)] = signal_net;

                // assign signal attributes
                for (const VerilogDataEntry& attribute : signal->m_attributes)
                {
                    if (!signal_net->set_data("attribute", attribute.m_name, "unknown", attribute.m_value))
                    {
                        log_warning("verilog_parser",
                                    "could not set attribute ({} = {}) for net '{}' of instance '{}' of type '{}'.",
                                    attribute.m_name,
                                    attribute.m_value,
                                    expanded_name,
                                    instance_identifier,
                                    instance_type);
                    }
                }
            }
        }

        // schedule assigned nets for merging
        for (const auto& [left_expanded_signal, right_expanded_signal] : verilog_module->m_expanded_assignments)
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
                return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to find alias for net '" + a + "'");
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
                return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to find alias for net '" + b + "'");
            }

            m_nets_to_merge[b].push_back(a);
        }

        // process instances i.e. gates or other entities
        for (const auto& instance : verilog_module->m_instances)
        {
            // will later hold either module or gate, so attributes can be assigned properly
            DataContainer* container = nullptr;

            // assign actual signal names to ports
            std::unordered_map<std::string, std::string> instance_assignments;

            // if the instance is another entity, recursively instantiate it
            if (auto module_it = m_modules_by_name.find(instance->m_type); module_it != m_modules_by_name.end())
            {
                // expand port assignments
                for (const auto& [port, assignment] : instance->m_expanded_port_assignments)
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
                        else if (assignment == "'Z'" || assignment == "'X'" || assignment.empty())
                        {
                            continue;
                        }
                        else if (verilog_module->m_expanded_port_expressions.find(assignment) != verilog_module->m_expanded_port_expressions.end())
                        {
                            continue;
                        }
                        else
                        {
                            return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': port assignment '" + port + " = " + assignment + "' is invalid");
                        }
                    }
                }

                if (auto res = instantiate_module(instance->m_name, module_it->second, module, instance_assignments); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not create instance '" + instance_identifier + "' of type '" + instance_type + "': unable to create instance '" + instance->m_name + "' of type '"
                                          + module_it->second->m_name + "'");
                }
                else
                {
                    container = res.get();
                }
            }
            // otherwise it has to be an element from the gate library
            else if (const auto gate_type_it = m_gate_types.find(instance->m_type); gate_type_it != m_gate_types.end())
            {
                // create the new gate
                instance_alias[instance->m_name] = get_unique_alias(m_instance_name_occurrences, instance->m_name);

                Gate* new_gate = m_netlist->create_gate(gate_type_it->second, instance_alias.at(instance->m_name));
                if (new_gate == nullptr)
                {
                    return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to create gate '" + instance->m_name + "'");
                }

                if (!module->is_top_module())
                {
                    module->assign_gate(new_gate);
                }

                container = new_gate;

                // if gate is of a GND or VCC gate type, mark it as such
                if (m_vcc_gate_types.find(instance->m_type) != m_vcc_gate_types.end() && !new_gate->mark_vcc_gate())
                {
                    return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to mark '" + instance->m_name + "' of type '" + instance->m_type
                               + "' as GND gate");
                }
                if (m_gnd_gate_types.find(instance->m_type) != m_gnd_gate_types.end() && !new_gate->mark_gnd_gate())
                {
                    return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to mark '" + instance->m_name + "' of type '" + instance->m_type
                               + "' as VCC gate");
                }

                // cache pin names
                std::unordered_map<std::string, GatePin*> pin_names_map;
                for (auto* pin : gate_type_it->second->get_pins())
                {
                    pin_names_map[pin->get_name()] = pin;
                }

                // expand pin assignments
                for (const auto& [pin, assignment] : instance->m_expanded_port_assignments)
                {
                    std::string signal;

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
                    else if (verilog_module->m_expanded_port_expressions.find(assignment) != verilog_module->m_expanded_port_expressions.end())
                    {
                        continue;
                    }
                    else
                    {
                        return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to assign '" + assignment + "' to pin '" + pin + "' of gate '"
                                   + instance->m_name + "' of type '" + instance->m_type + "' as the assignment is invalid");
                    }

                    // get the respective net for the assignment
                    if (const auto net_it = m_net_by_name.find(signal); net_it == m_net_by_name.end())
                    {
                        return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to assign signal'" + signal + "' to pin '" + pin
                                   + "' as the signal has not been declared");
                    }
                    else
                    {
                        Net* current_net = net_it->second;

                        // add net src/dst by pin types
                        bool is_input  = false;
                        bool is_output = false;

                        if (const auto it = pin_names_map.find(pin); it != pin_names_map.end())
                        {
                            PinDirection direction = it->second->get_direction();
                            if (direction == PinDirection::input || direction == PinDirection::inout)
                            {
                                is_input = true;
                            }

                            if (direction == PinDirection::output || direction == PinDirection::inout)
                            {
                                is_output = true;
                            }
                        }

                        if (!is_input && !is_output)
                        {
                            return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to assign net '" + signal + "' to pin '" + pin
                                       + "' as it is not a pin of gate '" + new_gate->get_name() + "' of type '" + new_gate->get_type()->get_name() + "'");
                        }

                        if (is_output && !current_net->add_source(new_gate, pin))
                        {
                            return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to add net '" + signal + "' as a source to gate '"
                                       + new_gate->get_name() + "' via pin '" + pin + "'");
                        }

                        if (is_input && !current_net->add_destination(new_gate, pin))
                        {
                            return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to add net '" + signal + "' as a destination to gate '"
                                       + new_gate->get_name() + "' via pin '" + pin + "'");
                        }
                    }
                }
            }
            else
            {
                return ERR("could not create instance '" + instance_identifier + "' of type '" + instance_type + "': failed to find gate type '" + instance->m_type + "' in gate library '"
                           + m_netlist->get_gate_library()->get_name() + "'");
            }

            // assign instance attributes
            for (const auto& attribute : instance->m_attributes)
            {
                if (!container->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
                {
                    log_warning("verilog_parser",
                                "could not set attribute '{} = {}' of type '{}' for instance '{}' of type '{}' within instance '{}' of type '{}'.",
                                attribute.m_name,
                                attribute.m_value,
                                attribute.m_type,
                                instance->m_name,
                                instance->m_type,
                                instance_identifier,
                                instance_type);
                }
            }

            // process generics
            for (const auto& parameter : instance->m_parameters)
            {
                if (!container->set_data("generic", parameter.m_name, parameter.m_type, parameter.m_value))
                {
                    log_warning("verilog_parser",
                                "could not set generic '{} = {}' of type '{}' for instance '{}' of type '{}' within instance '{}' of type '{}'.",
                                parameter.m_name,
                                parameter.m_value,
                                parameter.m_type,
                                instance->m_name,
                                instance->m_type,
                                instance_identifier,
                                instance_type);
                }
            }
        }

        return OK(module);
    }

    // ###########################################################################
    // ###################          Helper Functions          ####################
    // ###########################################################################

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
