#include "vhdl_verilog_writers/hdl_writer_vhdl.h"

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    bool HDLWriterVHDL::write(Netlist* netlist, std::stringstream& stream)
    {
        m_netlist = netlist;
        m_stream  = &stream;

        this->prepare_signal_names();

        auto library_includes = m_netlist->get_gate_library()->get_includes();

        *m_stream << "library IEEE;" << std::endl;
        *m_stream << "use IEEE.STD_LOGIC_1164.all;" << std::endl;
        *m_stream << "use IEEE.NUMERIC_STD.all;" << std::endl;
        *m_stream << std::endl;
        for (const auto& inc : library_includes)
        {
            *m_stream << "use " << inc << "all;" << std::endl;
        }

        this->print_module_interface_vhdl();

        std::string entity_name = m_netlist->get_design_name();

        *m_stream << std::endl << "architecture STRUCTURE of " << entity_name << " is" << std::endl;

        this->print_signal_definition_vhdl();

        *m_stream << "begin" << std::endl;

        this->print_gate_definitions_vhdl();

        *m_stream << "end STRUCTURE;" << std::endl;

        return true;
    }

    void HDLWriterVHDL::prepare_signal_names()
    {
        //Generate all signal names

        auto nets = m_netlist->get_nets();

        std::unordered_set<std::string> used_names;
        auto find_alias = [&](Net* n) {
            if (m_printable_signal_names.find(n) != m_printable_signal_names.end())
            {
                return;
            }

            auto formatted   = get_net_name(n);
            std::string name = formatted;

            u32 cnt = 0;
            while (used_names.find(name) != used_names.end())
            {
                name = formatted + "_" + std::to_string(cnt);
                cnt++;
            }

            used_names.insert(name);
            m_printable_signal_names[n]               = name;
            m_printable_signal_names_str_to_net[name] = n;
        };

        for (auto n : m_netlist->get_global_input_nets())
        {
            find_alias(n);
            m_input_net_names.push_back(m_printable_signal_names[n]);
        }
        for (auto n : m_netlist->get_global_output_nets())
        {
            find_alias(n);
            m_output_net_names.push_back(m_printable_signal_names[n]);
        }
        for (auto n : nets)
        {
            find_alias(n);
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

    std::string HDLWriterVHDL::get_net_name(Net* n)
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

    std::string HDLWriterVHDL::get_gate_name(Gate* g)
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

    std::string HDLWriterVHDL::get_port_name(std::string pin)
    {
        return pin;
    }

    void HDLWriterVHDL::print_module_interface_vhdl()
    {
        std::string entity_name = m_netlist->get_design_name();

        //Print module interface
        *m_stream << "entity " << entity_name << " is" << std::endl;
        *m_stream << "  port (" << std::endl;
        bool begin = true;
        for (auto in_name : m_input_net_names)
        {
            if (begin)
            {
                *m_stream << in_name << " : in STD_LOGIC";
                begin = false;
            }
            else
            {
                *m_stream << "; " << std::endl << "  " << in_name << " : in STD_LOGIC";
            }
        }

        for (auto out_name : m_output_net_names)
        {
            if (begin)
            {
                *m_stream << out_name << " : out STD_LOGIC";
                begin = false;
            }
            else
            {
                *m_stream << "; " << std::endl << "  " << out_name << " : out STD_LOGIC";
            }
        }

        *m_stream << std::endl;
        *m_stream << ");" << std::endl;
        *m_stream << "end " << entity_name << ";" << std::endl;
    }

    void HDLWriterVHDL::print_signal_definition_vhdl()
    {
        //Declare all wires
        std::vector<std::tuple<std::string, Net*>> nets;
        for (auto it : m_printable_signal_names_str_to_net)
        {
            if (it.second->get_name() == "'1'" || it.second->get_name() == "'0'")
            {
                continue;
            }
            if (it.second->is_global_input_net() || it.second->is_global_output_net())
            {
                continue;
            }
            nets.emplace_back(it.first, it.second);
        }

        std::sort(nets.begin(), nets.end(), [](const std::tuple<std::string, Net*>& a, const std::tuple<std::string, Net*>& b) -> bool { return std::get<1>(a)->get_id() < std::get<1>(b)->get_id(); });

        for (auto tup : nets)
        {
            *m_stream << "  signal " << std::get<0>(tup) << " : STD_LOGIC;" << std::endl;
        }
    }

    void HDLWriterVHDL::print_gate_definitions_vhdl()
    {
        auto unsorted_gates = m_netlist->get_gates();
        std::vector<Gate*> gates(unsorted_gates.begin(), unsorted_gates.end());
        std::sort(gates.begin(), gates.end(), [](Gate* a, Gate* b) -> bool { return a->get_id() < b->get_id(); });

        for (auto& gate : gates)
        {
            if (gate->is_gnd_gate() || gate->is_vcc_gate())
            {
                continue;
            }

            auto formatted = get_gate_name(gate);
            auto gate_name = formatted;
            while (m_printable_signal_names_str_to_net.find(gate_name) != m_printable_signal_names_str_to_net.end())
            {
                gate_name += "_inst";
            }
            *m_stream << gate_name;
            *m_stream << " : " << gate->get_type()->get_name() << std::endl;

            this->print_generic_map_vhdl(gate);

            *m_stream << " port map (" << std::endl;

            bool begin_signal_list = true;
            begin_signal_list      = this->print_gate_signal_list_vhdl(gate, gate->get_input_pins(), begin_signal_list, std::bind(&Gate::get_fan_in_net, gate, std::placeholders::_1));
            begin_signal_list      = this->print_gate_signal_list_vhdl(gate, gate->get_output_pins(), begin_signal_list, std::bind(&Gate::get_fan_out_net, gate, std::placeholders::_1));

            *m_stream << std::endl << ");" << std::endl;
        }
    }

    void HDLWriterVHDL::print_generic_map_vhdl(Gate* n)
    {
        // Map init value
        bool first_generic = true;
        for (const auto& d : n->get_data_map())
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
                    *m_stream << "  generic map(" << std::endl;
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "    " << std::get<1>(d.first) << " => " << content.c_str();
            }
            if (type == "bit_vector")
            {
                std::string bit_string = "X\"" + content + "\"";
                if (first_generic)
                {
                    *m_stream << "  generic map(" << std::endl;
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "   " << std::get<1>(d.first) << " => " << bit_string.c_str();
            }
            else if (type == "string")
            {
                if (first_generic)
                {
                    *m_stream << "  generic map(" << std::endl;
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "   " << std::get<1>(d.first) << " => "
                          << "\"" << content << "\"";
            }
            else if (type == "bit_value")
            {
                if (first_generic)
                {
                    *m_stream << "  generic map(" << std::endl;
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "   " << std::get<1>(d.first) << " => "
                          << "\'" << content << "\'";
            }
            else if (type == "boolean")
            {
                if (first_generic)
                {
                    *m_stream << "  generic map(" << std::endl;
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                std::string val = (content == "true") ? "true" : "false";
                *m_stream << "   " << std::get<1>(d.first) << " => " << val.c_str();
            }
            else if (type == "integer")
            {
                if (first_generic)
                {
                    *m_stream << "  generic map(" << std::endl;
                    first_generic = false;
                }
                else
                {
                    *m_stream << "," << std::endl;
                }
                *m_stream << "   " << std::get<1>(d.first) << " => " << content.c_str();
            }
        }
        if (!first_generic)
        {
            *m_stream << std::endl << "  )" << std::endl;
        }
    }

    bool HDLWriterVHDL::print_gate_signal_list_vhdl(Gate* n, std::vector<std::string> port_types, bool is_first, std::function<Net*(std::string)> get_net_fkt)
    {
        std::sort(port_types.begin(), port_types.end());
        for (auto& port_type : port_types)
        {
            Net* e = get_net_fkt(port_type);
            if (e == nullptr)
            {
                log_info("hdl_writer", "VHDL serializer skipped signal translation for gate {} with type {} and port {} NO EDGE available", n->get_name(), n->get_type()->get_name(), port_type);
            }
            else
            {
                if (!is_first)
                {
                    *m_stream << "," << std::endl;
                }
                auto srcs = e->get_sources();
                if (std::any_of(srcs.begin(), srcs.end(), [](auto src) { return src->get_gate()->is_vcc_gate(); }))
                {
                    *m_stream << "   " << port_type << " => '1'";
                }
                else if (std::any_of(srcs.begin(), srcs.end(), [](auto src) { return src->get_gate()->is_gnd_gate(); }))
                {
                    *m_stream << "   " << port_type << " => '0'";
                }
                else
                {
                    *m_stream << "   " << port_type << " => " << m_printable_signal_names.find(e)->second;
                }
                is_first = false;
            }
        }
        return is_first;
    }
}    // namespace hal
