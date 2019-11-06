#include "netlist/gate_library/gate_library_liberty_parser.h"
#include "netlist/boolean_function.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_type.h"

#include "core/log.h"
#include "core/utils.h"

#include <iostream>
#include <regex>
#include <sstream>

namespace gate_library_liberty_parser
{
    struct statement
    {
        statement()
        {
        }

        statement(statement* p_parent, bool p_is_group, std::string p_name, std::string p_value) : parent(p_parent), is_group(p_is_group), name(p_name), value(p_value)
        {
        }

        ~statement()
        {
            for (auto x : statements)
            {
                delete x;
            }
        }

        statement* parent;
        bool is_group;
        std::string name;
        std::string value;
        std::vector<statement*> statements;
    };

    struct sequential
    {
        std::pair<std::string, std::string> output_state;
        std::map<std::string, boolean_function> functions;
        std::pair<std::string, std::string> set_rst;
    };

    statement get_statements(std::stringstream& ss);
    std::shared_ptr<gate_library> get_gate_library(const statement& root);
    void remove_comments(std::string& line, bool& multi_line_comment);

    // ###########################################################################
    // #########          Parse liberty into intermediate format          ########
    // ###########################################################################

    std::shared_ptr<gate_library> parse(std::stringstream& ss)
    {
        return get_gate_library(get_statements(ss));
    }

    statement get_statements(std::stringstream& ss)
    {
        statement* current_group = nullptr;
        statement root;

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
                        current_group = current_group.parent;
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
                    if (current_group != nullptr)
                    {
                        current_group->statements.emplace_back(current_group, true, group_name, name);
                        current_group
                    }
                    else
                    {
                        root          = statement(current_group, true, group_name, name);
                        current_group = &root;
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
                        if (current_group != nullptr)
                        {
                            current_group->statements.emplace_back(current_group, false, name, value);
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

    std::shared_ptr<gate_library> get_gate_library(const statement& root)
    {
        std::shared_ptr<gate_library> lib;

        // depth: 0
        if (root.name == "library")
        {
            lib = std::make_shared<gate_library>(root.value);

            for (const auto& s1 : root.statements)
            {
                // depth: 1
                if (s1.name == "cell")
                {
                    sequential seq;
                    std::shared_ptr<gate_type> gt = std::make_shared<gate_type>(s1.value);

                    for (const auto& s2 : s1.statements)
                    {
                        // depth 2
                        if (s2.name == "pin")
                        {
                            for (const auto& s3 : s2.statements)
                            {
                                // depth 3
                                if (s3.name == "direction")
                                {
                                    if (s3.value == "input")
                                    {
                                        gt->add_input_pin(s2.value);
                                    }
                                }
                                else if (s3.name == "function")
                                {
                                    if (gt->get_base_type() == gate_type::combinatorial)
                                    {
                                        gt->add_boolean_function(s2.value, boolean_function::from_string(prepare_string(s3.value)));
                                    }
                                    else if (gt->get_base_type() == gate_type::ff)
                                    {
                                        if (s3.value == seq.output_state.first)
                                        {
                                            gt->add_boolean_function(s2.value, seq.functions.at("next_state"));
                                            gt->add_boolean_function("set_" + s2.value, seq.functions.at("set"));
                                            gt->add_boolean_function("reset_" + s2.value, !seq.functions.at("reset"));
                                        }
                                        else
                                        {
                                            gt->add_boolean_function(s2.value, !seq.functions.at("next_state"));
                                            gt->add_boolean_function("set_" + s2.value, !seq.functions.at("set"));
                                            gt->add_boolean_function("reset_" + s2.value, seq.functions.at("reset"));
                                        }
                                    }
                                }
                                else if (s3.name == "three_state")
                                {
                                    // TODO: Tri-State
                                    //p.three_state = prepare_string(s3.value);
                                }
                                else if (s3.name == "x_function")
                                {
                                    gt->add_boolean_function("undefined_" + s2.value, boolean_function::from_string(prepare_string(s3.value)));
                                }
                            }
                        }
                        else if (s2.name == "ff")
                        {
                            gt->set_base_type(gate_type::ff);

                            auto tokens      = core_utils::split(s2.value, ',');
                            seq.output_state = {prepare_string(tokens[0]), prepare_string(tokens[1])};

                            for (const auto& s3 : s2.statements)
                            {
                                // depth 3
                                if (s3.name == "clocked_on")
                                {
                                    gt->add_boolean_function("clock" + s2.value, boolean_function::from_string(prepare_string(s3.value)));
                                }
                                else if (s3.name == "next_state")
                                {
                                    seq.functions.emplace("next_state", boolean_function::from_string(prepare_string(s3.value)));
                                }
                                else if (s3.name == "clear")
                                {
                                    seq.functions.emplace("reset", boolean_function::from_string(prepare_string(s3.value)));
                                }
                                else if (s3.name == "preset")
                                {
                                    seq.functions.emplace("set", boolean_function::from_string(prepare_string(s3.value)));
                                }
                                else if (s3.name == "clear_preset_var1")
                                {
                                    seq.set_rst.first = prepare_string(s3.value);
                                }
                                else if (s3.name == "clear_preset_var2")
                                {
                                    seq.set_rst.second = prepare_string(s3.value);
                                }
                            }
                        }
                        // TODO
                        // else if (s2.name == "latch")
                        // {
                        //     c.is_latch = true;

                        //     for (const auto& s3 : s2.statements)
                        //     {
                        //         // depth 3
                        //         if (s3.name == "enable")
                        //         {
                        //             c.enable = s3.value;
                        //         }
                        //         else if (s3.name == "data_in")
                        //         {
                        //             c.data_in = s3.value;
                        //         }
                        //         else if (s3.name == "clear")
                        //         {
                        //             c.reset = s3.value;
                        //         }
                        //         else if (s3.name == "preset")
                        //         {
                        //             c.set = s3.value;
                        //         }
                        //         else if (s3.name == "clear_preset_var1")
                        //         {
                        //             c.clear_preset_var1 = s3.value;
                        //         }
                        //         else if (s3.name == "clear_preset_var2")
                        //         {
                        //             c.clear_preset_var2 = s3.value;
                        //         }
                        //     }
                        // }
                    }

                    lib->add_gate_type(gt);
                }
            }
        }
        else
        {
            log_error("netlist", "gate library does not start with 'library' node.");
            return nullptr;
        }

        return lib;
    }

    // ###########################################################################
    // ######          Build gate library from intermediate format         #######
    // ###########################################################################

    // std::shared_ptr<gate_library> get_gate_library(std::shared_ptr<intermediate_library> inter_lib)
    // {
    //     std::shared_ptr<gate_library> lib = std::make_shared<gate_library>(inter_lib->name);

    //     std::vector<std::string> input_pins;
    //     std::vector<std::string> output_pins;

    //     for (const auto& cell : inter_lib->cells)
    //     {
    //         gate_type gt(cell.name);

    //         for (const auto& pin : cell.pins)
    //         {
    //             if (pin.direction == "input")
    //             {
    //                 input_pins.push_back(pin.name);
    //             }
    //             else if (pin.direction == "output")
    //             {
    //                 output_pins.push_back(pin.name);
    //             }

    //             if (!pin.function.empty())
    //             {
    //                 if (pin.function == cell.output_state.first)
    //                 {
    //                     gt.add_boolean_function(pin.name, boolean_function::from_string(cell.data_in));
    //                 }
    //                 else if (pin.function == cell.output_state.second)
    //                 {
    //                     gt.add_boolean_function(pin.name, !(boolean_function::from_string(cell.data_in)));
    //                 }
    //                 else
    //                 {
    //                     gt.add_boolean_function(pin.name, boolean_function::from_string(pin.function));
    //                 }
    //             }
    //         }

    //         if (cell.is_ff)
    //         {
    //             gt.set_base_type(gate_type::ff);

    //             // TODO special functions
    //         }
    //         else if (cell.is_latch)
    //         {
    //             gt.set_base_type(gate_type::latch);

    //             // TODO special functions
    //         }
    //         else if (cell.is_lut)
    //         {
    //             gt.set_base_type(gate_type::lut);

    //             // TODO LUT stuff
    //         }
    //         else
    //         {
    //             gt.set_base_type(gate_type::combinatorial);
    //         }

    //         gt.add_input_pins(input_pins);
    //         gt.add_output_pins(output_pins);
    //     }

    //     return lib;
    // }

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
}    // namespace gate_library_liberty_parser
