#include "hdl_parser/hdl_parser_verilog.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

#include <iomanip>
#include <iostream>

#include <queue>

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

    // tokenize file
    if (!tokenize())
    {
        return nullptr;
    }

    // parse tokens into intermediate format
    m_last_parsed_line = 1;
    try
    {
        if (!parse_tokens())
        {
            log_error("hdl_parser", "could not parse tokens in line {}.", m_last_parsed_line);
            return nullptr;
        }
    }
    catch (std::out_of_range& e)
    {
        log_error("hdl_parser", "parsing exceeded the end of file near line {}.", m_last_parsed_line);
        return nullptr;
    }
    catch (std::invalid_argument& e)
    {
        log_error("hdl_parser", "{} near line {}.", e.what(), m_last_parsed_line);
        return nullptr;
    }

    if (m_entities.empty())
    {
        log_error("hdl_parser", "file did not contain any entities.");
        return nullptr;
    }

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

    // build the netlist from the intermediate format
    // the last entity in the file is considered the top module
    if (!build_netlist(m_last_entity))
    {
        return nullptr;
    }

    // add global gnd gate if required by any instance
    if (!m_zero_net->get_dsts().empty())
    {
        auto gnd_type   = m_netlist->get_gate_library()->get_gnd_gate_types().begin()->second;
        auto output_pin = gnd_type->get_output_pins().at(0);
        auto gnd        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");

        if (!m_netlist->mark_gnd_gate(gnd))
        {
            return nullptr;
        }

        auto gnd_net = m_net_by_name.find("'0'")->second;

        if (!gnd_net->set_src(gnd, output_pin))
        {
            return nullptr;
        }
    }
    else
    {
        m_netlist->delete_net(m_zero_net);
    }

    // add global vcc gate if required by any instance
    if (!m_one_net->get_dsts().empty())
    {
        auto vcc_type   = m_netlist->get_gate_library()->get_vcc_gate_types().begin()->second;
        auto output_pin = vcc_type->get_output_pins().at(0);
        auto vcc        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");

        if (!m_netlist->mark_vcc_gate(vcc))
        {
            return nullptr;
        }

        auto vcc_net = m_net_by_name.find("'1'")->second;

        if (!vcc_net->set_src(vcc, output_pin))
        {
            return nullptr;
        }
    }
    else
    {
        m_netlist->delete_net(m_one_net);
    }

    for (const auto& net : m_netlist->get_nets())
    {
        bool no_src = net->get_src().gate == nullptr && !net->is_global_input_net();
        bool no_dst = net->get_num_of_dsts() == 0 && !net->is_global_output_net();
        if (no_src && no_dst)
        {
            m_netlist->delete_net(net);
        }
    }

    return m_netlist;
}

bool hdl_parser_verilog::tokenize()
{
    std::string delimiters = ",()[]\\#: ;=.";
    std::string current_token;
    u32 line_number = 0;

    std::string line;
    bool escaped             = false;
    bool multi_line_comment  = false;
    bool multi_line_property = false;

    std::vector<token> parsed_tokens;
    while (std::getline(m_fs, line))
    {
        line_number++;
        this->remove_comments(line, multi_line_comment, multi_line_property);

        for (char c : core_utils::trim(line))
        {
            if (c == '\\')
            {
                escaped = true;
                continue;
            }
            else if (escaped && std::isspace(c))
            {
                escaped = false;
            }
            if (delimiters.find(c) == std::string::npos || escaped)
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

                if (c == '(' && parsed_tokens.back() == "#")
                {
                    parsed_tokens.back() = "#(";
                }
                else if (!std::isspace(c))
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
    std::string last_entity;

    while (m_token_stream.remaining() > 0)
    {
        m_last_parsed_line = m_token_stream.peek().number;

        if (m_token_stream.peek() == "module")
        {
            if (!parse_entity_definiton())
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in global scope in line {}: Expected 'module'.", m_token_stream.peek().string, m_token_stream.peek().number);
            return false;
        }
    }

    if (!connect_instances())
    {
        return false;
    }

    return true;
}

bool hdl_parser_verilog::parse_entity_definiton()
{
    entity e;
    e.line_number = m_token_stream.peek().number;
    m_token_stream.consume("module");
    e.name = m_token_stream.consume();

    if (m_token_stream.peek() == "#(")
    {
        // TODO generics
        m_token_stream.consume_until(")");
        m_token_stream.consume(")");
    }
    else if (m_token_stream.peek() == "(")
    {
        if (!parse_port_list(e))
        {
            return false;
        }

        m_token_stream.consume(";");
    }
    else
    {
        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ports or generics.", m_token_stream.peek().string, m_token_stream.peek().number);
        return false;
    }

    while (m_token_stream.peek() != "endmodule")
    {
        if (m_token_stream.peek() == "input" || m_token_stream.peek() == "output")
        {
            if (!parse_port_definition(e))
            {
                return false;
            }
        }
        else if (m_token_stream.peek() == "wire")
        {
            if (!parse_signal_definition(e))
            {
                return false;
            }
        }
        else if (m_token_stream.peek() == "assign")
        {
            if (!parse_assign(e))
            {
                return false;
            }
        }
        else
        {
            if (!parse_instance(e))
            {
                return false;
            }
        }
    }

    if (!m_token_stream.consume("endmodule"))
    {
        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected 'endmodule'.", m_token_stream.peek().string, m_token_stream.peek().number);
        return false;
    }

    if (!e.name.empty())
    {
        m_entities[e.name] = e;
        m_last_entity      = e.name;
    }

    return true;
}

bool hdl_parser_verilog::parse_port_list(entity& e)
{
    m_token_stream.consume("(");
    auto ports = m_token_stream.extract_until(")");

    // TODO support other port declaration style
    while (ports.remaining() > 0)
    {
        m_last_parsed_line = ports.peek().number;
        e.port_names.insert(ports.consume());
    }

    m_token_stream.consume(")");

    return true;
}

bool hdl_parser_verilog::parse_port_definition(entity& e)
{
    auto direction = m_token_stream.consume();
    auto port_str  = m_token_stream.extract_until(";");

    if (!m_token_stream.consume(";"))
    {
        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ';'.", m_token_stream.peek().string, m_token_stream.peek().number);
        return false;
    }

    for (const auto& expanded_port : get_expanded_signals(port_str))
    {
        if (e.port_names.find(expanded_port.first) == e.port_names.end())
        {
            log_error("hdl_parser", "port name '{}' in line {} has not been declared in entity port list.", expanded_port.first, port_str.peek().number);
            return false;
        }

        e.ports_expanded[expanded_port.first] = std::make_pair(direction.string, expanded_port.second);
        e.expanded_signal_names[expanded_port.first].insert(e.expanded_signal_names[expanded_port.first].end(), expanded_port.second.begin(), expanded_port.second.end());
    }

    return true;
}

bool hdl_parser_verilog::parse_signal_definition(entity& e)
{
    m_token_stream.consume("wire");
    auto signal_str = m_token_stream.extract_until(";");

    if (!m_token_stream.consume(";"))
    {
        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ';'.", m_token_stream.peek().string, m_token_stream.peek().number);
        return false;
    }

    for (const auto& expanded_signal : get_expanded_signals(signal_str))
    {
        e.signals_expanded.insert(e.signals_expanded.end(), expanded_signal.second.begin(), expanded_signal.second.end());
        e.expanded_signal_names[expanded_signal.first].insert(e.expanded_signal_names[expanded_signal.first].end(), expanded_signal.second.begin(), expanded_signal.second.end());
    }

    return true;
}

bool hdl_parser_verilog::parse_assign(entity& e)
{
    std::map<std::string, std::string> direct_assignment;

    auto assign_line = m_token_stream.peek().number;

    m_token_stream.consume("assign");
    auto left_str = m_token_stream.extract_until("=");
    m_token_stream.consume("=");
    auto right_str = m_token_stream.extract_until(";");
    m_token_stream.consume(";");

    auto left_parts  = get_assignment_signals(left_str, e);
    auto right_parts = get_assignment_signals(right_str, e);

    if (left_parts.size() != right_parts.size())
    {
        log_error("hdl_parser", "cannot parse direct assignment in line {} due to width mismatch.", assign_line);
        return false;
    }

    for (u32 i = 0; i < right_parts.size(); i++)
    {
        direct_assignment[left_parts[i]] = right_parts[i];
    }

    if (direct_assignment.empty())
    {
        return false;
    }

    for (const auto& a : direct_assignment)
    {
        e.direct_assignments.emplace(a);
    }

    return true;
}

bool hdl_parser_verilog::parse_instance(entity& e)
{
    instance inst;
    inst.type = m_token_stream.consume();

    if (m_token_stream.consume("#("))
    {
        auto generic_str = m_token_stream.extract_until(")");

        while (generic_str.remaining() > 0)
        {
            if (!generic_str.consume("."))
            {
                log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected '.'.", generic_str.peek().string, generic_str.peek().number);
                return false;
            }

            auto generic_lhs = generic_str.extract_until("(");

            generic_str.consume("(");

            auto generic_rhs = generic_str.extract_until(")");

            generic_str.consume(")");

            inst.generic_streams.emplace_back(generic_lhs, generic_rhs);

            generic_str.consume(",");
        }

        if (!m_token_stream.consume(")"))
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ')'.", m_token_stream.peek().string, m_token_stream.peek().number);
            return false;
        }
    }

    inst.name = m_token_stream.consume();

    if (!m_token_stream.consume("("))
    {
        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected '('.", m_token_stream.peek().string, m_token_stream.peek().number);
        return false;
    }

    auto port_str = m_token_stream.extract_until(")");

    while (port_str.remaining() > 0)
    {
        if (!port_str.consume("."))
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected '.'.", port_str.peek().string, port_str.peek().number);
            return false;
        }

        auto generic_lhs = port_str.extract_until("(");

        port_str.consume("(");

        auto generic_rhs = port_str.extract_until(")");

        port_str.consume(")");

        inst.port_streams.emplace_back(generic_lhs, generic_rhs);

        port_str.consume(",");
    }

    if (!m_token_stream.consume(")"))
    {
        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ')'.", m_token_stream.peek().string, m_token_stream.peek().number);
        return false;
    }

    if (!m_token_stream.consume(";"))
    {
        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ';'.", m_token_stream.peek().string, m_token_stream.peek().number);
        return false;
    }

    e.instances.push_back(inst);

    return true;
}

bool hdl_parser_verilog::connect_instances()
{
    for (auto& [name, e] : m_entities)
    {
        UNUSED(name);

        for (auto& inst : e.instances)
        {
            for (auto& generic : inst.generic_streams)
            {
                inst.generics.emplace_back(generic.first.consume().string, generic.second.consume().string);
            }

            for (auto& port : inst.port_streams)
            {
                std::map<std::string, std::string> port_assignments;

                auto port_line = port.first.peek().number;

                auto port_lhs = get_port_signals(port.first, inst.type);
                auto port_rhs = get_assignment_signals(port.second, e);

                if (port_lhs.size() != port_rhs.size())
                {
                    log_error("hdl_parser", "cannot parse port assignment in line '{}' due to width mismatch.", port_line);
                    return {};
                }

                for (u32 i = 0; i < port_rhs.size(); i++)
                {
                    port_assignments[port_lhs[i]] = port_rhs[i];
                }

                if (port_assignments.empty() == true)
                {
                    return false;
                }

                for (const auto& a : port_assignments)
                {
                    inst.ports.push_back(a);
                }
            }
        }
    }

    return true;
}

// ###########################################################################
// #######          Build the netlist from intermediate format          ######
// ###########################################################################

bool hdl_parser_verilog::build_netlist(const std::string& top_module)
{
    m_netlist->set_design_name(top_module);

    auto& top_entity = m_entities[top_module];

    // count the occurences of all names
    // names that occur multiple times will get a unique alias during parsing

    std::queue<entity*> q;
    q.push(&top_entity);

    for (const auto& expanded_port : top_entity.ports_expanded)
    {
        for (const auto& expanded_port_name : expanded_port.second.second)
        {
            m_name_occurrences[expanded_port_name]++;
        }
    }

    while (!q.empty())
    {
        auto e = q.front();
        q.pop();

        m_name_occurrences[e->name]++;

        for (const auto& x : e->signals_expanded)
        {
            m_name_occurrences[x]++;
        }

        for (const auto& x : e->instances)
        {
            m_name_occurrences[x.name]++;
            auto it = m_entities.find(x.type);
            if (it != m_entities.end())
            {
                q.push(&(it->second));
            }
        }
    }

    for (auto& [name, e] : m_entities)
    {
        UNUSED(e);
        if (m_name_occurrences[name] == 0)
        {
            log_warning("hdl_parser", "entity '{}' is defined but not used", name);
        }
    }

    // for the top module, generate global i/o signals for all ports

    std::map<std::string, std::function<bool(std::shared_ptr<net> const)>> port_dir_function = {{"input", [](std::shared_ptr<net> const net) { return net->mark_global_input_net(); }},
                                                                                                {"output", [](std::shared_ptr<net> const net) { return net->mark_global_output_net(); }}};

    std::map<std::string, std::string> top_assignments;

    for (const auto& expanded_port : top_entity.ports_expanded)
    {
        auto direction = expanded_port.second.first;

        for (const auto& expanded_port_name : expanded_port.second.second)
        {
            if (port_dir_function.find(direction) == port_dir_function.end())
            {
                log_error("hdl_parser", "entity {}, line {}+ : direction '{}' unknown", expanded_port_name, top_entity.line_number, direction);
                return false;
            }

            auto new_net = m_netlist->create_net(expanded_port_name);
            if (new_net == nullptr)
            {
                return false;
            }
            m_net_by_name[new_net->get_name()] = new_net;
            if (!port_dir_function[direction](new_net))
            {
                return false;
            }

            // for instances, point the ports to the newly generated signals
            top_assignments[new_net->get_name()] = new_net->get_name();
        }
    }

    // now create all instances of the top entity
    // this will recursively instantiate all sub-entities
    if (instantiate(top_entity, nullptr, top_assignments) == nullptr)
    {
        return false;
    }

    // netlist is created.
    // now merge nets
    while (!m_nets_to_merge.empty())
    {
        // master = net that other nets are merged into
        // slave = net to merge into master and then delete

        bool progress_made = false;

        for (const auto& [master, merge_set] : m_nets_to_merge)
        {
            // check if none of the slaves is itself a master
            bool okay = true;

            for (const auto& slave : merge_set)
            {
                if (m_nets_to_merge.find(slave) != m_nets_to_merge.end())
                {
                    okay = false;
                    break;
                }
            }

            if (!okay)
            {
                continue;
            }

            auto master_net = m_net_by_name.at(master);

            for (const auto& slave : merge_set)
            {
                auto slave_net = m_net_by_name.at(slave);

                // merge source
                auto slave_src = slave_net->get_src();
                if (slave_src.gate != nullptr)
                {
                    slave_net->remove_src();
                    if (master_net->get_src().gate == nullptr)
                    {
                        master_net->set_src(slave_src);
                    }
                    else if (slave_src.gate != master_net->get_src().gate)
                    {
                        return false;
                    }
                }

                // merge destinations
                for (const auto& dst : slave_net->get_dsts())
                {
                    slave_net->remove_dst(dst);
                    if (!master_net->is_a_dst(dst))
                    {
                        master_net->add_dst(dst);
                    }
                }

                // merge attributes etc.
                for (const auto& it : slave_net->get_data())
                {
                    if (!master_net->set_data(std::get<0>(it.first), std::get<1>(it.first), std::get<0>(it.second), std::get<1>(it.second)))
                    {
                        log_error("hdl_parser", "couldn't set data");
                    }
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
            log_error("hdl_parser", "cyclic dependency between signals found, cannot parse netlist");
            return false;
        }
    }

    return true;
}

std::shared_ptr<module> hdl_parser_verilog::instantiate(const entity& e, std::shared_ptr<module> parent, std::map<std::string, std::string> parent_module_assignments)
{
    // remember assigned aliases so they are not lost when recursively going deeper
    std::map<std::string, std::string> aliases;

    aliases[e.name] = get_unique_alias(e.name);

    // select/create a module for the entity
    std::shared_ptr<module> module;

    if (parent == nullptr)
    {
        module = m_netlist->get_top_module();
        module->set_name(aliases[e.name]);
    }
    else
    {
        module = m_netlist->create_module(aliases[e.name], parent);
    }

    if (module == nullptr)
    {
        return nullptr;
    }

    // create all internal signals
    for (const auto& name : e.signals_expanded)
    {
        // create new net for the signal
        aliases[name] = get_unique_alias(name);
        auto new_net  = m_netlist->create_net(aliases[name]);
        if (new_net == nullptr)
        {
            return nullptr;
        }
        m_net_by_name[aliases[name]] = new_net;
    }

    std::set<std::string> output_ports;
    std::set<std::string> input_ports;

    for (const auto& expanded_port : e.ports_expanded)
    {
        auto direction = expanded_port.second.first;

        if (direction == "input")
        {
            std::copy(expanded_port.second.second.begin(), expanded_port.second.second.end(), std::inserter(input_ports, input_ports.end()));
        }
        if (direction == "output")
        {
            std::copy(expanded_port.second.second.begin(), expanded_port.second.second.end(), std::inserter(output_ports, output_ports.end()));
        }
    }

    for (const auto& [s, assignment] : e.direct_assignments)
    {
        std::string a = s;
        std::string b = assignment;

        if (parent_module_assignments.find(a) != parent_module_assignments.end())
        {
            a = parent_module_assignments.at(a);
        }
        else
        {
            a = aliases.at(a);
        }
        if (parent_module_assignments.find(b) != parent_module_assignments.end())
        {
            b = parent_module_assignments.at(b);
        }
        else
        {
            b = aliases.at(b);
        }
        m_nets_to_merge[b].push_back(a);
    }

    // cache global vcc/gnd types
    auto vcc_gate_types = m_netlist->get_gate_library()->get_vcc_gate_types();
    auto gnd_gate_types = m_netlist->get_gate_library()->get_gnd_gate_types();

    // process instances i.e. gates or other entities
    for (const auto& inst : e.instances)
    {
        // will later hold either module or gate, so attributes can be assigned properly
        data_container* container;

        // assign actual signal names to ports
        std::map<std::string, std::string> instance_assignments;
        for (const auto& [pin, s] : inst.ports)
        {
            auto it2 = parent_module_assignments.find(s);
            if (it2 != parent_module_assignments.end())
            {
                instance_assignments[pin] = it2->second;
            }
            else
            {
                auto it3 = aliases.find(s);
                if (it3 != aliases.end())
                {
                    instance_assignments[pin] = it3->second;
                }
                else if (s == "'0'" || s == "'1'")
                {
                    instance_assignments[pin] = s;
                }
                else
                {
                    log_error("hdl_parser", "signal assignment \"{} = {}\" of instance {} is invalid", pin, s, inst.name);
                    return nullptr;
                }
            }
        }

        // if the instance is another entity, recursively instantiate it
        auto entity_it = m_entities.find(inst.type);
        if (entity_it != m_entities.end())
        {
            container = instantiate(entity_it->second, module, instance_assignments).get();
            if (container == nullptr)
            {
                return nullptr;
            }
        }
        // otherwise it has to be an element from the gate library
        else
        {
            // create the new gate
            aliases[inst.name] = get_unique_alias(inst.name);

            std::shared_ptr<gate> new_gate;
            {
                auto gate_types = m_netlist->get_gate_library()->get_gate_types();
                auto it         = gate_types.find(inst.type);
                if (it == gate_types.end())
                {
                    return nullptr;
                }
                new_gate = m_netlist->create_gate(it->second, aliases[inst.name]);
            }

            if (new_gate == nullptr)
            {
                return nullptr;
            }

            module->assign_gate(new_gate);
            container = new_gate.get();

            // if gate is a global type, register it as such
            if (vcc_gate_types.find(inst.type) != vcc_gate_types.end() && !new_gate->mark_vcc_gate())
            {
                return nullptr;
            }
            if (gnd_gate_types.find(inst.type) != gnd_gate_types.end() && !new_gate->mark_gnd_gate())
            {
                return nullptr;
            }

            // cache pin types
            auto input_pin_types  = new_gate->get_input_pins();
            auto output_pin_types = new_gate->get_output_pins();

            // check for port
            for (auto [pin, net_name] : inst.ports)
            {
                // apply port assignments
                {
                    auto it = instance_assignments.find(pin);
                    if (it != instance_assignments.end())
                    {
                        net_name = it->second;
                    }
                }

                // if the net is an internal signal, use its alias
                if (std::find(e.signals_expanded.begin(), e.signals_expanded.end(), net_name) != e.signals_expanded.end())
                {
                    net_name = aliases.at(net_name);
                }

                // get the respective net for the assignment
                auto it = m_net_by_name.find(net_name);

                if (it == m_net_by_name.end())
                {
                    log_error("hdl_parser", "signal '{}' of {} was not previously declared", net_name, e.name);
                    return nullptr;
                }
                auto current_net = it->second;

                // add net src/dst by pin types
                bool is_input  = std::find(input_pin_types.begin(), input_pin_types.end(), pin) != input_pin_types.end();
                bool is_output = std::find(output_pin_types.begin(), output_pin_types.end(), pin) != output_pin_types.end();

                if (!is_input && !is_output)
                {
                    log_error("hdl_parser", "undefined pin '{}' for '{}' ({})", pin, new_gate->get_name(), new_gate->get_type()->get_name());
                    return nullptr;
                }

                if (is_output)
                {
                    if (current_net->get_src().gate != nullptr)
                    {
                        auto src = current_net->get_src().gate;
                        log_error("hdl_parser",
                                  "net '{}' already has source gate '{}' (type {}), cannot assign '{}' (type {})",
                                  current_net->get_name(),
                                  src->get_name(),
                                  src->get_type()->get_name(),
                                  new_gate->get_name(),
                                  new_gate->get_type()->get_name());
                    }
                    if (!current_net->set_src(new_gate, pin))
                    {
                        return nullptr;
                    }
                }

                if (is_input && !current_net->add_dst(new_gate, pin))
                {
                    return nullptr;
                }
            }
        }

        // process generics
        for (auto [name, value] : inst.generics)
        {
            auto bit_vector_candidate = core_utils::trim(core_utils::replace(value, "_", ""));

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
                value     = get_number_from_literal(value, 16);
                data_type = "bit_vector";
            }
            else
            {
                log_error("hdl_parser", "cannot identify data type of generic map value '{}' in instance '{}'", value, inst.name);
                return nullptr;
            }

            // store generic information on gate
            if (!container->set_data("generic", name, data_type, value))
            {
                log_error("hdl_parser", "couldn't set data", value, inst.name);
                return nullptr;
            }
        }
    }

    return module;
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

void hdl_parser_verilog::expand_signal(std::vector<std::string>& expanded_signal, std::string current_signal, std::vector<std::pair<i32, i32>> bounds, u32 dimension)
{
    // expand signal recursively
    if (bounds.size() > dimension)
    {
        if (bounds[dimension].first < bounds[dimension].second)
        {
            // left_bound < right_bound
            for (i32 i = bounds[dimension].first; i <= bounds[dimension].second; i++)
            {
                this->expand_signal(expanded_signal, current_signal + "(" + std::to_string(i) + ")", bounds, dimension + 1);
            }
        }
        else
        {
            // left_bound >= right_bound
            for (i32 i = bounds[dimension].first; i >= bounds[dimension].second; i--)
            {
                this->expand_signal(expanded_signal, current_signal + "(" + std::to_string(i) + ")", bounds, dimension + 1);
            }
        }
    }
    else
    {
        // last dimension
        expanded_signal.push_back(current_signal);
    }
}

std::map<std::string, std::vector<std::string>> hdl_parser_verilog::get_expanded_signals(token_stream& signal_str)
{
    std::map<std::string, std::vector<std::string>> result;
    std::vector<std::pair<std::string, std::vector<std::pair<i32, i32>>>> signals;

    std::vector<std::pair<i32, i32>> bounds;
    std::vector<std::string> names;

    // extract bounds
    while (signal_str.peek() == "[")
    {
        signal_str.consume("[");
        auto lower = signal_str.consume();

        if (!signal_str.consume(":"))
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ':'.", signal_str.peek().string, signal_str.peek().number);
            return {};
        }

        auto upper = signal_str.consume();

        if (!signal_str.consume("]"))
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ']'.", signal_str.peek().string, signal_str.peek().number);
            return {};
        }

        try
        {
            bounds.emplace_back(std::stoi(lower.string), std::stoi(upper.string));
        }
        catch (std::invalid_argument& e)
        {
            log_error("hdl_parser", "no integer bounds given in entity defintion in line {}: [{}:{}] .", lower.string, upper.string, signal_str.peek().number);
            return {};
        }
        catch (std::out_of_range& e)
        {
            log_error("hdl_parser", "bounds our of range in entity defintion in line {}: [{}:{}].", lower.string, upper.string, signal_str.peek().number);
            return {};
        }
    }

    // extract names
    names.emplace_back(signal_str.consume());
    while (signal_str.consume(","))
    {
        names.emplace_back(signal_str.consume());
    }

    for (const auto& name : names)
    {
        signals.emplace_back(name, bounds);
    }

    // expand signals
    for (const auto& s : signals)
    {
        std::vector<std::string> expanded_signal;

        this->expand_signal(expanded_signal, s.first, s.second, 0);

        result[s.first] = expanded_signal;
    }

    return result;
}

std::vector<std::string> hdl_parser_verilog::get_assignment_signals(token_stream& signal_str, entity& e)
{
    // PARSE ASSIGNMENT
    //   assignment can currently be one of the following:
    //   (1) NAME *single*
    //   (2) NAME *multi-dimensional*
    //   (3) NUMBER
    //   (4) NAME[INDEX1][INDEX2]...
    //   (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
    //   (6) {(1) - (5), (1) - (5), ...}

    std::vector<std::string> result;
    std::vector<token_stream> parts;

    // (6) {(1) - (5), (1) - (5), ...}
    if (signal_str.peek() == "{")
    {
        signal_str.consume("{");

        auto assignment_list = signal_str.extract_until("}");

        while (assignment_list.remaining() > 0)
        {
            parts.push_back(assignment_list.extract_until(","));
            assignment_list.consume(",");
        }

        if (!signal_str.consume("}"))
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected '{}'.", m_token_stream.peek().string, m_token_stream.peek().number, "}");
            return {};
        }
    }
    else
    {
        parts.push_back(signal_str);
    }

    for (auto& s : parts)
    {
        auto signal_name = s.consume().string;

        // (3) NUMBER
        if (isdigit(signal_name[0]) || signal_name[0] == '\'')
        {
            for (auto bit : get_number_from_literal(signal_name, 2))
            {
                result.push_back("'" + std::to_string(bit - 48) + "'");
            }

            continue;
        }

        // valid signal/port name?
        if (e.expanded_signal_names.find(signal_name) == e.expanded_signal_names.end())
        {
            log_error("hdl_parser", "no wire or port '{}' within entity '{}'.", signal_name, e.name);
            return {};
        }

        if (s.consume("["))
        {
            //(5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
            if (s.find_next(":", s.position() + 2) != token_stream::END_OF_STREAM)
            {
                std::vector<std::pair<i32, i32>> bounds;
                std::vector<std::string> expanded_signal;

                do
                {
                    i32 left_bound = std::stoi(s.consume());
                    s.consume(":");
                    i32 right_bound = std::stoi(s.consume());

                    bounds.emplace_back(left_bound, right_bound);

                    if (!s.consume("]"))
                    {
                        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ']'.", m_token_stream.peek().string, m_token_stream.peek().number);
                    }
                } while (s.consume("["));

                expand_signal(expanded_signal, signal_name, bounds, 0);
                result.insert(result.end(), expanded_signal.begin(), expanded_signal.end());
            }
            //(4) NAME[INDEX1][INDEX2]...
            else
            {
                do
                {
                    signal_name += "(" + s.consume().string + ")";

                    if (!s.consume("]"))
                    {
                        log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}: Expected ']'.", m_token_stream.peek().string, m_token_stream.peek().number);
                    }
                } while (s.consume("["));

                result.push_back(signal_name);
            }
        }
        else
        {
            //   (1) NAME *single*
            //   (2) NAME *multi-dimensional*
            result.insert(result.end(), e.expanded_signal_names[signal_name].begin(), e.expanded_signal_names[signal_name].end());
        }
    }

    return result;
}

std::vector<std::string> hdl_parser_verilog::get_port_signals(token_stream& port_str, const std::string& instance_type)
{
    std::vector<std::string> result;
    auto gate_types = m_netlist->get_gate_library()->get_gate_types();

    auto port_name = port_str.consume();

    if (m_entities.find(instance_type) != m_entities.end())
    {
        // is instance a valid entity within netlist?
        if (m_entities[instance_type].ports_expanded.find(port_name.string) != m_entities[instance_type].ports_expanded.end())
        {
            // is port valid for given entity
            result.insert(result.end(), m_entities[instance_type].ports_expanded[port_name.string].second.begin(), m_entities[instance_type].ports_expanded[port_name.string].second.end());
        }
        else
        {
            log_error("hdl_parser", "invalid port '{}' for entity '{}' in line {}.", port_name.string, instance_type, port_name.number);
            return {};
        }
    }
    else if (gate_types.find(instance_type) != gate_types.end())
    {
        auto gt = gate_types.at(instance_type);

        if (m_gate_to_pin_map.find(instance_type) == m_gate_to_pin_map.end())
        {
            auto ipins                       = gt->get_input_pins();
            auto opins                       = gt->get_output_pins();
            m_gate_to_pin_map[instance_type] = ipins;
            m_gate_to_pin_map[instance_type].insert(m_gate_to_pin_map[instance_type].end(), opins.begin(), opins.end());
        }

        if (std::find(m_gate_to_pin_map[instance_type].begin(), m_gate_to_pin_map[instance_type].end(), port_name.string) != m_gate_to_pin_map[instance_type].end())
        {
            result.push_back(port_name.string);
        }
        else
        {
            log_error("hdl_parser", "invalid port '{}' for gate '{}' in line {}.", port_name.string, instance_type, port_name.number);
            return {};
        }
    }
    else
    {
        log_error("hdl_parser", "invalid entity or port '{}' in line {}.", instance_type, port_name.number);
        return {};
    }

    return result;
}

std::string hdl_parser_verilog::get_number_from_literal(const std::string& v, const u32 target_base)
{
    std::string value = core_utils::to_lower(core_utils::trim(core_utils::replace(v, "_", "")));
    std::string res;

    u32 len = 0, source_base = 0;
    std::string length, prefix, number;

    // base specified?
    if (value.find('\'') == std::string::npos)
    {
        source_base = 10;
        number      = value;
    }
    else
    {
        length = value.substr(0, value.find('\''));
        prefix = value.substr(value.find('\'') + 1, 1);
        number = value.substr(value.find('\'') + 2);

        // select base
        if (prefix == "b")
        {
            source_base = 2;
        }
        else if (prefix == "o")
        {
            source_base = 8;
        }
        else if (prefix == "d")
        {
            source_base = 10;
        }
        else if (prefix == "h")
        {
            source_base = 16;
        }
    }

    if (target_base == 2)
    {
        // constructing bit string
        u64 val = stoull(number, 0, source_base);

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
    }
    else if (target_base == 16)
    {
        // constructing hex string
        std::stringstream ss;

        u64 val = stoull(number, 0, source_base);

        ss << std::hex << val;

        res = ss.str();
    }

    return res;
}

std::string hdl_parser_verilog::get_unique_alias(const std::string& name)
{
    // if the name only appears once, we don't have to alias it
    // m_name_occurrences holds the precomputed number of occurences for each name
    if (m_name_occurrences[name] < 2)
    {
        return name;
    }

    // otherwise, add a unique string to the name
    // use m_current_instance_index[name] to get a unique id

    m_current_instance_index[name]++;

    if (name.back() == '\\')
    {
        return name.substr(0, name.size() - 1) + "_module_inst" + std::to_string(m_current_instance_index[name]) + "\\";
    }

    return name + "_module_inst" + std::to_string(m_current_instance_index[name]);
}
