#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/token_stream.h"
#include "xilinx_toolbox/plugin_xilinx_toolbox.h"
#include "xilinx_toolbox/types.h"

#include <fstream>
#include <regex>

namespace hal
{
    namespace xilinx_toolbox
    {
        // TODO ignore comments
        // TODO there is some weird escaping with curly braces happening
        TokenStream<std::string> tokenize(std::stringstream& ss)
        {
            const std::string delimiters = " ";
            std::string current_token;
            u32 line_number = 0;

            std::string line;
            char prev_char = 0;
            bool escaped   = false;

            std::vector<Token<std::string>> parsed_tokens;
            while (std::getline(ss, line))
            {
                line_number++;

                for (char c : line)
                {
                    // deal with escaping and strings
                    if (c == '\\')
                    {
                        escaped = true;
                        continue;
                    }
                    else if (escaped && std::isspace(c))
                    {
                        escaped = false;
                        continue;
                    }

                    if (((!std::isspace(c) && delimiters.find(c) == std::string::npos) || escaped))
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

            return TokenStream(parsed_tokens, {}, {});
        }

        Result<LOC> parse_LOC(TokenStream<std::string>& ts)
        {
            const auto loc_token = ts.consume();
            std::string loc_str  = loc_token.string;

            LOC new_loc;
            new_loc.loc_name = loc_str;

            // test for pin names
            const auto pin_pattern = std::regex("^[a-zA-Z]\\d+$");
            std::smatch match;
            if (std::regex_match(loc_str, match, pin_pattern)) {
                return OK(new_loc);
            }

            // test for slice type + x and y coordinates
            const std::string loc_type_str = loc_str.substr(0, loc_str.find_first_of("_"));

            if (is_valid_enum<LOCType>(loc_type_str))
            {
                new_loc.loc_type = enum_from_string<LOCType>(loc_type_str);
            }
            else
            {
                return ERR("cannot parse LOC: encountered unknown LOC type '" + loc_type_str + "'" + " at line " + std::to_string(loc_token.number));
            }

            loc_str = loc_str.substr(loc_type_str.size());
            loc_str = loc_str.substr(std::string("_X").size());

            const std::string x_str = loc_str.substr(0, loc_str.find_first_of("Y"));

            loc_str = loc_str.substr(x_str.size());
            loc_str = loc_str.substr(std::string("Y").size());

            const std::string y_str = loc_str;

            if (const auto res = utils::wrapped_stoull(x_str); res.is_ok())
            {
                new_loc.loc_x = res.get();
            }
            else
            {
                return ERR_APPEND(res.get_error(), "cannot parse LOC: failed to extract x position form " + loc_token.string + " at line " + std::to_string(loc_token.number));
            }

            if (const auto res = utils::wrapped_stoull(y_str); res.is_ok())
            {
                new_loc.loc_y = res.get();
            }
            else
            {
                return ERR_APPEND(res.get_error(), "cannot parse LOC: failed to extract y position form " + loc_token.string + " at line " + std::to_string(loc_token.number));
            }

            return OK(new_loc);
        }

        Result<std::string> parse_single_cell(TokenStream<std::string>& ts)
        {
            const auto loc_token = ts.consume();
            if(loc_token.string != "[get_cells")
            {
                return ERR("cannot parse cell name: expected token " + loc_token.string + " at line " + std::to_string(loc_token.number) + " to be exactly '[get_cells'");
            }

            const auto cell_token = ts.consume();
            std::string cell_str  = cell_token.string;

            // TODO handle excaping with curly braces in tokenize()
            // if (cell_str.substr(0, 1) != "{")
            // {
            //     return ERR("cannot parse cell name: expected cell token '" + cell_str + " at line " + std::to_string(loc_token.number) + "' to start with '{'");
            // }

            // if (cell_str.substr(cell_str.size() - 2, 2) != "}]")
            // {
            //     return ERR("cannot parse cell name: expected cell token '" + cell_str + " at line " + std::to_string(loc_token.number) + "' to end with '}]'");
            // }

            bool is_escpaed = cell_str.substr(0, 1) == "{" && cell_str.substr(cell_str.size() - 2, 1) == "}";

            if (cell_str.substr(cell_str.size() - 1, 1) != "]")
            {
                return ERR("cannot parse cell name: expected cell token '" + cell_str + " at line " + std::to_string(loc_token.number) + "' to end with ']'");
            }

            cell_str = cell_str.substr(is_escpaed ? 1 : 0, cell_str.size() - (is_escpaed ? 2 : 1));

            return OK(cell_str);
        }

        Result<std::unordered_map<std::string, CellData>> parse_tokens(TokenStream<std::string>& ts)
        {
            std::unordered_map<std::string, CellData> cell_data;

            while (true)
            {
                ts.consume_until("set_property");
                if (ts.remaining() == 0)
                {
                    break;
                }

                ts.consume("set_property");

                if (ts.peek().string == "BEL")
                {
                    ts.consume("BEL");

                    const auto bel_type = enum_from_string<BELType>(ts.consume().string);

                    if (const auto res = parse_single_cell(ts); res.is_ok())
                    {
                        cell_data[res.get()].bel_type = bel_type;
                    }
                    else
                    {
                        return ERR_APPEND(res.get_error(), "cannot parse xdc token stream: failed to extract cell name");
                    }
                }
                else if (ts.peek().string == "LOC")
                {
                    ts.consume("LOC");

                    LOC new_loc;
                    if (const auto res = parse_LOC(ts); res.is_ok())
                    {
                        new_loc = res.get();
                    }
                    else
                    {
                        return ERR_APPEND(res.get_error(), "cannot parse xdc token stream: failed to extract LOC");
                    }

                    if (const auto res = parse_single_cell(ts); res.is_ok())
                    {
                        cell_data[res.get()].loc = new_loc;
                    }
                    else
                    {
                        return ERR_APPEND(res.get_error(), "cannot parse xdc token stream: failed to extract cell name");
                    }
                }
                else
                {
                    ts.consume_current_line();
                }
            }

            return OK(cell_data);
        }
   
        // This might depend on the exact xilinx device (family) we are dealing with
        Result<std::pair<u32, u32>> reconstruct_coordinates(const CellData& cell_data)
        {
            // For the xilinx 7series boards we define that each Slice has 4 colums and 8 rows
            static std::map<BELType, std::pair<u32, u32>> bel_to_offset = {
                {BELType::A6LUT, {0, 0}},
                {BELType::B6LUT, {0, 2}},
                {BELType::C6LUT, {0, 4}},
                {BELType::D6LUT, {0, 6}},
                {BELType::A5LUT, {0, 1}},
                {BELType::B5LUT, {0, 3}},
                {BELType::C5LUT, {0, 5}},
                {BELType::D5LUT, {0, 7}},

                {BELType::F7AMUX, {1, 0}},
                {BELType::F7BMUX, {1, 2}},
                {BELType::F8MUX, {1, 1}},

                {BELType::CARRY4, {2, 0}},

                {BELType::AFF, {3, 0}},
                {BELType::BFF, {3, 2}},
                {BELType::CFF, {3, 4}},
                {BELType::DFF, {3, 6}},
                {BELType::A5FF, {3, 1}},
                {BELType::B5FF, {3, 3}},
                {BELType::C5FF, {3, 5}},
                {BELType::D5FF, {3, 7}},

                {BELType::BUFG, {0, 0}},
                {BELType::INBUF_EN, {0, 0}},
                {BELType::OUTBUF, {1, 0}},
            };
        }
    }    // namespace xilinx_toolbox

    Result<std::monostate> XilinxToolboxPlugin::parse_xdc_file(Netlist* nl, const std::filesystem::path& xdc_file)
    {
        std::stringstream ss;
        std::ifstream ifs;
        ifs.open(xdc_file.string(), std::ifstream::in);
        if (!ifs.is_open())
        {
            return ERR("could not parse xdc file '" + xdc_file.string() + "' : unable to open file");
        }
        ss << ifs.rdbuf();
        ifs.close();

        auto ts = xilinx_toolbox::tokenize(ss);

        std::unordered_map<std::string, xilinx_toolbox::CellData> cell_data;
        // parse tokens
        try
        {
            if (auto res = xilinx_toolbox::parse_tokens(ts); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse xdc '" + xdc_file.string() + "': unable to parse tokens");
            }
            else
            {
                cell_data = res.get();
            }
        }
        catch (TokenStream<std::string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                return ERR("could not parse xdc '" + xdc_file.string() + "': " + e.message + " (line " + std::to_string(e.line_number) + ")");
            }
            else
            {
                return ERR("could not parse xdc '" + xdc_file.string() + "': " + e.message);
            }
        }

        std::unordered_map<std::string, Gate*> gate_name_to_gate;
        for (const auto g : nl->get_gates())
        {
            gate_name_to_gate[g->get_name()] = g;
        }

        for (const auto& [gate_name, cd] : cell_data)
        {
            if (const auto it = gate_name_to_gate.find(gate_name); it == gate_name_to_gate.end())
            {   
                log_error("xilinx_toolbox", "Found gate name {} in xdc file but not in netlist!", gate_name);
                continue;
            }
            else
            {
                /*
                if (cd.loc.has_value())
                {
                    it->second->set_data("placement_information", "LOC", "string", "{type: " + enum_to_string<xilinx_toolbox::LOCType>(cd.loc.value().loc_type) + ", X: " + std::to_string(cd.loc.value().loc_x) + ", Y: " + std::to_string(cd.loc.value().loc_y) + "}");
                }

                if (cd.bel_type.has_value())
                {

                }
                */
            }

            std::cout << gate_name << ": " << (cd.bel_type.has_value() ? enum_to_string(cd.bel_type.value()) : "NONE") << " - "
                      << (cd.loc.has_value() ? (enum_to_string(cd.loc.value().loc_type) + std::to_string(cd.loc.value().loc_x) + std::to_string(cd.loc.value().loc_y)) : "NONE") << std::endl;
        }

        return OK({});
    }

}    // namespace hal