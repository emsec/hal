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

        std::vector<BooleanFunction::Value> get_binary_vector(std::string value)
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
                            log_error("verilog_parser", "invalid character within binary number literal {}.", value);
                            return {};
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
                            log_error("verilog_parser", "invalid character within octal number literal {}.", value);
                            return {};
                        }
                    }
                    break;

                case 'D': {
                    u64 tmp_val = 0;

                    for (const char c : number)
                    {
                        if ((c >= '0' && c <= '9') || c == 'X' || c == 'Z')
                        {
                            tmp_val = (tmp_val * 10) + (c - '0');
                        }
                        else
                        {
                            log_error("verilog_parser", "invalid character within decimal number literal {} in line {}.", value);
                            return {};
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
                            log_error("verilog_parser", "invalid character within hexadecimal number literal {}.", value);
                            return {};
                        }
                    }
                    break;
                }

                default: {
                    log_error("verilog_parser", "invalid base '{}' within number literal {}.", prefix, value);
                    return {};
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

            return result;
        }

        std::vector<assignment_t> parse_assignment_expression(TokenStream<std::string>&& stream)
        {
            std::vector<TokenStream<std::string>> parts;

            if (stream.size() == 0)
            {
                return {empty_t()};
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
                    numeral_t binary_vector = get_binary_vector(signal_name_token);
                    if (binary_vector.empty())
                    {
                        return {};
                    }
                    result.push_back(std::move(binary_vector));
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

            return result;
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

    bool VerilogParser::parse(const std::filesystem::path& file_path)
    {
        m_modules.clear();
        m_modules_by_name.clear();

        {
            std::ifstream ifs;
            ifs.open(file_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                log_error("verilog_parser", "unable to open '{}'.", file_path.string());
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
        for (auto& [module_name, verilog_module] : m_modules_by_name)
        {
            // expand port identifiers
            for (const auto& port : verilog_module->m_ports)
            {
                if (port->m_expression != port->m_identifier)
                {
                    if (!port->m_ranges.empty())
                    {
                        port->m_expanded_identifiers = expand_ranges(port->m_identifier, port->m_ranges);
                    }
                    else
                    {
                        port->m_expanded_identifiers = {port->m_identifier};
                    }
                }
                else
                {
                    if (!port->m_ranges.empty())
                    {
                        port->m_expanded_identifiers = expand_ranges(port->m_identifier, port->m_ranges);
                    }
                    else
                    {
                        port->m_expanded_identifiers = {port->m_identifier};
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
                    return false;
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
            for (const auto& instance : verilog_module->m_instances)
            {
                if (auto module_it = m_modules_by_name.find(instance->m_type); module_it != m_modules_by_name.end())
                {
                    for (const auto& port_assignment : instance->m_port_assignments)
                    {
                        if (!port_assignment.m_port_name.has_value())
                        {
                            log_error("verilog_parser", "parsing of unnamed ports is not yet supported.");
                            return false;
                        }

                        VerilogPort* port;
                        if (const auto port_it = module_it->second->m_ports_by_identifier.find(port_assignment.m_port_name.value()); port_it == module_it->second->m_ports_by_identifier.end())
                        {
                            return false;
                        }
                        else
                        {
                            port = port_it->second;
                        }

                        const std::vector<std::string>& left_port = port->m_expanded_identifiers;
                        const std::vector<std::string> right_port = expand_assignment_expression(verilog_module, port_assignment.m_assignment);
                        if (left_port.empty())
                        {
                            return false;
                        }

                        if (!right_port.empty())
                        {
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
                                instance->m_expanded_port_assignments.push_back(std::make_pair(left_port.at(i), right_port.at(i)));
                            }
                        }
                        else
                        {
                            for (const auto& expanded_port_name : left_port)
                            {
                                instance->m_expanded_port_assignments.push_back(std::make_pair(expanded_port_name, ""));
                            }
                        }
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

        m_gate_types.clear();
        m_gnd_gate_types.clear();
        m_vcc_gate_types.clear();
        m_instance_name_occurrences.clear();
        m_signal_name_occurrences.clear();
        m_net_by_name.clear();
        m_nets_to_merge.clear();
        m_module_ports.clear();
        for (const auto& module : m_modules)
        {
            for (const auto& instance : module->m_instances)
            {
                instance->m_expanded_port_assignments.clear();
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
        VerilogModule* top_module = m_modules_by_name.at(m_last_module);
        if (!construct_netlist(top_module))
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

        m_netlist->load_gate_locations_from_data();

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
        std::vector<VerilogDataEntry> attributes;

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

    bool VerilogParser::parse_module(std::vector<VerilogDataEntry>& attributes)
    {
        std::set<std::string> port_names;
        std::vector<VerilogDataEntry> internal_attributes;

        m_token_stream.consume("module", true);
        const u32 line_number         = m_token_stream.peek().number;
        const std::string module_name = m_token_stream.consume();

        // verify entity name
        if (const auto it = m_modules_by_name.find(module_name); it != m_modules_by_name.end())
        {
            log_error("verilog_parser", "a module with the name '{}' does already exist (see line {} and line {})", module_name, line_number, it->second->m_line_number);
            return false;
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
            log_warning("verilog_parser", "cannot parse parameter list provided for module '{}'.", module_name);
        }

        // parse port (declaration) list
        m_token_stream.consume("(", true);
        Token<std::string> next_token = m_token_stream.peek();
        if (next_token == "input" || next_token == "output" || next_token == "inout")
        {
            if (!parse_port_declaration_list(verilog_module_raw))
            {
                return false;
            }
        }
        else
        {
            if (!parse_port_list(verilog_module_raw))
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
                if (!parse_port_definition(verilog_module_raw, internal_attributes))
                {
                    return false;
                }
            }
            else if (next_token == "wire" || next_token == "tri")
            {
                if (!parse_signal_definition(verilog_module_raw, internal_attributes))
                {
                    return false;
                }
            }
            else if (next_token == "parameter")
            {
                // TODO add support for parameter parsing
                m_token_stream.consume_until(";");
                m_token_stream.consume(";", true);
                log_warning("verilog_parser", "cannot parse parameter provided for module '{}'.", module_name);
            }
            else if (next_token == "assign")
            {
                if (!parse_assignment(verilog_module_raw))
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
                if (!parse_instance(verilog_module_raw, internal_attributes))
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
            verilog_module->m_attributes.insert(verilog_module->m_attributes.end(), attributes.begin(), attributes.end());
            attributes.clear();
        }

        // add to collection of entities
        m_modules.push_back(std::move(verilog_module));
        m_modules_by_name[module_name] = verilog_module_raw;
        m_last_module                  = module_name;

        return true;
    }

    bool VerilogParser::parse_port_list(VerilogModule* verilog_module)
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

        return true;
    }

    bool VerilogParser::parse_port_declaration_list(VerilogModule* verilog_module)
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

        return true;
    }

    bool VerilogParser::parse_port_definition(VerilogModule* verilog_module, std::vector<VerilogDataEntry>& attributes)
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

            VerilogPort* port;
            if (const auto it = verilog_module->m_ports_by_expression.find(port_expression); it == verilog_module->m_ports_by_expression.end())
            {
                log_error("verilog_parser", "a port with name '{}' does not exist for module '{}' in line {}.", port_expression, verilog_module->m_name, port_expression_token.number);
                return false;
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

        return true;
    }

    bool VerilogParser::parse_signal_definition(VerilogModule* verilog_module, std::vector<VerilogDataEntry>& attributes)
    {
        // consume "wire" or "tri"
        m_token_stream.consume();

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
                assignment.m_assignment = parse_assignment_expression(signal_stream.extract_until(","));
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

        return true;
    }

    bool VerilogParser::parse_assignment(VerilogModule* verilog_module)
    {
        m_token_stream.consume("assign", true);
        VerilogAssignment assignment;

        assignment.m_variable = parse_assignment_expression(m_token_stream.extract_until("="));
        if (assignment.m_variable.empty())
        {
            return false;
        }
        m_token_stream.consume("=", true);
        assignment.m_assignment = parse_assignment_expression(m_token_stream.extract_until(";"));
        if (assignment.m_assignment.empty())
        {
            return false;
        }
        m_token_stream.consume(";", true);

        verilog_module->m_assignments.push_back(std::move(assignment));
        return true;
    }

    bool VerilogParser::parse_attribute(std::vector<VerilogDataEntry>& attributes)
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

        return true;
    }

    bool VerilogParser::parse_instance(VerilogModule* verilog_module, std::vector<VerilogDataEntry>& attributes)
    {
        auto instance    = std::make_unique<VerilogInstance>();
        instance->m_type = m_token_stream.consume().string;

        // parse generics map
        if (m_token_stream.consume("#("))
        {
            instance->m_parameters = parse_generic_assign();
        }

        // parse instance name
        instance->m_name = m_token_stream.consume().string;

        // parse port map
        if (!parse_port_assign(instance.get()))
        {
            return false;
        }

        // assign attributes to instance
        instance->m_attributes = attributes;
        attributes.clear();

        verilog_module->m_instances_by_name[instance->m_name] = instance.get();
        verilog_module->m_instances.push_back(std::move(instance));

        return true;
    }

    bool VerilogParser::parse_port_assign(VerilogInstance* instance)
    {
        m_token_stream.consume("(", true);

        do
        {
            if (m_token_stream.consume(".", false))
            {
                VerilogPortAssignment port_assignment;
                port_assignment.m_port_name = m_token_stream.consume().string;
                m_token_stream.consume("(", true);
                port_assignment.m_assignment = parse_assignment_expression(m_token_stream.extract_until(")"));
                m_token_stream.consume(")", true);
                instance->m_port_assignments.push_back(std::move(port_assignment));
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(")", true);
        m_token_stream.consume(";", true);

        return true;
    }

    std::vector<VerilogDataEntry> VerilogParser::parse_generic_assign()
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

                    if (value == "0" || value == "1")
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
                    generics.push_back(VerilogDataEntry({lhs.string, data_type, value}));
                }
            }
        } while (m_token_stream.consume(",", false));

        m_token_stream.consume(")", true);

        return generics;
    }

    // ###########################################################################
    // ###########      Assemble Netlist from Intermediate Format       ##########
    // ###########################################################################

    bool VerilogParser::construct_netlist(VerilogModule* top_module)
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
            for (const std::string& expanded_port_identifier : port->m_expanded_identifiers)
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
                    // cache pin groups
                    std::unordered_map<std::string, std::vector<std::string>> pin_groups;
                    for (const auto& [group_name, pins] : gate_type_it->second->get_pin_groups())
                    {
                        for (const auto& pin : pins)
                        {
                            pin_groups[group_name].push_back(pin.second);
                        }
                    }

                    for (const auto port_assignment : instance->m_port_assignments)
                    {
                        std::vector<std::string> left_port;
                        if (!port_assignment.m_port_name.has_value())
                        {
                            log_error("verilog_parser", "parsing of unnamed ports is not yet supported.");
                            return false;
                        }

                        const auto& port_name = port_assignment.m_port_name.value();

                        if (const auto group_it = pin_groups.find(port_name); group_it != pin_groups.end())
                        {
                            left_port = group_it->second;
                        }
                        else
                        {
                            left_port.push_back(port_name);
                        }
                        std::vector<std::string> right_port = expand_assignment_expression(verilog_module, port_assignment.m_assignment);

                        if (!right_port.empty())
                        {
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
                                instance->m_expanded_port_assignments.push_back(std::make_pair(left_port.at(i), right_port.at(i)));
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
            for (const std::string& expanded_port_identifier : port->m_expanded_identifiers)
            {
                Net* global_port_net = m_netlist->create_net(expanded_port_identifier);
                if (global_port_net == nullptr)
                {
                    log_error("verilog_parser", "could not create global port net '{}'.", expanded_port_identifier);
                    return false;
                }

                m_net_by_name[expanded_port_identifier] = global_port_net;

                // assign global port nets to ports of top module
                top_assignments[expanded_port_identifier] = expanded_port_identifier;

                if (port->m_direction == PinDirection::input || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_input_net())
                    {
                        log_error("verilog_parser", "could not mark global port net '{}' as global input.", expanded_port_identifier);
                        return false;
                    }
                }

                if (port->m_direction == PinDirection::output || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_output_net())
                    {
                        log_error("verilog_parser", "could not mark global port net '{}' as global output.", expanded_port_identifier);
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
                    if (const auto it = m_module_ports.find(slave_net); it != m_module_ports.end())
                    {
                        m_module_ports[master_net] = it->second;
                        m_module_ports.erase(it);
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
                log_error("verilog_parser", "cyclic dependency between signals detected, cannot parse netlist.");
                return false;
            }
        }

        // update module nets, internal nets, input nets, and output nets
        for (Module* module : m_netlist->get_modules())
        {
            module->update_nets();
        }

        // assign module pins
        for (const auto& [net, port_info] : m_module_ports)
        {
            std::get<2>(port_info)->assign_pin(std::get<1>(port_info), net);
        }

        for (Module* module : m_netlist->get_modules())
        {
            std::unordered_set<Net*> input_nets  = module->get_input_nets();
            std::unordered_set<Net*> output_nets = module->get_input_nets();

            if (module->get_pin(m_one_net) == nullptr && (input_nets.find(m_one_net) != input_nets.end() || output_nets.find(m_one_net) != input_nets.end()))
            {
                module->assign_pin("'1'", m_one_net);
            }

            if (module->get_pin(m_zero_net) == nullptr && (input_nets.find(m_zero_net) != input_nets.end() || output_nets.find(m_zero_net) != input_nets.end()))
            {
                module->assign_pin("'0'", m_one_net);
            }
        }

        m_netlist->enable_automatic_net_checks(true);
        return true;
    }

    Module* VerilogParser::instantiate_module(const std::string& instance_identifier,
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
            log_error("verilog_parser", "could not instantiate instance '{}' of module '{}'.", instance_identifier, instance_type);
            return nullptr;
        }
        module->set_type(instance_type);

        // assign entity-level attributes
        for (const VerilogDataEntry& attribute : verilog_module->m_attributes)
        {
            if (!module->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
            {
                log_warning("verilog_parser", "could not set attribute ({} = {}) for instance '{}' type '{}'.", attribute.m_name, attribute.m_value, instance_identifier, instance_type);
            }
        }

        // assign module port names and attributes
        for (const auto& port : verilog_module->m_ports)
        {
            for (const std::string& expanded_port_identifier : port->m_expanded_identifiers)
            {
                if (const auto it = parent_module_assignments.find(expanded_port_identifier); it != parent_module_assignments.end())
                {
                    Net* port_net            = m_net_by_name.at(it->second);
                    m_module_ports[port_net] = std::make_tuple(port->m_direction, expanded_port_identifier, module);

                    // assign port attributes
                    for (const VerilogDataEntry& attribute : port->m_attributes)
                    {
                        if (!port_net->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
                        {
                            log_warning("verilog_parser",
                                        "could not set attribute ({} = {}) for port '{}' of instance '{}' of type '{}'.",
                                        attribute.m_name,
                                        attribute.m_value,
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
            for (const std::string& expanded_name : signal->m_expanded_names)
            {
                signal_alias[expanded_name] = get_unique_alias(m_signal_name_occurrences, expanded_name);

                // create new net for the signal
                Net* signal_net = m_netlist->create_net(signal_alias.at(expanded_name));
                if (signal_net == nullptr)
                {
                    log_error("verilog_parser", "could not instantiate net '{}' of instance '{}' of type '{}'.", expanded_name, instance_identifier, instance_type, instance_identifier, instance_type);
                    return nullptr;
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
                log_error("verilog_parser", "cannot find alias for net '{}' of instance '{}' of type '{}'.", a, instance_identifier, instance_type);
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
                log_error("verilog_parser", "cannot find alias for net '{}' of instance '{}' of type '{}'.", b, instance_identifier, instance_type);
                return nullptr;
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
                        else if (assignment != "'Z'" && assignment != "'X'" && assignment != "")
                        {
                            log_error("verilog_parser", "port assignment \"{} = {}\" is invalid for instance '{}' of type '{}'.", port, assignment, instance->m_name, instance->m_type);
                            return nullptr;
                        }
                    }
                }

                container = instantiate_module(instance->m_name, module_it->second, module, instance_assignments);
                if (container == nullptr)
                {
                    return nullptr;
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
                    log_error("verilog_parser", "could not instantiate gate '{}' within instance '{}' of type '{}'.", instance->m_name, instance_identifier, instance_type);
                    return nullptr;
                }

                if (!module->is_top_module())
                {
                    module->assign_gate(new_gate);
                }

                container = new_gate;

                // if gate is of a GND or VCC gate type, mark it as such
                if (m_vcc_gate_types.find(instance->m_type) != m_vcc_gate_types.end() && !new_gate->mark_vcc_gate())
                {
                    return nullptr;
                }
                if (m_gnd_gate_types.find(instance->m_type) != m_gnd_gate_types.end() && !new_gate->mark_gnd_gate())
                {
                    return nullptr;
                }

                // cache pin directions
                std::unordered_map<std::string, PinDirection> pin_to_direction = gate_type_it->second->get_pin_directions();

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
                    else
                    {
                        log_error("verilog_parser", "pin assignment \"{} = {}\" is invalid for instance '{}' of type '{}'", pin, assignment, instance->m_name, instance->m_type);
                        return nullptr;
                    }

                    // get the respective net for the assignment
                    if (const auto net_it = m_net_by_name.find(signal); net_it == m_net_by_name.end())
                    {
                        log_error("verilog_parser", "signal '{}' of instance '{}' of type '{}' has not been declared.", signal, instance->m_name, instance->m_type);
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
                            log_error("verilog_parser", "undefined pin '{}' for gate '{}' of type '{}'.", pin, new_gate->get_name(), new_gate->get_type()->get_name());
                            return nullptr;
                        }

                        if (is_output && !current_net->add_source(new_gate, pin))
                        {
                            return nullptr;
                        }

                        if (is_input && !current_net->add_destination(new_gate, pin))
                        {
                            return nullptr;
                        }
                    }
                }
            }
            else
            {
                log_error("verilog_parser", "could not find gate type '{}' in gate library '{}'.", instance->m_type, m_netlist->get_gate_library()->get_name());
                return nullptr;
            }

            // assign instance attributes
            for (const VerilogDataEntry& attribute : instance->m_attributes)
            {
                if (!container->set_data("attribute", attribute.m_name, attribute.m_type, attribute.m_value))
                {
                    log_warning("verilog_parser",
                                "could not set attribute ({} = {}) for instance '{}' of type '{}' within instance '{}' of type '{}'.",
                                attribute.m_name,
                                attribute.m_value,
                                instance->m_name,
                                instance->m_type,
                                instance_identifier,
                                instance_type);
                }
            }

            // process generics
            for (const VerilogDataEntry& parameter : instance->m_parameters)
            {
                if (!container->set_data("generic", parameter.m_name, parameter.m_type, parameter.m_value))
                {
                    log_warning("verilog_parser",
                                "could not set generic ({}, {}, {}) for instance '{}' of type '{}' within instance '{}' of type '{}'.",
                                parameter.m_name,
                                parameter.m_type,
                                parameter.m_value,
                                instance->m_name,
                                instance->m_type,
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

    std::string VerilogParser::get_hex_from_literal(const Token<std::string>& value_token) const
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
                    log_error("verilog_parser", "invalid character within binary number literal {} in line {}.", value, line_number);
                    return "";
                }

                base = 2;
                break;
            }

            case 'O': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '7'); }))
                {
                    log_error("verilog_parser", "invalid character within octal number literal {} in line {}.", value, line_number);
                    return "";
                }

                base = 8;
                break;
            }

            case 'D': {
                if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9'); }))
                {
                    log_error("verilog_parser", "invalid character within decimal number literal {} in line {}.", value, line_number);
                    return "";
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
                        log_error("verilog_parser", "invalid character within hexadecimal number literal {} in line {}.", value, line_number);
                        return "";
                    }
                }

                return res;
            }

            default: {
                log_error("verilog_parser", "invalid base '{}' within number literal {} in line {}.", prefix, value, line_number);
                return "";
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
        return ss.str();
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
