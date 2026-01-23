#include "hal_core/netlist/boolean_function/symbolic_execution.h"
#include "hal_core/utilities/log.h"

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

            /**
             * Helper function to simplify a constant ADD operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Add(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                if (p0.size() <= 64 && p1.size() <= 64)
                {
                    const auto a_res = BooleanFunction::to_u64(p0);
                    const auto b_res = BooleanFunction::to_u64(p1);

                    if (a_res.is_ok() && b_res.is_ok())
                    {
                        const auto res = (a_res.get() + b_res.get()) & 0xffffffff;
                        return BooleanFunction::Const(res, p0.size());
                    }

                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::X));
                }

                if (std::any_of(p0.begin(), p0.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; })
                    || std::any_of(p1.begin(), p1.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; }))
                {
                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::X));
                }

                std::vector<BooleanFunction::Value> simplified;
                simplified.reserve(p0.size());
                auto carry = BooleanFunction::Value::ZERO;
                for (auto i = 0u; i < p0.size(); i++)
                {
                    auto res = p0[i] + p1[i] + carry;
                    simplified.emplace_back(static_cast<BooleanFunction::Value>(res & 0x1));
                    carry = static_cast<BooleanFunction::Value>(res >> 1);
                }
                return BooleanFunction::Const(simplified);
            }

            /**
             * Helper function to simplify a constant SUB operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Sub(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                if (p0.size() <= 64 && p1.size() <= 64)
                {
                    const auto a_res = BooleanFunction::to_u64(p0);
                    const auto b_res = BooleanFunction::to_u64(p1);

                    if (a_res.is_ok() && b_res.is_ok())
                    {
                        const auto res = (a_res.get() - b_res.get()) & 0xffffffff;
                        return BooleanFunction::Const(res, p0.size());
                    }

                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::X));
                }

                if (std::any_of(p0.begin(), p0.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; })
                    || std::any_of(p1.begin(), p1.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; }))
                {
                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::X));
                }

                std::vector<BooleanFunction::Value> simplified;
                simplified.reserve(p0.size());
                auto carry = BooleanFunction::Value::ONE;
                for (auto i = 0u; i < p0.size(); i++)
                {
                    auto res = p0[i] + !(p1[i]) + carry;
                    simplified.emplace_back(static_cast<BooleanFunction::Value>(res & 0x1));
                    carry = static_cast<BooleanFunction::Value>(res >> 1);
                }
                return BooleanFunction::Const(simplified);
            }

            /**
             * Helper function to simplify a constant MUL operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Mul(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                if (std::any_of(p0.begin(), p0.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; })
                    || std::any_of(p1.begin(), p1.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; }))
                {
                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::X));
                }

                auto bitsize = p0.size();
                std::vector<BooleanFunction::Value> simplified(bitsize, BooleanFunction::Value::ZERO);
                for (auto i = 0u; i < bitsize; i++)
                {
                    auto carry = BooleanFunction::Value::ZERO;
                    for (auto j = 0u; j < bitsize - i; j++)
                    {
                        auto res          = simplified[i + j] + (p0[i] & p1[j]) + carry;
                        simplified[i + j] = static_cast<BooleanFunction::Value>(res & 0x1);
                        carry             = static_cast<BooleanFunction::Value>(res >> 1);
                    }
                }
                return BooleanFunction::Const(simplified);
            }

            /**
             * Helper function to simplify a constant SHL operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Shl(const std::vector<BooleanFunction::Value>& p0, const u16 p1)
            {
                if (p1 >= p0.size())
                {
                    // Shift amount is too large, result is all zeros
                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::ZERO));
                }

                std::vector<BooleanFunction::Value> result(p0.size(), BooleanFunction::Value::ZERO);

                // Copy bits from original position to shifted position
                for (auto i = p1; i < p0.size(); i++)
                {
                    result[i] = p0[i - p1];
                }

                return BooleanFunction::Const(result);
            }

            /**
             * Helper function to simplify a constant LSHR operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Lshr(const std::vector<BooleanFunction::Value>& p0, const u16 p1)
            {
                if (p1 >= p0.size())
                {
                    // Shift amount is too large, result is all zeros
                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::ZERO));
                }

                std::vector<BooleanFunction::Value> result(p0.size(), BooleanFunction::Value::ZERO);

                // Copy bits from original position to shifted position
                for (auto i = 0u; i < p0.size() - p1; i++)
                {
                    result[i] = p0[i + p1];
                }

                return BooleanFunction::Const(result);
            }

            /**
             * Helper function to simplify a constant ASHR operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Ashr(const std::vector<BooleanFunction::Value>& p0, const u16 p1)
            {
                auto sign_bit = p0.back();    // MSB is the sign bit

                if (p1 >= p0.size())
                {
                    // Shift amount is too large, result is all sign bits
                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), sign_bit));
                }

                std::vector<BooleanFunction::Value> result(p0.size(), sign_bit);

                // Copy bits from original position to shifted position
                for (auto i = 0u; i < p0.size() - p1; i++)
                {
                    result[i] = p0[i + p1];
                }

                return BooleanFunction::Const(result);
            }

            /**
             * Helper function to simplify a constant ROL operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Rol(const std::vector<BooleanFunction::Value>& p0, const u16 p1)
            {
                auto rotate_amount = p1 % p0.size();    // Modulo for rotation

                if (rotate_amount == 0)
                {
                    return BooleanFunction::Const(p0);    // No rotation needed
                }

                std::vector<BooleanFunction::Value> result(p0.size());

                // Perform the rotation
                for (auto i = 0u; i < p0.size(); i++)
                {
                    auto new_pos    = (i + rotate_amount) % p0.size();
                    result[new_pos] = p0[i];
                }

                return BooleanFunction::Const(result);
            }

            /**
             * Helper function to simplify a constant ROR operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Ror(const std::vector<BooleanFunction::Value>& p0, const u16 p1)
            {
                auto rotate_amount = p1 % p0.size();    // Modulo for rotation

                if (rotate_amount == 0)
                {
                    return BooleanFunction::Const(p0);    // No rotation needed
                }

                std::vector<BooleanFunction::Value> result(p0.size());

                // Perform the rotation
                for (auto i = 0u; i < p0.size(); i++)
                {
                    auto new_pos    = (i + p0.size() - rotate_amount) % p0.size();
                    result[new_pos] = p0[i];
                }

                return BooleanFunction::Const(result);
            }
            /**
             * Helper function to simplify a constant SLE operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Sle(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                if (std::any_of(p0.begin(), p0.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; })
                    || std::any_of(p1.begin(), p1.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; }))
                {
                    return BooleanFunction::Const({BooleanFunction::Value::X});
                }

                auto msb_p0 = p0.back();
                auto msb_p1 = p1.back();
                if (msb_p0 == BooleanFunction::Value::ONE && msb_p1 == BooleanFunction::Value::ZERO)
                {
                    return BooleanFunction::Const(1, 1);
                }
                else if (msb_p0 == BooleanFunction::Value::ZERO && msb_p1 == BooleanFunction::Value::ONE)
                {
                    return BooleanFunction::Const(0, 1);
                }

                std::vector<BooleanFunction::Value> simplified;
                u8 carry = 1;
                u8 neq   = 0;
                u8 res   = 0;
                for (auto i = 0u; i < p0.size(); i++)
                {
                    res   = p0[i] + !(p1[i]) + carry;
                    carry = res >> 1;
                    neq |= res & 1;
                }

                return BooleanFunction::Const({static_cast<BooleanFunction::Value>((res & 1) | !neq)});
            }

            /**
             * Helper function to simplify a constant SLT operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Slt(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                if (std::any_of(p0.begin(), p0.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; })
                    || std::any_of(p1.begin(), p1.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; }))
                {
                    return BooleanFunction::Const({BooleanFunction::Value::X});
                }

                auto msb_p0 = p0.back();
                auto msb_p1 = p1.back();
                if (msb_p0 == BooleanFunction::Value::ONE && msb_p1 == BooleanFunction::Value::ZERO)
                {
                    return BooleanFunction::Const(1, 1);
                }
                else if (msb_p0 == BooleanFunction::Value::ZERO && msb_p1 == BooleanFunction::Value::ONE)
                {
                    return BooleanFunction::Const(0, 1);
                }

                std::vector<BooleanFunction::Value> simplified;
                u8 res   = 0;
                u8 carry = 1;
                for (auto i = 0u; i < p0.size(); i++)
                {
                    res   = p0[i] + !(p1[i]) + carry;
                    carry = (res >> 1) & 1;
                }

                return BooleanFunction::Const({static_cast<BooleanFunction::Value>(res & 1)});
            }

            /**
             * Helper function to simplify a constant ULE operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Ule(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                if (std::any_of(p0.begin(), p0.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; })
                    || std::any_of(p1.begin(), p1.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; }))
                {
                    return BooleanFunction::Const({BooleanFunction::Value::X});
                }

                for (i32 i = p0.size() - 1; i >= 0; i--)
                {
                    if (p0[i] == BooleanFunction::Value::ONE && p1[i] == BooleanFunction::Value::ZERO)
                    {
                        return BooleanFunction::Const(0, 1);
                    }
                    else if (p0[i] == BooleanFunction::Value::ZERO && p1[i] == BooleanFunction::Value::ONE)
                    {
                        return BooleanFunction::Const(1, 1);
                    }
                }
                return BooleanFunction::Const(1, 1);
            }

            /**
             * Helper function to simplify a constant ULT operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Ult(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1)
            {
                if (std::any_of(p0.begin(), p0.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; })
                    || std::any_of(p1.begin(), p1.end(), [](auto val) { return val == BooleanFunction::Value::X || val == BooleanFunction::Value::Z; }))
                {
                    return BooleanFunction::Const({BooleanFunction::Value::X});
                }

                for (i32 i = p0.size() - 1; i >= 0; i--)
                {
                    if (p0[i] == BooleanFunction::Value::ONE && p1[i] == BooleanFunction::Value::ZERO)
                    {
                        return BooleanFunction::Const(0, 1);
                    }
                    else if (p0[i] == BooleanFunction::Value::ZERO && p1[i] == BooleanFunction::Value::ONE)
                    {
                        return BooleanFunction::Const(1, 1);
                    }
                }
                return BooleanFunction::Const(0, 1);
            }

            /**
             * Helper function to simplify a constant ITE operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @param[in] p2 - Boolean function parameter 2.
             * @returns Boolean function with a simplified constant value.
             */
            BooleanFunction Ite(const std::vector<BooleanFunction::Value>& p0, const std::vector<BooleanFunction::Value>& p1, const std::vector<BooleanFunction::Value>& p2)
            {
                if (p0.front() == BooleanFunction::Value::ONE)
                {
                    return BooleanFunction::Const(p1);
                }
                else if (p0.front() == BooleanFunction::Value::ZERO)
                {
                    return BooleanFunction::Const(p2);
                }
                else
                {
                    return BooleanFunction::Const(std::vector<BooleanFunction::Value>(p0.size(), BooleanFunction::Value::X));
                }
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

        Result<BooleanFunction> SymbolicExecution::evaluate(const BooleanFunction& function) const
        {
            std::vector<BooleanFunction> stack;
            for (const auto& node : function.get_nodes())
            {
                std::vector<BooleanFunction> parameters;
                std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(parameters));
                stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

                if (auto simplified = this->simplify(node, std::move(parameters)); simplified.is_ok())
                {
                    stack.emplace_back(simplified.get());
                }
                else
                {
                    return ERR_APPEND(simplified.get_error(), "could not evaluate Boolean function within symbolic state: simplification failed");
                }
            }

            switch (stack.size())
            {
                case 1:
                    return OK(stack.back());
                default:
                    return ERR("could not evaluate Boolean function within symbolic state: stack is imbalanced");
            }
        }

        Result<std::monostate> SymbolicExecution::evaluate(const Constraint& constraint)
        {
            if (constraint.is_assignment())
            {
                const auto& assignment = constraint.get_assignment().get();
                if (auto res = this->evaluate(assignment->first).map<std::monostate>([&](auto&& rhs) -> Result<std::monostate> {
                        this->state.set(assignment->second.clone(), std::move(rhs));
                        return OK({});
                    });
                    res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not to evaluate assignment constraint within the symbolic state: evaluation failed");
                }
                else
                {
                    return OK({});
                }
            }
            else
            {
                const auto& function = constraint.get_function().get();
                auto node_type       = function->get_top_level_node().type;
                if (!(node_type == BooleanFunction::NodeType::Eq || node_type == BooleanFunction::NodeType::Slt || node_type == BooleanFunction::NodeType::Sle
                      || node_type == BooleanFunction::NodeType::Ult || node_type == BooleanFunction::NodeType::Ule))
                {
                    return ERR("invalid node type in function '" + function->to_string() + "'");
                }
                if (auto res = this->evaluate(*function); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not to evaluate function constraint within the symbolic state: evaluation failed");
                }
                else
                {
                    return OK({});
                }
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

        namespace
        {
            /**
             * Helper function to check whether one of the two functions is just the other function negated.
             */
            bool is_x_not_y(const BooleanFunction& x, const BooleanFunction& y)
            {
                const BooleanFunction& smaller = (x.get_nodes().size() < y.get_nodes().size()) ? x : y;
                const BooleanFunction& bigger  = (x.get_nodes().size() < y.get_nodes().size()) ? y : x;

                // The node vector of the first function needs to be exactly one element shorter than the second
                if (smaller.get_nodes().size() != (bigger.get_nodes().size() - 1))
                {
                    return false;
                }

                // The top level node of the bigger node vector needs to be a NOT node
                if (bigger.get_top_level_node().type != BooleanFunction::NodeType::Not)
                {
                    return false;
                }

                // Every n'th element in the smaller node vector has to be identical to the n'th element of the bigger node vector, except the last/top level node
                for (u32 idx = 0; idx < smaller.get_nodes().size(); idx++)
                {
                    if (smaller.get_nodes().at(idx) != bigger.get_nodes().at(idx))
                    {
                        return false;
                    }
                }

                return true;
            }
        }    // namespace

        Result<BooleanFunction> SymbolicExecution::simplify(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p) const
        {
            if (!p.empty() && std::all_of(p.begin(), p.end(), [](const auto& function) { return function.is_constant() || function.is_index(); }))
            {
                if (auto res = SymbolicExecution::constant_propagation(node, std::move(p)); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not simplify sub-expression in abstract syntax tree: constant propagation failed");
                }
                else
                {
                    return res;
                }
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
                    return OK(BooleanFunction::Const(node.constant));
                }
                case BooleanFunction::NodeType::Index: {
                    return OK(BooleanFunction::Index(node.index, node.size));
                }
                case BooleanFunction::NodeType::Variable: {
                    return OK(this->state.get(BooleanFunction::Var(node.variable, node.size)));
                }
                case BooleanFunction::NodeType::And: {
                    // X & 0   =>   0
                    if (p[1].has_constant_value(0))
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }
                    // X & 1  =>   X
                    if (p[1] == One(node.size))
                    {
                        return OK(p[0]);
                    }
                    // X & X   =>   X
                    if (p[0] == p[1])
                    {
                        return OK(p[0]);
                    }
                    // X & ~X   =>   0
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }

                    if (p[0].is(BooleanFunction::NodeType::Or) && p[1].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        auto p1_parameter = p[1].get_parameters();

                        // (X | Y) & (X | Z)   =>   X | (Y & Z)
                        if (p0_parameter[0] == p1_parameter[0])
                        {
                            return OK(p0_parameter[0] | (p0_parameter[1] & p1_parameter[1]));
                        }
                        // (X | Y) & (Z | X)   =>   X | (Y & Z)
                        if (p0_parameter[0] == p1_parameter[1])
                        {
                            return OK(p0_parameter[0] | (p0_parameter[1] & p1_parameter[0]));
                        }

                        // (X | Y) & (Y | Z)   =>   Y | (X & Z)
                        if (p0_parameter[1] == p1_parameter[0])
                        {
                            return OK(p0_parameter[1] | (p0_parameter[0] & p1_parameter[1]));
                        }
                        // (X | Y) & (Z | Y)   =>   Y | (X & Z)
                        if (p0_parameter[1] == p1_parameter[1])
                        {
                            return OK(p0_parameter[1] | (p0_parameter[0] & p1_parameter[0]));
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::And))
                    {
                        auto p1_parameter = p[1].get_parameters();
                        // X & (X & Y)   =>   (X & Y)
                        if (p[0] == p1_parameter[1])
                        {
                            return OK(p[1]);
                        }
                        // X & (Y & X)   =>   (Y & X)
                        if (p[0] == p1_parameter[0])
                        {
                            return OK(p[1]);
                        }

                        // X & (~X & Y)   =>   0
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(BooleanFunction::Const(0, node.size));
                        }
                        // X & (Y & ~X)   =>   0
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(BooleanFunction::Const(0, node.size));
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::Or))
                    {
                        auto p1_parameter = p[1].get_parameters();

                        // X & (X | Y)   =>   X
                        if (p1_parameter[0] == p[0])
                        {
                            return OK(p[0]);
                        }
                        // X & (Y | X)   =>   X
                        if (p1_parameter[1] == p[0])
                        {
                            return OK(p[0]);
                        }
                        // X & (~X | Y)   =>  X & Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return BooleanFunction::And(p[0].clone(), p1_parameter[1].clone(), node.size);
                        }
                        // X & (Y | ~X)   =>  X & Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return BooleanFunction::And(p[0].clone(), p1_parameter[0].clone(), node.size);
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X & Y) & X   =>    X & Y
                        if (p0_parameter[0] == p[1])
                        {
                            return OK(p[0]);
                        }

                        // (Y & X) & X   =>    Y & X
                        if (p0_parameter[1] == p[1])
                        {
                            return OK(p[0]);
                        }
                        // (~X & Y) & X   =>   0
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(BooleanFunction::Const(0, node.size));
                        }
                        // (Y & ~X) & X   =>   0
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(BooleanFunction::Const(0, node.size));
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X | Y) & X    =>   X
                        if (p0_parameter[0] == p[1])
                        {
                            return OK(p[1]);
                        }
                        // (Y | X) & X    =>   X
                        if (p0_parameter[1] == p[1])
                        {
                            return OK(p[1]);
                        }
                        // (~X | Y) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return BooleanFunction::And(p[1].clone(), p0_parameter[1].clone(), node.size);
                        }
                        // (Y | ~X) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return BooleanFunction::And(p[1].clone(), p0_parameter[0].clone(), node.size);
                        }
                    }

                    return OK(p[0] & p[1]);
                }
                case BooleanFunction::NodeType::Not: {
                    // ~~X   =>   X
                    if (p[0].is(BooleanFunction::NodeType::Not))
                    {
                        return OK(p[0].get_parameters()[0]);
                    }

                    // ~(~X & ~Y)   =>   X | Y
                    if (p[0].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        if (p0_parameter[0].is(BooleanFunction::NodeType::Not) && p0_parameter[1].is(BooleanFunction::NodeType::Not))
                        {
                            return BooleanFunction::Or(p0_parameter[0].get_parameters()[0].clone(), p0_parameter[1].get_parameters()[0].clone(), node.size);
                        }
                    }

                    // ~(~X | ~Y)   =>   X & Y
                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        if (p0_parameter[0].is(BooleanFunction::NodeType::Not) && p0_parameter[1].is(BooleanFunction::NodeType::Not))
                        {
                            return BooleanFunction::And(p0_parameter[0].get_parameters()[0].clone(), p0_parameter[1].get_parameters()[0].clone(), node.size);
                        }
                    }

                    // ~(X | Y)   =>   ~X & ~Y
                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        return OK((~p0_parameter[0]) & (~p0_parameter[1]));
                    }

                    // ~(X & Y)   =>   ~X | ~Y
                    if (p[0].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        return OK((~p0_parameter[0]) | (~p0_parameter[1]));
                    }

                    return BooleanFunction::Not(p[0].clone(), node.size);
                }

                case BooleanFunction::NodeType::Or: {
                    // X | 0   =>   X
                    if (p[1].has_constant_value(0))
                    {
                        return OK(p[0]);
                    }

                    // X | 1   =>   1
                    if (p[1] == One(node.size))
                    {
                        return OK(p[1]);
                    }

                    // X | X   =>   X
                    if (p[0] == p[1])
                    {
                        return OK(p[0]);
                    }

                    // X | ~X   =>   111...1
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(One(node.size));
                    }

                    if (p[0].is(BooleanFunction::NodeType::And) && p[1].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        auto p1_parameter = p[1].get_parameters();

                        // (X & Y) | (X & Z)    => X & (Y | Z)
                        if (p0_parameter[0] == p1_parameter[0])
                        {
                            return OK(p0_parameter[0] & (p0_parameter[1] | p1_parameter[1]));
                        }
                        // (X & Y) | (Z & X)    => X & (Y | Z)
                        if (p0_parameter[0] == p1_parameter[1])
                        {
                            return OK(p0_parameter[0] & (p0_parameter[1] | p1_parameter[0]));
                        }
                        // (X & Y) | (Y & Z)    => Y & (Y | Z)
                        if (p0_parameter[1] == p1_parameter[0])
                        {
                            return OK(p0_parameter[1] & (p0_parameter[0] | p1_parameter[1]));
                        }
                        // (X & Y) | (Z & Y)    => Y & (X | Z)
                        if (p0_parameter[1] == p1_parameter[1])
                        {
                            return OK(p0_parameter[1] & (p0_parameter[0] | p1_parameter[0]));
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::And))
                    {
                        auto p1_parameter = p[1].get_parameters();
                        // X | (Y & !X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return BooleanFunction::Or(p[0].clone(), p1_parameter[0].clone(), node.size);
                        }

                        // X | (X & Y)    =>   X
                        if ((p1_parameter[0] == p[0]) || (p1_parameter[1] == p[0]))
                        {
                            return OK(p[0]);
                        }

                        // X | (~X & Y)   =>   X | Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return BooleanFunction::Or(p[0].clone(), p1_parameter[1].clone(), node.size);
                        }
                        // X | (Y & ~X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return BooleanFunction::Or(p[0].clone(), p1_parameter[0].clone(), node.size);
                        }
                    }

                    if (p[1].is(BooleanFunction::NodeType::Or))
                    {
                        auto p1_parameter = p[1].get_parameters();

                        // X | (X | Y)   =>   (X | Y)
                        if (p1_parameter[0] == p[0])
                        {
                            return OK(p[1]);
                        }
                        // X | (Y | X)   =>   (Y | X)
                        if (p1_parameter[1] == p[0])
                        {
                            return OK(p[1]);
                        }

                        // X | (~X | Y)   =>   1
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(One(node.size));
                        }

                        // X | (Y | ~X)   =>   1
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(One(node.size));
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::Or))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X | Y) | X   =>   (X | Y)
                        if (p0_parameter[0] == p[1])
                        {
                            return OK(p[0]);
                        }
                        // (Y | X) | X   =>   (Y | X)
                        if (p0_parameter[1] == p[1])
                        {
                            return OK(p[0]);
                        }

                        // (~X | Y) | X   =>   1
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(One(node.size));
                        }

                        // (Y | ~X) | X =>   1
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(One(node.size));
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::And))
                    {
                        auto p0_parameter = p[0].get_parameters();

                        // (X & Y) | X    =>   X
                        if (p0_parameter[0] == p[1])
                        {
                            return OK(p[1]);
                        }
                        // (Y & X) | X    =>   X
                        if (p0_parameter[1] == p[1])
                        {
                            return OK(p[1]);
                        }

                        // (~X & Y) | X   =>   X | Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return BooleanFunction::Or(p0_parameter[1].clone(), p[1].clone(), node.size);
                        }

                        // (X & ~Y) | Y   =>   X | Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return BooleanFunction::Or(p0_parameter[0].clone(), p[1].clone(), node.size);
                        }
                    }

                    return BooleanFunction::Or(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Xor: {
                    // X ^ 0   =>   X
                    if (p[1].has_constant_value(0))
                    {
                        return OK(p[0]);
                    }
                    // X ^ 1  =>   ~X
                    if (p[1] == One(node.size))
                    {
                        return BooleanFunction::Not(p[0].clone(), node.size);
                    }
                    // X ^ X   =>   0
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }
                    // X ^ ~X   =>   1
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(One(node.size));
                    }

                    return BooleanFunction::Xor(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Add: {
                    // X + 0    =>   X
                    if (p[1].has_constant_value(0))
                    {
                        return OK(p[0]);
                    }

                    return BooleanFunction::Add(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Sub: {
                    // X - 0    =>   X
                    if (p[1].has_constant_value(0))
                    {
                        return OK(p[0]);
                    }
                    // X - X    =>   0
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }

                    return BooleanFunction::Sub(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Mul: {
                    // X * 0    =>   0
                    if (p[1].has_constant_value(0))
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }
                    // X * 1    =>   X
                    if (p[1].has_constant_value(1))
                    {
                        return OK(p[0]);
                    }

                    return BooleanFunction::Mul(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Sdiv: {
                    // X /s 1    =>   X
                    if (p[1].has_constant_value(1))
                    {
                        return OK(p[0]);
                    }
                    // X /s X    =>   1
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(1, node.size));
                    }

                    return BooleanFunction::Sdiv(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Udiv: {
                    // X / 1    =>   X
                    if (p[1].has_constant_value(1))
                    {
                        return OK(p[0]);
                    }
                    // X / X    =>   1
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(1, node.size));
                    }

                    return BooleanFunction::Udiv(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Srem: {
                    // X %s 1    =>   0
                    if (p[1].has_constant_value(1))
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }
                    // X %s X    =>   0
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }

                    return BooleanFunction::Srem(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Urem: {
                    // X %s 1    =>   0
                    if (p[1].has_constant_value(1))
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }
                    // X %s X    =>   0
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }

                    return BooleanFunction::Urem(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Slice: {
                    // SLICE(p, 0, 0)   =>   p (if p is 1-bit wide)
                    // if ((p[0].size() == 1) && p[1].has_index_value(0) && p[2].has_index_value(0) && (node.size == 1))
                    // {
                    //     return OK(p[0]);
                    // }

                    // SLICE(p, 0, size-1) => p
                    if (node.size == p[0].size() && p[1].has_index_value(0) && p[2].has_index_value(node.size - 1))
                    {
                        return OK(p[0]);
                    }

                    return BooleanFunction::Slice(p[0].clone(), p[1].clone(), p[2].clone(), node.size);
                }
                case BooleanFunction::NodeType::Concat: {
                    // CONCAT(X, Y) => CONST(X || Y)
                    if (p[0].is_constant() && p[1].is_constant())
                    {
                        if ((p[0].size() + p[1].size()) <= 64)
                        {
                            return OK(BooleanFunction::Const((p[0].get_constant_value_u64().get() << p[1].size()) + p[1].get_constant_value_u64().get(), p[0].size() + p[1].size()));
                        }
                    }

                    // We intend to group slices into the same concatination, so that they maybe can be merged into one slice. We try to do this from right to left to make succeeding simplifications easier.
                    if (p[0].is(BooleanFunction::NodeType::Slice) && p[1].is(BooleanFunction::NodeType::Concat))
                    {
                        auto p1_parameter = p[1].get_parameters();

                        if (p1_parameter[0].is(BooleanFunction::NodeType::Slice))
                        {
                            auto p0_parameter  = p[0].get_parameters();
                            auto p10_parameter = p1_parameter[0].get_parameters();

                            if (p0_parameter[0] == p10_parameter[0])
                            {
                                if (p1_parameter[1].is(BooleanFunction::NodeType::Slice))
                                {
                                    auto p11_parameter = p1_parameter[1].get_parameters();

                                    // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), SLICE(Z, m, n))) => CONCAT(CONCAT(SLICE(X, i, j), SLICE(X, k, l)), SLICE(Z, m, n)))
                                    if (p11_parameter[0] != p10_parameter[0])
                                    {
                                        if (auto concatination = BooleanFunction::Concat(p[0].clone(), p1_parameter[0].clone(), p[0].size() + p1_parameter[0].size()); concatination.is_ok())
                                        {
                                            return BooleanFunction::Concat(concatination.get(), p1_parameter[1].clone(), concatination.get().size() + p1_parameter[1].size());
                                        }
                                    }
                                }
                                else if (p1_parameter[1].is(BooleanFunction::NodeType::Concat))
                                {
                                    auto p11_parameter = p1_parameter[1].get_parameters();

                                    if (p11_parameter[0].is(BooleanFunction::NodeType::Slice))
                                    {
                                        auto p110_parameter = p11_parameter[0].get_parameters();

                                        // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), CONCAT(SLICE(Y, m, n), Z))) => CONCAT(CONCAT(SLICE(X, i, j), SLICE(X, k, l)), CONCAT(SLICE(Y, m, n), Z)))
                                        if (p110_parameter[0] != p10_parameter[0])
                                        {
                                            auto c1 = BooleanFunction::Concat(p[0].clone(), p1_parameter[0].clone(), p[0].size() + p1_parameter[0].size());

                                            return BooleanFunction::Concat(c1.get().clone(), p1_parameter[1].clone(), c1.get().size() + p1_parameter[1].size());
                                        }
                                    }
                                }
                                else
                                {
                                    // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), Y)) => CONCAT(CONCAT(SLICE(X, i, j), SLICE(X, k, l)), Y))
                                    if (auto concatination = BooleanFunction::Concat(p[0].clone(), p1_parameter[0].clone(), p[0].size() + p1_parameter[0].size()); concatination.is_ok())
                                    {
                                        return BooleanFunction::Concat(concatination.get(), p1_parameter[1].clone(), concatination.get().size() + p1_parameter[1].size());
                                    }
                                }
                            }
                        }
                    }

                    if (p[0].is(BooleanFunction::NodeType::Slice) && p[1].is(BooleanFunction::NodeType::Slice))
                    {
                        auto p0_parameter = p[0].get_parameters();
                        auto p1_parameter = p[1].get_parameters();

                        if (p0_parameter[0] == p1_parameter[0])
                        {
                            // CONCAT(SLICE(X, j+1, k), SLICE(X, i, j)) => SLICE(X, i, k)
                            if ((p1_parameter[2].get_index_value().get() == (p0_parameter[1].get_index_value().get() - 1)))
                            {
                                return BooleanFunction::Slice(p0_parameter[0].clone(), p1_parameter[1].clone(), p0_parameter[2].clone(), p[0].size() + p[1].size());
                            }

                            // CONCAT(SLICE(X, j, j), SLICE(X, i, j)) => SEXT(SLICE(X, i, j), j-i+1)
                            if ((p1_parameter[2].get_index_value().get() == p0_parameter[1].get_index_value().get())
                                && (p1_parameter[2].get_index_value().get() == p0_parameter[2].get_index_value().get()))
                            {
                                return BooleanFunction::Sext(p[1].clone(), BooleanFunction::Index(p[1].size() + 1, p[1].size() + 1), p[1].size() + 1);
                            }
                        }
                    }

                    // CONCAT(SLICE(X, j, j), SEXT(SLICE(X, i, j), j-i+n)) => SEXT(SLICE(X, i, j), j-i+n+1)
                    if (p[0].is(BooleanFunction::NodeType::Slice) && p[1].is(BooleanFunction::NodeType::Sext))
                    {
                        auto p1_parameter = p[1].get_parameters();

                        if (p1_parameter[0].is(BooleanFunction::NodeType::Slice))
                        {
                            auto p0_parameter  = p[0].get_parameters();
                            auto p10_parameter = p1_parameter[0].get_parameters();

                            if ((p0_parameter[0] == p10_parameter[0]) && (p0_parameter[1] == p0_parameter[2]) && (p0_parameter[1].get_index_value().get() == p10_parameter[2].get_index_value().get()))
                            {
                                return BooleanFunction::Sext(p1_parameter[0].clone(), BooleanFunction::Index(p[1].size() + 1, p[1].size() + 1), p[1].size() + 1);
                            }
                        }
                    }

                    // CONCAT(SLICE(X, j, j), CONCAT(SEXT(SLICE(X, i, j), j-i+n), Y)) => CONCAT(SEXT(SLICE(X, i, j), j-i+n+1), Y)
                    if (p[0].is(BooleanFunction::NodeType::Slice) && p[1].is(BooleanFunction::NodeType::Concat))
                    {
                        auto p1_parameter = p[1].get_parameters();

                        if (p1_parameter[0].is(BooleanFunction::NodeType::Sext))
                        {
                            auto p10_parameter = p1_parameter[0].get_parameters();

                            if (p10_parameter[0].is(BooleanFunction::NodeType::Slice))
                            {
                                auto p0_parameter   = p[0].get_parameters();
                                auto p100_parameter = p10_parameter[0].get_parameters();

                                if ((p0_parameter[0] == p100_parameter[0]) && (p0_parameter[1] == p0_parameter[2])
                                    && (p0_parameter[1].get_index_value().get() == p100_parameter[2].get_index_value().get()))
                                {
                                    if (auto extension =
                                            BooleanFunction::Sext(p10_parameter[0].clone(), BooleanFunction::Index(p1_parameter[0].size() + 1, p1_parameter[0].size() + 1), p1_parameter[0].size() + 1);
                                        extension.is_ok())
                                    {
                                        return BooleanFunction::Concat(extension.get(), p1_parameter[1].clone(), extension.get().size() + p1_parameter[1].size());
                                    }
                                }
                            }
                        }
                    }

                    return BooleanFunction::Concat(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Zext: {
                    return BooleanFunction::Zext(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Sext: {
                    return BooleanFunction::Sext(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Eq: {
                    // X == X   =>   1
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(1, node.size));
                    }

                    return BooleanFunction::Eq(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Sle: {
                    // X <=s X   =>   1
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(1, node.size));
                    }

                    return BooleanFunction::Sle(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Slt: {
                    // X <s X   =>   0
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }

                    return BooleanFunction::Slt(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Ule: {
                    // X <= X   =>   1
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(1, node.size));
                    }

                    return BooleanFunction::Ule(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Ult: {
                    // X < 0   =>   0
                    if (p[1].has_constant_value(0))
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }
                    // X < X   =>   0
                    if (p[0] == p[1])
                    {
                        return OK(BooleanFunction::Const(0, node.size));
                    }

                    return BooleanFunction::Ult(p[0].clone(), p[1].clone(), node.size);
                }
                case BooleanFunction::NodeType::Ite: {
                    // ITE(0, a, b)  =>  b
                    if (p[0].has_constant_value(0))
                    {
                        return OK(p[2]);
                    }
                    // ITE(1, a, b)  =>  a
                    if (p[0].has_constant_value(1))
                    {
                        return OK(p[1]);
                    }
                    // ITE(a, b, b)  =>  b
                    if (p[1] == p[2])
                    {
                        return OK(p[1]);
                    }

                    return BooleanFunction::Ite(p[0].clone(), p[1].clone(), p[2].clone(), node.size);
                }
                default:
                    return ERR("could not simplify sub-expression in abstract syntax tree: not implemented for given node type");
            }
        }

        Result<BooleanFunction> SymbolicExecution::constant_propagation(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p)
        {
            if (node.get_arity() != p.size())
            {
                return ERR("could not propagate constants: arity does not match number of parameters");
            }

            std::vector<std::vector<BooleanFunction::Value>> values;
            std::vector<u16> indices;

            for (const auto& parameter : p)
            {
                if (parameter.is_index())
                {
                    indices.push_back(parameter.get_index_value().get());
                }
                else
                {
                    const auto v = parameter.get_top_level_node().constant;
                    values.emplace_back(v);
                }
            }

            switch (node.type)
            {
                case BooleanFunction::NodeType::And:
                    return OK(ConstantPropagation::And(values[0], values[1]));
                case BooleanFunction::NodeType::Or:
                    return OK(ConstantPropagation::Or(values[0], values[1]));
                case BooleanFunction::NodeType::Not:
                    return OK(ConstantPropagation::Not(values[0]));
                case BooleanFunction::NodeType::Xor:
                    return OK(ConstantPropagation::Xor(values[0], values[1]));

                case BooleanFunction::NodeType::Add:
                    return OK(ConstantPropagation::Add(values[0], values[1]));
                case BooleanFunction::NodeType::Sub:
                    return OK(ConstantPropagation::Sub(values[0], values[1]));
                case BooleanFunction::NodeType::Mul:
                    return OK(ConstantPropagation::Mul(values[0], values[1]));

                case BooleanFunction::NodeType::Sdiv: {
                    // TODO implement
                    return ERR("could not propagate constants: not implemented for given node type");
                }
                case BooleanFunction::NodeType::Udiv: {
                    // TODO implement
                    return ERR("could not propagate constants: not implemented for given node type");
                }
                case BooleanFunction::NodeType::Srem: {
                    // TODO implement
                    return ERR("could not propagate constants: not implemented for given node type");
                }
                case BooleanFunction::NodeType::Urem: {
                    // TODO implement
                    return ERR("could not propagate constants: not implemented for given node type");
                }

                case BooleanFunction::NodeType::Concat: {
                    values[1].insert(values[1].end(), values[0].begin(), values[0].end());
                    return OK(BooleanFunction::Const(values[1]));
                }
                case BooleanFunction::NodeType::Slice: {
                    auto start = p[1].get_index_value().get();
                    auto end   = p[2].get_index_value().get();
                    return OK(BooleanFunction::Const(std::vector<BooleanFunction::Value>(values[0].begin() + start, values[0].begin() + end + 1)));
                }
                case BooleanFunction::NodeType::Zext: {
                    values[0].resize(node.size, BooleanFunction::Value::ZERO);
                    return OK(BooleanFunction::Const(values[0]));
                }
                case BooleanFunction::NodeType::Sext: {
                    values[0].resize(node.size, static_cast<BooleanFunction::Value>(values[0].back()));
                    return OK(BooleanFunction::Const(values[0]));
                }

                case BooleanFunction::NodeType::Shl:
                    return OK(ConstantPropagation::Shl(values[0], indices[0]));
                case BooleanFunction::NodeType::Lshr:
                    return OK(ConstantPropagation::Lshr(values[0], indices[0]));
                case BooleanFunction::NodeType::Ashr:
                    return OK(ConstantPropagation::Ashr(values[0], indices[0]));
                case BooleanFunction::NodeType::Rol:
                    return OK(ConstantPropagation::Rol(values[0], indices[0]));
                case BooleanFunction::NodeType::Ror:
                    return OK(ConstantPropagation::Ror(values[0], indices[0]));

                case BooleanFunction::NodeType::Eq:
                    return OK((values[0] == values[1]) ? BooleanFunction::Const(1, 1) : BooleanFunction::Const(0, 1));
                case BooleanFunction::NodeType::Sle:
                    return OK(ConstantPropagation::Sle(values[0], values[1]));
                case BooleanFunction::NodeType::Slt:
                    return OK(ConstantPropagation::Slt(values[0], values[1]));
                case BooleanFunction::NodeType::Ule:
                    return OK(ConstantPropagation::Ule(values[0], values[1]));
                case BooleanFunction::NodeType::Ult:
                    return OK(ConstantPropagation::Ult(values[0], values[1]));
                case BooleanFunction::NodeType::Ite:
                    return OK(ConstantPropagation::Ite(values[0], values[1], values[2]));

                default:
                    return ERR("could not propagate constants: not implemented for given node type");
            }
        }
    }    // namespace SMT
}    // namespace hal