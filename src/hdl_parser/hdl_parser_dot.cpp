#include "hdl_parser/hdl_parser_dot.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

hdl_parser_dot::hdl_parser_dot(std::stringstream& stream) : hdl_parser(stream)
{
}

std::shared_ptr<netlist> hdl_parser_dot::parse(const std::string& gate_library)
{
    if (gate_library != "DOT")
    {
        log_error("hdl_parser", "dot parser is only implemented for gate-library 'DOT' so far.");
        return nullptr;
    }

    m_netlist = netlist_factory::create_netlist(gate_library);
    if (m_netlist == nullptr)
    {
        log_error("hdl_parser", "netlist_factory returned nullptr.");
        return nullptr;
    }
    std::string line = "", buffer = "", design = "", header = "", gates = "", edges = "";
    while (std::getline(m_fs, line))
        buffer += line;

    header = buffer.substr(buffer.find("dinetlist ") + 8);
    header = core_utils::trim(header.substr(0, buffer.find("{")));
    if (!this->parse_header(header))
    {
        return nullptr;
    }

    design = buffer.substr(buffer.find("{") + 1);
    design = design.substr(0, design.find("}"));

    gates   = design.substr(0, design.find("->"));
    auto it = gates.find_last_of(";");
    gates   = gates.substr(0, it);
    if (!this->parse_gates(gates))
    {
        return nullptr;
    }

    edges = design.substr(it + 1);
    edges = edges.substr(0, edges.find_last_of(";"));
    if (!this->parse_edges(edges))
    {
        return nullptr;
    }

    return m_netlist;
}

bool hdl_parser_dot::parse_header(const std::string& header)
{
    m_netlist->set_design_name(header.substr(0, header.find(" ")));
    return true;
}

bool hdl_parser_dot::parse_gates(const std::string& gates)
{
    for (const auto& token : core_utils::split(gates, ';'))
    {
        auto name = token.substr(0, token.find("["));
        if (m_netlist->create_gate(m_netlist->get_unique_gate_id(), "UNK", name) == nullptr)
            return false;
    }
    return true;
}

bool hdl_parser_dot::parse_edges(const std::string& edges)
{
    for (const auto& token : core_utils::split(edges, ';'))
    {
        auto src      = core_utils::trim(token.substr(0, token.find("->")));
        auto src_gate = *(m_netlist->get_gates(DONT_CARE, src).begin());
        auto dst      = core_utils::trim(token.substr(token.find("->") + 2));
        auto dst_gate = *(m_netlist->get_gates(DONT_CARE, dst).begin());

        if (src_gate->get_fan_out_nets().size() == 0)
        {
            auto new_net_id   = m_netlist->get_unique_net_id();
            auto new_net_name = "net_" + std::to_string(new_net_id);
            auto new_net      = m_netlist->create_net(new_net_id, new_net_name);
            if (new_net == nullptr)
                return false;
            if (!new_net->set_src(src_gate, "O"))
                return false;
        }
        auto net = src_gate->get_fan_out_net("O");
        // determine number of already assigned input pins to handle double edges
        auto num_of_assigned_pins = 0;
        for (const auto& input_pin_type : dst_gate->get_input_pin_types())
        {
            if (dst_gate->get_fan_in_net(input_pin_type) != nullptr)
                num_of_assigned_pins++;
        }
        std::string dst_pin_type = "I" + std::to_string(num_of_assigned_pins);
        if (!net->add_dst(dst_gate, dst_pin_type))
            return false;
    }
    return true;
}
