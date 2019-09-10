#include "hdl_parser/hdl_parser_verilog.h"

#include "core/log.h"
#include "core/utils.h"

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

    // read the whole file into logical parts
    entity current_entity;

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
                // reset current_entity
                current_entity.name        = core_utils::trim(token.substr(0, token.find('(')).substr(token.find(' ') + 1));
                current_entity.line_number = static_cast<u32>(token_begin);
                current_entity.definition.ports.clear();
                current_entity.definition.wires.clear();
                current_entity.definition.instances.clear();
            }
            else if (core_utils::starts_with(token, "input ") || core_utils::starts_with(token, "output ") || core_utils::starts_with(token, "inout "))
            {
                // collect port declarations
                current_entity.definition.ports.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else if (core_utils::starts_with(token, "wire "))
            {
                // collect wire declarations
                current_entity.definition.wires.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else if (core_utils::starts_with(token, "assign "))
            {
                // collect assign statements
                current_entity.definition.assigns.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else
            {
                // collect instance declarations
                current_entity.definition.instances.push_back(file_line{static_cast<u32>(token_begin), token});
            }

            token_begin = -1;
            token.clear();
        }
        else if (core_utils::starts_with(token, "endmodule"))
        {
            // add entity to set of entities
            m_entities[current_entity.name] = current_entity;

            token_begin = -1;
            token.clear();
        }
    }

    if (m_entities.empty())
    {
        log_error("hdl_parser", "file did not contain any entities.");
        return nullptr;
    }

    // parse intermediate format
    for (auto& it : m_entities)
    {
        if (!this->parse_entity(it.second))
        {
            return nullptr;
        }
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
    if (!build_netlist(current_entity.name))
    {
        return nullptr;
    }

    // add global gnd gate if required by any instance
    if (!m_zero_net->get_dsts().empty())
    {
        auto gnd_type   = *(m_netlist->get_gate_library()->get_global_gnd_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pin_types(gnd_type).at(0);
        auto gnd        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");
        if (!m_netlist->mark_global_gnd_gate(gnd))
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
        auto vcc_type   = *(m_netlist->get_gate_library()->get_global_vcc_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pin_types(vcc_type).at(0);
        auto vcc        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");
        if (!m_netlist->mark_global_vcc_gate(vcc))
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
        bool no_src = net->get_src().gate == nullptr && !net->is_global_inout_net() && !net->is_global_input_net();
        bool no_dst = net->get_num_of_dsts() == 0 && !net->is_global_inout_net() && !net->is_global_output_net();
        if (no_src && no_dst)
        {
            m_netlist->delete_net(net);
        }
    }

    return m_netlist;
}

bool hdl_parser_verilog::parse_entity(entity& e)
{
    if (!parse_ports(e))
    {
        return false;
    }

    if (!parse_wires(e))
    {
        return false;
    }

    if (!parse_assigns(e))
    {
        return false;
    }

    if (!parse_instances(e))
    {
        return false;
    }

    return true;
}

bool hdl_parser_verilog::parse_ports(entity& e)
{
    for (const auto& line : e.definition.ports)
    {
        // get port information
        auto direction = line.text.substr(0, line.text.find(' '));
        auto names     = line.text.substr(line.text.find(' ') + 1);

        // add all signals of that port
        for (const auto& it : this->get_expanded_wire_signals(names))
        {
            for (const auto& signal : it.second)
            {
                e.ports.emplace_back(signal, direction);
                e.expanded_signal_names[it.first].push_back(signal);
            }
        }
    }

    log_debug("hdl_parser", "parsed ports of '{}'.", e.name);
    return true;
}

bool hdl_parser_verilog::parse_wires(entity& e)
{
    for (const auto& line : e.definition.wires)
    {
        // get wire information
        auto names = line.text.substr(line.text.find(' ') + 1);

        // add all (sub-)signals
        for (const auto& it : this->get_expanded_wire_signals(names))
        {
            for (const auto& signal : it.second)
            {
                e.signals.push_back(signal);
                e.expanded_signal_names[it.first].push_back(signal);
            }
        }
    }

    log_debug("hdl_parser", "parsed signals of '{}'.", e.name);
    return true;
}

bool hdl_parser_verilog::parse_assigns(entity& e)
{
    for (const auto& line : e.definition.assigns)
    {
        auto signal     = core_utils::trim(line.text.substr(6, line.text.find('=') - 6));
        auto assignment = core_utils::trim(line.text.substr(line.text.find('=') + 1));

        for (const auto& a : this->get_port_assignments(signal, assignment, e))
        {
            e.direct_assignments.emplace(a);
        }
    }

    for (auto a : e.direct_assignments)
    {
        std::cout << a.first << " = " << a.second << ";" << std::endl;
    }

    return true;
}

bool hdl_parser_verilog::parse_instances(entity& e)
{
    for (const auto& line : e.definition.instances)
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
            port_map  = core_utils::split(line.text.substr(0, line.text.rfind(')')).substr(line.text.find('(') + 1), ',', true);
        }
        else
        {
            // TYPE #(GENERIC_MAP) NAME (PORT_MAP)
            auto substr = line.text.substr(line.text.find('#') + 1);
            auto token  = core_utils::split(substr, ')', true);

            inst.type   = core_utils::trim(line.text.substr(0, line.text.find("#(")));
            inst.name   = core_utils::trim(token[1].substr(0, token[1].find('(')));
            generic_map = core_utils::split(token[0].substr(1), ',', true);
            port_map    = core_utils::split(token[1].substr(token[1].find('(') + 1), ',', true);
        }

        for (const auto& p : port_map)
        {
            if (p.empty())
            {
                continue;
            }

            // .KEY(VALUE)
            auto port       = core_utils::trim(p.substr(0, p.find('('))).substr(1);
            auto assignment = core_utils::trim(p.substr(0, p.find(')')).substr(p.find('(') + 1));

            for (const auto& a : this->get_port_assignments(port, assignment, e))
            {
                inst.ports.push_back(a);
            }
        }

        for (const auto& g : generic_map)
        {
            if (g.empty())
            {
                continue;
            }

            // .KEY(VALUE)
            auto generic    = core_utils::trim(g.substr(0, g.find('('))).substr(1);
            auto assignment = core_utils::trim(g.substr(0, g.find(')')).substr(g.find('(') + 1));

            inst.generics.emplace_back(generic, assignment);
        }

        e.instances.push_back(inst);
        log_debug("hdl_parser", "parsed instance '{}' of entity '{}'.", inst.name, e.name);
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

    // for the top module, generate global i/o signals for all ports

    std::map<std::string, std::function<bool(std::shared_ptr<net> const)>> port_dir_function = {
        {"input", [](std::shared_ptr<net> const net) { return net->mark_global_input_net(); }},
        {"output", [](std::shared_ptr<net> const net) { return net->mark_global_output_net(); }},
        {"inout", [](std::shared_ptr<net> const net) { return net->mark_global_inout_net(); }},
    };

    std::map<std::string, std::string> top_assignments;

    for (const auto& [name, direction] : top_entity.ports)
    {
        if (port_dir_function.find(direction) == port_dir_function.end())
        {
            log_error("hdl_parser", "entity {}, line {}+ : direction '{}' unknown", name, top_entity.line_number, direction);
            return false;
        }

        auto new_net = m_netlist->create_net(name);
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
    for (const auto& name : e.signals)
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

    for (const auto& [port, dir] : e.ports)
    {
        if (dir == "input")
        {
            input_ports.insert(port);
        }
        if (dir == "output")
        {
            output_ports.insert(port);
        }
    }

    std::cout << "front" << std::endl;

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

    std::cout << "end" << std::endl;

    // cache global vcc/gnd types
    auto global_vcc_gate_types = m_netlist->get_gate_library()->get_global_vcc_gate_types();
    auto global_gnd_gate_types = m_netlist->get_gate_library()->get_global_gnd_gate_types();

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
                    log_error("hdl_parser", "signal assignment \"{} = {}\" of instance {} is invalid", pin, signal, inst.name);
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
            auto new_gate      = m_netlist->create_gate(inst.type, aliases[inst.name]);
            if (new_gate == nullptr)
            {
                return nullptr;
            }
            module->assign_gate(new_gate);
            container = new_gate.get();

            // if gate is a global type, register it as such
            if (global_vcc_gate_types->find(inst.type) != global_vcc_gate_types->end() && !new_gate->mark_global_vcc_gate())
            {
                return nullptr;
            }
            if (global_gnd_gate_types->find(inst.type) != global_gnd_gate_types->end() && !new_gate->mark_global_gnd_gate())
            {
                return nullptr;
            }

            // cache pin types
            auto input_pin_types  = new_gate->get_input_pin_types();
            auto output_pin_types = new_gate->get_output_pin_types();
            auto inout_pin_types  = new_gate->get_inout_pin_types();

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
                if (std::find(e.signals.begin(), e.signals.end(), net_name) != e.signals.end())
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
                bool is_inout  = std::find(inout_pin_types.begin(), inout_pin_types.end(), pin) != inout_pin_types.end();
                bool is_input  = is_inout || std::find(input_pin_types.begin(), input_pin_types.end(), pin) != input_pin_types.end();
                bool is_output = is_inout || std::find(output_pin_types.begin(), output_pin_types.end(), pin) != output_pin_types.end();

                if (!is_input && !is_output)
                {
                    log_error("hdl_parser", "undefined pin '{}' for '{}' ({})", pin, new_gate->get_name(), new_gate->get_type());
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
                                  src->get_type(),
                                  new_gate->get_name(),
                                  new_gate->get_type());
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

        // TODO process generics
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

std::map<std::string, std::vector<std::string>> hdl_parser_verilog::get_expanded_wire_signals(const std::string& line)
{
    std::map<std::string, std::vector<std::string>> result;
    std::vector<std::pair<i32, i32>> bound_tokens;
    std::vector<std::string> names;

    auto wire_colon = line.find(':');

    if (wire_colon != std::string::npos)
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
    else
    {
        // no bounds given, just extract signals from list
        names = core_utils::split(line, ',');
    }

    for (auto name : names)
    {
        u32 dimension    = bound_tokens.size();
        name             = core_utils::trim(name);
        auto wire_escape = name.find('\\');

        if (wire_escape != std::string::npos)
        {
            name = name.substr(wire_escape + 1);
        }

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

std::map<std::string, std::string> hdl_parser_verilog::get_port_assignments(const std::string& port, const std::string& assignment, entity& e)
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

    auto assignment_curly = assignment.find('{');

    std::vector<std::string> assignment_list;

    // (6)
    if (assignment_curly == std::string::npos)
    {
        assignment_list.push_back(assignment);
    }
    else
    {
        assignment_list = core_utils::split(assignment.substr(0, assignment.find('}')).substr(assignment_curly + 1), ',');
    }

    for (auto a : assignment_list)
    {
        a = core_utils::trim(a);

        auto assignment_bracket = a.find('[');
        auto assignment_number  = a.find('\'');
        auto assignment_escape  = a.find('\\');

        if ((assignment_bracket == std::string::npos || assignment_escape != std::string::npos) && assignment_number == std::string::npos)
        {
            // (1) and (2)
            auto name = core_utils::trim(a);

            if (assignment_escape != std::string::npos)
            {
                name = core_utils::trim(name.substr(assignment_escape + 1));
            }

            if (e.expanded_signal_names.find(name) == e.expanded_signal_names.end())
            {
                log_error("hdl_parser", "no wire or port '{}' within current entity.", a);
                return {};
            }

            right_parts.insert(right_parts.end(), e.expanded_signal_names[name].begin(), e.expanded_signal_names[name].end());
        }
        else if (assignment_number != std::string::npos)
        {
            // (3)
            auto number = get_bin_from_number_literal(a);

            for (auto bit : number)
            {
                right_parts.push_back("'" + std::to_string(bit - 48) + "'");
            }
        }
        else if (assignment_bracket != std::string::npos)
        {
            // (4) and (5)
            auto assignment_colon = a.find(':');

            if (assignment_colon == std::string::npos)
            {
                // (4)
                auto substr = a.substr(0, a.rfind(']')).substr(assignment_bracket + 1);
                auto name   = core_utils::trim(a.substr(0, assignment_bracket));

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
                auto bounds = core_utils::split(a.substr(0, a.rfind(']')).substr(assignment_bracket + 1), ':');
                auto name   = core_utils::trim(a.substr(0, assignment_bracket));

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
            log_error("hdl_parser", "not implemented reached for port assignment '{}'", a);
            return {};
        }
    }

    // PARSE PORT
    //   port can currently be one of the following:
    //   (1) NAME *single*
    //   (2) NAME *one-dimensional*
    //   (3) NAME[INDEX1][INDEX2]...
    //   (4) NAME[BEGIN_INDEX:END_INDEX]

    auto port_bracket = port.find('[');
    auto port_escape  = port.find('\\');

    if (port_bracket == std::string::npos || port_escape != std::string::npos)
    {
        auto name = port;

        if (port_escape != std::string::npos)
        {
            name = name.substr(port_escape + 1);
        }

        // (1) and (2)
        // infer size from right side
        if (right_parts.size() > 1)
        {
            for (u32 i = right_parts.size(); i > 0; i--)
            {
                left_parts.push_back(name + "(" + std::to_string(i - 1) + ")");
            }
        }
        else
        {
            left_parts.push_back(name);
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

    for (u32 i = 0; i < right_parts.size(); i++)
    {
        result[left_parts[i]] = right_parts[i];
    }

    return result;
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
