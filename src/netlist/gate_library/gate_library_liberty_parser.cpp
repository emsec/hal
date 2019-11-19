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
    namespace
    {
        bool bdd_availability_tester(std::map<std::string, std::vector<std::pair<std::string, boolean_function>>>& boolean_function_map, std::shared_ptr<gate> gate)
        {
            return boolean_function_map.find(gate->get_type()) != boolean_function_map.end();
        }

        std::map<std::string, std::shared_ptr<bdd>> bdd_generator(std::map<std::string, std::vector<std::pair<std::string, boolean_function>>>& boolean_function_map,
                                                                  std::shared_ptr<gate> gate,
                                                                  std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
        {
            std::map<std::string, std::shared_ptr<bdd>> res;

            auto pins = boolean_function_map[gate->get_type()];

            for (auto& pin : pins)
            {
                res[pin.first] = std::make_shared<bdd>(boolean_function_to_bdd(pin.second, input_pin_type_to_bdd));
            }

            return res;
        }
    }    // namespace

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

        if (inter_lib == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<gate_library> lib = get_gate_library(inter_lib);
        register_bdds(inter_lib);

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
        auto input_pins     = lib->get_input_pins();
        auto inout_pins     = lib->get_inout_pins();
        auto output_pins    = lib->get_output_pins();
        auto& gate_to_input_map  = *lib->get_gate_type_map_to_input_pins();
        auto& gate_to_inout_map  = *lib->get_gate_type_map_to_inout_pins();
        auto& gate_to_output_map = *lib->get_gate_type_map_to_output_pins();
        auto global_gnd_types    = lib->get_global_gnd_gate_types();
        auto global_vcc_types    = lib->get_global_vcc_gate_types();

        for (const auto& cell : inter_lib->cells)
        {
            gate_types->insert(cell.name);

            for (const auto& pin : cell.pins)
            {
                if (pin.direction == "input")
                {
                    input_pins->insert(pin.name);
                    gate_to_input_map[cell.name].push_back(pin.name);
                }
                else if (pin.direction == "inout")
                {
                    inout_pins->insert(pin.name);
                    gate_to_inout_map[cell.name].push_back(pin.name);
                }
                else if (pin.direction == "output")
                {
                    output_pins->insert(pin.name);
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
            output_pins->insert("O");
            gate_to_output_map["GLOBAL_GND"].push_back("O");
        }
        else if (global_vcc_types->empty())
        {
            global_vcc_types->insert("GLOBAL_VCC");
            gate_types->insert("GLOBAL_VCC");
            output_pins->insert("O");
            gate_to_output_map["GLOBAL_VCC"].push_back("O");
        }

        return lib;
    }

    void register_bdds(std::shared_ptr<intermediate_library> inter_lib)
    {
        std::map<std::string, std::vector<std::pair<std::string, boolean_function>>> boolean_function_map;

        // create boolean_function_map
        for (const auto& c : inter_lib->cells)
        {
            for (const auto& p : c.pins)
            {
                if ((p.direction == "inout") || (p.direction == "output"))
                {
                    if (!p.function.empty())
                    {
                        if ((p.function != "IQ") && (p.function != "IQN"))
                        {
                            boolean_function_map[c.name].push_back(std::make_pair(p.name, string_to_boolean_function(p.function)));
                        }
                    }
                }
            }
        }

        // register bdd availability tester and generator
        using namespace std::placeholders;

        gate_decorator_system::register_bdd_decorator_function(inter_lib->name, std::bind(&bdd_availability_tester, boolean_function_map, _1), std::bind(&bdd_generator, boolean_function_map, _1, _2));
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

    std::string streamline_function(const std::string& func)
    {
        // streamline and, or, and xor
        static const std::regex re_and("\\s*[\\*\\&]\\s*");
        static const std::regex re_or("\\s*[\\+\\|]\\s*");
        static const std::regex re_xor("\\s*[\\^]\\s*");
        static const std::regex re_space("\\s+");
        static const std::regex re_neg("(\\w+)\\'");

        auto res = func;
        res      = std::regex_replace(res, re_and, "&");
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

    boolean_function string_to_boolean_function(std::string func)
    {
        boolean_function b_func;

        std::string boolean_operators = "&|^";

        // bracket handling
        u32 bracket_level = 0;
        auto open_pos     = std::string::npos;

        // boolean function generation
        std::string pin_name = "";
        bool local_neg       = false;
        boolean_function current_b_func;

        // check for values 0 and 1
        if (func == "0" || func == "1")
        {
            b_func.operand = func;
            return b_func;
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
                    current_b_func     = string_to_boolean_function(func.substr(open_pos + 1, i - open_pos - 1));
                    current_b_func.neg = local_neg;
                    b_func.sub_functions.push_back(current_b_func);
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
                    if (!pin_name.empty())
                    {
                        current_b_func     = boolean_function(pin_name);
                        current_b_func.neg = local_neg;
                        b_func.sub_functions.push_back(current_b_func);
                    }

                    pin_name  = "";
                    b_func.op = func[i];
                    local_neg = false;
                }
            }
        }

        // final operand
        if (!pin_name.empty())
        {
            current_b_func     = boolean_function(pin_name);
            current_b_func.neg = local_neg;
            b_func.sub_functions.push_back(current_b_func);
        }

        return b_func;
    }

    bdd boolean_function_to_bdd(const boolean_function& b_func, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        bdd res;

        // single operand on this level
        if (!b_func.operand.empty())
        {
            // check for values 1 and 0
            if (b_func.operand == "0")
            {
                res = bdd_false();
            }
            else if (b_func.operand == "1")
            {
                res = bdd_true();
            }
            else
            {
                res = *(input_pin_type_to_bdd[b_func.operand]);
            }
        }
        // multiple operands on this level
        else
        {
            // single operand within operand list
            if (b_func.sub_functions.size() == 1)
            {
                res = boolean_function_to_bdd(b_func.sub_functions[0], input_pin_type_to_bdd);
            }
            // boolean operations
            else if (b_func.op == '&')
            {
                res = bdd_true();

                for (const auto& sub_func : b_func.sub_functions)
                {
                    res &= boolean_function_to_bdd(sub_func, input_pin_type_to_bdd);
                }
            }
            else if (b_func.op == '|')
            {
                res = bdd_false();

                for (const auto& sub_func : b_func.sub_functions)
                {
                    res |= boolean_function_to_bdd(sub_func, input_pin_type_to_bdd);
                }
            }
            else if (b_func.op == '^')
            {
                res = bdd_false();

                for (const auto& sub_func : b_func.sub_functions)
                {
                    res ^= boolean_function_to_bdd(sub_func, input_pin_type_to_bdd);
                }
            }
        }

        // negate function
        if (b_func.neg)
        {
            return bdd_not(res);
        }
        else
        {
            return res;
        }
    }
}    // namespace gate_library_liberty_parser
