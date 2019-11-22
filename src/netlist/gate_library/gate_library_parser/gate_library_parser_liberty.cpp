#include "netlist/gate_library/gate_library_parser/gate_library_parser_liberty.h"

#include "core/log.h"

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_type/gate_type_lut.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"

gate_library_parser_liberty::gate_library_parser_liberty(std::stringstream& stream) : gate_library_parser(stream)
{
}

std::shared_ptr<gate_library> gate_library_parser_liberty::parse()
{
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
    catch (token_stream::token_stream_exception& e)
    {
        if (e.line_number != (u32)-1)
        {
            log_error("netlist", "{} near line {}.", e.message, e.line_number);
        }
        else
        {
            log_error("netlist", "{}.", e.message);
        }
        return nullptr;
    }

    return m_gate_lib;
}

bool gate_library_parser_liberty::tokenize()
{
    std::string delimiters = "{}();:\",";
    std::string current_token;
    u32 line_number = 0;

    std::string line;
    bool in_string          = false;
    bool multi_line_comment = false;

    std::vector<token> parsed_tokens;

    while (std::getline(m_fs, line))
    {
        line_number++;
        this->remove_comments(line, multi_line_comment);

        for (char c : line)
        {
            if (c == '\"')
            {
                in_string = !in_string;
                continue;
            }

            if (std::isspace(c))
            {
                continue;
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

    m_token_stream = token_stream(parsed_tokens, {"(", "{"}, {")", "}"});
    return true;
}

bool gate_library_parser_liberty::parse_tokens()
{
    m_token_stream.consume("library", true);
    m_token_stream.consume("(", true);
    auto lib_name = m_token_stream.consume();
    m_token_stream.consume(")", true);
    m_token_stream.consume("{", true);
    m_gate_lib          = std::make_shared<gate_library>(lib_name.string);
    auto library_stream = m_token_stream.extract_until("}", token_stream::END_OF_STREAM, true, true);
    m_token_stream.consume("}", true);

    while (library_stream.remaining() > 0)
    {
        if (library_stream.peek() == "cell" && library_stream.peek(1) == "(")
        {
            m_current_cell.clear();

            if (!parse_cell(library_stream))
            {
                return false;
            }

            auto gt = construct_gate_type();

            m_gate_lib->add_gate_type(gt);
        }
        else
        {
            library_stream.consume();
        }
    }

    return true;
}

bool gate_library_parser_liberty::parse_cell(token_stream& library_stream)
{
    library_stream.consume("cell", true);
    library_stream.consume("(", true);
    m_current_cell.name = library_stream.consume();
    library_stream.consume(")", true);
    library_stream.consume("{", true);
    auto cell_stream = library_stream.extract_until("}", token_stream::END_OF_STREAM, true, true);
    library_stream.consume("}", true);
    m_current_cell.type = gate_type::base_type::combinatorial;

    while (cell_stream.remaining() > 0)
    {
        if (cell_stream.peek() == "pin")
        {
            if (!parse_pin(cell_stream))
            {
                return false;
            }
        }
        else if (cell_stream.peek() == "ff")
        {
            if (!parse_ff(cell_stream))
            {
                return false;
            }
        }
        else if (cell_stream.peek() == "latch")
        {
            if (!parse_latch(cell_stream))
            {
                return false;
            }
        }
        else if (cell_stream.peek() == "lut")
        {
            if (!parse_lut(cell_stream))
            {
                return false;
            }
        }
        else
        {
            cell_stream.consume();
        }
    }

    return true;
}

bool gate_library_parser_liberty::parse_pin(token_stream& cell_stream)
{
    cell_stream.consume("pin", true);
    cell_stream.consume("(", true);
    auto pin_name = cell_stream.consume().string;
    cell_stream.consume(")", true);
    cell_stream.consume("{", true);
    auto pin_stream = cell_stream.extract_until("}", token_stream::END_OF_STREAM, true, true);
    cell_stream.consume("}", true);

    while (pin_stream.remaining() > 0)
    {
        if (pin_stream.consume("direction", false))
        {
            pin_stream.consume(":", true);
            auto pin_direction = pin_stream.consume();

            if (pin_direction == "input")
            {
                m_current_cell.input_pins.push_back(pin_name);
            }
            else if (pin_direction == "output")
            {
                m_current_cell.output_pins.push_back(pin_name);
            }

            pin_stream.consume(";", true);
        }
        else if (pin_stream.consume("function", false))
        {
            pin_stream.consume(":", true);
            m_current_cell.functions.emplace(pin_name, pin_stream.consume());
            pin_stream.consume(";", true);
        }
        else if (pin_stream.consume("x_function", false))
        {
            pin_stream.consume(":", true);
            m_current_cell.functions.emplace(pin_name + "_undefined", pin_stream.consume());
            pin_stream.consume(";", true);
        }
        else
        {
            pin_stream.consume();
        }
    }

    return true;
}

bool gate_library_parser_liberty::parse_ff(token_stream& cell_stream)
{
    cell_stream.consume("ff");
    cell_stream.consume("(", true);
    m_current_cell.state1 = cell_stream.consume().string;
    cell_stream.consume(",", true);
    m_current_cell.state2 = cell_stream.consume().string;
    cell_stream.consume(")", true);
    cell_stream.consume("{", true);
    auto ff_stream = cell_stream.extract_until("}", token_stream::END_OF_STREAM, true, true);
    cell_stream.consume("}", true);
    m_current_cell.type = gate_type::base_type::ff;

    while (ff_stream.remaining() > 0)
    {
        if (ff_stream.consume("next_state"))
        {
            ff_stream.consume(":", true);
            m_current_cell.next_state = ff_stream.consume();
            ff_stream.consume(";", true);
        }
        else if (ff_stream.consume("clear"))
        {
            ff_stream.consume(":", true);
            m_current_cell.reset = ff_stream.consume();
            ff_stream.consume(";", true);
        }
        else if (ff_stream.consume("preset"))
        {
            ff_stream.consume(":", true);
            m_current_cell.set = ff_stream.consume();
            ff_stream.consume(";", true);
        }
        else if (ff_stream.consume("clocked_on"))
        {
            ff_stream.consume(":", true);
            m_current_cell.clocked_on = ff_stream.consume();
            ff_stream.consume(";", true);
        }
        else if (ff_stream.consume("clear_preset_var1"))
        {
            ff_stream.consume(":", true);
            auto behav = ff_stream.consume();
            ff_stream.consume(";", true);

            std::string behav_string = "LHNTX";
            auto pos                 = behav_string.find(behav);

            if (pos != std::string::npos)
            {
                m_current_cell.special_behavior_var1 = gate_type_sequential::set_reset_behavior(pos);
            }
            else
            {
                log_error("netlist", "invalid clear_preset behavior '{}' near line {}.", behav.string, behav.number);
                return false;
            }
        }
        else if (ff_stream.consume("clear_preset_var2"))
        {
            ff_stream.consume(":", true);
            auto behav = ff_stream.consume();
            ff_stream.consume(";", true);

            std::string behav_string = "LHNTX";
            auto pos                 = behav_string.find(behav);

            if (pos != std::string::npos)
            {
                m_current_cell.special_behavior_var2 = gate_type_sequential::set_reset_behavior(pos);
            }
            else
            {
                log_error("netlist", "invalid clear_preset behavior '{}' near line {}.", behav.string, behav.number);
                return false;
            }
        }
        else if (ff_stream.consume("data_category"))
        {
            ff_stream.consume(":", true);
            m_current_cell.data_category = ff_stream.consume();
            ff_stream.consume(";", true);
        }
        else if (ff_stream.consume("data_key"))
        {
            ff_stream.consume(":", true);
            m_current_cell.data_identifier = ff_stream.consume();
            ff_stream.consume(";", true);
        }
        else if (ff_stream.consume("direction"))
        {
            ff_stream.consume(":", true);
            auto direction = ff_stream.consume();

            if (direction == "ascending" || direction == "descending")
            {
                m_current_cell.data_direction = direction;
            }
            else
            {
                log_error("netlist", "invalid data direction '{}' near line {}.", direction.string, direction.number);
                return false;
            }

            ff_stream.consume(";", true);
        }
        else
        {
            ff_stream.consume();
        }
    }

    return true;
}

bool gate_library_parser_liberty::parse_latch(token_stream& cell_stream)
{
    cell_stream.consume("latch");
    cell_stream.consume("(", true);
    m_current_cell.state1 = cell_stream.consume().string;
    cell_stream.consume(",", true);
    m_current_cell.state2 = cell_stream.consume().string;
    cell_stream.consume(")", true);
    cell_stream.consume("{", true);
    auto latch_stream = cell_stream.extract_until("}", token_stream::END_OF_STREAM, true, true);
    cell_stream.consume("}", true);
    m_current_cell.type = gate_type::base_type::latch;

    while (latch_stream.remaining() > 0)
    {
        if (latch_stream.consume("data_in"))
        {
            latch_stream.consume(":", true);
            m_current_cell.next_state = latch_stream.consume();
            latch_stream.consume(";", true);
        }
        else if (latch_stream.consume("clear"))
        {
            latch_stream.consume(":", true);
            m_current_cell.reset = latch_stream.consume();
            latch_stream.consume(";", true);
        }
        else if (latch_stream.consume("preset"))
        {
            latch_stream.consume(":", true);
            m_current_cell.set = latch_stream.consume();
            latch_stream.consume(";", true);
        }
        else if (latch_stream.consume("enable"))
        {
            latch_stream.consume(":", true);
            m_current_cell.clocked_on = latch_stream.consume();
            latch_stream.consume(";", true);
        }
        else if (latch_stream.consume("clear_preset_var1"))
        {
            latch_stream.consume(":", true);
            auto behav = latch_stream.consume();
            latch_stream.consume(";", true);

            std::string behav_string = "LHNTX";
            auto pos                 = behav_string.find(behav);

            if (pos != std::string::npos)
            {
                m_current_cell.special_behavior_var1 = gate_type_sequential::set_reset_behavior(pos);
            }
            else
            {
                log_error("netlist", "invalid clear_preset behavior '{}' near line {}.", behav.string, behav.number);
                return false;
            }
        }
        else if (latch_stream.consume("clear_preset_var2"))
        {
            latch_stream.consume(":", true);
            auto behav = latch_stream.consume();
            latch_stream.consume(";", true);

            std::string behav_string = "LHNTX";
            auto pos                 = behav_string.find(behav);

            if (pos != std::string::npos)
            {
                m_current_cell.special_behavior_var2 = gate_type_sequential::set_reset_behavior(pos);
            }
            else
            {
                log_error("netlist", "invalid clear_preset behavior '{}' near line {}.", behav.string, behav.number);
                return false;
            }
        }
        else
        {
            latch_stream.consume();
        }
    }

    return true;
}

bool gate_library_parser_liberty::parse_lut(token_stream& cell_stream)
{
    cell_stream.consume("lut");
    cell_stream.consume("(", true);
    m_current_cell.state1 = cell_stream.consume().string;
    cell_stream.consume(")", true);
    cell_stream.consume("{", true);
    auto lut_stream = cell_stream.extract_until("}", token_stream::END_OF_STREAM, true, true);
    cell_stream.consume("}", true);
    m_current_cell.type = gate_type::base_type::lut;

    while (lut_stream.remaining() > 0)
    {
        if (lut_stream.consume("data_category"))
        {
            lut_stream.consume(":", true);
            m_current_cell.data_category = lut_stream.consume();
            lut_stream.consume(";", true);
        }
        else if (lut_stream.consume("data_identifier"))
        {
            lut_stream.consume(":", true);
            m_current_cell.data_identifier = lut_stream.consume();
            lut_stream.consume(";", true);
        }
        else if (lut_stream.consume("direction"))
        {
            lut_stream.consume(":", true);
            auto direction = lut_stream.consume();

            if (direction == "ascending" || direction == "descending")
            {
                m_current_cell.data_direction = direction;
            }
            else
            {
                log_error("netlist", "invalid data direction '{}' near line {}.", direction.string, direction.number);
                return false;
            }

            lut_stream.consume(";", true);
        }
        else
        {
            lut_stream.consume();
        }
    }

    return true;
}

std::shared_ptr<gate_type> gate_library_parser_liberty::construct_gate_type()
{
    std::shared_ptr<gate_type> gt;

    if (m_current_cell.type == gate_type::base_type::combinatorial)
    {
        gt = std::make_shared<gate_type>(m_current_cell.name);

        for (auto& [pin_name, bf] : m_current_cell.functions)
        {
            gt->add_boolean_function(pin_name, boolean_function::from_string(bf));
        }
    }
    else if (m_current_cell.type == gate_type::base_type::ff || m_current_cell.type == gate_type::base_type::latch)
    {
        auto seq_gt = std::make_shared<gate_type_sequential>(m_current_cell.name, m_current_cell.type);

        bool is_ff = (m_current_cell.type == gate_type::base_type::ff);

        if (!m_current_cell.next_state.empty())
        {
            seq_gt->add_boolean_function(is_ff ? "next_state" : "data_in", boolean_function::from_string(m_current_cell.next_state));
        }

        if (!m_current_cell.clocked_on.empty())
        {
            seq_gt->add_boolean_function(is_ff ? "clock" : "enable", boolean_function::from_string(m_current_cell.clocked_on));
        }

        if (!m_current_cell.set.empty())
        {
            seq_gt->add_boolean_function("set", boolean_function::from_string(m_current_cell.set));
        }

        if (!m_current_cell.reset.empty())
        {
            seq_gt->add_boolean_function("reset", boolean_function::from_string(m_current_cell.reset));
        }

        seq_gt->set_set_reset_behavior(m_current_cell.special_behavior_var1, m_current_cell.special_behavior_var2);
        seq_gt->set_init_data_category(m_current_cell.data_category);
        seq_gt->set_init_data_identifier(m_current_cell.data_identifier);

        for (auto& [pin_name, bf_string] : m_current_cell.functions)
        {
            if (bf_string == m_current_cell.state1)
            {
                seq_gt->add_state_output_pin(pin_name);
            }
            else if (bf_string == m_current_cell.state2)
            {
                seq_gt->add_inverted_state_output_pin(pin_name);
            }
            else
            {
                seq_gt->add_boolean_function(pin_name, boolean_function::from_string(bf_string));
            }
        }

        gt = seq_gt;
    }
    else if (m_current_cell.type == gate_type::base_type::lut)
    {
        auto lut_gt = std::make_shared<gate_type_lut>(m_current_cell.name);

        lut_gt->set_config_data_category(m_current_cell.data_category);
        lut_gt->set_config_data_identifier(m_current_cell.data_identifier);
        lut_gt->set_config_data_ascending_order(m_current_cell.data_direction == "ascending");

        for (auto& [pin_name, bf_string] : m_current_cell.functions)
        {
            if (bf_string == m_current_cell.state1)
            {
                lut_gt->add_output_from_init_string_pin(pin_name);
            }
            else
            {
                lut_gt->add_boolean_function(pin_name, boolean_function::from_string(bf_string));
            }
        }

        gt = lut_gt;
    }

    gt->add_input_pins(m_current_cell.input_pins);
    gt->add_output_pins(m_current_cell.output_pins);

    return gt;
}

void gate_library_parser_liberty::remove_comments(std::string& line, bool& multi_line_comment)
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
