#include "converter/cpp_converter.h"

#include "hal_core/utilities/log.h"

#include <algorithm>

namespace hal
{
    namespace z3_utils
    {
        std::string Cpp_Converter::build_operand(const std::string& operand) const
        {
            if (operand.find("?") != std::string::npos)
            {
                return operand.substr(1);
            }
            if (operand.find("|") != std::string::npos)
            {
                auto operand_name = operand.substr(1, operand.size() - 2);
                return "values[" + operand_name + "]";
            }
            if (operand.find("sub_") != std::string::npos)
            {
                return operand;
            }

            log_error("z3_utils", "unkown operand format for {}", operand);
            return "NOT IMPLEMENTED REACHED";
        }

        std::string Cpp_Converter::build_operation(const Converter::Operation& operation, const std::vector<std::string>& operands) const
        {
            std::string op;

            switch (operation)
            {
                case Converter::bvand:
                    for (const auto& o : operands)
                    {
                        op += (build_operand(o) + " & ");
                    }
                    op.erase(op.size() - 3);
                    break;
                case Converter::bvnot:
                    if (operands.size() > 1)
                    {
                        log_error("z3_utils", "unhandled amount of operands for operation");
                        return "NOT IMPLEMENTED REACHED";
                    }
                    op = "!" + build_operand(operands.front());
                    break;
                case Converter::bvor:
                    for (const auto& o : operands)
                    {
                        op += (build_operand(o) + " | ");
                    }
                    op.erase(op.size() - 3);
                    break;
                case Converter::bvxor:
                    for (const auto& o : operands)
                    {
                        op += (build_operand(o) + " ^ ");
                    }
                    op.erase(op.size() - 3);
                    break;
                default:
                    log_error("z3_utils", "unkown operation for operation building ({})", operation);
                    return "NOT IMPLEMENTED REACHED";
            }

            op = '(' + op + ')';
            return op;
        }

        std::string Cpp_Converter::generate_assignment(const std::string& l) const
        {
            // stores variable the is being declared
            std::string lhs = extract_lhs(l);

            auto sub_expressions_start  = l.find(lhs) + lhs.size() + 1;            // hardcoded + 1 for the space between (just for my ocd)
            auto sub_expressions_size   = l.size() - sub_expressions_start - 2;    // hardcoded -2 for the last two closing brackest (just for my ocd)
            auto sub_expressions_string = l.substr(sub_expressions_start, sub_expressions_size);

            auto sub_expressions            = extract_sub_exrepssions(sub_expressions_string);
            auto translated_sub_expressions = translate_sub_expressions(sub_expressions);

            auto rhs = merge_sub_expressions(translated_sub_expressions);

            return "bool " + lhs + " = " + rhs + ";\n";
        }

        std::string Cpp_Converter::generate_initialization(const std::vector<u32>& inputs) const
        {
            std::string init = "";
            for (const auto& in : inputs)
            {
                init = init + std::to_string(in) + ",\n";
            }

            return init;
        }

        std::string Cpp_Converter::construct_function(const std::string& assignments, const std::string& initalization, const std::vector<u32>& inputs) const
        {
            std::string func = m_function_corpus;

            const auto max_input = *std::max_element(inputs.begin(), inputs.end());


            // extract last return val
            const auto last_assigment_pos = assignments.rfind("bool") + 5;    // length of "bool" + one space
            const auto last_line          = assignments.substr(last_assigment_pos);


            const auto return_var = extract_lhs(last_line);

            func = replace_all(func, "<INIT>", initalization);
            func = replace_all(func, "<ASSIGNMENTS>", assignments);
            func = replace_all(func, "<RETURN>", return_var);
            func = replace_all(func, "<INPUT_SIZE>", std::to_string(max_input + 1));


            return func;
        }
    }    // namespace z3_utils
}    // namespace hal
