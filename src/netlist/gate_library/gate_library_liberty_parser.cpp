#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/gate_decorator_system.h"

#include "netlist/gate.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_library_liberty_parser.h"

#include "core/log.h"
#include "core/utils.h"

#include <iostream>
#include <regex>
#include <sstream>

namespace gate_library_liberty_parser
{
    // ###########################################################################
    // #########          Parse liberty into intermediate format          ########
    // ###########################################################################

    std::shared_ptr<gate_library> parse(std::stringstream& ss)
    {
        std::shared_ptr<statement> root = get_statements(ss);

        if (root == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<intermediate_library> inter_lib = get_intermediate_library(root);

        if (root == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<gate_library> lib = get_gate_library(inter_lib);

        return lib;
    }

    std::shared_ptr<statement> get_statements(std::stringstream& ss)
    {
        std::shared_ptr<statement> current_group = nullptr;
        std::shared_ptr<statement> root          = nullptr;

        std::set<std::string> groups_of_interest     = {"library", "cell", "pin", "ff", "latch"};
        std::set<std::string> attributes_of_interest = {
            "direction", "function", "three_state", "x_function", "next_state", "clocked_on", "clear", "preset", "clear_preset_var1", "clear_preset_var2", "data_in", "enable"};

        std::string line;

        u32 line_number         = 0;
        i32 ignore_depth        = 0;
        i32 ignore_brackets     = 0;
        bool multi_line_comment = false;

        while (std::getline(ss, line))
        {
            line_number++;
            ignore_brackets = 0;

            // remove single-line and multi-line comments
            remove_comments(line, multi_line_comment);

            line = core_utils::trim(line);

            // skip empty lines
            if (line.empty())
            {
                continue;
            }

            // currently in ignored group?
            if (ignore_depth > 0)
            {
                auto closing_brackets = std::count(line.begin(), line.end(), '}');
                ignore_brackets       = ignore_depth;

                ignore_depth += std::count(line.begin(), line.end(), '{') - closing_brackets;

                // more closing brackets than should be ignored?
                if (ignore_depth <= 0)
                {
                    ignore_depth = 0;

                    for (i32 i = 0; i < closing_brackets - ignore_brackets; i++)
                    {
                        // move back in tree
                        current_group = current_group->parent.lock();
                    }
                }

                continue;
            }

            // detect and handle group statement
            auto statement_curly     = line.find('{');
            auto statement_semicolon = line.find(';');

            if (statement_curly != std::string::npos)
            {
                auto group_bracket = line.find('(');

                auto group_name = core_utils::trim(line.substr(0, group_bracket));
                auto name       = core_utils::trim(line.substr(0, line.find(')')).substr(group_bracket + 1));

                if (groups_of_interest.find(group_name) != groups_of_interest.end())
                {
                    auto group = std::make_shared<statement>(current_group, true, group_name, name);

                    if (root != nullptr)
                    {
                        current_group->statements.push_back(group);
                    }
                    else
                    {
                        root = group;
                    }

                    current_group = group;
                }
                else
                {
                    ignore_depth += 1;
                }
            }
            // detect and handle attribute statement
            else if (statement_semicolon != std::string::npos)
            {
                auto attribute_colon = line.find(':');

                // simple attribute
                if (attribute_colon != std::string::npos)
                {
                    auto name  = core_utils::trim(line.substr(0, attribute_colon));
                    auto value = core_utils::trim(line.substr(0, statement_semicolon).substr(attribute_colon + 1));

                    if (attributes_of_interest.find(name) != attributes_of_interest.end())
                    {
                        auto attribute = std::make_shared<statement>(current_group, false, name, value);

                        if (current_group != nullptr)
                        {
                            current_group->statements.push_back(attribute);
                        }
                    }
                }
            }

            for (i32 i = 0; i < std::count(line.begin(), line.end(), '}'); i++)
            {
                // move back in tree
                current_group = current_group->parent.lock();
            }
        }

        return root;
    }

    static std::string prepare_string(std::string str)
    {
        return str.substr(0, str.rfind("\"")).substr(str.find("\"") + 1);
    }

    std::shared_ptr<intermediate_library> get_intermediate_library(std::shared_ptr<statement> root)
    {
        std::shared_ptr<intermediate_library> inter_lib = std::make_shared<intermediate_library>();

        // depth: 0
        if (root->name == "library")
        {
            inter_lib->name = root->value;

            for (const auto& s1 : root->statements)
            {
                // depth: 1
                if (s1->name == "cell")
                {
                    cell c(s1->value);

                    for (const auto& s2 : s1->statements)
                    {
                        // depth 2
                        if (s2->name == "pin")
                        {
                            pin p(s2->value);

                            for (const auto& s3 : s2->statements)
                            {
                                // depth 3
                                if (s3->name == "direction")
                                {
                                    p.direction = s3->value;
                                }
                                else if (s3->name == "function")
                                {
                                    p.function = streamline_function(prepare_string(s3->value));
                                    m_cells_to_pin_functions[inter_lib->name][c.name].push_back(std::make_pair(p.name, p.function));
                                }
                                else if (s3->name == "three_state")
                                {
                                    p.three_state = streamline_function(prepare_string(s3->value));
                                }
                                else if (s3->name == "x_function")
                                {
                                    p.x_function = streamline_function(prepare_string(s3->value));
                                }
                            }

                            c.pins.push_back(p);
                        }
                        else if (s2->name == "ff")
                        {
                            c.is_ff = true;

                            for (const auto& s3 : s2->statements)
                            {
                                // depth 3
                                if (s3->name == "clocked_on")
                                {
                                    c.clocked_on = streamline_function(prepare_string(s3->value));
                                }
                                else if (s3->name == "next_state")
                                {
                                    c.next_state = streamline_function(prepare_string(s3->value));
                                }
                                else if (s3->name == "clear")
                                {
                                    c.clear = streamline_function(prepare_string(s3->value));
                                }
                                else if (s3->name == "preset")
                                {
                                    c.preset = streamline_function(prepare_string(s3->value));
                                }
                                else if (s3->name == "clear_preset_var1")
                                {
                                    c.clear_preset_var1 = prepare_string(s3->value);
                                }
                                else if (s3->name == "clear_preset_var2")
                                {
                                    c.clear_preset_var2 = prepare_string(s3->value);
                                }
                            }
                        }
                        else if (s2->name == "latch")
                        {
                            c.is_latch = true;

                            for (const auto& s3 : s2->statements)
                            {
                                // depth 3
                                if (s3->name == "enable")
                                {
                                    c.enable = s3->value;
                                }
                                else if (s3->name == "data_in")
                                {
                                    c.data_in = s3->value;
                                }
                                else if (s3->name == "clear")
                                {
                                    c.clear = s3->value;
                                }
                                else if (s3->name == "preset")
                                {
                                    c.preset = s3->value;
                                }
                                else if (s3->name == "clear_preset_var1")
                                {
                                    c.clear_preset_var1 = s3->value;
                                }
                                else if (s3->name == "clear_preset_var2")
                                {
                                    c.clear_preset_var2 = s3->value;
                                }
                            }
                        }
                    }

                    inter_lib->cells.push_back(c);
                }
            }
        }
        else
        {
            log_error("netlist", "gate library does not start with 'library' node.");
            return nullptr;
        }

        return inter_lib;
    }

    // ###########################################################################
    // ######          Build gate library from intermediate format         #######
    // ###########################################################################

    std::shared_ptr<gate_library> get_gate_library(std::shared_ptr<intermediate_library> inter_lib)
    {
        std::shared_ptr<gate_library> lib = std::make_shared<gate_library>(inter_lib->name);

        auto gate_types          = lib->get_gate_types();
        auto input_pin_types     = lib->get_input_pin_types();
        auto inout_pin_types     = lib->get_inout_pin_types();
        auto output_pin_types    = lib->get_output_pin_types();
        auto& gate_to_input_map  = *lib->get_gate_type_map_to_input_pin_types();
        auto& gate_to_inout_map  = *lib->get_gate_type_map_to_inout_pin_types();
        auto& gate_to_output_map = *lib->get_gate_type_map_to_output_pin_types();
        auto global_gnd_types    = lib->get_global_gnd_gate_types();
        auto global_vcc_types    = lib->get_global_vcc_gate_types();

        for (const auto& cell : inter_lib->cells)
        {
            gate_types->insert(cell.name);

            for (const auto& pin : cell.pins)
            {
                if (pin.direction == "input")
                {
                    input_pin_types->insert(pin.name);
                    gate_to_input_map[cell.name].push_back(pin.name);
                }
                else if (pin.direction == "inout")
                {
                    inout_pin_types->insert(pin.name);
                    gate_to_inout_map[cell.name].push_back(pin.name);
                }
                else if (pin.direction == "output")
                {
                    output_pin_types->insert(pin.name);
                    gate_to_output_map[cell.name].push_back(pin.name);

                    if (pin.function == "0")
                    {
                        global_gnd_types->insert(cell.name);
                    }
                    else if (pin.function == "1")
                    {
                        global_vcc_types->insert(cell.name);
                    }
                }
            }
        }

        if (global_gnd_types->empty())
        {
            global_gnd_types->insert("GLOBAL_GND");
            gate_types->insert("GLOBAL_GND");
            output_pin_types->insert("O");
            gate_to_output_map["GLOBAL_GND"].push_back("O");
        }
        else if (global_vcc_types->empty())
        {
            global_vcc_types->insert("GLOBAL_VCC");
            gate_types->insert("GLOBAL_VCC");
            output_pin_types->insert("O");
            gate_to_output_map["GLOBAL_VCC"].push_back("O");
        }

        return lib;
    }

    // ###########################################################################
    // ###################          Helper functions          ####################
    // ###########################################################################

    void remove_comments(std::string& line, bool& multi_line_comment)
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

            auto single_line_comment_begin = line.find("//");
            auto multi_line_comment_begin  = line.find("/*");
            auto multi_line_comment_end    = line.find("*/");

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
            else
            {
                if (single_line_comment_begin != std::string::npos)
                {
                    if (multi_line_comment_begin == std::string::npos || (multi_line_comment_begin != std::string::npos && multi_line_comment_begin > single_line_comment_begin))
                    {
                        // single-line comment
                        line   = line.substr(0, single_line_comment_begin);
                        repeat = true;
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
    }

    std::string streamline_function(const std::string& func)
    {
        // streamline and, or, and xor
        static const std::regex re_and("\\s*[\\*\\&]\\s*");
        static const std::regex re_or("\\s*[\\+\\|]\\s*");
        static const std::regex re_xor("\\s*[\\^]\\s*");
        static const std::regex re_space("\\s+");
        static const std::regex re_neg("(\\w+)\\'");

        auto res = func;
        res      = std::regex_replace(func, re_and, "&");
        res      = std::regex_replace(res, re_or, "|");
        res      = std::regex_replace(res, re_xor, "^");
        res      = std::regex_replace(res, re_space, "&");

        // streamline negation
        while (res.find(")'") != std::string::npos)
        {
            i32 bracket_level = 0;
            i32 open_pos      = 0;
            i32 close_pos     = res.rfind(")'");

            for (i32 i = close_pos - 1; i >= 0; i--)
            {
                if (res[i] == ')')
                {
                    bracket_level++;
                }
                else if (res[i] == '(')
                {
                    if (bracket_level == 0)
                    {
                        open_pos = i;
                        break;
                    }

                    bracket_level--;
                }
            }

            res = res.substr(0, open_pos) + "!" + res.substr(open_pos, close_pos - open_pos + 1) + res.substr(close_pos + 2);
        }

        res = std::regex_replace(res, re_neg, "!$1");

        return res;
    }

    bdd build_bdd(const bdd& first, const bdd& second, char current_op, bool neg)
    {
        bdd res;

        if (neg == true)
        {
            res = bdd_not(second);
        }
        else
        {
            res = second;
        }

        if (current_op == '&')
        {
            res = first & res;
        }
        else if (current_op == '|')
        {
            res = first | res;
        }
        else if (current_op == '^')
        {
            res = first ^ res;
        }

        return res;
    }

    bdd function_to_bdd(const std::string& func, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        bdd res;

        std::string boolean_operators = "&|^";

        // bracket handling
        u32 bracket_level = 0;
        auto open_pos     = std::string::npos;

        // BDD generation
        std::string pin_name = "";
        char current_op      = '\0';
        bool local_neg       = false;

        // check for values 1 and 0
        if (func == "1")
        {
            return bdd_true();
        }
        else if (func == "0")
        {
            return bdd_false();
        }

        // parse equation
        for (u32 i = 0; i < func.size(); i++)
        {
            // recursively call function for parts within brackets
            if (func[i] == '(')
            {
                bracket_level++;

                if (open_pos == std::string::npos)
                {
                    open_pos = i;
                }
            }
            else if (func[i] == ')')
            {
                bracket_level--;

                if (bracket_level == 0)
                {
                    bdd tmp = function_to_bdd(func.substr(open_pos + 1, i - open_pos - 1), input_pin_type_to_bdd);
                    build_bdd(res, tmp, current_op, local_neg);
                    open_pos = std::string::npos;
                }
            }
            // if not within brackets, deal with the operands
            else if (bracket_level == 0)
            {
                if (func[i] == '!')
                {
                    local_neg = true;
                }
                else if (boolean_operators.find(func[i]) == std::string::npos)
                {
                    pin_name += func[i];
                }
                else
                {
                    res = build_bdd(res, *(input_pin_type_to_bdd[pin_name]), current_op, local_neg);

                    pin_name   = "";
                    current_op = func[i];
                    local_neg  = false;
                }
            }
        }

        if (!pin_name.empty())
        {
            res = build_bdd(res, *(input_pin_type_to_bdd[pin_name]), current_op, local_neg);
        }

        return res;
    }
}    // namespace gate_library_liberty_parser