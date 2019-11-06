#include "hdl_parser/hdl_parser_vhdl.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

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

    // read the whole file into logical parts

    std::vector<file_line> header;
    std::vector<file_line> libraries;
    std::vector<std::vector<file_line>> components;

    std::vector<file_line> current_component;
    entity current_entity;

    std::string line;
    u32 line_number = 0;

    enum
    {
        NONE,
        IN_ENTITY_HEADER,
        IN_ARCH_HEADER,
        IN_COMPONENT,
        IN_ARCH_BODY
    } state;

    while (std::getline(m_fs, line))
    {
        line_number++;
        line = core_utils::trim(line);

        // skip empty lines
        if (line.empty())
        {
            continue;
        }

        if (core_utils::starts_with(line, "--"))
        {
            header.push_back(file_line{line_number, line});
        }
        else if (core_utils::starts_with(line, "library ", true) || core_utils::starts_with(line, "use ", true))
        {
            libraries.push_back(file_line{line_number, line});
        }
        else if (core_utils::starts_with(line, "entity ", true))
        {
            if (!current_entity.name.empty())
            {
                m_entities[current_entity.name] = current_entity;
            }

            state = IN_ENTITY_HEADER;
            current_entity.definition.entity_header.clear();
            current_entity.definition.architecture_header.clear();
            current_entity.definition.architecture_body.clear();
            current_entity.name        = line.substr(line.find(' ') + 1);
            current_entity.name        = current_entity.name.substr(0, current_entity.name.rfind(' '));
            current_entity.line_number = line_number;
        }
        else
        {
            if (core_utils::starts_with(line, "architecture ", true))
            {
                state = IN_ARCH_HEADER;
                continue;
            }
            else if (state == IN_ARCH_HEADER && core_utils::starts_with(line, "component ", true))
            {
                state = IN_COMPONENT;
                // no continue, first line is already important
            }
            else if (state == IN_COMPONENT && core_utils::starts_with(line, "end component ", true))
            {
                components.push_back(current_component);
                current_component.clear();
                state = IN_ARCH_HEADER;
                continue;
            }
            else if (state == IN_ARCH_HEADER && core_utils::equals_ignore_case(line, "begin"))
            {
                state = IN_ARCH_BODY;
                continue;
            }
            else if (core_utils::starts_with(line, "end ", true))
            {
                state = NONE;
                continue;
            }

            if (state == IN_ENTITY_HEADER)
            {
                current_entity.definition.entity_header.push_back(file_line{line_number, line});
            }
            else if (state == IN_ARCH_HEADER)
            {
                current_entity.definition.architecture_header.push_back(file_line{line_number, line});
            }
            else if (state == IN_ARCH_BODY)
            {
                current_entity.definition.architecture_body.push_back(file_line{line_number, line});
            }
            else if (state == IN_COMPONENT)
            {
                current_component.push_back(file_line{line_number, line});
            }
        }
    }
    if (!current_entity.name.empty())
    {
        m_entities[current_entity.name] = current_entity;
    }

    if (m_entities.empty())
    {
        log_error("hdl_parser", "file did not contain any entities.");
        return nullptr;
    }

    // parse additional information
    if (!this->parse_header(header))
    {
        log_error("hdl_parser", "could not parse header information");
        return nullptr;
    }
    if (!this->parse_libraries(libraries))
    {
        log_error("hdl_parser", "could not parse vhdl libraries");
        return nullptr;
    }
    if (!this->parse_components(components))
    {
        log_error("hdl_parser", "could not parse components");
        return nullptr;
    }

    // parse intermediate format
    for (auto& it : m_entities)
    {
        if (!this->parse_entity(it.second))
        {
            log_error("hdl_parser", "could not parse entity {}", it.second.name);
            return nullptr;
        }
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
    if (!build_netlist(current_entity.name))
    {
        log_error("hdl_parser", "could not build netlist from parsed data");
        return nullptr;
    }

    // add global gnd gate if required by any instance
    if (!m_zero_net->get_dsts().empty())
    {
        auto gnd_type   = m_netlist->get_gate_library()->get_global_gnd_gate_types().begin()->second;
        auto output_pin = gnd_type->get_output_pins().at(0);
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
        auto vcc_type   = m_netlist->get_gate_library()->get_global_vcc_gate_types().begin()->second;
        auto output_pin = vcc_type->get_output_pins().at(0);
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
        bool no_src = net->get_src().gate == nullptr && !net->is_global_input_net();
        bool no_dst = net->get_num_of_dsts() == 0 && !net->is_global_output_net();
        if (no_src && no_dst)
        {
            m_netlist->delete_net(net);
        }
    }

    return m_netlist;
}

bool hdl_parser_vhdl::parse_header(const std::vector<file_line>& header)
{
    for (const auto& line : header)
    {
        // read out device comment
        if (core_utils::starts_with(line.text, "-- Device"))
        {
            auto device_name = core_utils::trim(line.text.substr(line.text.find(':') + 1));
            m_netlist->set_device_name(device_name);
            break;
        }
    }
    log_debug("hdl_parser", "parsed header.");
    return true;
}

bool hdl_parser_vhdl::parse_libraries(const std::vector<file_line>& libs)
{
    for (const auto& line : libs)
    {
        if (core_utils::starts_with(line.text, "use ", true))
        {
            auto lib = line.text.substr(line.text.find(' '));
            lib      = core_utils::trim(lib.substr(0, lib.rfind(".") + 1));
            m_libraries.insert(core_utils::to_lower(lib));
        }
    }
    log_debug("hdl_parser", "parsed header.");
    return true;
}

bool hdl_parser_vhdl::parse_components(const std::vector<std::vector<file_line>>& components)
{
    UNUSED(components);
    // Gate library mutation not allowed
    /*
    auto lib = m_netlist->get_gate_library();

    for (const auto& component : components)
    {
        std::string new_component;

        bool in_port_def = false;

        for (const auto& line : component)
        {
            // line starts with "component" -> extract name
            if (core_utils::starts_with(line.text, "component "))
            {
                new_component = core_utils::trim(line.text.substr(line.text.find(' ') + 1));
                new_component = new_component.substr(0, new_component.find(' '));

                lib->get_gate_types()->insert(new_component);
            }
            else if (line.text == "port (")
            {
                in_port_def = true;
            }
            else if (line.text == ");")
            {
                in_port_def = false;
            }
            else if (in_port_def)
            {
                // get port information
                auto token     = core_utils::split(line.text, ':');
                auto name      = core_utils::rtrim(token[0]);
                auto config    = core_utils::ltrim(token[1]);
                auto direction = config.substr(0, config.find(' '));
                auto type      = config.substr(config.find(' ') + 1);

                // add all signals of that port
                for (const auto signal : get_vector_signals(name, type))
                {
                    if (direction == "in")
                    {
                        lib->get_input_pin_types()->insert(signal);
                        (*lib->get_gate_type_map_to_input_pin_types())[new_component].push_back(signal);
                    }
                    else if (direction == "out")
                    {
                        lib->get_output_pin_types()->insert(signal);
                        (*lib->get_gate_type_map_to_output_pin_types())[new_component].push_back(signal);
                    }
                    else
                    {
                        log_error("hdl_parser", "line {}: unknown/unsupported port direction {}", line.number, direction);
                        return false;
                    }
                }
            }
        }
    }
    */
    return true;
}

void hdl_parser_vhdl::parse_attribute(std::map<std::string, std::set<std::tuple<std::string, std::string, std::string>>>& mapping, const file_line& line)
{
    auto left  = core_utils::rtrim(line.text.substr(0, line.text.find(':')));
    auto right = core_utils::ltrim(line.text.substr(line.text.find(':') + 1));
    right.pop_back();

    auto left_parts = core_utils::split(left, ' ');

    if (left_parts.size() == 2)
    {
        m_attribute_types[core_utils::to_lower(left_parts[1])] = right;
    }
    else if (left_parts.size() == 4 && core_utils::equals_ignore_case(left_parts[2], "of"))
    {
        std::string value = right.substr(core_utils::to_lower(right).find(" is ") + 4);
        if (value[0] == '"' && value.back() == '"')
        {
            value = value.substr(1, value.size() - 2);
        }
        auto type_it = m_attribute_types.find(core_utils::to_lower(left_parts[1]));
        if (type_it == m_attribute_types.end())
        {
            log_warning("hdl_parser", "line {}: attribute {} has unknown type", line.number, left_parts[1]);
            mapping[left_parts[3]].insert({left_parts[1], "UNKNOWN", value});
        }
        else
        {
            mapping[left_parts[3]].insert({left_parts[1], type_it->second, value});
        }
    }
    else
    {
        log_error("hdl_parser", "malformed attribute defintion '{}'", line.text);
    }
}

bool hdl_parser_vhdl::parse_entity(entity& e)
{
    if (!parse_entity_header(e))
    {
        return false;
    }

    if (!parse_architecture_header(e))
    {
        return false;
    }

    if (!parse_architecture_body(e))
    {
        return false;
    }

    return true;
}

bool hdl_parser_vhdl::parse_entity_header(entity& e)
{
    bool in_port_def = false;

    for (const auto& line : e.definition.entity_header)
    {
        // line starts with "port" -> extract port definition
        if (core_utils::equals_ignore_case(line.text, "port ("))
        {
            in_port_def = true;
        }
        // line is ");" -> no more ports
        else if (in_port_def && line.text == ");")
        {
            in_port_def = false;
        }
        // we expect ports at this point
        else if (in_port_def)
        {
            // get port information
            auto token     = core_utils::split(line.text, ':');
            auto name      = core_utils::trim(token[0]);
            auto config    = core_utils::trim(token[1]);
            auto direction = core_utils::to_lower(config.substr(0, config.find(' ')));
            auto type      = config.substr(config.find(' ') + 1);

            // add all signals of that port
            for (const auto signal : get_vector_signals(name, type))
            {
                e.ports.emplace_back(signal, direction);
                e.expanded_signal_names[name].push_back(signal);
            }
        }
        else if (!in_port_def && core_utils::starts_with(line.text, "attribute ", true))
        {
            parse_attribute(e.entity_attributes, line);
        }
    }

    log_debug("hdl_parser", "parsed entity header of '{}'.", e.name);
    return true;
}

bool hdl_parser_vhdl::parse_architecture_header(entity& e)
{
    for (const auto& line : e.definition.architecture_header)
    {
        if (core_utils::starts_with(line.text, "signal ", true))
        {
            // get signal information
            auto items = core_utils::split(line.text, ':');
            auto name  = core_utils::trim(items[0].substr(items[0].find(' ') + 1));
            auto type  = core_utils::trim(items[1]);

            // add all (sub-)signals
            for (const auto signal : get_vector_signals(name, type))
            {
                e.expanded_signal_names[name].push_back(signal);
                e.signals.push_back(signal);
            }
        }
        else if (core_utils::starts_with(line.text, "attribute ", true))
        {
            if (core_utils::to_lower(line.text).find("signal is \"") != std::string::npos)
            {
                parse_attribute(e.signal_attributes, line);
            }
            else
            {
                parse_attribute(e.instance_attributes, line);
            }
        }
        else
        {
            log_error("hdl_parser", "line {}: unexpected line in architecture header: '{}'", line.number, line.text);
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

    log_debug("hdl_parser", "parsed architecture header of '{}'.", e.name);
    return true;
}

bool hdl_parser_vhdl::parse_architecture_body(entity& e)
{
    std::vector<file_line> instance_lines;

    bool in_instance = false;

    for (const auto& line : e.definition.architecture_body)
    {
        // new instance found
        if (!in_instance && line.text.find(':') != std::string::npos)
        {
            in_instance = true;
            instance_lines.push_back(line);
        }
        // end of instance
        else if (in_instance && line.text == ");")
        {
            if (!parse_instance(e, instance_lines))
            {
                return false;
            }
            instance_lines.clear();
            in_instance = false;
        }
        // add line to instance
        else if (in_instance)
        {
            instance_lines.push_back(line);
        }
        // not in instance -> has to be a direct assignment
        else
        {
            auto arrow_pos = line.text.find("<=");
            if (arrow_pos != std::string::npos)
            {
                auto signal     = core_utils::rtrim(line.text.substr(0, arrow_pos));
                auto assignment = line.text.substr(arrow_pos + 2);
                assignment.pop_back();

                for (const auto& [name, value] : get_port_assignments(signal, core_utils::trim(assignment)))
                {
                    e.direct_assignments[name] = value;
                }
            }
            else
            {
                log_error("hdl_parser", "line {}: unexpected line in architecture body: '{}'", line.number, line.text);
            }
        }
    }

    log_debug("hdl_parser", "parsed architecture body of '{}'.", e.name);
    return true;
}

bool hdl_parser_vhdl::parse_instance(entity& e, const std::vector<file_line>& lines)
{
    instance inst;

    // traverse content
    enum
    {
        NONE,
        IN_GENERIC_MAP,
        IN_PORT_MAP
    } state = NONE;

    // extract name and component
    auto pos  = lines[0].text.find(':');
    inst.name = core_utils::trim(lines[0].text.substr(0, pos));
    inst.type = core_utils::trim(lines[0].text.substr(pos + 1));

    {
        auto map_pos = core_utils::to_lower(inst.type).find(" port map");
        if (map_pos != std::string::npos)
        {
            inst.type = inst.type.substr(0, map_pos);
            state     = IN_PORT_MAP;
        }
        map_pos = core_utils::to_lower(inst.type).find(" generic map");
        if (map_pos != std::string::npos)
        {
            inst.type = inst.type.substr(0, map_pos);
            state     = IN_GENERIC_MAP;
        }
    }

    // remove prefix from component
    if (core_utils::starts_with(inst.type, "entity ", true))
    {
        pos = inst.type.find('.');
        if (pos == std::string::npos)
        {
            inst.type = inst.type.substr(inst.type.find(' ') + 1);
        }
        else
        {
            inst.type = inst.type.substr(pos + 1);
        }
    }
    else if (core_utils::starts_with(inst.type, "component ", true))
    {
        inst.type = inst.type.substr(inst.type.find(' ') + 1);
    }
    else
    {
        auto low_type      = core_utils::to_lower(inst.type);
        std::string prefix = "";

        // find longest matching prefix
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

    // start from second line
    for (u32 i = 1; i < lines.size(); ++i)
    {
        auto line = lines[i];
        if (core_utils::starts_with(line.text, "generic map", true))
        {
            state = IN_GENERIC_MAP;
            continue;
        }
        else if (core_utils::starts_with(line.text, "port map", true))
        {
            state = IN_PORT_MAP;
            continue;
        }
        else if (core_utils::starts_with(line.text, ")"))
        {
            state = NONE;
            continue;
        }

        if (state == IN_GENERIC_MAP)
        {
            auto key   = core_utils::rtrim(line.text.substr(0, line.text.find("=>")));
            auto value = core_utils::ltrim(line.text.substr(line.text.find("=>") + 2));
            if (value.back() == ',')
            {
                value.pop_back();
            }
            inst.generics.emplace_back(key, value);
        }
        else if (state == IN_PORT_MAP)
        {
            auto key   = core_utils::rtrim(line.text.substr(0, line.text.find("=>")));
            auto value = core_utils::ltrim(line.text.substr(line.text.find("=>") + 2));
            if (value.back() == ',')
            {
                value.pop_back();
            }
            for (const auto& [a, b] : get_port_assignments(key, value))
            {
                inst.ports.emplace_back(a, b);
            }
        }
    }

    e.instances.push_back(inst);

    log_debug("hdl_parser", "parsed instance '{}' of entity '{}'.", inst.name, e.name);
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

    // for the top module, generate global i/o signals for all ports

    std::map<std::string, std::function<bool(std::shared_ptr<net> const)>> port_dir_function = {
        {"in", [](std::shared_ptr<net> const net) { return net->mark_global_input_net(); }},
        {"out", [](std::shared_ptr<net> const net) { return net->mark_global_output_net(); }},
    };

    std::map<std::string, std::string> top_assignments;

    for (const auto& [name, direction] : top_entity.ports)
    {
        if (port_dir_function.find(direction) == port_dir_function.end())
        {
            log_error("hdl_parser", "entity {}, line {}+ : direction '{}' unkown", name, top_entity.line_number, direction);
            return false;
        }

        auto new_net = m_netlist->create_net(name);
        if (new_net == nullptr)
        {
            log_error("hdl_parser", "could not create new net '{}'", name);
            return false;
        }
        m_net_by_name[new_net->get_name()] = new_net;
        if (!port_dir_function[direction](new_net))
        {
            log_error("hdl_parser", "could not mark net '{}' as global input/output", name);
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
                    log_error("hdl_parser", "signal assignment \"{} => {}\" of instance {} is invalid", pin, signal, inst.name);
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
            if (global_vcc_gate_types.find(inst.type) != global_vcc_gate_types.end() && !new_gate->mark_global_vcc_gate())
            {
                return nullptr;
            }
            if (global_gnd_gate_types.find(inst.type) != global_gnd_gate_types.end() && !new_gate->mark_global_gnd_gate())
            {
                return nullptr;
            }

            // cache pin types
            auto input_pin_types  = new_gate->get_input_pin_types();
            auto output_pin_types = new_gate->get_output_pin_types();

            // check for port
            for (auto [pin_it, net_name] : inst.ports)
            {
                std::string pin = pin_it; // copy to be able to overwrite

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

std::vector<std::string> hdl_parser_vhdl::get_vector_signals(const std::string& name, const std::string& type)
{
    // if there is no range given, the signal is just the name
    if (type.find('(') == std::string::npos)
    {
        return {name};
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

std::map<std::string, std::string> hdl_parser_vhdl::get_port_assignments(const std::string& port, const std::string& assignment)
{
    // a port may be
    // (1): a => ...
    // (2): a(x) => ...
    //      a(x, y, z) => ...
    // (3): a(x to y) => B"01010101..."
    // (4): a(x to y) => b(s to t)

    // case (1)
    auto open_pos = port.find('(');
    if (open_pos == std::string::npos)
    {
        return {std::make_pair(port, assignment)};
    }

    // case (2)

    // extract left bounds and the part of the line which comes before the left bounds
    auto left_part   = port.substr(0, open_pos);
    auto left_bounds = port.substr(open_pos + 1);
    left_bounds      = core_utils::trim(left_bounds.substr(0, left_bounds.find(')')));

    if (core_utils::to_lower(left_bounds).find("to") == std::string::npos)
    {
        return {std::make_pair(port, assignment)};
    }

    auto right_part = assignment;

    // check for case (3) or (4)
    if (right_part.find('(') == std::string::npos)
    {
        // case (3)
        // right part has to be a bitvector
        if (!core_utils::starts_with(right_part, "B\"", true))
        {
            log_error("hdl_parser", "assignment of anything but a binary bitvector is not supported ('{} => {}')", port, assignment);
            return {};
        }

        // extract right values
        auto right_values = right_part.substr(right_part.find('\"') + 1);
        right_values      = right_values.substr(0, right_values.find('\"'));

        // extract counting direction and min/max of left bounds
        int left_dir = -1;
        if (core_utils::to_lower(left_bounds).find(" downto ") == std::string::npos)
        {
            left_dir = 1;
        }
        int left_start = std::stoi(left_bounds.substr(0, left_bounds.find(' ')));
        int left_end   = std::stoi(left_bounds.substr(left_bounds.find_last_of(' ') + 1));

        // assemble assignment strings
        std::map<std::string, std::string> result;
        int l   = left_start;
        int cnt = 0;
        while (true)
        {
            result[left_part + "(" + std::to_string(l) + ")"] = std::string("'") + right_values[cnt] + "'";
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
        if (core_utils::to_lower(left_bounds).find(" downto ") == std::string::npos)
        {
            left_dir = 1;
        }

        int right_dir = -1;
        if (core_utils::to_lower(right_bounds).find(" downto ") == std::string::npos)
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
            log_error("hdl_parser", "ranges on port assignment '{} => {}' do not match.", port, assignment);
            return {};
        }

        // assemble assignment strings
        std::map<std::string, std::string> result;
        int l = left_start;
        int r = right_start;
        while (dist >= 0)
        {
            result[left_part + "(" + std::to_string(l) + ")"] = right_part + "(" + std::to_string(r) + ")";
            l += left_dir;
            r += right_dir;
            dist--;
        }

        return result;
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
