#include "edif_parser/edif_parser.h"

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <fstream>
#include <iomanip>
#include <queue>

namespace hal
{
    Result<std::monostate> EdifParser::parse(const std::filesystem::path& file_path)
    {
        m_path = file_path;

        {
            std::ifstream ifs;
            ifs.open(file_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                return ERR("could not parse EDIF file '" + m_path.string() + "' : unable to open file");
            }
            m_fs << ifs.rdbuf();
            ifs.close();
        }

        // tokenize file
        tokenize();

        // parse tokens into intermediate format
        try
        {
            if (auto res = parse_tokens(); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse EDIF file '" + file_path.string() + "': unable to parse tokens");
            }
        }
        catch (TokenStream<std::string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                return ERR("could not parse EDIF file '" + m_path.string() + "': " + e.message + " (line " + std::to_string(e.line_number) + ")");
            }
            else
            {
                return ERR("could not parse EDIF file '" + m_path.string() + "': " + e.message);
            }
        }

        return OK({});
    }

    Result<std::unique_ptr<Netlist>> EdifParser::instantiate(const GateLibrary* gate_library)
    {
        // create empty netlist
        std::unique_ptr<Netlist> result = netlist_factory::create_netlist(gate_library);
        m_netlist                       = result.get();
        if (m_netlist == nullptr)
        {
            return ERR("could not instantiate EDIF netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create empty netlist");
        }

        std::unordered_set<std::string> referenced_library_names;
        std::unordered_set<std::string> referenced_cell_names;
        for (auto& [_lib_name, lib] : m_libraries_by_name)
        {
            for (const auto& [_cell_name, cell] : lib->m_cells_by_name)
            {
                for (const auto& [_, inst] : cell->m_instances_by_name)
                {
                    referenced_cell_names.insert(inst->m_cell->m_name);
                    referenced_library_names.insert(inst->m_cell->m_library->m_name);
                }
            }
        }

        EdifLibrary* top_lib = nullptr;
        for (const auto& [name, lib] : m_libraries_by_name)
        {
            if (referenced_library_names.find(name) == referenced_library_names.end())
            {
                if (top_lib != nullptr)
                {
                    return ERR("could not instantiate EDIF netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name()
                               + "': found multiple libraries as candidates for the top library");
                }

                top_lib = lib;
            }
        }

        if (top_lib == nullptr)
        {
            return ERR("could not instantiate EDIF netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': unable to find a top library");
        }

        EdifCell* top_cell = nullptr;
        for (const auto& [name, cell] : top_lib->m_cells_by_name)
        {
            if (referenced_cell_names.find(name) == referenced_cell_names.end())
            {
                if (top_cell != nullptr)
                {
                    return ERR("could not instantiate EDIF netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name()
                               + "': found multiple cells as candidates for the top cell");
                }

                top_cell = cell;
            }
        }

        if (top_cell == nullptr)
        {
            return ERR("could not instantiate EDIF netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': unable to find a top cell");
        }

        // buffer gate types
        m_gate_types     = gate_library->get_gate_types();
        m_gnd_gate_types = gate_library->get_gnd_gate_types();
        m_vcc_gate_types = gate_library->get_vcc_gate_types();

        // create const 0 and const 1 net, will be removed if unused
        m_zero_net = m_netlist->create_net("'0'");
        if (m_zero_net == nullptr)
        {
            return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create zero net");
        }
        m_net_by_name[m_zero_net->get_name()] = m_zero_net;

        m_one_net = m_netlist->create_net("'1'");
        if (m_one_net == nullptr)
        {
            return ERR("could not instantiate Verilog netlist '" + m_path.string() + "' with gate library '" + gate_library->get_name() + "': failed to create one net");
        }
        m_net_by_name[m_one_net->get_name()] = m_one_net;

        return ERR("not implemented");
    }

    // ###########################################################################
    // ###########          Parse HDL into Intermediate Format          ##########
    // ###########################################################################

    void EdifParser::tokenize()
    {
        const std::string delimiters = "() ";
        std::string current_token;
        u32 line_number = 0;

        std::string line;
        bool in_string = false;

        std::vector<Token<std::string>> parsed_tokens;
        while (std::getline(m_fs, line))
        {
            line_number++;
            // this->remove_comments(line, multi_line_comment);

            for (char c : line)
            {
                // deal with escaping and strings
                if (c == '"')
                {
                    in_string = !in_string;
                }

                if (delimiters.find(c) == std::string::npos || in_string)
                {
                    current_token += c;
                }
                else
                {
                    if (!current_token.empty())
                    {
                        parsed_tokens.emplace_back(line_number, current_token);
                        current_token.clear();
                    }

                    if (!std::isspace(c))
                    {
                        parsed_tokens.emplace_back(line_number, std::string(1, c));
                    }
                }
            }
            if (!current_token.empty())
            {
                parsed_tokens.emplace_back(line_number, current_token);
                current_token.clear();
            }
        }

        m_token_stream = TokenStream(parsed_tokens, {"("}, {")"});
    }

    Result<std::monostate> EdifParser::parse_tokens()
    {
        m_token_stream.consume("(", true);
        m_token_stream.consume("edif", true);
        m_token_stream.consume();    // consume design name (repeated later on)

        while (m_token_stream.remaining() > 1)
        {
            m_token_stream.consume("(", true);
            const auto next_token = m_token_stream.consume();
            if (next_token == "edifVersion")
            {
                if (!m_token_stream.consume("2") || !m_token_stream.consume("0") || !m_token_stream.consume("0"))
                {
                    return ERR("could not parse tokens: parser currently only supports EDIF version 2.0.0");
                }
            }
            else if (next_token == "library")
            {
                if (auto res = parse_library(); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens: failed to parse library");
                }
            }
            else
            {
                m_token_stream.consume_until(")", TokenStream<std::string>::END_OF_STREAM, true);
            }
            m_token_stream.consume(")", true);
        }

        m_token_stream.consume(")", true);

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_library()
    {
        auto lib      = std::make_unique<EdifLibrary>();
        auto* lib_raw = lib.get();

        const auto lib_name_token = m_token_stream.consume();
        lib_raw->m_name           = lib_name_token.string;
        while (m_token_stream.consume("("))
        {
            const auto next_token = m_token_stream.consume();
            if (next_token == "cell")
            {
                if (auto res = parse_cell(lib_raw); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse library '" + lib_name_token.string + "' in line " + std::to_string(lib_name_token.number) + ": failed to parse cell");
                }
            }
            else
            {
                m_token_stream.consume_until(")", TokenStream<std::string>::END_OF_STREAM, true);
            }
            m_token_stream.consume(")", true);
        }

        m_libraries.push_back(std::move(lib));
        m_libraries_by_name[lib_raw->m_name] = lib_raw;

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_cell(EdifLibrary* lib)
    {
        auto cell      = std::make_unique<EdifCell>();
        auto* cell_raw = cell.get();

        cell->m_library = lib;

        const auto first_token = m_token_stream.peek();
        u32 line_number        = first_token.number;

        if (first_token != "(")
        {
            cell_raw->m_name = m_token_stream.consume();
        }

        while (m_token_stream.consume("("))
        {
            const auto next_token = m_token_stream.consume();
            if (next_token == "rename")
            {
                const auto rename_res = parse_rename(m_token_stream);
                if (rename_res.is_error())
                {
                    return ERR(rename_res.get_error());
                }
                cell_raw->m_name = rename_res.get();
            }
            else if (next_token == "view")
            {
                if (auto res = parse_view(cell_raw); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse cell '" + cell_raw->m_name + "' in line " + std::to_string(next_token.number) + ": failed to parse view");
                }
            }
            else
            {
                m_token_stream.consume_until(")");
            }
            m_token_stream.consume(")", true);
        }

        if (cell_raw->m_name.empty())
        {
            return ERR("could not parse cell in line " + std::to_string(line_number) + ": no name given for cell");
        }

        lib->m_cells.push_back(std::move(cell));
        lib->m_cells_by_name[cell_raw->m_name] = cell_raw;

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_view(EdifCell* cell)
    {
        const auto view_name = m_token_stream.consume();
        while (m_token_stream.consume("("))
        {
            const auto next_token = m_token_stream.consume();
            if (next_token == "interface")
            {
                if (auto res = parse_interface(cell); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse view '" + view_name.string + "' in line " + std::to_string(view_name.number) + ": failed to parse interface");
                }
            }
            else if (next_token == "contents")
            {
                if (auto res = parse_contents(cell); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse view '" + view_name.string + "' in line " + std::to_string(view_name.number) + ": failed to parse contents");
                }
            }
            else
            {
                m_token_stream.consume_until(")", TokenStream<std::string>::END_OF_STREAM, true);
            }
            m_token_stream.consume(")", true);
        }

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_interface(EdifCell* cell)
    {
        while (m_token_stream.consume("("))
        {
            m_token_stream.consume("port", true);

            auto port = std::make_unique<EdifPort>();

            if (m_token_stream.peek() != "(")
            {
                port->m_name = m_token_stream.consume();
                port->m_expanded_names.push_back(port->m_name);
            }

            while (m_token_stream.consume("("))
            {
                const auto next_token = m_token_stream.consume();
                if (next_token == "rename")
                {
                    const auto rename_res = parse_rename(m_token_stream);
                    if (rename_res.is_error())
                    {
                        return ERR(rename_res.get_error());
                    }
                    port->m_name = rename_res.get();
                    port->m_expanded_names.push_back(port->m_name);
                }
                else if (next_token == "array")
                {
                    const auto rename_res = parse_array(m_token_stream);
                    if (rename_res.is_error())
                    {
                        return ERR(rename_res.get_error());
                    }
                    const auto array = rename_res.get();
                    port->m_name     = array.first;
                    port->m_width    = array.second;

                    if (array_msb_at_0)
                    {
                        for (i32 i = (i32)port->m_width - 1; i >= 0; i--)
                        {
                            port->m_expanded_names.push_back(port->m_name + "(" + std::to_string(i) + ")");
                        }
                    }
                    else
                    {
                        for (u32 i = 0; i < port->m_width; i++)
                        {
                            port->m_expanded_names.push_back(port->m_name + "(" + std::to_string(i) + ")");
                        }
                    }
                }
                else if (next_token == "property")
                {
                    if (const auto property_res = parse_property(m_token_stream); property_res.is_ok())
                    {
                        port->m_properties.push_back(property_res.get());
                    }
                    else
                    {
                        return ERR(property_res.get_error());
                    }
                }
                else if (next_token == "direction")
                {
                    const auto direction_token = m_token_stream.consume();
                    port->m_direction          = enum_from_string<PinDirection>(utils::to_lower(direction_token.string), PinDirection::none);
                    if (port->m_direction == PinDirection::none)
                    {
                        return ERR("could not parse interface: invalid port direction " + direction_token.string + " in line " + std::to_string(direction_token.number));
                    }
                }
                else
                {
                    return ERR("unsupported token '" + next_token.string + "' in line " + std::to_string(next_token.number));
                }

                m_token_stream.consume(")", true);
            }

            m_token_stream.consume(")", true);
            cell->m_ports_by_name[port->m_name] = port.get();
            cell->m_ports.push_back(std::move(port));
        }

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_contents(EdifCell* cell)
    {
        while (m_token_stream.consume("("))
        {
            const auto next_token = m_token_stream.consume();
            if (next_token == "instance")
            {
                if (auto res = parse_instance(cell); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse contents: failed to parse instance in line " + std::to_string(next_token.number));
                }
            }
            else if (next_token == "net")
            {
                if (auto res = parse_net(cell); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse contents: failed to parse net in line " + std::to_string(next_token.number));
                }
            }
            else
            {
                return ERR("unsupported token '" + next_token.string + "' in line " + std::to_string(next_token.number));
            }
            m_token_stream.consume(")", true);
        }

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_instance(EdifCell* cell)
    {
        auto instance          = std::make_unique<EdifInstance>();
        instance->m_library    = cell->m_library;
        const auto first_token = m_token_stream.peek();

        if (first_token != "(")
        {
            instance->m_name = m_token_stream.consume();
        }

        while (m_token_stream.consume("("))
        {
            const auto next_token = m_token_stream.consume();
            if (next_token == "rename")
            {
                const auto rename_res = parse_rename(m_token_stream, false);
                if (rename_res.is_error())
                {
                    return ERR(rename_res.get_error());
                }
                instance->m_name = rename_res.get();
            }
            else if (next_token == "viewRef")
            {
                m_token_stream.consume();    // view name
                m_token_stream.consume("(", true);
                m_token_stream.consume("cellRef", true);
                auto cell_name = m_token_stream.consume();

                // check for library ref
                if (m_token_stream.consume("("))
                {
                    m_token_stream.consume("libraryRef", true);
                    auto library_name = m_token_stream.consume();
                    if (const auto it = m_libraries_by_name.find(library_name.string); it != m_libraries_by_name.end())
                    {
                        instance->m_library = it->second;
                    }
                    else
                    {
                        return ERR("invalid library name '" + library_name.string + "' for instance '" + instance->m_name + "' within cell '" + cell->m_name + "' of library '"
                                   + cell->m_library->m_name + "'.");
                    }
                    m_token_stream.consume(")", true);
                }

                if (const auto it = instance->m_library->m_cells_by_name.find(cell_name); it != instance->m_library->m_cells_by_name.end())
                {
                    instance->m_cell = it->second;
                }
                else
                {
                    return ERR("invalid cell name '" + cell_name.string + "' for instance '" + instance->m_name + "' within cell '" + cell->m_name + "' of library '" + cell->m_library->m_name + "'.");
                }

                m_token_stream.consume(")", true);
            }
            else if (next_token == "property")
            {
                if (const auto property_res = parse_property(m_token_stream); property_res.is_ok())
                {
                    instance->m_properties.push_back(property_res.get());
                }
                else
                {
                    return ERR(property_res.get_error());
                }
            }
            else
            {
                return ERR("unsupported token '" + next_token.string + "' in line " + std::to_string(next_token.number));
            }
            m_token_stream.consume(")", true);
        }

        cell->m_instances_by_name[instance->m_name] = instance.get();
        cell->m_instances.push_back(std::move(instance));

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_net(EdifCell* cell)
    {
        std::string net_name;
        const auto first_token = m_token_stream.peek();

        if (first_token != "(")
        {
            net_name = m_token_stream.consume();
        }

        while (m_token_stream.consume("("))
        {
            const auto next_token = m_token_stream.consume();
            if (next_token == "rename")
            {
                const auto rename_res = parse_rename(m_token_stream, false);
                if (rename_res.is_error())
                {
                    return ERR(rename_res.get_error());
                }
                net_name = rename_res.get();
            }
            else if (next_token == "joined")
            {
                if (const auto ep_res = parse_endpoints(cell, net_name); ep_res.is_error())
                {
                    return ERR(ep_res.get_error());
                }
            }
            else
            {
                return ERR("unsupported token '" + next_token.string + "' in line " + std::to_string(next_token.number));
            }
            m_token_stream.consume(")", true);
        }

        cell->m_net_names.push_back(net_name);

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_endpoints(EdifCell* cell, const std::string& net_name)
    {
        while (m_token_stream.consume("("))
        {
            EdifEndpoint ep;

            u32 line_number;
            m_token_stream.consume("portRef", true);

            if (m_token_stream.peek() != "(")
            {
                const auto name_token = m_token_stream.consume();
                ep.m_port_name        = name_token.string;
                line_number           = name_token.number;
            }

            while (m_token_stream.consume("("))
            {
                const auto next_token = m_token_stream.consume();
                if (next_token == "member")
                {
                    const auto name_token  = m_token_stream.consume();
                    ep.m_port_name         = name_token.string;
                    line_number            = name_token.number;
                    const auto index_token = m_token_stream.consume();
                    try
                    {
                        ep.m_index = std::stoul(index_token.string);
                    }
                    catch (const std::exception& e)
                    {
                        return ERR("could not convert string '" + index_token.string + "' in line " + std::to_string(index_token.number) + " to integer: " + e.what());
                    }
                }
                else if (next_token == "instanceRef")
                {
                    ep.m_instance_name = m_token_stream.consume();
                }
                else
                {
                    return ERR("unsupported token '" + next_token.string + "' in line " + std::to_string(next_token.number));
                }
                m_token_stream.consume(")", true);
            }

            if (!ep.m_instance_name.empty())
            {
                if (const auto inst_it = cell->m_instances_by_name.find(ep.m_instance_name); inst_it != cell->m_instances_by_name.end())
                {
                    auto* inst      = inst_it->second;
                    auto* inst_cell = inst->m_cell;
                    if (const auto port_it = inst_cell->m_ports_by_name.find(ep.m_port_name); port_it != inst_cell->m_ports_by_name.end())
                    {
                        EdifPortAssignment pa;
                        pa.m_net_name = net_name;
                        pa.m_index    = ep.m_index;
                        pa.m_port     = port_it->second;
                        inst->m_port_assignments.push_back(pa);
                    }
                    else
                    {
                        return ERR("invalid port name '" + ep.m_port_name + "' given for port ref of instance '" + inst->m_name + "' in cell '" + cell->m_name + "' of library '"
                                   + cell->m_library->m_name + "' in line " + std::to_string(line_number));
                    }
                }
                else
                {
                    return ERR("invalid instance name '" + ep.m_instance_name + "' given for port ref in cell '" + cell->m_name + "' of library '" + cell->m_library->m_name + "' in line "
                               + std::to_string(line_number));
                }
            }
            else
            {
                if (const auto it = cell->m_ports_by_name.find(ep.m_port_name); it != cell->m_ports_by_name.end())
                {
                    EdifPortAssignment pa;
                    pa.m_net_name = net_name;
                    pa.m_index    = ep.m_index;
                    pa.m_port     = it->second;
                    cell->m_internal_port_assignments.push_back(pa);
                }
                else
                {
                    return ERR("invalid port name '" + ep.m_port_name + "' given for internal port ref in cell '" + cell->m_name + "' of library '" + cell->m_library->m_name + "'");
                }
            }

            m_token_stream.consume(")", true);
        }

        return OK({});
    }

    // ###########################################################################
    // ###########      Assemble Netlist from Intermediate Format       ##########
    // ###########################################################################

    Result<std::monostate> EdifParser::construct_netlist(EdifCell* top_cell)
    {
        m_netlist->set_design_name(top_cell->m_name);
        m_netlist->enable_automatic_net_checks(false);

        // preparations for alias: count the occurences of all names
        std::queue<EdifCell*> q;
        q.push(top_cell);

        const auto* gl = m_netlist->get_gate_library();

        while (!q.empty())
        {
            EdifCell* cell = q.front();
            q.pop();

            // collect and count all net names in the netlist
            for (const auto& net_name : cell->m_net_names)
            {
                m_net_name_occurences[net_name]++;
            }

            for (const auto& [instance_name, instance] : cell->m_instances_by_name)
            {
                m_instance_name_occurences[instance_name]++;

                // add type of instance to q if it is a module
                if (gl->get_gate_type_by_name(instance->m_cell->m_name) == nullptr)
                {
                    q.push(cell);
                }
            }
        }

        // for the top module, generate global i/o signals for all ports
        std::unordered_map<std::string, std::string> top_assignments;
        for (const auto& [_, port] : top_cell->m_ports_by_name)
        {
            for (const auto& expanded_name : port->m_expanded_names)
            {
                const auto signal_name = get_unique_alias("", expanded_name + "__GLOBAL_IO__", m_net_name_occurences);
                Net* global_port_net   = m_netlist->create_net(signal_name);
                if (global_port_net == nullptr)
                {
                    return ERR("could not construct netlist: failed to create global I/O net '" + signal_name + "'");
                }

                m_net_by_name[signal_name] = global_port_net;

                // assign global port nets to ports of top module
                top_assignments[expanded_name] = signal_name;

                if (port->m_direction == PinDirection::input || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_input_net())
                    {
                        return ERR("could not construct netlist: failed to mark global I/O net '" + signal_name + "' as global input");
                    }
                }

                if (port->m_direction == PinDirection::output || port->m_direction == PinDirection::inout)
                {
                    if (!global_port_net->mark_global_output_net())
                    {
                        return ERR("could not construct netlist: failed to mark global I/O net '" + signal_name + "' as global output");
                    }
                }
            }
        }

        if (auto res = instantiate_module("top_module", top_cell, nullptr, top_assignments); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not construct netlist: unable to instantiate top module");
        }

        // merge nets without gates in between them
        std::unordered_map<std::string, std::string> merged_nets;
        std::unordered_map<std::string, std::vector<std::string>> master_to_slaves;

        for (auto& [master, slave] : m_nets_to_merge)
        {
            // check if master net has already been merged into other net
            while (true)
            {
                if (const auto master_it = merged_nets.find(master); master_it != merged_nets.end())
                {
                    master = master_it->second;
                }
                else
                {
                    break;
                }
            }

            // check if slave net has already been merged into other net
            while (true)
            {
                if (const auto slave_it = merged_nets.find(slave); slave_it != merged_nets.end())
                {
                    slave = slave_it->second;
                }
                else
                {
                    break;
                }
            }

            auto master_net = m_net_by_name.at(master);
            auto slave_net  = m_net_by_name.at(slave);

            if (master_net == slave_net)
            {
                continue;
            }

            // merge sources
            if (slave_net->is_global_input_net())
            {
                master_net->mark_global_input_net();
            }

            for (auto src : slave_net->get_sources())
            {
                Gate* src_gate   = src->get_gate();
                GatePin* src_pin = src->get_pin();

                if (!slave_net->remove_source(src))
                {
                    return ERR("could not construct netlist: failed to remove source from net '" + slave_net->get_name() + "' with ID " + std::to_string(slave_net->get_id()));
                }

                if (!master_net->is_a_source(src_gate, src_pin))
                {
                    if (!master_net->add_source(src_gate, src_pin))
                    {
                        return ERR("could not construct netlist: failed to add source to net '" + master_net->get_name() + "' with ID " + std::to_string(master_net->get_id()));
                    }
                }
            }

            // merge destinations
            if (slave_net->is_global_output_net())
            {
                master_net->mark_global_output_net();
            }

            for (auto dst : slave_net->get_destinations())
            {
                Gate* dst_gate   = dst->get_gate();
                GatePin* dst_pin = dst->get_pin();

                if (!slave_net->remove_destination(dst))
                {
                    return ERR("could not construct netlist: failed to remove destination from net '" + slave_net->get_name() + "' with ID " + std::to_string(slave_net->get_id()));
                }

                if (!master_net->is_a_destination(dst_gate, dst_pin))
                {
                    if (!master_net->add_destination(dst_gate, dst_pin))
                    {
                        return ERR("could not construct netlist: failed to add destination to net '" + master_net->get_name() + "' with ID " + std::to_string(master_net->get_id()));
                    }
                }
            }

            // merge generics and attributes
            for (const auto& [identifier, content] : slave_net->get_data_map())
            {
                if (!master_net->set_data(std::get<0>(identifier), std::get<1>(identifier), std::get<0>(content), std::get<1>(content)))
                {
                    log_warning("edif_parser",
                                "unable to transfer data from slave net '{}' with ID {} to master net '{}' with ID {}.",
                                slave_net->get_name(),
                                slave_net->get_id(),
                                master_net->get_name(),
                                master_net->get_id());
                }
            }

            // update module ports
            if (const auto it = m_module_port_by_net.find(slave_net); it != m_module_port_by_net.end())
            {
                for (auto [module, index] : it->second)
                {
                    std::get<1>(m_module_ports.at(module).at(index)) = master_net;
                }
                m_module_port_by_net[master_net].insert(m_module_port_by_net[master_net].end(), it->second.begin(), it->second.end());
                m_module_port_by_net.erase(it);
            }

            m_netlist->delete_net(slave_net);
            m_net_by_name.erase(slave);
            merged_nets[slave] = master;
            master_to_slaves[master].push_back(slave);
        }

        // annotate all surviving master nets with the net names that where merged into them
        for (auto& master_net : m_netlist->get_nets())
        {
            const auto master_name = master_net->get_name();

            if (const auto m2s_it = master_to_slaves.find(master_name); m2s_it != master_to_slaves.end())
            {
                std::vector<std::vector<std::string>> merged_slaves;
                auto current_slaves = m2s_it->second;

                while (!current_slaves.empty())
                {
                    std::vector<std::string> next_slaves;
                    for (const auto& s : current_slaves)
                    {
                        if (const auto m2s_inner_it = master_to_slaves.find(s); m2s_inner_it != master_to_slaves.end())
                        {
                            next_slaves.insert(next_slaves.end(), m2s_inner_it->second.begin(), m2s_inner_it->second.end());
                        }
                    }

                    merged_slaves.push_back(current_slaves);
                    current_slaves = next_slaves;
                    next_slaves.clear();
                }

                // annotate all merged slave wire names as a JSON formatted list of list of strings
                // each net can span a tree of "consumed" slave wire names where the nth list represents all wire names that where merged at depth n
                std::string merged_str = "";
                bool has_merged_nets   = false;
                for (const auto& vec : merged_slaves)
                {
                    if (!vec.empty())
                    {
                        has_merged_nets = true;
                    }
                    const auto s = utils::join(", ", vec, [](const auto e) { return '"' + e + '"'; });
                    merged_str += "[" + s + "], ";
                }
                merged_str = merged_str.substr(0, merged_str.size() - 2);

                if (has_merged_nets)
                {
                    master_net->set_data("parser_annotation", "merged_nets", "string", "[" + merged_str + "]");
                }
            }
        }

        // add global GND gate if required by any instance
        if (m_netlist->get_gnd_gates().empty())
        {
            if (!m_zero_net->get_destinations().empty())
            {
                GateType* gnd_type  = m_gnd_gate_types.begin()->second;
                GatePin* output_pin = gnd_type->get_output_pins().front();
                Gate* gnd           = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");

                if (!m_netlist->mark_gnd_gate(gnd))
                {
                    return ERR("failed to mark GND gate");
                }

                if (m_zero_net->add_source(gnd, output_pin) == nullptr)
                {
                    return ERR("failed to add source to GND gate");
                }
            }
            else
            {
                m_netlist->delete_net(m_zero_net);
                m_zero_net = nullptr;
            }
        }

        // add global VCC gate if required by any instance
        if (m_netlist->get_vcc_gates().empty())
        {
            if (!m_one_net->get_destinations().empty())
            {
                GateType* vcc_type  = m_vcc_gate_types.begin()->second;
                GatePin* output_pin = vcc_type->get_output_pins().front();
                Gate* vcc           = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");

                if (!m_netlist->mark_vcc_gate(vcc))
                {
                    return ERR("failed to mark VCC gate");
                }

                if (m_one_net->add_source(vcc, output_pin) == nullptr)
                {
                    return ERR("failed to add source to VCC gate");
                }
            }
            else
            {
                m_netlist->delete_net(m_one_net);
                m_one_net = nullptr;
            }
        }

        // update module nets, internal nets, input nets, and output nets
        for (Module* module : m_netlist->get_modules())
        {
            module->update_nets();
        }

        // assign module pins
        for (const auto& [module, ports] : m_module_ports)
        {
            std::unordered_set<Net*> input_nets  = module->get_input_nets();
            std::unordered_set<Net*> output_nets = module->get_output_nets();

            for (const auto& [port_name, port_net] : ports)
            {
                if (!module->is_input_net(port_net) && !module->is_output_net(port_net))
                {
                    continue;
                }

                if (auto res = module->create_pin(port_name, port_net); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not construct netlist: failed to create pin '" + port_name + "' at net '" + port_net->get_name() + "' with ID " + std::to_string(port_net->get_id())
                                          + " within module '" + module->get_name() + "' with ID " + std::to_string(module->get_id()));
                }
            }
        }

        m_netlist->enable_automatic_net_checks(true);
        return OK({});
    }

    Result<Module*> EdifParser::instantiate_module(const std::string& instance_name, EdifCell* cell, Module* parent, const std::unordered_map<std::string, std::string>& parent_module_assignments)
    {
        std::unordered_map<std::string, std::string> signal_alias;
        std::unordered_map<std::string, std::string> instance_alias;

        const std::string parent_name = (parent == nullptr) ? "" : parent->get_name();
        instance_alias[instance_name] = get_unique_alias(parent_name, instance_name, m_instance_name_occurences);

        // create netlist module
        Module* module;
        if (parent == nullptr)
        {
            module = m_netlist->get_top_module();
            module->set_name(instance_alias.at(instance_name));
        }
        else
        {
            module = m_netlist->create_module(instance_alias.at(instance_name), parent);
        }

        std::string instance_type = cell->m_name;
        if (module == nullptr)
        {
            return ERR("could not create instance '" + instance_name + "' of type '" + instance_type + "': failed to create module");
        }
        module->set_type(instance_type);

        // assign module port names
        for (const auto& port : cell->m_ports)
        {
            for (const auto& expanded_name : port->m_expanded_names)
            {
                if (const auto it = parent_module_assignments.find(expanded_name); it != parent_module_assignments.end())
                {
                    Net* port_net = m_net_by_name.at(it->second);
                    m_module_ports[module].push_back(std::make_tuple(expanded_name, port_net));
                    m_module_port_by_net[port_net].push_back(std::make_pair(module, m_module_ports[module].size() - 1));
                }
            }
        }

        // create internal signals
        for (const auto& net_name : cell->m_net_names)
        {
            signal_alias[net_name] = get_unique_alias(module->get_name(), net_name, m_net_name_occurences);

            // create new net for the signal
            Net* signal_net = m_netlist->create_net(signal_alias.at(net_name));
            if (signal_net == nullptr)
            {
                return ERR("could not create instance '" + instance_name + "' of type '" + instance_type + "': failed to create net '" + net_name + "'");
            }

            m_net_by_name[signal_alias.at(net_name)] = signal_net;
        }

        for (const auto& pa : cell->m_internal_port_assignments)
        {
            if (const auto it = parent_module_assignments.find(pa.m_port->m_name); it != parent_module_assignments.end())
            {
                const auto& ext_net_name = std::get<1>(*it);
                if (const auto alias_it = signal_alias.find(pa.m_net_name); alias_it != signal_alias.end())
                {
                    const auto& int_net_name = std::get<1>(*alias_it);
                    const bool swap          = ext_net_name.find("__GLOBAL_IO__") == std::string::npos;
                    m_nets_to_merge.push_back(swap ? std::make_pair(ext_net_name, int_net_name) : std::make_pair(int_net_name, ext_net_name));
                }
            }
        }

        // TODO create instances and assign nets

        return ERR("not implemented");
    }

    // ###########################################################################
    // ###################          Helper Functions          ####################
    // ###########################################################################

    Result<std::string> EdifParser::parse_rename(TokenStream<std::string>& stream, bool enforce_match)
    {
        auto new_name = stream.consume();
        auto old_name = stream.consume();
        if (old_name.string.substr(1, old_name.string.size() - 2) != new_name.string)
        {
            if (enforce_match)
            {
                return ERR("could not parse instance in line " + std::to_string(old_name.number) + ": renaming not yet supported");
            }
            else
            {
                log_warning("edif_parser", "renaming not yet supported, ignoring old name in line {}", old_name.number);
            }
        }

        return OK(new_name);
    }

    Result<std::pair<std::string, u32>> EdifParser::parse_array(TokenStream<std::string>& stream)
    {
        std::string name;
        u32 width;

        if (stream.consume("("))
        {
            stream.consume("rename", true);
            const auto rename_res = parse_rename(stream, false);
            if (rename_res.is_error())
            {
                return ERR(rename_res.get_error());
            }
            name = rename_res.get();
            stream.consume(")", true);
        }
        else
        {
            name = stream.consume();
        }

        const auto width_token = stream.consume();
        try
        {
            width = std::stoul(width_token.string);
        }
        catch (const std::exception& e)
        {
            return ERR("could not convert string '" + width_token.string + "' in line " + std::to_string(width_token.number) + " to integer: " + e.what());
        }

        return OK(std::make_pair(std::move(name), width));
    }

    Result<EdifParser::EdifProperty> EdifParser::parse_property(TokenStream<std::string>& stream)
    {
        EdifProperty prop;
        prop.m_name = stream.consume();
        stream.consume("(", true);
        prop.m_type  = stream.consume();
        prop.m_value = stream.consume();
        stream.consume(")", true);
        return OK(prop);
    }

    std::string EdifParser::get_unique_alias(const std::string& parent_name, const std::string& name, const std::unordered_map<std::string, u32>& name_occurences) const
    {
        std::string unique_alias = name;

        if (!parent_name.empty())
        {
            // if there is no other instance with that name, we omit the name prefix
            if (const auto instance_name_it = name_occurences.find(name); instance_name_it != name_occurences.end() && instance_name_it->second > 1)
            {
                unique_alias = parent_name + instance_name_seperator + unique_alias;
            }
        }

        return unique_alias;
    }

}    // namespace hal
