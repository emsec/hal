#include "vhdl_verilog_writers/hdl_writer_verilog.h"

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"


namespace hal
{
    bool HDLWriterVerilog::write(Netlist* netlist, std::stringstream& stream)
    {
        m_netlist = netlist;
        m_stream = &stream;

        this->prepare_signal_names();

        this->print_module_interface_verilog();

        this->print_signal_definition_verilog();

        this->print_gate_definitions_verilog();

        *m_stream << "endmodule" << std::endl;

        return true;
    }

    void HDLWriterVerilog::prepare_signal_names()
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
        for (auto it : m_netlist->get_global_input_nets())
        {
            m_in_names[it]                                = this->get_net_name(it);
            m_in_names_str_to_net[this->get_net_name(it)] = it;
            m_only_wire_names.erase(it);
            m_only_wire_names_str_to_net.erase(this->get_net_name(it));
        }
        //output entity
        for (auto it : m_netlist->get_global_output_nets())
        {
            m_out_names[it]                                = this->get_net_name(it);
            m_out_names_str_to_net[this->get_net_name(it)] = it;
            m_only_wire_names.erase(it);
            m_only_wire_names_str_to_net.erase(this->get_net_name(it));
        }

        //vcc gates
        for (auto n : m_netlist->get_vcc_gates())
        {
            for (auto e : n->get_fan_out_nets())
            {
                if (e->get_name() == "'1'")
                {
                    continue;
                }
                m_vcc_names[e]                                = this->get_net_name(e);
                m_vcc_names_str_to_net[this->get_net_name(e)] = e;
                m_only_wire_names.erase(e);
                m_only_wire_names_str_to_net.erase(this->get_net_name(e));
            }
        }
        //gnd gates
        for (auto n : m_netlist->get_gnd_gates())
        {
            for (auto e : n->get_fan_out_nets())
            {
                if (e->get_name() == "'0'")
                {
                    continue;
                }
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

    std::string HDLWriterVerilog::get_net_name(Net* n)
    {
        std::string name = n->get_name();

        if (name == "'1'")
        {
            name = "1'b1";
        }
        if (name == "'0'")
        {
            name = "1'b0";
        }

        replace(name, "(", "_");
        remove(name, ")");
        replace(name, ", ", "_");
        replace(name, ",", "_");
        remove(name, "/");
        remove(name, "\\");
        replace(name, "[", "_");
        replace(name, "]", "_");
        replace(name, "<", "_");
        replace(name, ">", "_");
        replace(name, "__", "_");
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

    std::string HDLWriterVerilog::get_gate_name(Gate* g)
    {
        std::string name = g->get_name();

        replace(name, "(", "_");
        remove(name, ")");
        replace(name, ", ", "_");
        replace(name, ",", "_");
        remove(name, "/");
        remove(name, "\\");
        replace(name, "[", "_");
        replace(name, "]", "_");
        replace(name, "<", "_");
        replace(name, ">", "_");
        replace(name, "__", "_");
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

        return name + "_inst";
    }

    std::string HDLWriterVerilog::get_port_name(std::string pin)
    {
        std::string pin_temp = pin;
        std::size_t found;
        bool marked = utils::starts_with(pin_temp, std::string("\\"));
        while ((found = pin_temp.find("(")) != std::string::npos)
        {
            pin_temp.replace(found, 1, "[");
            if (!marked)
            {
                pin_temp = "\\" + pin_temp;
                marked   = true;
            }
        }
        while ((found = pin_temp.find(")")) != std::string::npos)
        {
            pin_temp.replace(found, 1, "]");
            if (!marked)
            {
                pin_temp = "\\" + pin_temp;
                marked   = true;
            }
        }
        return pin_temp;
    }

    void HDLWriterVerilog::print_module_interface_verilog()
    {
        std::string entity_name = m_netlist->get_design_name();

        //Print module interface
        *m_stream << "module " << entity_name << " (" << std::endl;
        *m_stream << "  ";
        bool begin = true;
        for (auto in_name : m_in_names_str_to_net)
        {
            if (begin)
            {
                *m_stream << in_name.first.c_str();
                begin = false;
            }
            else
            {
                *m_stream << ", " << std::endl << "  " << in_name.first.c_str();
            }
        }

        for (auto out_name : m_out_names_str_to_net)
        {
            if (begin)
            {
                *m_stream << out_name.first.c_str();
                begin = false;
            }
            else
            {
                *m_stream << ", " << std::endl << "  " << out_name.first.c_str();
            }
        }

        *m_stream << std::endl;
        *m_stream << " ) ;" << std::endl;
    }

    void HDLWriterVerilog::print_signal_definition_verilog()
    {
        //Declare all wires
        for (auto in_name : m_in_names_str_to_net)
        {
            *m_stream << "  input " << in_name.first << " ;" << std::endl;
        }
        for (auto out_name : m_out_names_str_to_net)
        {
            *m_stream << "  output " << out_name.first << " ;" << std::endl;
        }
        for (auto name : m_only_wire_names_str_to_net)
        {
            if (name.second->get_name() == "'1'" || name.second->get_name() == "'0'")
            {
                continue;
            }
            *m_stream << "  wire " << name.first << " ;" << std::endl;
        }
        for (auto name : m_vcc_names_str_to_net)
        {
            *m_stream << "  wire " << name.first << " = 1'h1 ;" << std::endl;
        }
        for (auto name : m_gnd_names_str_to_net)
        {
            *m_stream << "  wire " << name.first << " = 1'h0 ;" << std::endl;
        }
    }

    void HDLWriterVerilog::print_gate_definitions_verilog()
    {
        auto gates = m_netlist->get_gates();
        for (auto&& gate : gates)
        {
            // TODO ugly bad bad bad
            if (gate->get_type()->get_name() == "GLOBAL_GND" || gate->get_type()->get_name() == "GLOBAL_VCC")
            {
                continue;
            }
            *m_stream << gate->get_type()->get_name() << " ";

            this->print_generic_map_verilog(gate);

            *m_stream << get_gate_name(gate);

            // Search for collision of gate name with a net name
            auto search_it = m_printable_signal_names_str_to_net.find(gate->get_name());
            if (search_it != m_printable_signal_names_str_to_net.end())
            {
                *m_stream << "_inst";
            }
            *m_stream << " (" << std::endl;

            bool begin_signal_list = true;
            begin_signal_list      = this->print_gate_signal_list_verilog(gate, gate->get_input_pins(), begin_signal_list, std::bind(&Gate::get_fan_in_net, gate, std::placeholders::_1));
            begin_signal_list      = this->print_gate_signal_list_verilog(gate, gate->get_output_pins(), begin_signal_list, std::bind(&Gate::get_fan_out_net, gate, std::placeholders::_1));

            *m_stream << std::endl << " ) ;" << std::endl;
        }
    }

    void HDLWriterVerilog::print_generic_map_verilog(Gate* n)
    {
        // Map init value
        auto data          = n->get_data_map();
        bool first_generic = true;
        for (auto d : data)
        {
            std::string content = std::get<1>(d.second);
            auto type           = std::get<0>(d.second);
            auto category       = std::get<0>(d.first);
            if (std::get<1>(d.first) == "loc" || type == "time" || category == "gui")
            {
                continue;
            }
            if (type == "invalid")
            {
                log_info("hdl_writer", "Found invalid generic content {}\n", content);
                continue;
            }
            if (type == "bit_vector")
            {
                unsigned long len = content.length() * 4;
                char& first       = content.at(0);
                if (first == '0' || first == '1')
                {
                    len -= 3;
                }
                else if (first == '2' || first == '3')
                {
                    len -= 2;
                }
                else if (first == '4' || first == '5' || first == '6' || first == '7')
                {
                    len -= 1;
                }
                std::string bit_string = std::to_string(len) + "'h" + content;
                if (first_generic)
                {
                    *m_stream << "#(";
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "." << std::get<1>(d.first) << "(" << bit_string << ")";
            }
            else if (type == "string")
            {
                if (first_generic)
                {
                    *m_stream << "#(";
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "." << std::get<1>(d.first) << "(\"" << content << "\")";
            }
            else if (type == "bit_value")
            {
                if (first_generic)
                {
                    *m_stream << "#(";
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "." << std::get<1>(d.first) << "(1\'b" << content << ")";
            }
            else if (type == "boolean")
            {
                if (first_generic)
                {
                    *m_stream << "#(";
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                std::string val = (content == "TRUE") ? "1" : "0";
                *m_stream << "." << std::get<1>(d.first) << "(" << val << ")";
            }
            else if (type == "integer")
            {
                if (first_generic)
                {
                    *m_stream << "#(";
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "." << std::get<1>(d.first) << "(" << content << ")";
            }
        }
        if (!first_generic)
        {
            *m_stream << ")" << std::endl;
        }
    }

    bool HDLWriterVerilog::print_gate_signal_list_verilog(Gate* n, std::vector<std::string> port_types, bool is_first, std::function<Net*(std::string)> get_net_fkt)
    {
        std::vector<std::string> port_types_sorted;
        auto port_types_normalized = this->get_gate_signal_buses_verilog(port_types);
        for (const auto& types_normalized : port_types_normalized)
        {
            port_types_sorted.push_back(types_normalized.first);
        }
        std::sort(port_types_sorted.begin(), port_types_sorted.end());
        for (auto&& port_type : port_types_sorted)
        {
            if (port_types_normalized[port_type].size() > 1)
            {
                std::stringstream tmp;
                if (!is_first)
                    tmp << "," << std::endl;

                tmp << "  .\\" << this->get_port_name(port_type) << " ({ ";
                bool first_in_line = true;
                bool skip          = false;
                for (const auto& ptype : port_types_normalized[port_type])
                {
                    Net* e = get_net_fkt(ptype);
                    if (e == nullptr)
                    {
                        skip = true;
                        log_info("hdl_writer",
                                 "Verilog serializer skipped signal translation for gate {} with type {} and port {} NO EDGE available (skipping pin: {})",
                                 n->get_name(),
                                 n->get_type()->get_name(),
                                 ptype,
                                 port_type);
                    }
                    else
                    {
                        if (!first_in_line)
                        {
                            tmp << ", ";
                        }
                        // !! The space between port type and ( is important and must not be removed !!
                        tmp << m_printable_signal_names.find(e)->second;
                        first_in_line = false;
                    }
                }
                tmp << " })";
                is_first = false;
                if (!skip)
                    *m_stream << tmp.str();
            }
            else
            {
                Net* e = get_net_fkt(port_type);
                if (e == nullptr)
                {
                    log_info("hdl_writer", "Verilog serializer skipped signal translation for gate {} with type {} and port {} NO EDGE available", n->get_name(), n->get_type()->get_name(), port_type);
                }
                else
                {
                    if (!is_first)
                    {
                        *m_stream << "," << std::endl;
                    }
                    // !! The space between port type and ( is important and must not be removed !!
                    *m_stream << "  .\\" << this->get_port_name(port_type) << " (" << m_printable_signal_names.find(e)->second << " )";
                    is_first = false;
                }
            }
        }
        return is_first;
    }

    std::map<std::string, std::vector<std::string>> HDLWriterVerilog::get_gate_signal_buses_verilog(std::vector<std::string> port_types)
    {
        std::map<std::string, std::vector<std::string>> collected_port_types;
        for (const auto& port_type : port_types)
        {
            if (port_type.find("(") != std::string::npos)
            {
                std::string port_name = port_type.substr(0, port_type.find("("));
                collected_port_types[port_name].push_back(port_type);
            }
            else
            {
                collected_port_types[port_type].push_back(port_type);
            }
        }
        //Extract position in bus
        for (auto& pt : collected_port_types)
        {
            if (pt.second.size() > 1)
            {
                std::vector<std::string> pins_sorted;
                pins_sorted.resize(pt.second.size());
                for (const auto& pin : pt.second)
                {
                    auto p           = pin.substr(pin.find("(") + 1);
                    p                = p.substr(0, p.size() - 1);
                    int idx          = std::stoi(p);
                    pins_sorted[idx] = pin;
                }
                std::reverse(pins_sorted.begin(), pins_sorted.end());
                pt.second = pins_sorted;
            }
        }
        return collected_port_types;
    }
}    // namespace hal
