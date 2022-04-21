#include "liberty_parser/liberty_parser.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/utilities/log.h"

#include <fstream>

// TODO remove LUT parsing

namespace hal
{
    Result<std::unique_ptr<GateLibrary>> LibertyParser::parse(const std::filesystem::path& file_path)
    {
        m_path = file_path;

        {
            std::ifstream ifs;
            ifs.open(m_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                return ERR("could not parse Liberty file '" + m_path.string() + "' : unable to open file");
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
                return ERR_APPEND(res.get_error(), "could not parse Liberty file '" + file_path.string() + "': unable to parse tokens");
            }
        }
        catch (TokenStream<std::string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                return ERR("could not parse Liberty file '" + m_path.string() + "': " + e.message + " (line " + std::to_string(e.line_number) + ")");
            }
            else
            {
                return ERR("could not parse Liberty file '" + m_path.string() + "': " + e.message);
            }
        }

        return OK(std::move(m_gate_lib));
    }

    void LibertyParser::tokenize()
    {
        std::string delimiters = "{}()[];:\",";
        std::string current_token;
        u32 line_number = 0;

        std::string line;
        bool in_string          = false;
        bool was_in_string      = false;
        bool multi_line_comment = false;

        std::vector<Token<std::string>> parsed_tokens;

        while (std::getline(m_fs, line))
        {
            line_number++;
            this->remove_comments(line, multi_line_comment);

            for (char c : line)
            {
                if (c == '\"')
                {
                    was_in_string = true;
                    in_string     = !in_string;
                    continue;
                }

                if (std::isspace(c) && !in_string)
                {
                    continue;
                }

                if (delimiters.find(c) == std::string::npos || in_string)
                {
                    current_token += c;
                }
                else
                {
                    if (was_in_string || !current_token.empty())
                    {
                        parsed_tokens.emplace_back(line_number, current_token);
                        current_token.clear();
                        was_in_string = false;
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

        m_token_stream = TokenStream<std::string>(parsed_tokens, {"(", "{"}, {")", "}"});
    }

    Result<std::monostate> LibertyParser::parse_tokens()
    {
        m_token_stream.consume("library", true);
        m_token_stream.consume("(", true);
        auto lib_name = m_token_stream.consume();
        m_token_stream.consume(")", true);
        m_token_stream.consume("{", true);
        m_gate_lib       = std::make_unique<GateLibrary>(m_path, lib_name.string);
        auto library_str = m_token_stream.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, false);
        m_token_stream.consume("}", false);

        do
        {
            auto next_token = library_str.consume();
            if (next_token == "{")
            {
                library_str.consume_until("}");
                library_str.consume("}");
            }
            else if (next_token == "cell" && library_str.peek() == "(")
            {
                if (auto cell = parse_cell(library_str); cell.is_error())
                {
                    return ERR_APPEND(cell.get_error(), "could not parse tokens: unable to parse cell (line " + std::to_string(next_token.number) + ")");
                }
                else
                {
                    if (auto res = construct_gate_type(cell.get()); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse tokens: unable to construct gate type (line " + std::to_string(next_token.number) + ")");
                    }
                }
            }
            else if (next_token == "type" && library_str.peek() == "(")
            {
                if (auto res = parse_type(library_str); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens: unable to parse type (line " + std::to_string(next_token.number) + ")");
                }
                else
                {
                    auto type              = res.get();
                    m_bus_types[type.name] = std::move(type);
                }
            }
        } while (library_str.remaining() > 0);

        if (const u32 unparsed = m_token_stream.remaining(); unparsed == 0)
        {
            return OK({});
        }
        else
        {
            return ERR("could not parse tokens: " + std::to_string(unparsed) + " unparsed tokens remaining");
        }
    }

    Result<LibertyParser::type_group> LibertyParser::parse_type(TokenStream<std::string>& str)
    {
        type_group type;
        type.line_number = str.peek().number;
        str.consume("(", true);
        type.name = str.consume().string;
        str.consume(")", true);
        str.consume("{", true);
        auto type_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

        type.start_index = 0;
        type.width       = 1;
        type.ascending   = true;
        while (type_str.remaining() > 0)
        {
            auto next_token = type_str.consume();
            if (next_token == "{")
            {
                type_str.consume_until("}");
                type_str.consume("}");
            }
            else if (next_token == "base_type")
            {
                type_str.consume(":", true);
                type_str.consume("array", true);
            }
            else if (next_token == "data_type")
            {
                type_str.consume(":", true);
                type_str.consume("bit", true);
            }
            else if (next_token == "bit_width")
            {
                type_str.consume(":", true);
                type.width = std::stol(type_str.consume().string);
            }
            else if (next_token == "bit_from")
            {
                type_str.consume(":", true);
                type.start_index = std::stol(type_str.consume().string);
            }
            else if (next_token == "bit_to")
            {
                type_str.consume(":", true);
            }
            else if (next_token == "downto")
            {
                type_str.consume(":", true);
                auto bval = type_str.consume();
                if (bval == "false")
                {
                    type.ascending = true;
                }
                else if (bval == "true")
                {
                    type.ascending = false;
                }
                else
                {
                    return ERR("could not parse type '" + type.name + "': invalid Boolean value '" + bval.string + "' (line " + std::to_string(bval.number) + ")");
                }
            }
            else
            {
                return ERR("could not parse type '" + type.name + "': invalid token '" + next_token.string + "' (line " + std::to_string(next_token.number) + ")");
            }
            type_str.consume(";", true);
        }
        return OK(type);
    }

    Result<LibertyParser::cell_group> LibertyParser::parse_cell(TokenStream<std::string>& str)
    {
        cell_group cell;

        cell.line_number = str.peek().number;
        str.consume("(", true);
        cell.name = str.consume().string;
        str.consume(")", true);
        str.consume("{", true);
        auto cell_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

        if (const auto cell_it = m_cell_names.find(cell.name); cell_it != m_cell_names.end())
        {
            return ERR("could not parse cell '" + cell.name + "': a cell with that name already exists");
        }

        m_cell_names.insert(cell.name);

        while (cell_str.remaining() > 0)
        {
            auto next_token = cell_str.consume();
            if (next_token == "{")
            {
                cell_str.consume_until("}");
                cell_str.consume("}");
            }
            else if (next_token == "pin")
            {
                if (auto pin = parse_pin(cell_str, cell); pin.is_error())
                {
                    return ERR_APPEND(pin.get_error(), "could not parse cell '" + cell.name + "': failed to parse 'pin' group (line " + std::to_string(next_token.number) + ")");
                }
                else
                {
                    cell.pins.push_back(pin.get());
                }
            }
            else if (next_token == "pg_pin")
            {
                if (auto pin = parse_pg_pin(cell_str, cell); pin.is_ok())
                {
                    cell.pins.push_back(pin.get());
                }
            }
            else if (next_token == "bus")
            {
                if (auto res = parse_bus(cell_str, cell); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not parse cell '" + cell.name + "': failed to parse 'bus' group (line " + std::to_string(next_token.number) + ")");
                }
                else
                {
                    auto bus             = res.get();
                    cell.buses[bus.name] = std::move(bus);
                }
            }
            else if (next_token == "ff")
            {
                if (auto ff = parse_ff(cell_str); ff.is_error())
                {
                    return ERR_APPEND(ff.get_error(), "could not parse cell '" + cell.name + "': failed to parse 'ff' group (line " + std::to_string(next_token.number) + ")");
                }
                else
                {
                    cell.properties.insert(GateTypeProperty::ff);
                    cell.properties.insert(GateTypeProperty::sequential);
                    cell.ff = ff.get();
                }
            }
            else if (next_token == "latch")
            {
                if (auto latch = parse_latch(cell_str); latch.is_error())
                {
                    return ERR_APPEND(latch.get_error(), "could not parse cell '" + cell.name + "': failed to parse 'latch' group (line " + std::to_string(next_token.number) + ")");
                }
                else
                {
                    cell.properties.insert(GateTypeProperty::latch);
                    cell.properties.insert(GateTypeProperty::sequential);
                    cell.latch = latch.get();
                }
            }
        }

        return OK(cell);
    }

    Result<LibertyParser::pin_group> LibertyParser::parse_pin(TokenStream<std::string>& str, cell_group& cell, PinDirection direction, const std::string& external_pin_name)
    {
        pin_group pin;

        pin.line_number = str.peek().number;
        str.consume("(", true);
        auto pin_names_str = str.extract_until(")", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume(")", true);
        str.consume("{", true);
        auto pin_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

        if (pin_names_str.size() == 0)
        {
            return ERR("could not parse pin: no pin name given (line " + std::to_string(pin.line_number) + ")");
        }

        do
        {
            std::string name = pin_names_str.consume().string;
            if (!external_pin_name.empty() && name != external_pin_name)
            {
                return ERR("could not parse pin '" + name + "': pin name does not match external pin name '" + external_pin_name + "' (line " + std::to_string(pin.line_number) + ")");
            }

            if (pin_names_str.consume("["))
            {
                if (pin_names_str.peek(1) == ":")
                {
                    i32 start = std::stol(pin_names_str.consume().string);
                    pin_names_str.consume(":", true);
                    i32 end = std::stol(pin_names_str.consume().string);
                    i32 dir = (start <= end) ? 1 : -1;

                    for (int i = start; i != (end + dir); i += dir)
                    {
                        auto new_name = name + "(" + std::to_string(i) + ")";

                        if (const auto pin_it = cell.pin_names.find(new_name); pin_it != cell.pin_names.end())
                        {
                            return ERR("could not parse pin '" + new_name + "': a pin with that name already exists (line " + std::to_string(pin.line_number) + ")");
                        }

                        cell.pin_names.insert(new_name);
                        pin.pin_names.push_back(std::move(new_name));
                    }
                }
                else
                {
                    u32 index     = std::stoul(pin_names_str.consume().string);
                    auto new_name = name + "(" + std::to_string(index) + ")";

                    if (const auto pin_it = cell.pin_names.find(new_name); pin_it != cell.pin_names.end())
                    {
                        return ERR("could not parse pin '" + new_name + "': a pin with that name already exists (line " + std::to_string(pin.line_number) + ")");
                    }

                    cell.pin_names.insert(new_name);
                    pin.pin_names.push_back(std::move(new_name));
                }

                pin_names_str.consume("]", true);
            }
            else
            {
                if (const auto pin_it = cell.pin_names.find(name); pin_it != cell.pin_names.end())
                {
                    return ERR("could not parse pin '" + name + "': a pin with that name already exists (line " + std::to_string(pin.line_number) + ")");
                }

                cell.pin_names.insert(name);
                pin.pin_names.push_back(std::move(name));
            }

            pin_names_str.consume(",", pin_names_str.remaining() > 0);
        } while (pin_names_str.remaining() > 0);

        pin.direction = direction;

        while (pin_str.remaining() > 0)
        {
            auto next_token = pin_str.consume();
            if (next_token == "direction")
            {
                pin_str.consume(":", true);
                auto direction_str = pin_str.consume().string;
                try
                {
                    pin.direction = enum_from_string<PinDirection>(direction_str);
                }
                catch (const std::runtime_error&)
                {
                    return ERR("could not parse pin: invalid pin direction '" + direction_str + "' (line " + std::to_string(pin.line_number) + ")");
                }
                pin_str.consume(";", true);
            }
            else if (next_token == "function")
            {
                pin_str.consume(":", true);
                pin.function = pin_str.consume().string;
                pin_str.consume(";", true);
            }
            else if (next_token == "x_function")
            {
                pin_str.consume(":", true);
                pin.x_function = pin_str.consume().string;
                pin_str.consume(";", true);
            }
            else if (next_token == "three_state")
            {
                pin_str.consume(":", true);
                pin.z_function = pin_str.consume().string;
                pin_str.consume(";", true);
            }
            else if (next_token == "clock")
            {
                pin_str.consume(":", true);
                if (pin_str.consume() == "true")
                {
                    pin.clock = true;
                }
                pin_str.consume(";", true);
            }
        }

        if (pin.direction == PinDirection::none)
        {
            return ERR("could not parse pin: no pin direction given (line " + std::to_string(pin.line_number) + ")");
        }

        return OK(pin);
    }

    Result<LibertyParser::pin_group> LibertyParser::parse_pg_pin(TokenStream<std::string>& str, cell_group& cell)
    {
        pin_group pin;

        pin.line_number = str.peek().number;
        str.consume("(", true);
        TokenStream<std::string> pin_names_str = str.extract_until(")", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume(")", true);
        str.consume("{", true);
        TokenStream<std::string> pin_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

        if (pin_names_str.size() == 0)
        {
            return ERR("could not parse power/ground pin: no pin name given (line " + std::to_string(pin.line_number) + ")");
        }
        else if (pin_names_str.size() > 1)
        {
            return ERR("could not parse power/ground pins '" + pin_names_str.join("").string + "': more than one pin name given (line " + std::to_string(pin.line_number) + ")");
        }

        std::string name = pin_names_str.consume().string;
        if (const auto pin_it = cell.pin_names.find(name); pin_it != cell.pin_names.end())
        {
            return ERR("could not parse power/ground pin '" + name + "': a pin with that name already exists (line " + std::to_string(pin.line_number) + ")");
        }

        while (pin_str.remaining() > 0)
        {
            auto next_token = pin_str.consume();
            if (next_token == "pg_type")
            {
                pin_str.consume(":", true);
                std::string type = pin_str.consume().string;
                if (type == "primary_power")
                {
                    pin.power = true;
                }
                else if (type == "primary_ground")
                {
                    pin.ground = true;
                }
                else
                {
                    return ERR("could not parse power/ground pin '" + name + "': invalid pin type '" + type + "' (line " + std::to_string(pin.line_number) + ")");
                }
                pin_str.consume(";", true);
            }
        }

        pin.direction = PinDirection::input;
        cell.pin_names.insert(name);
        pin.pin_names.push_back(std::move(name));

        return OK(pin);
    }

    Result<LibertyParser::bus_group> LibertyParser::parse_bus(TokenStream<std::string>& str, cell_group& cell)
    {
        bus_group bus;
        std::vector<u32> range;

        bus.line_number = str.peek().number;
        str.consume("(", true);
        bus.name = str.consume().string;
        str.consume(")", true);
        str.consume("{", true);
        auto bus_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

        do
        {
            auto next_token = bus_str.consume();
            if (next_token == "bus_type")
            {
                bus_str.consume(":", true);
                auto bus_type_str = bus_str.consume().string;
                if (const auto& it = m_bus_types.find(bus_type_str); it == m_bus_types.end())
                {
                    return ERR("could not parse bus '" + bus.name + "': invalid bus type '" + bus_type_str + "' (line " + std::to_string(bus.line_number) + ")");
                }
                else
                {
                    auto& type      = it->second;
                    bus.ascending   = type.ascending;
                    bus.start_index = type.start_index;
                    i32 dir         = (type.ascending ? 1 : -1);
                    for (i32 i = type.start_index; i != (i32)type.start_index + dir * type.width; i += dir)
                    {
                        range.push_back(i);
                    }
                }
                bus_str.consume(";", true);
            }
            else if (next_token == "direction")
            {
                bus_str.consume(":", true);
                auto direction_str = bus_str.consume().string;
                if (direction_str == "input")
                {
                    bus.direction = PinDirection::input;
                }
                else if (direction_str == "output")
                {
                    bus.direction = PinDirection::output;
                }
                else if (direction_str == "inout")
                {
                    bus.direction = PinDirection::inout;
                }
                else if (direction_str == "internal")
                {
                    bus.direction = PinDirection::internal;
                }
                else
                {
                    return ERR("could not parse bus '" + bus.name + "': invalid bus direction '" + direction_str + "' (line " + std::to_string(bus.line_number) + ")");
                }
                bus_str.consume(";", true);
            }
            else if (next_token == "pin")
            {
                if (auto res = parse_pin(bus_str, cell, bus.direction, bus.name); res.is_error())
                {
                    return ERR("could not parse bus '" + bus.name + "': failed to parse pin (line " + std::to_string(bus.line_number) + ")");
                }
                else
                {
                    auto pin = res.get();
                    cell.pins.push_back(pin);
                    bus.pins.push_back(std::move(pin));
                }
            }
        } while (bus_str.remaining() > 0);

        for (const auto& index : range)
        {
            auto pin_name = bus.name + "(" + std::to_string(index) + ")";
            bus.pin_names.push_back(pin_name);
            bus.index_to_pin[index] = pin_name;
        }

        if (bus.direction == PinDirection::none)
        {
            return ERR("could not parse bus '" + bus.name + "': no bus direction given (line " + std::to_string(bus.line_number) + ")");
        }

        return OK(bus);
    }

    Result<LibertyParser::ff_group> LibertyParser::parse_ff(TokenStream<std::string>& str)
    {
        ff_group ff;

        ff.line_number = str.peek().number;
        str.consume("(", true);
        ff.state1 = str.consume().string;
        str.consume(",", true);
        ff.state2 = str.consume().string;
        str.consume(")", true);
        str.consume("{", true);
        auto ff_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

        do
        {
            auto next_token = ff_str.consume();
            if (next_token == "clocked_on")
            {
                ff_str.consume(":", true);
                ff.clocked_on = ff_str.consume();
                ff_str.consume(";", true);
            }
            else if (next_token == "next_state")
            {
                ff_str.consume(":", true);
                ff.next_state = ff_str.consume();
                ff_str.consume(";", true);
            }
            else if (next_token == "clear")
            {
                ff_str.consume(":", true);
                ff.clear = ff_str.consume();
                ff_str.consume(";", true);
            }
            else if (next_token == "preset")
            {
                ff_str.consume(":", true);
                ff.preset = ff_str.consume();
                ff_str.consume(";", true);
            }
            else if (next_token == "clear_preset_var1" || next_token == "clear_preset_var2")
            {
                ff_str.consume(":", true);
                Token<std::string> behav_str = ff_str.consume();
                ff_str.consume(";", true);

                if (auto behav = enum_from_string<AsyncSetResetBehavior>(behav_str, AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
                {
                    if (next_token == "clear_preset_var1")
                    {
                        ff.special_behavior_var1 = behav;
                    }
                    else
                    {
                        ff.special_behavior_var2 = behav;
                    }
                }
                else
                {
                    return ERR("could not parse 'ff' group: invalid clear_preset behavior '" + behav_str.string + "' (line " + std::to_string(behav_str.number) + ")");
                }
            }
        } while (ff_str.remaining() > 0);

        return OK(ff);
    }

    Result<LibertyParser::latch_group> LibertyParser::parse_latch(TokenStream<std::string>& str)
    {
        latch_group latch;

        latch.line_number = str.peek().number;
        str.consume("(", true);
        latch.state1 = str.consume().string;
        str.consume(",", true);
        latch.state2 = str.consume().string;
        str.consume(")", true);
        str.consume("{", true);
        auto latch_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

        do
        {
            auto next_token = latch_str.consume();
            if (next_token == "enable")
            {
                latch_str.consume(":", true);
                latch.enable = latch_str.consume();
                latch_str.consume(";", true);
            }
            else if (next_token == "data_in")
            {
                latch_str.consume(":", true);
                latch.data_in = latch_str.consume();
                latch_str.consume(";", true);
            }
            else if (next_token == "clear")
            {
                latch_str.consume(":", true);
                latch.clear = latch_str.consume();
                latch_str.consume(";", true);
            }
            else if (next_token == "preset")
            {
                latch_str.consume(":", true);
                latch.preset = latch_str.consume();
                latch_str.consume(";", true);
            }
            else if (next_token == "clear_preset_var1" || next_token == "clear_preset_var2")
            {
                latch_str.consume(":", true);
                Token<std::string> behav_str = latch_str.consume();
                latch_str.consume(";", true);

                if (auto behav = enum_from_string<AsyncSetResetBehavior>(behav_str, AsyncSetResetBehavior::undef); behav != AsyncSetResetBehavior::undef)
                {
                    if (next_token == "clear_preset_var1")
                    {
                        latch.special_behavior_var1 = behav;
                    }
                    else
                    {
                        latch.special_behavior_var2 = behav;
                    }
                }
                else
                {
                    return ERR("could not parse 'latch' group: invalid clear_preset behavior '" + behav_str.string + "' (line " + std::to_string(behav_str.number) + ")");
                }
            }
        } while (latch_str.remaining() > 0);

        return OK(latch);
    }

    Result<std::monostate> LibertyParser::construct_gate_type(cell_group&& cell)
    {
        // get input and from pin groups
        bool has_inputs = false;
        u32 num_outputs = 0;
        std::string output_func;
        for (const auto& pin : cell.pins)
        {
            if (pin.direction == PinDirection::input || pin.direction == PinDirection::inout)
            {
                if (!pin.power && !pin.ground)
                {
                    has_inputs = true;
                }
            }
            else if (pin.direction == PinDirection::output || pin.direction == PinDirection::inout)
            {
                num_outputs += pin.pin_names.size();
                output_func = pin.function;
            }
        }

        std::unique_ptr<GateTypeComponent> parent_component = nullptr;
        if (!has_inputs && num_outputs == 1)
        {
            if (output_func == "0")
            {
                cell.properties.insert(GateTypeProperty::combinational);
                cell.properties.insert(GateTypeProperty::ground);
            }
            else if (output_func == "1")
            {
                cell.properties.insert(GateTypeProperty::combinational);
                cell.properties.insert(GateTypeProperty::power);
            }
        }
        else if (cell.ff.has_value())
        {
            if (cell.ff->clocked_on.empty() || cell.ff->next_state.empty())
            {
                return ERR("could not construct gate type '" + cell.name + "': missing 'clocked_on' or 'next_state' function (or both)");
            }

            std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(nullptr, cell.ff->state1, cell.ff->state2);

            auto next_state_function = BooleanFunction::from_string(cell.ff->next_state);
            if (next_state_function.is_error())
            {
                return ERR_APPEND(next_state_function.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'next_state' function from string");
            }
            auto clocked_on_function = BooleanFunction::from_string(cell.ff->clocked_on);
            if (clocked_on_function.is_error())
            {
                return ERR_APPEND(next_state_function.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'clocked_on' function from string");
            }
            parent_component = GateTypeComponent::create_ff_component(std::move(state_component), next_state_function.get(), clocked_on_function.get());

            FFComponent* ff_component = parent_component->convert_to<FFComponent>();
            if (!cell.ff->clear.empty())
            {
                auto clear_function = BooleanFunction::from_string(cell.ff->clear);
                if (clear_function.is_error())
                {
                    return ERR_APPEND(next_state_function.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'clear' function from string");
                }
                ff_component->set_async_reset_function(clear_function.get());
            }
            if (!cell.ff->preset.empty())
            {
                auto preset_function = BooleanFunction::from_string(cell.ff->preset);
                if (preset_function.is_error())
                {
                    return ERR_APPEND(next_state_function.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'preset' function from string");
                }
                ff_component->set_async_set_function(preset_function.get());
            }

            ff_component->set_async_set_reset_behavior(cell.ff->special_behavior_var1, cell.ff->special_behavior_var2);

            for (auto& pin : cell.pins)
            {
                if (pin.clock == true)
                {
                    pin.type = PinType::clock;
                }
                else if (pin.function == cell.ff->state1)
                {
                    pin.type = PinType::state;
                }
                else if (pin.function == cell.ff->state2)
                {
                    pin.type = PinType::neg_state;
                }
            }
        }
        else if (cell.latch.has_value())
        {
            std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(nullptr, cell.latch->state1, cell.latch->state2);

            parent_component                = GateTypeComponent::create_latch_component(std::move(state_component));
            LatchComponent* latch_component = parent_component->convert_to<LatchComponent>();
            assert(latch_component != nullptr);

            if (!cell.latch->data_in.empty())
            {
                auto res = BooleanFunction::from_string(cell.latch->data_in);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'data_in' function from string");
                }
                latch_component->set_data_in_function(res.get());
            }
            if (!cell.latch->enable.empty())
            {
                auto res = BooleanFunction::from_string(cell.latch->enable);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'enable' function from string");
                }
                latch_component->set_enable_function(res.get());
            }
            if (!cell.latch->clear.empty())
            {
                auto res = BooleanFunction::from_string(cell.latch->clear);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'clear' function from string");
                }
                latch_component->set_async_reset_function(res.get());
            }
            if (!cell.latch->preset.empty())
            {
                auto res = BooleanFunction::from_string(cell.latch->preset);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not construct gate type '" + cell.name + "': failed parsing 'preset' function from string");
                }
                latch_component->set_async_set_function(res.get());
            }

            latch_component->set_async_set_reset_behavior(cell.latch->special_behavior_var1, cell.latch->special_behavior_var2);

            for (auto& pin : cell.pins)
            {
                if (pin.clock == true)
                {
                    pin.type = PinType::clock;
                }
                else if (pin.function == cell.latch->state1)
                {
                    pin.type = PinType::state;
                }
                else if (pin.function == cell.latch->state2)
                {
                    pin.type = PinType::neg_state;
                }
            }
        }

        if (cell.properties.empty())
        {
            cell.properties.insert(GateTypeProperty::combinational);
        }

        GateType* gt = m_gate_lib->create_gate_type(cell.name, cell.properties, std::move(parent_component));

        // get input and output pins from pin groups
        for (auto& pin : cell.pins)
        {
            if (pin.power == true)
            {
                pin.type = PinType::power;
            }

            if (pin.ground == true)
            {
                pin.type = PinType::ground;
            }

            for (const auto& pin_name : pin.pin_names)
            {
                GatePin* pin_inst;
                if (auto res = gt->create_pin(pin_name, pin.direction, pin.type); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not construct gate type '" + cell.name + "': failed to create pin '" + pin_name + "'");
                }
                else
                {
                    pin_inst = res.get();
                }
                if (auto res = gt->create_pin_group(pin_name, {pin_inst}, pin.direction, pin.type); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not construct gate type '" + cell.name + "': failed to create pin group '" + pin_name + "'");
                }
            }
        }

        for (const auto& [bus_name, bus_info] : cell.buses)
        {
            std::vector<GatePin*> pins;
            for (const auto& pin_name : bus_info.pin_names)
            {
                if (auto res = gt->get_pin_by_name(pin_name); res == nullptr)
                {
                    return ERR("could not construct gate type '" + cell.name + "': failed to get pin by name '" + pin_name + "'");
                }
                else
                {
                    pins.push_back(res);
                }
            }
            if (auto res = gt->create_pin_group(bus_name, pins, bus_info.direction, PinType::none, bus_info.ascending, bus_info.start_index); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not construct gate type '" + cell.name + "': failed to create pin group '" + bus_name + "'");
            }
        }

        if (!cell.buses.empty())
        {
            auto functions = construct_bus_functions(cell);
            if (functions.is_error())
            {
                return ERR_APPEND(functions.get_error(), "could not construct gate type '" + cell.name + "': failed to construct bus functions");
            }
            gt->add_boolean_functions(functions.get());
        }
        else
        {
            for (const auto& pin : cell.pins)
            {
                if (!pin.function.empty())
                {
                    for (const auto& name : pin.pin_names)
                    {
                        auto function = BooleanFunction::from_string(pin.function);
                        if (function.is_error())
                        {
                            return ERR_APPEND(function.get_error(), "could not construct gate type '" + cell.name + "': failed parsing output function from string");
                        }
                        gt->add_boolean_function(name, function.get());
                    }
                }

                if (!pin.x_function.empty())
                {
                    for (const auto& name : pin.pin_names)
                    {
                        auto function = BooleanFunction::from_string(pin.x_function);
                        if (function.is_error())
                        {
                            return ERR_APPEND(function.get_error(), "could not construct gate type '" + cell.name + "': failed parsing undefined function from string");
                        }
                        gt->add_boolean_function(name + "_undefined", function.get());
                    }
                }

                if (!pin.z_function.empty())
                {
                    for (const auto& name : pin.pin_names)
                    {
                        auto function = BooleanFunction::from_string(pin.z_function);
                        if (function.is_error())
                        {
                            return ERR_APPEND(function.get_error(), "could not construct gate type '" + cell.name + "': failed parsing tristate function from string");
                        }
                        gt->add_boolean_function(name + "_tristate", function.get());
                    }
                }
            }
        }

        return OK({});
    }

    void LibertyParser::remove_comments(std::string& line, bool& multi_line_comment)
    {
        bool repeat = true;

        while (repeat)
        {
            repeat = false;

            // skip empty lines
            if (line.empty())
            {
                break;
            }
            auto multi_line_comment_begin = line.find("/*");
            auto multi_line_comment_end   = line.find("*/");

            std::string begin = "";
            std::string end   = "";

            if (multi_line_comment == true)
            {
                if (multi_line_comment_end != std::string::npos)
                {
                    // multi-line comment ends in current line
                    multi_line_comment = false;
                    line               = line.substr(multi_line_comment_end + 2);
                    repeat             = true;
                }
                else
                {
                    // current line entirely within multi-line comment
                    line = "";
                    break;
                }
            }
            else if (multi_line_comment_begin != std::string::npos)
            {
                if (multi_line_comment_end != std::string::npos)
                {
                    // multi-line comment entirely in current line
                    line   = line.substr(0, multi_line_comment_begin) + line.substr(multi_line_comment_end + 2);
                    repeat = true;
                }
                else
                {
                    // multi-line comment starts in current line
                    multi_line_comment = true;
                    line               = line.substr(0, multi_line_comment_begin);
                }
            }
        }
    }

    std::vector<std::string> LibertyParser::tokenize_function(const std::string& function)
    {
        std::string delimiters = "()[]:!'^+|&* ";
        std::string current_token;
        std::vector<std::string> res;

        for (char c : function)
        {
            if (delimiters.find(c) == std::string::npos)
            {
                current_token += c;
            }
            else
            {
                if (!current_token.empty())
                {
                    res.push_back(current_token);
                    current_token.clear();
                }
                res.push_back(std::string(1, c));
            }
        }
        if (!current_token.empty())
        {
            res.push_back(current_token);
            current_token.clear();
        }

        return res;
    }

    std::map<std::string, std::string> LibertyParser::expand_bus_function(const std::map<std::string, bus_group>& buses, const std::vector<std::string>& pin_names, const std::string& function)
    {
        auto tokenized_funtion = tokenize_function(function);
        std::map<std::string, std::string> res;

        for (const auto& name : pin_names)
        {
            res.emplace(name, "");
        }

        std::string tmp = "";

        for (u32 i = 0; i < tokenized_funtion.size(); i++)
        {
            if (const auto& it = buses.find(tokenized_funtion.at(i)); it != buses.end())
            {
                if (tokenized_funtion.size() > (i + 3) && tokenized_funtion.at(i + 1) == "[")
                {
                    if (tokenized_funtion.size() > (i + 5) && tokenized_funtion.at(i + 3) == ":" && tokenized_funtion.at(i + 5) == "]")
                    {
                        i32 start     = std::stoul(tokenized_funtion.at(i + 2));
                        i32 end       = std::stoul(tokenized_funtion.at(i + 4));
                        i32 direction = (start < end) ? 1 : -1;

                        for (const auto& name : pin_names)
                        {
                            res[name] += tmp + it->second.index_to_pin.at(start);
                            start += direction;
                        }

                        i += 5;
                    }
                    else if (tokenized_funtion.at(i + 3) == "]")
                    {
                        u32 index = std::stoul(tokenized_funtion.at(i + 2));

                        for (const auto& name : pin_names)
                        {
                            res[name] += tmp + it->second.index_to_pin.at(index);
                        }

                        i += 3;
                    }
                    else
                    {
                        log_warning("liberty_parser", "could not handle bus '{}' in function '{}' near line {}, ignoring function.", it->first, function, it->second.line_number);
                        return {};
                    }
                }
                else
                {
                    for (u32 j = 0; j < pin_names.size(); j++)
                    {
                        res[pin_names.at(j)] += tmp + it->second.pin_names.at(j);
                    }
                }
                tmp = "";
            }
            else
            {
                tmp += tokenized_funtion.at(i);
            }
        }

        if (!tmp.empty())
        {
            for (const auto& name : pin_names)
            {
                res[name] += tmp;
            }
        }

        return res;
    }

    std::string LibertyParser::prepare_pin_function(const std::map<std::string, bus_group>& buses, const std::string& function)
    {
        auto tokenized_funtion = tokenize_function(function);
        std::string res        = "";

        for (u32 i = 0; i < tokenized_funtion.size(); i++)
        {
            if (const auto& it = buses.find(tokenized_funtion.at(i)); it != buses.end())
            {
                if (tokenized_funtion.size() > (i + 3) && tokenized_funtion.at(i + 1) == "[" && tokenized_funtion.at(i + 3) == "]")
                {
                    u32 index = std::stoul(tokenized_funtion.at(i + 2));

                    res += it->second.index_to_pin.at(index);

                    i += 3;
                }
                else
                {
                    log_warning("liberty_parser", "could not handle bus '{}' in function '{}' near line {}, ignoring function.", it->first, function, it->second.line_number);
                    return "";
                }
            }
            else
            {
                res += tokenized_funtion.at(i);
            }
        }

        return res;
    }

    Result<std::unordered_map<std::string, BooleanFunction>> LibertyParser::construct_bus_functions(const cell_group& cell)
    {
        std::unordered_map<std::string, BooleanFunction> res;

        for (const auto& [bus_name, bus] : cell.buses)
        {
            UNUSED(bus_name);

            if (bus.direction != PinDirection::output && bus.direction != PinDirection::inout)
            {
                continue;
            }

            for (const auto& pin : bus.pins)
            {
                if (!pin.function.empty())
                {
                    for (auto [pin_name, function] : expand_bus_function(cell.buses, pin.pin_names, pin.function))
                    {
                        auto bf = BooleanFunction::from_string(function);
                        if (bf.is_error())
                        {
                            return ERR_APPEND(bf.get_error(), "could not construct gate type '" + cell.name + "': failed parsing output function from string");
                        }
                        res.emplace(pin_name, bf.get());
                    }
                }

                if (!pin.x_function.empty())
                {
                    for (auto [pin_name, function] : expand_bus_function(cell.buses, pin.pin_names, pin.x_function))
                    {
                        auto bf = BooleanFunction::from_string(function);
                        if (bf.is_error())
                        {
                            return ERR_APPEND(bf.get_error(), "could not construct gate type '" + cell.name + "': failed parsing undefined function from string");
                        }
                        res.emplace(pin_name + "_undefined", bf.get());
                    }
                }

                if (!pin.z_function.empty())
                {
                    for (auto [pin_name, function] : expand_bus_function(cell.buses, pin.pin_names, pin.z_function))
                    {
                        auto bf = BooleanFunction::from_string(function);
                        if (bf.is_error())
                        {
                            return ERR_APPEND(bf.get_error(), "could not construct gate type '" + cell.name + "': failed parsing tristate function from string");
                        }
                        res.emplace(pin_name + "_tristate", bf.get());
                    }
                }
            }
        }

        for (const auto& pin : cell.pins)
        {
            if (!pin.function.empty())
            {
                if (auto function = prepare_pin_function(cell.buses, pin.function); !function.empty())
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        auto bf = BooleanFunction::from_string(function);
                        if (bf.is_error())
                        {
                            return ERR_APPEND(bf.get_error(), "could not construct gate type '" + cell.name + "': failed parsing output function from string");
                        }
                        res.emplace(pin_name, bf.get());
                    }
                }
            }

            if (!pin.x_function.empty())
            {
                if (auto function = prepare_pin_function(cell.buses, pin.x_function); !function.empty())
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        auto bf = BooleanFunction::from_string(function);
                        if (bf.is_error())
                        {
                            return ERR_APPEND(bf.get_error(), "could not construct gate type '" + cell.name + "': failed parsing undefined function from string");
                        }
                        res.emplace(pin_name + "_undefined", bf.get());
                    }
                }
            }

            if (!pin.z_function.empty())
            {
                if (auto function = prepare_pin_function(cell.buses, pin.z_function); !function.empty())
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        auto bf = BooleanFunction::from_string(function);
                        if (bf.is_error())
                        {
                            return ERR_APPEND(bf.get_error(), "could not construct gate type '" + cell.name + "': failed parsing tristate function from string");
                        }
                        res.emplace(pin_name + "_tristate", bf.get());
                    }
                }
            }
        }

        return OK(res);
    }
}    // namespace hal