#include "functional_verifier.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_utils.h"

namespace hal
{
    namespace smt_utils
    {
        namespace
        {
            Result<BooleanFunction> get_concat_net_function(const std::vector<Net*>& nets, u32 zero_extend_to_size = 0)
            {
                if (nets.empty())
                {
                    return ERR("could not concatenate nets: no nets provided.");
                }

                if (nets.front() == nullptr)
                {
                    return ERR("could not concatenate nets: nets contain a 'nullptr'.");
                }
                auto var = BooleanFunction::Var("net_" + std::to_string(nets.front()->get_id()), 1);

                for (u32 i = 1; i < nets.size(); i++)
                {
                    if (nets.at(i) == nullptr)
                    {
                        return ERR("could not concatenate nets: nets contain a 'nullptr'.");
                    }

                    if (auto res = BooleanFunction::Concat(BooleanFunction::Var("net_" + std::to_string(nets.at(i)->get_id()), 1), var.clone(), i + 1); res.is_error())
                    {
                        var = res.get();
                    }
                    else
                    {
                        return ERR_APPEND(res.get_error(),
                                          "could not concatenate nets: unable to concatenate net '" + nets.at(i)->get_name() + "' with ID " + std::to_string(nets.at(i)->get_id()) + " at position "
                                              + std::to_string(i) + " of the provided nets to function.");
                    }
                }

                if (zero_extend_to_size > 0)
                {
                    if (auto res = BooleanFunction::Zext(var.clone(), BooleanFunction::Index(zero_extend_to_size, zero_extend_to_size), zero_extend_to_size); res.is_error())
                    {
                        var = res.get();
                    }
                    else
                    {
                        return ERR_APPEND(res.get_error(),
                                          "could not concatenate nets: unable to zero extend function of size " + std::to_string(var.size()) + " to size " + std::to_string(zero_extend_to_size) + ".");
                    }
                }

                return OK(var);
            }
        }    // namespace

        Result<bool> verify_ult(const std::vector<const Gate*>& candidate_gates, const std::vector<Net*>& input_a, const std::vector<Net*>& input_b, const Net* output)
        {
            if (output == nullptr)
            {
                return ERR("could not verify whether candidate gates implement a less-than comparison: 'nullptr' given as output net");
            }

            u32 size = std::max(input_a.size(), input_b.size());
            if (size == 0)
            {
                return ERR("could not verify whether candidate gates implement a less-than comparison: no inputs provided");
            }

            auto var_a = get_concat_net_function(input_a, size);
            if (var_a.is_error())
            {
                return ERR_APPEND(var_a.get_error(), "could not verify whether candidate gates implement a less-than comparison: could not concatenate nets of input A to Boolean function");
            }
            auto var_b = get_concat_net_function(input_b, size);
            if (var_b.is_error())
            {
                return ERR_APPEND(var_b.get_error(), "could not verify whether candidate gates implement a less-than comparison: could not concatenate nets of input B to Boolean function");
            }

            return BooleanFunction::Ult(var_a.get(), var_b.get(), size)
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

        Result<bool> verify_slt(const std::vector<const Gate*>& candidate_gates, const std::vector<Net*>& input_a, const std::vector<Net*>& input_b, const Net* output)
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

            return BooleanFunction::Slt(std::move(var_a), std::move(var_b), size)
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