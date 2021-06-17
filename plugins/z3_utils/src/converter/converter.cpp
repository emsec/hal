#include "converter/converter.h"

#include "hal_core/utilities/log.h"

#include <iomanip>
#include <queue>
#include <set>
#include <sstream>

namespace hal
{
    namespace z3_utils
    {
        // UTILS
        std::string Converter::integer_with_leading_zeros(const u32 i, const u32 total_length) const {
            std::stringstream ss;
            ss << std::setw(total_length) << std::setfill('0') << i;
            return ss.str();
        }

        std::string Converter::replace_all(const std::string& str, const std::string& from, const std::string& to) const {
            std::string r_str = str;
            size_t start_index = 0;
            while (true) {
                size_t found_index = r_str.find(from, start_index);
                if (found_index == std::string::npos) {
                    break;
                }

                r_str = r_str.replace(found_index, from.size(), to);

                start_index = found_index + to.size();
            }

            return r_str;
        }

        std::string Converter::operation_to_string(const Converter::Operation& op) const {
            std::string op_str;
            
            switch (op) {
                case Converter::bvand:
                    op_str = "bvand";
                    break;
                case Converter::bvnot:
                    op_str = "bvnot";
                    break;
                case Converter::bvor:
                    op_str = "bvor";
                    break;
                case Converter::bvxor:
                    op_str = "bvxor";
                    break;
                default:
                    log_error("z3_utils", "Tried to convert unkown operation to string.");
                    op_str = "NOT IMPLEMENTED REACHED";
            }

            return op_str;
        }

        // METHODS
        bool Converter::does_line_contain_assignment(const std::string& l) const {
            return l.find("let") != std::string::npos;
        }

        std::string Converter::extract_lhs(const std::string& l) const {
            auto start_index = l.find_first_of('x');
            auto end_index = l.find_first_of(' ', start_index);

            return l.substr(start_index, end_index - start_index);
        }

        std::map<u32, u32> Converter::extract_paranthesis_pairs(const std::string& l) const {
            std::map<u32, u32> pairs;
            auto start_index = 0;
            while (true) {
                auto found_index = l.find_first_of('(', start_index);
                if (found_index == std::string::npos) {
                    break;
                }
                
                // search for closing bracket after finding an open open one
                u32 count = 1;
                for (u32 i = found_index + 1; i < l.size(); i++) {
                    if (l.at(i) == '(') {
                        count++;
                    } else if (l.at(i) == ')') {
                        count--;
                    }

                    if (count == 0) {
                        pairs.insert({found_index, i});
                        break;
                    }
                }

                start_index = found_index + 1;
                log_debug("z3_utils", "Trying to find paranthesis_pairs.");
            }

            return pairs;
        }
    
        bool Converter::contains_one_operation(const std::string& l) const {
            u32 count = 0;
            
            for (const auto& op : m_operations) {
                size_t start_index = 0;
                auto op_str = operation_to_string(op);
                while (true) {
                    size_t found_index = l.find(op_str, start_index);
                    if (found_index == std::string::npos) {
                        break;
                    }
                    count++;

                    start_index = found_index + op_str.size();
                    log_debug("z3_utils", "{}", op_str);
                }
            }

            return count == 1;
        }

        std::map<std::string, std::string> Converter::extract_sub_exrepssions(const std::string& l) const {
            // stores the start and end position of each pair of parenthesis
            std::map<u32, u32> parentheses = extract_paranthesis_pairs(l);
            // stores the sub expressions with a name in the form of "sub_<count>"
            std::map<std::string, std::string> sub_expressions;
            u32 count = 0;
            for (const auto& [start, end]: parentheses) {
                auto sub_expression = l.substr(start, end - start + 1);
                std::string sub_name = "sub_" + integer_with_leading_zeros(count, 5);
                sub_expressions.insert({sub_name, sub_expression});
                count++;
            }

            // eliminate duplicates:
            std::set<std::string> to_erase;
            for (const auto& [dom_name, dom_expr] : sub_expressions) {
                for (const auto& [sub_name, sub_expr] : sub_expressions) {
                    if (dom_expr == sub_expr && dom_name != sub_name && to_erase.find(dom_name) == to_erase.end()) {
                        to_erase.insert(sub_name);
                    }
                }
            }
            for (const auto& n : to_erase) {
                sub_expressions.erase(n);
            }

            // push names to waiting q
            std::queue<std::string> q;
            for (const auto& [n, e] : sub_expressions) {
                q.push(n);
            }

            // replace all sub expressions in higher order expressions with their name
            while (!q.empty()) {
                auto f_name = q.front();
                auto f_expr = sub_expressions.at(f_name);
                q.pop();

                if (contains_one_operation(f_expr)) {
                    for (auto [n, e] : sub_expressions) {
                        if (f_expr == e) {
                            continue;
                        }

                        sub_expressions.at(n) = replace_all(e, f_expr, f_name);
                    }
                } else {
                    q.push(f_name);
                }

                if (!q.empty()) {
                    log_debug("z3_utils", "Queue: {} at front {}.", q.size(), q.front());
                    log_debug("z3_utils", "Sub: {}", sub_expressions.at(q.front()));
                }
            }

            return sub_expressions;
        }

        Converter::Operation Converter::extract_operation(const std::string& se) const {
            for (const auto& op : m_operations) {
                auto op_str = operation_to_string(op);
                if (se.find(op_str) != std::string::npos) {
                    return op;
                }
            }

            log_error("z3_utils", "Unkown operation in subexpression {}", se);
            return Converter::NONE;
        }

        std::vector<std::string> Converter::extract_operands(const std::string& se) const {
            // remove parentheses
            std::string shortend_string = se.substr(1);
            shortend_string.pop_back();

            std::vector<std::string> operands;

            std::istringstream iss(shortend_string);
            std::string tmp;
            while (getline(iss, tmp, ' ')) {
                operands.push_back(tmp);
            }

            // remove first element, since this is the operation and not an operand
            operands.erase(operands.begin());

            return operands;
        }
        
        std::map<std::string, std::string> Converter::translate_sub_expressions(const std::map<std::string, std::string>& se) const {
            std::map<std::string, std::string> translated_sub_expressions;

            for (const auto& [n, s] : se) {
                Converter::Operation operation = extract_operation(s);
                std::vector<std::string> operands = extract_operands(s);
    
                translated_sub_expressions.insert({n, build_operation(operation, operands)});
            }
            
            return translated_sub_expressions;
        }

        std::string Converter::merge_sub_expressions(const std::map<std::string, std::string>& translated_sub_expressions) const {
            std::string assignment = translated_sub_expressions.at("sub_00000"); // first element contains main expression 

            // replace the sub_<id> placeholders in the sub expressions
            std::map<std::string, std::string> merged_sub_expressions = translated_sub_expressions;
            for (const auto& [n, e] : merged_sub_expressions) {
                for (auto [m, f] : merged_sub_expressions) {
                    merged_sub_expressions.at(m) = replace_all(f, n, e);
                }
            }

            // replace in assignment
            for (const auto& [n, e] : merged_sub_expressions) {
                assignment = replace_all(assignment, n, e);
            }

            return assignment;
        }
        
        std::string Converter::convert_z3_expr_to_func(const z3Wrapper& e) const {
            std::string assignments = "";

            // TODO remove this is only for debugging
            // auto test_ctx = std::make_unique<z3::context>();
            // z3::expr t_1 = test_ctx->bv_const("1001", 1);
            // z3::expr t_2 = {*test_ctx, Z3_mk_bvnot(*test_ctx, t_1)};
            // z3::expr t_3 = {*test_ctx, Z3_mk_bvnot(*test_ctx, t_2)};
            // auto wrap_t3 = z3Wrapper(std::move(test_ctx), t_3);

            auto smt = e.get_smt2_string();


            std::istringstream iss(smt);
            for (std::string line; std::getline(iss, line); ) {
                if (does_line_contain_assignment(line)) {
                    auto assignment = generate_assignment(line);
                    assignments = assignments + assignment;;
                }
            }

            log_debug("z3_utils", "found {} assignments.", assignments.size());

            if (assignments.empty()) {
                // in order to stay compliant with the rest of the converter structure we simply simulate a dummy assignment with a double negation.
                auto dummy_assignment = "(let ((?x1 (bvnot (bvnot " + e.get_expr().to_string() + ")))))";
                assignments += generate_assignment(dummy_assignment);
            }

            const auto input_ids = e.get_inputs_net_ids();

            std::string initialization = generate_initialization(input_ids);

            return construct_function(assignments, initialization, input_ids);
        }
    }  //namespace z3_utils
}   // namespace hal