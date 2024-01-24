#include "bitwuzla_utils/bitwuzla_utils.h"

#include "hal_core/defines.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace bitwuzla_utils
    {
        namespace
        {
            Result<bitwuzla::Term> reduce_to_bitwuzla_term(const BooleanFunction::Node& node, std::vector<bitwuzla::Term>&& p, const std::map<std::string, bitwuzla::Term>& var2term = {})
            {
                if (node.get_arity() != p.size())
                {
                    return ERR("trying to append two nodes of unequal size");
                }
                switch (node.type)
                {
                    case BooleanFunction::NodeType::Index:
                        return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(node.size), node.index));
                    case BooleanFunction::NodeType::Constant: {
                        // since our constants are defined as arbitrary bit-vectors,
                        // we have to concat each bit just to be on the safe side
                        auto constant = (node.constant.front() == BooleanFunction::Value::ONE) ? bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1)) : bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1));
                        for (u32 i = 1; i < node.constant.size(); i++)
                        {
                            const auto bit = (node.constant.at(i) == BooleanFunction::Value::ONE) ? bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1)) : bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1));
                            constant       = bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {bit, constant});
                        }
                        return OK(constant);
                    }
                    case BooleanFunction::NodeType::Variable: {
                        if (auto it = var2term.find(node.variable); it != var2term.end())
                        {
                            return OK(it->second);
                        }
                        return OK(bitwuzla::mk_const(bitwuzla::mk_bv_sort(node.size), node.variable));
                    }
                    case BooleanFunction::NodeType::And:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Or:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Not:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::NOT, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Xor:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::XOR, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Add:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ADD, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Sub:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SUB, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Mul:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_MUL, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Sdiv:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SDIV, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Udiv:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_UDIV, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Srem:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SREM, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Urem:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_UREM, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Concat:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {p[0], p[1]}));
                    // case BooleanFunction::NodeType::Slice:
                    //     return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_EXTRACT, {p[0]}, std::vector<u64>{p[2].value(), p[1].value()}));
                    // case BooleanFunction::NodeType::Zext:
                    //     return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ZERO_EXTEND, {p[0]}, {p[1].value() - p[0].get_sort().bv_size()}));
                    // case BooleanFunction::NodeType::Sext:
                    //     return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SIGN_EXTEND, {p[0]}, {p[1].value() - p[0].get_sort().bv_size()}));
                    case BooleanFunction::NodeType::Shl:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SHL, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Lshr:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SHR, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Ashr:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ASHR, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Rol:
                    //     return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ROL, {p[0]}, {p[1].value()}));
                    // case BooleanFunction::NodeType::Ror:
                    //     return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ROR, {p[0]}, {p[1].value()}));
                    case BooleanFunction::NodeType::Eq:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Sle:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SLE, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Slt:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SLT, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Ule:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ULE, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Ult:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ULT, {p[0], p[1]}));
                    case BooleanFunction::NodeType::Ite:
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::ITE, {p[0], p[1], p[2]}));
                    default:
                        log_error("bitwuzla_utils", "Not implemented reached for nodetype {} in z3 conversion", node.type);
                        return ERR("Not implemented reached");
                }
            };

        }    // namespace

        Result<bitwuzla::Term> from_bf(const BooleanFunction& bf, const std::map<std::string, bitwuzla::Term>& var2term)
        {
            std::vector<bitwuzla::Term> stack;
            for (const auto& node : bf.get_nodes())
            {
                std::vector<bitwuzla::Term> operands;
                std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
                stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

                auto res = reduce_to_bitwuzla_term(node, std::move(operands));
                if (res.is_ok())
                {
                    stack.emplace_back(res.get());
                }
                else
                {
                    return ERR_APPEND(res.get_error(), "could not create boolean function for node");
                }
            }

            switch (stack.size())
            {
                case 1:
                    return OK(stack.back());
                default:
                    return ERR("all nodes somehow didnt get finished");
            }
        }

        Result<BooleanFunction> to_bf(const bitwuzla::Term& t)
        {
            u64 size;
            if (t.sort().is_bv())
            {
                size = t.sort().bv_size();

                if (size > 64)
                {
                    return ERR("can only translate bit vector sizes < 64, but input bit vector has size " + std::to_string(size));
                }

                if (t.is_value())
                {
                    // return OK(BooleanFunction::Const(t.get_numeral_uint64(), size)); // TODO
                }
                else if (t.is_const())
                {
                    // return OK(BooleanFunction::Var(t.to_string(), size)); // TODO
                }
            }

            return ERR("todo");

            const auto op = t.kind(); // TODO: const auto op = e.decl().decl_kind();
            auto num_args = t.num_children(); // TODO auto num_args = e.num_args();
            std::vector<bitwuzla::Term> children = t.children();
            std::vector<BooleanFunction> args;

            for (const auto& child : children) // TODO
            {
                const auto arg = child;
                if (const auto res = to_bf(arg); res.is_ok())
                {
                    args.push_back(std::move(res.get()));
                }
                else
                {
                    return ERR(res.get_error());
                }
            }

            switch (op)
            {
                case bitwuzla::Kind::AND: {
                    auto bf_res = BooleanFunction::And(std::move(args.at(0)), std::move(args.at(1)), size);
                    for (u64 i = 2; i < num_args; i++)
                    {
                        bf_res = bf_res.map<BooleanFunction>([arg = std::move(args.at(i)), size](BooleanFunction&& bf) mutable { return BooleanFunction::And(std::move(bf), std::move(arg), size); });
                    }
                    return bf_res;
                }
                case bitwuzla::Kind::OR: {
                    auto bf_res = BooleanFunction::Or(std::move(args.at(0)), std::move(args.at(1)), size);
                    for (u64 i = 2; i < num_args; i++)
                    {
                        bf_res = bf_res.map<BooleanFunction>([arg = std::move(args.at(i)), size](BooleanFunction&& bf) mutable { return BooleanFunction::Or(std::move(bf), std::move(arg), size); });
                    }
                    return bf_res;
                }
                case bitwuzla::Kind::NOT: {
                    if (num_args != 1)
                    {
                        return ERR("operation 'NOT' must have arity 1");
                    }
                    return BooleanFunction::Not(std::move(args.at(0)), size);
                }
                case bitwuzla::Kind::XOR: {
                    auto bf_res = BooleanFunction::Xor(std::move(args.at(0)), std::move(args.at(1)), size);
                    for (u64 i = 2; i < num_args; i++)
                    {
                        bf_res = bf_res.map<BooleanFunction>([arg = std::move(args.at(i)), size](BooleanFunction&& bf) mutable { return BooleanFunction::Xor(std::move(bf), std::move(arg), size); });
                    }
                    return bf_res;
                }
                case bitwuzla::Kind::BV_ADD: {
                    auto bf_res = BooleanFunction::Add(std::move(args.at(0)), std::move(args.at(1)), size);
                    for (u64 i = 2; i < num_args; i++)
                    {
                        bf_res = bf_res.map<BooleanFunction>([arg = std::move(args.at(i)), size](BooleanFunction&& bf) mutable { return BooleanFunction::Add(std::move(bf), std::move(arg), size); });
                    }
                    return bf_res;
                }
                case bitwuzla::Kind::BV_SUB: {
                    auto bf_res = BooleanFunction::Sub(std::move(args.at(0)), std::move(args.at(1)), size);
                    for (u64 i = 2; i < num_args; i++)
                    {
                        bf_res = bf_res.map<BooleanFunction>([arg = std::move(args.at(i)), size](BooleanFunction&& bf) mutable { return BooleanFunction::Sub(std::move(bf), std::move(arg), size); });
                    }
                    return bf_res;
                }
                case bitwuzla::Kind::BV_MUL: {
                    auto bf_res = BooleanFunction::Mul(std::move(args.at(0)), std::move(args.at(1)), size);
                    for (u64 i = 2; i < num_args; i++)
                    {
                        bf_res = bf_res.map<BooleanFunction>([arg = std::move(args.at(i)), size](BooleanFunction&& bf) mutable { return BooleanFunction::Mul(std::move(bf), std::move(arg), size); });
                    }
                    return bf_res;
                }
                case bitwuzla::Kind::BV_SDIV:
                    if (num_args != 2)
                    {
                        return ERR("operation 'SDIV' must have arity 2");
                    }
                    return BooleanFunction::Sdiv(std::move(args.at(0)), std::move(args.at(1)), size);
                case bitwuzla::Kind::BV_UDIV:
                    if (num_args != 2)
                    {
                        return ERR("operation 'UDIV' must have arity 2");
                    }
                    return BooleanFunction::Udiv(std::move(args.at(0)), std::move(args.at(1)), size);
                case bitwuzla::Kind::BV_SREM:
                    if (num_args != 2)
                    {
                        return ERR("operation 'SREM' must have arity 2");
                    }
                    return BooleanFunction::Srem(std::move(args.at(0)), std::move(args.at(1)), size);
                case bitwuzla::Kind::BV_UREM:
                    if (num_args != 2)
                    {
                        return ERR("operation 'UREM' must have arity 2");
                    }
                    return BooleanFunction::Urem(std::move(args.at(0)), std::move(args.at(1)), size);
                case bitwuzla::Kind::BV_CONCAT: {
                    auto bf_res = BooleanFunction::Concat(std::move(args.at(0)), std::move(args.at(1)), size);
                    for (u64 i = 2; i < num_args; i++)
                    {
                        bf_res =
                            bf_res.map<BooleanFunction>([arg = std::move(args.at(i)), size](BooleanFunction&& bf) mutable { return BooleanFunction::Concat(std::move(bf), std::move(arg), size); });
                    }
                    return bf_res;
                }
                // case bitwuzla::Kind::BV_EXTRACT: {
                //     if (num_args != 1)
                //     {
                //         return ERR("operation 'SLICE' must have arity 1");
                //     }

                //     const u32 operand_size = args.at(0).size();

                //     return BooleanFunction::Slice(std::move(args.at(0)), BooleanFunction::Index(t.lo(), operand_size), BooleanFunction::Index(t.hi(), operand_size), size);
                // }
                case bitwuzla::Kind::BV_ZERO_EXTEND: {
                    if (num_args != 1)
                    {
                        return ERR("operation 'ZEXT' must have arity 1");
                    }

                    return BooleanFunction::Zext(std::move(args.at(0)), BooleanFunction::Index(size, size), size);
                }
                case bitwuzla::Kind::BV_SIGN_EXTEND: {
                    if (num_args != 1)
                    {
                        return ERR("operation 'SEXT' must have arity 1");
                    }

                    return BooleanFunction::Sext(std::move(args.at(0)), BooleanFunction::Index(size, size), size);
                }
                case bitwuzla::Kind::BV_SHL:
                    if (num_args != 2)
                    {
                        return ERR("operation 'SHL' must have arity 2");
                    }
                    return BooleanFunction::Shl(std::move(args.at(0)), BooleanFunction::Index((u16)args.at(1).get_constant_value_u64().get(), size), size);
                // case bitwuzla::Kind::BV_LSHR:
                //     if (num_args != 2)
                //     {
                //         return ERR("operation 'LSHR' must have arity 2");
                //     }
                //     return BooleanFunction::Lshr(std::move(args.at(0)), BooleanFunction::Index((u16)args.at(1).get_constant_value_u64().get(), size), size);
                case bitwuzla::Kind::BV_ASHR:
                    if (num_args != 2)
                    {
                        return ERR("operation 'ASHR' must have arity 2");
                    }
                    return BooleanFunction::Ashr(std::move(args.at(0)), BooleanFunction::Index((u16)args.at(1).get_constant_value_u64().get(), size), size);
                // case bitwuzla::Kind::BV_ROL:
                //     if (num_args != 1)
                //     {
                //         return ERR("operation 'ROL' must have arity 1");
                //     }
                //     return BooleanFunction::Rol(std::move(args.at(0)), BooleanFunction::Index((u16)Z3_get_decl_int_parameter(Z3_context(t.ctx()), Z3_func_decl(t.decl()), 0), size), size);
                // case bitwuzla::Kind::BV_ROR:
                //     if (num_args != 1)
                //     {
                //         return ERR("operation 'ROR' must have arity 1");
                //     }
                //     return BooleanFunction::Ror(std::move(args.at(0)), BooleanFunction::Index((u16)Z3_get_decl_int_parameter(Z3_context(t.ctx()), Z3_func_decl(t.decl()), 0), size), size);
                case bitwuzla::Kind::EQUAL:
                    if (num_args != 2)
                    {
                        return ERR("operation 'EQ' must have arity 2");
                    }
                    return BooleanFunction::Eq(std::move(args.at(0)), std::move(args.at(1)), 1);
                case bitwuzla::Kind::BV_SLE:
                    if (num_args != 2)
                    {
                        return ERR("operation 'SLE' must have arity 2");
                    }
                    return BooleanFunction::Sle(std::move(args.at(0)), std::move(args.at(1)), 1);
                case bitwuzla::Kind::BV_SLT:
                    if (num_args != 2)
                    {
                        return ERR("operation 'SLT' must have arity 2");
                    }
                    return BooleanFunction::Slt(std::move(args.at(0)), std::move(args.at(1)), 1);
                case bitwuzla::Kind::BV_ULE:
                    if (num_args != 2)
                    {
                        return ERR("operation 'ULE' must have arity 2");
                    }
                    return BooleanFunction::Ule(std::move(args.at(0)), std::move(args.at(1)), 1);
                case bitwuzla::Kind::BV_ULT:
                    if (num_args != 2)
                    {
                        return ERR("operation 'ULT' must have arity 2");
                    }
                    return BooleanFunction::Ult(std::move(args.at(0)), std::move(args.at(1)), 1);
                case bitwuzla::Kind::ITE:
                    if (num_args != 3)
                    {
                        return ERR("operation 'ITE' must have arity 3");
                    }
                    return BooleanFunction::Ite(std::move(args.at(0)), std::move(args.at(1)), std::move(args.at(2)), size);
                default:
                    // return ERR("operation '" + t.decl().name().str() + "' with arity " + std::to_string(num_args) + " is not yet implemented");
                    return ERR("not yet implemented");
            }
        }

    }    // namespace bitwuzla_utils
}    // namespace hal

//         namespace bw
//         {
//             namespace
//             {
//                 Result<bitwuzla::Term> reduce_to_bw(const auto& node, auto&& p)
//                 {
//                     if (node.get_arity() != p.size())
//                     {
//                         return ERR("trying to append two nodes of unequal size");
//                     }

//                     switch (node.type)
//                     {
//                         case BooleanFunction::NodeType::Index:
//                             return expr(node.index);    //{true, context.bv_val(node.index, node.size)};
//                         case BooleanFunction::NodeType::Constant: {
//                             // since our constants are defined as arbitrary bit-vectors,
//                             // we have to concat each bit just to be on the safe side
//                             auto const = context.bw_const(node.constant.front());
//                             for (u32 i = 1; i < node.constant.size(); i++)
//                             {
//                                 const auto bit = node.constant.at(i);
//                                 constant       = bw::concat(context.bv_val(bit, 1), constant);
//                             }
//                             return OK(constant);
//                         }
//                         case BooleanFunction::NodeType::Variable: {
//                             if (auto it = var2expr.find(node.variable); it != var2expr.end())
//                             {
//                                 return {true, it->second};
//                             }
//                             return {true, context.bv_const(node.variable.c_str(), node.size)};
//                         }

//                         case BooleanFunction::NodeType::And:
//                             return OK(AND(p[0], p[1]));
//                         case BooleanFunction::NodeType::Or:
//                         return OK(OR(p[0], p[1])) case BooleanFunction::NodeType::Not:
//                         return OK(NOT(p[0], p[1])) case BooleanFunction::NodeType::Xor:
//                         return OK(XOR(p[0], p[1])) case BooleanFunction::NodeType::Add:
//                         return OK(ADD(p[0], p[1])) case BooleanFunction::NodeType::Sub:
//                         return OK(SUB(p[0], p[1])) case BooleanFunction::NodeType::Mul:
//                         return OK(MUL(p[0], p[1])) case BooleanFunction::NodeType::Sdiv:
//                         return OK(SDIV(p[0], p[1])) case BooleanFunction::NodeType::Udiv:
//                         return OK(UDIV(p[0], p[1])) case BooleanFunction::NodeType::Srem:
//                         return OK(SREM(p[0], p[1])) case BooleanFunction::NodeType::Urem:
//                         return OK(UREM(p[0], p[1])) case BooleanFunction::NodeType::Concat:
//                         return OK(CONCAT(p[0], p[1])) case BooleanFunction::NodeType::Slice:
//                             return OK(SLICE(p[0], p[2].get_value(), p[1].get_value()));
//                         case BooleanFunction::NodeType::Zext:
//                             return OK(ZEXT(p[0], p[1].get_value() - p[0].bv_size()));
//                         case BooleanFunction::NodeType::Sext:
//                             return OK(SEXT(p[0], p[1].get_value() - p[0].bv_size()));
//                         case BooleanFunction::NodeType::Shl:
//                         return OK(SHL(p[0], p[1])) case BooleanFunction::NodeType::Lshr:
//                         return OK(LSHR(p[0], p[1])) case BooleanFunction::NodeType::Ashr:
//                         return OK(ASHR(p[0], p[1])) case BooleanFunction::NodeType::Rol:
//                         return OK(ROL(p[0], p[1].get_value())) case BooleanFunction::NodeType::Ror:
//                         return OK(ROR(p[0], p[1].get_value())) case BooleanFunction::NodeType::Eq:
//                         return OK(EQ(p[0], p[1])) case BooleanFunction::NodeType::Sle:
//                         return OK(SLE(p[0], p[1])) case BooleanFunction::NodeType::Slt:
//                         return OK(SLT(p[0], p[1])) case BooleanFunction::NodeType::Ule:
//                         return OK(ULE(p[0], p[1])) case BooleanFunction::NodeType::Ult:
//                         return OK(ULT(p[0], p[1])) case BooleanFunction::NodeType::Ite:
//                             return OK(ITE(p[0], p[1], p[2])) default : return ERR("netlist", "Not implemented reached for nodetype {} in bw conversion", node.type);
//                     }
//                 };

//             }    // namespace

//             Result<expr> from_bf(const BooleanFunction& bf, const std::map<std::string, bw::expr>& var2expr)
//             {
//                 std::vector<expr> stack;
//                 for (const auto& node : bf.get_nodes())
//                 {
//                     std::vector<bw::expr> operands;
//                     std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
//                     stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

//                     auto res = reduce_to_bw(node, std::move(operands));
//                     if (res.is_ok())
//                     {
//                         stack.emplace_back(res.get());
//                     }
//                     else
//                     {
//                         return ERR_APPEND(res.get_error(), "could not create boolean function for node");
//                     }
//                 }

//                 switch (stack.size())
//                 {
//                     case 1:
//                         return OK(stack.back());
//                     default:
//                         return ERR("all nodes somehow didnt get finished");
//                 }
//             }

//             std::set<std::string> get_variable_names(const bw::expr& e)
//             {
//                 auto map = e.m_term.statistics();
//             }

//         }    // namespace bw
//     }        // namespace bitwuzla_utils
// }    // namespace hal