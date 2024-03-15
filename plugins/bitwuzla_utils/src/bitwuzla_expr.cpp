// #include "bitwuzla_utils/bitwuzla_expr.h"

// #include <bitwuzla/cpp/bitwuzla.h>

// namespace hal
// {
//     namespace bitwuzla_utils
//     {
//         namespace bw
//         {
//             int expr::bv_size()
//             {
//                 return (this->m_term).sort().bv_size();
//             }
//             int expr::get_value()
//             {
//                 return this->m_value;
//             }

//             expr expr::bw_const(BooleanFunction::Value value)
//             {
//                 bitwuzla::Term term;
//                 switch (value)
//                 {
//                     case BooleanFunction::Value::ONE:
//                         term = bitwuzla::mk_true();
//                         break;
//                     case BooleanFunction::Value::ZERO:
//                         term = bitwuzla::mk_false();
//                         break;
//                     case BooleanFunction::Value::X:
//                         term = bitwuzla::mk_const(bitwuzla::mk_bool_sort(), "X");
//                         break;
//                     case BooleanFunction::Value::Z:
//                         term = bitwuzla::mk_const(bitwuzla::mk_bool_sort(), "Z");
//                         break;
//                 }
//                 return expr(term);
//             }
//             expr expr::variable(std::string name)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_var(bitwuzla::mk_bool_sort(), name);
//                 return expr(term);
//             }
//             expr expr::CONCAT(expr expr, BooleanFunction::Value value)
//             {
//                 return CONCAT(expr, bw_const(value));
//             }
//             expr expr::CONCAT(BooleanFunction::Value value, expr expr)
//             {
//                 return CONCAT(bw_const(value), expr);
//             }
//             expr expr::CONCAT(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }

//             expr expr::AND(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::AND, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::OR(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::OR, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::NOT(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::NOT, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::XOR(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::XOR, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::ADD(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_ADD, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::SUB(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SUB, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::MUL(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_MUL, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::SDIV(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SDIV, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::UDIV(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_UDIV, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::SREM(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SREM, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::UREM(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_UREM, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::SLICE(expr expr1, u64 start, u64 end)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_EXTRACT, {expr1.m_term}, std::vector<u64>{start, end});
//                 return expr(term);
//             }
//             expr expr::ZEXT(expr expr1, u64 size)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_ZERO_EXTEND, {expr1.m_term}, {size});
//                 return expr(term);
//             }
//             expr expr::SEXT(expr expr1, u64 size)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SIGN_EXTEND, {expr1.m_term}, {size});
//                 return expr(term);
//             }
//             expr expr::SHL(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SHL, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::LSHR(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SHR, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::ASHR(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_ASHR, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::ROR(expr expr1, u64 amount)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_ROR, {expr1.m_term}, {amount});
//                 return expr(term);
//             }
//             expr expr::ROL(expr expr1, u64 amount)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_ROL, {expr1.m_term}, {amount});
//                 return expr(term);
//             }
//             expr expr::EQ(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::SLE(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SLE, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::SLT(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_SLT, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::ULE(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_ULE, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::ULT(expr expr1, expr expr2)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::BV_ULT, {expr1.m_term, expr2.m_term});
//                 return expr(term);
//             }
//             expr expr::ITE(expr expr1, expr expr2, expr expr3)
//             {
//                 bitwuzla::Term term = bitwuzla::mk_term(bitwuzla::Kind::ITE, {expr1.m_term, expr2.m_term, expr3.m_term});
//                 return expr(term);
//             }
//         }    // namespace bw
//     }        // namespace bitwuzla_utils
// }    // namespace hal
