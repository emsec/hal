#include "hal_core/netlist/boolean_function/translator.h"

#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

namespace hal
{
    namespace SMT
    {
        namespace Translator
        {
            /**
	 * Helper function to reduce a view into the abstract syntax tree of a node
	 * and its parameter leaf nodes to an SMT-LIB string.
	 *
	 * @param[in] node - Boolean function node.
	 * @param[in] p - Boolean function node parameters.
	 * @returns OK() and SMT-LIB v2 string representation of node and operands on success, Err() otherwise.
	 */
            Result<std::string> reduce_to_smt2(const BooleanFunction::Node& node, std::vector<std::string>&& p)
            {
                if (node.get_arity() != p.size())
                {
                    return ERR("could not reduce into SMT-Lib v2 string: arity of node '" + node.to_string() + "' does not match number of parameters");
                }

                switch (node.type)
                {
                    case BooleanFunction::NodeType::Constant: {
                        if (auto [ok, str] = utils::translate_to<u64>(node.to_string()); ok)
                        {
                            return OK("(_ bv" + std::to_string(str) + " " + std::to_string(node.size) + ")");
                        }
                        return ERR("could not reduce into SMT-Lib v2 string: unable to translate constant '" + node.to_string() + "'");
                    }
                    case BooleanFunction::NodeType::Index:
                        return OK(std::to_string(node.index));
                    case BooleanFunction::NodeType::Variable:
                        return OK(node.variable);

                    case BooleanFunction::NodeType::And:
                        return OK("(bvand " + p[0] + " " + p[1] + ")");
                    case BooleanFunction::NodeType::Or:
                        return OK("(bvor " + p[0] + " " + p[1] + ")");
                    case BooleanFunction::NodeType::Not:
                        return OK("(bvnot " + p[0] + ")");
                    case BooleanFunction::NodeType::Xor:
                        return OK("(bvxor " + p[0] + " " + p[1] + ")");

                    case BooleanFunction::NodeType::Add:
                        return OK("(bvadd " + p[0] + " " + p[1] + ")");
                    case BooleanFunction::NodeType::Sub:
                        return OK("(bvsub " + p[0] + " " + p[1] + ")");

                    case BooleanFunction::NodeType::Concat:
                        return OK("(concat " + p[0] + " " + p[1] + ")");

                    default:
                        return ERR("could not reduce into SMT-Lib v2 string: not implemented for given node type");
                }
            }

            Result<std::string> translate_to_smt2(const BooleanFunction& function)
            {
                std::vector<std::string> stack;
                for (const auto& node : function.get_nodes())
                {
                    std::vector<std::string> operands;
                    std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
                    stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

                    if (auto reduction = reduce_to_smt2(node, std::move(operands)); reduction.is_ok())
                    {
                        stack.emplace_back(reduction.get());
                    }
                    else
                    {
                        return ERR_APPEND(reduction.get_error(), "could not translate Boolean function to SMT-Lib v2 string: reduction to SMT-Lib v2 string failed");
                    }
                }

                switch (stack.size())
                {
                    case 1:
                        return OK(stack.back());
                    default:
                        return ERR("could not translate Boolean function to SMT-Lib v2 string: stack is imbalanced");
                }
            }
        }    // namespace Translator
    }        // namespace SMT
}    // namespace hal
