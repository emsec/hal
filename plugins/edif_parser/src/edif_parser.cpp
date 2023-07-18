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
    namespace
    {

    }    // namespace

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

        return ERR("not implemented");
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
        const auto lib_name = m_token_stream.consume();
        while (m_token_stream.consume("("))
        {
            const auto next_token = m_token_stream.consume();
            if (next_token == "cell")
            {
                if (auto res = parse_cell(); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse library '" + lib_name.string + "' in line " + std::to_string(lib_name.number) + ": failed to parse cell");
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

    Result<std::monostate> EdifParser::parse_cell()
    {
        EdifCell cell;
        const auto first_token = m_token_stream.peek();
        u32 line_number        = first_token.number;

        if (first_token != "(")
        {
            cell.m_name = m_token_stream.consume();
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
                cell.m_name = rename_res.get();
            }
            else if (next_token == "view")
            {
                if (auto res = parse_view(cell); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse cell '" + cell.m_name + "' in line " + std::to_string(next_token.number) + ": failed to parse view");
                }
            }
            else
            {
                m_token_stream.consume_until(")");
            }
            m_token_stream.consume(")", true);
        }

        if (cell.m_name.empty())
        {
            return ERR("could not parse cell in line " + std::to_string(line_number) + ": no name given for cell");
        }

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_view(EdifCell& cell)
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

    Result<std::monostate> EdifParser::parse_interface(EdifCell& cell)
    {
        while (m_token_stream.consume("("))
        {
            m_token_stream.consume("port", true);

            EdifPort port;

            if (m_token_stream.consume("("))
            {
                const auto next_token = m_token_stream.consume();
                if (next_token == "rename")
                {
                    const auto rename_res = parse_rename(m_token_stream);
                    if (rename_res.is_error())
                    {
                        return ERR(rename_res.get_error());
                    }
                    port.m_name = rename_res.get();
                }
                else if (next_token == "array")
                {
                    const auto rename_res = parse_array(m_token_stream);
                    if (rename_res.is_error())
                    {
                        return ERR(rename_res.get_error());
                    }
                    const auto array = rename_res.get();
                    port.m_name      = array.first;
                    port.m_width     = array.second;
                }
                else
                {
                    return ERR("unsupported token '" + next_token.string + "' in line " + std::to_string(next_token.number));
                }
                m_token_stream.consume(")", true);
            }
            else
            {
                port.m_name = m_token_stream.consume();
            }

            m_token_stream.consume("(", true);
            m_token_stream.consume("direction", true);
            const auto direction_token = m_token_stream.consume();
            port.m_direction           = enum_from_string<PinDirection>(utils::to_lower(direction_token.string), PinDirection::none);
            if (port.m_direction == PinDirection::none)
            {
                return ERR("could not parse interface: invalid port direction " + direction_token.string + " in line " + std::to_string(direction_token.number));
            }
            m_token_stream.consume(")", true);
            m_token_stream.consume(")", true);
            cell.m_ports.push_back(port);
        }

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_contents(EdifCell& cell)
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

    Result<std::monostate> EdifParser::parse_instance(EdifCell& cell)
    {
        EdifInstance instance;
        const auto first_token = m_token_stream.peek();

        if (first_token != "(")
        {
            instance.m_name = m_token_stream.consume();
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
                instance.m_name = rename_res.get();
            }
            else if (next_token == "viewRef")
            {
                m_token_stream.consume();    // view name
                m_token_stream.consume("(", true);
                m_token_stream.consume("cellRef", true);
                instance.m_type = m_token_stream.consume();
                m_token_stream.consume_until(")");    // skip libraryRef
                m_token_stream.consume(")", true);
            }
            else if (next_token == "property")
            {
                EdifProperty prop;
                prop.m_name = m_token_stream.consume();
                m_token_stream.consume("(", true);
                prop.m_type  = m_token_stream.consume();
                prop.m_value = m_token_stream.consume();
                m_token_stream.consume(")", true);
                instance.m_properties.push_back(prop);
            }
            else
            {
                return ERR("unsupported token '" + next_token.string + "' in line " + std::to_string(next_token.number));
            }
            m_token_stream.consume(")", true);
        }

        cell.m_instances.push_back(std::move(instance));

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_net(EdifCell& cell)
    {
        EdifNet net;
        const auto first_token = m_token_stream.peek();

        if (first_token != "(")
        {
            net.m_name = m_token_stream.consume();
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
                net.m_name = rename_res.get();
            }
            else if (next_token == "joined")
            {
                if (const auto ep_res = parse_endpoints(net); ep_res.is_error())
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

        cell.m_nets.push_back(std::move(net));

        return OK({});
    }

    Result<std::monostate> EdifParser::parse_endpoints(EdifNet& net)
    {
        while (m_token_stream.consume("("))
        {
            m_token_stream.consume("portRef", true);
            EdifEndpoint ep;

            if (!m_token_stream.consume("("))
            {
                ep.m_port_name = m_token_stream.consume();
            }

            while (m_token_stream.consume("("))
            {
                const auto next_token = m_token_stream.consume();
                if (next_token == "member")
                {
                    ep.m_port_name         = m_token_stream.consume();
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

            net.m_endpoints.push_back(std::move(ep));
            m_token_stream.consume(")", true);
        }

        return OK({});
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
            const auto rename_res = parse_rename(stream);
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

}    // namespace hal
