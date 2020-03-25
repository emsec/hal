#include "netlist/hdl_parser/hdl_parser_vhdl.h"

#include "core/log.h"

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

    // tokenize file
    if (!tokenize())
    {
        return nullptr;
    }

    // parse tokens into intermediate format
    try
    {
        if (!parse_tokens())
        {
            return nullptr;
        }
    }
    catch (token_stream::token_stream_exception& e)
    {
        if (e.line_number != (u32)-1)
        {
            log_error("hdl_parser", "{} near line {}.", e.message, e.line_number);
        }
        else
        {
            log_error("hdl_parser", "{}.", e.message);
        }
        return nullptr;
    }

    // create const 0 and const 1 net, will be removed if unused
    auto zero_net = m_netlist->create_net("'0'");
    if (zero_net == nullptr)
    {
        log_error("hdl_parser", "could not instantiate GND net");
        return nullptr;
    }
    m_net_by_name[zero_net->get_name()] = zero_net;

    auto one_net = m_netlist->create_net("'1'");
    if (one_net == nullptr)
    {
        log_error("hdl_parser", "could not instantiate VCC net");
        return nullptr;
    }
    m_net_by_name[one_net->get_name()] = one_net;

    auto z_net = m_netlist->create_net("'Z'");
    if (z_net == nullptr)
    {
        log_error("hdl_parser", "could not instantiate Z net");
        return nullptr;
    }
    m_net_by_name[z_net->get_name()] = z_net;

    // build the netlist from the intermediate format
    // the last entity in the file is considered the top module
    if (!build_netlist(m_last_entity))
    {
        log_error("hdl_parser", "could not build netlist from parsed data");
        return nullptr;
    }

    // add global gnd gate if required by any instance
    if (!zero_net->get_destinations().empty())
    {
        auto gnd_type   = m_netlist->get_gate_library()->get_gnd_gate_types().begin()->second;
        auto output_pin = gnd_type->get_output_pins().at(0);
        auto gnd        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");
        if (!m_netlist->mark_gnd_gate(gnd))
        {
            return nullptr;
        }
        auto gnd_net = m_net_by_name.find("'0'")->second;
        if (!gnd_net->add_source(gnd, output_pin))
        {
            return nullptr;
        }
    }
    else
    {
        m_netlist->delete_net(zero_net);
    }

    // add global vcc gate if required by any instance
    if (!one_net->get_destinations().empty())
    {
        auto vcc_type   = m_netlist->get_gate_library()->get_vcc_gate_types().begin()->second;
        auto output_pin = vcc_type->get_output_pins().at(0);
        auto vcc        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");
        if (!m_netlist->mark_vcc_gate(vcc))
        {
            return nullptr;
        }
        auto vcc_net = m_net_by_name.find("'1'")->second;
        if (!vcc_net->add_source(vcc, output_pin))
        {
            return nullptr;
        }
    }
    else
    {
        m_netlist->delete_net(one_net);
    }

    for (const auto& net : m_netlist->get_nets())
    {
        bool no_source      = net->get_source().get_gate() == nullptr && !net->is_global_input_net();
        bool no_destination = net->get_num_of_destinations() == 0 && !net->is_global_output_net();
        if (no_source && no_destination)
        {
            if (net != z_net && net != one_net && net != zero_net)
            {
                log_warning("hdl_parser", "net '{}' has neither a source nor destinations and is thus removed", net->get_name());
            }
            m_netlist->delete_net(net);
        }
    }

    return m_netlist;
}

static bool is_digits(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);    // C++11
}

bool hdl_parser_vhdl::tokenize()
{
    std::vector<token> tmp_tokens;
    std::string delimiters = ",(): ;=><";
    std::string current_token;
    u32 line_number = 0;

    std::string line;
    bool in_string = false;
    bool escaped   = false;
    while (std::getline(m_fs, line))
    {
        line_number++;
        if (line.find("--") != std::string::npos)
        {
            line = line.substr(0, line.find("--"));
        }
        for (char c : core_utils::trim(line))
        {
            if (c == '\\')
            {
                escaped = !escaped;
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

            if (delimiters.find(c) == std::string::npos || escaped || in_string)
            {
                current_token += c;
            }
            else
            {
                if (!current_token.empty())
                {
                    if (tmp_tokens.size() > 1 && is_digits(tmp_tokens.at(tmp_tokens.size() - 2)) && tmp_tokens.at(tmp_tokens.size() - 1) == "." && is_digits(current_token))
                    {
                        tmp_tokens.pop_back();
                        tmp_tokens.back() += "." + current_token;
                    }
                    else
                    {
                        tmp_tokens.emplace_back(line_number, current_token, false);
                    }
                    current_token.clear();
                }
                if (c == '=' && tmp_tokens.at(tmp_tokens.size() - 1) == "<")
                {
                    tmp_tokens.at(tmp_tokens.size() - 1) = "<=";
                }
                else if (c == '=' && tmp_tokens.at(tmp_tokens.size() - 1) == ":")
                {
                    tmp_tokens.at(tmp_tokens.size() - 1) = ":=";
                }
                else if (c == '>' && tmp_tokens.at(tmp_tokens.size() - 1) == "=")
                {
                    tmp_tokens.at(tmp_tokens.size() - 1) = "=>";
                }
                else if (!std::isspace(c))
                {
                    tmp_tokens.emplace_back(line_number, std::string(1, c), false);
                }
            }
        }
        if (!current_token.empty())
        {
            tmp_tokens.emplace_back(line_number, current_token, false);
            current_token.clear();
        }
    }
    m_token_stream = token_stream(tmp_tokens, {"("}, {")"});
    return true;
}

bool hdl_parser_vhdl::parse_tokens()
{
    std::string last_entity;

    while (m_token_stream.remaining() > 0)
    {
        if (m_token_stream.peek() == "library" || m_token_stream.peek() == "use")
        {
            if (!parse_library())
            {
                return false;
            }
        }
        else if (m_token_stream.peek() == "entity")
        {
            if (!parse_entity_definiton())
            {
                return false;
            }
        }
        else if (m_token_stream.peek() == "architecture")
        {
            if (!parse_architecture())
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in global scope in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
            return false;
        }
    }

    return true;
}

bool hdl_parser_vhdl::parse_library()
{
    if (m_token_stream.peek() == "use")
    {
        m_token_stream.consume("use", true);
        auto lib = m_token_stream.consume().string;
        m_token_stream.consume(";", true);

        // remove specific import like ".all" but keep the "."
        lib = core_utils::trim(lib.substr(0, lib.rfind(".") + 1));
        m_libraries.insert(core_utils::to_lower(lib));
    }
    else
    {
        m_token_stream.consume_until(";");
        m_token_stream.consume(";", true);
    }
    return true;
}

bool hdl_parser_vhdl::parse_entity_definiton()
{
    entity e;
    e.line_number = m_token_stream.peek().number;
    m_token_stream.consume("entity", true);
    e.name = m_token_stream.consume();
    m_token_stream.consume("is", true);
    while (m_token_stream.peek() != "end")
    {
        if (m_token_stream.peek() == "generic")
        {
            //TODO handle default values for generics
            m_token_stream.consume_until(";");
            m_token_stream.consume(";", true);
        }
        else if (m_token_stream.peek() == "port")
        {
            if (!parse_port_definiton(e))
            {
                return false;
            }
        }
        else if (m_token_stream.peek() == "attribute")
        {
            if (!parse_attribute(e.entity_attributes))
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
            return false;
        }
    }
    m_token_stream.consume("end", true);
    m_token_stream.consume();
    m_token_stream.consume(";", true);

    if (!e.name.empty())
    {
        m_entities[e.name] = e;
        m_last_entity                            = e.name;
    }

    return true;
}

bool hdl_parser_vhdl::parse_port_definiton(entity& e)
{
    m_token_stream.consume("port", true);
    m_token_stream.consume("(", true);
    auto ports = m_token_stream.extract_until(")");

    while (ports.remaining() > 0)
    {
        auto port_names = ports.extract_until(":");
        ports.consume(":", true);

        auto direction = ports.consume().string;
        auto type      = ports.extract_until(";");
        ports.consume(";", ports.remaining() > 0);    // last entry has no semicolon, so no throw in that case

        while (port_names.remaining() > 0)
        {
            auto name = port_names.consume();
            port_names.consume(",", port_names.remaining() > 0);
            for (const auto signal : get_vector_signals(name, type))
            {
                e.ports.emplace_back(signal, direction);
                e.expanded_signal_names[name].push_back(signal);
                e.type_of_signal[name] = type;
            }
        }
    }
    m_token_stream.consume(")", true);
    m_token_stream.consume(";", true);
    return true;
}

bool hdl_parser_vhdl::parse_architecture()
{
    m_token_stream.consume("architecture", true);
    m_token_stream.consume();
    m_token_stream.consume("of", true);
    auto& e = m_entities[m_token_stream.consume().string];
    m_token_stream.consume("is", true);
    return parse_architecture_header(e) && parse_architecture_body(e);
}

bool hdl_parser_vhdl::parse_architecture_header(entity& e)
{
    while (m_token_stream.peek() != "begin")
    {
        if (m_token_stream.peek() == "signal")
        {
            m_token_stream.consume("signal", true);
            auto names = m_token_stream.extract_until(":");
            m_token_stream.consume(":", true);
            auto type = m_token_stream.extract_until(";");
            m_token_stream.consume(";", true);

            // add all (sub-)signals
            while (names.remaining() > 0)
            {
                auto name = names.consume().string;
                names.consume(",", names.remaining() > 0);
                for (const auto signal : get_vector_signals(name, type))
                {
                    e.expanded_signal_names[name].push_back(signal);
                    e.signals.push_back(signal);
                    e.type_of_signal[name] = type;
                }
            }
        }
        else if (m_token_stream.peek() == "component")
        {
            m_token_stream.consume("component", true);
            auto component_name = m_token_stream.consume().string;
            // components are ignored
            m_token_stream.consume_until("end");
            m_token_stream.consume("end", true);
            m_token_stream.consume("component", true);
            if (m_token_stream.peek() != ";")
            {
                m_token_stream.consume(component_name, true);    // optional repetition of component name
            }
            m_token_stream.consume(";", true);
        }
        else if (m_token_stream.peek() == "attribute")
        {
            auto end_pos    = m_token_stream.find_next(";");
            auto signal_pos = m_token_stream.find_next("signal", end_pos);

            if (signal_pos < end_pos && m_token_stream.at(signal_pos + 1) == "is")
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
            log_error("hdl_parser", "unexpected token '{}' in architecture header in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
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
    m_token_stream.consume("begin", true);
    while (m_token_stream.peek() != "end")
    {
        // new instance found
        if (m_token_stream.peek(1) == ":")
        {
            if (!parse_instance(e))
            {
                return false;
            }
        }
        // not in instance -> has to be a direct assignment
        else if (m_token_stream.find_next("<=") < m_token_stream.find_next(";"))
        {
            auto lhs = m_token_stream.extract_until("<=");
            m_token_stream.consume("<=", true);
            auto rhs = m_token_stream.extract_until(";");
            m_token_stream.consume(";", true);

            for (const auto& [name, value] : get_assignments(e, lhs, rhs))
            {
                e.direct_assignments[name] = value;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in architecture body in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
            return false;
        }
    }

    m_token_stream.consume("end", true);
    m_token_stream.consume();
    m_token_stream.consume(";", true);
    return true;
}

bool hdl_parser_vhdl::parse_attribute(case_insensitive_string_map<std::set<std::tuple<std::string, std::string, std::string>>>& mapping)
{
    u32 line_number = m_token_stream.peek().number;
    m_token_stream.consume("attribute", true);
    auto attr_type = m_token_stream.consume().string;
    if (m_token_stream.peek() == ":")
    {
        m_token_stream.consume(":", true);
        m_attribute_types[attr_type] = m_token_stream.join_until(";", " ");
        m_token_stream.consume(";", true);
    }
    else if (m_token_stream.peek() == "of" && m_token_stream.peek(2) == ":")
    {
        m_token_stream.consume("of", true);
        auto attr_target = m_token_stream.consume();
        m_token_stream.consume(":", true);
        m_token_stream.consume();
        m_token_stream.consume("is", true);
        auto value = m_token_stream.join_until(";", " ").string;
        m_token_stream.consume(";", true);

        if (value[0] == '"' && value.back() == '"')
        {
            value = value.substr(1, value.size() - 2);
        }
        auto type_it = m_attribute_types.find(attr_type);
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
    inst.line_number = m_token_stream.peek().number;
    inst.name        = m_token_stream.consume();
    m_token_stream.consume(":", true);

    // remove prefix from type
    if (m_token_stream.peek() == "entity")
    {
        m_token_stream.consume("entity", true);
        inst.type = m_token_stream.consume();
        auto pos  = inst.type.find('.');
        if (pos != std::string::npos)
        {
            inst.type = inst.type.substr(pos + 1);
        }
        if (m_entities.find(inst.type) == m_entities.end())
        {
            log_error("hdl_parser", "trying to instantiate unknown entity '{}' in line {}", inst.type, inst.line_number);
            return false;
        }
    }
    else if (m_token_stream.peek() == "component")
    {
        m_token_stream.consume("component", true);
        inst.type = m_token_stream.consume();
    }
    else
    {
        inst.type     = m_token_stream.consume();
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

    if (m_token_stream.peek() == "generic")
    {
        m_token_stream.consume("generic", true);
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        auto generic_map = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);
        while (generic_map.remaining() > 0)
        {
            auto lhs = generic_map.join_until("=>", " ");
            generic_map.consume("=>", true);
            auto rhs = generic_map.join_until(",", " ");
            generic_map.consume(",", generic_map.remaining() > 0);    // last entry has no comma

            inst.generics.emplace_back(lhs, rhs);
        }
    }

    if (m_token_stream.peek() == "port")
    {
        m_token_stream.consume("port", true);
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        auto port_map = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);
        while (port_map.remaining() > 0)
        {
            auto lhs = port_map.extract_until("=>");
            port_map.consume("=>", true);
            auto rhs = port_map.extract_until(",");
            port_map.consume(",", port_map.remaining() > 0);    // last entry has no comma
            for (const auto& [a, b] : get_assignments(e, lhs, rhs))
            {
                inst.ports.emplace_back(a, b);
            }
        }
    }

    m_token_stream.consume(";", true);

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

    case_insensitive_string_map<std::string> top_assignments;

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

        if (core_utils::equals_ignore_case(direction, "in") || core_utils::equals_ignore_case(direction, "inout"))
        {
            if (!new_net->mark_global_input_net())
            {
                log_error("hdl_parser", "could not mark net '{}' as global input", name);
                return false;
            }
        }
        if (core_utils::equals_ignore_case(direction, "out") || core_utils::equals_ignore_case(direction, "inout"))
        {
            if (!new_net->mark_global_output_net())
            {
                log_error("hdl_parser", "could not mark net '{}' as global output", name);
                return false;
            }
        }
        if (!core_utils::equals_ignore_case(direction, "in") && !core_utils::equals_ignore_case(direction, "out") && !core_utils::equals_ignore_case(direction, "inout"))
        {
            log_error("hdl_parser", "entity '{}', line {} : unkown direction '{}'", name, top_entity.line_number, direction);
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
                auto slave_source = slave_net->get_source();
                if (slave_source.get_gate() != nullptr)
                {
                    slave_net->remove_source(slave_source);

                    if (master_net->get_source().get_gate() == nullptr)
                    {
                        master_net->add_source(slave_source);
                    }
                    else if (slave_source.get_gate() != master_net->get_source().get_gate())
                    {
                        log_error("hdl_parser", "could not merge nets '{}' and '{}'", slave_net->get_name(), master_net->get_name());
                        return false;
                    }
                }

                // merge destinations
                if (slave_net->is_global_output_net())
                {
                    master_net->mark_global_output_net();
                }

                for (const auto& dst : slave_net->get_destinations())
                {
                    slave_net->remove_destination(dst);

                    if (!master_net->is_a_destination(dst))
                    {
                        master_net->add_destination(dst);
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

std::shared_ptr<module> hdl_parser_vhdl::instantiate(const entity& e, std::shared_ptr<module> parent, case_insensitive_string_map<std::string> parent_module_assignments)
{
    // remember assigned aliases so they are not lost when recursively going deeper
    std::unordered_map<std::string, std::string> aliases;

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

    for (const auto& [signal, assignment] : e.direct_assignments)
    {
        std::string a = signal;
        std::string b = assignment;
        if (parent_module_assignments.find(a) != parent_module_assignments.end())
        {
            a = parent_module_assignments.at(a);
        }
        else if (auto it = aliases.find(a); it != aliases.end())
        {
            a = it->second;
        }
        else
        {
            log_warning("hdl_parser", "no alias for net '{}'", a);
        }
        if (parent_module_assignments.find(b) != parent_module_assignments.end())
        {
            b = parent_module_assignments.at(b);
        }
        else if (auto it = aliases.find(b); it != aliases.end())
        {
            b = it->second;
        }
        else
        {
            log_warning("hdl_parser", "no alias for net '{}'", b);
        }
        m_nets_to_merge[b].push_back(a);
    }

    // caches
    auto vcc_gate_types = m_netlist->get_gate_library()->get_vcc_gate_types();
    auto gnd_gate_types = m_netlist->get_gate_library()->get_gnd_gate_types();
    auto gate_types     = m_netlist->get_gate_library()->get_gate_types();

    // process instances i.e. gates or other entities
    for (const auto& inst : e.instances)
    {
        // will later hold either module or gate, so attributes can be assigned properly
        data_container* container;

        // assign actual signal names to ports
        case_insensitive_string_map<std::string> instance_assignments;
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
                else
                {
                    instance_assignments[pin] = signal;
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
                auto it = std::find_if(gate_types.begin(), gate_types.end(), [&](auto& v) { return core_utils::equals_ignore_case(v.first, inst.type); });
                if (it == gate_types.end())
                {
                    log_error("hdl_parser", "could not find gate type '{}' in gate library '{}'", inst.type, m_netlist->get_gate_library()->get_name());
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
            auto input_pins  = new_gate->get_input_pins();
            auto output_pins = new_gate->get_output_pins();

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
                        log_warning("hdl_parser", "creating undeclared signal '{}' assigned to port '{}' of instance '{}' (starting at line {})", net_name, pin, inst.name, inst.line_number);

                        current_net                            = m_netlist->create_net(net_name);
                        m_net_by_name[current_net->get_name()] = current_net;
                    }
                    else
                    {
                        current_net = it->second;
                    }
                }

                // add net src/dst by pin types
                bool is_input = false;
                {
                    auto it = std::find_if(input_pins.begin(), input_pins.end(), [&](auto& s) { return core_utils::equals_ignore_case(s, pin); });
                    if (it != input_pins.end())
                    {
                        is_input = true;
                        pin      = *it;
                    }
                }
                bool is_output = false;
                {
                    auto it = std::find_if(output_pins.begin(), output_pins.end(), [&](auto& s) { return core_utils::equals_ignore_case(s, pin); });
                    if (it != output_pins.end())
                    {
                        is_output = true;
                        pin       = *it;
                    }
                }

                if (!is_input && !is_output)
                {
                    log_error("hdl_parser", "gate '{}' ({}) has no pin '{}'", new_gate->get_name(), new_gate->get_type()->get_name(), pin);
                    log_error("hdl_parser", "  available input pins: {}", core_utils::join(", ", new_gate->get_type()->get_input_pins()));
                    log_error("hdl_parser", "  available output pins: {}", core_utils::join(", ", new_gate->get_type()->get_output_pins()));
                    return nullptr;
                }

                if (is_output)
                {
                    if (current_net->get_source().get_gate() != nullptr)
                    {
                        auto src = current_net->get_source().get_gate();
                        log_error("hdl_parser",
                                  "net '{}' already has source gate '{}' (type {}), cannot assign '{}' (type {})",
                                  current_net->get_name(),
                                  src->get_name(),
                                  src->get_type()->get_name(),
                                  new_gate->get_name(),
                                  new_gate->get_type()->get_name());
                    }
                    if (!current_net->add_source(new_gate, pin))
                    {
                        return nullptr;
                    }
                }

                if (is_input && !current_net->add_destination(new_gate, pin))
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

std::vector<std::string> hdl_parser_vhdl::get_vector_signals(const std::string& base_name, token_stream type)
{
    // remove default assignment if available
    type = type.extract_until(":=");

    // if there is no range given, the signal is just the name
    if (type.size() == 1)
    {
        type.consume("std_logic");
        return {base_name};
    }

    auto type_name = type.consume();
    type.consume("(");
    auto bounds = type.extract_until(")");
    type.consume(")");

    // process ranges
    std::vector<std::vector<u32>> ranges;
    while (bounds.remaining() > 0)
    {
        auto bound = bounds.extract_until(",");
        bounds.consume(",", bounds.remaining() > 0);
        ranges.emplace_back(parse_range(bound));
    }

    // find the best matching supported vector type
    std::unordered_map<std::string, u32> std_logic_vector_identifier_to_dimension = {
        {"STD_LOGIC_VECTOR", 1},
        {"STD_LOGIC_VECTOR2", 2},
        {"STD_LOGIC_VECTOR3", 3},
    };
    u32 dimension = 0;
    for (const auto& it : std_logic_vector_identifier_to_dimension)
    {
        if ((core_utils::to_upper(type_name).find(it.first) != std::string::npos) && (dimension < it.second))
        {
            dimension = it.second;
        }
    }

    // compare expected dimension with actually found dimension
    if (dimension != (u32)ranges.size())
    {
        log_error("hdl_parser", "dimension-bound mismatch in line {} : expected {}, got {}", type_name.number, dimension, ranges.size());
        return {};
    }

    // depending on the dimension, build the signal string(s)
    std::vector<std::string> result;
    if (dimension == 1)
    {
        for (auto x : ranges[0])
        {
            result.push_back(base_name + "(" + std::to_string(x) + ")");
        }
    }
    else if (dimension == 2)
    {
        for (auto x : ranges[0])
        {
            for (auto y : ranges[1])
            {
                result.push_back(base_name + "(" + std::to_string(x) + ", " + std::to_string(y) + ")");
            }
        }
    }
    else if (dimension == 3)
    {
        for (auto x : ranges[0])
        {
            for (auto y : ranges[1])
            {
                for (auto z : ranges[2])
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

std::unordered_map<std::string, std::string> hdl_parser_vhdl::get_assignments(entity& e, token_stream& lhs, token_stream& rhs)
{
    // a port may be
    // (1): a => ( aggregate assignment )
    // (2): a => ...
    // (3): a(x) => ...
    //      a(x, y, z) => ...
    // (4): a(x to y) => B"01010101..."
    // (5): a(x to y) => b(s to t)

    // case (1) and (2)
    if (lhs.size() == 1)
    {
        if (rhs.peek() == "(")
        {
            // case (1)
            auto left_base_name = lhs.consume().string;
            auto left_type      = e.type_of_signal.at(left_base_name);
            auto left_signals   = get_vector_signals(left_base_name, left_type);

            rhs.consume("(");
            rhs = rhs.extract_until(")");

            std::unordered_map<std::string, std::string> res;
            u32 i = 0;
            while (rhs.remaining() > 0)
            {
                auto aggregate = rhs.extract_until(",");
                rhs.consume(",");

                auto right_name                            = aggregate.consume().string;
                std::vector<std::string> aggregate_signals = {right_name};

                // if aggregate contains a range, parse it
                if (aggregate.remaining() > 0)
                {
                    aggregate_signals.clear();
                    for (auto right_index : parse_range(aggregate))
                    {
                        aggregate_signals.push_back(right_name + "(" + std::to_string(right_index) + ")");
                    }
                }

                for (const auto& signal : aggregate_signals)
                {
                    res.emplace(left_signals[i], signal);
                    ++i;
                }
            }
            if (i != left_signals.size())
            {
                log_error("hdl_parser", "aggregate assignment did not match size of base signal (line {})", lhs.at(0).number);
                return {};
            }
            return res;
        }
        else
        {
            // case (2)
            return {std::make_pair(lhs.at(0), rhs.join(""))};
        }
    }

    // check left and right bounds for ranges
    bool left_contains_range  = lhs.size() == 6 && lhs.at(1) == "(" && lhs.at(5) == ")";
    bool right_contains_range = rhs.size() == 6 && rhs.at(1) == "(" && rhs.at(5) == ")";

    if (!left_contains_range)
    {
        // case (3)
        return {std::make_pair(lhs.join(""), rhs.join(""))};
    }
    else    // -> left_contains_range true
    {
        auto left_base_name = lhs.consume().string;
        auto left_range     = parse_range(lhs);

        if (!right_contains_range)
        {
            // case (4)

            // right part has to be a bitvector
            if (rhs.size() != 1 || !core_utils::starts_with(rhs.at(0).string, "B\"", true))
            {
                log_error("hdl_parser", "assignment of anything but a binary bitvector is not supported (line {})", lhs.at(0).number);
                return {};
            }

            // extract value
            std::string right_values = rhs.at(0).string.substr(2, rhs.at(0).string.size() - 3);

            // assemble assignment strings
            std::unordered_map<std::string, std::string> result;
            for (u32 i = 0; i < left_range.size(); ++i)
            {
                result.emplace(left_base_name + "(" + std::to_string(left_range[i]) + ")", std::string("'") + right_values[i] + "'");
            }

            return result;
        }
        else    // -> right_contains_range true
        {
            // case (5)

            // right part is a range just like left part
            auto right_base_name = rhs.consume().string;
            auto right_range     = parse_range(rhs);

            // check that both ranges match
            if (left_range.size() != right_range.size())
            {
                lhs.set_position(0);
                rhs.set_position(0);
                log_error("hdl_parser", "ranges on assignment '{} => {}' do not match in line {}", lhs.join("").string, rhs.join("").string, lhs.at(0).number);
                return {};
            }

            // assemble assignment strings
            std::unordered_map<std::string, std::string> result;
            for (u32 i = 0; i < right_range.size(); ++i)
            {
                result.emplace(left_base_name + "(" + std::to_string(left_range[i]) + ")", right_base_name + "(" + std::to_string(right_range[i]) + ")");
            }

            return result;
        }
    }
}

std::vector<u32> hdl_parser_vhdl::parse_range(token_stream& range)
{
    bool left_bracket_consumed = range.consume("(");

    if (range.remaining() == 0)
    {
        return {};
    }

    if (range.remaining() == 1 || range.peek(1) == ")")
    {
        return {(u32)std::stoi(range.consume())};
    }

    int direction = 1;
    int start     = std::stoi(range.consume());
    if (range.peek() == "downto")
    {
        range.consume("downto");
        direction = -1;
    }
    else
    {
        range.consume("to", true);
    }
    int end = std::stoi(range.consume());

    if (left_bracket_consumed)
    {
        range.consume(")");
    }

    std::vector<u32> res;
    for (int i = start; i != end + direction; i += direction)
    {
        res.push_back((u32)i);
    }
    return res;
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
