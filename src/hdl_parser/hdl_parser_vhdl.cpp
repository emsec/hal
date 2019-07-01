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
        else if (core_utils::starts_with(line, "library ") || core_utils::starts_with(line, "use "))
        {
            libraries.push_back(file_line{line_number, line});
        }
        else if (core_utils::starts_with(line, "entity "))
        {
            if (!current_entity.name.empty())
            {
                m_entities[current_entity.name] = current_entity;
            }
            state = IN_ENTITY_HEADER;
            current_entity.name.clear();
            current_entity.definition.entity_header.clear();
            current_entity.definition.architecture_header.clear();
            current_entity.definition.architecture_body.clear();
            current_entity.name        = core_utils::split(line, ' ')[1];
            current_entity.line_number = line_number;
        }
        else
        {
            if (core_utils::starts_with(line, "architecture "))
            {
                state = IN_ARCH_HEADER;
                continue;
            }
            else if (state == IN_ARCH_HEADER && core_utils::starts_with(line, "component "))
            {
                state = IN_COMPONENT;
                // no continue
            }
            else if (state == IN_COMPONENT && core_utils::starts_with(line, "end component "))
            {
                components.push_back(current_component);
                current_component.clear();
                state = IN_ARCH_HEADER;
                continue;
            }
            else if (state == IN_ARCH_HEADER && line == "begin")
            {
                state = IN_ARCH_BODY;
                continue;
            }
            else if (core_utils::starts_with(line, "end "))
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
            else if (state == IN_COMPONENT)
            {
                current_component.push_back(file_line{line_number, line});
            }
            else if (state == IN_ARCH_BODY)
            {
                current_entity.definition.architecture_body.push_back(file_line{line_number, line});
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

    // parse logical parts
    if (!this->parse_header(header))
    {
        return nullptr;
    }
    if (!this->parse_libraries(libraries))
    {
        return nullptr;
    }
    if (!this->parse_components(components))
    {
        return nullptr;
    }

    m_netlist->set_design_name(current_entity.name);

    for (auto& it : m_entities)
    {
        if (!this->parse_entity(it.second))
        {
            return nullptr;
        }
    }

    if (!build_netlist(current_entity.name))
    {
        return nullptr;
    }

    auto modules_copy = m_netlist->get_modules();
    for (const auto& m : modules_copy)
    {
        if (m->get_gates().empty())
        {
            m_netlist->delete_module(m);
        }
    }

    // add global gnd gate if required by any instance
    if (m_net.find("'0'") != m_net.end())
    {
        auto gnd_type   = *(m_netlist->get_gate_library()->get_global_gnd_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pin_types(gnd_type).at(0);
        auto gnd        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");
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
        auto vcc_type   = *(m_netlist->get_gate_library()->get_global_vcc_gate_types()->begin());
        auto output_pin = m_netlist->get_output_pin_types(vcc_type).at(0);
        auto vcc        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");
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

bool hdl_parser_vhdl::parse_header(const std::vector<file_line>& header)
{
    // header contains device name and library includes
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
    // header contains device name and library includes
    for (const auto& line : libs)
    {
        if (core_utils::starts_with(line.text, "use "))
        {
            // remove the .all in the end
            auto lib = core_utils::to_lower(line.text.substr(line.text.find(' ')));
            lib      = core_utils::trim(lib.substr(0, lib.find("all;")));
            if (std::get<1>(m_libraries.insert(lib)))
            {
                // log_info("hdl_parser", "added lib '{}'.", lib);
            }
        }
    }
    log_debug("hdl_parser", "parsed header.");
    return true;
}

bool hdl_parser_vhdl::parse_components(const std::vector<std::vector<file_line>>& components)
{
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

                // log_info("hdl_parser", "component '{}'.", new_component);
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

                        // log_info("hdl_parser", "input '{}'", signal);
                    }
                    else if (direction == "out")
                    {
                        lib->get_output_pin_types()->insert(signal);
                        (*lib->get_gate_type_map_to_output_pin_types())[new_component].push_back(signal);
                        // log_info("hdl_parser", "output '{}'", signal);
                    }
                    else if (direction == "inout")
                    {
                        lib->get_inout_pin_types()->insert(signal);
                        (*lib->get_gate_type_map_to_inout_pin_types())[new_component].push_back(signal);
                        // log_info("hdl_parser", "inout '{}'", signal);
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }
    }
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
    else if (left_parts.size() == 4 && left_parts[2] == "of")
    {
        std::string value = right.substr(right.find(" is ") + 4);
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
        // log_info("hdl_parser", "attribute '{}' '{} '{}' '{}'", left_parts[3], left_parts[1], m_attribute_types[left_parts[1]], value);
    }
    else
    {
        log_error("hdl_parser", "malformed attribute defintion '{}'", line.text);
    }
}

bool hdl_parser_vhdl::parse_entity(entity& e)
{
    // log_info("hdl_parser", "parsing entity '{}'.", e.name);

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

    // log_info("hdl_parser", "\n");

    return true;
}

bool hdl_parser_vhdl::parse_entity_header(entity& e)
{
    bool in_port_def = false;

    for (const auto& line : e.definition.entity_header)
    {
        // line starts with "entity" -> extract name
        if (line.text == "port (")
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
            auto direction = config.substr(0, config.find(' '));
            auto type      = config.substr(config.find(' ') + 1);

            // add all signals of that port
            for (const auto signal : get_vector_signals(name, type))
            {
                e.ports.emplace_back(signal, direction);
                e.expanded_signal_names[name].push_back(signal);
                // log_info("hdl_parser", "port '{}' ({})", signal, direction);
            }
        }
        else if (!in_port_def && core_utils::starts_with(line.text, "attribute "))
        {
            parse_attribute(e.entity_attributes, line);
        }
    }

    log_debug("hdl_parser", "parsed entity.");
    return true;
}

bool hdl_parser_vhdl::parse_architecture_header(entity& e)
{
    // architecture contains all signals and attributes

    for (const auto& line : e.definition.architecture_header)
    {
        if (core_utils::starts_with(line.text, "signal "))
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
                // log_info("hdl_parser", "signal '{}'", signal);
            }
        }
        else if (core_utils::starts_with(line.text, "attribute "))
        {
            if (line.text.find("signal is \"") != std::string::npos)
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
            log_error("hdl_parser", "line {}: architecture is not expected to contain '{}'", line.number, line.text);
            return false;
        }
    }

    for (const auto& [original, splits] : e.expanded_signal_names)
    {
        auto it = e.signal_attributes.find(original);
        if (it != e.signal_attributes.end())
        {
            // log_info("hdl_parser", "splitting attribute of {}", original);
            for (const auto& split : splits)
            {
                e.signal_attributes[split] = it->second;
                // log_info("hdl_parser", "  to {}", split);
            }
        }
    }

    return true;
}

bool hdl_parser_vhdl::parse_architecture_body(entity& e)
{
    std::vector<file_line> instance_lines;

    bool in_instance = false;

    for (const auto& line : e.definition.architecture_body)
    {
        if (!in_instance && line.text.find(':') != std::string::npos)
        {
            in_instance = true;
            instance_lines.push_back(line);
        }
        else if (in_instance && line.text == ");")
        {
            if (!parse_instance(e, instance_lines))
            {
                return false;
            }
            instance_lines.clear();
            in_instance = false;
        }
        else if (in_instance)
        {
            instance_lines.push_back(line);
        }
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
                    // log_info("hdl_parser", "direct assignment '{}' <= '{}' (l {})", name, value, line.number);
                    e.direct_assignments[name] = value;
                }
            }
            else
            {
                log_error("hdl_parser", "unexpected line in architecture body: '{}' (l {})", line.text, line.number);
            }
        }
    }

    return true;
}

bool hdl_parser_vhdl::parse_instance(entity& e, const std::vector<file_line>& lines)
{
    instance inst;

    // extract name and component
    auto pos  = lines[0].text.find(':');
    inst.name = core_utils::trim(lines[0].text.substr(0, pos));
    inst.type = core_utils::trim(lines[0].text.substr(pos + 1));

    // remove prefix from component
    if (core_utils::starts_with(inst.type, "entity "))
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
        m_num_instances[inst.type]++;
    }
    else if (core_utils::starts_with(inst.type, "component "))
    {
        inst.type = inst.type.substr(inst.type.find(' ') + 1);
    }
    else
    {
        auto low_type      = core_utils::to_lower(inst.type);
        std::string prefix = "";
        for (const auto& lib : m_libraries)
        {
            if (lib.size() > prefix.size() && prefix.size() < lib.size() && core_utils::starts_with(low_type, lib))
            {
                prefix = lib;
            }
        }
        if (!prefix.empty())
        {
            inst.type = inst.type.substr(prefix.size());
        }
    }

    // log_info("hdl_parser", "instance '{}' ({})", inst.name, inst.type);

    // traverse content
    enum
    {
        NONE,
        IN_GENERICS_MAP,
        IN_PORT_MAP
    } state;

    for (u32 i = 1; i < lines.size(); ++i)
    {
        auto& line = lines[i];
        if (core_utils::starts_with(line.text, "generic map"))
        {
            state = IN_GENERICS_MAP;
            continue;
        }
        else if (core_utils::starts_with(line.text, "port map"))
        {
            state = IN_PORT_MAP;
            continue;
        }
        else if (core_utils::starts_with(line.text, ")"))
        {
            state = NONE;
            continue;
        }

        if (state == IN_GENERICS_MAP)
        {
            auto key   = core_utils::rtrim(line.text.substr(0, line.text.find("=>")));
            auto value = core_utils::ltrim(line.text.substr(line.text.find("=>") + 2));
            if (value.back() == ',')
            {
                value.pop_back();
            }
            inst.generics.emplace_back(key, value);
            // log_info("hdl_parser", "generic map '{}' => '{}'", key, value);
        }
        else if (state == IN_PORT_MAP)
        {
            auto key   = core_utils::rtrim(line.text.substr(0, line.text.find("=>")));
            auto value = core_utils::ltrim(line.text.substr(line.text.find("=>") + 2));
            if (value.back() == ',')
            {
                value.pop_back();
            }
            inst.ports.emplace_back(key, value);
            // log_info("hdl_parser", "port map '{}' => '{}'", key, value);
        }
    }

    e.instances.push_back(inst);

    log_debug("hdl_parser", "parsed instance.");
    return true;
}

// ###########################################################################
// ###########################################################################
// ###########################################################################

bool hdl_parser_vhdl::build_netlist(const std::string& top_module)
{
    log_info("hdl_parser", "builing netlist\n");

    auto& e = m_entities[top_module];

    std::map<std::string, std::function<bool(std::shared_ptr<net> const)>> port_dir_function = {
        {"in", [](std::shared_ptr<net> const net) { return net->mark_global_input_net(); }},
        {"out", [](std::shared_ptr<net> const net) { return net->mark_global_output_net(); }},
        {"inout", [](std::shared_ptr<net> const net) { return net->mark_global_inout_net(); }},
    };

    for (const auto& [name, direction] : e.ports)
    {
        if (port_dir_function.find(direction) == port_dir_function.end())
        {
            log_error("hdl_parser", "entity {}, line {}+ : direction '{}' unkown", name, e.line_number, direction);
            return false;
        }

        // log_info("hdl_parser", "global {} net {}", direction, name);

        auto new_net = m_netlist->create_net(name);
        if (new_net == nullptr)
        {
            return false;
        }
        m_net[name] = new_net;
        if (!port_dir_function[direction](new_net))
        {
            return false;
        }
    }

    if (instantiate(e, nullptr, {}) == nullptr)
    {
        return false;
    }

    return true;
}

static std::string spacing = "";
#define DEBUG_LOG(x, ...) /*log_info("hdl_parser", "{}" x, spacing, __VA_ARGS__); */

std::shared_ptr<module> hdl_parser_vhdl::instantiate(const entity& e, std::shared_ptr<module> parent, const std::map<std::string, std::string>& parent_module_assignments)
{
    spacing += "|   ";
    DEBUG_LOG("instantiating {}", e.name);

    std::string prefix = "";

    std::shared_ptr<module> module;
    if (parent == nullptr)
    {
        module = m_netlist->get_top_module();
        module->set_name(e.name);
        DEBUG_LOG("{}", "using topmodule");
    }
    else
    {
        module = m_netlist->create_module(e.name, parent);
        DEBUG_LOG("{}", "new module");
        prefix = e.name + "_inst_" + std::to_string(++m_instance_count[e.name]) + "_";
    }

    {
        auto attribute_it = e.entity_attributes.find(e.name);
        if (attribute_it != e.entity_attributes.end())
        {
            for (const auto& attr : attribute_it->second)
            {
                module->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr));
                DEBUG_LOG("entity has attribute {}", std::get<0>(attr));
            }
        }
    }

    for (const auto& name : e.signals)
    {
        DEBUG_LOG("internal signal {}", prefix + name);
        auto new_net = m_netlist->create_net(prefix + name);
        if (new_net == nullptr)
        {
            return nullptr;
        }
        m_net[prefix + name] = new_net;

        {
            auto attribute_it = e.signal_attributes.find(name);
            if (attribute_it != e.signal_attributes.end())
            {
                for (const auto& attr : attribute_it->second)
                {
                    new_net->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr));
                    DEBUG_LOG("signal has attribute {}", std::get<0>(attr));
                }
            }
        }
    }

    for (const auto& [name, assignment] : parent_module_assignments)
    {
        auto attribute_it = e.signal_attributes.find(name);
        if (attribute_it != e.signal_attributes.end())
        {
            for (const auto& attr : attribute_it->second)
            {
                m_net[assignment]->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr));
                DEBUG_LOG("signal has attribute {}", std::get<0>(attr));
            }
        }
    }

    // instances contain all components with port assignments
    auto global_vcc_gate_types = m_netlist->get_gate_library()->get_global_vcc_gate_types();
    auto global_gnd_gate_types = m_netlist->get_gate_library()->get_global_gnd_gate_types();

    for (const auto& inst : e.instances)
    {
        DEBUG_LOG("processing instance {}", inst.name);

        data_container* container;

        std::map<std::string, std::string> local_assignments;

        // check for port
        for (const auto& [port, assignment] : inst.ports)
        {
            auto new_assignments = get_port_assignments(port, assignment);
            local_assignments.insert(new_assignments.begin(), new_assignments.end());
        }

        local_assignments.insert(e.direct_assignments.begin(), e.direct_assignments.end());

        for (auto& it : local_assignments)
        {
            auto it2 = parent_module_assignments.find(it.second);
            if (it2 != parent_module_assignments.end())
            {
                it.second = it2->second;
            }
            else if (std::find(e.signals.begin(), e.signals.end(), it.second) != e.signals.end())
            {
                it.second = prefix + it.second;
            }
        }

        for (const auto& it : local_assignments)
        {
            DEBUG_LOG("assign {} => {}", it.first, it.second);
        }

        auto entity_it = m_entities.find(inst.type);
        if (entity_it != m_entities.end())
        {
            container = instantiate(entity_it->second, module, local_assignments).get();
            if (container == nullptr)
            {
                return nullptr;
            }
        }
        else
        {
            DEBUG_LOG("new gate {} of type {}", prefix + inst.name, inst.type);

            auto new_gate = m_netlist->create_gate(inst.type, prefix + inst.name);
            if (new_gate == nullptr)
            {
                return nullptr;
            }
            module->assign_gate(new_gate);

            container = new_gate.get();

            // if gate id a global type, register it as such
            if (global_vcc_gate_types->find(inst.type) != global_vcc_gate_types->end() && !new_gate->mark_global_vcc_gate())
            {
                return nullptr;
            }
            if (global_gnd_gate_types->find(inst.type) != global_gnd_gate_types->end() && !new_gate->mark_global_gnd_gate())
            {
                return nullptr;
            }

            auto input_pin_types  = new_gate->get_input_pin_types();
            auto output_pin_types = new_gate->get_output_pin_types();
            auto inout_pin_types  = new_gate->get_inout_pin_types();

            // check for port
            for (const auto& [port, assignment] : inst.ports)
            {
                // split assignments of the current port (in case of e.g. "a(3 to 6) => b(7 to 10)")
                for (auto [pin, net_name] : get_port_assignments(port, assignment))
                {
                    std::shared_ptr<net> current_net = nullptr;

                    {
                        auto it = local_assignments.find(pin);
                        if (it != local_assignments.end())
                        {
                            net_name = it->second;
                        }
                    }

                    if (std::find(e.signals.begin(), e.signals.end(), net_name) != e.signals.end())
                    {
                        net_name = prefix + net_name;
                    }

                    // search for existing net by name, otherwise add it
                    auto it = m_net.find(net_name);
                    if (it == m_net.end())
                    {
                        DEBUG_LOG("new net '{}'", net_name);
                        current_net = m_netlist->create_net(net_name);
                        if (current_net == nullptr)
                        {
                            return nullptr;
                        }
                        m_net[net_name] = current_net;

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
                    bool is_inout  = std::find(inout_pin_types.begin(), inout_pin_types.end(), pin) != inout_pin_types.end();
                    bool is_input  = is_inout || std::find(input_pin_types.begin(), input_pin_types.end(), pin) != input_pin_types.end();
                    bool is_output = is_inout || std::find(output_pin_types.begin(), output_pin_types.end(), pin) != output_pin_types.end();

                    if (!is_input && !is_output)
                    {
                        log_error("hdl_parser", "undefined pin '{}' for '{}' ({})", pin, new_gate->get_name(), new_gate->get_type());
                        return nullptr;
                    }

                    if (is_input && !current_net->add_dst(new_gate, pin))
                    {
                        return nullptr;
                    }
                    if (is_output && !current_net->set_src(new_gate, pin))
                    {
                        return nullptr;
                    }
                }
            }
        }

        {
            auto attribute_it = e.instance_attributes.find(inst.name);
            if (attribute_it != e.instance_attributes.end())
            {
                for (const auto& attr : attribute_it->second)
                {
                    container->set_data("vhdl_attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr));
                    DEBUG_LOG("instance has attribute {}", std::get<0>(attr));
                }
            }
        }

        // check for generic
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
                log_error("hdl_parser", "cannot identify data type of generic map value '{}' in instance '{}'", value, inst.name);
                return nullptr;
            }

            // store generic information on gate
            if (!container->set_data("generic", name, data_type, value))
            {
                return nullptr;
            }
        }
    }

    spacing = spacing.substr(4);
    return module;
}

// ###########################################################################
// ###########################################################################
// ###########################################################################

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

    if (left_bounds.find("to") == std::string::npos)
    {
        return {std::make_pair(port, assignment)};
    }

    auto right_part = assignment;

    // check for case (3) or (4)
    if (right_part.find('(') == std::string::npos)
    {
        // case (3)
        // right part has to be a bitvector
        if (!core_utils::starts_with(right_part, "B\""))
        {
            log_error("hdl_parser", "assignment of anything but a binary bitvector is not supported ('{} => {}')", port, assignment);
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
