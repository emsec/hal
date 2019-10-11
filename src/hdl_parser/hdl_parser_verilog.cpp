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
                current_entity.name = core_utils::trim(token.substr(0, token.find('(')).substr(token.find(' ') + 1));

                if (current_entity.name.empty())
                {
                    log_error("hdl_parser", "Verilog module must have a name.");
                    return nullptr;
                }

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
        if (!parse_ports(it.second))
        {
            return nullptr;
        }

        if (!parse_signals(it.second))
        {
            return nullptr;
        }
    }

    for (auto& it : m_entities)
    {
        if (!parse_assigns(it.second))
        {
            return nullptr;
        }

        if (!parse_instances(it.second))
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

bool hdl_parser_verilog::parse_ports(entity& e)
{
    for (const auto& line : e.definition.ports)
    {
        // get port information
        auto direction = line.text.substr(0, line.text.find(' '));
        auto port_str  = line.text.substr(line.text.find(' ') + 1);

        // add all signals of that port
        for (const auto& expanded_port : this->get_expanded_signals(port_str))
        {
            e.ports_expanded[expanded_port.first] = std::make_pair(direction, expanded_port.second);
            e.expanded_signal_names[expanded_port.first].insert(e.expanded_signal_names[expanded_port.first].end(), expanded_port.second.begin(), expanded_port.second.end());
        }
    }

    log_debug("hdl_parser", "parsed ports of '{}'.", e.name);
    return true;
}

bool hdl_parser_verilog::parse_signals(entity& e)
{
    for (const auto& line : e.definition.wires)
    {
        // get wire information
        auto signal_str = line.text.substr(line.text.find(' ') + 1);

        // add all (sub-)signals
        for (const auto& expanded_signal : this->get_expanded_signals(signal_str))
        {
            e.signals_expanded.insert(e.signals_expanded.end(), expanded_signal.second.begin(), expanded_signal.second.end());
            e.expanded_signal_names[expanded_signal.first].insert(e.expanded_signal_names[expanded_signal.first].end(), expanded_signal.second.begin(), expanded_signal.second.end());
        }
    }

    log_debug("hdl_parser", "parsed signals of '{}'.", e.name);
    return true;
}

bool hdl_parser_verilog::parse_assigns(entity& e)
{
    for (const auto& line : e.definition.assigns)
    {
        auto left  = core_utils::trim(line.text.substr(6, line.text.find('=') - 6));
        auto right = core_utils::trim(line.text.substr(line.text.find('=') + 1));

        auto direct_assignment = this->get_direct_assignments(left, right, e);

        if (direct_assignment.empty() == true)
        {
            return false;
        }

        for (const auto& a : direct_assignment)
        {
            e.direct_assignments.emplace(a);
        }
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

        auto inst_escape = inst.name.find('\\');

        if (inst_escape != std::string::npos)
        {
            inst.name = inst.name.substr(inst_escape + 1);
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

            auto port_assignments = this->get_port_assignments(inst.type, port, assignment, e);

            if (port_assignments.empty() == true)
            {
                return false;
            }

            for (const auto& a : port_assignments)
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

    std::map<std::string, std::function<bool(std::shared_ptr<net> const)>> port_dir_function = {
        {"input", [](std::shared_ptr<net> const net) { return net->mark_global_input_net(); }},
        {"output", [](std::shared_ptr<net> const net) { return net->mark_global_output_net(); }},
        {"inout", [](std::shared_ptr<net> const net) { return net->mark_global_inout_net(); }},
    };

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
    auto global_vcc_gate_types = m_netlist->get_gate_library()->get_global_vcc_gate_types();
    auto global_gnd_gate_types = m_netlist->get_gate_library()->get_global_gnd_gate_types();

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

std::map<std::string, std::vector<std::string>> hdl_parser_verilog::get_expanded_signals(const std::string& signal_str)
{
    std::map<std::string, std::vector<std::string>> result;
    std::vector<std::pair<std::string, std::vector<std::pair<i32, i32>>>> signals;

    auto signal_colon  = signal_str.find(':');
    auto signal_escape = signal_str.find('\\');

    // bounds given?
    if (signal_colon != std::string::npos)
    {
        // extract signal bounds "[a:b][c:d]..."
        std::vector<std::pair<i32, i32>> bounds;
        auto bounds_str = signal_str.substr(0, signal_str.rfind("]", signal_escape)).substr(signal_str.find("[") + 1);
        auto names      = core_utils::split(signal_str.substr(signal_str.rfind(']', signal_escape) + 1), ',');

        for (const auto& part : core_utils::split(bounds_str, ']'))
        {
            auto bound = core_utils::trim(part.substr(part.find('[') + 1));

            i32 left_bound  = std::stoi(bound.substr(0, bound.find(':')));
            i32 right_bound = std::stoi(bound.substr(bound.find(':') + 1));

            bounds.emplace_back(left_bound, right_bound);
        }

        // extract signal names
        for (const auto& name : names)
        {
            std::pair<std::string, std::vector<std::pair<i32, i32>>> s;
            signal_escape = name.find('\\');

            if (signal_escape != std::string::npos)
            {
                s.first = core_utils::trim(name.substr(signal_escape + 1));
            }
            else
            {
                s.first = core_utils::trim(name);
            }

            s.second = bounds;

            signals.push_back(s);
        }
    }
    else
    {
        // extract signal names
        auto names = core_utils::split(signal_str, ',');

        for (const auto& name : names)
        {
            std::pair<std::string, std::vector<std::pair<i32, i32>>> s;
            signal_escape = name.find('\\');

            if (signal_escape != std::string::npos)
            {
                s.first = core_utils::trim(name.substr(signal_escape + 1));
            }
            else
            {
                s.first = core_utils::trim(name);
            }

            signals.push_back(s);
        }
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

std::vector<std::string> hdl_parser_verilog::get_assignment_signals(const std::string& signal_str, entity& e)
{
    std::vector<std::string> result, signal_list;

    // PARSE ASSIGNMENT
    //   assignment can currently be one of the following:
    //   (1) NAME *single*
    //   (2) NAME *multi-dimensional*
    //   (3) NUMBER
    //   (4) NAME[INDEX1][INDEX2]...
    //   (5) NAME[BEGIN_INDEX1:END_INDEX1][BEGIN_INDEX2:END_INDEX2]...
    //   (6) {(1) - (5), (1) - (5), ...}

    auto signal_curly = signal_str.find('{');

    // (6)
    if (signal_curly == std::string::npos)
    {
        signal_list.push_back(signal_str);
    }
    else
    {
        signal_list = core_utils::split(signal_str.substr(0, signal_str.find('}')).substr(signal_curly + 1), ',');
    }

    for (auto s : signal_list)
    {
        std::string signal_name;

        s = core_utils::trim(s);

        auto signal_escape     = s.find('\\');
        auto signal_bracket    = s.find('[');
        auto signal_apostrophe = s.find('\'');

        // extract signal name
        if (signal_escape != std::string::npos)
        {
            auto signal_space = s.find(' ', signal_escape);

            if (signal_space != std::string::npos)
            {
                signal_name = core_utils::trim(s.substr(0, signal_space).substr(signal_escape + 1));
                s           = s.substr(signal_space + 1);
            }
            else
            {
                signal_name = core_utils::trim(s.substr(signal_escape + 1));
                s           = "";
            }
        }
        else
        {
            if (signal_bracket != std::string::npos)
            {
                // (4), (5) part 1
                signal_name = core_utils::trim(s.substr(0, signal_bracket));
                s           = s.substr(signal_bracket);
            }
            else if (signal_apostrophe != std::string::npos)
            {
                // (3)
                for (auto bit : get_bin_from_number_literal(s))
                {
                    result.push_back("'" + std::to_string(bit - 48) + "'");
                }

                continue;
            }
            else
            {
                // (1), (2) part 1
                signal_name = s;
                s           = "";
            }
        }

        // valid signal/port name?
        if (e.expanded_signal_names.find(signal_name) == e.expanded_signal_names.end())
        {
            log_error("hdl_parser", "no wire or port '{}' within entity '{}'.", signal_name, e.name);
            return {};
        }

        signal_bracket = s.find('[');

        // bounds given?
        if (signal_bracket != std::string::npos)
        {
            // (4), (5) part 2
            auto assignment_colon = s.find(':');

            if (assignment_colon == std::string::npos)
            {
                // (4)
                auto bounds_str = s.substr(0, s.rfind(']')).substr(signal_bracket + 1);

                for (const auto& part : core_utils::split(bounds_str, ']'))
                {
                    auto index = std::stoi(core_utils::trim(part.substr(part.find('[') + 1)));
                    signal_name += "(" + std::to_string(index) + ")";
                }

                result.push_back(signal_name);
            }
            else
            {
                // (5)
                auto bounds_str = s.substr(0, s.rfind(']')).substr(signal_bracket + 1);
                std::vector<std::pair<i32, i32>> bounds;
                std::vector<std::string> expanded_signal;

                for (const auto& part : core_utils::split(bounds_str, ']'))
                {
                    auto bound = core_utils::trim(part.substr(part.find('[') + 1));

                    i32 left_bound  = std::stoi(bound.substr(0, bound.find(':')));
                    i32 right_bound = std::stoi(bound.substr(bound.find(':') + 1));

                    bounds.emplace_back(left_bound, right_bound);
                }

                expand_signal(expanded_signal, signal_name, bounds, 0);

                result.insert(result.end(), expanded_signal.begin(), expanded_signal.end());
            }
        }
        else
        {
            // (1), (2) part 2
            result.insert(result.end(), e.expanded_signal_names[signal_name].begin(), e.expanded_signal_names[signal_name].end());
        }
    }

    return result;
}

std::vector<std::string> hdl_parser_verilog::get_port_signals(const std::string& port_str, const std::string& instance_name)
{
    std::vector<std::string> result;

    std::string port_name;

    auto signal_escape = port_str.find('\\');

    // extract port name
    if (signal_escape != std::string::npos)
    {
        auto signal_space = port_str.find(' ', signal_escape);

        port_name = core_utils::trim(port_str.substr(0, signal_space).substr(signal_escape + 1));
    }
    else
    {
        port_name = port_str;
    }

    // try to validate port name
    if (m_entities.find(instance_name) != m_entities.end())
    {
        // is instance a valid entity within netlist?
        if (m_entities[instance_name].ports_expanded.find(port_name) != m_entities[instance_name].ports_expanded.end())
        {
            // is port valid for given entity
            result.insert(result.end(), m_entities[instance_name].ports_expanded[port_name].second.begin(), m_entities[instance_name].ports_expanded[port_name].second.end());
        }
        else
        {
            log_error("hdl_parser", "no port '{}' within entity '{}'.", port_name, instance_name);
            return {};
        }
    }
    else
    {
        // assume port is valid port for gate of given gate library
        result.push_back(port_name);
    }

    return result;
}

std::map<std::string, std::string> hdl_parser_verilog::get_direct_assignments(const std::string& left, const std::string& right, entity& e)
{
    std::map<std::string, std::string> result;

    auto left_parts  = get_assignment_signals(left, e);
    auto right_parts = get_assignment_signals(right, e);

    if (left_parts.empty() || right_parts.empty())
    {
        log_error("hdl_parser", "cannot parse direct assignment '{}'.", left + " = " + right);
        return {};
    }

    if (left_parts.size() != right_parts.size())
    {
        log_error("hdl_parser", "cannot parse direct assignment '{}' due to width mismatch.", left + " = " + right);
        return {};
    }

    for (u32 i = 0; i < right_parts.size(); i++)
    {
        result[left_parts[i]] = right_parts[i];
    }

    return result;
}

std::map<std::string, std::string> hdl_parser_verilog::get_port_assignments(const std::string& inst, const std::string& port, const std::string& assignment, entity& e)
{
    std::map<std::string, std::string> result;

    auto left_parts  = get_port_signals(port, inst);
    auto right_parts = get_assignment_signals(assignment, e);

    if (left_parts.empty() || right_parts.empty())
    {
        log_error("hdl_parser", "cannot parse port assignment '{}'.", port + "(" + assignment + ")");
        return {};
    }

    if (left_parts.size() != right_parts.size())
    {
        log_error("hdl_parser", "cannot parse port assignment '{}' due to width mismatch.", port + "(" + assignment + ")");
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
