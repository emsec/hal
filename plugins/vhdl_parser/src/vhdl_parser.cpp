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
    namespace
    {
        std::vector<u32> parse_range(TokenStream<ci_string>& range_stream)
        {
            if (range_stream.remaining() == 1)
            {
                return {(u32)std::stoi(core_strings::to<std::string>(range_stream.consume().string))};
            }

            int direction = 1;
            const int end = std::stoi(core_strings::to<std::string>(range_stream.consume().string));

            if (range_stream.peek() == "downto")
            {
                range_stream.consume("downto");
            }
            else
            {
                range_stream.consume("to", true);
                direction = -1;
            }

            const int start = std::stoi(core_strings::to<std::string>(range_stream.consume().string));

            std::vector<u32> res;
            for (int i = start; i != end + direction; i += direction)
            {
                res.push_back((u32)i);
            }
            return res;
        }

        const static std::map<core_strings::CaseInsensitiveString, size_t> id_to_dim = {{"std_logic_vector", 1}, {"std_logic_vector2", 2}, {"std_logic_vector3", 3}};

        Result<std::vector<std::vector<u32>>> parse_signal_ranges(TokenStream<ci_string>& signal_stream)
        {
            std::vector<std::vector<u32>> ranges;
            const u32 line_number = signal_stream.peek().number;

            const Token<ci_string> type_name = signal_stream.consume();
            if (type_name == "std_logic")
            {
                return OK(ranges);
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
                    return ERR("could not parse signal ranges: mismatch of dimensions (line " + std::to_string(line_number) + ")");
                }
            }
            else
            {
                return ERR("could not parse signal ranges: type name '" + core_strings::to<std::string>(type_name.string) + "' is invalid (line " + std::to_string(line_number) + ")");
            }

            return OK(ranges);
        }

        void expand_ranges_recursively(std::vector<ci_string>& expanded_names, const ci_string& current_name, const std::vector<std::vector<u32>>& ranges, u32 dimension)
        {
            // expand signal recursively
            if (ranges.size() > dimension)
            {
                for (const u32 index : ranges[dimension])
                {
                    expand_ranges_recursively(expanded_names, current_name + "(" + core_strings::to<ci_string>(std::to_string(index)) + ")", ranges, dimension + 1);
                }
            }
            else
            {
                // last dimension
                expanded_names.push_back(current_name);
            }
        }

        std::vector<ci_string> expand_ranges(const ci_string& name, const std::vector<std::vector<u32>>& ranges)
        {
            std::vector<ci_string> res;

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

            std::string prefix;
            std::string number;
            std::vector<BooleanFunction::Value> result;

            // base specified?
            if (value.at(0) != '\"')
            {
                prefix = value.at(0);
                number = value.substr(2, value.rfind('\"') - 2);
            }
            else
            {
                prefix = "B";
                number = value.substr(1, value.rfind('\"') - 1);
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

            return OK(result);
        }

        Result<std::string> get_hex_from_literal(const Token<ci_string>& value_token)
        {
            const u32 line_number = value_token.number;
            const ci_string value = utils::to_upper(utils::replace(value_token.string, ci_string("_"), ci_string("")));

            ci_string prefix;
            ci_string number;
            u32 base;

            // base specified?
            if (value.at(0) != '\"')
            {
                prefix = value.at(0);
                number = value.substr(2, value.rfind('\"') - 2);
            }
            else
            {
                prefix = "B";
                number = value.substr(1, value.rfind('\"') - 1);
            }

            // select base
            switch (prefix.at(0))
            {
                case 'B': {
                    if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '1'); }))
                    {
                        return ERR("could not convert token to hexadecimal string: invalid character within binary number literal '" + core_strings::to<std::string>(value) + "' (line "
                                   + std::to_string(line_number) + ")");
                    }

                    base = 2;
                    break;
                }

                case 'O': {
                    if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '7'); }))
                    {
                        return ERR("could not convert token to hexadecimal string: invalid character within ocatl number literal '" + core_strings::to<std::string>(value) + "' (line "
                                   + std::to_string(line_number) + ")");
                    }

                    base = 8;
                    break;
                }

                case 'D': {
                    if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9'); }))
                    {
                        return ERR("could not convert token to hexadecimal string: invalid character within decimal number literal '" + core_strings::to<std::string>(value) + "' (line "
                                   + std::to_string(line_number) + ")");
                    }

                    base = 10;
                    break;
                }

                case 'X': {
                    std::string res;

                    for (const char c : number)
                    {
                        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))
                        {
                            res += c;
                        }
                        else
                        {
                            return ERR("could not convert token to hexadecimal string: invalid character within hexadecimal number literal '" + core_strings::to<std::string>(value) + "' (line "
                                       + std::to_string(line_number) + ")");
                        }
                    }

                    return OK(res);
                }

                default: {
                    return ERR("could not convert token to hexadecimal string: invalid base '" + core_strings::to<std::string>(prefix) + "' within number literal '"
                               + core_strings::to<std::string>(value) + "' (line " + std::to_string(line_number) + ")");
                }
            }

            std::stringstream ss;
            ss << std::uppercase << std::hex << stoull(core_strings::to<std::string>(number), 0, base);
            return OK(ss.str());
        }

        Result<std::vector<assignment_t>> parse_assignment_expression(TokenStream<ci_string>&& stream)
        {
            // PARSE ASSIGNMENT
            //   assignment can currently be one of the following:
            //   (1) NAME
            //   (2) NUMBER
            //   (3) NAME(INDEX1, INDEX2, ...)
            //   (4) NAME(BEGIN_INDEX1 to/downto END_INDEX1, BEGIN_INDEX2 to/downto END_INDEX2, ...)
            //   (5) ((1 - 4), (1 - 4), ...)

            std::vector<TokenStream<ci_string>> parts;

            if (stream.size() == 0)
            {
                return OK({});
            }

            // (5) ((1 - 4), (1 - 4), ...)
            if (stream.consume("("))
            {
                do
                {
                    parts.push_back(stream.extract_until(","));
                } while (stream.consume(",", false));

                stream.consume(")", true);
            }
            else
            {
                parts.push_back(stream);
            }

            std::vector<assignment_t> result;
            result.reserve(parts.size());

            for (auto it = parts.rbegin(); it != parts.rend(); it++)
            {
                TokenStream<ci_string>& part_stream = *it;

                const Token<ci_string> signal_name_token = part_stream.consume();
                ci_string signal_name                    = signal_name_token.string;

                // (2) NUMBER
                if (utils::starts_with(signal_name, core_strings::CaseInsensitiveString("\"")) || utils::starts_with(signal_name, core_strings::CaseInsensitiveString("b\""))
                    || utils::starts_with(signal_name, core_strings::CaseInsensitiveString("o\"")) || utils::starts_with(signal_name, core_strings::CaseInsensitiveString("x\"")))
                {
                    if (auto res = get_binary_vector(core_strings::to<std::string>(signal_name_token.string)); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not expand assignment signal: unable to convert literal to binary string (line " + std::to_string(signal_name_token.number) + ")");
                    }
                    else
                    {
                        result.push_back(std::move(res.get()));
                    }
                }
                else if (signal_name == "'0'")
                {
                    result.push_back(numeral_t({BooleanFunction::Value::ZERO}));
                }
                else if (signal_name == "'1'")
                {
                    result.push_back(numeral_t({BooleanFunction::Value::ONE}));
                }
                else if (signal_name == "'X'")
                {
                    result.push_back(numeral_t({BooleanFunction::Value::X}));
                }
                else if (signal_name == "'Z'")
                {
                    result.push_back(numeral_t({BooleanFunction::Value::Z}));
                }
                else
                {
                    // (3) NAME(INDEX1, INDEX2, ...)
                    // (4) NAME(BEGIN_INDEX1 to/downto END_INDEX1, BEGIN_INDEX2 to/downto END_INDEX2, ...)
                    if (part_stream.consume("("))
                    {
                        std::vector<std::vector<u32>> ranges;
                        u32 closing_pos = part_stream.find_next(")");
                        do
                        {
                            TokenStream<ci_string> range_stream = part_stream.extract_until(",", closing_pos);
                            ranges.emplace_back(parse_range(range_stream));

                        } while (part_stream.consume(",", false));
                        part_stream.consume(")", true);
                        result.push_back(ranged_identifier_t({std::move(signal_name), std::move(ranges)}));
                    }
                    else
                    {
                        // (1) NAME
                        result.push_back(std::move(signal_name));
                    }
                }
            }

            return OK(result);
        }

        Result<std::vector<ci_string>> expand_assignment_expression(VhdlEntity* vhdl_entity, const std::vector<assignment_t>& vars)
        {
            std::vector<ci_string> result;
            for (const auto& var : vars)
            {
                if (const identifier_t* identifier = std::get_if<identifier_t>(&var); identifier != nullptr)
                {
                    std::vector<std::vector<u32>> ranges;

                    if (const auto signal_it = vhdl_entity->m_signals_by_name.find(*identifier); signal_it != vhdl_entity->m_signals_by_name.end())
                    {
                        ranges = signal_it->second->m_ranges;
                    }
                    else if (const auto port_it = vhdl_entity->m_ports_by_identifier.find(*identifier); port_it != vhdl_entity->m_ports_by_identifier.end())
                    {
                        ranges = port_it->second->m_ranges;
                    }
                    else
                    {
                        return ERR("could not expand assignment expression': '" + core_strings::to<std::string>(*identifier) + "' is neither a signal nor a port of entity '"
                                   + core_strings::to<std::string>(vhdl_entity->m_name) + "'");
                    }

                    std::vector<ci_string> expanded = expand_ranges(*identifier, ranges);
                    result.insert(result.end(), expanded.begin(), expanded.end());
                }
                else if (const ranged_identifier_t* ranged_identifier = std::get_if<ranged_identifier_t>(&var); ranged_identifier != nullptr)
                {
                    std::vector<ci_string> expanded = expand_ranges(ranged_identifier->first, ranged_identifier->second);
                    result.insert(result.end(), expanded.begin(), expanded.end());
                }
                else if (const numeral_t* numeral = std::get_if<numeral_t>(&var); numeral != nullptr)
                {
                    for (auto value : *numeral)
                    {
                        result.push_back(core_strings::to<ci_string>("'" + BooleanFunction::to_string(value) + "'"));
                    }
                }
            }

            return OK(result);
        }

    }    // namespace

    Result<std::monostate> VHDLParser::parse(const std::filesystem::path& file_path)
    {
        m_path = file_path;
        m_entities.clear();
        m_attribute_buffer.clear();
        m_attribute_types.clear();

        {
            std::ifstream ifs;
            ifs.open(file_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                return ERR("unable to open VHDL file '" + file_path.string() + "'");
            }
            m_fs << ifs.rdbuf();
            ifs.close();
        }

        // tokenize file
        tokenize();

        // parse tokens into intermediate format
        try
        {
            if (const auto res = parse_tokens(); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse VHDL file '" + m_path.string() + "'");
            }
        }
        catch (TokenStream<ci_string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                return ERR("could not parse VHDL file '" + m_path.string() + "': " + core_strings::to<std::string>(e.message) + " near line " + std::to_string(e.line_number));
            }
            else
            {
                return ERR("could not parse VHDL file '" + m_path.string() + "': " + core_strings::to<std::string>(e.message));
            }
        }

        if (m_entities.empty())
        {
            return ERR("could not parse VHDL file '" + m_path.string() + "': it does not contain any entities");
        }

        // expand module port identifiers, signals, and assignments
        for (auto& [entity_name, vhdl_entity] : m_entities_by_name)
        {
            // expand port identifiers
            for (const auto& port : vhdl_entity->m_ports)
            {
                if (!port->m_ranges.empty())
                {
                    port->m_expanded_identifiers = expand_ranges(port->m_identifier, port->m_ranges);
                    vhdl_entity->m_expanded_port_identifiers.insert(port->m_expanded_identifiers.begin(), port->m_expanded_identifiers.end());
                }
                else
                {
                    port->m_expanded_identifiers = {port->m_identifier};
                    vhdl_entity->m_expanded_port_identifiers.insert(port->m_identifier);
                }
            }

            // expand signals
            for (auto& signal : vhdl_entity->m_signals)
            {
                if (!signal->m_ranges.empty())
                {
                    signal->m_expanded_names = expand_ranges(signal->m_name, signal->m_ranges);
                }
                else
                {
                    signal->m_expanded_names = std::vector<ci_string>({signal->m_name});
                }
            }

            // expand assignments
            for (auto& assignment : vhdl_entity->m_assignments)
            {
                auto left_res = expand_assignment_expression(vhdl_entity, assignment.m_variable);
                if (left_res.is_error())
                {
                    return ERR_APPEND(left_res.get_error(), "could not parse VHDL file '" + m_path.string() + "': unable to expand signal assignment");
                }
                const std::vector<ci_string> left_signals = left_res.get();

                auto right_res = expand_assignment_expression(vhdl_entity, assignment.m_assignment);
                if (right_res.is_error())
                {
                    return ERR_APPEND(right_res.get_error(), "could not parse VHDL file '" + m_path.string() + "': unable to expand signal assignment");
                }
                const std::vector<ci_string> right_signals = right_res.get();

                u32 left_size  = left_signals.size();
                u32 right_size = right_signals.size();
                if (left_signals.empty() || right_signals.empty())
                {
                    return ERR("could not parse VHDL file '" + m_path.string() + "': failed to expand assignments within entity '" + core_strings::to<std::string>(entity_name) + "'");
                }
                else if (left_size != right_size)
                {
                    return ERR("could not parse VHDL file '" + m_path.string() + "': assignment width mismatch within entity '" + core_strings::to<std::string>(entity_name) + "'");
                }
                else
                {
                    for (u32 i = 0; i < right_size; i++)
                    {
                        vhdl_entity->m_expanded_assignments.push_back(std::make_pair(left_signals.at(i), right_signals.at(i)));
                    }
                }
            }
        }

        // expand entity port assignments
        for (auto& [entity_name, vhdl_entity] : m_entities_by_name)
        {
            for (auto& instance : vhdl_entity->m_instances)
            {
                if (auto entity_it = m_entities_by_name.find(instance->m_type); entity_it != m_entities_by_name.end())
                {
                    instance->m_is_entity = true;

                    if (!instance->m_port_assignments.empty())
                    {
                        if (instance->m_port_assignments.front().m_port.has_value())
                        {
                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                auto right_res = expand_assignment_expression(vhdl_entity, port_assignment.m_assignment);
                                if (right_res.is_error())
                                {
                                    return ERR_APPEND(right_res.get_error(), "could not parse VHDL file '" + m_path.string() + "': unable to expand entity port assignment");
                                }
                                const std::vector<ci_string> right_port = right_res.get();
                                if (!right_port.empty())
                                {
                                    std::vector<ci_string> left_port;
                                    if (const identifier_t* identifier = std::get_if<identifier_t>(&(port_assignment.m_port.value())); identifier != nullptr)
                                    {
                                        if (const auto it = entity_it->second->m_ports_by_identifier.find(*identifier); it != entity_it->second->m_ports_by_identifier.end())
                                        {
                                            left_port = it->second->m_expanded_identifiers;
                                        }
                                        else
                                        {
                                            return ERR("could not parse VHDL file '" + m_path.string() + "': unable to assign signal to port '" + core_strings::to<std::string>(*identifier)
                                                       + "' as it is not a port of entity '" + core_strings::to<std::string>(entity_it->first) + "'");
                                        }
                                    }
                                    else if (const ranged_identifier_t* ranged_identifier = std::get_if<ranged_identifier_t>(&(port_assignment.m_port.value())); ranged_identifier != nullptr)
                                    {
                                        left_port = expand_ranges(ranged_identifier->first, ranged_identifier->second);
                                    }
                                    else
                                    {
                                        return ERR("could not parse VHDL file '" + m_path.string() + "': unable to expand entity port assignment");
                                    }

                                    if (left_port.empty())
                                    {
                                        return ERR("could not parse VHDL file '" + m_path.string() + "': unable to expand entity port assignment");
                                    }

                                    u32 max_size = right_port.size() <= left_port.size() ? right_port.size() : left_port.size();

                                    for (u32 i = 0; i < max_size; i++)
                                    {
                                        instance->m_expanded_port_assignments.push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                                    }
                                }
                            }
                        }
                        else
                        {
                            std::vector<ci_string> ports;
                            for (const auto& port : m_entities_by_name.at(instance->m_type)->m_ports)
                            {
                                ports.insert(ports.end(), port->m_expanded_identifiers.begin(), port->m_expanded_identifiers.end());
                            }

                            auto port_it = ports.begin();

                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                auto right_res = expand_assignment_expression(vhdl_entity, port_assignment.m_assignment);
                                if (right_res.is_error())
                                {
                                    return ERR_APPEND(right_res.get_error(), "could not parse VHDL file '" + m_path.string() + "': unable to expand entity port assignment");
                                }
                                const std::vector<ci_string> right_port = right_res.get();
                                if (!right_port.empty())
                                {
                                    std::vector<ci_string> left_port;

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

    Result<std::unique_ptr<Netlist>> VHDLParser::instantiate(const GateLibrary* gate_library)
    {
        // create empty netlist
        std::unique_ptr<Netlist> result = netlist_factory::create_netlist(gate_library);
        m_netlist                       = result.get();
        if (m_netlist == nullptr)
        {
            return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create empty netlist");
        }

        m_gate_types.clear();
        m_gnd_gate_types.clear();
        m_vcc_gate_types.clear();
        m_instance_name_occurrences.clear();
        m_signal_name_occurrences.clear();
        m_net_by_name.clear();
        m_nets_to_merge.clear();
        m_module_ports.clear();
        for (const auto& vhdl_entity : m_entities)
        {
            for (const auto& instance : vhdl_entity->m_instances)
            {
                if (!instance->m_is_entity)
                {
                    instance->m_expanded_port_assignments.clear();
                }
            }
        }

        // buffer gate types
        for (const auto& [gt_name, gt] : gate_library->get_gate_types())
        {
            m_gate_types[core_strings::to<ci_string>(gt_name)] = gt;
        }
        for (const auto& [gt_name, gt] : gate_library->get_gnd_gate_types())
        {
            m_gnd_gate_types[core_strings::to<ci_string>(gt_name)] = gt;
        }
        for (const auto& [gt_name, gt] : gate_library->get_vcc_gate_types())
        {
            m_vcc_gate_types[core_strings::to<ci_string>(gt_name)] = gt;
        }

        // create const 0 and const 1 net, will be removed if unused
        m_zero_net = m_netlist->create_net("'0'");
        if (m_zero_net == nullptr)
        {
            return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create zero net");
        }
        m_net_by_name[core_strings::to<ci_string>(m_zero_net->get_name())] = m_zero_net;

        m_one_net = m_netlist->create_net("'1'");
        if (m_one_net == nullptr)
        {
            return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create one net");
        }
        m_net_by_name[core_strings::to<ci_string>(m_one_net->get_name())] = m_one_net;

        // construct the netlist with the last module being considered the top module
        std::map<ci_string, u32> entity_name_to_refereneces;
        for (const auto& [_name, vhdl_entity] : m_entities_by_name)
        {
            for (const auto& instance : vhdl_entity->m_instances)
            {
                if (const auto it = m_entities_by_name.find(instance->m_type); it != m_entities_by_name.end())
                {
                    entity_name_to_refereneces[it->first]++;
                }
            }
        }

        std::vector<ci_string> top_module_candidates;
        for (const auto& [name, _entity] : m_entities_by_name)
        {
            if (entity_name_to_refereneces.find(name) == entity_name_to_refereneces.end())
            {
                top_module_candidates.push_back(name);
            }
        }

        if (top_module_candidates.empty())
        {
            return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': unable to find any top module candidates");
        }

        if (top_module_candidates.size() > 1)
        {
            return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': found multiple entities as candidates for the top module");
        }

        // construct the netlist with the the top module
        VhdlEntity* top_entity = m_entities_by_name.at(top_module_candidates.front());

        if (const auto res = construct_netlist(top_entity); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "'");
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
                    return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to mark GND gate");
                }

                if (!m_zero_net->add_source(gnd, output_pin))
                {
                    return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': could not add source to GND gate");
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
                    return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to mark VCC gate");
                }

                if (!m_one_net->add_source(vcc, output_pin))
                {
                    return ERR("could not instantiate VHDL netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': could not add source to VCC gate");
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

    void VHDLParser::tokenize()
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
    }

    Result<std::monostate> VHDLParser::parse_tokens()
    {
        while (m_token_stream.remaining() > 0)
        {
            if (m_token_stream.peek() == "library" || m_token_stream.peek() == "use")
            {
                parse_library();
            }
            else if (m_token_stream.peek() == "entity")
            {
                if (const auto res = parse_entity(); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens");
                }
            }
            else if (m_token_stream.peek() == "architecture")
            {
                if (const auto res = parse_architecture(); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens");
                }
            }
            else
            {
                return ERR("could not parse tokens: unexpected token '" + core_strings::to<std::string>(m_token_stream.peek().string) + "' in global scope (line "
                           + std::to_string(m_token_stream.peek().number) + ")");
            }
        }

        return OK({});
    }

    void VHDLParser::parse_library()
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
    }

    Result<std::monostate> VHDLParser::parse_entity()
    {
        m_token_stream.consume("entity", true);
        const u32 line_number       = m_token_stream.peek().number;
        const ci_string entity_name = m_token_stream.consume().string;

        // verify entity name
        if (const auto it = m_entities_by_name.find(entity_name); it != m_entities_by_name.end())
        {
            return ERR("could not parse entity '" + core_strings::to<std::string>(entity_name) + "' (line " + std::to_string(line_number) + "): an entity with name '"
                       + core_strings::to<std::string>(entity_name) + "' does already exist (line " + std::to_string(it->second->m_line_number) + ")");
        }

        m_token_stream.consume("is", true);
        auto vhdl_entity               = std::make_unique<VhdlEntity>();
        VhdlEntity* vhdl_entity_raw    = vhdl_entity.get();
        vhdl_entity_raw->m_line_number = line_number;
        vhdl_entity_raw->m_name        = entity_name;

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
                if (const auto res = parse_port_definitons(vhdl_entity_raw); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not parse entity '" + core_strings::to<std::string>(entity_name) + "': failed to parse port definition (line " + std::to_string(next_token.number) + ")");
                }
            }
            else if (next_token == "attribute")
            {
                if (const auto res = parse_attribute(); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not parse entity '" + core_strings::to<std::string>(entity_name) + "': failed to parse attribute (line " + std::to_string(next_token.number) + ")");
                }
            }
            else
            {
                return ERR("could not parse entity '" + core_strings::to<std::string>(entity_name) + "': unexpected token '" + core_strings::to<std::string>(next_token.string)
                           + "' in entity definition (line " + std::to_string(next_token.number) + ")");
            }

            next_token = m_token_stream.peek();
        }

        m_token_stream.consume("end", true);
        m_token_stream.consume();
        m_token_stream.consume(";", true);

        // add to collection of entities
        m_entities.push_back(std::move(vhdl_entity));
        m_entities_by_name[entity_name] = vhdl_entity_raw;
        m_last_entity                   = entity_name;

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_port_definitons(VhdlEntity* vhdl_entity)
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
                return ERR("could not parse port definitions: invalid direction '" + core_strings::to<std::string>(direction_str) + "' for port declaration (line " + std::to_string(line_number)
                           + ")");
            }

            // extract ranges
            TokenStream<ci_string> port_stream = port_def_stream.extract_until(";");
            std::vector<std::vector<u32>> ranges;
            if (auto res = parse_signal_ranges(port_stream); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse port definitions: unable to parse signal ranges (line " + std::to_string(line_number) + ")");
            }
            else
            {
                ranges = res.get();
            }

            port_def_stream.consume(";", port_def_stream.remaining() > 0);    // last entry has no semicolon, so no throw in that case

            for (const ci_string& port_name : port_names)
            {
                auto port                                              = std::make_unique<VhdlPort>();
                port->m_identifier                                     = port_name;
                port->m_direction                                      = direction;
                port->m_ranges                                         = ranges;
                vhdl_entity->m_ports_by_identifier[port->m_identifier] = port.get();
                vhdl_entity->m_ports.push_back(std::move(port));
            }
        }

        m_token_stream.consume(")", true);
        m_token_stream.consume(";", true);

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_attribute()
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
                return OK({});
            }

            VhdlDataEntry attribute;
            attribute.m_name  = core_strings::to<std::string>(attribute_name);
            attribute.m_type  = core_strings::to<std::string>(attribute_type);
            attribute.m_value = core_strings::to<std::string>(attribute_value);
            m_attribute_buffer[target_class].emplace(attribute_target, attribute);
        }
        else
        {
            return ERR("could not parse attribute: malformed attribute definition (line " + std::to_string(line_number) + ")");
        }

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_architecture()
    {
        m_token_stream.consume("architecture", true);
        m_token_stream.consume();
        m_token_stream.consume("of", true);

        u32 line_number              = m_token_stream.peek().number;
        const auto entity_name_token = m_token_stream.consume();

        if (const auto it = m_entities_by_name.find(entity_name_token.string); it == m_entities_by_name.end())
        {
            return ERR("could not parse architecture: architecture refers to non-existent entity '" + core_strings::to<std::string>(entity_name_token.string) + "' (line "
                       + std::to_string(entity_name_token.number) + ")");
        }
        else
        {
            VhdlEntity* vhdl_entity = it->second;

            m_token_stream.consume("is", true);

            if (const auto res = parse_architecture_header(vhdl_entity); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse architecture: unable to parse architecture header (line " + std::to_string(line_number) + ")");
            }
            if (const auto res = parse_architecture_body(vhdl_entity); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse architecture: unable to parse architecture body (line " + std::to_string(line_number) + ")");
            }
            if (const auto res = assign_attributes(vhdl_entity); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse architecture: unable to assign attributes (line " + std::to_string(line_number) + ")");
            }

            return OK({});
        }
    }

    Result<std::monostate> VHDLParser::parse_architecture_header(VhdlEntity* vhdl_entity)
    {
        auto next_token = m_token_stream.peek();
        while (next_token != "begin")
        {
            if (next_token == "signal")
            {
                if (const auto res = parse_signal_definition(vhdl_entity); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse architecture header: unable to parse signal definition (line " + std::to_string(next_token.number) + ")");
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
                if (const auto res = parse_attribute(); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse architecture header: unable to parse attribute (line " + std::to_string(next_token.number) + ")");
                }
            }
            else
            {
                return ERR("could not parse architecture header: unexpected token '" + core_strings::to<std::string>(next_token.string) + "' (line " + std::to_string(next_token.number) + ")");
            }

            next_token = m_token_stream.peek();
        }

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_signal_definition(VhdlEntity* vhdl_entity)
    {
        m_token_stream.consume("signal", true);
        u32 line_number = m_token_stream.peek().number;

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
        if (auto res = parse_signal_ranges(signal_stream); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not parse signal definition: unable to parse signal ranges (line " + std::to_string(line_number) + ")");
        }
        else
        {
            ranges = res.get();
        }

        m_token_stream.consume(";", true);

        for (const auto& signal_name : signal_names)
        {
            auto signal                                 = std::make_unique<VhdlSignal>();
            signal->m_name                              = signal_name;
            signal->m_ranges                            = ranges;
            vhdl_entity->m_signals_by_name[signal_name] = signal.get();
            vhdl_entity->m_signals.push_back(std::move(signal));
        }

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_architecture_body(VhdlEntity* vhdl_entity)
    {
        m_token_stream.consume("begin", true);
        auto next_token = m_token_stream.peek();

        while (next_token != "end")
        {
            // new instance found
            if (m_token_stream.peek(1) == ":")
            {
                if (const auto res = parse_instance(vhdl_entity); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not parse architecture body of entity '" + core_strings::to<std::string>(vhdl_entity->m_name) + "': unable to parse instance (line "
                                          + std::to_string(next_token.number) + ")");
                }
            }
            // not in instance -> has to be a direct assignment
            else if (m_token_stream.find_next("<=") < m_token_stream.find_next(";"))
            {
                if (const auto res = parse_assignment(vhdl_entity); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not parse architecture body of entity '" + core_strings::to<std::string>(vhdl_entity->m_name) + "': unable to parse assignment (line "
                                          + std::to_string(next_token.number) + ")");
                }
            }
            else
            {
                return ERR("could not parse architecture body of entity '" + core_strings::to<std::string>(vhdl_entity->m_name) + "': unexpected token '"
                           + core_strings::to<std::string>(m_token_stream.peek().string) + "' (line " + std::to_string(next_token.number) + ")");
            }

            next_token = m_token_stream.peek();
        }

        m_token_stream.consume("end", true);
        m_token_stream.consume();
        m_token_stream.consume(";", true);

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_assignment(VhdlEntity* vhdl_entity)
    {
        u32 line_number = m_token_stream.peek().number;
        VhdlAssignment assignment;

        if (auto res = parse_assignment_expression(m_token_stream.extract_until("<=")); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not parse assignment: unable to parse assignment expression (line " + std::to_string(line_number) + ")");
        }
        else
        {
            assignment.m_variable = res.get();
        }
        m_token_stream.consume("<=", true);
        if (auto res = parse_assignment_expression(m_token_stream.extract_until(";")); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not parse assignment: unable to parse assignment expression (line " + std::to_string(line_number) + ")");
        }
        else
        {
            assignment.m_assignment = res.get();
        }
        m_token_stream.consume(";", true);

        vhdl_entity->m_assignments.push_back(std::move(assignment));
        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_instance(VhdlEntity* vhdl_entity)
    {
        auto instance    = std::make_unique<VhdlInstance>();
        u32 line_number  = m_token_stream.peek().number;
        instance->m_name = m_token_stream.consume();
        m_token_stream.consume(":", true);

        // remove prefix from type
        if (m_token_stream.peek() == "entity")
        {
            m_token_stream.consume("entity", true);
            instance->m_type = m_token_stream.consume();
            if (const size_t pos = instance->m_type.find('.'); pos != std::string::npos)
            {
                instance->m_type = instance->m_type.substr(pos + 1);
            }
            if (m_entities_by_name.find(instance->m_type) == m_entities_by_name.end())
            {
                return ERR("could not parse instance '" + core_strings::to<std::string>(instance->m_name) + "' of type '" + core_strings::to<std::string>(instance->m_type) + "': entity with name '"
                           + core_strings::to<std::string>(instance->m_type) + "' does not exist (line " + std::to_string(line_number) + ")");
            }
        }
        else if (m_token_stream.peek() == "component")
        {
            m_token_stream.consume("component", true);
            instance->m_type = m_token_stream.consume();
        }
        else
        {
            instance->m_type = m_token_stream.consume();
            ci_string prefix;

            // find longest matching library prefix
            for (const auto& lib : m_libraries)
            {
                if (lib.size() > prefix.size() && utils::starts_with(instance->m_type, lib))
                {
                    prefix = lib;
                }
            }

            // remove prefix
            if (!prefix.empty())
            {
                instance->m_type = instance->m_type.substr(prefix.size());
            }
        }

        if (m_token_stream.consume("generic"))
        {
            line_number = m_token_stream.peek().number;
            if (const auto res = parse_generic_assign(instance.get()); res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "could not parse instance '" + core_strings::to<std::string>(instance->m_name) + "' of type '" + core_strings::to<std::string>(instance->m_type)
                                      + "': unable to parse generic assignment (line " + std::to_string(line_number) + ")");
            }
        }

        if (m_token_stream.peek() == "port")
        {
            if (const auto res = parse_port_assign(instance.get()); res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "could not parse instance '" + core_strings::to<std::string>(instance->m_name) + "' of type '" + core_strings::to<std::string>(instance->m_type)
                                      + "': unable to parse port assignment (line " + std::to_string(line_number) + ")");
            }
        }

        vhdl_entity->m_instances_by_name[instance->m_name] = instance.get();
        vhdl_entity->m_instances.push_back(std::move(instance));

        m_token_stream.consume(";", true);

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_port_assign(VhdlInstance* instance)
    {
        u32 line_number = m_token_stream.peek().number;
        m_token_stream.consume("port", true);
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        TokenStream<ci_string> port_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        if (port_stream.find_next("=>") != TokenStream<ci_string>::END_OF_STREAM)
        {
            while (port_stream.remaining() > 0)
            {
                TokenStream<ci_string> left_stream = port_stream.extract_until("=>");
                port_stream.consume("=>", true);
                TokenStream<ci_string> right_stream = port_stream.extract_until(",");
                port_stream.consume(",", port_stream.remaining() > 0);    // last entry has no comma

                if (!right_stream.consume("open"))
                {
                    VhdlPortAssignment port_assignment;

                    if (auto res = parse_assignment_expression(std::move(left_stream)); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse port assignment: unable to parse port (line " + std::to_string(line_number) + ")");
                    }
                    else
                    {
                        // vector can only have a single entry for left side of port assignment
                        port_assignment.m_port = res.get().front();
                    }

                    if (auto res = parse_assignment_expression(std::move(right_stream)); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse port assignment: unable to parse assignment (line " + std::to_string(line_number) + ")");
                    }
                    else
                    {
                        port_assignment.m_assignment = res.get();
                    }

                    instance->m_port_assignments.push_back(std::move(port_assignment));
                }
            }
        }
        else
        {
            while (port_stream.remaining() > 0)
            {
                TokenStream<ci_string> right_stream = port_stream.extract_until(",");
                port_stream.consume(",", port_stream.remaining() > 0);    // last entry has no comma

                if (!right_stream.consume("open"))
                {
                    VhdlPortAssignment port_assignment;

                    if (auto res = parse_assignment_expression(std::move(right_stream)); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse port assignment: unable to parse assignment (line " + std::to_string(line_number) + ")");
                    }
                    else
                    {
                        port_assignment.m_assignment = res.get();
                    }

                    instance->m_port_assignments.push_back(std::move(port_assignment));
                }
            }
        }

        return OK({});
    }

    Result<std::monostate> VHDLParser::parse_generic_assign(VhdlInstance* instance)
    {
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        TokenStream<ci_string> generic_stream = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (generic_stream.remaining() > 0)
        {
            VhdlDataEntry generic;

            generic.m_line_number = generic_stream.peek().number;
            generic.m_name        = core_strings::to<std::string>(generic_stream.join_until("=>", "").string);
            generic_stream.consume("=>", true);
            const auto rhs = generic_stream.join_until(",", "");
            generic_stream.consume(",", generic_stream.remaining() > 0);    // last entry has no comma

            // determine data type
            if ((rhs == "true") || (rhs == "false"))
            {
                generic.m_value = core_strings::to<std::string>(rhs.string);
                generic.m_type  = "boolean";
            }
            else if (utils::is_integer(rhs.string))
            {
                generic.m_value = core_strings::to<std::string>(rhs.string);
                generic.m_type  = "integer";
            }
            else if (utils::is_floating_point(rhs.string))
            {
                generic.m_value = core_strings::to<std::string>(rhs.string);
                generic.m_type  = "floating_point";
            }
            else if (utils::ends_with(rhs.string, ci_string("s")) || utils::ends_with(rhs.string, ci_string("sec")) || utils::ends_with(rhs.string, ci_string("min"))
                     || utils::ends_with(rhs.string, ci_string("hr")))
            {
                generic.m_value = core_strings::to<std::string>(rhs.string);
                generic.m_type  = "time";
            }
            else if (rhs.string.at(0) == '\"' && rhs.string.back() == '\"')
            {
                generic.m_value = core_strings::to<std::string>(rhs.string.substr(1, rhs.string.size() - 2));
                generic.m_type  = "string";
            }
            else if (rhs.string.at(0) == '\'' && rhs.string.at(2) == '\'')
            {
                generic.m_value = core_strings::to<std::string>(rhs.string.substr(1, 1));
                generic.m_type  = "bit_value";
            }
            else if (rhs.string.at(1) == '\"' && rhs.string.back() == '\"')
            {
                if (auto res = get_hex_from_literal(rhs); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse generic assignment: unable to translate token to hexadecimal string");
                }
                else
                {
                    generic.m_value = core_strings::to<std::string>(res.get());
                    generic.m_type  = "bit_vector";
                }
            }
            else
            {
                return ERR("could not parse generic assignment: unable to identify data type of generic map value '" + core_strings::to<std::string>(rhs.string) + "' in instance '"
                           + core_strings::to<std::string>(instance->m_name) + "' (line " + std::to_string(generic.m_line_number) + ")");
            }

            instance->m_generics.push_back(generic);
        }

        return OK({});
    }

    Result<std::monostate> VHDLParser::assign_attributes(VhdlEntity* vhdl_entity)
    {
        for (const auto& [target_class, attributes] : m_attribute_buffer)
        {
            // entity attributes
            if (target_class == AttributeTarget::ENTITY)
            {
                for (const auto& [target, attribute] : attributes)
                {
                    if (vhdl_entity->m_name != target)
                    {
                        return ERR("could not assign attributes: invalid attribute target '" + core_strings::to<std::string>(target) + "' within entity '"
                                   + core_strings::to<std::string>(vhdl_entity->m_name) + "' (line " + std::to_string(attribute.m_line_number) + ")");
                    }
                    else
                    {
                        vhdl_entity->m_attributes.push_back(attribute);
                    }
                }
            }
            // instance attributes
            else if (target_class == AttributeTarget::INSTANCE)
            {
                for (const auto& [target, attribute] : attributes)
                {
                    if (const auto instance_it = vhdl_entity->m_instances_by_name.find(target); instance_it == vhdl_entity->m_instances_by_name.end())
                    {
                        return ERR("could not assign attributes: invalid attribute target '" + core_strings::to<std::string>(target) + "' within entity '"
                                   + core_strings::to<std::string>(vhdl_entity->m_name) + "' (line " + std::to_string(attribute.m_line_number) + ")");
                    }
                    else
                    {
                        instance_it->second->m_attributes.push_back(attribute);
                    }
                }
            }
            // signal attributes
            else if (target_class == AttributeTarget::SIGNAL)
            {
                for (const auto& [target, attribute] : attributes)
                {
                    if (const auto signal_it = vhdl_entity->m_signals_by_name.find(target); signal_it != vhdl_entity->m_signals_by_name.end())
                    {
                        signal_it->second->m_attributes.push_back(attribute);
                    }
                    else if (const auto port_it = vhdl_entity->m_ports_by_identifier.find(target); port_it != vhdl_entity->m_ports_by_identifier.end())
                    {
                        port_it->second->m_attributes.push_back(attribute);
                    }
                    else
                    {
                        return ERR("could not assign attributes: invalid attribute target '" + core_strings::to<std::string>(target) + "' within entity '"
                                   + core_strings::to<std::string>(vhdl_entity->m_name) + "' (line " + std::to_string(attribute.m_line_number) + ")");
                    }
                }
            }
        }

        return OK({});
    }

    // ###########################################################################
    // ###########      Assemble Netlist from Intermediate Format       ##########
    // ###########################################################################

    Result<std::monostate> VHDLParser::construct_netlist(VhdlEntity* top_entity)
    {
        m_netlist->set_design_name(core_strings::to<std::string>(top_entity->m_name));
        m_netlist->enable_automatic_net_checks(false);

        std::unordered_map<ci_string, u32> instantiation_count;

        // preparations for alias: count the occurences of all names
        std::queue<VhdlEntity*> q;
        q.push(top_entity);

        // top entity instance will be named after its entity, so take into account for aliases
        m_instance_name_occurrences["top_module"]++;

        // global input/output signals will be named after ports, so take into account for aliases
        for (const auto& port : top_entity->m_ports)
        {
            for (const auto& expanded_port_identifier : port->m_expanded_identifiers)
            {
                m_signal_name_occurrences[expanded_port_identifier]++;
            }
        }

        while (!q.empty())
        {
            VhdlEntity* entity = q.front();
            q.pop();

            instantiation_count[entity->m_name]++;

            for (const auto& signal : entity->m_signals)
            {
                for (const auto& expanded_name : signal->m_expanded_names)
                {
                    m_signal_name_occurrences[expanded_name]++;
                }
            }

            for (const auto& instance : entity->m_instances)
            {
                m_instance_name_occurrences[instance->m_name]++;

                if (const auto it = m_entities_by_name.find(instance->m_type); it != m_entities_by_name.end())
                {
                    q.push(it->second);
                }
            }
        }

        for (auto& [entity_name, vhdl_entity] : m_entities_by_name)
        {
            // detect unused entities
            if (instantiation_count[entity_name] == 0)
            {
                log_warning("vhdl_parser", "entity '{}' has been defined in the netlist but is not instantiated.", entity_name);
                continue;
            }

            // expand gate pin assignments
            for (const auto& instance : vhdl_entity->m_instances)
            {
                if (const auto gate_type_it = m_gate_types.find(instance->m_type); gate_type_it != m_gate_types.end())
                {
                    if (!instance->m_port_assignments.empty())
                    {
                        // all port assignments by name
                        if (instance->m_port_assignments.front().m_port.has_value())
                        {
                            // cache pin groups
                            std::unordered_map<ci_string, std::vector<ci_string>> pin_groups;
                            for (const auto pin_group : gate_type_it->second->get_pin_groups())
                            {
                                for (const auto pin : pin_group->get_pins())
                                {
                                    pin_groups[core_strings::to<ci_string>(pin_group->get_name())].push_back(core_strings::to<ci_string>(pin->get_name()));
                                }
                            }

                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                auto right_res = expand_assignment_expression(vhdl_entity, port_assignment.m_assignment);
                                if (right_res.is_error())
                                {
                                    return ERR_APPEND(right_res.get_error(), "could not construct netlist: unable to expand gate port assignment");
                                }
                                auto right_port = right_res.get();

                                if (!right_port.empty())
                                {
                                    std::vector<ci_string> left_port;

                                    if (const identifier_t* identifier = std::get_if<identifier_t>(&(port_assignment.m_port.value())); identifier != nullptr)
                                    {
                                        if (const auto group_it = pin_groups.find(*identifier); group_it != pin_groups.end())
                                        {
                                            left_port = group_it->second;
                                        }
                                        else
                                        {
                                            left_port.push_back(*identifier);
                                        }
                                    }
                                    else if (const ranged_identifier_t* ranged_identifier = std::get_if<ranged_identifier_t>(&(port_assignment.m_port.value())); ranged_identifier != nullptr)
                                    {
                                        left_port = expand_ranges(ranged_identifier->first, ranged_identifier->second);
                                    }
                                    else
                                    {
                                        return ERR("could not construct netlist: unable to expand gate port assignment");
                                    }

                                    if (right_port.size() != left_port.size())
                                    {
                                        return ERR("could not construct netlist: pin assignment width mismatch at instance '" + core_strings::to<std::string>(instance->m_name) + "' of gate type '"
                                                   + core_strings::to<std::string>(instance->m_type) + "' within entity '" + core_strings::to<std::string>(entity_name) + "'");
                                    }

                                    for (u32 i = 0; i < left_port.size(); i++)
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
                            std::vector<ci_string> pins;
                            for (const auto pin : gate_type_it->second->get_pins())
                            {
                                pins.push_back(core_strings::to<ci_string>(pin->get_name()));
                            }
                            auto pin_it = pins.begin();

                            for (const auto& port_assignment : instance->m_port_assignments)
                            {
                                auto right_res = expand_assignment_expression(vhdl_entity, port_assignment.m_assignment);
                                if (right_res.is_error())
                                {
                                    return ERR_APPEND(right_res.get_error(), "could not construct netlist: unable to expand gate port assignment");
                                }
                                auto right_port = right_res.get();

                                if (!right_port.empty())
                                {
                                    std::vector<ci_string> left_port;

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
        std::unordered_map<ci_string, ci_string> top_assignments;
        for (const auto& port : top_entity->m_ports)
        {
            for (const auto& expanded_port_identifier : port->m_expanded_identifiers)
            {
                Net* global_port_net = m_netlist->create_net(core_strings::to<std::string>(expanded_port_identifier));
                if (global_port_net == nullptr)
                {
                    return ERR("could not construct netlist: failed to create global I/O net '" + core_strings::to<std::string>(expanded_port_identifier) + "'");
                }

                m_net_by_name[expanded_port_identifier] = global_port_net;

                // assign global port nets to ports of top module
                top_assignments[expanded_port_identifier] = expanded_port_identifier;

                if (port->m_direction == PinDirection::input || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_input_net())
                    {
                        return ERR("could not construct netlist: failed to mark global I/O net '" + core_strings::to<std::string>(expanded_port_identifier) + "' as global input");
                    }
                }

                if (port->m_direction == PinDirection::output || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_output_net())
                    {
                        return ERR("could not construct netlist: failed to mark global I/O net '" + core_strings::to<std::string>(expanded_port_identifier) + "' as global output");
                    }
                }
            }
        }

        if (const auto res = instantiate_entity("top_module", top_entity, nullptr, top_assignments); res.is_error())
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
                            return ERR("could not construct netlist: unable to remove source from net '" + slave_net->get_name() + "' with ID " + std::to_string(slave_net->get_id()));
                        }

                        if (!master_net->is_a_source(src_gate, src_pin))
                        {
                            if (!master_net->add_source(src_gate, src_pin))
                            {
                                return ERR("could not construct netlist: unable to add source to net '" + master_net->get_name() + "' with ID " + std::to_string(master_net->get_id()));
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
                            return ERR("could not construct netlist: unable to remove destination from net '" + slave_net->get_name() + "' with ID " + std::to_string(slave_net->get_id()));
                        }

                        if (!master_net->is_a_destination(dst_gate, dst_pin))
                        {
                            if (!master_net->add_destination(dst_gate, dst_pin))
                            {
                                return ERR("could not construct netlist: unable to add destination to net '" + master_net->get_name() + "' with ID " + std::to_string(master_net->get_id()));
                            }
                        }
                    }

                    // merge generics and attributes
                    for (const auto& it : slave_net->get_data_map())
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
                    if (const auto it = m_module_port_by_net.find(slave_net); it != m_module_port_by_net.end())
                    {
                        for (auto [module, index] : it->second)
                        {
                            std::get<1>(m_module_ports.at(module).at(index)) = master_net;
                        }
                        m_module_port_by_net[master_net].insert(m_module_port_by_net[master_net].end(), it->second.begin(), it->second.end());
                        m_module_port_by_net.erase(it);
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

            if ((module->get_pin_by_net(m_one_net) == nullptr) && (input_nets.find(m_one_net) != input_nets.end() || output_nets.find(m_one_net) != input_nets.end()))
            {
                if (auto res = module->create_pin("'1'", m_one_net); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not construct netlist: failed to create pin '1' at net '" + m_one_net->get_name() + "' with ID " + std::to_string(m_one_net->get_id()) + "within module '"
                                          + module->get_name() + "' with ID " + std::to_string(module->get_id()));
                }
            }

            if ((module->get_pin_by_net(m_zero_net) == nullptr) && (input_nets.find(m_zero_net) != input_nets.end() || output_nets.find(m_zero_net) != input_nets.end()))
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

    Result<Module*>
        VHDLParser::instantiate_entity(const ci_string& instance_identifier, VhdlEntity* vhdl_entity, Module* parent, const std::unordered_map<ci_string, ci_string>& parent_module_assignments)
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
            module->set_name(core_strings::to<std::string>(instance_alias.at(instance_identifier)));
        }
        else
        {
            module = m_netlist->create_module(core_strings::to<std::string>(instance_alias.at(instance_identifier)), parent);
        }

        ci_string instance_type = vhdl_entity->m_name;
        if (module == nullptr)
        {
            return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                       + "': failed to create module");
        }
        module->set_type(core_strings::to<std::string>(instance_type));

        // assign entity-level attributes
        for (const VhdlDataEntry& attribute : vhdl_entity->m_attributes)
        {
            if (!module->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
            {
                log_warning("vhdl_parser",
                            "could not set attribute '{} = {}' of type '{}' for instance '{}' type '{}'.",
                            attribute.m_name,
                            attribute.m_value,
                            attribute.m_type,
                            instance_identifier,
                            instance_type);
            }
        }

        // assign module port names and attributes
        for (const auto& port : vhdl_entity->m_ports)
        {
            for (const auto& expanded_port_identifier : port->m_expanded_identifiers)
            {
                if (const auto it = parent_module_assignments.find(expanded_port_identifier); it != parent_module_assignments.end())
                {
                    Net* port_net = m_net_by_name.at(it->second);
                    m_module_ports[module].push_back(std::make_pair(core_strings::to<std::string>(expanded_port_identifier), port_net));
                    m_module_port_by_net[port_net].push_back(std::make_pair(module, m_module_ports[module].size() - 1));

                    // assign port attributes
                    for (const VhdlDataEntry& attribute : port->m_attributes)
                    {
                        if (!port_net->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
                        {
                            log_warning("vhdl_parser",
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
        for (const auto& signal : vhdl_entity->m_signals)
        {
            for (const auto& expanded_name : signal->m_expanded_names)
            {
                signal_alias[expanded_name] = get_unique_alias(m_signal_name_occurrences, expanded_name);

                // create new net for the signal
                Net* signal_net = m_netlist->create_net(core_strings::to<std::string>(signal_alias.at(expanded_name)));
                if (signal_net == nullptr)
                {
                    return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                               + "': failed to create net '" + core_strings::to<std::string>(expanded_name) + "'");
                }

                m_net_by_name[signal_alias.at(expanded_name)] = signal_net;

                // assign signal attributes
                for (const VhdlDataEntry& attribute : signal->m_attributes)
                {
                    if (!signal_net->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
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
        for (const auto& [left_expanded_signal, right_expanded_signal] : vhdl_entity->m_expanded_assignments)
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
                return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                           + "': failed to find alias for net '" + core_strings::to<std::string>(a) + "'");
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
                return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                           + "': failed to find alias for net '" + core_strings::to<std::string>(b) + "'");
            }

            m_nets_to_merge[b].push_back(a);
        }

        // process instances i.e. gates or other entities
        for (const auto& instance : vhdl_entity->m_instances)
        {
            // will later hold either module or gate, so attributes can be assigned properly
            DataContainer* container = nullptr;

            // assign actual signal names to ports
            std::unordered_map<ci_string, ci_string> instance_assignments;

            // if the instance is another entity, recursively instantiate it
            if (auto entity_it = m_entities_by_name.find(instance->m_type); entity_it != m_entities_by_name.end())
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
                        else if (assignment != "'Z'" && assignment != "'X'")
                        {
                            continue;
                        }
                        else if (vhdl_entity->m_expanded_port_identifiers.find(assignment) != vhdl_entity->m_expanded_port_identifiers.end())
                        {
                            continue;
                        }
                        else
                        {
                            return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                                       + "': port assignment '" + core_strings::to<std::string>(port) + " = " + core_strings::to<std::string>(assignment) + "' is invalid");
                        }
                    }
                }

                if (auto res = instantiate_entity(instance->m_name, entity_it->second, module, instance_assignments); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                                          + "': unable to create instance '" + core_strings::to<std::string>(instance->m_name) + "' of type '"
                                          + core_strings::to<std::string>(entity_it->second->m_name) + "'");
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

                Gate* new_gate = m_netlist->create_gate(gate_type_it->second, core_strings::to<std::string>(instance_alias.at(instance->m_name)));
                if (new_gate == nullptr)
                {
                    return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                               + "': failed to create gate '" + core_strings::to<std::string>(instance->m_name) + "'");
                }

                if (!module->is_top_module())
                {
                    module->assign_gate(new_gate);
                }

                container = new_gate;

                // if gate is of a GND or VCC gate type, mark it as such
                if (m_vcc_gate_types.find(instance->m_type) != m_vcc_gate_types.end() && !new_gate->mark_vcc_gate())
                {
                    return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type) + "': failed to mark '"
                               + core_strings::to<std::string>(instance->m_name) + "' of type '" + core_strings::to<std::string>(instance->m_type) + "' as GND gate");
                }
                if (m_gnd_gate_types.find(instance->m_type) != m_gnd_gate_types.end() && !new_gate->mark_gnd_gate())
                {
                    return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type) + "': failed to mark '"
                               + core_strings::to<std::string>(instance->m_name) + "' of type '" + core_strings::to<std::string>(instance->m_type) + "' as VCC gate");
                }

                // cache pin names
                std::unordered_map<ci_string, GatePin*> pin_names_map;
                for (auto* pin : gate_type_it->second->get_pins())
                {
                    pin_names_map[core_strings::to<ci_string>(pin->get_name())] = pin;
                }

                // expand pin assignments
                for (const auto& [pin, assignment] : instance->m_expanded_port_assignments)
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
                    else if (vhdl_entity->m_expanded_port_identifiers.find(assignment) != vhdl_entity->m_expanded_port_identifiers.end())
                    {
                        continue;
                    }
                    else
                    {
                        return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                                   + "': failed to assign '" + core_strings::to<std::string>(assignment) + "' to pin '" + core_strings::to<std::string>(pin) + "' of gate '"
                                   + core_strings::to<std::string>(instance->m_name) + "' of type '" + core_strings::to<std::string>(instance->m_type) + "' as the assignment is invalid");
                    }

                    // get the respective net for the assignment
                    if (const auto net_it = m_net_by_name.find(signal); net_it == m_net_by_name.end())
                    {
                        return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                                   + "': failed to assign signal'" + core_strings::to<std::string>(signal) + "' to pin '" + core_strings::to<std::string>(pin)
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
                            return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                                       + "': failed to assign net '" + core_strings::to<std::string>(signal) + "' to pin '" + core_strings::to<std::string>(pin) + "' as it is not a pin of gate '"
                                       + new_gate->get_name() + "' of type '" + new_gate->get_type()->get_name() + "'");
                        }

                        if (is_output && !current_net->add_source(new_gate, core_strings::to<std::string>(pin)))
                        {
                            return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                                       + "': failed to add net '" + core_strings::to<std::string>(signal) + "' as a source to gate '" + new_gate->get_name() + "' via pin '"
                                       + core_strings::to<std::string>(pin) + "'");
                        }

                        if (is_input && !current_net->add_destination(new_gate, core_strings::to<std::string>(pin)))
                        {
                            return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                                       + "': failed to add net '" + core_strings::to<std::string>(signal) + "' as a destination to gate '" + new_gate->get_name() + "' via pin '"
                                       + core_strings::to<std::string>(pin) + "'");
                        }
                    }
                }
            }
            else
            {
                return ERR("could not create instance '" + core_strings::to<std::string>(instance_identifier) + "' of type '" + core_strings::to<std::string>(instance_type)
                           + "': failed to find gate type '" + core_strings::to<std::string>(instance->m_type) + "' in gate library '" + m_netlist->get_gate_library()->get_name() + "'");
            }

            // assign instance attributes
            for (const auto& attribute : instance->m_attributes)
            {
                if (!container->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
                {
                    log_warning("vhdl_parser",
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
            for (const auto& generic : instance->m_generics)
            {
                if (!container->set_data("generic", generic.m_name, generic.m_type, generic.m_value))
                {
                    log_warning("vhdl_parser",
                                "could not set generic '{} = {}' of type '{}' for instance '{}' of type '{}' within instance '{}' of type '{}'.",
                                generic.m_name,
                                generic.m_value,
                                generic.m_type,
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

    VHDLParser::ci_string VHDLParser::get_unique_alias(std::unordered_map<ci_string, u32>& name_occurrences, const ci_string& name) const
    {
        // if the name only appears once, we don't have to suffix it
        if (name_occurrences[name] < 2)
        {
            return name;
        }

        name_occurrences[name]++;

        // otherwise, add a unique string to the name
        return name + "__[" + core_strings::to<ci_string>(std::to_string(name_occurrences[name])) + "]__";
    }
}    // namespace hal
