#include "z3_utils/simplification.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/log.h"
#include "z3_utils/z3_utils.h"

#include <deque>

namespace hal
{
    namespace BV_ConstantPropagation
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

        /**
         * Applies constant propagation for the node operation and the constant parameters
         * 
         * @param[in] node - Boolean function node that specifies the operation type
         * @param[in] p - The parameters as a vector of constant Boolean functions
         * @return Constant Boolean fucntion as the result of the operation. 
         */
        Result<BooleanFunction> constant_propagation(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p)
        {
            std::vector<std::vector<BooleanFunction::Value>> values;
            for (const auto& parameter : p)
            {
                values.emplace_back(parameter.get_top_level_node().constant);
            }

            switch (node.type)
            {
                case BooleanFunction::NodeType::And:
                    return OK(And(values[0], values[1]));
                case BooleanFunction::NodeType::Or:
                    return OK(Or(values[0], values[1]));
                case BooleanFunction::NodeType::Not:
                    return OK(Not(values[0]));
                case BooleanFunction::NodeType::Xor:
                    return OK(Xor(values[0], values[1]));
                case BooleanFunction::NodeType::Add:
                    return OK(Add(values[0], values[1]));
                case BooleanFunction::NodeType::Sub:
                    return OK(Sub(values[0], values[1]));
                case BooleanFunction::NodeType::Mul:
                    return OK(Mul(values[0], values[1]));

                case BooleanFunction::NodeType::Sle:
                    // TODO implement for z3 where Boolean and bitvector sort exist
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Slt:
                    // TODO implement for z3 where Boolean and bitvector sort exist
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Ule:
                    // TODO implement for z3 where Boolean and bitvector sort exist
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Ult:
                    // TODO implement for z3 where Boolean and bitvector sort exist
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Ite:
                    // TODO implement for z3 where Boolean and bitvector sort exist
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Eq:
                    // TODO implement for z3 where Boolean and bitvector sort exist
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Zext:
                    // TODO implement for z3 where indices are not part of the parameters
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Sext:
                    // TODO implement for z3 where indices are not part of the parameters
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Slice:
                    // TODO implement for z3 where indices are not part of the parameters
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Concat:
                    // TODO implement for z3 where indices are not part of the parameters
                    return OK(BooleanFunction());
                case BooleanFunction::NodeType::Sdiv:
                    // TODO implement
                case BooleanFunction::NodeType::Udiv:
                    // TODO implement
                case BooleanFunction::NodeType::Srem:
                    // TODO implement
                case BooleanFunction::NodeType::Urem:
                    // TODO implement
                default:
                    return ERR("could not propagate constants: not implemented for given node type");
            }
        }

    }    // namespace BV_ConstantPropagation

    namespace
    {

        /**
         * Helper function that checks two expressions for syntactical equality.
         */
        bool is_x_y(const z3::expr& x, const z3::expr& y)
        {
            if (x.id() == y.id())
            {
                return true;
            }

            if (z3::eq(x, y))
            {
                return true;
            }

            return false;
        }

        /**
         * Helper function to check whether one of the two expressions is just the other function negated.
         */
        bool is_x_not_y(const z3::expr& x, const z3::expr& y)
        {
            const auto x_kind = x.decl().decl_kind();
            const auto y_kind = y.decl().decl_kind();

            if (x_kind == Z3_OP_BNOT || x_kind == Z3_OP_NOT)
            {
                if (is_x_y(x.arg(0), y))
                {
                    return true;
                }
            }
            if (y_kind == Z3_OP_BNOT || y_kind == Z3_OP_NOT)
            {
                if (is_x_y(y.arg(0), x))
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * Helper function to get the parameters of an expression formatted as a vector of expressions.
         */
        std::vector<z3::expr> get_parameters(const z3::expr& e)
        {
            std::vector<z3::expr> p;
            for (u32 i = 0; i < e.num_args(); i++)
            {
                p.push_back(e.arg(i));
            }

            return p;
        }

        /**
         * Helper function to check whether an expression is numeral and all bits are one.
         */
        bool is_ones(const z3::expr& e)
        {
            if (!e.is_numeral())
            {
                return false;
            }

            const std::string val_str = Z3_get_numeral_binary_string(e.ctx(), e);

            // Check if the binary representation consists only of '1's
            return val_str.find('0') == std::string::npos;
        }

        /**
         * Helper function to check whether an expression is numeral and all bits are zero.
         */
        bool is_zero(const z3::expr& e)
        {
            if (!e.is_numeral())
            {
                return false;
            }

            const std::string val_str = Z3_get_numeral_binary_string(e.ctx(), e);

            // Check if the binary representation consists only of '0's
            return val_str.find('1') == std::string::npos;
        }

        /**
         * Helper function to check whether an expression is numeral and is equal to a specific value.
         * Only works for numerals < 64 bit.
         */
        bool has_constant_value(const z3::expr& e, const u64& val)
        {
            if (!e.is_numeral())
            {
                return false;
            }

            if (e.get_sort().bv_size() > 64)
            {
                return false;
            }

            return e.get_numeral_uint64() == val;
        }

        /**
         * Helper function to check whether an expression is of a specific type.
         */
        bool is_kind(const z3::expr& e, const Z3_decl_kind& t)
        {
            return e.decl().decl_kind() == t;
        }

        /**
         * Helper function to determine whether an operation is commutative or not.
         */
        bool is_commutative(const Z3_decl_kind& t)
        {
            switch (t)
            {
                case Z3_OP_TRUE:
                case Z3_OP_FALSE:
                case Z3_OP_AND:
                case Z3_OP_OR:
                case Z3_OP_NOT:
                case Z3_OP_BAND:
                case Z3_OP_BNOT:
                case Z3_OP_BOR:
                case Z3_OP_BXOR:
                case Z3_OP_BADD:
                    return true;

                case Z3_OP_BNEG:
                case Z3_OP_BSUB:
                    return false;

                case Z3_OP_BMUL:
                    return true;

                case Z3_OP_BSDIV:
                case Z3_OP_BUDIV:
                case Z3_OP_BSREM:
                case Z3_OP_BUREM:
                    return false;

                case Z3_OP_EXTRACT:
                    return true;

                case Z3_OP_CONCAT:
                    return false;

                case Z3_OP_ZERO_EXT:
                case Z3_OP_SIGN_EXT:
                case Z3_OP_EQ:
                    return true;

                case Z3_OP_SLEQ:
                case Z3_OP_SLT:
                case Z3_OP_ULEQ:
                case Z3_OP_ULT:
                case Z3_OP_ITE:
                    return false;

                default: {
                    log_error("z3_utils", "commutative check not implemeted for type {}!", t);
                    return false;
                }
            }
        }

        /**
         * Helper function to normalize a vector of parameters by sorting them based on their declaration. 
         */
        std::vector<z3::expr> normalize(std::vector<z3::expr>&& p)
        {
            if (p.size() <= 1ul)
            {
                return std::move(p);
            }

            std::sort(p.begin(), p.end(), [](const auto& lhs, const auto& rhs) { return lhs.decl().decl_kind() > rhs.decl().decl_kind(); });
            return std::move(p);
        }

        /**
         * Helper function to check whether the orignal version of an expression and its simplification are still semantically equivalent 
         */
        bool check_simplification_for_correctness(const z3::expr& org, const z3::expr& smp)
        {
            log_warning("z3_utils", "Checking simplification for correctness. This is slow and expensive!");

            z3::solver s(org.ctx());
            s.add(org != smp);
            const auto r = s.check();

            if (r != z3::unsat)
            {
                std::cout << "Correctness Check failed!!!" << std::endl;
                std::cout << "ORG: " << org << std::endl;
                std::cout << "NEW: " << smp << std::endl;

                return false;
            }

            return true;
        }

        /**
         * Helper function to simplfy two expressions that are conactinated together
         */
        std::vector<z3::expr> simplify_concat(z3::context& ctx, const z3::expr& p0, const z3::expr& p1)
        {
            const u64 p0_size = p0.get_sort().bv_size();
            const u64 p1_size = p1.get_sort().bv_size();

            // TODO make this able to handle more than 64 bits
            // CONCAT(CONST(X), CONST(Y)) => CONST(X || Y)
            if (p0.is_numeral() && p1.is_numeral())
            {
                if ((p0_size + p1_size) <= 64)
                {
                    return {ctx.bv_val((p1.get_numeral_uint64() << p0_size) + p0.get_numeral_uint64(), p0_size + p1_size)};
                }
            }

            if (is_kind(p0, Z3_OP_EXTRACT) && is_kind(p1, Z3_OP_EXTRACT))
            {
                auto p0_parameter = get_parameters(p0);
                auto p1_parameter = get_parameters(p1);

                if (is_x_y(p0_parameter[0], p1_parameter[0]))
                {
                    // CONCAT(SLICE(X, j+1, k), SLICE(X, i, j)) => SLICE(X, i, k)
                    if (p1.lo() == (p0.hi() + 1))
                    {
                        const auto res = p0_parameter[0].extract(p1.hi(), p0.lo());
                        return {res};
                    }

                    // CONCAT(SLICE(X, j, j), SLICE(X, j, i)) => SEXT(SLICE(X, j, i), 1)
                    if ((p1.lo() == p1.hi()) && (p1.lo() == p0.hi()))
                    {
                        const auto res = z3::expr(ctx, Z3_mk_sign_ext(ctx, 1, p0));
                        return {res};
                    }
                }
            }

            if (is_kind(p0, Z3_OP_EXTRACT) && p1.is_numeral())
            {
                // CONCAT(00..00, SLICE(X, i, j)) => ZEXT(SLICE(X, i, k), n)
                if (is_zero(p1))
                {
                    const auto res = z3::expr(ctx, Z3_mk_zero_ext(ctx, p1_size, p0));
                    return {res};
                }
            }

            if (is_kind(p0, Z3_OP_SIGN_EXT) && is_kind(p1, Z3_OP_EXTRACT))
            {
                auto p0_parameter  = get_parameters(p0);
                auto p1_parameter  = get_parameters(p1);
                auto p00_parameter = get_parameters(p0_parameter[0]);

                if (is_x_y(p00_parameter[0], p1_parameter[0]))
                {
                    // CONCAT(SLICE(X, j, j), SEXT(SLICE(X, j, i), n)) => SEXT(SLICE(X, j, i), n + 1)
                    if ((p1.lo() == p1.hi()) && (p1.lo() == p0_parameter[0].hi()))
                    {
                        const u32 extend_by = p0.get_sort().bv_size() - p0_parameter[0].get_sort().bv_size() + 1;
                        auto res            = z3::expr(ctx, Z3_mk_sign_ext(ctx, extend_by, p0_parameter[0]));
                        return {res};
                    }
                }
            }

            if (is_kind(p0, Z3_OP_ZERO_EXT) && p1.is_numeral())
            {
                auto p0_parameter = get_parameters(p0);

                // CONCAT(00..00, ZEXT(SLICE(X, j, i), n)) => ZEXT(SLICE(X, j, i), n + m))
                if (is_zero(p1))
                {
                    const u32 extend_by = p0.get_sort().bv_size() - p0_parameter[0].get_sort().bv_size() + p1_size;
                    const auto res      = z3::expr(ctx, Z3_mk_zero_ext(ctx, extend_by, p0_parameter[0]));

                    return {res};
                }
            }

            return {p0, p1};
        }

        /** 
         * Internal simplification function that is called recursively on the expression and its parameters to simplfy teh whole AST of the expression.
        */
        Result<z3::expr> simplify_internal(const z3::expr& e, std::unordered_map<u32, z3::expr>& cache, const bool check_correctness)
        {
            if (const auto it = cache.find(e.id()); it != cache.end())
            {
                return OK(it->second);
            }

            u64 size;
            if (e.is_bv())
            {
                if (e.is_numeral())
                {
                    return OK(e);
                }
                else if (e.is_const())
                {
                    return OK(e);
                }
                else if (e.is_var())
                {
                    return OK(e);
                }
                size = e.get_sort().bv_size();
            }

            auto& ctx = e.ctx();

            // 1. Simplify all parameters of the function recursively
            const auto op = e.decl().decl_kind();
            std::vector<z3::expr> p;
            for (u32 i = 0; i < e.num_args(); i++)
            {
                const auto arg = e.arg(i);

                const auto res = simplify_internal(arg, cache, check_correctness);

                if (res.is_ok())
                {
                    if (check_correctness && !check_simplification_for_correctness(arg, res.get()))
                    {
                        return ERR("simplification failed correctness check!");
                    }

                    const auto [it, _] = cache.insert({arg.id(), res.get()});
                    p.push_back(it->second);
                }
                else
                {
                    return ERR(res.get_error());
                }
            }

            // 2. If all simplified parameters are constants apply constant propagation
            if (!p.empty() && std::all_of(p.begin(), p.end(), [](const auto& p_e) { return p_e.is_numeral() && p_e.is_bv(); }))
            {
                const auto bf = z3_utils::to_bf(e).get();

                std::vector<BooleanFunction> p_bf;
                for (const auto& p_e : p)
                {
                    const auto t = z3_utils::to_bf(p_e).get();
                    p_bf.push_back(t);
                }

                auto res = BV_ConstantPropagation::constant_propagation(bf.get_top_level_node(), std::move(p_bf));
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not simplify sub-expression in abstract syntax tree: constant propagation failed");
                }

                auto bf_c = res.get();
                if (!bf_c.is_empty())
                {
                    const auto bf_z3 = z3_utils::from_bf(bf_c, ctx);
                    return OK(bf_z3);
                }
            }

            // 3. If the operation is commutative normalize the order of operands
            if (is_commutative(op))
            {
                p = normalize(std::move(p));
            }

            // 4. Apply various simplifcation rules based on the operation
            switch (op)
            {
                case Z3_OP_TRUE: {
                    return OK(e);
                }

                case Z3_OP_FALSE: {
                    return OK(e);
                }

                case Z3_OP_AND: {
                    // X & False   =>   False
                    if (p[1].is_bool() && p[1].is_false())
                    {
                        return OK(ctx.bool_val(false));
                    }
                    // X & True  =>   X
                    if (p[1].is_bool() && p[1].is_true())
                    {
                        return OK(p[0]);
                    }
                    // X & X   =>   X
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(p[0]);
                    }
                    // X & ~X   =>   False
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(ctx.bool_val(false));
                    }

                    if (p[0].is_or() && p[1].is_or())
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        auto p1_parameter = get_parameters(p[1]);

                        // (X | Y) & (X | Z)   =>   X | (Y & Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            return OK(p0_parameter[0] | (p0_parameter[1] & p1_parameter[1]));
                        }
                        // (X | Y) & (Z | X)   =>   X | (Y & Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[1]))
                        {
                            return OK(p0_parameter[0] | (p0_parameter[1] & p1_parameter[0]));
                        }

                        // (X | Y) & (Y | Z)   =>   Y | (X & Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[0]))
                        {
                            return OK(p0_parameter[1] | (p0_parameter[0] & p1_parameter[1]));
                        }
                        // (X | Y) & (Z | Y)   =>   Y | (X & Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[1]))
                        {
                            return OK(p0_parameter[1] | (p0_parameter[0] & p1_parameter[0]));
                        }
                    }

                    if (p[1].is_and())
                    {
                        auto p1_parameter = get_parameters(p[1]);
                        // X & (X & Y)   =>   (X & Y)
                        if (is_x_y(p[0], p1_parameter[1]))
                        {
                            return OK(p[1]);
                        }
                        // X & (Y & X)   =>   (Y & X)
                        if (is_x_y(p[0], p1_parameter[0]))
                        {
                            return OK(p[1]);
                        }

                        // X & (~X & Y)   =>   0
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(ctx.bool_val(false));
                        }
                        // X & (Y & ~X)   =>   0
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(ctx.bool_val(false));
                        }
                    }

                    if (p[1].is_or())
                    {
                        auto p1_parameter = get_parameters(p[1]);

                        // X & (X | Y)   =>   X
                        if (is_x_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[0]);
                        }
                        // X & (Y | X)   =>   X
                        if (is_x_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[0]);
                        }
                        // X & (~X | Y)   =>  X & Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[0] & p1_parameter[1]);
                        }
                        // X & (Y | ~X)   =>  X & Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[0] & p1_parameter[0]);
                        }
                    }

                    if (p[0].is_and())
                    {
                        auto p0_parameter = get_parameters(p[0]);

                        // (X & Y) & X   =>    X & Y
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[0]);
                        }

                        // (Y & X) & X   =>    Y & X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[0]);
                        }
                        // (~X & Y) & X   =>   0
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(ctx.bool_val(false));
                        }
                        // (Y & ~X) & X   =>   0
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(ctx.bool_val(false));
                        }
                    }

                    if (p[0].is_or())
                    {
                        auto p0_parameter = get_parameters(p[0]);

                        // (X | Y) & X    =>   X
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (Y | X) & X    =>   X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (~X | Y) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[1] & p0_parameter[1]);
                        }
                        // (Y | ~X) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[1] & p0_parameter[0]);
                        }
                    }

                    return OK(p[0] & p[1]);
                }

                case Z3_OP_OR: {
                    // X | False   =>   X
                    if (p[1].is_false())
                    {
                        return OK(p[0]);
                    }

                    // X | True   =>   True
                    if (p[1].is_true())
                    {
                        return OK(p[1]);
                    }

                    // X | X   =>   X
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(p[0]);
                    }

                    // X | ~X   =>  True
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(ctx.bool_val(true));
                    }

                    if (p[0].is_and() && p[1].is_and())
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        auto p1_parameter = get_parameters(p[1]);

                        // (X & Y) | (X & Z)    => X & (Y | Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            return OK(p0_parameter[0] & (p0_parameter[1] | p1_parameter[1]));
                        }
                        // (X & Y) | (Z & X)    => X & (Y | Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[1]))
                        {
                            return OK(p0_parameter[0] & (p0_parameter[1] | p1_parameter[0]));
                        }
                        // (X & Y) | (Y & Z)    => Y & (Y | Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[0]))
                        {
                            return OK(p0_parameter[1] & (p0_parameter[0] | p1_parameter[1]));
                        }
                        // (X & Y) | (Z & Y)    => Y & (X | Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[1]))
                        {
                            return OK(p0_parameter[1] & (p0_parameter[0] | p1_parameter[0]));
                        }
                    }

                    if (p[1].is_and())
                    {
                        auto p1_parameter = get_parameters(p[1]);
                        // X | (Y & !X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[0] | p1_parameter[0]);
                        }

                        // X | (X & Y)    =>   X
                        if ((is_x_y(p1_parameter[0], p[0])) || (is_x_y(p1_parameter[1], p[0])))
                        {
                            return OK(p[0]);
                        }

                        // X | (~X & Y)   =>   X | Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[0] | p1_parameter[1]);
                        }
                        // X | (Y & ~X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[0] | p1_parameter[0]);
                        }
                    }

                    if (p[1].is_or())
                    {
                        auto p1_parameter = get_parameters(p[1]);

                        // X | (X | Y)   =>   (X | Y)
                        if (is_x_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[1]);
                        }
                        // X | (Y | X)   =>   (Y | X)
                        if (is_x_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[1]);
                        }

                        // X | (~X | Y)   =>   True
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(ctx.bool_val(true));
                        }

                        // X | (Y | ~X)   =>   True
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(ctx.bool_val(true));
                        }
                    }

                    if (p[0].is_or())
                    {
                        auto p0_parameter = get_parameters(p[0]);

                        // (X | Y) | X   =>   (X | Y)
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[0]);
                        }
                        // (Y | X) | X   =>   (Y | X)
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[0]);
                        }

                        // (~X | Y) | X   =>   True
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(ctx.bool_val(true));
                        }

                        // (Y | ~X) | X     =>  True
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(ctx.bool_val(true));
                        }
                    }

                    if (p[0].is_and())
                    {
                        auto p0_parameter = get_parameters(p[0]);

                        // (X & Y) | X    =>   X
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (Y & X) | X    =>   X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[1]);
                        }

                        // (~X & Y) | X   =>   X | Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(p0_parameter[1] | p[1]);
                        }

                        // (X & ~Y) | Y   =>   X | Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(p0_parameter[0] | p[1]);
                        }
                    }

                    if (p[0].is_eq() && p[1].is_eq())
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        auto p1_parameter = get_parameters(p[1]);

                        // eq(X, 1) | eq(X, 0)  => True
                        if (p0_parameter[0].get_sort().is_bv() && is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            if (p0_parameter[0].get_sort().bv_size() == 1)
                            {
                                if (is_ones(p0_parameter[1]) && is_zero(p1_parameter[1]))
                                {
                                    return OK(ctx.bool_val(true));
                                }

                                if (is_zero(p0_parameter[1]) && is_ones(p1_parameter[1]))
                                {
                                    return OK(ctx.bool_val(true));
                                }
                            }
                        }
                    }

                    return OK(p[0] | p[1]);
                }

                case Z3_OP_NOT: {
                    // ~~X   =>   X
                    if (p[0].is_not())
                    {
                        return OK(get_parameters(p[0])[0]);
                    }

                    // ~(~X & ~Y)   =>   X | Y
                    if (is_kind(p[0], Z3_OP_BAND))
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        if (p0_parameter[0].is_not() && p0_parameter[1].is_not())
                        {
                            return OK(get_parameters(p0_parameter[0])[0] | get_parameters(p0_parameter[1])[0]);
                        }
                    }

                    // ~(~X | ~Y)   =>   X & Y
                    if (is_kind(p[0], Z3_OP_BOR))
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        if (p0_parameter[0].is_not() && p0_parameter[1].is_not())
                        {
                            return OK(get_parameters(p0_parameter[0])[0] & get_parameters(p0_parameter[1])[0]);
                        }
                    }

                    // ~(X | Y)   =>   ~X & ~Y
                    if (p[0].is_or())
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        return OK((~p0_parameter[0]) & (~p0_parameter[1]));
                    }

                    // ~(X & Y)   =>   ~X | ~Y
                    if (p[0].is_and())
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        return OK((~p0_parameter[0]) | (~p0_parameter[1]));
                    }

                    return OK(~p[0]);
                }

                case Z3_OP_BAND: {
                    if (p.size() == 2)
                    {
                        // X & 0   =>   0
                        if (is_zero(p[1]))
                        {
                            return OK(ctx.bv_val(0, size));
                        }
                        // X & 1  =>   X
                        if (is_ones(p[1]))
                        {
                            return OK(p[0]);
                        }
                        // X & X   =>   X
                        if (is_x_y(p[0], p[1]))
                        {
                            return OK(p[0]);
                        }
                        // X & ~X   =>   0
                        if (is_x_not_y(p[0], p[1]))
                        {
                            return OK(ctx.bv_val(0, size));
                        }

                        if (is_kind(p[0], Z3_OP_BOR) && is_kind(p[1], Z3_OP_BOR))
                        {
                            auto p0_parameter = get_parameters(p[0]);
                            auto p1_parameter = get_parameters(p[1]);

                            // (X | Y) & (X | Z)   =>   X | (Y & Z)
                            if (is_x_y(p0_parameter[0], p1_parameter[0]))
                            {
                                return OK(p0_parameter[0] | (p0_parameter[1] & p1_parameter[1]));
                            }
                            // (X | Y) & (Z | X)   =>   X | (Y & Z)
                            if (is_x_y(p0_parameter[0], p1_parameter[1]))
                            {
                                return OK(p0_parameter[0] | (p0_parameter[1] & p1_parameter[0]));
                            }

                            // (X | Y) & (Y | Z)   =>   Y | (X & Z)
                            if (is_x_y(p0_parameter[1], p1_parameter[0]))
                            {
                                return OK(p0_parameter[1] | (p0_parameter[0] & p1_parameter[1]));
                            }
                            // (X | Y) & (Z | Y)   =>   Y | (X & Z)
                            if (is_x_y(p0_parameter[1], p1_parameter[1]))
                            {
                                return OK(p0_parameter[1] | (p0_parameter[0] & p1_parameter[0]));
                            }
                        }

                        if (is_kind(p[1], Z3_OP_BAND))
                        {
                            auto p1_parameter = get_parameters(p[1]);
                            // X & (X & Y)   =>   (X & Y)
                            if (is_x_y(p[0], p1_parameter[1]))
                            {
                                return OK(p[1]);
                            }
                            // X & (Y & X)   =>   (Y & X)
                            if (is_x_y(p[0], p1_parameter[0]))
                            {
                                return OK(p[1]);
                            }

                            // X & (~X & Y)   =>   0
                            if (is_x_not_y(p1_parameter[0], p[0]))
                            {
                                return OK(ctx.bv_val(0, size));
                            }
                            // X & (Y & ~X)   =>   0
                            if (is_x_not_y(p1_parameter[1], p[0]))
                            {
                                return OK(ctx.bv_val(0, size));
                            }
                        }

                        if (is_kind(p[1], Z3_OP_BOR))
                        {
                            auto p1_parameter = get_parameters(p[1]);

                            // X & (X | Y)   =>   X
                            if (is_x_y(p1_parameter[0], p[0]))
                            {
                                return OK(p[0]);
                            }
                            // X & (Y | X)   =>   X
                            if (is_x_y(p1_parameter[1], p[0]))
                            {
                                return OK(p[0]);
                            }
                            // X & (~X | Y)   =>  X & Y
                            if (is_x_not_y(p1_parameter[0], p[0]))
                            {
                                return OK(p[0] & p1_parameter[1]);
                            }
                            // X & (Y | ~X)   =>  X & Y
                            if (is_x_not_y(p1_parameter[1], p[0]))
                            {
                                return OK(p[0] & p1_parameter[0]);
                            }
                        }

                        if (is_kind(p[0], Z3_OP_BAND))
                        {
                            auto p0_parameter = get_parameters(p[0]);

                            // (X & Y) & X   =>    X & Y
                            if (is_x_y(p0_parameter[0], p[1]))
                            {
                                return OK(p[0]);
                            }

                            // (Y & X) & X   =>    Y & X
                            if (is_x_y(p0_parameter[1], p[1]))
                            {
                                return OK(p[0]);
                            }
                            // (~X & Y) & X   =>   0
                            if (is_x_not_y(p0_parameter[0], p[1]))
                            {
                                return OK(ctx.bv_val(0, size));
                            }
                            // (Y & ~X) & X   =>   0
                            if (is_x_not_y(p0_parameter[1], p[1]))
                            {
                                return OK(ctx.bv_val(0, size));
                            }
                        }

                        if (is_kind(p[0], Z3_OP_BOR))
                        {
                            auto p0_parameter = get_parameters(p[0]);

                            // (X | Y) & X    =>   X
                            if (is_x_y(p0_parameter[0], p[1]))
                            {
                                return OK(p[1]);
                            }
                            // (Y | X) & X    =>   X
                            if (is_x_y(p0_parameter[1], p[1]))
                            {
                                return OK(p[1]);
                            }
                            // (~X | Y) & X   =>   X & Y
                            if (is_x_not_y(p0_parameter[0], p[1]))
                            {
                                return OK(p[1] & p0_parameter[1]);
                            }
                            // (Y | ~X) & X   =>   X & Y
                            if (is_x_not_y(p0_parameter[1], p[1]))
                            {
                                return OK(p[1] & p0_parameter[0]);
                            }
                        }

                        return OK(p[0] & p[1]);
                    }

                    // TODO simplify with more than two parameters
                    z3::expr res = p[0] & p[1];
                    for (u32 i = 2; i < p.size(); i++)
                    {
                        res = res & p[i];
                    }
                    return OK(res);
                }

                case Z3_OP_BNOT: {
                    // ~~X   =>   X
                    if (is_kind(p[0], Z3_OP_BNOT))
                    {
                        return OK(get_parameters(p[0])[0]);
                    }

                    // ~(~X & ~Y)   =>   X | Y
                    if (is_kind(p[0], Z3_OP_BAND))
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        if (is_kind(p0_parameter[0], Z3_OP_BNOT) && is_kind(p0_parameter[1], Z3_OP_BNOT))
                        {
                            return OK(get_parameters(p0_parameter[0])[0] | get_parameters(p0_parameter[1])[0]);
                        }
                    }

                    // ~(~X | ~Y)   =>   X & Y
                    if (is_kind(p[0], Z3_OP_BOR))
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        if (is_kind(p0_parameter[0], Z3_OP_BNOT) && is_kind(p0_parameter[1], Z3_OP_BNOT))
                        {
                            return OK(get_parameters(p0_parameter[0])[0] & get_parameters(p0_parameter[1])[0]);
                        }
                    }

                    // ~(X | Y)   =>   ~X & ~Y
                    if (is_kind(p[0], Z3_OP_BOR))
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        return OK((~p0_parameter[0]) & (~p0_parameter[1]));
                    }

                    // ~(X & Y)   =>   ~X | ~Y
                    if (is_kind(p[0], Z3_OP_BAND))
                    {
                        auto p0_parameter = get_parameters(p[0]);
                        return OK((~p0_parameter[0]) | (~p0_parameter[1]));
                    }

                    return OK(~p[0]);
                }

                case Z3_OP_BOR: {
                    if (p.size() == 2)
                    {
                        // X | 0   =>   X
                        if (is_zero(p[1]))
                        {
                            return OK(p[0]);
                        }

                        // X | 1   =>   1
                        if (is_ones(p[1]))
                        {
                            return OK(p[1]);
                        }

                        // X | X   =>   X
                        if (is_x_y(p[0], p[1]))
                        {
                            return OK(p[0]);
                        }

                        // X | ~X   =>   1
                        if (is_x_not_y(p[0], p[1]))
                        {
                            return OK(ctx.bv_val(-1, size));
                        }

                        if (is_kind(p[0], Z3_OP_BAND) && is_kind(p[1], Z3_OP_BAND))
                        {
                            auto p0_parameter = get_parameters(p[0]);
                            auto p1_parameter = get_parameters(p[1]);

                            // (X & Y) | (X & Z)    => X & (Y | Z)
                            if (is_x_y(p0_parameter[0], p1_parameter[0]))
                            {
                                return OK(p0_parameter[0] & (p0_parameter[1] | p1_parameter[1]));
                            }
                            // (X & Y) | (Z & X)    => X & (Y | Z)
                            if (is_x_y(p0_parameter[0], p1_parameter[1]))
                            {
                                return OK(p0_parameter[0] & (p0_parameter[1] | p1_parameter[0]));
                            }
                            // (X & Y) | (Y & Z)    => Y & (Y | Z)
                            if (is_x_y(p0_parameter[1], p1_parameter[0]))
                            {
                                return OK(p0_parameter[1] & (p0_parameter[0] | p1_parameter[1]));
                            }
                            // (X & Y) | (Z & Y)    => Y & (X | Z)
                            if (is_x_y(p0_parameter[1], p1_parameter[1]))
                            {
                                return OK(p0_parameter[1] & (p0_parameter[0] | p1_parameter[0]));
                            }
                        }

                        if (is_kind(p[1], Z3_OP_BAND))
                        {
                            auto p1_parameter = get_parameters(p[1]);
                            // X | (Y & !X)   =>   X | Y
                            if (is_x_not_y(p1_parameter[1], p[0]))
                            {
                                return OK(p[0] | p1_parameter[0]);
                            }

                            // X | (X & Y)    =>   X
                            if ((is_x_y(p1_parameter[0], p[0])) || (is_x_y(p1_parameter[1], p[0])))
                            {
                                return OK(p[0]);
                            }

                            // X | (~X & Y)   =>   X | Y
                            if (is_x_not_y(p1_parameter[0], p[0]))
                            {
                                return OK(p[0] | p1_parameter[1]);
                            }
                            // X | (Y & ~X)   =>   X | Y
                            if (is_x_not_y(p1_parameter[1], p[0]))
                            {
                                return OK(p[0] | p1_parameter[0]);
                            }
                        }

                        if (is_kind(p[1], Z3_OP_BOR))
                        {
                            auto p1_parameter = get_parameters(p[1]);

                            // X | (X | Y)   =>   (X | Y)
                            if (is_x_y(p1_parameter[0], p[0]))
                            {
                                return OK(p[1]);
                            }
                            // X | (Y | X)   =>   (Y | X)
                            if (is_x_y(p1_parameter[1], p[0]))
                            {
                                return OK(p[1]);
                            }

                            // X | (~X | Y)   =>   1
                            if (is_x_not_y(p1_parameter[0], p[0]))
                            {
                                return OK(ctx.bv_val(-1, size));
                            }

                            // X | (Y | ~X)   =>   1
                            if (is_x_not_y(p1_parameter[1], p[0]))
                            {
                                return OK(ctx.bv_val(-1, size));
                            }
                        }

                        if (is_kind(p[0], Z3_OP_BOR))
                        {
                            auto p0_parameter = get_parameters(p[0]);

                            // (X | Y) | X   =>   (X | Y)
                            if (is_x_y(p0_parameter[0], p[1]))
                            {
                                return OK(p[0]);
                            }
                            // (Y | X) | X   =>   (Y | X)
                            if (is_x_y(p0_parameter[1], p[1]))
                            {
                                return OK(p[0]);
                            }

                            // (~X | Y) | X   =>   1
                            if (is_x_not_y(p0_parameter[0], p[1]))
                            {
                                return OK(ctx.bv_val(-1, size));
                            }

                            // (Y | ~X) | X =>   1
                            if (is_x_not_y(p0_parameter[1], p[1]))
                            {
                                return OK(ctx.bv_val(-1, size));
                            }
                        }

                        if (is_kind(p[0], Z3_OP_BAND))
                        {
                            auto p0_parameter = get_parameters(p[0]);

                            // (X & Y) | X    =>   X
                            if (is_x_y(p0_parameter[0], p[1]))
                            {
                                return OK(p[1]);
                            }
                            // (Y & X) | X    =>   X
                            if (is_x_y(p0_parameter[1], p[1]))
                            {
                                return OK(p[1]);
                            }

                            // (~X & Y) | X   =>   X | Y
                            if (is_x_not_y(p0_parameter[0], p[1]))
                            {
                                return OK(p0_parameter[1] | p[1]);
                            }

                            // (X & ~Y) | Y   =>   X | Y
                            if (is_x_not_y(p0_parameter[1], p[1]))
                            {
                                return OK(p0_parameter[0] | p[1]);
                            }
                        }

                        return OK(p[0] | p[1]);
                    }

                    // TODO simplify with more than two parameters
                    z3::expr res = p[0] | p[1];
                    for (u32 i = 2; i < p.size(); i++)
                    {
                        res = res | p[i];
                    }
                    return OK(res);
                }

                case Z3_OP_BXOR: {
                    if (p.size() == 2)
                    {
                        // X ^ 0   =>   X
                        if (is_zero(p[1]))
                        {
                            return OK(p[0]);
                        }
                        // X ^ 1  =>   ~X
                        if (is_ones(p[1]))
                        {
                            return OK(~p[0]);
                        }
                        // X ^ X   =>   0
                        if (is_x_y(p[0], p[1]))
                        {
                            return OK(ctx.bv_val(0, size));
                        }
                        // X ^ ~X   =>   1
                        if (is_x_not_y(p[0], p[1]))
                        {
                            return OK(ctx.bv_val(-1, size));
                        }

                        return OK(p[0] ^ p[1]);
                    }

                    // TODO simplify with more than two parameters
                    z3::expr res = p[0] ^ p[1];
                    for (u32 i = 2; i < p.size(); i++)
                    {
                        res = res ^ p[i];
                    }
                    return OK(res);
                }

                case Z3_OP_BNEG: {
                    // --X   =>   X
                    if (is_kind(p[0], Z3_OP_BNEG))
                    {
                        return OK(get_parameters(p[0])[0]);
                    }

                    return OK(-p[0]);
                }

                case Z3_OP_BADD: {
                    if (p.size() == 2)
                    {
                        // X + 0    =>   X
                        if (is_zero(p[1]))
                        {
                            return OK(p[0]);
                        }

                        const u64 p0_size = p[0].get_sort().bv_size();
                        const u64 p1_size = p[1].get_sort().bv_size();

                        if (is_kind(p[0], Z3_OP_BNEG))
                        {
                            const auto p0_parameter = get_parameters(p[0]);

                            return OK(p[1] - p0_parameter[0]);
                        }

                        if (is_kind(p[1], Z3_OP_BNEG))
                        {
                            const auto p1_parameter = get_parameters(p[1]);

                            return OK(p[0] - p1_parameter[0]);
                        }

                        // SLICE(X, 0, 0) + SLICE(Y, 0, 0) => SLICE(X + Y, 0, 0)
                        if ((is_kind(p[0], Z3_OP_EXTRACT)) && is_kind(p[1], Z3_OP_EXTRACT))
                        {
                            if ((p[0].lo() == 0) && (p[0].hi() == 0) && (p[1].lo() == 0) && (p[1].hi() == 0))
                            {
                                auto p0_parameter = get_parameters(p[0]);
                                auto p1_parameter = get_parameters(p[1]);

                                return OK((p0_parameter[0] + p1_parameter[0]).extract(0, 0));
                            }
                        }

                        return OK(p[0] + p[1]);
                    }

                    // TODO simplify SUM with more than two parameters
                    z3::expr sum = p[0] + p[1];
                    for (u32 i = 2; i < p.size(); i++)
                    {
                        sum = sum + p[i];
                    }
                    return OK(sum);
                }

                case Z3_OP_BSUB: {
                    if (p.size() == 2)
                    {
                        // X - 0    =>   X
                        if (is_zero(p[1]))
                        {
                            return OK(p[0]);
                        }
                        // X - X    =>   0
                        if (is_x_y(p[0], p[1]))
                        {
                            return OK(ctx.bv_val(0, size));
                        }

                        // X - (-Y) => X + Y
                        if (is_kind(p[1], Z3_OP_BNEG))
                        {
                            const auto p1_parameter = get_parameters(p[1]);

                            return OK(p[0] + p1_parameter[0]);
                        }

                        return OK(p[0] - p[1]);
                    }

                    // TODO implement for more than two parameters
                    z3::expr sum = p[0] - p[1];
                    for (u32 i = 2; i < p.size(); i++)
                    {
                        sum = sum - p[i];
                    }
                    return OK(sum);
                }

                case Z3_OP_BMUL: {
                    // X * 0    =>   0
                    if (is_zero(p[1]))
                    {
                        return OK(ctx.bv_val(0, size));
                    }
                    // X * 1    =>   X
                    if (has_constant_value(p[1], 1))
                    {
                        return OK(p[0]);
                    }

                    // This currently leads to problems, since the HAL boolean function can not handle this, so translation causes errors
                    // // X * -1   =>  -X
                    // if (is_ones(p[1]))
                    // {
                    //     return OK(-p[0]);
                    // }

                    return OK(p[0] * p[1]);
                }

                case Z3_OP_BSDIV: {
                    // X /s 1    =>   X
                    if (has_constant_value(p[1], 1))
                    {
                        return OK(p[0]);
                    }
                    // X /s X    =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bv_val(1, size));
                    }

                    return OK(p[0] / p[1]);
                }

                case Z3_OP_BUDIV: {
                    // X / 1    =>   X
                    if (has_constant_value(p[1], 1))
                    {
                        return OK(p[0]);
                    }
                    // X / X    =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bv_val(1, size));
                    }

                    return OK(z3::expr(ctx, Z3_mk_bvudiv(ctx, p[0], p[1])));
                }

                case Z3_OP_BSREM: {
                    // X %s 1    =>   0
                    if (has_constant_value(p[1], 1))
                    {
                        return OK(ctx.bv_val(0, size));
                    }
                    // X %s X    =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bv_val(0, size));
                    }

                    return OK(z3::expr(ctx, Z3_mk_bvsrem(ctx, p[0], p[1])));
                }

                case Z3_OP_BUREM: {
                    // X % 1    =>   0
                    if (has_constant_value(p[1], 1))
                    {
                        return OK(ctx.bv_val(0, size));
                    }
                    // X % X    =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bv_val(0, size));
                    }

                    return OK(z3::expr(ctx, Z3_mk_bvurem(ctx, p[0], p[1])));
                }

                case Z3_OP_EXTRACT: {
                    // SLICE(p, 0, 0)   =>   p (if p is 1-bit wide)
                    if ((e.lo() == 0) && (e.hi() == 0) && (p[0].get_sort().bv_size() == 1))
                    {
                        return OK(p[0]);
                    }

                    // SLICE(p, n, 0)   =>  p (if size of p is n+1)
                    if (((e.hi() - e.lo()) == (p[0].get_sort().bv_size() - 1)) && (e.lo() == 0))
                    {
                        return OK(p[0]);
                    }

                    // SLICE(011..010, i, j) => 010..001
                    if (p[0].is_numeral())
                    {
                        const std::string p0_str = Z3_get_numeral_binary_string(e.ctx(), p[0]);
                        const std::string p0_pad = std::string(p[0].get_sort().bv_size() - p0_str.length(), '0') + p0_str;

                        // reverse string to place bit 0 at index 0 of string
                        std::string p0_rev = p0_pad;
                        std::reverse(p0_rev.begin(), p0_rev.end());

                        const std::string ex_str = p0_rev.substr(e.lo(), e.hi() - e.lo() + 1);

                        const auto res = z3_utils::value_from_binary_string(ctx, ex_str);

                        return res;
                    }

                    return OK(p[0].extract(e.hi(), e.lo()));
                }

                case Z3_OP_CONCAT: {
                    std::vector<z3::expr> q = {p.begin(), p.end()};
                    std::vector<z3::expr> res;

                    while (q.size() > 1)
                    {
                        const auto p0 = q.back();
                        q.pop_back();

                        const auto p1 = q.back();
                        q.pop_back();

                        const auto sc = simplify_concat(ctx, p0, p1);

                        if (sc.size() == 1)
                        {
                            q.push_back(sc.front());
                        }
                        else
                        {
                            res.insert(res.begin(), sc.back());
                            q.push_back(sc.front());
                        }
                    }

                    res.push_back(q.front());

                    if (res.size() > 1)
                    {
                        z3::expr_vector res_e(ctx);
                        for (const auto& e : res)
                        {
                            res_e.push_back(e);
                        }

                        return OK(z3::concat(res_e));
                    }

                    return OK(res.front());
                }

                case Z3_OP_ZERO_EXT: {
                    const u64 i = e.get_sort().bv_size() - p[0].get_sort().bv_size();
                    return OK(z3::expr(ctx, Z3_mk_zero_ext(ctx, i, p[0])));
                }

                case Z3_OP_SIGN_EXT: {
                    const u64 i = e.get_sort().bv_size() - p[0].get_sort().bv_size();
                    return OK(z3::expr(ctx, Z3_mk_sign_ext(ctx, i, p[0])));
                }

                case Z3_OP_EQ: {
                    // X == X   =>  true
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bool_val(true));
                    }

                    // 010..010 == 101..101   => false
                    if (p[0].is_numeral() && p[1].is_numeral())
                    {
                        const std::string p0_str = Z3_get_numeral_binary_string(e.ctx(), p[0]);
                        const std::string p1_str = Z3_get_numeral_binary_string(e.ctx(), p[1]);

                        if (p0_str != p1_str)
                        {
                            return OK(ctx.bool_val(false));
                        }
                    }

                    return OK(p[0] == p[1]);
                }

                case Z3_OP_SLEQ: {
                    // X <=s X   =>   true
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bool_val(true));
                    }

                    return OK(z3::expr(ctx, Z3_mk_bvsle(ctx, p[0], p[1])));
                }

                case Z3_OP_SLT: {
                    // X <s X   =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bool_val(false));
                    }

                    return OK(z3::expr(ctx, Z3_mk_bvslt(ctx, p[0], p[1])));
                }

                case Z3_OP_ULEQ: {
                    // X <= X   =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bool_val(true));
                    }

                    return OK(z3::expr(ctx, Z3_mk_bvule(ctx, p[0], p[1])));
                }

                case Z3_OP_ULT: {
                    // X < 0   =>   0
                    if (is_zero(p[1]))
                    {
                        return OK(ctx.bool_val(false));
                    }
                    // X < X   =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(ctx.bool_val(false));
                    }

                    return OK(z3::expr(ctx, Z3_mk_bvult(ctx, p[0], p[1])));
                }

                case Z3_OP_ITE: {
                    // ITE(false, a, b)  =>  b
                    if (p[0].is_false())
                    {
                        return OK(p[2]);
                    }
                    // ITE(true, a, b)  =>  a
                    if (p[0].is_true())
                    {
                        return OK(p[1]);
                    }
                    // ITE(a, b, b)  =>  b
                    if (is_x_y(p[1], p[2]))
                    {
                        return OK(p[1]);
                    }

                    return OK(z3::ite(p[0], p[1], p[2]));
                }

                default:
                    return ERR("could not simplify sub-expression in abstract syntax tree: not implemented for given node type " + std::to_string(op));
            }
        }

    }    // namespace

    namespace z3_utils
    {
        Result<z3::expr> simplify_local(const z3::expr& e, std::unordered_map<u32, z3::expr>& cache, const bool check_correctness)
        {
            std::cout << "start simplification" << std::endl;

            const u32 max_loop_iterations = 128;
            u32 iteration                 = 0;

            z3::expr res      = e;
            z3::expr prev_res = res;

            do
            {
                prev_res                = res;
                const auto simplify_res = simplify_internal(res, cache, check_correctness);
                if (simplify_res.is_error())
                {
                    return simplify_res;
                }
                const auto [it, _] = cache.insert({e.id(), simplify_res.get()});
                res                = it->second;

                iteration++;
                if (iteration > max_loop_iterations)
                {
                    return ERR("Triggered max iteration counter during simplificaton!");
                }
            } while (!z3::eq(prev_res, res));

            if (check_correctness && !check_simplification_for_correctness(e, res))
            {
                return ERR("Simplification failed");
            }

            return OK(res);
        }

        Result<z3::expr> simplify_local(const z3::expr& e, const bool check_correctness)
        {
            std::unordered_map<u32, z3::expr> cache;
            return simplify_local(e, cache, check_correctness);
        }

    }    // namespace z3_utils
}    // namespace hal
