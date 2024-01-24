// #include "bitwuzla_utils/bitwuzla_utils.h"

// namespace hal
// {
//     namespace bitwuzla_utils
//     {
//         namespace bw
//         {
//             namespace
//             {
//                 Result<bw::expr> reduce_to_bw(const auto& node, auto&& p)
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