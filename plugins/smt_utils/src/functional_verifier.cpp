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

            const auto bf_res = netlist_utils::get_subgraph_function(output, candidate_gates);
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(), "could not verify whether candidate gates implement a less-than comparison: unable to gte subgraph function");
            }
            auto bf = bf_res.get();

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

            auto function = BooleanFunction::Ult(var_a.clone(), var_b.clone(), size)
                                .map<BooleanFunction>([bf, size](auto&& ult) { return BooleanFunction::Eq(std::move(ult), bf.clone(), size); })
                                .map<BooleanFunction>([bf, size](auto&& eq) { return BooleanFunction::Not(std::move(eq), 1); });
            // if (auto ult_res = BooleanFunction::Ult(var_a.clone(), var_b.clone(), size); ult_res.is_ok())
            // {
            //     if (auto eq_res = BooleanFunction::Eq(ult_res.get(), bf.clone(), size); eq_res.is_ok())
            //     {
            //         if (auto not_res = BooleanFunction::Not(eq_res.get(), 1); not_res.is_ok())
            //         {
            //             auto constraint = SMT::Constraint(not_res.get());
            //             auto solver     = SMT::Solver({constraint});
            //             if (auto query_res = solver.query(SMT::QueryConfig().with_solver(SMT::SolverType::Z3).with_local_solver().with_timeout(1000)); query_res.is_ok())
            //             {
            //                 return OK(query_res.get().is_unsat());
            //             }
            //             else
            //             {
            //                 return ERR_APPEND(query_res.get_error(), "could not verify whether candidate gates implement a less-than comparison: unable to query SMT solver");
            //             }
            //         }
            //         else
            //         {
            //             return ERR_APPEND(not_res.get_error(), "could not verify whether candidate gates implement a less-than comparison: unable to create NOT function");
            //         }
            //     }
            //     else
            //     {
            //         return ERR_APPEND(eq_res.get_error(), "could not verify whether candidate gates implement a less-than comparison: unable to create EQ function");
            //     }
            // }
            // else
            // {
            // return ERR_APPEND(ult_res.get_error(), "could not verify whether candidate gates implement a less-than comparison: unable to create ULT function");
            // }
            return OK(true);
        }
    }    // namespace smt_utils
}    // namespace hal