#include "liberty_parser/liberty_parser.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/utilities/log.h"

#include <fstream>

// TODO remove LUT parsing

namespace hal
{
    std::unique_ptr<GateLibrary> LibertyParser::parse(const std::filesystem::path& file_path)
    {
        m_path = file_path;

        {
            std::ifstream ifs;
            ifs.open(m_path.string(), std::ifstream::in);
            if (!ifs.is_open())
            {
                log_error("liberty_parser", "unable to open '{}'.", m_path.string());
                return nullptr;
            }
            m_fs << ifs.rdbuf();
            ifs.close();
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
        catch (TokenStream<std::string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                log_error("liberty_parser", "{} near line {}.", e.message, e.line_number);
            }
            else
            {
                log_error("liberty_parser", "{}.", e.message);
            }
            return nullptr;
        }

        return std::move(m_gate_lib);
    }

    bool LibertyParser::tokenize()
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
        return true;
    }

    bool LibertyParser::parse_tokens()
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
                auto cell = parse_cell(library_str);
                if (!cell.has_value())
                {
                    return false;
                }

                if (!construct_gate_type(cell.value()))
                {
                    return false;
                }
            }
            else if (next_token == "type" && library_str.peek() == "(")
            {
                auto type = parse_type(library_str);
                if (!type.has_value())
                {
                    return false;
                }
                m_bus_types.emplace(type->name, type.value());
            }
        } while (library_str.remaining() > 0);

        return m_token_stream.remaining() == 0;
    }

    std::optional<LibertyParser::type_group> LibertyParser::parse_type(TokenStream<std::string>& str)
    {
        type_group type;
        i32 width     = 1;
        i32 start     = 0;
        i32 end       = 0;
        i32 direction = 1;

        type.line_number = str.peek().number;
        str.consume("(", true);
        type.name = str.consume().string;
        str.consume(")", true);
        str.consume("{", true);
        auto type_str = str.extract_until("}", TokenStream<std::string>::END_OF_STREAM, true, true);
        str.consume("}", true);

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
                width = std::stol(type_str.consume().string);
            }
            else if (next_token == "bit_from")
            {
                type_str.consume(":", true);
                start = std::stol(type_str.consume().string);
            }
            else if (next_token == "bit_to")
            {
                type_str.consume(":", true);
                end = std::stol(type_str.consume().string);
            }
            else if (next_token == "downto")
            {
                type_str.consume(":", true);
                if (type_str.consume("false"))
                {
                    direction = 1;
                }
                else if (type_str.consume("true"))
                {
                    direction = -1;
                }
                else
                {
                    log_error(
                        "liberty_parser", "invalid token '{}' for boolean value in 'downto' statement in type group '{}' near line {}.", type_str.peek().string, type.name, type_str.peek().number);
                    return std::nullopt;
                }
            }
            else
            {
                log_error("liberty_parser", "invalid token '{}' in type group '{}' near line {}.", type_str.peek().string, type.name, type_str.peek().number);
                return std::nullopt;
            }

            type_str.consume(";");
        }

        if (width != (direction * (end - start)) + 1)
        {
            log_error("liberty_parser", "invalid 'bit_width' value {} for type group '{}' near line {}.", width, type.name, type.line_number);
            return std::nullopt;
        }

        for (int i = start; i != end + direction; i += direction)
        {
            type.range.push_back((u32)i);
        }

        return type;
    }

    std::optional<LibertyParser::cell_group> LibertyParser::parse_cell(TokenStream<std::string>& str)
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
            log_error("liberty_parser", "a cell with the name '{}' does already exist.", cell.name);
            return std::nullopt;
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
                auto pin = parse_pin(cell_str, cell);
                if (!pin.has_value())
                {
                    return std::nullopt;
                }
                cell.pins.push_back(pin.value());
            }
            else if (next_token == "pg_pin")
            {
                auto pin = parse_pg_pin(cell_str, cell);
                if (pin.has_value())
                {
                    cell.pins.push_back(pin.value());
                }
            }
            else if (next_token == "bus")
            {
                auto bus = parse_bus(cell_str, cell);
                if (!bus.has_value())
                {
                    return std::nullopt;
                }
                cell.buses.emplace(bus->name, bus.value());
            }
            else if (next_token == "ff")
            {
                auto ff = parse_ff(cell_str);
                if (!ff.has_value())
                {
                    return std::nullopt;
                }
                cell.properties.insert(GateTypeProperty::ff);
                cell.properties.insert(GateTypeProperty::sequential);
                cell.ff = ff.value();
            }
            else if (next_token == "latch")
            {
                auto latch = parse_latch(cell_str);
                if (!latch.has_value())
                {
                    return std::nullopt;
                }
                cell.properties.insert(GateTypeProperty::latch);
                cell.properties.insert(GateTypeProperty::sequential);
                cell.latch = latch.value();
            }
        }

        return cell;
    }

    std::optional<LibertyParser::pin_group> LibertyParser::parse_pin(TokenStream<std::string>& str, cell_group& cell, PinDirection direction, const std::string& external_pin_name)
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
            log_error("liberty_parser", "no pin name given near line {}.", pin.line_number);
            return std::nullopt;
        }

        do
        {
            std::string name = pin_names_str.consume().string;
            if (!external_pin_name.empty() && name != external_pin_name)
            {
                log_error("liberty_parser", "invalid pin name '{}' near line {}.", name, pin.line_number);
                return std::nullopt;
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
                            log_error("liberty_parser", "a pin with name '{}' does already exist for cell '{}'.", new_name, cell.name);
                            return std::nullopt;
                        }

                        cell.pin_names.insert(new_name);
                        pin.pin_names.push_back(new_name);
                    }
                }
                else
                {
                    u32 index     = std::stoul(pin_names_str.consume().string);
                    auto new_name = name + "(" + std::to_string(index) + ")";

                    if (const auto pin_it = cell.pin_names.find(new_name); pin_it != cell.pin_names.end())
                    {
                        log_error("liberty_parser", "a pin with name '{}' does already exist for cell '{}'.", new_name, cell.name);
                        return std::nullopt;
                    }

                    cell.pin_names.insert(new_name);
                    pin.pin_names.push_back(new_name);
                }

                pin_names_str.consume("]", true);
            }
            else
            {
                if (const auto pin_it = cell.pin_names.find(name); pin_it != cell.pin_names.end())
                {
                    log_error("liberty_parser", "a pin with name '{}' does already exist for cell '{}'.", name, cell.name);
                    return std::nullopt;
                }

                cell.pin_names.insert(name);
                pin.pin_names.push_back(name);
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
                    log_warning("liberty_parser", "invalid pin direction '{}' near line {}.", direction_str, pin.line_number);
                    return std::nullopt;
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
            log_error("liberty_parser", "no pin direction given near line {}.", pin.line_number);
            return std::nullopt;
        }

        return pin;
    }

    std::optional<LibertyParser::pin_group> LibertyParser::parse_pg_pin(TokenStream<std::string>& str, cell_group& cell)
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
            log_warning("liberty_parser", "no pg_pin name given near line {}.", pin.line_number);
            return std::nullopt;
        }
        else if (pin_names_str.size() > 1)
        {
            log_warning("liberty_parser", "more than one pg_pin name given near line {}.", pin.line_number);
            return std::nullopt;
        }

        std::string name = pin_names_str.consume().string;

        if (const auto pin_it = cell.pin_names.find(name); pin_it != cell.pin_names.end())
        {
            log_warning("liberty_parser", "a pin with name '{}' does already exist for cell '{}'.", name, cell.name);
            return std::nullopt;
        }

        cell.pin_names.insert(name);
        pin.pin_names.push_back(name);

        pin.direction = PinDirection::input;

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
                    return std::nullopt;
                }
                pin_str.consume(";", true);
            }
        }

        return pin;
    }

    std::optional<LibertyParser::bus_group> LibertyParser::parse_bus(TokenStream<std::string>& str, cell_group& cell)
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
                if (const auto& it = m_bus_types.find(bus_type_str); it != m_bus_types.end())
                {
                    range = it->second.range;
                }
                else
                {
                    log_error("liberty_parser", "invalid bus type '{}' near line {}.", bus_type_str, bus.line_number);
                    return std::nullopt;
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
                    log_error("liberty_parser", "invalid pin direction '{}' near line {}.", direction_str, bus.line_number);
                    return std::nullopt;
                }
                bus_str.consume(";", true);
            }
            else if (next_token == "pin")
            {
                auto pin = parse_pin(bus_str, cell, bus.direction, bus.name);
                if (!pin.has_value())
                {
                    return std::nullopt;
                }

                cell.pins.push_back(pin.value());
                bus.pins.push_back(pin.value());
            }
        } while (bus_str.remaining() > 0);

        for (const auto& index : range)
        {
            auto pin_name = bus.name + "(" + std::to_string(index) + ")";
            bus.pin_names.push_back(pin_name);
            bus.pin_indices.push_back(std::make_pair(index, pin_name));
            bus.index_to_pin.emplace(index, pin_name);
        }

        if (bus.direction == PinDirection::none)
        {
            log_error("liberty_parser", "no bus direction given near line {}.", bus.line_number);
            return std::nullopt;
        }

        return bus;
    }

    std::optional<LibertyParser::ff_group> LibertyParser::parse_ff(TokenStream<std::string>& str)
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
                    log_error("liberty_parser", "invalid clear_preset behavior '{}' near line {}.", behav_str.string, behav_str.number);
                    return std::nullopt;
                }
            }
        } while (ff_str.remaining() > 0);

        return ff;
    }

    std::optional<LibertyParser::latch_group> LibertyParser::parse_latch(TokenStream<std::string>& str)
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
                    log_error("liberty_parser", "invalid clear_preset behavior '{}' near line {}.", behav_str.string, behav_str.number);
                    return std::nullopt;
                }
            }
        } while (latch_str.remaining() > 0);

        return latch;
    }

    bool LibertyParser::construct_gate_type(cell_group& cell)
    {
        // get input and from pin groups
        bool has_inputs = false;
        u32 num_outputs = 0;
        std::string output_func;
        std::vector<std::string> bf_vars;
        for (const auto& pin : cell.pins)
        {
            bf_vars.insert(bf_vars.end(), pin.pin_names.begin(), pin.pin_names.end());

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
        std::unordered_map<std::string, PinType> pin_types;
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
                return false;
            }

            std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(nullptr, cell.ff->state1, cell.ff->state2);
            bf_vars.push_back(cell.ff->state1);
            bf_vars.push_back(cell.ff->state2);

            auto next_state_function = BooleanFunction::from_string(cell.ff->next_state);
            auto clocked_on_function = BooleanFunction::from_string(cell.ff->clocked_on);

            parent_component =
                GateTypeComponent::create_ff_component(std::move(state_component),
                                                       (next_state_function.is_ok()) ? next_state_function.get() : BooleanFunction(),
                                                       (clocked_on_function.is_ok()) ? clocked_on_function.get() : BooleanFunction());

            FFComponent* ff_component = parent_component->convert_to<FFComponent>();
            if (!cell.ff->clear.empty())
            {
                auto clear_function = BooleanFunction::from_string(cell.ff->clear);
                ff_component->set_async_reset_function((clear_function.is_ok()) ? clear_function.get() : BooleanFunction());
            }
            if (!cell.ff->preset.empty())
            {
                auto preset_function = BooleanFunction::from_string(cell.ff->preset);
                ff_component->set_async_set_function((preset_function.is_ok()) ? preset_function.get() : BooleanFunction());
            }

            ff_component->set_async_set_reset_behavior(cell.ff->special_behavior_var1, cell.ff->special_behavior_var2);

            for (auto& pin : cell.pins)
            {
                if (pin.clock == true)
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        pin_types[pin_name] = PinType::clock;
                    }
                }
                else if (pin.function == cell.ff->state1)
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        pin_types[pin_name] = PinType::state;
                    }
                }
                else if (pin.function == cell.ff->state2)
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        pin_types[pin_name] = PinType::neg_state;
                    }
                }
            }
        }
        else if (cell.latch.has_value())
        {
            if (cell.latch->enable.empty() || cell.latch->data_in.empty())
            {
                return false;
            }

            std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(nullptr, cell.latch->state1, cell.latch->state2);
            bf_vars.push_back(cell.latch->state1);
            bf_vars.push_back(cell.latch->state2);

            parent_component                = GateTypeComponent::create_latch_component(std::move(state_component));
            LatchComponent* latch_component = parent_component->convert_to<LatchComponent>();
            assert(latch_component != nullptr);

            if (!cell.latch->data_in.empty() && !cell.latch->enable.empty())
            {
                auto data_in_function = BooleanFunction::from_string(cell.latch->data_in);
                latch_component->set_data_in_function((data_in_function.is_ok()) ? data_in_function.get() : BooleanFunction());

                auto enable_function = BooleanFunction::from_string(cell.latch->enable);
                latch_component->set_enable_function((enable_function.is_ok()) ? enable_function.get() : BooleanFunction());
            }
            else if (cell.latch->data_in.empty())
            {
                log_error("liberty_parser", "missing 'data_in' specification for latch gate type '{}'.", cell.name);
                return false;
            }
            else if (cell.latch->enable.empty())
            {
                log_error("liberty_parser", "missing 'enable' specification for latch gate type '{}'.", cell.name);
                return false;
            }

            if (!cell.latch->clear.empty())
            {
                auto clear_function = BooleanFunction::from_string(cell.latch->clear);
                latch_component->set_async_reset_function((clear_function.is_ok()) ? clear_function.get() : BooleanFunction());
            }
            if (!cell.latch->preset.empty())
            {
                auto preset_function = BooleanFunction::from_string(cell.latch->preset);
                latch_component->set_async_set_function((preset_function.is_ok()) ? preset_function.get() : BooleanFunction());
            }

            latch_component->set_async_set_reset_behavior(cell.latch->special_behavior_var1, cell.latch->special_behavior_var2);

            for (auto& pin : cell.pins)
            {
                if (pin.clock == true)
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        pin_types[pin_name] = PinType::clock;
                    }
                }
                else if (pin.function == cell.latch->state1)
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        pin_types[pin_name] = PinType::state;
                    }
                }
                else if (pin.function == cell.latch->state2)
                {
                    for (const auto& pin_name : pin.pin_names)
                    {
                        pin_types[pin_name] = PinType::neg_state;
                    }
                }
            }
        }

        if (cell.properties.empty())
        {
            cell.properties.insert(GateTypeProperty::combinational);
        }

        GateType* gt = m_gate_lib->create_gate_type(cell.name, cell.properties, std::move(parent_component));

        // get input and output pins from pin groups
        for (const auto& pin : cell.pins)
        {
            gt->add_pins(pin.pin_names, pin.direction);

            if (pin.power == true)
            {
                for (const auto& pin_name : pin.pin_names)
                {
                    pin_types[pin_name] = PinType::power;
                }
            }

            if (pin.ground == true)
            {
                for (const auto& pin_name : pin.pin_names)
                {
                    pin_types[pin_name] = PinType::ground;
                }
            }
        }

        for (const auto& [bus_name, bus_info] : cell.buses)
        {
            if (!gt->assign_pin_group(bus_name, bus_info.pin_indices))
            {
                return false;
            }
        }

        for (const auto& [pin, type] : pin_types)
        {
            gt->assign_pin_type(pin, type);
        }

        if (!cell.buses.empty())
        {
            auto functions = construct_bus_functions(cell, bf_vars);
            gt->add_boolean_functions(functions);
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
                        gt->add_boolean_function(name, (function.is_ok()) ? function.get() : BooleanFunction());
                    }
                }

                if (!pin.x_function.empty())
                {
                    for (const auto& name : pin.pin_names)
                    {
                        auto function = BooleanFunction::from_string(pin.x_function);
                        gt->add_boolean_function(name + "_undefined", (function.is_ok()) ? function.get() : BooleanFunction());
                    }
                }

                if (!pin.z_function.empty())
                {
                    for (const auto& name : pin.pin_names)
                    {
                        auto function = BooleanFunction::from_string(pin.z_function);
                        gt->add_boolean_function(name + "_tristate", (function.is_ok()) ? function.get() : BooleanFunction());
                    }
                }
            }
        }

        return true;
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

    std::unordered_map<std::string, BooleanFunction> LibertyParser::construct_bus_functions(const cell_group& cell, const std::vector<std::string>& all_pins)
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
                        res.emplace(pin_name, (bf.is_ok()) ? bf.get() : BooleanFunction());
                    }
                }

                if (!pin.x_function.empty())
                {
                    for (auto [pin_name, function] : expand_bus_function(cell.buses, pin.pin_names, pin.x_function))
                    {
                        auto bf = BooleanFunction::from_string(function);
                        res.emplace(pin_name + "_undefined", (bf.is_ok()) ? bf.get() : BooleanFunction());
                    }
                }

                if (!pin.z_function.empty())
                {
                    for (auto [pin_name, function] : expand_bus_function(cell.buses, pin.pin_names, pin.z_function))
                    {
                        auto bf = BooleanFunction::from_string(function);
                        res.emplace(pin_name + "_tristate", (bf.is_ok()) ? bf.get() : BooleanFunction());
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
                        res.emplace(pin_name, (bf.is_ok()) ? bf.get() : BooleanFunction());
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
                        res.emplace(pin_name + "_undefined", (bf.is_ok()) ? bf.get() : BooleanFunction());
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
                        res.emplace(pin_name + "_tristate", (bf.is_ok()) ? bf.get() : BooleanFunction());
                    }
                }
            }
        }

        return res;
    }
}    // namespace hal
