#include "netlist/hdl_parser/hdl_parser_verilog.h"

#include "core/log.h"
#include "core/utils.h"

#include <iomanip>

hdl_parser_verilog::hdl_parser_verilog(std::stringstream& stream) : hdl_parser(stream)
{
}

// ###########################################################################
// ###########          Parse HDL into intermediate format          ##########
// ###########################################################################

bool hdl_parser_verilog::parse()
{
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
    catch (token_stream<std::string>::token_stream_exception& e)
    {
        if (e.line_number != (u32)-1)
        {
            log_error("hdl_parser", "{} near line {}", e.message, e.line_number);
        }
        else
        {
            log_error("hdl_parser", "{}", e.message);
        }
        return false;
    }

    return true;
}

bool hdl_parser_verilog::tokenize()
{
    const std::string delimiters = ",()[]{}\\#*: ;=.";
    std::string current_token;
    u32 line_number = 0;

    std::string line;
    bool escaped            = false;
    bool multi_line_comment = false;

    std::vector<token<std::string>> parsed_tokens;
    while (std::getline(m_fs, line))
    {
        line_number++;
        this->remove_comments(line, multi_line_comment);

        for (char c : line)
        {
            if (c == '\\')
            {
                escaped = true;
                continue;
            }
            else if (escaped && std::isspace(c))
            {
                escaped = false;
                continue;
            }

            if ((!std::isspace(c) && delimiters.find(c) == std::string::npos) || escaped)
            {
                current_token += c;
            }
            else
            {
                if (!current_token.empty())
                {
                    parsed_tokens.emplace_back(line_number, current_token);
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

    m_token_stream = token_stream(parsed_tokens, {"(", "["}, {")", "]"});
    return true;
}

bool hdl_parser_verilog::parse_tokens()
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
        else
        {
            if (!parse_entity(attributes))
            {
                return false;
            }
        }
    }

    return true;
}

bool hdl_parser_verilog::parse_entity(std::map<std::string, std::string>& attributes)
{
    std::set<std::string> port_names;
    std::map<std::string, std::string> internal_attributes;

    m_token_stream.consume("module", true);
    const auto line_number        = m_token_stream.peek().number;
    const std::string entity_name = m_token_stream.consume();

    // verify entity name
    if (m_entities.find(entity_name) != m_entities.end())
    {
        log_error("hdl_parser", "an entity with the name '{}' does already exist (see line {} and line {})", entity_name, line_number, m_entities.at(entity_name).get_line_number());
        return false;
    }

    entity e(line_number, entity_name);

    // parse port list
    if (m_token_stream.consume("#("))
    {
        m_token_stream.consume_until(")");
        m_token_stream.consume(")", true);
    }

    parse_port_list(port_names);

    m_token_stream.consume(";", true);

    auto next_token = m_token_stream.peek();
    while (next_token != "endmodule")
    {
        if (next_token == "input" || next_token == "output" || next_token == "inout")
        {
            if (!parse_port_definition(e, port_names, internal_attributes))
            {
                return false;
            }
        }
        else if (next_token == "wire")
        {
            if (!parse_signal_definition(e, internal_attributes))
            {
                return false;
            }
        }
        else if (next_token == "assign")
        {
            if (!parse_assign(e))
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
            if (!parse_instance(e, internal_attributes))
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
            e.add_attribute(attribute_name, "unknown", attribute_value);
        }

        attributes.clear();
    }

    // initialize entity (expand ports, signals, and assignments)
    e.initialize(this);

    // add to collection of entities
    m_entities.emplace(entity_name, e);
    m_last_entity = entity_name;

    return true;
}

void hdl_parser_verilog::parse_port_list(std::set<std::string>& port_names)
{
    m_token_stream.consume("(", true);
    auto ports_str = m_token_stream.extract_until(")");
    m_token_stream.consume(")", true);

    while (ports_str.remaining() > 0)
    {
        port_names.insert(ports_str.consume().string);
        ports_str.consume(",", ports_str.remaining() > 0);
    }
}

bool hdl_parser_verilog::parse_port_definition(entity& e, const std::set<std::string>& port_names, std::map<std::string, std::string>& attributes)
{
    const auto line_number   = m_token_stream.peek().number;
    const auto direction_str = m_token_stream.consume().string;
    auto ports               = parse_signal_list();

    port_direction direction;
    if (direction_str == "input")
    {
        direction = port_direction::IN;
    }
    else if (direction_str == "output")
    {
        direction = port_direction::OUT;
    }
    else if (direction_str == "inout")
    {
        direction = port_direction::INOUT;
    }
    else
    {
        log_error("hdl_parser", "invalid direction '{}' for port declaration in line {}", direction_str, line_number);
        return false;
    }

    if (ports.empty())
    {
        // error already printed in subfunction
        return false;
    }

    for (auto& p : ports)
    {
        // check port name against port declarations
        if (port_names.find(p.first) == port_names.end())
        {
            log_error("hdl_parser", "port name '{}' in line {} has not been declared in entity port list", p.first, line_number);
            return false;
        }

        // assign port attributes
        for (const auto& [attribute_name, attribute_value] : attributes)
        {
            p.second.add_attribute(attribute_name, "unknown", attribute_value);
        }

        e.add_port(direction, p.second);
    }

    attributes.clear();

    return true;
}

bool hdl_parser_verilog::parse_signal_definition(entity& e, std::map<std::string, std::string>& attributes)
{
    m_token_stream.consume("wire", true);
    auto signals = parse_signal_list();

    if (signals.empty())
    {
        // error already printed in subfunction
        return false;
    }

    // assign attributes to signals
    if (!attributes.empty())
    {
        for (auto& s : signals)
        {
            for (const auto& [attribute_name, attribute_value] : attributes)
            {
                s.second.add_attribute(attribute_name, "unknown", attribute_value);
            }
        }

        attributes.clear();
    }

    // assign signals to entity
    e.add_signals(signals);

    return true;
}

bool hdl_parser_verilog::parse_assign(entity& e)
{
    const auto line_number = m_token_stream.peek().number;
    m_token_stream.consume("assign", true);
    auto left_str = m_token_stream.extract_until("=");
    m_token_stream.consume("=", true);
    auto right_str = m_token_stream.extract_until(";");
    m_token_stream.consume(";", true);

    // extract assignments for each bit
    const auto left_parts  = get_assignment_signals(e, left_str, false);
    const auto right_parts = get_assignment_signals(e, right_str, true);

    // verify correctness
    if (!left_parts.has_value() || !right_parts.has_value())
    {
        // error already printed in subfunction
        return false;
    }

    if (left_parts->second != right_parts->second)
    {
        log_error("hdl_parser", "assignment width mismatch: left side has size {} and right side has size {} in line {}", left_parts->second, right_parts->second, line_number);
        return false;
    }

    e.add_assignment(left_parts->first, right_parts->first);

    return true;
}

bool hdl_parser_verilog::parse_attribute(std::map<std::string, std::string>& attributes)
{
    m_token_stream.consume("(*", true);
    auto attribute_str = m_token_stream.extract_until("*)");
    m_token_stream.consume("*)", true);

    // extract attributes
    do
    {
        const std::string attribute_name = attribute_str.consume().string;
        std::string attribute_value;

        // attribute value specified?
        if (attribute_str.consume("="))
        {
            attribute_value = attribute_str.consume();

            // remove "
            if (attribute_value[0] == '\"' && attribute_value.back() == '\"')
            {
                attribute_value = attribute_value.substr(1, attribute_value.size() - 2);
            }
        }

        attributes.emplace(attribute_name, attribute_value);

    } while (attribute_str.consume(",", false));

    return true;
}

bool hdl_parser_verilog::parse_instance(entity& e, std::map<std::string, std::string>& attributes)
{
    const auto line_number   = m_token_stream.peek().number;
    const auto instance_type = m_token_stream.consume().string;
    instance inst(line_number, instance_type);

    // parse generics map
    if (m_token_stream.consume("#("))
    {
        if (!parse_generic_assign(inst))
        {
            return false;
        }
    }

    // parse instance name
    const auto instance_name = m_token_stream.consume().string;
    inst.set_name(instance_name);

    // parse port map
    if (!parse_port_assign(e, inst))
    {
        return false;
    }

    // verify instance name
    const auto& instances = e.get_instances();
    if (instances.find(instance_name) != instances.end())
    {
        log_error("hdl_parser", "an instance with the name '{}' does already exist (see line {} and line {})", instance_name, line_number, instances.at(instance_name).get_line_number());
        return false;
    }

    // assign attributes to instance
    if (!attributes.empty())
    {
        for (const auto& [attribute_name, attribute_value] : attributes)
        {
            inst.add_attribute(attribute_name, "unknown", attribute_value);
        }

        attributes.clear();
    }

    // assign instance to entity
    e.add_instance(inst);

    return true;
}

bool hdl_parser_verilog::parse_port_assign(entity& e, instance& inst)
{
    m_token_stream.consume("(", true);
    auto port_str = m_token_stream.extract_until(")");
    m_token_stream.consume(")", true);

    while (port_str.remaining() > 0)
    {
        port_str.consume(".", true);
        auto left_str = port_str.consume();
        port_str.consume("(", true);
        auto right_str = port_str.extract_until(")");
        port_str.consume(")", true);
        port_str.consume(",", port_str.remaining() > 0);

        // check if port unconnected
        if (right_str.size() != 0)
        {
            const signal s(left_str.number, left_str.string, {}, false, false);
            const auto right_parts = get_assignment_signals(e, right_str, true);

            // verify correctness
            if (!right_parts.has_value())
            {
                // error already printed in subfunction
                return false;
            }

            inst.add_port_assignment(s, right_parts->first);
        }
    }

    m_token_stream.consume(";", true);

    return true;
}

bool hdl_parser_verilog::parse_generic_assign(instance& inst)
{
    auto generic_str = m_token_stream.extract_until(")");
    m_token_stream.consume(")", true);

    while (generic_str.remaining() > 0)
    {
        std::string value, data_type;

        const auto line_number = generic_str.peek().number;
        generic_str.consume(".", true);
        const auto lhs = generic_str.join_until("(", "");
        generic_str.consume("(", true);
        const auto rhs = generic_str.join_until(")", "");
        generic_str.consume(")", true);
        generic_str.consume(",", generic_str.remaining() > 0);

        if (core_utils::is_integer(rhs))
        {
            value     = rhs;
            data_type = "integer";
        }
        else if (core_utils::is_floating_point(rhs))
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
                return false;
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
            log_error("hdl_parser", "cannot identify data type of generic map value '{}' in instance '{}' in line {}", rhs.string, inst.get_name(), line_number);
            return false;
        }

        inst.add_generic_assignment(lhs, data_type, value);
    }

    return true;
}

// ###########################################################################
// ###################          Helper functions          ####################
// ###########################################################################

void hdl_parser_verilog::remove_comments(std::string& line, bool& multi_line_comment)
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

        const auto single_line_comment_begin = line.find("//");
        const auto multi_line_comment_begin  = line.find("/*");
        const auto multi_line_comment_end    = line.find("*/");

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

std::vector<u32> hdl_parser_verilog::parse_range(token_stream<std::string>& range_str)
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

std::map<std::string, hdl_parser_verilog::signal> hdl_parser_verilog::parse_signal_list()
{
    std::map<std::string, signal> signals;
    std::vector<std::vector<u32>> ranges;

    auto signal_str = m_token_stream.extract_until(";");
    m_token_stream.consume(";", true);

    // extract bounds
    while (signal_str.consume("["))
    {
        const auto range = parse_range(signal_str);
        signal_str.consume("]", true);

        ranges.emplace_back(range);
    }

    // extract names
    do
    {
        const auto signal_name = signal_str.consume();

        signal s(signal_name.number, signal_name.string, ranges);
        signals.emplace(signal_name, s);
    } while (signal_str.consume(",", false));

    return signals;
}

std::optional<std::pair<std::vector<hdl_parser_verilog::signal>, i32>> hdl_parser_verilog::get_assignment_signals(entity& e, token_stream<std::string>& signal_str, bool allow_numerics)
{
    // PARSE ASSIGNMENT
    //   assignment can currently be one of the following:
    //   (1) NAME *single-dimensional*
    //   (2) NAME *multi-dimensional*
    //   (3) NUMBER
    //   (4) NAME[INDEX1][INDEX2]...
    //   (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
    //   (6) {(1 - 5), (1 - 5), ...}

    std::vector<signal> result;
    std::vector<token_stream<std::string>> parts;
    i32 size = 0;

    // (6) {(1) - (5), (1) - (5), ...}
    if (signal_str.peek() == "{")
    {
        signal_str.consume("{", true);

        auto assignment_list_str = signal_str.extract_until("}");
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

    for (auto& part_stream : parts)
    {
        const auto signal_name_token = part_stream.consume();
        const auto line_number       = signal_name_token.number;
        auto signal_name             = signal_name_token.string;
        std::vector<std::vector<u32>> ranges;
        bool is_binary = false;

        // (3) NUMBER
        if (isdigit(signal_name[0]) || signal_name[0] == '\'')
        {
            if (!allow_numerics)
            {
                log_error("hdl_parser", "numeric value {} not allowed at this position in line {}", signal_name, line_number);
                return std::nullopt;
            }

            signal_name = get_bin_from_literal(signal_name_token);
            if (signal_name.empty())
            {
                return std::nullopt;
            }

            ranges    = {};
            is_binary = true;
        }
        else
        {
            std::vector<std::vector<u32>> reference_ranges;

            const auto& signals = e.get_signals();
            const auto& ports   = e.get_ports();
            if (const auto signal_it = signals.find(signal_name); signal_it != signals.end())
            {
                reference_ranges = signal_it->second.get_ranges();
            }
            else if (const auto port_it = ports.find(signal_name); port_it != ports.end())
            {
                reference_ranges = port_it->second.second.get_ranges();
            }
            else
            {
                log_error("hdl_parser", "signal name '{}' is invalid in assignment in line {}", signal_name, line_number);
                return std::nullopt;
            }

            // any bounds specified?
            if (part_stream.consume("["))
            {
                // (4) NAME[INDEX1][INDEX2]...
                // (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
                do
                {
                    auto range_str = part_stream.extract_until("]");
                    ranges.emplace_back(parse_range(range_str));
                    part_stream.consume("]", true);
                } while (part_stream.consume("[", false));
            }
            else
            {
                // (1) NAME *single-dimensional*
                // (2) NAME *multi-dimensional*
                ranges = reference_ranges;
            }
        }

        // create new signal for assign
        signal s(line_number, signal_name, ranges, is_binary);
        size += s.get_size();
        result.push_back(s);
    }

    return std::make_pair(result, size);
}

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

std::string hdl_parser_verilog::get_bin_from_literal(const token<std::string>& value_token)
{
    const auto line_number = value_token.number;
    const auto value       = core_utils::to_lower(core_utils::replace(value_token.string, "_", ""));

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
            for (const auto& c : number)
            {
                if (c >= '0' && c <= '1')
                {
                    res += c;
                }
                else
                {
                    log_error("hdl_parser", "invalid character within binary number literal {} in line {}", value, line_number);
                    return "";
                }
            }
            break;
        }

        case 'o':
            for (const auto& c : number)
            {
                if (c >= '0' && c <= '7')
                {
                    res += oct_to_bin.at(c);
                }
                else
                {
                    log_error("hdl_parser", "invalid character within octal number literal {} in line {}", value, line_number);
                    return "";
                }
            }
            break;

        case 'd': {
            u64 tmp_val = 0;

            for (const auto& c : number)
            {
                if (c >= '0' && c <= '9')
                {
                    tmp_val = (tmp_val * 10) + (c - '0');
                }
                else
                {
                    log_error("hdl_parser", "invalid character within octal number literal {} in line {}", value, line_number);
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
            for (const auto& c : number)
            {
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                {
                    res += hex_to_bin.at(c);
                }
                else
                {
                    log_error("hdl_parser", "invalid character within hexadecimal number literal {} in line {}", value, line_number);
                    return "";
                }
            }
            break;
        }

        default: {
            log_error("hdl_parser", "invalid base '{}' within number literal {} in line {}", prefix, value, line_number);
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

std::string hdl_parser_verilog::get_hex_from_literal(const token<std::string>& value_token)
{
    const auto line_number = value_token.number;
    const auto value       = core_utils::to_lower(core_utils::replace(value_token.string, "_", ""));

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
                log_error("hdl_parser", "invalid character within binary number literal {} in line {}", value, line_number);
                return "";
            }

            base = 2;
            break;
        }

        case 'o': {
            if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '7'); }))
            {
                log_error("hdl_parser", "invalid character within octal number literal {} in line {}", value, line_number);
                return "";
            }

            base = 8;
            break;
        }

        case 'd': {
            if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9'); }))
            {
                log_error("hdl_parser", "invalid character within decimal number literal {} in line {}", value, line_number);
                return "";
            }

            base = 10;
            break;
        }

        case 'h': {
            if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }))
            {
                log_error("hdl_parser", "invalid character within hexadecimal number literal {} in line {}", value, line_number);
                return "";
            }

            base = 16;
            break;
        }

        default: {
            log_error("hdl_parser", "invalid base '{}' within number literal {} in line {}", prefix, value, line_number);
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

bool hdl_parser_verilog::is_in_bounds(const std::vector<std::pair<i32, i32>>& bounds, const std::vector<std::pair<i32, i32>>& reference_bounds) const
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