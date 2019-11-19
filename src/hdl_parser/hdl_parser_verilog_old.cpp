#include <hdl_parser/hdl_parser_verilog_old.h>
#include <iostream>

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

hdl_parser_verilog_old::hdl_parser_verilog_old(std::stringstream& stream) : hdl_parser(stream)
{
}

std::shared_ptr<netlist> hdl_parser_verilog_old::parse(const std::string& gate_library)
{
    m_netlist = netlist_factory::create_netlist(gate_library);
    if (m_netlist == nullptr)
    {
        log_error("hdl_parser", "netlist_factory returned nullptr.");
        return nullptr;
    }

    // read whole file into token parts
    std::string buffer, token;
    std::vector<std::tuple<int, std::string>> tokens;

    int current_line       = 0;
    int token_begin        = -1;
    bool multiline_comment = false, multiline_property = false;
    while (std::getline(m_fs, buffer))
    {
        current_line++;

        remove_comments(buffer, multiline_comment, multiline_property);

        buffer = core_utils::trim(buffer);

        if (buffer.empty())
        {
            continue;
        }

        // add current line to token
        token += buffer;
        if (token_begin == -1)
        {
            token_begin = current_line;
        }

        // finalize the token when it is done
        if (token.back() == ';')
        {
            token.pop_back();                           // remove ';' from string
            tokens.emplace_back(token_begin, token);    // emplace_back creates the tuple automatically
            token_begin = -1;
            token.clear();
        }
    }

    // parse token parts: module, wire, assign, instance
    auto gate_types = m_netlist->get_gate_library()->get_gate_types();
    for (const auto& it : tokens)
    {
        auto line_number = std::get<0>(it);
        token            = std::get<1>(it);
        auto identifier  = token.substr(0, token.find(" "));

        if (identifier == "module")
        {
            if (!this->parse_module(token, line_number))
            {
                return nullptr;
            }
        }
        else if ((identifier == "input") || (identifier == "output") || (identifier == "wire"))
        {
            if (!this->parse_architecture(token, identifier, line_number))
            {
                return nullptr;
            }
        }
        else if (identifier == "assign")
        {
            if (!this->parse_assign(token, line_number))
            {
                return nullptr;
            }
        }
        else if (gate_types->find(identifier) != gate_types->end())
        {
            if (!this->parse_instance(token, identifier, line_number))
            {
                return nullptr;
            }
        }
        else if (identifier == "endmodule")
        {
            break;
        }
        else
        {
            log_error("hdl_parser", "cannot parse token '{}' (line: {}).", token, line_number);
            return nullptr;
        }
    }

    // add global gnd and vcc signals
    std::map<std::string, std::shared_ptr<net>>::iterator it;
    if (((it = m_net.find("1'h0")) != m_net.end()) || ((it = m_net.find("1'b0")) != m_net.end()))
    {
        auto gnd_type   = *(m_netlist->get_gate_library()->get_global_gnd_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pins(gnd_type).at(0);
        auto gnd        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");
        if (!m_netlist->mark_global_gnd_gate(gnd))
        {
            return nullptr;
        }
        auto gnd_net = it->second;
        if (!gnd_net->set_src(gnd, output_pin))
        {
            return nullptr;
        }
    }
    if (((it = m_net.find("1'h1")) != m_net.end()) || ((it = m_net.find("1'b1")) != m_net.end()))
    {
        auto vcc_type   = *(m_netlist->get_gate_library()->get_global_vcc_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pins(vcc_type).at(0);
        auto vcc        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");
        if (!m_netlist->mark_global_vcc_gate(vcc))
        {
            return nullptr;
        }
        auto vcc_net = it->second;
        if (!vcc_net->set_src(vcc, output_pin))
        {
            return nullptr;
        }
    }
    return m_netlist;
}

void hdl_parser_verilog_old::remove_comments(std::string& buffer, bool& multiline_comment, bool& multiline_property)
{
    bool repeat = true;
    while (repeat)
    {
        repeat = false;

        // skip empty lines and single line comments
        if (buffer.empty())
        {
            break;
        }

        auto single_line_comment_begin = buffer.find("//");
        auto multi_line_comment_begin  = buffer.find("/*");
        auto multi_line_comment_end    = buffer.find("*/");
        auto multi_line_property_begin = buffer.find("(*");
        auto multi_line_property_end   = buffer.find("*)");

        std::string begin = "";
        std::string end   = "";

        if (!multiline_comment)
        {
            if (single_line_comment_begin != std::string::npos)
            {
                if (multi_line_comment_begin == std::string::npos || (multi_line_comment_begin != std::string::npos && multi_line_comment_begin > single_line_comment_begin))
                {
                    buffer = buffer.substr(0, single_line_comment_begin);
                    repeat = true;
                }
            }
            else if (multi_line_comment_begin != std::string::npos && multi_line_comment_end != std::string::npos)
            {
                if (multi_line_comment_begin == 0 && (multi_line_comment_end + 2) == buffer.size())
                {
                    buffer = "";
                    continue;
                }
                if (multi_line_comment_begin > 0)
                {
                    begin = buffer.substr(0, multi_line_comment_begin - 1);
                }
                if ((multi_line_comment_end + 2) < buffer.size())
                {
                    end = buffer.substr(multi_line_comment_end + 2);
                }
                buffer = begin + end;
                repeat = true;
            }
            else if (multi_line_comment_begin != std::string::npos)
            {
                multiline_comment = true;
                buffer            = buffer.substr(0, multi_line_comment_begin);
                repeat            = true;
            }
            else if (!multiline_property && multi_line_property_begin != std::string::npos && multi_line_property_end != std::string::npos)
            {
                if (multi_line_property_begin == 0 && (multi_line_property_end + 2) == buffer.size())
                {
                    buffer = "";
                    continue;
                }
                if (multi_line_property_begin > 0)
                {
                    begin = buffer.substr(0, multi_line_property_begin - 1);
                }
                if ((multi_line_property_end + 2) < buffer.size())
                {
                    end = buffer.substr(multi_line_property_end + 2);
                }
                buffer = begin + end;
                repeat = true;
            }
            else if (!multiline_property && multi_line_property_begin != std::string::npos)
            {
                multiline_property = true;
                buffer             = buffer.substr(0, multi_line_property_begin);
                repeat             = true;
            }
            else if (multiline_property && multi_line_property_end != std::string::npos)
            {
                multiline_property = false;
                buffer             = buffer.substr(multi_line_property_end + 2);
                repeat             = true;
            }
        }
        else
        {
            if (multi_line_comment_end != std::string::npos)
            {
                multiline_comment = false;
                buffer            = buffer.substr(multi_line_comment_end + 2);
                repeat            = true;
            }
        }
    }
}

bool hdl_parser_verilog_old::parse_module(const std::string& module, const int line)
{
    UNUSED(line);
    /* parse design name */
    auto design_name = core_utils::trim(module.substr(module.find("module") + 6));
    design_name      = design_name.substr(0, design_name.find("("));
    m_netlist->set_design_name(core_utils::trim(design_name));

    return true;
}

bool hdl_parser_verilog_old::parse_architecture(const std::string& signal_token, const std::string& identifier, const int line)
{
    std::map<std::string, std::function<int(std::shared_ptr<netlist> const, std::shared_ptr<net> const)>> identifier_to_addition = {
        {"input", [](std::shared_ptr<netlist> const g, std::shared_ptr<net> const net) { return g->mark_global_input_net(net); }},
        {"output", [](std::shared_ptr<netlist> const g, std::shared_ptr<net> const net) { return g->mark_global_output_net(net); }},
        {"inout", [](std::shared_ptr<netlist> const g, std::shared_ptr<net> const net) { return g->mark_global_inout_net(net); }},
        {"wire",
         [](std::shared_ptr<netlist> const g, std::shared_ptr<net> const net) {
             UNUSED(g);
             UNUSED(net);
             return true;
         }},
    };
    auto signal_list = core_utils::trim(signal_token.substr(signal_token.find(identifier) + identifier.size()));
    auto bus_indices = this->get_vector_bounds(signal_list);

    if (bus_indices.empty())
    {
        for (auto name : core_utils::split(signal_list, ','))
        {
            if (name.empty())
            {
                continue;
            }
            name = core_utils::trim(name);
            name = std::string(name.begin(), std::remove_if(name.begin(), name.end(), isspace));

            if (m_net.find(name) != m_net.end())
            {
                continue;
            }

            auto new_net               = m_netlist->create_net(m_netlist->get_unique_net_id(), name);
            m_net[new_net->get_name()] = new_net;
            if (new_net == nullptr || !identifier_to_addition[identifier](m_netlist, new_net))
            {
                log_error("hdl_parser", "cannot parse '{}' (line: {}).", identifier, line);
                return false;
            }
        }
    }
    else
    {
        signal_list = core_utils::trim(signal_list.substr(static_cast<unsigned long>(get_idx_of_last_vector_bound(signal_list))));
        for (auto name : core_utils::split(signal_list, ','))
        {
            if (name.empty())
            {
                continue;
            }
            name = core_utils::trim(name);
            name = std::string(name.begin(), std::remove_if(name.begin(), name.end(), isspace));

            for (const auto bus_index : bus_indices)
            {
                auto bus_signal_name = name + "[" + std::to_string(bus_index) + "]";

                if (m_net.find(bus_signal_name) != m_net.end())
                {
                    continue;
                }

                auto new_net               = m_netlist->create_net(m_netlist->get_unique_net_id(), bus_signal_name);
                m_net[new_net->get_name()] = new_net;

                if (new_net == nullptr || !identifier_to_addition[identifier](m_netlist, new_net))
                {
                    log_error("hdl_parser", "cannot parse '{}' (line: {}).", identifier, line);
                    return false;
                }
            }
            if (m_net.find(name) != m_net.end())
            {
                m_netlist->delete_net(m_net[name]);
                m_net.erase(m_net.find(name));
            }
        }
    }

    return true;
}

bool hdl_parser_verilog_old::parse_instance(const std::string& instance, const std::string& type, const int line)
{
    // split instance into logical parts: name, generic map, and port map
    std::string name, generic_map, port_map;

    if (instance.find("#(") == std::string::npos)
    {
        // no generic map present
        name = core_utils::trim(instance.substr(instance.find(type) + type.size()));
        name = core_utils::trim(name.substr(0, name.find('(')));

        port_map = instance.substr(instance.find('('));
        port_map = core_utils::trim(port_map.substr(1, port_map.size() - 2));
    }
    else
    {
        auto substr = core_utils::trim(instance.substr(instance.find('#') + 1));
        auto token  = core_utils::split(substr, ')', true);

        name = core_utils::trim(token[1].substr(0, token[1].find('(')));

        generic_map = token[0].substr(1);

        port_map = core_utils::trim(token[1].substr(token[1].find('(') + 1));
    }

    // add gate to netlist and check for global vcc / gnd gates
    auto new_gate = m_netlist->create_gate(m_netlist->get_unique_gate_id(), type, name);
    if (new_gate == nullptr)
    {
        log_error("hdl_parser", "cannot parse instance '{}' (line: {}).", name, line);
        return false;
    }

    auto global_vcc_gate_types = m_netlist->get_gate_library()->get_global_vcc_gate_types();
    if (global_vcc_gate_types->find(type) != global_vcc_gate_types->end())
    {
        if (!m_netlist->mark_global_vcc_gate(new_gate))
        {
            log_error("hdl_parser", "cannot parse instance '{}' (line: {}).", name, line);
            return false;
        }
    }

    auto global_gnd_gate_types = m_netlist->get_gate_library()->get_global_gnd_gate_types();
    if (global_gnd_gate_types->find(type) != global_gnd_gate_types->end())
    {
        if (!m_netlist->mark_global_gnd_gate(new_gate))
        {
            log_error("hdl_parser", "cannot parse instance '{}' (line: {}).", name, line);
            return false;
        }
    }

    // parse generic map
    for (const auto& token : core_utils::split(generic_map, ',', true))
    {
        if (token.empty())
        {
            continue;
        }

        auto key            = core_utils::trim(token.substr(1, token.find('(') - 1));
        auto value          = core_utils::trim(token.substr(token.find('(') + 1));
        auto value_stripped = core_utils::trim(value.substr(0, value.length() - 1));

        if (value.back() == ',')
        {
            value.pop_back();
        }

        // determine data type
        auto data_type = std::string();
        if (core_utils::is_integer(value))
        {
            data_type = "integer";
        }
        else if (core_utils::is_floating_point(value))
        {
            data_type = "floating_point";
        }
        else if (core_utils::starts_with(value, "\"") && core_utils::ends_with(value, "\""))
        {
            value     = value.substr(1, value.size() - 2);
            data_type = "string";
        }
        else if (value.find('\'') != std::string::npos)
        {
            value     = get_hex_from_number_literal(value);
            data_type = "bit_vector";
        }
        else
        {
            log_error("hdl_parser", "cannot identify data type of generic map value '{}' in instance '{}'", value, new_gate->get_name());
            return false;
        }

        if (!new_gate->set_data("generic", key, data_type, value))
        {
            log_error("hdl_parser", "cannot parse generic attribute '{}' in instance '{}' (line: {}).", key, name, line);
            return false;
        }
    }

    auto splited = core_utils::split(port_map, ',', true);
    for (const auto& token : splited)
    {
        if (token.empty())
        {
            continue;
        }
        auto pin      = core_utils::trim(token.substr(0, token.find("(")));
        auto net_name = token.substr(token.find('(') + 1);
        net_name      = core_utils::trim(net_name.substr(0, net_name.find(')')));
        // remove start char '.' from pin
        pin = pin.substr(1);

        auto pins = this->parse_pin(new_gate, pin, line);
        auto nets = this->parse_net(net_name, line);

        if (pins.size() != nets.size())
        {
            std::string imploded_nets;
            for (const auto& net : nets)
            {
                imploded_nets += net + ",";
            }
            std::string imploded_pins;
            for (const auto& p : pins)
            {
                imploded_pins += p + ",";
            }
            log_error("hdl_parser",
                      "Cannot connect nets ({}; len={}) to pins ({}; len={}) of gate {} line: {}",
                      imploded_nets,
                      std::to_string(nets.size()),
                      imploded_pins,
                      std::to_string(pins.size()),
                      name,
                      line);
            return false;
        }

        for (u32 i = 0; i < pins.size(); ++i)
        {
            if (!this->connect_net_to_pin(nets[i], new_gate, pins[i], line))
            {
                return false;
            }
        }
    }
    return true;
}

bool hdl_parser_verilog_old::connect_net_to_pin(const std::string& net_name, std::shared_ptr<gate>& new_gate, const std::string& pin_name, const int line)
{
    // unconnected case
    if (net_name.empty())
    {
        return true;
    }

    auto input_pins  = new_gate->get_input_pins();
    auto output_pins = new_gate->get_output_pins();
    auto inout_pins  = new_gate->get_inout_pins();
    // add non-registered signal
    if (m_net.find(net_name) == m_net.end())
    {
        auto new_net = m_netlist->create_net(m_netlist->get_unique_net_id(), net_name);
        if (new_net == nullptr)
        {
            log_error("hdl_parser", "cannot parse instance '{}' (line: {}).", new_gate->get_name(), line);
            return false;
        }

        m_net[net_name] = new_net;

        if ((net_name != "1'h0") && (net_name != "1'h1") && (net_name != "1'b0") && (net_name != "1'b1") && (net_name != "1'bz") && (net_name != "1'bx"))
            log_warning("hdl_parser", "signal '{}' was not previously declared in architecture (line: {})", net_name, line);
    }
    auto current_net = m_net[net_name];

    if ((std::find(input_pins.begin(), input_pins.end(), pin_name) == inout_pins.end())
        && (std::find(output_pins.begin(), output_pins.end(), pin_name) == output_pins.end())
        && (std::find(inout_pins.begin(), inout_pins.end(), pin_name) == inout_pins.end()))
    {
        log_error("hdl_parser", "undefined pin '{}' for '{}' (line: {}).", pin_name, new_gate->get_name(), line);
        return false;
    }

    if ((std::find(input_pins.begin(), input_pins.end(), pin_name) != input_pins.end())
        || (std::find(inout_pins.begin(), inout_pins.end(), pin_name) != inout_pins.end()))
    {
        if (!current_net->add_dst(new_gate, pin_name))
        {
            return false;
        }
    }
    if ((std::find(output_pins.begin(), output_pins.end(), pin_name) != output_pins.end())
        || (std::find(inout_pins.begin(), inout_pins.end(), pin_name) != inout_pins.end()))
    {
        if (!current_net->set_src(new_gate, pin_name))
        {
            return false;
        }
    }
    return true;
}

std::string hdl_parser_verilog_old::get_hex_from_number_literal(const std::string& v)
{
    std::string value = v;
    if (value.find("'h") != std::string::npos)
    {
        value = value.substr(value.find('\'') + 2);
        value = core_utils::to_lower(value);
        return value;
    }
    int radix = 10;
    if (value.find("'d") != std::string::npos || value.find('\'') == std::string::npos)
    {
        radix = 10;
    }
    if (value.find("'b") != std::string::npos)
    {
        radix = 2;
    }
    if (value.find("'o") != std::string::npos)
    {
        radix = 8;
    }
    value = value.substr(value.find('\'') + 2);
    std::stringstream ss;
    ss << std::hex << stoull(value, 0, radix);
    value = ss.str();
    return value;
}

bool hdl_parser_verilog_old::parse_assign(const std::string& token, const int line)
{
    // parse net names
    auto net_name_left  = token.substr(token.find("assign") + 6);
    net_name_left       = core_utils::trim(net_name_left.substr(0, net_name_left.find("=")));
    auto net_name_right = core_utils::trim(token.substr(token.find('=') + 1));

    auto nets_lhs = this->parse_net(net_name_left, line);
    auto nets_rhs = this->parse_net(net_name_right, line);

    if (nets_lhs.size() != nets_rhs.size())
    {
        std::string imploded_nets1;
        for (const auto& net : nets_lhs)
        {
            imploded_nets1 += net + ",";
        }
        std::string imploded_nets2;
        for (const auto& net : nets_rhs)
        {
            imploded_nets2 += net + ",";
        }
        log_error(
            "hdl_parser", "Cannot connect nets ({}; len={}) to pins ({}; len={}) line: {}", imploded_nets1, std::to_string(nets_lhs.size()), imploded_nets2, std::to_string(nets_rhs.size()), line);
        return false;
    }
    for (u32 i = 0; i < nets_lhs.size(); ++i)
    {
        std::string gate_name = nets_lhs[i] + "_" + "buffer";
        auto new_gate         = m_netlist->create_gate(m_netlist->get_unique_gate_id(), "BUF", gate_name);    //Hardcoded for UNISIM lib Very hacky bad bad bad
        if (new_gate == nullptr)
        {
            return false;
        }
        if (!this->connect_net_to_pin(nets_rhs[i], new_gate, "I", line))
        {
            return false;
        }
        if (!this->connect_net_to_pin(nets_lhs[i], new_gate, "O", line))
        {
            return false;
        }
    }
    return true;
}

std::vector<int> hdl_parser_verilog_old::get_vector_bounds(const std::string& s)
{
    if (s[0] != '[')
    {
        return std::vector<int>();
    }

    /* remove brackets */
    auto bounds = s.substr(s.find('['));
    bounds      = core_utils::trim(bounds.substr(0, static_cast<unsigned long>(get_idx_of_last_vector_bound(bounds))));

    /* remove whitespace and '['*/
    bounds = std::string(bounds.begin(), std::remove_if(bounds.begin(), bounds.end(), isspace));
    bounds.erase(std::remove(bounds.begin(), bounds.end(), '['), bounds.end());

    if (bounds.find('+') != std::string::npos || bounds.find('-') != std::string::npos)
    {
        log_error("hdl_parser", "Unsupported vector range specification {}.", bounds);
    }

    std::vector<std::tuple<int, int>> bound_tokens;
    for (auto& bound : core_utils::split(bounds, ']'))
    {
        if (bound.empty())
        {
            continue;
        }
        int left_bound  = std::stoi(bound.substr(0, bound.find(':')));
        int right_bound = std::stoi(bound.substr(bound.find(':') + 1));

        bound_tokens.push_back(std::make_tuple(left_bound, right_bound));
    }

    std::vector<int> result;
    u32 dimension = (u32)std::count(bounds.begin(), bounds.end(), ']');
    switch (dimension)
    {
        case 1:
            if (std::get<0>(bound_tokens[0]) >= std::get<1>(bound_tokens[0]))
            {
                for (auto x = std::get<0>(bound_tokens[0]); x >= std::get<1>(bound_tokens[0]); --x)
                {
                    result.push_back(x);
                }
            }
            else
            {
                for (auto x = std::get<0>(bound_tokens[0]); x <= std::get<1>(bound_tokens[0]); ++x)
                {
                    result.push_back(x);
                }
                log_error("hdl_parser", "Unsupported vector range specification [{}:{}].", std::get<0>(bound_tokens[0]), std::get<1>(bound_tokens[0]));
            }

            break;
        default:
            log_error("hdl_parser", "not implemented reached for dimension '{}' (did you forget to add the case here?)", (int)dimension);
            return std::vector<int>();
    }

    return result;
}

int hdl_parser_verilog_old::get_idx_of_last_vector_bound(const std::string& s)
{
    int idx = 0;
    for (auto&& c : s)
    {
        if (c == ':' or c == ']' or c == '[' or isdigit(c))
        {
            idx++;
        }
        else
        {
            break;    //Found first character of net name
        }
    }
    return idx;
}

std::vector<std::string> hdl_parser_verilog_old::parse_net(const std::string& token, const int line)
{
    std::string t = token;
    t.erase(std::remove_if(t.begin(), t.end(), isspace), t.end());

    std::vector<std::string> wires;
    // 1. got array of values
    if (core_utils::starts_with(token, "{") && core_utils::ends_with(token, "}"))
    {
        t.erase(std::remove(t.begin(), t.end(), '{'), t.end());
        t.erase(std::remove(t.begin(), t.end(), '}'), t.end());

        // Iterate from MSB to LSB
        for (const auto& net_chunk : core_utils::split(t, ','))
        {
            if (net_chunk.empty())
            {
                continue;
            }
            auto chunk = this->parse_net_single(net_chunk, line);
            wires.insert(wires.end(), chunk.begin(), chunk.end());
        }
    }
    else
    {
        // 2. got single value
        wires = this->parse_net_single(t, line);
    }
    return wires;
}

std::vector<std::string> hdl_parser_verilog_old::parse_net_single(const std::string& t, const int line)
{
    std::string token = core_utils::trim(t);
    std::vector<std::string> nets;
    // 0. unconnected
    if (token.empty())
    {
        return {""};
    }

    // 1. Is numeric value
    if (this->is_numeric(token))
    {
        nets = this->get_binary_string_from_number_literal(token, line);
    }
    else
    {
        // 2. Is signal
        // 2.a Is single vector
        if (m_net.find(token) != m_net.end())
        {
            nets.push_back(token);
        }
        else
        {
            // 2.b Is vector of signals
            if (token.find("[") != std::string::npos)
            {
                auto base   = core_utils::trim(token.substr(0, token.find("[")));
                auto bounds = this->get_vector_bounds(token.substr(token.find("[")));
                for (const auto& bound : bounds)
                {
                    std::string net_name = base + "[" + std::to_string(bound) + "]";
                    if (m_net.find(net_name) == m_net.end())
                    {
                        log_error("hdl_parser", "Cannot find net {} line {}", net_name, line);
                        return std::vector<std::string>();
                    }
                    else
                    {
                        nets.push_back(net_name);
                    }
                }
            }
            else
            {
                int lowest = -1;
                for (const auto& net : m_net)
                {
                    if (core_utils::starts_with(net.second->get_name(), token + "["))
                    {
                        auto p = net.second->get_name().substr(net.second->get_name().find("[") + 1);
                        p      = p.substr(0, p.size() - 1);
                        if (lowest == -1 || std::stoi(p) < lowest)
                        {
                            lowest = std::stoi(p);
                        }
                        nets.push_back(net.second->get_name());
                    }
                }
                std::vector<std::string> nets_sorted;
                nets_sorted.resize(nets.size());
                for (const auto& net : nets)
                {
                    auto p                    = net.substr(net.find("[") + 1);
                    p                         = p.substr(0, p.size() - 1);
                    int idx                   = std::stoi(p);
                    nets_sorted[idx - lowest] = net;
                }
                std::reverse(nets_sorted.begin(), nets_sorted.end());
                nets = nets_sorted;
            }
        }
    }
    return nets;
}

std::vector<std::string> hdl_parser_verilog_old::parse_pin(std::shared_ptr<gate>& new_gate, const std::string& token, const int line)
{
    UNUSED(line);

    std::string t = token;
    t.erase(std::remove_if(t.begin(), t.end(), isspace), t.end());
    t.erase(std::remove(t.begin(), t.end(), '\\'), t.end());

    std::vector<std::string> pins;

    auto input_pins  = new_gate->get_input_pins();
    auto inout_pins  = new_gate->get_inout_pins();
    auto output_pins = new_gate->get_output_pins();

    if (std::find(input_pins.begin(), input_pins.end(), t) != input_pins.end() || std::find(inout_pins.begin(), inout_pins.end(), t) != inout_pins.end()
        || std::find(output_pins.begin(), output_pins.end(), t) != output_pins.end())
    {
        // Found single port
        pins.emplace_back(t);
    }
    else
    {
        // Found multi-port
        for (const auto& input_pin : input_pins)
        {
            if (core_utils::starts_with(input_pin, t))
            {
                pins.emplace_back(input_pin);
            }
        }
        if (pins.empty())
        {
            for (const auto& output_pin : output_pins)
            {
                if (core_utils::starts_with(output_pin, t))
                {
                    pins.emplace_back(output_pin);
                }
            }
        }
        if (pins.empty())
        {
            for (const auto& inout_pin : inout_pins)
            {
                if (core_utils::starts_with(inout_pin, t))
                {
                    pins.emplace_back(inout_pin);
                }
            }
        }
    }
    return pins;
}

bool hdl_parser_verilog_old::is_numeric(const std::string& token)
{
    if (token.empty())
    {
        return false;
    }
    else if (token.find('\'') != std::string::npos)
    {
        return true;
    }
    else if (std::find_if(token.begin(), token.end(), [](char c) { return !std::isdigit(c); }) == token.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::vector<std::string> hdl_parser_verilog_old::get_binary_string_from_number_literal(const std::string& v, const int line)
{
    std::string value = v;
    std::vector<std::string> binary_vector;
    int len = -1;
    if (value.find('\'') != std::string::npos && !core_utils::starts_with(value, "'"))
    {
        len = std::stoi(value.substr(0, value.find('\'')));
    }
    if (value.find("'h") != std::string::npos)
    {
        value = value.substr(value.find('\'') + 2);
        for (const auto& c : value)
        {
            switch (c)
            {
                case '0':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    break;
                case '1':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    break;
                case '2':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    break;
                case '3':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    break;
                case '4':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    break;
                case '5':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    break;
                case '6':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    break;
                case '7':
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    break;
                case '8':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    break;
                case '9':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    break;
                case 'a':
                case 'A':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    break;
                case 'b':
                case 'B':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    break;
                case 'c':
                case 'C':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b0");
                    break;
                case 'd':
                case 'D':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    binary_vector.push_back("1'b1");
                    break;
                case 'e':
                case 'E':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b0");
                    break;
                case 'f':
                case 'F':
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    binary_vector.push_back("1'b1");
                    break;
                case 'x':
                case 'X':
                    binary_vector.push_back("1'bx");
                    binary_vector.push_back("1'bx");
                    binary_vector.push_back("1'bx");
                    binary_vector.push_back("1'bx");
                    break;
                case 'z':
                case 'Z':
                    binary_vector.push_back("1'bz");
                    binary_vector.push_back("1'bz");
                    binary_vector.push_back("1'bz");
                    binary_vector.push_back("1'bz");
                    break;
                default:
                    log_error("hdl_parser", "Cannot parse binary value {} (token: {}) line: {}", c, v, line);
                    return std::vector<std::string>();
            }
            if (len > -1 && binary_vector.size() > (u32)len)
            {
                binary_vector.erase(binary_vector.begin(), binary_vector.begin() + binary_vector.size() - len);
            }
        }
    }
    else if (value.find("'b") != std::string::npos)
    {
        value = value.substr(value.find('\'') + 2);
        for (const auto& c : value)
        {
            switch (c)
            {
                case '1':
                    binary_vector.push_back("1'b1");
                    break;
                case '0':
                    binary_vector.push_back("1'b0");
                    break;
                case 'x':
                case 'X':
                    binary_vector.push_back("1'bx");
                    break;
                case 'z':
                case 'Z':
                    binary_vector.push_back("1'bz");
                    break;
                default:
                    log_error("hdl_parser", "Cannot parse binary value {} (token: {}) line: {}", c, v, line);
                    return std::vector<std::string>();
            }
        }
    }
    else if (value.find("'d") != std::string::npos || value.find('\'') == std::string::npos)
    {
        value              = value.substr(value.find('\'') + 2);
        unsigned int v_int = (unsigned int)std::stoul(value);
        if (len != -1)
        {
            for (int i = 0; i < len; ++i)
            {
                if (v_int & 1)
                {
                    binary_vector.push_back("1'b1");
                }
                else
                {
                    binary_vector.push_back("1'b0");
                }
                v_int >>= 1;
            }
        }
        else
        {
            while (v_int > 0)
            {
                if (v_int & 1)
                {
                    binary_vector.push_back("1'b1");
                }
                else
                {
                    binary_vector.push_back("1'b0");
                }
                v_int >>= 1;
            }
        }
        std::reverse(binary_vector.begin(), binary_vector.end());
    }
    else
    {
        log_error("hdl_parser", "Cannot parse numeric literal {} on line {}.", value, line);
    }
    return binary_vector;
}
