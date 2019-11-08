#include "hdl_parser/hdl_parser_vhdl.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

#include <iostream>    //TODO remove
#include <queue>

hdl_parser_vhdl::hdl_parser_vhdl(std::stringstream& stream) : hdl_parser(stream)
{
}

// ###########################################################################
// ###########          Parse HDL into intermediate format          ##########
// ###########################################################################

std::shared_ptr<netlist> hdl_parser_vhdl::parse(const std::string& gate_library)
{
    m_netlist = netlist_factory::create_netlist(gate_library);
    if (m_netlist == nullptr)
    {
        log_error("hdl_parser", "netlist_factory returned nullptr");
        return nullptr;
    }

    std::string delimiters = ",(): ;=><";
    std::string token;
    u32 line_number = 0;

    std::string line;
    bool in_string = false;
    while (std::getline(m_fs, line))
    {
        line_number++;
        if (line.find("--") != std::string::npos)
        {
            line = line.substr(0, line.find("--"));
        }
        for (char c : core_utils::trim(line))
        {
            if (c == '"')
            {
                in_string = !in_string;
            }
            if (delimiters.find(c) == std::string::npos || in_string)
            {
                token += c;
            }
            else
            {
                if (!token.empty())
                {
                    m_tokens.emplace_back(line_number, token);
                    token.clear();
                }
                if (c == '=' && m_tokens.back() == "<")
                {
                    m_tokens[m_tokens.size() - 1] = "<=";
                }
                else if (c == '>' && m_tokens.back() == "=")
                {
                    m_tokens[m_tokens.size() - 1] = "=>";
                }
                else if (c != ' ')
                {
                    m_tokens.emplace_back(line_number, std::string(1, c));
                }
            }
        }
        if (!token.empty())
        {
            m_tokens.emplace_back(line_number, token);
            token.clear();
        }
    }

    // parse file into intermediate format
    if (!parse_tokens())
    {
        return nullptr;
    }

    // create const 0 and const 1 net, will be removed if unused
    m_zero_net = m_netlist->create_net("'0'");
    if (m_zero_net == nullptr)
    {
        log_error("hdl_parser", "could not instantiate GND net");
        return nullptr;
    }
    m_net_by_name[m_zero_net->get_name()] = m_zero_net;

    m_one_net = m_netlist->create_net("'1'");
    if (m_one_net == nullptr)
    {
        log_error("hdl_parser", "could not instantiate VCC net");
        return nullptr;
    }
    m_net_by_name[m_one_net->get_name()] = m_one_net;

    // build the netlist from the intermediate format
    // the last entity in the file is considered the top module
    if (!build_netlist(m_last_entity))
    {
        log_error("hdl_parser", "could not build netlist from parsed data");
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

bool hdl_parser_vhdl::parse_tokens()
{
    std::string last_entity;
    while (!m_tokens.consumed())
    {
        if (m_tokens.peek() == "library" || m_tokens.peek() == "use")
        {
            if (!parse_library())
            {
                return false;
            }
        }
        else if (m_tokens.peek() == "entity")
        {
            if (!parse_entity_definiton())
            {
                return false;
            }
        }
        else if (m_tokens.peek() == "architecture")
        {
            if (!parse_architecture())
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in global scope in line {}", m_tokens.peek().string, m_tokens.peek().number);
            return false;
        }
    }
    return true;
}

bool hdl_parser_vhdl::parse_library()
{
    if (m_tokens.peek() == "use")
    {
        m_tokens.consume("use");    // "use"
        auto lib = m_tokens.consume().string;
        m_tokens.consume(";");    // ";"

        // remove specific import like ".all" but keep the "."
        lib = core_utils::trim(lib.substr(0, lib.rfind(".") + 1));
        m_libraries.insert(core_utils::to_lower(lib));
    }
    else
    {
        m_tokens.consume_until(";");
        m_tokens.consume(";");
    }
    return true;
}

bool hdl_parser_vhdl::parse_entity_definiton()
{
    entity e;
    e.line_number = m_tokens.peek().number;
    m_tokens.consume();    // "entity"
    e.name = m_tokens.consume();
    m_tokens.consume();    // "is"
    while (m_tokens.peek() != "end")
    {
        if (m_tokens.peek() == "generic")
        {
            //TODO handle default values for generics
            m_tokens.consume_until(";");
            m_tokens.consume(";");
        }
        else if (m_tokens.peek() == "port")
        {
            if (!parse_port_definiton(e))
            {
                return false;
            }
        }
        else if (m_tokens.peek() == "attribute")
        {
            if (!parse_attribute(e.entity_attributes))
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}", m_tokens.peek().string, m_tokens.peek().number);
            return false;
        }
    }
    m_tokens.consume("end");
    m_tokens.consume();
    m_tokens.consume(";");

    if (!e.name.empty())
    {
        m_entities[e.name] = e;
        m_last_entity      = e.name;
    }

    return true;
}

bool hdl_parser_vhdl::parse_port_definiton(entity& e)
{
    m_tokens.consume("port");
    m_tokens.consume("(");
    auto ports = m_tokens.extract_until(")");

    while (!ports.consumed())
    {
        auto base_name = ports.consume();
        ports.consume(":");    // ":"
        auto direction    = ports.consume();
        token_stream type = ports.extract_until(";");

        if (!ports.consumed())
        {
            ports.consume(";");
        }

        for (const auto signal : get_vector_signals(base_name, type))
        {
            e.ports.emplace_back(signal, direction);
            e.expanded_signal_names[base_name].push_back(signal);
        }
    }
    m_tokens.consume(")");
    m_tokens.consume(";");
    return true;
}

bool hdl_parser_vhdl::parse_architecture()
{
    m_tokens.consume("architecture");
    m_tokens.consume();
    m_tokens.consume("of");
    auto& e = m_entities[m_tokens.consume()];
    m_tokens.consume();    // "is"
    return parse_architecture_header(e) && parse_architecture_body(e);
}

bool hdl_parser_vhdl::parse_architecture_header(entity& e)
{
    while (m_tokens.peek() != "begin")
    {
        if (m_tokens.peek() == "signal")
        {
            m_tokens.consume("signal");    // "signal"
            auto name = m_tokens.consume().string;
            m_tokens.consume(":");    // ":"
            auto type = m_tokens.extract_until(";");
            m_tokens.consume(";");

            // add all (sub-)signals
            for (const auto signal : get_vector_signals(name, type))
            {
                e.expanded_signal_names[name].push_back(signal);
                e.signals.push_back(signal);
            }
        }
        else if (m_tokens.peek() == "component")
        {
            // components are ignored
            m_tokens.consume_until("end");
            m_tokens.consume("end");
            m_tokens.consume();
            m_tokens.consume(";");
        }
        else if (m_tokens.peek() == "attribute")
        {
            auto signal_pos = m_tokens.find_next("signal");
            if (signal_pos < m_tokens.find_next(";") && m_tokens.at(signal_pos + 1) == "is")
            {
                parse_attribute(e.signal_attributes);
            }
            else
            {
                parse_attribute(e.instance_attributes);
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in architecture header in line {}", m_tokens.peek().string, m_tokens.peek().number);
            return false;
        }
    }

    for (const auto& [original, splits] : e.expanded_signal_names)
    {
        auto it = e.signal_attributes.find(original);
        if (it != e.signal_attributes.end())
        {
            for (const auto& split : splits)
            {
                e.signal_attributes[split] = it->second;
            }
        }
    }

    return true;
}

bool hdl_parser_vhdl::parse_architecture_body(entity& e)
{
    m_tokens.consume("begin");
    while (m_tokens.peek() != "end")
    {
        // new instance found
        if (m_tokens.peek(1) == ":")
        {
            if (!parse_instance(e))
            {
                return false;
            }
        }
        // not in instance -> has to be a direct assignment
        else if (m_tokens.find_next("<=") < m_tokens.find_next(";"))
        {
            auto middle_pos = m_tokens.find_next("<=");
            auto end_pos    = m_tokens.find_next(";");
            auto lhs        = m_tokens.extract_until("<=");
            m_tokens.consume("<=");
            auto rhs = m_tokens.extract_until(";");
            m_tokens.consume(";");

            for (const auto& [name, value] : get_assignments(lhs, rhs))
            {
                e.direct_assignments[name] = value;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in architecture body in line {}", m_tokens.peek().string, m_tokens.peek().number);
            return false;
        }
    }

    m_tokens.consume("end");
    m_tokens.consume();
    m_tokens.consume(";");

    return true;
}

bool hdl_parser_vhdl::parse_attribute(std::map<std::string, std::set<std::tuple<std::string, std::string, std::string>>>& mapping)
{
    u32 line_number = m_tokens.peek().number;
    m_tokens.consume();    // "attribute"
    auto attr_type = m_tokens.consume().string;
    if (m_tokens.peek() == ":")
    {
        m_tokens.consume(":");
        m_attribute_types[core_utils::to_lower(attr_type)] = m_tokens.join_until(";");
        m_tokens.consume(";");
    }
    else if (m_tokens.peek() == "of" && m_tokens.peek(2) == ":")
    {
        m_tokens.consume("of");
        auto attr_target = m_tokens.consume();
        m_tokens.consume(":");
        m_tokens.consume();
        m_tokens.consume("is");
        auto value = m_tokens.join_until(";").string;
        m_tokens.consume(";");

        if (value[0] == '"' && value.back() == '"')
        {
            value = value.substr(1, value.size() - 2);
        }
        auto type_it = m_attribute_types.find(core_utils::to_lower(attr_type));
        if (type_it == m_attribute_types.end())
        {
            log_warning("hdl_parser", "attribute {} has unknown base type in line {}", attr_type, line_number);
            mapping[attr_target].insert({attr_type, "UNKNOWN", value});
        }
        else
        {
            mapping[attr_target].insert({attr_type, type_it->second, value});
        }
    }
    else
    {
        log_error("hdl_parser", "malformed attribute defintion in line {}", line_number);
        return false;
    }
    return true;
}

bool hdl_parser_vhdl::parse_instance(entity& e)
{
    instance inst;

    // extract name and type
    inst.name = m_tokens.consume();
    m_tokens.consume(":");

    // remove prefix from type
    if (m_tokens.peek() == "entity")
    {
        m_tokens.consume("entity");
        inst.type = m_tokens.consume();
        auto pos  = inst.type.find('.');
        if (pos != std::string::npos)
        {
            inst.type = inst.type.substr(pos + 1);
        }
    }
    else if (m_tokens.peek() == "component")
    {
        m_tokens.consume("component");
        inst.type = m_tokens.consume();
    }
    else
    {
        inst.type     = m_tokens.consume();
        auto low_type = core_utils::to_lower(inst.type);
        std::string prefix;

        // find longest matching library prefix
        for (const auto& lib : m_libraries)
        {
            if (lib.size() > prefix.size() && core_utils::starts_with(low_type, lib))
            {
                prefix = lib;
            }
        }

        // remove prefix
        if (!prefix.empty())
        {
            inst.type = inst.type.substr(prefix.size());
        }
    }

    if (m_tokens.peek() == "generic")
    {
        m_tokens.consume("generic");
        m_tokens.consume("map");
        m_tokens.consume("(");
        auto generic_map = m_tokens.extract_until(")");
        m_tokens.consume(")");
        while (!generic_map.consumed())
        {
            auto lhs = generic_map.join_until("=>");
            generic_map.consume("=>");
            auto rhs = generic_map.join_until(",");
            if (!generic_map.consumed())
            {
                generic_map.consume(";");
            }

            inst.generics.emplace_back(lhs, rhs);
        }
    }

    if (m_tokens.peek() == "port")
    {
        m_tokens.consume("port");
        m_tokens.consume("map");
        m_tokens.consume("(");
        auto port_map = m_tokens.extract_until(")");
        m_tokens.consume(")");
        while (!port_map.consumed())
        {
            auto lhs = port_map.extract_until("=>");
            port_map.consume("=>");
            auto rhs = port_map.extract_until(",");
            if (!port_map.consumed())
            {
                port_map.consume(";");
            }

            for (const auto& [a, b] : get_assignments(lhs, rhs))
            {
                inst.ports.emplace_back(a, b);
            }
        }
    }

    if (!m_tokens.consume(";"))
    {
        log_error("hdl_parser", "unexpected end of instance declaration '{}' in line {}", m_tokens.peek().string, m_tokens.peek().number);
        return false;
    }

    e.instances.push_back(inst);

    return true;
}

// ###########################################################################
// #######          Build the netlist from intermediate format          ######
// ###########################################################################

bool hdl_parser_vhdl::build_netlist(const std::string& top_module)
{
    m_netlist->set_design_name(top_module);

    auto& top_entity = m_entities[top_module];

    // count the occurences of all names
    // names that occur multiple times will get a unique alias during parsing

    std::queue<entity*> q;
    q.push(&top_entity);

    for (const auto& x : top_entity.ports)
    {
        m_name_occurrences[x.first]++;
    }

    while (!q.empty())
    {
        auto e = q.front();
        q.pop();

        m_name_occurrences[e->name]++;

        for (const auto& x : e->signals)
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

    std::map<std::string, std::string> top_assignments;

    for (const auto& [name, direction] : top_entity.ports)
    {
        auto new_net                       = m_netlist->create_net(name);
        m_net_by_name[new_net->get_name()] = new_net;

        // for instances, point the ports to the newly generated signals
        top_assignments[new_net->get_name()] = new_net->get_name();

        if (new_net == nullptr)
        {
            log_error("hdl_parser", "could not create new net '{}'", name);
            return false;
        }

        if (core_utils::equals_ignore_case(direction, "in"))
        {
            if (!new_net->mark_global_input_net())
            {
                log_error("hdl_parser", "could not mark net '{}' as global input", name);
                return false;
            }
        }
        else if (core_utils::equals_ignore_case(direction, "out"))
        {
            if (!new_net->mark_global_output_net())
            {
                log_error("hdl_parser", "could not mark net '{}' as global output", name);
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "entity '{}', line {} : direction '{}' unkown", name, top_entity.line_number, direction);
            return false;
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
                continue;

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
                        log_error("hdl_parser", "could not merge nets '{}' and '{}'", slave_net->get_name(), master_net->get_name());
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

std::shared_ptr<module> hdl_parser_vhdl::instantiate(const entity& e, std::shared_ptr<module> parent, std::map<std::string, std::string> parent_module_assignments)
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
        log_error("hdl_parser", "could not instantiate the module '{}'", e.name);
        return nullptr;
    }

    // assign entity-level attributes
    {
        auto attribute_it = e.entity_attributes.find(e.name);
        if (attribute_it != e.entity_attributes.end())
        {
            for (const auto& attr : attribute_it->second)
            {
                if (!module->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                {
                    log_error("hdl_parser", "couldn't set data");
                }
            }
        }
    }

    // create all internal signals
    for (const auto& name : e.signals)
    {
        // create new net for the signal
        aliases[name] = get_unique_alias(name);
        auto new_net  = m_netlist->create_net(aliases[name]);
        if (new_net == nullptr)
        {
            log_error("hdl_parser", "could not instantiate the net '{}'", name);
            return nullptr;
        }
        m_net_by_name[aliases[name]] = new_net;

        // assign signal attributes
        {
            auto attribute_it = e.signal_attributes.find(name);
            if (attribute_it != e.signal_attributes.end())
            {
                for (const auto& attr : attribute_it->second)
                {
                    if (!new_net->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                    {
                        log_error("hdl_parser", "couldn't set data");
                    }
                }
            }
        }
    }

    // assign attributes to signals that are connected to ports
    for (const auto& [name, assignment] : parent_module_assignments)
    {
        auto attribute_it = e.signal_attributes.find(name);
        if (attribute_it != e.signal_attributes.end())
        {
            for (const auto& attr : attribute_it->second)
            {
                if (!m_net_by_name[assignment]->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                {
                    log_error("hdl_parser", "couldn't set data");
                }
            }
        }
    }

    std::set<std::string> output_ports;
    std::set<std::string> input_ports;
    for (const auto& [port, dir] : e.ports)
    {
        if (dir == "in")
        {
            input_ports.insert(port);
        }
        if (dir == "out")
        {
            output_ports.insert(port);
        }
    }

    for (const auto& [signal, assignment] : e.direct_assignments)
    {
        std::string a = signal;
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
        for (const auto& [pin, signal] : inst.ports)
        {
            auto it2 = parent_module_assignments.find(signal);
            if (it2 != parent_module_assignments.end())
            {
                instance_assignments[pin] = it2->second;
            }
            else
            {
                auto it3 = aliases.find(signal);
                if (it3 != aliases.end())
                {
                    instance_assignments[pin] = it3->second;
                }
                else if (signal == "'0'" || signal == "'1'")
                {
                    instance_assignments[pin] = signal;
                }
                else
                {
                    log_error("hdl_parser", "signal assignment \"{} => {}\" of instance {} is invalid ('{}' unknown)", pin, signal, inst.name, signal);
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
                auto it         = std::find_if(gate_types.begin(), gate_types.end(), [&](auto& v) { return core_utils::equals_ignore_case(v.first, inst.type); });
                if (it == gate_types.end())
                {
                    log_error("hdl_parser", "could not find gate type '{}' in gate library", inst.type);
                    return nullptr;
                }
                new_gate = m_netlist->create_gate(it->second, aliases[inst.name]);
            }

            if (new_gate == nullptr)
            {
                log_error("hdl_parser", "could not instantiate gate '{}'", inst.name);
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
            for (auto [pin_it, net_name] : inst.ports)
            {
                std::string pin = pin_it;    // copy to be able to overwrite

                // apply port assignments
                {
                    auto it = instance_assignments.find(pin);
                    if (it != instance_assignments.end())
                    {
                        net_name = it->second;
                    }
                }

                // if the net is an internal signal, use its alias
                if (std::find(e.signals.begin(), e.signals.end(), net_name) != e.signals.end())
                {
                    net_name = aliases.at(net_name);
                }

                // get the respective net for the assignment
                std::shared_ptr<net> current_net = nullptr;
                {
                    auto it = m_net_by_name.find(net_name);
                    if (it == m_net_by_name.end())
                    {
                        log_error("hdl_parser", "signal '{}' of {} was not previously declared", net_name, e.name);
                        return nullptr;
                    }
                    current_net = it->second;
                }

                // add net src/dst by pin types
                bool is_input = false;
                {
                    auto it = std::find_if(input_pin_types.begin(), input_pin_types.end(), [&](auto& s) { return core_utils::equals_ignore_case(s, pin); });
                    if (it != input_pin_types.end())
                    {
                        is_input = true;
                        pin      = *it;
                    }
                }
                bool is_output = false;
                {
                    auto it = std::find_if(output_pin_types.begin(), output_pin_types.end(), [&](auto& s) { return core_utils::equals_ignore_case(s, pin); });
                    if (it != output_pin_types.end())
                    {
                        is_output = true;
                        pin       = *it;
                    }
                }

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

        // assign instance attributes
        {
            auto attribute_it = e.instance_attributes.find(inst.name);
            if (attribute_it != e.instance_attributes.end())
            {
                for (const auto& attr : attribute_it->second)
                {
                    if (!container->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                    {
                        log_error("hdl_parser", "couldn't set data: key: {}, value_data_type: {}, value: {}", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr));
                    }
                }
            }
        }

        // process generics
        for (auto [name, value] : inst.generics)
        {
            auto bit_vector_candidate = core_utils::trim(core_utils::replace(value, "_", ""));

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
            else if (core_utils::ends_with(value, "s", true) || core_utils::ends_with(value, "sec", true) || core_utils::ends_with(value, "min", true) || core_utils::ends_with(value, "hr", true))
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

std::string hdl_parser_vhdl::get_hex_from_number_literal(const std::string& v)
{
    std::string value = core_utils::trim(core_utils::replace(v, "_", ""));
    value.pop_back();
    if (core_utils::starts_with(value, "X\"", true))
    {
        value = value.substr(2);
        return core_utils::to_lower(value);
    }
    // Conversion required
    int radix = 10;
    if (core_utils::starts_with(value, "D\"", true))
    {
        radix = 10;
        value = value.substr(2);
    }
    if (core_utils::starts_with(value, "B\"", true))
    {
        radix = 2;
        value = value.substr(2);
    }
    if (core_utils::starts_with(value, "O\"", true))
    {
        radix = 8;
        value = value.substr(2);
    }
    std::stringstream ss;
    ss << std::hex << stoull(value, 0, radix);
    return ss.str();
}

std::vector<std::string> hdl_parser_vhdl::get_vector_signals(const std::string& base_name, const std::vector<file_string>& type)
{
    // if there is no range given, the signal is just the name
    if (type.size() == 1)
    {
        return {base_name};
    }

    // extract bounds "a to/downto b, c to/downto d, ..."
    auto bounds = core_utils::to_lower(type.substr(type.find("(") + 1));
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
        if ((core_utils::to_upper(type).find(it.first) != std::string::npos) && (dimension < it.second))
        {
            dimension = it.second;
        }
    }

    // compare expected dimension with actually found dimension
    if (dimension != (u32)bound_tokens.size())
    {
        log_error("hdl_parser", "dimension-bound mismatch for {} : {}", base_name, type);
        return {};
    }

    // depending on the dimension, build the signal string(s)
    std::vector<std::string> result;
    if (dimension == 1)
    {
        for (auto x = std::get<0>(bound_tokens[0]); x <= std::get<1>(bound_tokens[0]); x++)
        {
            result.push_back(base_name + "(" + std::to_string(x) + ")");
        }
    }
    else if (dimension == 2)
    {
        for (auto x = std::get<0>(bound_tokens[0]); x <= std::get<1>(bound_tokens[0]); x++)
        {
            for (auto y = std::get<0>(bound_tokens[1]); y <= std::get<1>(bound_tokens[1]); y++)
            {
                result.push_back(base_name + "(" + std::to_string(x) + ", " + std::to_string(y) + ")");
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
                    result.push_back(base_name + "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
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

std::map<std::string, std::string> hdl_parser_vhdl::get_assignments(const std::vector<file_string>& lhs, const std::vector<file_string>& rhs)
{
    // a port may be
    // (1): a => ...
    // (2): a(x) => ...
    //      a(x, y, z) => ...
    // (3): a(x to y) => B"01010101..."
    // (4): a(x to y) => b(s to t)
    auto left_base_name  = lhs[0].string;
    auto right_base_name = rhs[0].string;

    // case (1)
    if (lhs.size() == 1)
    {
        return {std::make_pair(left_base_name, join(rhs))};
    }

    // check left and right bounds for ranges
    bool left_contains_range  = false;
    bool right_contains_range = false;
    for (u32 i = 1; i < lhs.size(); ++i)
    {
        if (lhs[i] == "to" || lhs[i] == "downto")
        {
            left_contains_range = true;
            break;
        }
    }
    for (u32 i = 1; i < rhs.size(); ++i)
    {
        if (rhs[i] == "to" || lhs[i] == "downto")
        {
            right_contains_range = true;
            break;
        }
    }

    if (!left_contains_range)
    {
        // case (2)
        return {std::make_pair(join(lhs), join(rhs))};
    }
    else    // -> left_contains_range true
    {
        int left_dir, left_start, left_end;
        if (lhs[3] == "downto")
        {
            left_start = std::stoi(lhs[4]);
            left_end   = std::stoi(lhs[2]);
            left_dir   = -1;
        }
        else if (lhs[3] == "to")
        {
            left_start = std::stoi(lhs[2]);
            left_end   = std::stoi(lhs[4]);
            left_dir   = 1;
        }
        else
        {
            log_error("hdl_parser", "port range '{}' could not be parsed (line {})", join(lhs), lhs[0].number);
            return {};
        }

        if (!right_contains_range)
        {
            // case (3)
            // right part has to be a bitvector
            if (rhs.size() != 1 || !core_utils::starts_with(rhs[0], "B\"", true))
            {
                log_error("hdl_parser", "assignment of anything but a binary bitvector is not supported (line {})", lhs[0].number);
                return {};
            }

            // extract value
            auto right_values = rhs[0].string.substr(2, rhs[0].string.size() - 3);

            // assemble assignment strings
            std::map<std::string, std::string> result;
            int i   = left_start;
            int cnt = 0;
            while (true)
            {
                result.emplace(left_base_name + "(" + std::to_string(i) + ")", std::string("'") + right_values[cnt] + "'");
                if (i == left_end)
                {
                    break;
                }
                cnt++;
                i += left_dir;
            }

            return result;
        }
        else    // -> right_contains_range true
        {
            // case (4)
            // right part is a range just like left part
            int right_dir, right_start, right_end;
            if (rhs[3] == "downto")
            {
                right_start = std::stoi(rhs[4]);
                right_end   = std::stoi(rhs[2]);
                right_dir   = -1;
            }
            else if (rhs[3] == "to")
            {
                right_start = std::stoi(rhs[2]);
                right_end   = std::stoi(rhs[4]);
                right_dir   = 1;
            }
            else
            {
                log_error("hdl_parser", "port range '{}' could not be parsed (line {})", join(rhs), rhs[0].number);
                return {};
            }

            int left_dist  = left_dir * (left_end - left_start);
            int right_dist = right_dir * (right_end - right_start);

            // check that both ranges match
            if (left_dist != right_dist)
            {
                log_error("hdl_parser", "ranges on port assignment '{} => {}' do not match in line {}", join(lhs), join(rhs), lhs[0].number);
                return {};
            }

            // assemble assignment strings
            std::map<std::string, std::string> result;
            int l = left_start;
            int r = right_start;
            while (left_dist >= 0)
            {
                result.emplace(left_base_name + "(" + std::to_string(l) + ")", right_base_name + "(" + std::to_string(r) + ")");
                l += left_dir;
                r += right_dir;
                left_dist--;
            }

            return result;
        }
    }
}

std::string hdl_parser_vhdl::get_unique_alias(const std::string& name)
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
