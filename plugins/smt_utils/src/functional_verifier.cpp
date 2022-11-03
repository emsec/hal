#include "functional_verifier.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_utils.h"

namespace hal
{
    namespace smt_utils
    {
        Result<bool> verify_add(const std::vector<Gate*>& candidate_gates, const std::vector<Net*>& input_a, const std::vector<Net*>& input_b, const std::vector<Net*>& output)
        {
            return OK(false);
        }

        Result<bool> verify_ult(const std::vector<const Gate*>& candidate_gates, const std::vector<Net*>& input_a, const std::vector<Net*>& input_b, const Net* output)
        {
            if (output == nullptr)
            {
                return ERR("could not verify whether candidate gates implement a less-than comparison: 'nullptr' given as output net");
            }

            u32 size = input_a.size();
            if (size == 0)
            {
                return ERR("could not verify whether candidate gates implement a less-than comparison: no inputs provided");
            }
            else if (size != input_b.size())
            {
                return ERR("could not verify whether candidate gates implement a less-than comparison: input sizes do not match");
            }

            auto var_a = BooleanFunction::Var("net_" + std::to_string(input_a.front()->get_id()), 1);
            auto var_b = BooleanFunction::Var("net_" + std::to_string(input_b.front()->get_id()), 1);
            for (u32 i = 1; i < size; i++)
            {
                if (auto var_a_res = BooleanFunction::Concat(BooleanFunction::Var("net_" + std::to_string(input_a.at(i)->get_id()), 1), var_a.clone(), i + 1); var_a_res.is_error())
                {
                    var_a = var_a_res.get();
                }
                else
                {
                    return ERR_APPEND(var_a_res.get_error(), "could not verify whether candidate gates implement a less-than comparison: unable to concatenate variables of input A");
                }
                if (auto var_b_res = BooleanFunction::Concat(BooleanFunction::Var("net_" + std::to_string(input_b.at(i)->get_id()), 1), var_b.clone(), i + 1); var_b_res.is_ok())
                {
                    var_b = var_b_res.get();
                }
                else
                {
                    return ERR_APPEND(var_b_res.get_error(), "could not verify whether candidate gates implement a less-than comparison: unable to concatenate variables of input B");
                }
            }

            return BooleanFunction::Ult(std::move(var_a), std::move(var_b), size)
                .map<BooleanFunction>([&](auto&& ult_bf) -> Result<BooleanFunction> {
                    return netlist_utils::get_subgraph_function(output, candidate_gates).map<BooleanFunction>([&](auto&& subgraph_bf) -> Result<BooleanFunction> {
                        return BooleanFunction::Eq(std::move(ult_bf), std::move(subgraph_bf), size);
                    });
                })
                .map<BooleanFunction>([](auto&& eq_bf) -> Result<BooleanFunction> { return BooleanFunction::Not(std::move(eq_bf), 1); })
                .map<SMT::Solver>([](auto&& not_bf) -> Result<SMT::Solver> { return OK(SMT::Solver({SMT::Constraint(std::move(not_bf))})); })
                .map<SMT::SolverResult>([](auto&& solver) -> Result<SMT::SolverResult> { return solver.query(SMT::QueryConfig()); })
                .map<bool>([](auto&& result) -> Result<bool> { return OK(result.is_unsat()); });
        }
    }    // namespace smt_utils
}    // namespace hal