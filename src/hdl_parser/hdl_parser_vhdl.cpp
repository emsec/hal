
#include <hdl_parser/hdl_parser_vhdl.h>

#include "hdl_parser/hdl_parser_vhdl.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

hdl_parser_vhdl::hdl_parser_vhdl(std::stringstream& stream) : hdl_parser(stream)
{
}

std::shared_ptr<netlist> hdl_parser_vhdl::parse(const std::string& gate_library)
{
    m_netlist = netlist_factory::create_netlist(gate_library);
    if (m_netlist == nullptr)
    {
        log_error("hdl_parser", "netlist_factory returned nullptr");
        return nullptr;
    }

    // read the whole file into logical parts

    std::string buffer;
    std::vector<std::tuple<int, std::string>> header;
    std::vector<std::tuple<int, std::string>> entity;
    std::vector<std::tuple<int, std::string>> architecture;
    std::vector<std::tuple<int, std::string>> instances;

    // simple state machine to distinguish the parts
    enum State
    {
        INIT,
        HEADER,
        ENTITY,
        ARCHITECTURE,
        INSTANCE
    };

    State state = INIT;

    int line = 0;

    while (std::getline(m_fs, buffer))
    {
        line++;
        buffer = core_utils::trim(buffer);

        // skip empty lines
        if (buffer.empty())
        {
            continue;
        }

        // update state machine
        if (core_utils::starts_with(buffer, "--"))
        {
            if (state != HEADER)
            {
                header.clear();
            }
            state = HEADER;
        }
        else if (core_utils::starts_with(buffer, "entity "))
        {
            if (state != ENTITY)
            {
                if (!entity.empty())
                {
                    if (!add_entity_definition(entity))
                    {
                        return nullptr;
                    }
                }
                entity.clear();
            }
            state = ENTITY;
        }
        else if (core_utils::starts_with(buffer, "architecture "))
        {
            if (state != ARCHITECTURE)
            {
                architecture.clear();
            }
            state = ARCHITECTURE;
            continue;
        }
        else if (buffer == "begin")
        {
            if (state != INSTANCE)
            {
                instances.clear();
            }
            state = INSTANCE;
            continue;
        }

        // insert line depending on current state
        if (state == HEADER)
        {
            header.push_back(std::make_tuple(line, buffer));
        }
        else if (state == ENTITY)
        {
            entity.push_back(std::make_tuple(line, buffer));
        }
        else if (state == ARCHITECTURE)
        {
            architecture.push_back(std::make_tuple(line, buffer));
        }
        else if (state == INSTANCE)
        {
            instances.push_back(std::make_tuple(line, buffer));
        }
    }

    // parse logical parts

    if (!this->parse_header(header))
    {
        return nullptr;
    }

    if (!this->parse_entity(entity))
    {
        return nullptr;
    }

    if (!this->parse_architecture(architecture))
    {
        return nullptr;
    }

    if (!this->parse_instances(instances))
    {
        return nullptr;
    }

    // add global gnd gate if required by any instance
    if (m_net.find("'0'") != m_net.end())
    {
        auto gnd_type = *(m_netlist->get_gate_library()->get_global_gnd_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pin_types(gnd_type).at(0);
        auto gnd = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");
        if (!m_netlist->mark_global_gnd_gate(gnd))
        {
            return nullptr;
        }
        auto gnd_net = m_net.find("'0'")->second;
        if (!gnd_net->set_src(gnd, output_pin))
        {
            return nullptr;
        }
    }

    // add global vcc gate if required by any instance
    if (m_net.find("'1'") != m_net.end())
    {
        auto vcc_type = *(m_netlist->get_gate_library()->get_global_vcc_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pin_types(vcc_type).at(0);
        auto vcc = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");
        if (!m_netlist->mark_global_vcc_gate(vcc))
        {
            return nullptr;
        }
        auto vcc_net = m_net.find("'1'")->second;
        if (!vcc_net->set_src(vcc, output_pin))
        {
            return nullptr;
        }
    }

    return m_netlist;
}

bool hdl_parser_vhdl::parse_header(const std::vector<std::tuple<int, std::string>>& header)
{
    // header contains device name and library includes
    for (const auto& entry : header)
    {
        //auto line_number = std::get<0>(entry);
        auto line = std::get<1>(entry);

        // read out device comment
        if (core_utils::starts_with(line, "-- Device"))
        {
            auto device_name = core_utils::trim(line.substr(line.find(':') + 1));
            m_netlist->set_device_name(device_name);
        }
        // extract used libraries
        else if (core_utils::starts_with(line, "use "))
        {
            // remove the .all in the end
            auto lib = core_utils::to_lower(line.substr(line.find(' ')));
            lib      = core_utils::trim(lib.substr(0, lib.find("all;")));
            m_libraries.insert(lib);
        }
    }
    log_debug("hdl_parser", "parsed header.");
    return true;
}

bool hdl_parser_vhdl::add_entity_definition(const std::vector<std::tuple<int, std::string>>& entity)
{
    auto lib                = m_netlist->get_gate_library();
    std::string entity_name = "null";

    bool in_port_def = false;

    for (const auto& entry : entity)
    {
        //auto line_number = std::get<0>(entry);
        auto line = std::get<1>(entry);

        // line starts with "entity" -> extract name
        if (core_utils::starts_with(line, "entity "))
        {
            entity_name = core_utils::trim(line.substr(line.find(' ') + 1));
            entity_name = entity_name.substr(0, entity_name.find(' '));
            lib->get_gate_types()->insert(entity_name);
            m_libraries.insert("work.");
            log_debug("hdl_parser", "added external entity '{}'.", entity_name);
        }
        else if (core_utils::starts_with(line, "port ("))
        {
            in_port_def = true;
        }
        else if (in_port_def && line == ");")
        {
            in_port_def = false;
        }
        else if (in_port_def)
        {
            // get port information
            auto token     = core_utils::split(line, ':');
            auto name      = core_utils::trim(token[0]);
            auto config    = core_utils::trim(token[1]);
            auto direction = config.substr(0, config.find(' '));
            auto type      = config.substr(config.find(' ') + 1);

            // add all signals of that port
            for (const auto signal : get_vector_signals(name, type))
            {
                if (direction == "in")
                {
                    lib->get_input_pin_types()->insert(signal);
                    (*lib->get_gate_type_map_to_input_pin_types())[entity_name].push_back(signal);
                }
                else if (direction == "out")
                {
                    lib->get_output_pin_types()->insert(signal);
                    (*lib->get_gate_type_map_to_output_pin_types())[entity_name].push_back(signal);
                }
                else if (direction == "inout")
                {
                    lib->get_inout_pin_types()->insert(signal);
                    (*lib->get_gate_type_map_to_inout_pin_types())[entity_name].push_back(signal);
                }
                else
                {
                    return false;
                }
            }
        }
    }
    //log_debug("hdl_parser", "added new entity.");
    return true;
}

bool hdl_parser_vhdl::parse_entity(const std::vector<std::tuple<int, std::string>>& entity)
{
    // entity contains global port definitions

    std::map<std::string, std::function<int(std::shared_ptr<netlist> const, std::shared_ptr<net> const)>> port_dir_function = {
        {"in", [](std::shared_ptr<netlist> const g, std::shared_ptr<net> const net) { return g->mark_global_input_net(net); }},
        {"out", [](std::shared_ptr<netlist> const g, std::shared_ptr<net> const net) { return g->mark_global_output_net(net); }},
        {"inout", [](std::shared_ptr<netlist> const g, std::shared_ptr<net> const net) { return g->mark_global_inout_net(net); }},
    };

    // remember logical position
    bool in_port_def = false;

    for (const auto& entry : entity)
    {
        auto line_number = std::get<0>(entry);
        auto line        = std::get<1>(entry);

        // line starts with "entity" -> extract name
        if (core_utils::starts_with(line, "entity "))
        {
            auto design_name = core_utils::trim(line.substr(line.find(' ') + 1));
            design_name      = design_name.substr(0, design_name.find(' '));
            m_netlist->set_design_name(design_name);
            log_debug("hdl_parser", "parsing entity '{}'.", design_name);
        }
        // line starts with "port (" -> next lines contain ports
        else if (line == "port (")
        {
            in_port_def = true;
        }
        // line is ");" -> no more ports
        else if (in_port_def && line == ");")
        {
            in_port_def = false;
        }
        // we expect ports at this point
        else if (in_port_def)
        {
            // get port information
            auto token     = core_utils::split(line, ':');
            auto name      = core_utils::trim(token[0]);
            auto config    = core_utils::trim(token[1]);
            auto direction = config.substr(0, config.find(' '));
            auto type      = config.substr(config.find(' ') + 1);

            // check whether the direction is known
            if (port_dir_function.find(direction) == port_dir_function.end())
            {
                log_error("hdl_parser", "line {}: direction '{}' unkown", line_number, direction);
                return false;
            }

            // add all signals of that port
            for (const auto signal : get_vector_signals(name, type))
            {
                auto new_net               = m_netlist->create_net(m_netlist->get_unique_net_id(), signal);
                m_net[new_net->get_name()] = new_net;

                if (new_net == nullptr || port_dir_function[direction](m_netlist, new_net) != true)
                {
                    return false;
                }
            }
        }
    }
    log_debug("hdl_parser", "parsed entity.");
    return true;
}

bool hdl_parser_vhdl::parse_architecture(const std::vector<std::tuple<int, std::string>>& architecture)
{
    // architecture contains all signals and attributes

    for (const auto& entry : architecture)
    {
        auto line_number = std::get<0>(entry);
        auto line        = std::get<1>(entry);
        if (core_utils::starts_with(line, "signal "))
        {
            // get signal information
            auto items = core_utils::split(line, ':');
            auto name  = core_utils::trim(items[0].substr(items[0].find(' ') + 1));
            auto type  = core_utils::trim(items[1]);

            // add all (sub-)signals
            for (const auto signal : get_vector_signals(name, type))
            {
                auto new_net               = m_netlist->create_net(m_netlist->get_unique_net_id(), signal);
                m_net[new_net->get_name()] = new_net;
                if (new_net == nullptr)
                {
                    return false;
                }
            }
        }
        else if (core_utils::starts_with(line, "attribute "))
        {
            // attributes are simply ignored for now
            log_debug("hdl_parser", "ignoring '{}'", line);
        }
        else
        {
            log_error("hdl_parser", "line {}: architecture is not expected to contain '{}'", line_number, line);
            return false;
        }
    }
    log_debug("hdl_parser", "parsed architecture.");
    return true;
}

bool hdl_parser_vhdl::parse_instances(const std::vector<std::tuple<int, std::string>>& instances)
{
    std::string instance;
    int start_line_number = -1;

    for (const auto& entry : instances)
    {
        auto line = std::get<1>(entry);
        if (instance.empty())
        {
            start_line_number = std::get<0>(entry);
            if (core_utils::starts_with(line, "end "))
            {
                log_debug("hdl_parser", "parsed instances.");
                return true;
            }
        }

        instance += line + " ";

        if (line.back() == ';')
        {
            if (!parse_instance(instance))
            {
                log_error("hdl_parser", "line {}: cannot parse instance", start_line_number);
                return false;
            }
            instance.clear();
        }
    }

    log_debug("hdl_parser", "reached EOF without 'end NAME;'.");
    return false;
}

bool hdl_parser_vhdl::parse_instance(std::string instance)
{
    // instances contain all components with port assignments
    auto global_vcc_gate_types = m_netlist->get_gate_library()->get_global_vcc_gate_types();
    auto global_gnd_gate_types = m_netlist->get_gate_library()->get_global_gnd_gate_types();

    // extract name and component
    auto pos  = instance.find(':');
    auto name = core_utils::trim(instance.substr(0, pos));
    auto type = core_utils::trim(instance.substr(pos + 1));
    pos       = type.find(" generic map");
    if (pos == std::string::npos)
    {
        pos = type.find(" port map");
    }
    instance = core_utils::trim(type.substr(pos));
    type     = core_utils::trim(type.substr(0, pos));
    if (core_utils::starts_with(type, "entity"))
    {
        type = core_utils::trim(type.substr(type.find(' ')));
    }

    // remove library prefix from component
    auto low_type      = core_utils::to_lower(type);
    std::string prefix = "";
    for (const auto& lib : m_libraries)
    {
        if (prefix.size() < lib.size() && core_utils::starts_with(low_type, lib))
        {
            prefix = lib;
        }
    }
    if (!prefix.empty())
    {
        type = type.substr(prefix.size());
    }

    // insert a std::make_shared<gate> for the component
    auto new_gate = m_netlist->create_gate(m_netlist->get_unique_gate_id(), type, name);
    if (new_gate == nullptr)
    {
        return false;
    }

    // if gate id a global type, register it as such
    if (global_vcc_gate_types->find(type) != global_vcc_gate_types->end())
    {
        if (!m_netlist->mark_global_vcc_gate(new_gate))
        {
            return false;
        }
    }
    if (global_gnd_gate_types->find(type) != global_gnd_gate_types->end())
    {
        if (!m_netlist->mark_global_gnd_gate(new_gate))
        {
            return false;
        }
    }

    auto input_pin_types  = new_gate->get_input_pin_types();
    auto output_pin_types = new_gate->get_output_pin_types();
    auto inout_pin_types  = new_gate->get_inout_pin_types();

    // check for generic
    if (core_utils::starts_with(instance, "generic map"))
    {
        std::string generics = instance.substr(0, instance.find("port map"));
        instance             = core_utils::trim(instance.substr(generics.size()));
        generics             = generics.substr(generics.find('(') + 1);
        generics             = core_utils::trim(generics.substr(0, generics.find_last_of(')')));

        for (const auto& line : core_utils::split(generics, ','))
        {
            // get generic information
            auto key                  = core_utils::trim(line.substr(0, line.find("=>")));
            auto value                = core_utils::trim(line.substr(line.find("=>") + 2));
            auto value_stripped       = core_utils::trim(value.substr(1, value.length() - 1));
            auto bit_vector_candidate = core_utils::trim(core_utils::replace(value_stripped, "_", ""));

            // determine data type
            auto data_type = std::string();
            if ((core_utils::to_lower(value) == "true") || (core_utils::to_lower(value) == "false"))
            {
                data_type = "boolean";
            }
            else if (core_utils::is_integer(value))
            {
                data_type = "integer";
            }
            else if (core_utils::is_floating_point(value))
            {
                data_type = "floating_point";
            }
            else if (core_utils::ends_with(value, "s") || core_utils::ends_with(value, "sec") || core_utils::ends_with(value, "min") || core_utils::ends_with(value, "hr"))
            {
                data_type = "time";
            }
            else if ((value[0] == '\"' && value.back() == '\"') && !std::all_of(bit_vector_candidate.begin(), bit_vector_candidate.end(), ::isxdigit))
            {
                value     = value.substr(1, value.size() - 2);
                data_type = "string";
            }
            else if (value[0] == '\'' && value.back() == '\'')
            {
                value     = value.substr(1, value.size() - 2);
                data_type = "bit_value";
            }
            else if ((value[0] == 'D' || value[0] == 'X' || value[0] == 'B' || value[0] == 'O') && value[1] == '\"' && value.back() == '\"')
            {
                value     = get_hex_from_number_literal(value);
                data_type = "bit_vector";
            }
            else
            {
                log_error("hdl_parser", "cannot identify data type of generic map value '{}' in instance '{}'", value, new_gate->get_name());
                return false;
            }

            // store generic information on gate
            if (!new_gate->set_data("generic", key, data_type, value))
            {
                return false;
            }
        }
    }

    // check for port
    std::string ports = instance.substr(instance.find('(') + 1);
    ports             = core_utils::trim(ports.substr(0, ports.find_last_of(')'))) + ",";

    while (!ports.empty())
    {
        // locate next port
        // ports are separated by comma, commas within () have to be ignored
        size_t comma_index = ports.find(',');
        size_t open_index  = ports.find('(');
        size_t close_index = ports.find(')');
        if (comma_index > open_index && comma_index < close_index && open_index != std::string::npos)
        {
            comma_index = ports.find(',', close_index);
        }

        std::string port = ports.substr(0, comma_index);
        ports            = ports.substr(comma_index + 1);

        // split assignments of the current port (in case of e.g. "a(3 to 6) => b(7 to 10)")
        for (const auto& assignment : get_port_assignments(port))
        {
            // get pin and net information
            auto pin      = core_utils::trim(assignment.substr(0, assignment.find("=>")));
            auto net_name = core_utils::trim(assignment.substr(assignment.find("=>") + 2));
            if (net_name.back() == ',')
            {
                net_name.pop_back();
            }

            std::shared_ptr<net> current_net = nullptr;

            // search for existing net by name, otherwise add it
            auto it = m_net.find(net_name);
            if (it == m_net.end())
            {
                current_net     = m_netlist->create_net(m_netlist->get_unique_net_id(), net_name);
                m_net[net_name] = current_net;
                if (current_net == nullptr)
                {
                    return false;
                }

                if (net_name != "'0'" && net_name != "'1'")
                {
                    log_warning("hdl_parser", "signal '{}' was not previously declared in architecture", net_name);
                }
            }
            else
            {
                current_net = it->second;
            }

            // add net src/dst by pin types

            if ((std::find(input_pin_types.begin(), input_pin_types.end(), pin) == input_pin_types.end())
                && (std::find(output_pin_types.begin(), output_pin_types.end(), pin) == output_pin_types.end())
                && (std::find(inout_pin_types.begin(), inout_pin_types.end(), pin) == inout_pin_types.end()))
            {
                log_error("hdl_parser", "undefined pin '{}' for '{}' ({})", pin, new_gate->get_name(), new_gate->get_type());
                return false;
            }

            if ((std::find(input_pin_types.begin(), input_pin_types.end(), pin) != input_pin_types.end()) || (std::find(inout_pin_types.begin(), inout_pin_types.end(), pin) != inout_pin_types.end()))
            {
                if (!current_net->add_dst(new_gate, pin))
                {
                    return false;
                }
            }
            if ((std::find(output_pin_types.begin(), output_pin_types.end(), pin) != output_pin_types.end())
                || (std::find(inout_pin_types.begin(), inout_pin_types.end(), pin) != inout_pin_types.end()))
            {
                if (!current_net->set_src(new_gate, pin))
                {
                    return false;
                }
            }
        }
    }

    log_debug("hdl_parser", "parsed instance.");
    return true;
}

std::string hdl_parser_vhdl::get_hex_from_number_literal(const std::string& v)
{
    std::string value = core_utils::trim(core_utils::replace(v, "_", ""));
    if (core_utils::starts_with(value, "X\""))
    {
        value = value.substr(2);
        value = value.substr(0, value.length() - 1);
        value = core_utils::to_lower(value);
        return value;
    }
    // Conversion required
    int radix = 10;
    if (core_utils::starts_with(value, "D\""))
    {
        radix = 10;
        value = value.substr(2);
    }
    if (core_utils::starts_with(value, "B\""))
    {
        radix = 2;
        value = value.substr(2);
    }
    if (core_utils::starts_with(value, "O\""))
    {
        radix = 8;
        value = value.substr(2);
    }
    value = value.substr(0, value.length() - 1);
    std::stringstream ss;
    ss << std::hex << stoull(value, 0, radix);
    value = ss.str();
    return value;
}

std::vector<std::string> hdl_parser_vhdl::get_vector_signals(const std::string& name, const std::string& type)
{
    // if there is no range given, the signal is just the name
    if (type.find('(') == std::string::npos)
    {
        return {name};
    }

    // extract bounds "a to/downto b, c to/downto d, ..."
    auto bounds = type.substr(type.find("(") + 1);
    bounds      = core_utils::trim(bounds.substr(0, bounds.find(")")));

    // process bounds into tuples (low, high)
    std::vector<std::tuple<int, int>> bound_tokens;
    for (const auto& part : core_utils::split(bounds, ','))
    {
        std::string bound = core_utils::trim(part);
        bool downto       = true;
        if (bound.find(" downto ") == std::string::npos)
        {
            downto = false;
        }

        int left_bound  = std::stoi(bound.substr(0, bound.find(' ')));
        int right_bound = std::stoi(bound.substr(bound.find_last_of(' ') + 1));

        if (downto)
        {
            bound_tokens.push_back(std::make_tuple(right_bound, left_bound));
        }
        else
        {
            bound_tokens.push_back(std::make_tuple(left_bound, right_bound));
        }
    }

    // find the best matching supported vector type
    std::map<std::string, u32> std_logic_vector_identifier_to_dimension = {
        {"STD_LOGIC_VECTOR", 1},
        {"STD_LOGIC_VECTOR2", 2},
        {"STD_LOGIC_VECTOR3", 3},
    };
    u32 dimension = 0;
    for (const auto& it : std_logic_vector_identifier_to_dimension)
    {
        if ((type.find(it.first) != std::string::npos) && (dimension < it.second))
        {
            dimension = it.second;
        }
    }

    // compare expected dimension with actually found dimension
    if (dimension != (u32)bound_tokens.size())
    {
        log_error("hdl_parser", "dimension-bound mismatch for {} : {}", name, type);
        return {};
    }

    // depending on the dimension, build the signal string(s)
    std::vector<std::string> result;
    if (dimension == 1)
    {
        for (auto x = std::get<0>(bound_tokens[0]); x <= std::get<1>(bound_tokens[0]); x++)
        {
            result.push_back(name + "(" + std::to_string(x) + ")");
        }
    }
    else if (dimension == 2)
    {
        for (auto x = std::get<0>(bound_tokens[0]); x <= std::get<1>(bound_tokens[0]); x++)
        {
            for (auto y = std::get<0>(bound_tokens[1]); y <= std::get<1>(bound_tokens[1]); y++)
            {
                result.push_back(name + "(" + std::to_string(x) + ", " + std::to_string(y) + ")");
            }
        }
    }
    else if (dimension == 3)
    {
        for (auto x = std::get<0>(bound_tokens[0]); x <= std::get<1>(bound_tokens[0]); x++)
        {
            for (auto y = std::get<0>(bound_tokens[1]); y <= std::get<1>(bound_tokens[1]); y++)
            {
                for (auto z = std::get<0>(bound_tokens[2]); z <= std::get<1>(bound_tokens[2]); z++)
                {
                    result.push_back(name + "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
                }
            }
        }
    }
    else
    {
        log_error("hdl_parser", "not implemented reached for dimension '{}' (did you forget to add the case here?)", dimension);
        return {};
    }

    return result;
}

std::vector<std::string> hdl_parser_vhdl::get_port_assignments(const std::string& line)
{
    // a port may be
    // (1): a => ...
    // (2): a(x) => ...
    //      a(x, y, z) => ...
    // (3): a(x to y) => B"01010101..."
    // (4): a(x to y) => b(s to t)

    // case (1)
    size_t arrow_pos = line.find("=>");

    size_t open_pos = line.find('(');
    if (open_pos == std::string::npos || open_pos > arrow_pos)
    {
        return {line};
    }

    // extract left bounds and the part of the line which comes before the left bounds
    auto left_part   = line.substr(0, open_pos);
    auto left_bounds = line.substr(open_pos + 1);
    left_bounds      = core_utils::trim(left_bounds.substr(0, left_bounds.find(')')));

    // case (2)
    if (left_bounds.find("to") == std::string::npos)
    {
        return {line};
    }

    // extract the part of the line which comes after the left bounds
    auto right_part = core_utils::trim(line.substr(arrow_pos));

    // check for case (3) or (4)
    if (right_part.find('(') == std::string::npos)
    {
        // case (3)
        // right part has to be a bitvector
        if (line.find(" B\"") == std::string::npos)
        {
            log_error("hdl_parser", "assignment of anything but a binary bitvector is not supported ('{}')", line);
            return {};
        }

        // extract right values
        auto right_values = right_part.substr(right_part.find('\"') + 1);
        right_values      = right_values.substr(0, right_values.find('\"'));

        // extract counting direction and min/max of left bounds
        int left_dir = -1;
        if (left_bounds.find(" downto ") == std::string::npos)
        {
            left_dir = 1;
        }
        int left_start = std::stoi(left_bounds.substr(0, left_bounds.find(' ')));
        int left_end   = std::stoi(left_bounds.substr(left_bounds.find_last_of(' ') + 1));

        // assemble assignment strings
        std::vector<std::string> result;
        int l   = left_start;
        int cnt = 0;
        while (true)
        {
            std::string new_line = left_part + "(" + std::to_string(l) + ") => '" + right_values[cnt] + "'";
            //printf("  -> %s\n", new_line.c_str());
            result.push_back(new_line);
            if (l == left_end)
            {
                break;
            }
            cnt++;
            l += left_dir;
        }

        return result;
    }
    else
    {
        // case (4)
        // right part is a range just like left part
        auto right_bounds = right_part.substr(right_part.find('(') + 1);
        right_bounds      = core_utils::trim(right_bounds.substr(0, right_bounds.find(')')));

        right_part = right_part.substr(0, right_part.find_last_of('('));

        // extract counting direction and min/max of left and right bounds
        int left_dir = -1;
        if (left_bounds.find(" downto ") == std::string::npos)
        {
            left_dir = 1;
        }

        int right_dir = -1;
        if (right_bounds.find(" downto ") == std::string::npos)
        {
            right_dir = 1;
        }

        int left_start = std::stoi(left_bounds.substr(0, left_bounds.find(' ')));
        int left_end   = std::stoi(left_bounds.substr(left_bounds.find_last_of(' ') + 1));
        int dist       = left_dir * (left_end - left_start);

        int right_start = std::stoi(right_bounds.substr(0, right_bounds.find(' ')));
        int right_end   = std::stoi(right_bounds.substr(right_bounds.find_last_of(' ') + 1));
        int dist2       = right_dir * (right_end - right_start);

        // check that both ranges match
        if (dist != dist2)
        {
            log_error("hdl_parser", "ranges on port assignment '{}' do not match.", line);
            return {};
        }

        // assemble assignment strings
        std::vector<std::string> result;
        int l = left_start;
        int r = right_start;
        while (dist >= 0)
        {
            std::string new_line = left_part + "(" + std::to_string(l) + ")" + right_part + "(" + std::to_string(r) + ")";
            result.push_back(new_line);
            l += left_dir;
            r += right_dir;
            dist--;
        }

        return result;
    }
}
