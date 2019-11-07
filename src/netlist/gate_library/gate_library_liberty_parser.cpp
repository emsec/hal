#include "netlist/gate_library/gate_library_liberty_parser.h"
#include "core/log.h"
#include "core/utils.h"
#include "netlist/boolean_function.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_type.h"
#include "netlist/gate_library/gate_type_ff.h"
#include "netlist/gate_library/gate_type_latch.h"
#include "netlist/gate_library/gate_type_lut.h"

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
                for (auto y : x.second)
                {
                    delete y;
                }
            }
        }

        statement* parent;
        bool is_group;
        std::string name;
        std::string value;
        std::map<std::string, std::vector<statement*>> statements;
    };

    struct sequential
    {
        std::pair<std::string, std::string> output_state;
        std::map<std::string, boolean_function> functions;
        std::pair<char, char> set_rst;
    };

    statement* get_statements(std::stringstream& ss);
    std::shared_ptr<gate_library> get_gate_library(statement* root);
    void remove_comments(std::string& line, bool& multi_line_comment);

    // ###########################################################################
    // #########          Parse liberty into intermediate format          ########
    // ###########################################################################

    std::shared_ptr<gate_library> parse(std::stringstream& ss)
    {
        auto statements = get_statements(ss);
        auto lib        = get_gate_library(statements);
        delete statements;
        return lib;
    }

    statement* get_statements(std::stringstream& ss)
    {
        statement* current_group = nullptr;
        statement* root          = nullptr;

        std::set<std::string> groups_of_interest     = {"library", "cell", "pin", "ff", "latch", "lut"};
        std::set<std::string> attributes_of_interest = {"direction",
                                                        "function",
                                                        "x_function",
                                                        "next_state",
                                                        "clocked_on",
                                                        "clear",
                                                        "preset",
                                                        "clear_preset_var1",
                                                        "clear_preset_var2",
                                                        "data_in",
                                                        "enable",
                                                        "data_category",
                                                        "data_identifier",
                                                        "bit_order"};

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
                        current_group = current_group->parent;
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
                    auto new_group = new statement(current_group, true, group_name, name);
                    if (root != nullptr)
                    {
                        current_group->statements[group_name].push_back(new_group);
                    }
                    else
                    {
                        root = new_group;
                    }

                    current_group = new_group;
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
                            current_group->statements[name].push_back(new statement(current_group, false, name, value));
                        }
                    }
                }
            }

            for (i32 i = 0; i < std::count(line.begin(), line.end(), '}'); i++)
            {
                // move back in tree
                current_group = current_group->parent;
            }
        }

        return root;
    }

    static std::string prepare_string(std::string str)
    {
        return str.substr(0, str.rfind("\"")).substr(str.find("\"") + 1);
    }

    bool parse_pins(const std::vector<statement*>& statements, std::shared_ptr<gate_type>& gt, std::pair<std::string, std::string>& internal_state_names)
    {
        for (const auto& pin : statements)
        {
            for (const auto& s : pin->statements)
            {
                if (s.first == "direction")
                {
                    if (s.second[0]->value == "input")
                    {
                        gt->add_input_pin(pin->value);
                    }
                    else if (s.second[0]->value == "output")
                    {
                        gt->add_output_pin(pin->value);
                    }
                }

                if (s.first == "function")
                {
                    if (gt->get_base_type() == gate_type::base_type::combinatorial)
                    {
                        gt->add_boolean_function(pin->value, boolean_function::from_string(prepare_string(s.second[0]->value)));
                    }
                    else if (gt->get_base_type() == gate_type::base_type::ff)
                    {
                        auto ff_ptr = std::dynamic_pointer_cast<gate_type_ff>(gt);

                        if (s.second[0]->value == internal_state_names.second)
                        {
                            ff_ptr->set_output_pin_inverted(pin->value, true);
                        }
                    }
                    else if (gt->get_base_type() == gate_type::base_type::latch)
                    {
                        auto latch_ptr = std::dynamic_pointer_cast<gate_type_latch>(gt);

                        if (s.second[0]->value == internal_state_names.second)
                        {
                            latch_ptr->set_output_pin_inverted(pin->value, true);
                        }
                    }
                }
                else if (s.first == "x_function")
                {
                    gt->add_boolean_function(pin->value + "_undefined", boolean_function::from_string(prepare_string(s.second[0]->value)));
                }
            }
        }

        return true;
    }

    std::pair<std::string, std::string> parse_ff_block(statement* block, std::shared_ptr<gate_type_ff> gt)
    {
        for (const auto& s : block->statements)
        {
            if (s.first == "clocked_on")
            {
                gt->set_clock_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "next_state")
            {
                gt->set_next_state_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "clear")
            {
                gt->set_reset_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "preset")
            {
                gt->set_set_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "clear_preset_var1")
            {
                switch (prepare_string(s.second[0]->value).at(0))
                {
                    case 'L':
                        gt->set_special_behavior1(gate_type_ff::special_behavior::L);
                        break;
                    case 'H':
                        gt->set_special_behavior1(gate_type_ff::special_behavior::H);
                        break;
                    case 'N':
                        gt->set_special_behavior1(gate_type_ff::special_behavior::N);
                        break;
                    case 'T':
                        gt->set_special_behavior1(gate_type_ff::special_behavior::T);
                        break;
                    case 'X':
                        gt->set_special_behavior1(gate_type_ff::special_behavior::X);
                        break;
                }
            }
            else if (s.first == "clear_preset_var2")
            {
                switch (prepare_string(s.second[0]->value).at(0))
                {
                    case 'L':
                        gt->set_special_behavior2(gate_type_ff::special_behavior::L);
                        break;
                    case 'H':
                        gt->set_special_behavior2(gate_type_ff::special_behavior::H);
                        break;
                    case 'N':
                        gt->set_special_behavior2(gate_type_ff::special_behavior::N);
                        break;
                    case 'T':
                        gt->set_special_behavior2(gate_type_ff::special_behavior::T);
                        break;
                    case 'X':
                        gt->set_special_behavior2(gate_type_ff::special_behavior::X);
                        break;
                }
            }
        }

        auto tokens = core_utils::split(block->value, ',');
        return {prepare_string(tokens[0]), prepare_string(tokens[1])};
    }

    std::pair<std::string, std::string> parse_latch_block(statement* block, std::shared_ptr<gate_type_latch> gt)
    {
        for (const auto& s : block->statements)
        {
            if (s.first == "enable")
            {
                gt->set_enable_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "data_in")
            {
                gt->set_data_in_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "clear")
            {
                gt->set_reset_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "preset")
            {
                gt->set_set_function(boolean_function::from_string(prepare_string(s.second[0]->value)));
            }
            else if (s.first == "clear_preset_var1")
            {
                switch (prepare_string(s.second[0]->value).at(0))
                {
                    case 'L':
                        gt->set_special_behavior1(gate_type_latch::special_behavior::L);
                        break;
                    case 'H':
                        gt->set_special_behavior1(gate_type_latch::special_behavior::H);
                        break;
                    case 'N':
                        gt->set_special_behavior1(gate_type_latch::special_behavior::N);
                        break;
                    case 'T':
                        gt->set_special_behavior1(gate_type_latch::special_behavior::T);
                        break;
                    case 'X':
                        gt->set_special_behavior1(gate_type_latch::special_behavior::X);
                        break;
                }
            }
            else if (s.first == "clear_preset_var2")
            {
                switch (prepare_string(s.second[0]->value).at(0))
                {
                    case 'L':
                        gt->set_special_behavior2(gate_type_latch::special_behavior::L);
                        break;
                    case 'H':
                        gt->set_special_behavior2(gate_type_latch::special_behavior::H);
                        break;
                    case 'N':
                        gt->set_special_behavior2(gate_type_latch::special_behavior::N);
                        break;
                    case 'T':
                        gt->set_special_behavior2(gate_type_latch::special_behavior::T);
                        break;
                    case 'X':
                        gt->set_special_behavior2(gate_type_latch::special_behavior::X);
                        break;
                }
            }
        }

        auto tokens = core_utils::split(block->value, ',');
        return {prepare_string(tokens[0]), prepare_string(tokens[1])};
    }

    void parse_lut_block(statement* block, std::shared_ptr<gate_type_lut> gt)
    {
        for (const auto& s : block->statements)
        {
            if (s.first == "data_category")
            {
                gt->set_data_category(prepare_string(s.second[0]->value));
            }
            else if (s.first == "data_identifier")
            {
                gt->set_data_identifier(prepare_string(s.second[0]->value));
            }
            else if (s.first == "bit_order")
            {
                if (prepare_string(s.second[0]->value) == "ascending")
                {
                    gt->set_data_ascending_order(true);
                }
                else
                {
                    gt->set_data_ascending_order(false);
                }
            }
        }
    }

    bool parse_cells(const std::vector<statement*>& statements, std::shared_ptr<gate_library>& lib)
    {
        for (const auto& cell : statements)
        {
            std::shared_ptr<gate_type> gt;
            std::pair<std::string, std::string> internal_state_names;
            if (cell->statements.find("ff") != cell->statements.end())
            {
                auto ff_type         = std::make_shared<gate_type_ff>(cell->value);
                internal_state_names = parse_ff_block(cell->statements["ff"][0], ff_type);
                gt                   = ff_type;
            }
            else if (cell->statements.find("latch") != cell->statements.end())
            {
                auto latch_type      = std::make_shared<gate_type_latch>(cell->value);
                internal_state_names = parse_latch_block(cell->statements["latch"][0], latch_type);
                gt                   = latch_type;
            }
            else if (cell->statements.find("lut") != cell->statements.end())
            {
                auto lut_type = std::make_shared<gate_type_lut>(cell->value);
                parse_lut_block(cell->statements["lut"][0], lut_type);
                gt = lut_type;
            }
            else
            {
                gt = std::make_shared<gate_type>(cell->value);
            }

            if (!parse_pins(cell->statements["pin"], gt, internal_state_names))
            {
                log_error("netlist", "error while parsing pin definitions for cell '{}'", gt->get_name());
                return false;
            }

            lib->add_gate_type(gt);
        }

        return true;
    }

    std::shared_ptr<gate_library> get_gate_library(statement* root)
    {
        if (root->name != "library")
        {
            log_error("netlist", "gate library does not start with 'library' node.");
            return nullptr;
        }

        auto lib = std::make_shared<gate_library>(root->value);

        if (!parse_cells(root->statements["cell"], lib))
        {
            log_error("netlist", "error while parsing cell definitions");
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
