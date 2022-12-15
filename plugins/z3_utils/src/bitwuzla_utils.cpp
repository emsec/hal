#include "bitwuzla/bitwuzla.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "plugin_z3_utils.h"

namespace hal
{
    namespace Bitwuzla
    {
        Result<SMT::SolverResult> query(const std::string& smt2)
        {
            auto bzla = bitwuzla_new();

            char* in;
            size_t in_len = {};

            char* out;
            size_t out_len = {};

            auto in_stream  = open_memstream(&in, &in_len);
            auto out_stream = open_memstream(&out, &out_len);

            char* error;

            BitwuzlaResult _r;

            bitwuzla_set_option(bzla, BITWUZLA_OPT_PRODUCE_MODELS, 1);

            // write smt2 string to in stream
            fprintf(in_stream, smt2.c_str());
            fflush(in_stream);

            // printf("Input: %s\n", in);

            auto res = bitwuzla_parse_format(bzla, "smt2", in_stream, "VIRTUAL FILE", out_stream, &error, &_r);
            fflush(out_stream);

            // printf("Output: %s\n", out);

            if (error != nullptr)
            {
                return ERR("failed to solve provided smt2 solver with bitwuzla: " + std::string(error));
            }

            fclose(in_stream);
            fclose(out_stream);

            const std::map<BitwuzlaResult, SMT::SolverResult> to_hal{
                {BitwuzlaResult::BITWUZLA_SAT, SMT::SolverResult::Sat()},
                {BitwuzlaResult::BITWUZLA_UNSAT, SMT::SolverResult::UnSat()},
                {BitwuzlaResult::BITWUZLA_UNKNOWN, SMT::SolverResult::Unknown()},
            };

            bitwuzla_delete(bzla);

            return OK(to_hal.at(res));
        }

    }    // namespace Bitwuzla
}    // namespace hal