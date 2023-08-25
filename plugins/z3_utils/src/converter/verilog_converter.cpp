#include "converter/verilog_converter.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    namespace z3_utils
    {
        void VerilogConverter::set_control_mapping(const std::map<std::string, bool>& control_mapping)
        {
            m_control_mapping = control_mapping;
        }

        std::string VerilogConverter::build_operand(const std::string& operand) const
        {
            if (operand.find("?") != std::string::npos)
            {
                return operand.substr(1);
            }

            return operand;
        }

        std::string VerilogConverter::build_operation(const Converter::Operation& operation, const std::vector<std::string>& operands) const
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
                    op = "~" + build_operand(operands.front());
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
                    log_error("verifiaction", "unkown operation for operation building ({})", operation_to_string(operation));
                    return "NOT IMPLEMENTED REACHED";
            }

            op = '(' + op + ')';
            return op;
        }

        std::string VerilogConverter::generate_assignment(const std::string& l) const
        {
            // stores variable the is being declared
            std::string lhs = extract_lhs(l);

            auto sub_expressions_start  = l.find(lhs) + lhs.size() + 1;            // hardcoded + 1 for the space between (just for my ocd)
            auto sub_expressions_size   = l.size() - sub_expressions_start - 2;    // hardcoded -2 for the last two closing brackest (just for my ocd)
            auto sub_expressions_string = l.substr(sub_expressions_start, sub_expressions_size);

            auto sub_expressions = extract_sub_exrepssions(sub_expressions_string);

            log_debug("z3_utils", "Found {} sub_expressions.", sub_expressions.size());

            auto translated_sub_expressions = translate_sub_expressions(sub_expressions);

            log_debug("z3_utils", "Translated {} sub_expressions.", translated_sub_expressions.size());

            auto rhs = merge_sub_expressions(translated_sub_expressions);

            return "\twire " + lhs + ";\n" + "\tassign " + lhs + " = " + rhs + ";\n";
        }

        std::string VerilogConverter::generate_initialization(const std::vector<std::string>& input_vars) const
        {
            std::string init = "module func ( ";
            int counter      = 0;
            for (const auto& in : input_vars)
            {
                counter++;
                if (m_control_mapping.find(in) == m_control_mapping.end())
                {
                    init = init + in + ", ";
                }
            }
            init += "out );\n";

            for (const auto& in : input_vars)
            {
                if (m_control_mapping.find(in) == m_control_mapping.end())
                {
                    init += "\tinput " + in + ";\n";
                }
            }

            init += "\toutput out;\n";

            init += "\n\n";

            for (const auto& [control_var, value] : m_control_mapping)
            {
                init += "\twire " + control_var + ";\n";
                init += "\tassign " + control_var + " = " + std::to_string(value) + "; // control bit\n";
            }

            init += "\n\n";
            return init;
        }

        std::string VerilogConverter::construct_function(const std::string& assignments, const std::string& initialization, const std::vector<std::string>& input_vars) const
        {
            // NOTE the inputs are unused because they are included in the initialization for the verilog converter
            UNUSED(input_vars);
            std::string return_var;
            if (assignments.empty())
            {
                const auto begin = initialization.find("i");
                const auto end   = initialization.find(",");
                return_var       = initialization.substr(begin, end - begin);
            }
            else
            {
                const auto last_assigment_pos = assignments.rfind("assign") + 7;    // length of "assign" + one space
                const auto last_line          = assignments.substr(last_assigment_pos);
                return_var                    = extract_lhs(last_line);
            }

            std::string final_words = "\tassign out = " + return_var + ";\n";
            final_words += "endmodule";
            return (initialization + assignments + final_words);
        }
    }    // namespace z3_utils
}    // namespace hal
