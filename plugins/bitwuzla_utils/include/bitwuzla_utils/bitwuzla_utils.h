// #include "bitwuzla_utils/bitwuzla_expr.h"
// #include "hal_core/netlist/boolean_function.h"

// #include <bitwuzla/cpp/bitwuzla.h>

// namespace hal
// {
//     namespace bitwuzla_utils
//     {
//         namespace bw
//         {
//             class Config
//             {
//             public:
//                 Config()
//                 {
//                     options.set(bitwuzla::Option::PRODUCE_MODELS, true);
//                     options.set(bitwuzla::Option::SAT_SOLVER, "cadical");
//                     this->wuzla = bitwuzla::Bitwuzla(options);
//                 };

//                 static bw::expr bw_const(BooleanFunction::Value value);

//             private:
//                 bitwuzla::Options options;
//                 bitwuzla::Bitwuzla wuzla;
//             };

//         }    // namespace bw

//     }    // namespace bitwuzla_utils
// }    // namespace hal
