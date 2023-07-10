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
        const std::string delimiters = "() \"";
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
        u32 line_number;

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
        const auto first_token = m_token_stream.consume();
        if (first_token != "(")
        {
            cell.m_name = m_token_stream.consume();
        }
        else
        {
            m_token_stream.consume("rename", true);
        }

        return OK({});
    }

    // ###########################################################################
    // ###################          Helper Functions          ####################
    // ###########################################################################

}    // namespace hal
