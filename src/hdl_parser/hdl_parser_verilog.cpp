#include "hdl_parser/hdl_parser_verilog.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

#include <iomanip>
#include <iostream>

//#include <queue>

hdl_parser_verilog::hdl_parser_verilog(std::stringstream& stream) : hdl_parser(stream)
{
}

// ###########################################################################
// ###########          Parse HDL into intermediate format          ##########
// ###########################################################################

std::shared_ptr<netlist> hdl_parser_verilog::parse(const std::string& gate_library)
{
    m_netlist = netlist_factory::create_netlist(gate_library);
    if (m_netlist == nullptr)
    {
        log_error("hdl_parser", "netlist_factory returned nullptr");
        return nullptr;
    }

    // read the whole file into logical parts
    module current_module;

    std::string line, token;

    u32 line_number          = 0;
    i32 token_begin          = -1;
    bool multi_line_comment  = false;
    bool multi_line_property = false;

    while (std::getline(m_fs, line))
    {
        line_number++;

        // remove single-line and multi-line comments
        this->remove_comments(line, multi_line_comment, multi_line_property);

        line = core_utils::trim(line);

        // skip empty lines
        if (line.empty())
        {
            continue;
        }

        // build tokens
        token += line;
        if (token_begin == -1)
        {
            token_begin = line_number;
        }

        // tokens end with ";"
        if (core_utils::ends_with(token, ";"))
        {
            token = core_utils::trim(token.substr(0, token.find(';')));

            if (core_utils::starts_with(token, "module "))
            {
                // reset current_module
                current_module.name        = token.substr(0, token.find('(')).substr(token.find(' ') + 1);
                current_module.line_number = static_cast<u32>(token_begin);
                current_module.definition.ports.clear();
                current_module.definition.wires.clear();
                current_module.definition.instances.clear();
            }
            else if (core_utils::starts_with(token, "input ") || core_utils::starts_with(token, "output ") || core_utils::starts_with(token, "inout "))
            {
                // collect port declarations
                current_module.definition.ports.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else if (core_utils::starts_with(token, "wire "))
            {
                // collect wire declarations
                current_module.definition.wires.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else if (core_utils::starts_with(token, "assign "))
            {
                // collect assign statements
                current_module.definition.assigns.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else
            {
                // collect instance declarations
                current_module.definition.instances.push_back(file_line{static_cast<u32>(token_begin), token});
            }

            token_begin = -1;
            token.clear();
        }
        else if (core_utils::starts_with(token, "endmodule"))
        {
            // add module to set of modules
            m_modules[current_module.name] = current_module;

            token_begin = -1;
            token.clear();
        }
    }

    if (m_modules.empty())
    {
        log_error("hdl_parser", "file did not contain any modules.");
        return nullptr;
    }

    // parse intermediate format
    for (auto& it : m_modules)
    {
        if (!this->parse_module(it.second))
        {
            return nullptr;
        }
    }

    return nullptr;
}

bool hdl_parser_verilog::parse_module(module& m)
{
    if (!parse_ports(m))
    {
        return false;
    }

    if (!parse_wires(m))
    {
        return false;
    }

    // TODO re-enable
    // if (!parse_assigns(m))
    // {
    //     return false;
    // }

    if (!parse_instances(m))
    {
        return false;
    }

    return true;
}

bool hdl_parser_verilog::parse_ports(module& m)
{
    for (const auto& line : m.definition.ports)
    {
        // get port information
        auto direction = line.text.substr(0, line.text.find(' '));
        auto names     = line.text.substr(line.text.find(' ') + 1);

        // add all signals of that port
        for (const auto& it : this->get_expanded_wire_signals(names))
        {
            for (const auto& signal : it.second)
            {
                m.ports.emplace_back(signal, direction);
                m.expanded_signal_names[it.first].push_back(signal);
            }
        }
    }

    log_debug("hdl_parser", "parsed ports of '{}'.", m.name);
    return true;
}

bool hdl_parser_verilog::parse_wires(module& m)
{
    for (const auto& line : m.definition.wires)
    {
        // get wire information
        auto names = line.text.substr(line.text.find(' ') + 1);

        // add all (sub-)signals
        for (const auto& it : this->get_expanded_wire_signals(names))
        {
            for (const auto& signal : it.second)
            {
                m.signals.push_back(signal);
                m.expanded_signal_names[it.first].push_back(signal);
            }
        }
    }

    log_debug("hdl_parser", "parsed signals of '{}'.", m.name);
    return true;
}

bool hdl_parser_verilog::parse_assigns(module& m)
{
    for (const auto& line : m.definition.assigns)
    {
        auto signal     = core_utils::trim(line.text.substr(6, line.text.find('=') - 6));
        auto assignment = core_utils::trim(line.text.substr(line.text.find('=') + 1));

        // this->get_port_assignments(signal, assignment);
        // TODO finish assignment parser
    }

    return false;
}

bool hdl_parser_verilog::parse_instances(module& m)
{
    for (const auto& line : m.definition.instances)
    {
        instance inst;
        std::vector<std::string> generic_map, port_map;
        auto generic_pos = line.text.find("#(");

        if (generic_pos == std::string::npos)
        {
            // TYPE NAME (PORT_MAP)
            inst.type = line.text.substr(0, line.text.find('('));
            inst.name = core_utils::trim(inst.type.substr(inst.type.find(' ') + 1));
            inst.type = core_utils::trim(inst.type.substr(0, inst.type.find(' ')));
            port_map  = core_utils::split(line.text.substr(0, line.text.rfind(')')).substr(line.text.find('(') + 1), ',');
        }
        else
        {
            // TYPE #(GENERIC_MAP) NAME (PORT_MAP)
            auto substr = line.text.substr(line.text.find('#') + 1);
            auto token  = core_utils::split(substr, ')', true);

            inst.type   = core_utils::trim(line.text.substr(0, line.text.find("#(")));
            inst.name   = core_utils::trim(token[1].substr(0, token[1].find('(')));
            generic_map = core_utils::split(token[0].substr(1), ',');
            port_map    = core_utils::split(token[1].substr(token[1].find('(') + 1), ',');
        }

        for (const auto& port : port_map)
        {
            if (port.empty())
            {
                continue;
            }

            // .KEY(VALUE)
            auto key   = core_utils::trim(port.substr(0, port.find('('))).substr(1);
            auto value = core_utils::trim(port.substr(0, port.find(')')).substr(port.find('(') + 1));

            get_port_assignments(key, value, m);
            //inst.ports.emplace_back(key, value);
        }

        std::cout << std::endl;

        for (const auto& generic : generic_map)
        {
            if (generic.empty())
            {
                continue;
            }

            // .KEY(VALUE)
            auto key   = core_utils::trim(generic.substr(0, generic.find('('))).substr(1);
            auto value = core_utils::trim(generic.substr(0, generic.find(')')).substr(generic.find('(') + 1));

            inst.generics.emplace_back(key, value);
        }
    }

    return false;
}

// ###########################################################################
// ###################          Helper functions          ####################
// ###########################################################################

void hdl_parser_verilog::remove_comments(std::string& line, bool& multi_line_comment, bool& multi_line_property)
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

        auto single_line_comment_begin = line.find("//");
        auto multi_line_comment_begin  = line.find("/*");
        auto multi_line_comment_end    = line.find("*/");
        auto multi_line_property_begin = line.find("(*");
        auto multi_line_property_end   = line.find("*)");

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
        else if (multi_line_property == true)
        {
            if (multi_line_property_end != std::string::npos)
            {
                // multi-line property ends in current line
                multi_line_property = false;
                line                = line.substr(multi_line_property_end + 2);
                repeat              = true;
            }
            else
            {
                // current line entirely in multi-line property
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
            else if (multi_line_property_begin != std::string::npos)
            {
                if (multi_line_property_end != std::string::npos)
                {
                    // multi-line property entirely in current line
                    line   = line.substr(0, multi_line_property_begin) + line.substr(multi_line_property_end + 2);
                    repeat = true;
                }
                else
                {
                    // multi-line property starts in current line
                    multi_line_property = true;
                    line                = line.substr(0, multi_line_property_begin);
                }
            }
        }
    }
}

std::map<std::string, std::vector<std::string>> hdl_parser_verilog::get_expanded_wire_signals(const std::string& line)
{
    std::map<std::string, std::vector<std::string>> result;
    std::vector<std::pair<i32, i32>> bound_tokens;
    std::vector<std::string> names;

    auto colon_pos   = line.find(':');
    auto bracket_pos = line.find('[');

    if (bracket_pos != std::string::npos && colon_pos != std::string::npos)
    {
        // extract bounds "[a:b][c:d]..."
        auto bounds = line.substr(0, line.rfind("]")).substr(line.find("[") + 1);
        names       = core_utils::split(line.substr(line.rfind(']') + 1), ',');

        for (const auto& part : core_utils::split(bounds, ']'))
        {
            auto bound = core_utils::trim(part.substr(part.find('[') + 1));

            i32 left_bound  = std::stoi(bound.substr(0, bound.find(':')));
            i32 right_bound = std::stoi(bound.substr(bound.find(':') + 1));

            bound_tokens.emplace_back(left_bound, right_bound);
        }
    }
    else if (bracket_pos != std::string::npos)
    {
        log_error("hdl_parser", "not implemented reached for wire '{}'", line);
        return {};
    }
    else
    {
        // no bounds given, just extract signals from list
        names = core_utils::split(line, ',');
    }

    for (auto name : names)
    {
        u32 dimension = bound_tokens.size();
        name          = core_utils::trim(name);

        if (dimension == 0)
        {
            result[name].push_back(name);
        }
        else if (dimension == 1)
        {
            if (bound_tokens[0].first > bound_tokens[0].second)
            {
                for (auto i = bound_tokens[0].first; i >= bound_tokens[0].second; i--)
                {
                    result[name].push_back(name + "(" + std::to_string(i) + ")");
                }
            }
            else
            {
                for (auto i = bound_tokens[0].first; i <= bound_tokens[0].second; i++)
                {
                    result[name].push_back(name + "(" + std::to_string(i) + ")");
                }
            }
        }
        else
        {
            log_error("hdl_parser", "not implemented reached for dimension '{}' (did you forget to add the case here?)", dimension);
            return {};
        }
    }

    return result;
}

std::map<std::string, std::string> hdl_parser_verilog::get_port_assignments(const std::string& port, const std::string& assignment, module& m)
{
    std::map<std::string, std::string> result;
    std::vector<std::string> left_parts, right_parts;

    // PARSE ASSIGNMENT
    //   assignment can currently be one of the following:
    //   (1) NAME *single*
    //   (2) NAME *one-dimensional*
    //   (3) NUMBER
    //   (4) NAME[INDEX1][INDEX2]...
    //   (5) NAME[BEGIN_INDEX:END_INDEX]
    // TODO  (6) {(1) - (5), (1) - (5), ...}

    auto assignment_bracket = assignment.find('[');
    auto assignment_number  = assignment.find('\'');

    if (assignment_bracket == std::string::npos && assignment_number == std::string::npos)
    {
        // (1) and (2)
        if (m.expanded_signal_names.find(assignment) == m.expanded_signal_names.end())
        {
            log_error("hdl_parser", "no wire or port '{}' within current module.", assignment);
            return {};
        }

        right_parts = m.expanded_signal_names[assignment];
    }
    else if (assignment_number != std::string::npos)
    {
        // (3)
        auto number = get_bin_from_number_literal(assignment);
        for (auto bit : number)
        {
            right_parts.push_back("'" + std::to_string(bit - 48) + "'");
        }
    }
    else if (assignment_bracket != std::string::npos)
    {
        // (4) and (5)
        auto assignment_colon = assignment.find(':');

        if (assignment_colon == std::string::npos)
        {
            // (4)
            auto substr = assignment.substr(0, assignment.rfind(']')).substr(assignment_bracket + 1);
            auto name   = core_utils::trim(assignment.substr(0, assignment_bracket));

            for (const auto& part : core_utils::split(substr, ']'))
            {
                auto index = std::stoi(core_utils::trim(part.substr(part.find('[') + 1)));
                name += "(" + std::to_string(index) + ")";
            }

            right_parts.push_back(name);
        }
        else
        {
            // (5)
            auto bounds = core_utils::split(assignment.substr(0, assignment.rfind(']')).substr(assignment_bracket + 1), ':');
            auto name   = core_utils::trim(assignment.substr(0, assignment_bracket));

            auto left_bound  = std::stoi(core_utils::trim(bounds[0]));
            auto right_bound = std::stoi(core_utils::trim(bounds[1]));

            if (left_bound > right_bound)
            {
                for (auto i = left_bound; i >= right_bound; i--)
                {
                    right_parts.push_back(name + "(" + std::to_string(i) + ")");
                }
            }
            else
            {
                for (auto i = left_bound; i <= right_bound; i++)
                {
                    right_parts.push_back(name + "(" + std::to_string(i) + ")");
                }
            }
        }
    }
    else
    {
        log_error("hdl_parser", "not implemented reached for port assignment '{}'", assignment);
        return {};
    }

    // PARSE PORT
    //   port can currently be one of the following:
    //   (1) NAME *single*
    //   (2) NAME *one-dimensional*
    //   (3) NAME[INDEX1][INDEX2]...
    //   (4) NAME[BEGIN_INDEX:END_INDEX]

    auto port_bracket = port.find('[');

    if (port_bracket == std::string::npos)
    {
        // (1) and (2)
        // infer size from right side
        if (right_parts.size() > 1)
        {
            for (u32 i = right_parts.size(); i > 0; i--)
            {
                left_parts.push_back(port + "(" + std::to_string(i - 1) + ")");
            }
        }
        else
        {
            left_parts.push_back(port);
        }
    }
    else if (port_bracket != std::string::npos)
    {
        // (3) and (4)
        auto port_colon = port.find(':');

        if (port_colon == std::string::npos)
        {
            // (3)
            auto substr = port.substr(0, port.rfind(']')).substr(port_bracket + 1);
            auto name   = core_utils::trim(port.substr(0, port_bracket));

            for (const auto& part : core_utils::split(substr, ']'))
            {
                auto index = std::stoi(core_utils::trim(part.substr(part.find('[') + 1)));
                name += "(" + std::to_string(index) + ")";
            }

            left_parts.push_back(name);
        }
        else
        {
            // (4)
            auto bounds = core_utils::split(port.substr(0, port.rfind(']')).substr(port_bracket + 1), ':');
            auto name   = core_utils::trim(port.substr(0, port_bracket));

            auto left_bound  = std::stoi(core_utils::trim(bounds[0]));
            auto right_bound = std::stoi(core_utils::trim(bounds[1]));

            if (left_bound > right_bound)
            {
                for (auto i = left_bound; i >= right_bound; i--)
                {
                    left_parts.push_back(name + "(" + std::to_string(i) + ")");
                }
            }
            else
            {
                for (auto i = left_bound; i <= right_bound; i++)
                {
                    left_parts.push_back(name + "(" + std::to_string(i) + ")");
                }
            }
        }
    }
    else
    {
        log_error("hdl_parser", "not implemented reached for port '{}'", port);
        return {};
    }

    if (left_parts.size() != right_parts.size())
    {
        log_error("hdl_parser", "cannot connect wires due to width mismatch.");
        return {};
    }

    for (u32 i = 0; i < left_parts.size(); i++)
    {
        std::cout << left_parts[i] << " = " << right_parts[i] << std::endl;
    }

    return {};
}

std::string hdl_parser_verilog::get_bin_from_number_literal(const std::string& v)
{
    std::string value = core_utils::to_lower(core_utils::trim(core_utils::replace(v, "_", "")));

    u32 len = 0, radix = 0;
    std::string length, prefix, number;

    // base specified?
    if (value.find('\'') == std::string::npos)
    {
        prefix = "d";
        number = value;
    }
    else
    {
        length = value.substr(0, value.find('\''));
        prefix = value.substr(value.find('\'') + 1, 1);
        number = value.substr(value.find('\'') + 2);
    }

    // select radix
    if (prefix == "b")
    {
        radix = 2;
    }
    else if (prefix == "o")
    {
        radix = 8;
    }
    else if (prefix == "d")
    {
        radix = 10;
    }
    else if (prefix == "h")
    {
        radix = 16;
    }

    // constructing bitstring
    u64 val = stoull(number, 0, radix);
    std::string res;

    if (!length.empty())
    {
        len = std::stoi(length);

        for (u32 i = 0; i < len; i++)
        {
            res = std::to_string(val & 0x1) + res;
            val >>= 1;
        }
    }
    else
    {
        do
        {
            res = std::to_string(val & 0x1) + res;
            val >>= 1;
        } while (val != 0);
    }

    return res;
}