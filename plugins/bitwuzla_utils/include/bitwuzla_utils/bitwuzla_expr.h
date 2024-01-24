// #include "hal_core/netlist/boolean_function.h"

// #include <bitwuzla/cpp/bitwuzla.h>

// namespace hal
// {
//     namespace bitwuzla_utils
//     {
//         namespace bw
//         {
//             class expr
//             {
//             public:
//                 expr(const bitwuzla::Term term) : m_term{term} {};
//                 expr(int value) : m_value{value}
//                 {
//                     is_value = true;
//                 };

//                 int get_value();
//                 int bv_size();

//                 static expr variable(std::string name);
//                 static expr bw_const(BooleanFunction::Value value);
//                 static expr CONCAT(expr expr, BooleanFunction::Value value);
//                 static expr CONCAT(BooleanFunction::Value value, expr expr);
//                 static expr CONCAT(expr expr1, expr expr2);
//                 static expr AND(expr expr1, expr expr2);
//                 static expr OR(expr expr1, expr expr2);
//                 static expr NOT(expr expr1, expr expr2);
//                 static expr XOR(expr expr1, expr expr2);
//                 static expr ADD(expr expr1, expr expr2);
//                 static expr SUB(expr expr1, expr expr2);
//                 static expr MUL(expr expr1, expr expr2);
//                 static expr SDIV(expr expr1, expr expr2);
//                 static expr UDIV(expr expr1, expr expr2);
//                 static expr SREM(expr expr1, expr expr2);
//                 static expr UREM(expr expr1, expr expr2);
//                 static expr SLICE(expr expr, u64 start, u64 end);
//                 static expr ZEXT(expr expr, u64 size);
//                 static expr SEXT(expr expr, u64 size);
//                 static expr SHL(expr expr1, expr expr2);
//                 static expr LSHR(expr expr1, expr expr2);
//                 static expr ASHR(expr expr1, expr expr2);
//                 static expr ROR(expr expr, u64 amount);
//                 static expr ROL(expr expr, u64 amount);
//                 static expr EQ(expr expr1, expr expr2);
//                 static expr SLE(expr expr1, expr expr2);
//                 static expr SLT(expr expr1, expr expr2);
//                 static expr ULE(expr expr1, expr expr2);
//                 static expr ULT(expr expr1, expr expr2);
//                 static expr ITE(expr expr1, expr expr2, expr expr3);

//                 bitwuzla::Term m_term;

//             private:
//                 bool is_value = false;
//                 int m_value;
//             };

//         }    // namespace bw
//     }        // namespace bitwuzla_utils
// }    // namespace hal
