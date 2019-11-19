#include "netlist/gate_library/gate_library_parser/gate_library_parser_liberty.h"

#include "core/log.h"

#include "netlist/boolean_function.h"
#include "netlist/gate_library/gate_type/gate_type_ff.h"
#include "netlist/gate_library/gate_type/gate_type_latch.h"
#include "netlist/gate_library/gate_type/gate_type_lut.h"

// #include "core/utils.h"
// #include "netlist/gate.h"
// #include "netlist/gate_library/gate_library.h"

#include <iostream>
// #include <regex>
// #include <sstream>

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
                m_current_cell.special_behavior_var1 = gate_type::special_behavior(pos);
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
                m_current_cell.special_behavior_var2 = gate_type::special_behavior(pos);
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
                m_current_cell.special_behavior_var1 = gate_type::special_behavior(pos);
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
                m_current_cell.special_behavior_var2 = gate_type::special_behavior(pos);
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
    else if (m_current_cell.type == gate_type::base_type::ff)
    {
        std::shared_ptr<gate_type_ff> gt_ff = std::make_shared<gate_type_ff>(m_current_cell.name);

        gt_ff->set_next_state_function(boolean_function::from_string(m_current_cell.next_state));
        gt_ff->set_clock_function(boolean_function::from_string(m_current_cell.clocked_on));
        gt_ff->set_set_function(boolean_function::from_string(m_current_cell.set));
        gt_ff->set_reset_function(boolean_function::from_string(m_current_cell.reset));
        gt_ff->set_special_behavior(m_current_cell.special_behavior_var1, m_current_cell.special_behavior_var2);
        gt_ff->set_data_category(m_current_cell.data_category);
        gt_ff->set_data_identifier(m_current_cell.data_identifier);
        gt_ff->set_data_ascending_order(m_current_cell.data_direction == "ascending");

        for (auto& [pin_name, bf] : m_current_cell.functions)
        {
            if (bf == m_current_cell.state1)
            {
                gt_ff->add_state_output_pin(pin_name);
            }
            else if (bf == m_current_cell.state2)
            {
                gt_ff->add_inverted_state_output_pin(pin_name);
            }
            else
            {
                gt_ff->add_boolean_function(pin_name, boolean_function::from_string(bf));
            }
        }

        gt = gt_ff;
    }
    else if (m_current_cell.type == gate_type::base_type::latch)
    {
        std::shared_ptr<gate_type_latch> gt_latch = std::make_shared<gate_type_latch>(m_current_cell.name);

        gt_latch->set_data_in_function(boolean_function::from_string(m_current_cell.next_state));
        gt_latch->set_enable_function(boolean_function::from_string(m_current_cell.clocked_on));
        gt_latch->set_set_function(boolean_function::from_string(m_current_cell.set));
        gt_latch->set_reset_function(boolean_function::from_string(m_current_cell.reset));
        gt_latch->set_special_behavior(m_current_cell.special_behavior_var1, m_current_cell.special_behavior_var2);

        for (auto& [pin_name, bf] : m_current_cell.functions)
        {
            if (bf == m_current_cell.state1)
            {
                gt_latch->add_state_output_pin(pin_name);
            }
            else if (bf == m_current_cell.state2)
            {
                gt_latch->add_inverted_state_output_pin(pin_name);
            }
            else
            {
                gt_latch->add_boolean_function(pin_name, boolean_function::from_string(bf));
            }
        }

        gt = gt_latch;
    }
    else if (m_current_cell.type == gate_type::base_type::lut)
    {
        std::shared_ptr<gate_type_lut> gt_lut = std::make_shared<gate_type_lut>(m_current_cell.name);

        gt_lut->set_data_category(m_current_cell.data_category);
        gt_lut->set_data_identifier(m_current_cell.data_identifier);
        gt_lut->set_data_ascending_order(m_current_cell.data_direction == "ascending");

        for (auto& [pin_name, bf] : m_current_cell.functions)
        {
            if (bf == m_current_cell.state1)
            {
                gt_lut->add_output_from_init_string_pin(pin_name);
            }
            else
            {
                gt_lut->add_boolean_function(pin_name, boolean_function::from_string(bf));
            }
        }

        gt = gt_lut;
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

// namespace gate_library_parser_liberty
// {
//     struct statement
//     {
//         statement()
//         {
//         }

//         statement(statement* p_parent, bool p_is_group, std::string p_name, std::string p_value) : parent(p_parent), is_group(p_is_group), name(p_name), value(p_value)
//         {
//         }

//         ~statement()
//         {
//             for (auto x : statements)
//             {
//                 for (auto y : x.second)
//                 {
//                     delete y;
//                 }
//             }
//         }

//         statement* parent;
//         bool is_group;
//         std::string name;
//         std::string value;
//         std::map<std::string, std::vector<statement*>> statements;
//     };

//     struct sequential
//     {
//         std::pair<std::string, std::string> output_state;
//         std::map<std::string, boolean_function> functions;
//         std::pair<char, char> set_rst;
//     };

//     // stores the input stream to the file
//     std::stringstream& m_fs;

//     bool tokenize();
//     bool parse_tokens();

//     statement* get_statements(std::stringstream& ss);
//     std::shared_ptr<gate_library> get_gate_library(statement* root);
//     void remove_comments(std::string& line, bool& multi_line_comment);

//     // ###########################################################################
//     // #########          Parse liberty into intermediate format          ########
//     // ###########################################################################

//     std::shared_ptr<gate_library> parse(std::stringstream& ss)
//     {
//         m_fs = ss;
//         return nullptr;
//         // auto statements = get_statements(ss);
//         // auto lib        = get_gate_library(statements);
//         // delete statements;
//         // return lib;
//     }

//     bool tokenize()
//     {
//     }

//     statement* get_statements(std::stringstream& ss)
//     {
//         statement* current_group = nullptr;
//         statement* root          = nullptr;

//         std::set<std::string> groups_of_interest     = {"library", "cell", "pin", "ff", "latch", "lut"};
//         std::set<std::string> attributes_of_interest = {"direction",
//                                                         "function",
//                                                         "x_function",
//                                                         "next_state",
//                                                         "clocked_on",
//                                                         "clear",
//                                                         "preset",
//                                                         "clear_preset_var1",
//                                                         "clear_preset_var2",
//                                                         "data_in",
//                                                         "enable",
//                                                         "data_category",
//                                                         "data_identifier",
//                                                         "bit_order"};

//         std::string line;

//         u32 line_number         = 0;
//         i32 ignore_depth        = 0;
//         i32 ignore_brackets     = 0;
//         bool multi_line_comment = false;

//         while (std::getline(ss, line))
//         {
//             line_number++;
//             ignore_brackets = 0;

//             // remove single-line and multi-line comments
//             remove_comments(line, multi_line_comment);

//             line = core_utils::trim(line);

//             // skip empty lines
//             if (line.empty())
//             {
//                 continue;
//             }

//             // currently in ignored group?
//             if (ignore_depth > 0)
//             {
//                 auto closing_brackets = std::count(line.begin(), line.end(), '}');
//                 ignore_brackets       = ignore_depth;

//                 ignore_depth += std::count(line.begin(), line.end(), '{') - closing_brackets;

//                 // more closing brackets than should be ignored?
//                 if (ignore_depth <= 0)
//                 {
//                     ignore_depth = 0;

//                     for (i32 i = 0; i < closing_brackets - ignore_brackets; i++)
//                     {
//                         // move back in tree
//                         current_group = current_group->parent;
//                     }
//                 }

//                 continue;
//             }

//             // detect and handle group statement
//             auto statement_curly     = line.find('{');
//             auto statement_semicolon = line.find(';');

//             if (statement_curly != std::string::npos)
//             {
//                 auto group_bracket = line.find('(');

//                 auto group_name = core_utils::trim(line.substr(0, group_bracket));
//                 auto name       = core_utils::trim(line.substr(0, line.find(')')).substr(group_bracket + 1));

//                 if (groups_of_interest.find(group_name) != groups_of_interest.end())
//                 {
//                     auto new_group = new statement(current_group, true, group_name, name);
//                     if (root != nullptr)
//                     {
//                         current_group->statements[group_name].push_back(new_group);
//                     }
//                     else
//                     {
//                         root = new_group;
//                     }

//                     current_group = new_group;
//                 }
//                 else
//                 {
//                     ignore_depth += 1;
//                 }
//             }
//             // detect and handle attribute statement
//             else if (statement_semicolon != std::string::npos)
//             {
//                 auto attribute_colon = line.find(':');

//                 // simple attribute
//                 if (attribute_colon != std::string::npos)
//                 {
//                     auto name  = core_utils::trim(line.substr(0, attribute_colon));
//                     auto value = core_utils::trim(line.substr(0, statement_semicolon).substr(attribute_colon + 1));

//                     if (attributes_of_interest.find(name) != attributes_of_interest.end())
//                     {
//                         if (current_group != nullptr)
//                         {
//                             current_group->statements[name].push_back(new statement(current_group, false, name, value));
//                         }
//                     }
//                 }
//             }

//             for (i32 i = 0; i < std::count(line.begin(), line.end(), '}'); i++)
//             {
//                 // move back in tree
//                 current_group = current_group->parent;
//             }
//         }

//         return root;
//     }

//     static std::string prepare_string(std::string str)
//     {
//         return str.substr(0, str.rfind("\"")).substr(str.find("\"") + 1);
//     }

//     bool parse_pins(const std::vector<statement*>& statements, std::shared_ptr<gate_type>& gt, std::pair<std::string, std::string>& internal_state_names)
//     {
//         for (const auto& pin : statements)
//         {
//             for (const auto& s : pin->statements)
//             {
//                 if (s.first == "direction")
//                 {
//                     if (s.second[0]->value == "input")
//                     {
//                         gt->add_input_pin(pin->value);
//                     }
//                     else if (s.second[0]->value == "output")
//                     {
//                         gt->add_output_pin(pin->value);
//                     }
//                 }

//                 if (s.first == "function")
//                 {
//                     if (gt->get_base_type() == gate_type::base_type::combinatorial)
//                     {
//                         gt->add_boolean_function(pin->value, boolean_function::from_string(prepare_string(s.second[0]->value)));
//                     }
//                     else if (gt->get_base_type() == gate_type::base_type::ff)
//                     {
//                         auto ff_ptr = std::dynamic_pointer_cast<gate_type_ff>(gt);

//                         if (s.second[0]->value == internal_state_names.second)
//                         {
//                             ff_ptr->set_output_pin_inverted(pin->value, true);
//                         }
//                     }
//                     else if (gt->get_base_type() == gate_type::base_type::latch)
//                     {
//                         auto latch_ptr = std::dynamic_pointer_cast<gate_type_latch>(gt);

//                         if (s.second[0]->value == internal_state_names.second)
//                         {
//                             latch_ptr->set_output_pin_inverted(pin->value, true);
//                         }
//                     }
//                 }
//                 else if (s.first == "x_function")
//                 {
//                     gt->add_boolean_function(pin->value + "_undefined", boolean_function::from_string(prepare_string(s.second[0]->value)));
//                 }
//             }
//         }

//         return true;
//     }

//     std::pair<std::string, std::string> parse_ff_block(statement* block, std::shared_ptr<gate_type_ff> gt)
//     {
//         for (const auto& s : block->statements)
//         {
//             if (s.first == "clocked_on")
//             {
//                 gt->set_clock_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "next_state")
//             {
//                 gt->set_next_state_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "clear")
//             {
//                 gt->set_reset_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "preset")
//             {
//                 gt->set_set_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "clear_preset_var1")
//             {
//                 switch (prepare_string(s.second[0]->value).at(0))
//                 {
//                     case 'L':
//                         gt->set_special_behavior1(gate_type_ff::special_behavior::L);
//                         break;
//                     case 'H':
//                         gt->set_special_behavior1(gate_type_ff::special_behavior::H);
//                         break;
//                     case 'N':
//                         gt->set_special_behavior1(gate_type_ff::special_behavior::N);
//                         break;
//                     case 'T':
//                         gt->set_special_behavior1(gate_type_ff::special_behavior::T);
//                         break;
//                     case 'X':
//                         gt->set_special_behavior1(gate_type_ff::special_behavior::X);
//                         break;
//                 }
//             }
//             else if (s.first == "clear_preset_var2")
//             {
//                 switch (prepare_string(s.second[0]->value).at(0))
//                 {
//                     case 'L':
//                         gt->set_special_behavior2(gate_type_ff::special_behavior::L);
//                         break;
//                     case 'H':
//                         gt->set_special_behavior2(gate_type_ff::special_behavior::H);
//                         break;
//                     case 'N':
//                         gt->set_special_behavior2(gate_type_ff::special_behavior::N);
//                         break;
//                     case 'T':
//                         gt->set_special_behavior2(gate_type_ff::special_behavior::T);
//                         break;
//                     case 'X':
//                         gt->set_special_behavior2(gate_type_ff::special_behavior::X);
//                         break;
//                 }
//             }
//         }

//         auto tokens = core_utils::split(block->value, ',');
//         return {prepare_string(tokens[0]), prepare_string(tokens[1])};
//     }

//     std::pair<std::string, std::string> parse_latch_block(statement* block, std::shared_ptr<gate_type_latch> gt)
//     {
//         for (const auto& s : block->statements)
//         {
//             if (s.first == "enable")
//             {
//                 gt->set_enable_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "data_in")
//             {
//                 gt->set_data_in_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "clear")
//             {
//                 gt->set_reset_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "preset")
//             {
//                 gt->set_set_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
//             }
//             else if (s.first == "clear_preset_var1")
//             {
//                 switch (prepare_string(s.second[0]->value).at(0))
//                 {
//                     case 'L':
//                         gt->set_special_behavior1(gate_type_latch::special_behavior::L);
//                         break;
//                     case 'H':
//                         gt->set_special_behavior1(gate_type_latch::special_behavior::H);
//                         break;
//                     case 'N':
//                         gt->set_special_behavior1(gate_type_latch::special_behavior::N);
//                         break;
//                     case 'T':
//                         gt->set_special_behavior1(gate_type_latch::special_behavior::T);
//                         break;
//                     case 'X':
//                         gt->set_special_behavior1(gate_type_latch::special_behavior::X);
//                         break;
//                 }
//             }
//             else if (s.first == "clear_preset_var2")
//             {
//                 switch (prepare_string(s.second[0]->value).at(0))
//                 {
//                     case 'L':
//                         gt->set_special_behavior2(gate_type_latch::special_behavior::L);
//                         break;
//                     case 'H':
//                         gt->set_special_behavior2(gate_type_latch::special_behavior::H);
//                         break;
//                     case 'N':
//                         gt->set_special_behavior2(gate_type_latch::special_behavior::N);
//                         break;
//                     case 'T':
//                         gt->set_special_behavior2(gate_type_latch::special_behavior::T);
//                         break;
//                     case 'X':
//                         gt->set_special_behavior2(gate_type_latch::special_behavior::X);
//                         break;
//                 }
//             }
//         }

//         auto tokens = core_utils::split(block->value, ',');
//         return {prepare_string(tokens[0]), prepare_string(tokens[1])};
//     }

//     void parse_lut_block(statement* block, std::shared_ptr<gate_type_lut> gt)
//     {
//         for (const auto& s : block->statements)
//         {
//             if (s.first == "data_category")
//             {
//                 gt->set_data_category(prepare_string(s.second[0]->value));
//             }
//             else if (s.first == "data_identifier")
//             {
//                 gt->set_data_identifier(prepare_string(s.second[0]->value));
//             }
//             else if (s.first == "bit_order")
//             {
//                 if (prepare_string(s.second[0]->value) == "ascending")
//                 {
//                     gt->set_data_ascending_order(true);
//                 }
//                 else
//                 {
//                     gt->set_data_ascending_order(false);
//                 }
//             }
//         }
//     }

//     bool parse_cells(const std::vector<statement*>& statements, std::shared_ptr<gate_library>& lib)
//     {
//         for (const auto& cell : statements)
//         {
//             std::shared_ptr<gate_type> gt;
//             std::pair<std::string, std::string> internal_state_names;
//             if (cell->statements.find("ff") != cell->statements.end())
//             {
//                 auto ff_type         = std::make_shared<gate_type_ff>(cell->value);
//                 internal_state_names = parse_ff_block(cell->statements["ff"][0], ff_type);
//                 gt                   = ff_type;
//             }
//             else if (cell->statements.find("latch") != cell->statements.end())
//             {
//                 auto latch_type      = std::make_shared<gate_type_latch>(cell->value);
//                 internal_state_names = parse_latch_block(cell->statements["latch"][0], latch_type);
//                 gt                   = latch_type;
//             }
//             else if (cell->statements.find("lut") != cell->statements.end())
//             {
//                 auto lut_type = std::make_shared<gate_type_lut>(cell->value);
//                 parse_lut_block(cell->statements["lut"][0], lut_type);
//                 gt = lut_type;
//             }
//             else
//             {
//                 gt = std::make_shared<gate_type>(cell->value);
//             }

//             if (!parse_pins(cell->statements["pin"], gt, internal_state_names))
//             {
//                 log_error("netlist", "error while parsing pin definitions for cell '{}'", gt->get_name());
//                 return false;
//             }

//             lib->add_gate_type(gt);
//         }

//         return true;
//     }

//     std::shared_ptr<gate_library> get_gate_library(statement* root)
//     {
//         if (root->name != "library")
//         {
//             log_error("netlist", "gate library does not start with 'library' node.");
//             return nullptr;
//         }

//         auto lib = std::make_shared<gate_library>(root->value);

//         if (!parse_cells(root->statements["cell"], lib))
//         {
//             log_error("netlist", "error while parsing cell definitions");
//             return nullptr;
//         }

//         return lib;
//     }

//     // ###########################################################################
//     // ######          Build gate library from intermediate format         #######
//     // ###########################################################################

//     // std::shared_ptr<gate_library> get_gate_library(std::shared_ptr<intermediate_library> inter_lib)
//     // {
//     //     std::shared_ptr<gate_library> lib = std::make_shared<gate_library>(inter_lib->name);

//     //     std::vector<std::string> input_pins;
//     //     std::vector<std::string> output_pins;

//     //     for (const auto& cell : inter_lib->cells)
//     //     {
//     //         gate_type gt(cell.name);

//     //         for (const auto& pin : cell.pins)
//     //         {
//     //             if (pin.direction == "input")
//     //             {
//     //                 input_pins.push_back(pin.name);
//     //             }
//     //             else if (pin.direction == "output")
//     //             {
//     //                 output_pins.push_back(pin.name);
//     //             }

//     //             if (!pin.function.empty())
//     //             {
//     //                 if (pin.function == cell.output_state.first)
//     //                 {
//     //                     gt.add_boolean_function(pin.name, boolean_function::from_string(cell.data_in));
//     //                 }
//     //                 else if (pin.function == cell.output_state.second)
//     //                 {
//     //                     gt.add_boolean_function(pin.name, !(boolean_function::from_string(cell.data_in)));
//     //                 }
//     //                 else
//     //                 {
//     //                     gt.add_boolean_function(pin.name, boolean_function::from_string(pin.function));
//     //                 }
//     //             }
//     //         }

//     //         if (cell.is_ff)
//     //         {
//     //             gt.set_base_type(gate_type::ff);

//     //             // TODO special functions
//     //         }
//     //         else if (cell.is_latch)
//     //         {
//     //             gt.set_base_type(gate_type::latch);

//     //             // TODO special functions
//     //         }
//     //         else if (cell.is_lut)
//     //         {
//     //             gt.set_base_type(gate_type::lut);

//     //             // TODO LUT stuff
//     //         }
//     //         else
//     //         {
//     //             gt.set_base_type(gate_type::combinatorial);
//     //         }

//     //         gt.add_input_pins(input_pins);
//     //         gt.add_output_pins(output_pins);
//     //     }

//     //     return lib;
//     // }

//     // ###########################################################################
//     // ###################          Helper functions          ####################
//     // ###########################################################################

// }    // namespace gate_library_parser_liberty
