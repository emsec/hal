#include "hdl_writer/hdl_writer_vhdl.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <fstream>

hdl_writer_vhdl::hdl_writer_vhdl(std::stringstream& stream) : hdl_writer(stream)
{
}

bool hdl_writer_vhdl::write(std::shared_ptr<netlist> const g)
{
    m_netlist = g;

    this->prepare_signal_names();

    auto library_includes = m_netlist->get_gate_library()->get_includes();

    m_stream << "library IEEE;" << std::endl;
    m_stream << "use IEEE.STD_LOGIC_1164.all;" << std::endl;
    m_stream << "use IEEE.NUMERIC_STD.all;" << std::endl;
    m_stream << std::endl;
    for (const auto& inc : library_includes)
    {
        m_stream << "use " << inc << "all;" << std::endl;
    }

    this->print_module_interface_vhdl();

    std::string entity_name = m_netlist->get_design_name();

    m_stream << std::endl << "architecture STRUCTURE of " << entity_name << " is" << std::endl;

    this->print_signal_definition_vhdl();

    m_stream << "begin" << std::endl;

    this->print_gate_definitions_vhdl();

    m_stream << "end STRUCTURE;" << std::endl;

    return true;
}

void hdl_writer_vhdl::prepare_signal_names()
{
    //Generate all signal names

    auto nets = m_netlist->get_nets();

    for (auto e : nets)
    {
        std::string name_tmp                          = this->get_net_name(e);
        m_printable_signal_names[e]                   = name_tmp;
        m_printable_signal_names_str_to_net[name_tmp] = e;
    }
    std::vector<std::string> del_items;
    for (auto n : m_printable_signal_names_str_to_net)
    {
        std::string net_temp = n.first;
        if (std::all_of(net_temp.begin(), net_temp.end(), ::isdigit))
        {
            net_temp   = "NET_" + net_temp;
            int i      = 10;
            bool error = false;
            while (m_printable_signal_names_str_to_net.find(net_temp) != m_printable_signal_names_str_to_net.end())
            {
                if (i <= 0)
                {
                    log_error("hdl_writer", "Could not find unique value for {}! Current solution: {}", n.first, net_temp);
                    error = true;
                    break;
                }
                --i;
                net_temp = "COLLISION_" + net_temp;
            }
            if (!error)
            {
                m_printable_signal_names[n.second]            = net_temp;
                m_printable_signal_names_str_to_net[net_temp] = n.second;
                del_items.push_back(n.first);
            }
        }
    }
    for (auto str : del_items)
    {
        m_printable_signal_names_str_to_net.erase(str);
    }
    m_only_wire_names.insert(m_printable_signal_names.begin(), m_printable_signal_names.end());
    for (auto&& wire_name : m_only_wire_names)
    {
        m_only_wire_names_str_to_net[wire_name.second] = wire_name.first;
    }
    //input entity
    std::set<std::shared_ptr<net>> in_nets = m_netlist->get_global_input_nets();
    for (auto it : in_nets)
    {
        m_in_names[it]                                = this->get_net_name(it);
        m_in_names_str_to_net[this->get_net_name(it)] = it;
        m_only_wire_names.erase(it);
        m_only_wire_names_str_to_net.erase(this->get_net_name(it));
    }
    //output entity
    std::set<std::shared_ptr<net>> out_nets = m_netlist->get_global_output_nets();
    for (auto it : out_nets)
    {
        m_out_names[it]                                = this->get_net_name(it);
        m_out_names_str_to_net[this->get_net_name(it)] = it;
        m_only_wire_names.erase(it);
        m_only_wire_names_str_to_net.erase(this->get_net_name(it));
    }

    //vcc gates
    std::set<std::shared_ptr<gate>> one_gates = m_netlist->get_gates("X_ONE");
    for (auto n : one_gates)
    {
        std::set<std::shared_ptr<net>> o_nets = n->get_fan_out_nets();
        for (auto e : o_nets)
        {
            if (e->get_name() == "'1'")
                continue;
            m_vcc_names[e]                                = this->get_net_name(e);
            m_vcc_names_str_to_net[this->get_net_name(e)] = e;
            m_only_wire_names.erase(e);
            m_only_wire_names_str_to_net.erase(this->get_net_name(e));
        }
    }
    //gnd gates
    std::set<std::shared_ptr<gate>> zero_gates = m_netlist->get_gates("X_ZERO");
    for (auto n : zero_gates)
    {
        std::set<std::shared_ptr<net>> o_nets = n->get_fan_out_nets();
        for (auto e : o_nets)
        {
            if (e->get_name() == "'0'")
                continue;
            m_gnd_names[e]                                = this->get_net_name(e);
            m_gnd_names_str_to_net[this->get_net_name(e)] = e;
            m_only_wire_names.erase(e);
            m_only_wire_names_str_to_net.erase(this->get_net_name(e));
        }
    }
}

static void replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        pos += to.size();
    }
}

static void remove(std::string& str, const std::string& s)
{
    size_t pos = 0;
    while ((pos = str.find(s, pos)) != std::string::npos)
    {
        str.erase(pos, s.length());
    }
}

std::string hdl_writer_vhdl::get_net_name(const std::shared_ptr<net> n)
{
    std::string name = n->get_name();

    replace(name, "(", "_");
    remove(name, ")");
    replace(name, ", ", "_");
    replace(name, ",", "_");
    remove(name, "/");
    //remove(name, "\\");
    replace(name, "[", "_");
    replace(name, "]", "_");
    replace(name, "<", "_");
    replace(name, ">", "_");
    replace(name, "__", "_");
    if (name[0] == '\\')
    {
        remove(name, "\\");
        name.insert(0, 1, '\\');
        name.push_back('\\');
    }
    else
    {
        remove(name, "\\");
    }
    if (name[0] == '_')
    {
        name.erase(0, 1);
    }
    if (name[name.size() - 1] == '_')
    {
        name.erase(name.size() - 1, 1);
    }

    if (std::all_of(name.begin(), name.end(), ::isdigit))
    {
        name = "NET_" + name;
    }

    return name;
}

std::string hdl_writer_vhdl::get_gate_name(const std::shared_ptr<gate> g)
{
    std::string name = g->get_name();

    replace(name, "(", "_");
    remove(name, ")");
    replace(name, ", ", "_");
    replace(name, ",", "_");
    remove(name, "/");
    //remove(name, "\\");
    replace(name, "[", "_");
    replace(name, "]", "_");
    replace(name, "<", "_");
    replace(name, ">", "_");
    replace(name, "__", "_");
    if (name[0] == '\\')
    {
        remove(name, "\\");
        name.insert(0, 1, '\\');
        name.push_back('\\');
    }
    else
    {
        remove(name, "\\");
    }
    if (name[0] == '_')
    {
        name.erase(0, 1);
    }
    if (name[name.size() - 1] == '_')
    {
        name.erase(name.size() - 1, 1);
    }

    if (std::all_of(name.begin(), name.end(), ::isdigit))
    {
        name = "GATE_" + name;
    }

    return name;
}

std::string hdl_writer_vhdl::get_port_name(std::string pin)
{
    return pin;
}

void hdl_writer_vhdl::print_module_interface_vhdl()
{
    std::string entity_name = m_netlist->get_design_name();

    //Print module interface
    m_stream << "entity " << entity_name << " is" << std::endl;
    m_stream << "  port (" << std::endl;
    bool begin = true;
    for (auto in_name : m_in_names_str_to_net)
    {
        if (begin)
        {
            m_stream << in_name.first << " : in STD_LOGIC := 'X'";
            begin = false;
        }
        else
        {
            m_stream << "; " << std::endl << "  " << in_name.first << " : in STD_LOGIC := 'X'";
        }
    }

    for (auto out_name : m_out_names_str_to_net)
    {
        if (begin)
        {
            m_stream << out_name.first << " : out STD_LOGIC";
            begin = false;
        }
        else
        {
            m_stream << "; " << std::endl << "  " << out_name.first << " : out STD_LOGIC";
        }
    }

    m_stream << std::endl;
    m_stream << ");" << std::endl;
    m_stream << "end " << entity_name << ";" << std::endl;
}

void hdl_writer_vhdl::print_signal_definition_vhdl()
{
    //Declare all wires

    std::vector<std::tuple<std::string, std::shared_ptr<net>>> nets;
    for (auto name : m_only_wire_names_str_to_net)
    {
        if (name.second->get_name() == "'1'" || name.second->get_name() == "'0'")
            continue;
        nets.emplace_back(name.first, name.second);
    }

    std::sort(nets.begin(), nets.end(), [](const std::tuple<std::string, std::shared_ptr<net>>& a, const std::tuple<std::string, std::shared_ptr<net>>& b) -> bool {
        return std::get<1>(a)->get_id() < std::get<1>(b)->get_id();
    });

    for (auto tup : nets)
    {
        m_stream << "  signal " << std::get<0>(tup) << " : STD_LOGIC;" << std::endl;
    }

    for (auto name : m_vcc_names_str_to_net)
    {
        m_stream << "  signal " << name.first << " : STD_LOGIC := '1';" << std::endl;
    }
    for (auto name : m_gnd_names_str_to_net)
    {
        m_stream << "  signal " << name.first << " : STD_LOGIC := '0';" << std::endl;
    }
}

void hdl_writer_vhdl::print_gate_definitions_vhdl()
{
    auto unsorted_gates = m_netlist->get_gates();
    std::vector<std::shared_ptr<gate>> gates(unsorted_gates.begin(), unsorted_gates.end());
    std::sort(gates.begin(), gates.end(), [](const std::shared_ptr<gate>& a, const std::shared_ptr<gate>& b) -> bool { return a->get_id() < b->get_id(); });

    for (auto&& gate : gates)
    {
        if (gate->get_type()->get_name() == "GLOBAL_GND" || gate->get_type()->get_name() == "GLOBAL_VCC")
            continue;
        m_stream << get_gate_name(gate);
        m_stream << " : " << gate->get_type()->get_name() << std::endl;

        this->print_generic_map_vhdl(gate);

        m_stream << " port map (" << std::endl;

        bool begin_signal_list = true;
        begin_signal_list      = this->print_gate_signal_list_vhdl(gate, gate->get_input_pins(), begin_signal_list, std::bind(&gate::get_fan_in_net, gate, std::placeholders::_1));
        begin_signal_list      = this->print_gate_signal_list_vhdl(gate, gate->get_output_pins(), begin_signal_list, std::bind(&gate::get_fan_out_net, gate, std::placeholders::_1));

        m_stream << std::endl << ");" << std::endl;
    }
}

void hdl_writer_vhdl::print_generic_map_vhdl(std::shared_ptr<gate> const& n)
{
    // Map init value
    bool first_generic = true;
    for (const auto& d : n->get_data())
    {
        if (std::get<0>(d.first) != "generic")
            continue;

        auto content = std::get<1>(d.second);
        auto type    = std::get<0>(d.second);
        if (type == "invalid")
        {
            log_info("hdl_writer", "Found invalid generic content {}", content);
            continue;
        }
        if (std::get<1>(d.first) == "loc" || type == "time")
            continue;
        if (type == "time")
        {
            if (first_generic)
            {
                m_stream << "  generic map(" << std::endl;
                first_generic = false;
            }
            else
            {
                m_stream << "," << std::endl;
            }
            m_stream << "    " << std::get<1>(d.first) << " => " << content.c_str();
        }
        if (type == "bit_vector")
        {
            std::string bit_string = "X\"" + content + "\"";
            if (first_generic)
            {
                m_stream << "  generic map(" << std::endl;
                first_generic = false;
            }
            else
            {
                m_stream << "," << std::endl;
            }
            m_stream << "   " << std::get<1>(d.first) << " => " << bit_string.c_str();
        }
        else if (type == "string")
        {
            if (first_generic)
            {
                m_stream << "  generic map(" << std::endl;
                first_generic = false;
            }
            else
            {
                m_stream << "," << std::endl;
            }
            m_stream << "   " << std::get<1>(d.first) << " => "
                     << "\"" << content << "\"";
        }
        else if (type == "bit_value")
        {
            if (first_generic)
            {
                m_stream << "  generic map(" << std::endl;
                first_generic = false;
            }
            else
            {
                m_stream << "," << std::endl;
            }
            m_stream << "   " << std::get<1>(d.first) << " => "
                     << "\'" << content << "\'";
        }
        else if (type == "boolean")
        {
            if (first_generic)
            {
                m_stream << "  generic map(" << std::endl;
                first_generic = false;
            }
            else
            {
                m_stream << "," << std::endl;
            }
            std::string val = (content == "true") ? "true" : "false";
            m_stream << "   " << std::get<1>(d.first) << " => " << val.c_str();
        }
        else if (type == "integer")
        {
            if (first_generic)
            {
                m_stream << "  generic map(" << std::endl;
                first_generic = false;
            }
            else
            {
                m_stream << "," << std::endl;
            }
            m_stream << "   " << std::get<1>(d.first) << " => " << content.c_str();
        }
    }
    if (!first_generic)
    {
        m_stream << std::endl << "  )" << std::endl;
    }
}

bool hdl_writer_vhdl::print_gate_signal_list_vhdl(std::shared_ptr<gate> n, std::vector<std::string> port_types, bool is_first, std::function<std::shared_ptr<net>(std::string)> get_net_fkt)
{
    std::vector<std::string> port_types_sorted;
    std::copy(port_types.begin(), port_types.end(), std::back_inserter(port_types_sorted));
    std::sort(port_types_sorted.begin(), port_types_sorted.end());
    for (auto&& port_type : port_types_sorted)
    {
        std::shared_ptr<net> e = get_net_fkt(port_type);
        if (e == nullptr)
        {
            log_info("hdl_writer", "VHDL serializer skipped signal translation for gate {} with type {} and port {} NO EDGE available", n->get_name(), n->get_type()->get_name(), port_type);
        }
        else
        {
            if (!is_first)
            {
                m_stream << "," << std::endl;
            }
            m_stream << "   " << port_type << " => " << m_printable_signal_names.find(e)->second;
            is_first = false;
        }
    }
    return is_first;
}
