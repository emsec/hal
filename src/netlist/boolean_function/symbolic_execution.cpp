#include "hal_core/netlist/boolean_function/symbolic_execution.h"

#include "hal_core/utilities/log.h"

// local macro to ease error message return handling
#define ERROR(MESSAGE)       \
    std::stringstream error; \
    error << MESSAGE;        \
    return error.str()

namespace hal
{
    namespace SMT
    {
        namespace ConstantPropagation
        {
            /**
	 * Helper function to simplify a constant AND operation.
	 * 
	 * @param[in] p0 - Boolean function parameter 0.
	 * @param[in] p1 - Boolean function parameter 1.
	 * @returns Boolean function with a simplified constant value.
	 */
            BooleanFunction And(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                std::vector<BooleanFunction::Value> simplified;
                simplified.reserve(p0.size());
                for (auto i = 0u; i < p0.size(); i++)
                {
                    if ((p0[i] == 0) || (p1[i] == 0))
                    {
                        simplified.emplace_back(BooleanFunction::Value::ZERO);
                    }
                    else if ((p0[i] == 1) && (p1[i] == 1))
                    {
                        simplified.emplace_back(BooleanFunction::Value::ONE);
                    }
                    else
                    {
                        simplified.emplace_back(BooleanFunction::Value::X);
                    }
                }
                return BooleanFunction::Const(simplified);
            }

            /**
	 * Helper function to simplify a constant OR operation.
	 * 
	 * @param[in] p0 - Boolean function parameter 0.
	 * @param[in] p1 - Boolean function parameter 1.
	 * @returns Boolean function with a simplified constant value.
	 */
            BooleanFunction Or(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                std::vector<BooleanFunction::Value> simplified;
                simplified.reserve(p0.size());
                for (auto i = 0u; i < p0.size(); i++)
                {
                    if ((p0[i] == 0) && (p1[i] == 0))
                    {
                        simplified.emplace_back(BooleanFunction::Value::ZERO);
                    }
                    else if ((p0[i] == 1) || (p1[i] == 1))
                    {
                        simplified.emplace_back(BooleanFunction::Value::ONE);
                    }
                    else
                    {
                        simplified.emplace_back(BooleanFunction::Value::X);
                    }
                }
                return BooleanFunction::Const(simplified);
            }

            /**
	 * Helper function to simplify a constant NOT operation.
	 * 
	 * @param[in] p - Boolean function parameter.
	 * @returns Boolean function with a simplified constant value.
	 */
            BooleanFunction Not(const std::vector<BooleanFunction::Value>& p)
            {
                std::vector<BooleanFunction::Value> simplified;
                simplified.reserve(p.size());
                for (const auto& value : p)
                {
                    if (value == BooleanFunction::Value::ZERO)
                    {
                        simplified.emplace_back(BooleanFunction::Value::ONE);
                    }
                    else if (value == BooleanFunction::Value::ONE)
                    {
                        simplified.emplace_back(BooleanFunction::Value::ZERO);
                    }
                    else
                    {
                        simplified.emplace_back(value);
                    }
                }
                return BooleanFunction::Const(simplified);
            }

            /**
	 * Helper function to simplify a constant XOR operation.
	 * 
	 * @param[in] p0 - Boolean function parameter 0.
	 * @param[in] p1 - Boolean function parameter 1.
	 * @returns Boolean function with a simplified constant value.
	 */
            BooleanFunction Xor(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                std::vector<BooleanFunction::Value> simplified;
                simplified.reserve(p0.size());
                for (auto i = 0u; i < p0.size(); i++)
                {
                    if (((p0[i] == 0) && (p1[i] == 1)) || ((p0[i] == 1) && (p1[i] == 0)))
                    {
                        simplified.emplace_back(BooleanFunction::Value::ONE);
                    }
                    else if (((p0[i] == 0) && (p1[i] == 0)) || ((p0[i] == 1) && (p1[i] == 1)))
                    {
                        simplified.emplace_back(BooleanFunction::Value::ZERO);
                    }
                    else
                    {
                        simplified.emplace_back(BooleanFunction::Value::X);
                    }
                }
                return BooleanFunction::Const(simplified);
            }

        }    // namespace ConstantPropagation

        namespace
        {
            /**
	 * Helper function to generate an n-bit vector of all 1s.
	 * 
	 * @param[in] size - Bit-size of vector.
	 * @returns Boolean function with all 1s constant.
	 */
            BooleanFunction One(u16 size)
            {
                return BooleanFunction::Const(std::vector<BooleanFunction::Value>(size, BooleanFunction::Value::ONE));
            }
        }    // namespace

        SymbolicExecution::SymbolicExecution(const std::vector<BooleanFunction>& variables) : state(SymbolicState(variables))
        {
        }

        std::variant<BooleanFunction, std::string> SymbolicExecution::evaluate(const BooleanFunction& function) const
        {
            std::vector<BooleanFunction> stack;
            for (const auto& node : function.get_nodes())
            {
                std::vector<BooleanFunction> parameters;
                std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(parameters));
                stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

                if (auto simplified = this->simplify(node, std::move(parameters)); std::get_if<BooleanFunction>(&simplified) != nullptr)
                {
                    stack.emplace_back(std::get<BooleanFunction>(simplified));
                }
                else
                {
                    ERROR("Cannot simplify '" << function.to_string() << "' (= " << std::get<std::string>(simplified) << ").");
                }
            }

            switch (stack.size())
            {
                case 1:
                    return stack.back();
                default:
                    ERROR("Cannot simplify '" << function.to_string() << "' (= imbalanced stack with " << stack.size() << " remaining parts).");
            }
        }

        std::optional<std::string> SymbolicExecution::evaluate(const Constraint& constraint)
        {
            if (auto rhs = this->evaluate(constraint.rhs); std::get_if<BooleanFunction>(&rhs) != nullptr)
            {
                this->state.set(constraint.lhs.clone(), std::get<BooleanFunction>(rhs).clone());
                return std::nullopt;
            }
            else
            {
                return std::get<std::string>(rhs);
            }
        }

        std::vector<BooleanFunction> SymbolicExecution::normalize(std::vector<BooleanFunction>&& p)
        {
            if (p.size() <= 1ul)
            {
                return std::move(p);
            }

            std::sort(p.begin(), p.end(), [](const auto& lhs, const auto& rhs) {
                if (lhs.get_top_level_node().type == rhs.get_top_level_node().type)
                {
                    return lhs < rhs;
                }
                return rhs.is_constant();
            });
            return std::move(p);
        }

        std::variant<BooleanFunction, std::string> SymbolicExecution::simplify(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p) const
        {
            if (!p.empty() && std::all_of(p.begin(), p.end(), [](const auto& function) { return function.is_constant(); }))
            {
                return SymbolicExecution::constant_propagation(node, std::move(p));
            }

            if (node.is_commutative())
            {
                p = SymbolicExecution::normalize(std::move(p));
            }

            /// # Developer Note
            /// Since the simplify() function vists the abstract syntax tree of the
            /// Boolean function, we want to prevent the use of any recursive call
            /// to the simplify() function of a sub-expression tree. Hence, use the
            /// simplify() function with care, as otherwise run-time may explode :)

            switch (node.type)
            {
                case BooleanFunction::NodeType::Constant: {
                    return BooleanFunction::Const(node.constant);
                }
                case BooleanFunction::NodeType::Index: {
                    return BooleanFunction::Index(node.index, node.size);
                }
                case BooleanFunction::NodeType::Variable: {
                    return this->state.get(BooleanFunction::Var(node.variable, node.size));
                }
                case BooleanFunction::NodeType::And: {
                    // X & 0   =>   0
                    if (p[1].has_constant_value(0))
                    {
                        return BooleanFunction::Const(0, node.size);
                    }
                    // X & 1  =>   X
                    if (p[1] == One(node.size))
                    {
                        return p[0];
                    }
                    // X & X   =>   X
                    if (p[0] == p[1])
                    {
                        return p[0];
                    }
                    // X & ~X   =>   0
                    if (~p[0] == p[1])
                    {
                        return BooleanFunction::Const(0, node.size);
                    }

                    if (p[0].is(BooleanFunction::NodeType::Or) && p[1].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        auto p1_parameter = p[1].get_parameters();

                        // (X | Y) & (X | Z)   =>   X | (Y & Z)
                        if (p0_parameter[0] == p1_parameter[0])
                        {
                            return p0_parameter[0] | (p0_parameter[1] & p1_parameter[1]);
                        }
                        // (X | Y) & (Z | X)   =>   X | (Y & Z)
                        if (p0_parameter[0] == p1_parameter[1])
                        {
                            return p0_parameter[0] | (p0_parameter[1] & p1_parameter[0]);
                        }

                        // (X | Y) & (Y | Z)   =>   Y | (X & Z)
                        if (p0_parameter[1] == p1_parameter[0])
                        {
                            return p0_parameter[1] | (p0_parameter[0] & p1_parameter[1]);
                        }
                        // (X | Y) & (Z | Y)   =>   Y | (X & Z)
                        if (p0_parameter[1] == p1_parameter[1])
                        {
                            return p0_parameter[1] | (p0_parameter[0] & p1_parameter[0]);
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::And))
                    {
                        auto p1_parameter = p[1].get_parameters();
                        // X & (X & Y)   =>   (X & Y)
                        if (p[0] == p1_parameter[1])
                        {
                            return p[1];
                        }
                        // X & (Y & X)   =>   (Y & X)
                        if (p[0] == p1_parameter[0])
                        {
                            return p[1];
                        }

                        // X & (~X & Y)   =>   0
                        if ((~p1_parameter[0] == p[0]) || (p1_parameter[0] == ~p[0]))
                        {
                            return BooleanFunction::Const(0, node.size);
                        }
                        // X & (Y & ~X)   =>   0
                        if ((~p1_parameter[1] == p[0]) || (p1_parameter[1] == ~p[0]))
                        {
                            return BooleanFunction::Const(0, node.size);
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::Or))
                    {
                        auto p1_parameter = p[1].get_parameters();

                        // X & (X | Y)   =>   X
                        if (p1_parameter[0] == p[0])
                        {
                            return p[0];
                        }
                        // X & (Y | X)   =>   X
                        if (p1_parameter[1] == p[0])
                        {
                            return p[0];
                        }
                        // X & (~X | Y)   =>  X & Y
                        if ((~p1_parameter[0] == p[0]) || (p1_parameter[0] == ~p[0]))
                        {
                            return p[0] & p1_parameter[1];
                        }
                        // X & (Y | ~X)   =>  X & Y
                        if ((~p1_parameter[1] == p[0]) || (p1_parameter[1] == ~p[0]))
                        {
                            return p[0] & p1_parameter[0];
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X & Y) & X   =>    X & Y
                        if (p0_parameter[0] == p[1])
                        {
                            return p[0];
                        }

                        // (Y & X) & X   =>    Y & X
                        if (p0_parameter[1] == p[1])
                        {
                            return p[0];
                        }
                        // (~X & Y) & X   =>   0
                        if ((~p0_parameter[0] == p[1]) || (p0_parameter[0] == ~p[1]))
                        {
                            return BooleanFunction::Const(0, node.size);
                        }
                        // (Y & ~X) & X   =>   0
                        if ((~p0_parameter[1] == p[1]) || (p0_parameter[1] == ~p[1]))
                        {
                            return BooleanFunction::Const(0, node.size);
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X | Y) & X    =>   X
                        if (p0_parameter[0] == p[1])
                        {
                            return p[1];
                        }
                        // (Y | X) & X    =>   X
                        if (p0_parameter[1] == p[1])
                        {
                            return p[1];
                        }
                        // (~X | Y) & X   =>   X & Y
                        if ((~p0_parameter[0] == p[1]) || (p0_parameter[0] == ~p[1]))
                        {
                            return p[1] & p0_parameter[1];
                        }
                        // (Y | ~X) & X   =>   X & Y
                        if ((~p0_parameter[1] == p[1]) || (p0_parameter[1] == ~p[1]))
                        {
                            return p[1] & p0_parameter[0];
                        }
                    }

                    return p[0] & p[1];
                }
                case BooleanFunction::NodeType::Not: {
                    // ~~X   =>   X
                    if (p[0].is(BooleanFunction::NodeType::Not))
                    {
                        return p[0].get_parameters()[0];
                    }

                    // ~(~X & ~Y)   =>   X | Y
                    if (p[0].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        if (p0_parameter[0].is(BooleanFunction::NodeType::Not) && p0_parameter[1].is(BooleanFunction::NodeType::Not))
                        {
                            return p0_parameter[0].get_parameters()[0] | p0_parameter[1].get_parameters()[0];
                        }
                    }

                    // ~(~X | ~Y)   =>   X & Y
                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        if (p0_parameter[0].is(BooleanFunction::NodeType::Not) && p0_parameter[1].is(BooleanFunction::NodeType::Not))
                        {
                            return p0_parameter[0].get_parameters()[0] & p0_parameter[1].get_parameters()[0];
                        }
                    }

                    // ~(X | Y)   =>   ~X & ~Y
                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        return (~p0_parameter[0]) & (~p0_parameter[1]);
                    }

                    return ~p[0];
                }

                case BooleanFunction::NodeType::Or: {
                    // X | 0   =>   X
                    if (p[1].has_constant_value(0))
                    {
                        return p[0];
                    }

                    // X | 1   =>   1
                    if (p[1] == One(node.size))
                    {
                        return p[1];
                    }

                    // X | X   =>   X
                    if (p[0] == p[1])
                    {
                        return p[0];
                    }

                    // X | ~X   =>   1
                    if ((~p[0] == p[1]) || (p[0] == ~p[1]))
                    {
                        return ~BooleanFunction::Const(0, node.size).simplify();
                    }

                    if (p[0].is(BooleanFunction::NodeType::And) && p[1].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        auto p1_parameter = p[1].get_parameters();

                        // (X & Y) | (X & Z)    => X & (Y | Z)
                        if (p0_parameter[0] == p1_parameter[0])
                        {
                            return p0_parameter[0] & (p0_parameter[1] | p1_parameter[1]);
                        }
                        // (X & Y) | (Z & X)    => X & (Y | Z)
                        if (p0_parameter[0] == p1_parameter[1])
                        {
                            return p0_parameter[0] & (p0_parameter[1] | p1_parameter[0]);
                        }
                        // (X & Y) | (Y & Z)    => Y & (Y | Z)
                        if (p0_parameter[1] == p1_parameter[0])
                        {
                            return p0_parameter[1] & (p0_parameter[0] | p1_parameter[1]);
                        }
                        // (X & Y) | (Z & Y)    => Y & (X | Z)
                        if (p0_parameter[1] == p1_parameter[1])
                        {
                            return p0_parameter[1] & (p0_parameter[0] | p1_parameter[0]);
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::And))
                    {
                        auto p1_parameter = p[1].get_parameters();
                        // X | (Y & !X)   =>   X | Y
                        if ((~p1_parameter[1]) == p[0])
                        {
                            return p[0] | p1_parameter[0];
                        }

                        // X | (X & Y)    =>   X
                        if ((p1_parameter[0] == p[0]) || (p1_parameter[1] == p[0]))
                        {
                            return p[0];
                        }

                        // X | (~X & Y)   =>   X | Y
                        if ((~p1_parameter[0] == p[0]) || (p1_parameter[0] == ~p[0]))
                        {
                            return p[0] | p1_parameter[1];
                        }
                        // X | (Y & ~X)   =>   X | Y
                        if ((~p1_parameter[1] == p[0]) || (p1_parameter[1] == ~p[0]))
                        {
                            return p[0] | p1_parameter[0];
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::Or))
                    {
                        auto p1_parameter = p[1].get_parameters();

                        // X | (X | Y)   =>   (X | Y)
                        if (p1_parameter[0] == p[0])
                        {
                            return p[1];
                        }
                        // X | (Y | X)   =>   (Y | X)
                        if (p1_parameter[1] == p[0])
                        {
                            return p[1];
                        }

                        // X | (~X | Y)   =>   1
                        if ((~p1_parameter[0] == p[0]) || (p1_parameter[0] == ~p[0]))
                        {
                            return One(node.size);
                        }

                        // X | (Y | ~X)   =>   1
                        if ((~p1_parameter[1] == p[0]) || (p1_parameter[1] == ~p[0]))
                        {
                            return One(node.size);
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X | Y) | X   =>   (X | Y)
                        if (p0_parameter[0] == p[1])
                        {
                            return p[0];
                        }
                        // (Y | X) | X   =>   (Y | X)
                        if (p0_parameter[1] == p[1])
                        {
                            return p[0];
                        }

                        // (~X | Y) | X   =>   1
                        if ((~p0_parameter[0] == p[1]) || (p0_parameter[0] == ~p[1]))
                        {
                            return One(node.size);
                        }

                        // (Y | ~X) | X =>   1
                        if ((~p0_parameter[1] == p[1]) || (p0_parameter[1] == ~p[1]))
                        {
                            return One(node.size);
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X & Y) | X    =>   X
                        if (p0_parameter[0] == p[1])
                        {
                            return p[1];
                        }
                        // (Y & X) | X    =>   X
                        if (p0_parameter[1] == p[1])
                        {
                            return p[1];
                        }

                        // (~X & Y) | X   =>   X | Y
                        if ((~p0_parameter[0] == p[1]) || (p0_parameter[0] == ~p[1]))
                        {
                            return p0_parameter[1] | p[1];
                        }

                        // (X & ~Y) | Y   =>   X | Y
                        if ((~p0_parameter[1] == p[1]) || (p0_parameter[1] == ~p[1]))
                        {
                            return p0_parameter[0] | p[1];
                        }
                    }

                    return p[0] | p[1];
                }
                case BooleanFunction::NodeType::Xor: {
                    // X ^ 0   =>   x
                    if (p[1].has_constant_value(0))
                    {
                        return p[0];
                    }
                    // X ^ 1  =>   ~X
                    if (p[1] == One(node.size))
                    {
                        return ~p[0];
                    }
                    // X ^ X   =>   0
                    if (p[0] == p[1])
                    {
                        return BooleanFunction::Const(0, node.size);
                    }
                    // X ^ ~X   =>   1
                    if (~p[0] == p[1])
                    {
                        return One(node.size);
                    }

                    return p[0] ^ p[1];
                }
                default:
                    return "not implemented reached";
            }
        }

        std::variant<BooleanFunction, std::string> SymbolicExecution::constant_propagation(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p)
        {
            if (node.get_arity() != p.size())
            {
                ERROR("Arity " << node.get_arity() << " does not match number of parameters (= " << p.size() << ").");
            }

            std::vector<std::vector<BooleanFunction::Value>> values;
            for (const auto& parameter : p)
            {
                values.emplace_back(parameter.get_top_level_node().constant);
            }

            switch (node.type)
            {
                case BooleanFunction::NodeType::And:
                    return ConstantPropagation::And(values[0], values[1]);
                case BooleanFunction::NodeType::Or:
                    return ConstantPropagation::Or(values[0], values[1]);
                case BooleanFunction::NodeType::Not:
                    return ConstantPropagation::Not(values[0]);
                case BooleanFunction::NodeType::Xor:
                    return ConstantPropagation::Xor(values[0], values[1]);

                default:
                    return "not implemented reached";
            }
        }
    }    // namespace SMT
}    // namespace hal