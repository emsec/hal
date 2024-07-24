#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/utilities/log.h"
#include "module_identification/candidates/functional_candidate.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/utils/utils.h"

namespace hal
{
    namespace module_identification
    {
        namespace
        {
            BooleanFunction create_operand(const std::vector<Net*>& op, const u32 size = 0)
            {
                if (op.empty())
                {
                    log_error("module_identification", "nets are empty, cannot create bit vector");
                    return BooleanFunction();
                }

                BooleanFunction bf;
                const auto op_size = (size == 0) ? op.size() : size;
                for (u32 i = 0; i < op_size; i++)
                {
                    const auto net = op.at(i);

                    BooleanFunction new_bit_bf;
                    if (net->is_gnd_net())
                    {
                        new_bit_bf = BooleanFunction::Const(BooleanFunction::Value::ZERO);
                    }
                    else if (net->is_vcc_net())
                    {
                        new_bit_bf = BooleanFunction::Const(BooleanFunction::Value::ONE);
                    }
                    else
                    {
                        new_bit_bf = BooleanFunctionNetDecorator(*net).get_boolean_variable();
                    }

                    if (bf.is_empty())
                    {
                        bf = std::move(new_bit_bf);
                    }
                    else
                    {
                        auto bf_res = BooleanFunction::Concat(std::move(new_bit_bf), std::move(bf), bf.size() + 1);
                        if (bf_res.is_error())
                        {
                            log_error("module_identification", "{}", bf_res.get_error().get());
                        }
                        bf = bf_res.get();
                    }
                }
                return bf;
            }

            Result<std::pair<bool, u64>> do_probe_testing(const std::vector<BooleanFunction>& actual_funcitons, const BooleanFunction& expected_function, const u32 num_tests)
            {
                const auto start_probe = std::chrono::steady_clock::now();

                std::srand(std::time(nullptr));    //init rand
                for (u32 i = 0; i < num_tests; i++)
                {
                    std::unordered_map<std::string, std::vector<BooleanFunction::Value>> test_mapping;
                    //create random inputs
                    auto variables  = expected_function.get_variable_names();
                    u64 mapping     = std::rand();
                    u64 current_bit = 0;
                    for (auto current_variable : variables)
                    {
                        std::vector<BooleanFunction::Value> current_input;
                        current_input.push_back(BooleanFunction::Value((mapping >> current_bit) & 1));
                        test_mapping[current_variable] = current_input;

                        current_bit++;
                    }

                    auto expected_result = expected_function.evaluate(test_mapping);
                    if (expected_result.is_error())
                    {
                        return ERR_APPEND(expected_result.get_error(), "failed to evalute boolean function");
                    }

                    for (u32 func_idx = 0; func_idx < expected_function.size(); func_idx++)
                    {
                        const auto& actual_funciton = actual_funcitons.at(func_idx);
                        auto actual_result          = actual_funciton.evaluate(test_mapping);
                        if (actual_result.is_error())
                        {
                            return ERR_APPEND(actual_result.get_error(), "failed to evalute boolean function");
                        }

                        if (actual_result.get()[0] != expected_result.get()[func_idx])
                        {
                            return OK({false, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_probe).count()});
                        }
                    }
                }

                return OK({true, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_probe).count()});
            }

            Result<bool>
                smt_check(const std::vector<BooleanFunction>& actual_funcitons, BooleanFunction&& expected_function, const u32 num_probes, std::map<std::string, std::map<std::string, u64>>& timings)
            {
                // const auto start_probe_timing = std::chrono::steady_clock::now();

                auto probe_res = do_probe_testing(actual_funcitons, expected_function, num_probes);
                if (probe_res.is_error())
                {
                    return ERR_APPEND(probe_res.get_error(), "failed selective testing on boolean function");
                }
                const auto [probe, probe_duration] = probe_res.get();

                timings["Probe"]["Duration"] += probe_duration;
                if (probe)
                {
                    timings["Probe"]["#Successful"] += 1;
                }
                else
                {
                    timings["Probe"]["#Failed"] += 1;
                    return OK(false);
                }

                const auto start_solver_timing = std::chrono::steady_clock::now();

                auto config = SMT::QueryConfig();
                auto solver = SMT::Solver();

#ifdef BITWUZLA_LIBRARY
                auto s_type = SMT::SolverType::Bitwuzla;
                auto s_call = SMT::SolverCall::Library;
                config      = config.with_solver(s_type).with_call(s_call).without_model_generation();
#endif

                // for (u32 idx = 0; idx < expected_function.size(); idx++)
                // {
                //     auto bf_slice_res = BooleanFunction::Slice(expected_function.clone(), BooleanFunction::Index(idx, expected_function.size()), BooleanFunction::Index(idx, expected_function.size()), 1);
                //     if (bf_slice_res.is_error())
                //     {
                //         return ERR_APPEND(bf_slice_res.get_error(), "failed equal check: could not build single bit sliced version of expected function");
                //     }
                //     auto bf_slice = bf_slice_res.get();

                //     // actual == expected
                //     auto bf_eq_res = BooleanFunction::Eq(actual_funcitons.at(idx).clone(), std::move(bf_slice), 1);
                //     if (bf_eq_res.is_error())
                //     {
                //         return ERR_APPEND(bf_eq_res.get_error(), "failed to concatenate boolean functions");
                //     }
                //     auto bf_eq = bf_eq_res.get();

                //     // actual != expected
                //     auto bf_neq_res = BooleanFunction::Not(std::move(bf_eq), 1);
                //     if (bf_neq_res.is_error())
                //     {
                //         return ERR_APPEND(bf_eq_res.get_error(), "failed to concatenate boolean functions");
                //     }
                //     auto bf_neq = bf_neq_res.get();

                //     auto solver_result_res = solver.with_constraint(SMT::Constraint(std::move(bf_neq))).query(config);
                //     if (solver_result_res.is_error())
                //     {
                //         return ERR_APPEND(solver_result_res.get_error(), "failed solver check for module identification utils");
                //     }
                //     auto solver_result = solver_result_res.get();

                //     if (!solver_result.is_unsat())
                //     {
                //         return OK(false);
                //     }
                // }
                // return OK(true);

                // OLD version checking everything all at once

                // auto actual_funciton = concat_bfs(actual_funcitons);
                auto actual_funciton = actual_funcitons.at(0).clone();
                for (u32 i = 1; i < expected_function.size(); i++)
                {
                    auto bf_res = BooleanFunction::Concat(actual_funcitons.at(i).clone(), std::move(actual_funciton), actual_funciton.size() + 1);
                    if (bf_res.is_error())
                    {
                        return ERR_APPEND(bf_res.get_error(), "cannot compelete equal check: failed to concatenate output functions");
                    }
                    actual_funciton = bf_res.get();
                }

                // z == (a + b)
                auto bf_eq_res = BooleanFunction::Eq(std::move(actual_funciton), std::move(expected_function), 1);
                if (bf_eq_res.is_error())
                {
                    return ERR_APPEND(bf_eq_res.get_error(), "failed to concatenate boolean functions");
                }
                auto bf_eq = bf_eq_res.get();

                // z != (a + b)
                auto bf_neq_res = BooleanFunction::Not(std::move(bf_eq), 1);
                if (bf_neq_res.is_error())
                {
                    return ERR_APPEND(bf_eq_res.get_error(), "failed to concatenate boolean functions");
                }
                auto bf_neq = bf_neq_res.get();

                auto solver_result_res = solver.with_constraint(SMT::Constraint(std::move(bf_neq))).query(config);
                if (solver_result_res.is_error())
                {
                    return ERR_APPEND(solver_result_res.get_error(), "failed solver check for module identification utils");
                }
                auto solver_result = solver_result_res.get();

                const u64 solver_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_solver_timing).count();

                if (solver_result.is_unsat())
                {
                    timings["Unsat_Solver_Query"]["Duration"] += solver_duration;
                    timings["Unsat_Solver_Query"]["#Amount"] += 1;
                }

                if (solver_result.is_sat())
                {
                    timings["Sat_Solver_Query"]["Duration"] += solver_duration;
                    timings["Sat_Solver_Query"]["#Amount"] += 1;
                }

                if (solver_result.is_unknown())
                {
                    timings["Unknown_Solver_Query"]["#Amount"] += 1;
                    timings["Unknown_Solver_Query"]["Duration"] += solver_duration;
                }

                return OK({solver_result.is_unsat()});
            }

        }    // namespace

        namespace
        {
            Result<VerifiedCandidate> create_verified_candidate(const FunctionalCandidate& fc, const CandidateType& type, const BooleanFunction& operation)
            {
                return OK(VerifiedCandidate(fc.m_operands,
                                            fc.m_output_nets,
                                            fc.m_control_signals,
                                            {fc.m_control_mapping},
                                            {{fc.m_control_mapping, operation.clone()}},
                                            fc.m_gates,
                                            fc.m_base_gates,
                                            fc.m_additional_data,
                                            {type}));
            }
        }    // namespace

        namespace
        {
            //counter helper
            Result<std::pair<bool, BooleanFunction>> check_count_n(const std::vector<BooleanFunction>& output_functions,
                                                                   BooleanFunction&& bf_i,
                                                                   const std::vector<BooleanFunction::Value>& n_vec,
                                                                   std::map<std::string, std::map<std::string, u64>>& timings)
            {
                // building constraint
                // a + b
                auto increment_vec = n_vec;

                while (bf_i.size() > increment_vec.size())
                {
                    // sign extension
                    const auto msb = n_vec.back();
                    increment_vec.push_back(msb);
                }

                auto increment = BooleanFunction::Const(increment_vec);

                auto bf_add_res = BooleanFunction::Add(std::move(bf_i), std::move(increment), bf_i.size());
                if (bf_add_res.is_error())
                {
                    return ERR_APPEND(bf_add_res.get_error(), "cannot check for counter: failed to build addition Boolean function");
                }
                auto bf_add = bf_add_res.get();

                // if the constructed sum is larger than the original sum we slice it and omit the least significant bits
                if (bf_add.size() > output_functions.size())
                {
                    auto bf_slice_res = BooleanFunction::Slice(std::move(bf_add),
                                                               BooleanFunction::Index(bf_add.size() - output_functions.size(), bf_add.size()),
                                                               BooleanFunction::Index(bf_add.size() - 1, bf_add.size()),
                                                               output_functions.size());

                    if (bf_slice_res.is_error())
                    {
                        return ERR_APPEND(bf_add_res.get_error(), "cannot check for counter: failed to slice Boolean function");
                    }
                    bf_add = bf_slice_res.get();
                }

                auto smt_check_res = smt_check(output_functions, bf_add.clone(), 8, timings);
                if (smt_check_res.is_error())
                {
                    return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
                }

                return OK({smt_check_res.get(), bf_add});
            }

            Result<std::pair<bool, BooleanFunction>>
                check_negation(const std::vector<BooleanFunction>& output_functions, BooleanFunction&& bf_i, std::map<std::string, std::map<std::string, u64>>& timings)
            {
                // building constraint

                // !a
                auto bf_inv_res = BooleanFunction::Not(std::move(bf_i), bf_i.size());
                if (bf_inv_res.is_error())
                {
                    return ERR_APPEND(bf_inv_res.get_error(), "failed to concatenate boolean functions");
                }
                auto bf_inv = bf_inv_res.get();

                // !a + 1
                auto bf_add_res = BooleanFunction::Add(std::move(bf_inv), BooleanFunction::Const(1, bf_inv.size()), bf_inv.size());
                if (bf_add_res.is_error())
                {
                    return ERR_APPEND(bf_add_res.get_error(), "failed to concatenate boolean functions");
                }
                auto bf_add = bf_add_res.get();

                // NOTE this is currently disabled since we are not able to reorder the input bits for the case that the output is truncated
                // if the output is smaller than the input operands we extract the most significant bits
                // if (bf_o.size() < bf_add.size())
                // {
                //     const u32 offset  = bf_add.size() - bf_o.size();
                //     auto bf_slice_res = BooleanFunction::Slice(std::move(bf_add), BooleanFunction::Index(offset, bf_o.size()), BooleanFunction::Index(bf_add.size() - 1, bf_o.size()), bf_o.size());
                //     if (bf_slice_res.is_error())
                //     {
                //         log_error("module_identification", "{}", bf_slice_res.get_error().get());
                //         return false;
                //     }
                //     bf_add = bf_slice_res.get();
                // }

                if (output_functions.size() < bf_add.size())
                {
                    return OK({false, BooleanFunction()});
                }
                auto smt_check_res = smt_check(output_functions, bf_add.clone(), 2, timings);
                if (smt_check_res.is_error())
                {
                    return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
                }

                return OK({smt_check_res.get(), bf_add});
            }

            Result<VerifiedCandidate> check_sum_helper(FunctionalCandidate& fc,
                                                       const std::vector<BooleanFunction>& output_functions,
                                                       const std::vector<std::vector<Net*>>& operands,
                                                       const std::vector<bool>& subtract,
                                                       const CandidateType type,
                                                       std::map<std::string, std::map<std::string, u64>>& timings)
            {
                u32 first_op_idx = 0;
                for (u32 op_idx = 0; op_idx < operands.size(); op_idx++)
                {
                    if (!subtract.at(op_idx))
                    {
                        first_op_idx = op_idx;
                        break;
                    }
                }

                auto bf_sum = create_operand(operands.at(first_op_idx));

                // SUM(operands)
                for (u32 op_idx = 0; op_idx < operands.size(); op_idx++)
                {
                    if (op_idx == first_op_idx)
                    {
                        continue;
                    }

                    auto bf_op = create_operand(operands.at(op_idx));
                    auto bf_add_res =
                        subtract.at(op_idx) ? BooleanFunction::Sub(std::move(bf_sum), std::move(bf_op), bf_sum.size()) : BooleanFunction::Add(std::move(bf_sum), std::move(bf_op), bf_sum.size());
                    if (bf_add_res.is_error())
                    {
                        return ERR(bf_add_res.get_error().get());
                    }
                    bf_sum = bf_add_res.get();
                }

                auto smt_check_res = smt_check(output_functions, bf_sum.clone(), 2, timings);
                if (smt_check_res.is_error())
                {
                    return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
                }

                if (smt_check_res.get())
                {
                    const auto op_str = (subtract == std::vector<bool>{true, false}) ? "[-, +]" : (subtract == std::vector<bool>{true, false}) ? ("[+, -]") : "?";
                    fc.add_additional_data("SUM OPERATIONS", op_str);
                    return create_verified_candidate(fc, type, bf_sum);
                }

                return OK(VerifiedCandidate());
            }
        }    // namespace

        Result<VerifiedCandidate> check_leq(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            // create bit vectors
            auto bf_a = create_operand(fc.m_operands.at(0));
            auto bf_b = create_operand(fc.m_operands.at(1));

            // building constraint
            // a <= b
            if (bf_a.is_empty() || bf_b.is_empty())
            {
                return ERR("bit vectors of boolean function could not be generated, aborting...");
            }
            auto bf_leq_res = BooleanFunction::Ule(bf_a.clone(), bf_b.clone(), 1);
            if (bf_leq_res.is_error())
            {
                return ERR_APPEND(bf_leq_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_leq = bf_leq_res.get();

            auto leq_check_res = smt_check(output_functions, bf_leq.clone(), 2, fc.m_timings["SMT_CHECK"]["LESS_EQUAL"]);
            if (leq_check_res.is_error())
            {
                return ERR_APPEND(leq_check_res.get_error(), "failed equal check for less equal candidate");
            }

            if (leq_check_res.get())
            {
                return create_verified_candidate(fc, module_identification::CandidateType::less_equal, bf_leq);
            }

            // building constraint
            // a < b
            auto bf_lt_res = BooleanFunction::Ult(bf_a.clone(), bf_b.clone(), 1);
            if (bf_lt_res.is_error())
            {
                return ERR_APPEND(bf_lt_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_lt = bf_lt_res.get();

            auto lt_check_res = smt_check(output_functions, bf_lt.clone(), 2, fc.m_timings["SMT_CHECK"]["LESS_THAN"]);
            if (lt_check_res.is_error())
            {
                return ERR_APPEND(lt_check_res.get_error(), "failed equal check for less than candidate");
            }

            if (lt_check_res.get())
            {
                return create_verified_candidate(fc, module_identification::CandidateType::less_than, bf_lt);
            }

            // building constraint
            // a <= b
            auto bf_sleq_res = BooleanFunction::Sle(bf_a.clone(), bf_b.clone(), 1);
            if (bf_sleq_res.is_error())
            {
                return ERR_APPEND(bf_sleq_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_sleq = bf_sleq_res.get();

            auto sleq_check_res = smt_check(output_functions, bf_sleq.clone(), 2, fc.m_timings["SMT_CHECK"]["SIGNED_LESS_EQUAL"]);
            if (sleq_check_res.is_error())
            {
                return ERR_APPEND(sleq_check_res.get_error(), "failed equal check for signed less equal candidate");
            }

            if (sleq_check_res.get())
            {
                return create_verified_candidate(fc, module_identification::CandidateType::signed_less_equal, bf_sleq);
            }

            // building constraint
            // a < b
            auto bf_slt_res = BooleanFunction::Slt(bf_a.clone(), bf_b.clone(), 1);
            if (bf_slt_res.is_error())
            {
                return ERR_APPEND(bf_slt_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_slt = bf_slt_res.get();

            auto slt_check_res = smt_check(output_functions, bf_slt.clone(), 2, fc.m_timings["SMT_CHECK"]["SIGNED_LESS_THAN"]);
            if (slt_check_res.is_error())
            {
                return ERR_APPEND(slt_check_res.get_error(), "failed equal check for signed less than candidate");
            }

            if (slt_check_res.get())
            {
                return create_verified_candidate(fc, module_identification::CandidateType::signed_less_than, bf_slt);
            }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> check_add(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            auto bf_sum = create_operand(fc.m_operands.at(0));

            // SUM(operands)
            for (u32 op_idx = 1; op_idx < fc.m_operands.size(); op_idx++)
            {
                auto bf_op      = create_operand(fc.m_operands.at(op_idx));
                auto bf_add_res = BooleanFunction::Add(std::move(bf_sum), std::move(bf_op), bf_sum.size());
                if (bf_add_res.is_error())
                {
                    return ERR_APPEND(bf_add_res.get_error(), "failed to concatenate boolean functions");
                }
                bf_sum = bf_add_res.get();
            }

            // check for a constant offset
            auto bf_o = output_functions.at(0);
            for (u32 i = 1; i < output_functions.size(); i++)
            {
                auto bf_res = BooleanFunction::Concat(output_functions.at(i).clone(), std::move(bf_o), bf_o.size() + 1);
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(), "cannot check for counter: failed to build conactenated output function");
                }
                bf_o = bf_res.get();
            }

            std::map<std::string, BooleanFunction> eval_mapping;
            for (const auto& var : bf_o.get_variable_names())
            {
                eval_mapping.insert({var, BooleanFunction::Const(0, 1)});
            }

            const auto substitution_res = bf_o.substitute(eval_mapping);
            if (substitution_res.is_error())
            {
                return ERR_APPEND(substitution_res.get_error(), "cannot check for constant offset: failed to evaluate output functions for zero input");
            }
            const auto eval = substitution_res.get().simplify_local();

            const auto increment_res = eval.get_constant_value();
            if (increment_res.is_error())
            {
                return ERR_APPEND(increment_res.get_error(), "cannot check for constant offset: failed to get constant value of substituted boolean function");
            }
            const auto increment_vec = increment_res.get();
            const auto increment     = BooleanFunction::Const(increment_vec);

            if (!increment.has_constant_value(0))
            {
                auto bf_sum_plus_offset = BooleanFunction::Add(bf_sum.clone(), increment.clone(), bf_sum.size());
                if (bf_sum_plus_offset.is_error())
                {
                    return ERR_APPEND(bf_sum_plus_offset.get_error(), "failed to build sum boolean functions");
                }
                bf_sum = bf_sum_plus_offset.get();
            }

            auto smt_check_res = smt_check(output_functions, bf_sum.clone(), 8, fc.m_timings["SMT_CHECK"]["ADD"]);
            if (smt_check_res.is_error())
            {
                return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
            }

            if (smt_check_res.get())
            {
                return create_verified_candidate(fc, CandidateType::adder, bf_sum);
            }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> check_sub(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions, const std::vector<std::vector<Gate*>>& registers)
        {
            if (fc.m_operands.size() != 2)
            {
                return OK(VerifiedCandidate());
            }

            std::vector<VerifiedCandidate> possible_results;

            for (u32 pos = 1; pos < output_functions.size(); pos++)
            {
                // auto bf_o = create_operand(output_functions, pos + 1);
                auto bf_a = create_operand(fc.m_operands.at(0), pos + 1);
                auto bf_b = create_operand(fc.m_operands.at(1), pos + 1);

                // building constraint
                // a - b
                auto bf_sum_res = BooleanFunction::Sub(std::move(bf_a), std::move(bf_b), pos + 1);
                if (bf_sum_res.is_error())
                {
                    return ERR_APPEND(bf_sum_res.get_error(), "failed to concatenate boolean functions");
                }
                auto bf_sum = bf_sum_res.get();

                auto smt_check_res = smt_check(output_functions, bf_sum.clone(), 8, fc.m_timings["SMT_CHECK"]["SUB"]);
                if (smt_check_res.is_error())
                {
                    return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
                }

                if (smt_check_res.get())
                {
                    if (pos == (output_functions.size() - 1))
                    {
                        possible_results.push_back(create_verified_candidate(fc, CandidateType::subtraction, bf_sum).get());
                    }
                    else
                    {
                        continue;
                    }
                }

                // first check failed swap -> last input bits
                auto tmp                        = std::move(fc.m_operands.at(0).at(pos - 1));
                fc.m_operands.at(0).at(pos - 1) = std::move(fc.m_operands.at(1).at(pos - 1));
                fc.m_operands.at(1).at(pos - 1) = tmp;

                // bf_o = create_operand(output_functions, pos + 1);
                bf_a = create_operand(fc.m_operands.at(0), pos + 1);
                bf_b = create_operand(fc.m_operands.at(1), pos + 1);

                // a - b
                auto bf_swapped_sum_res = BooleanFunction::Sub(std::move(bf_a), std::move(bf_b), pos + 1);
                if (bf_swapped_sum_res.is_error())
                {
                    return ERR_APPEND(bf_swapped_sum_res.get_error(), "failed to concatenate boolean functions");
                }
                auto bf_swapped_sum = bf_swapped_sum_res.get();

                auto smt_check_res_2 = smt_check(output_functions, bf_swapped_sum.clone(), 8, fc.m_timings["SMT_CHECK"]["SUB"]);
                if (smt_check_res_2.is_error())
                {
                    return ERR_APPEND(smt_check_res_2.get_error(), "failed equal check for constant multiplication candidate");
                }

                if (smt_check_res_2.get())
                {
                    if (pos == (output_functions.size() - 1))
                    {
                        possible_results.push_back(create_verified_candidate(fc, CandidateType::subtraction, bf_swapped_sum).get());
                    }
                    else
                    {
                        continue;
                    }
                }

                if (!possible_results.empty())
                {
                    break;
                }

                // second check failed -> no valid sub
                return OK(VerifiedCandidate());
            }

            // compare possible results against each other based on the amount of source registers for each operand
            std::sort(possible_results.begin(), possible_results.end(), [&registers](const auto& vc1, const auto& vc2) {
                u32 op_reg_score_1 = 0;
                u32 op_reg_score_2 = 0;

                for (const auto& op : vc1.m_operands)
                {
                    op_reg_score_1 += find_neighboring_registers(op, registers).size();
                }

                for (const auto& op : vc2.m_operands)
                {
                    op_reg_score_2 += find_neighboring_registers(op, registers).size();
                }

                return op_reg_score_2 < op_reg_score_1;
            });

            return OK(possible_results.front());
        }

        Result<VerifiedCandidate> check_sliced_add(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            // check whether the outputs require reordering
            // count the input variables to each output function
            std::map<u32, u32> input_var_count;
            for (u32 idx = 0; idx < output_functions.size(); idx++)
            {
                const auto& func = output_functions.at(idx);
                input_var_count.insert({idx, func.get_variable_names().size()});
            }

            std::map<u32, std::vector<u32>> count_to_indices;
            for (const auto& [idx, c] : input_var_count)
            {
                count_to_indices[c].push_back(idx);
            }

            u32 overfull_bins = 0;
            for (const auto& [c, indices] : count_to_indices)
            {
                if (indices.size() > 1)
                {
                    overfull_bins++;
                }
            }

            // we can only handle one overfull bin, if there are no overfull bins reordering is not required
            if (overfull_bins != 1)
            {
                return OK(VerifiedCandidate());
            }

            // we can only handle cases with the "last" bin being the overfull bin
            if (count_to_indices.rbegin()->second.size() == 1)
            {
                return OK(VerifiedCandidate());
            }
            if (count_to_indices.size() < 2)
            {
                return OK(VerifiedCandidate());
            }

            std::vector<u32> next_bit_candidates = count_to_indices.rbegin()->second;

            std::vector<BooleanFunction> ordered_output_functions;
            std::vector<Net*> ordered_output_nets;
            for (const auto& [_c, indices] : count_to_indices)
            {
                if (indices.size() != 1)
                {
                    continue;
                }

                ordered_output_functions.push_back(output_functions.at(indices.front()).clone());
                ordered_output_nets.push_back(fc.m_output_nets.at(indices.front()));
            }

            // SUM(operands)
            auto bf_sum = create_operand(fc.m_operands.at(0));
            for (u32 op_idx = 1; op_idx < fc.m_operands.size(); op_idx++)
            {
                auto bf_op      = create_operand(fc.m_operands.at(op_idx));
                auto bf_add_res = BooleanFunction::Add(std::move(bf_sum), std::move(bf_op), bf_sum.size());
                if (bf_add_res.is_error())
                {
                    return ERR_APPEND(bf_add_res.get_error(), "failed to concatenate boolean functions");
                }
                bf_sum = bf_add_res.get();
            }

            const auto initial_size      = bf_sum.size();
            const auto initial_end_index = ordered_output_functions.size();
            auto initial_i0              = BooleanFunction::Index(0, initial_size);
            auto initial_i1              = BooleanFunction::Index(initial_end_index - 1, initial_size);
            auto initial_bf_slice_res    = BooleanFunction::Slice(bf_sum.clone(), std::move(initial_i0), std::move(initial_i1), initial_end_index);
            if (initial_bf_slice_res.is_error())
            {
                return ERR_APPEND(initial_bf_slice_res.get_error(), "cannot check adder slice: failed to build sliced boolean function");
            }
            auto initial_bf_slice = initial_bf_slice_res.get();

            auto initial_smt_check_res = smt_check(output_functions, std::move(initial_bf_slice), 2, fc.m_timings["SMT_CHECK"]["ADD_SLICED"]);
            if (initial_smt_check_res.is_error())
            {
                return ERR_APPEND(initial_smt_check_res.get_error(), "failed equal check for sliced add candidate");
            }

            if (!initial_smt_check_res.get())
            {
                return OK(VerifiedCandidate());
            }

            while (!next_bit_candidates.empty())
            {
                bool found_valid_idx = false;
                u32 valid_idx;
                for (const auto& idx : next_bit_candidates)
                {
                    auto tmp = ordered_output_functions;
                    tmp.push_back(output_functions.at(idx).clone());

                    const auto size      = bf_sum.size();
                    const auto end_index = tmp.size();
                    auto i0              = BooleanFunction::Index(0, size);
                    auto i1              = BooleanFunction::Index(end_index - 1, size);
                    auto bf_slice_res    = BooleanFunction::Slice(bf_sum.clone(), std::move(i0), std::move(i1), end_index);
                    if (bf_slice_res.is_error())
                    {
                        return ERR_APPEND(bf_slice_res.get_error(), "cannot check adder slice: failed to build sliced boolean function");
                    }
                    auto bf_slice = bf_slice_res.get();

                    auto smt_check_res = smt_check(output_functions, bf_slice.clone(), 2, fc.m_timings["SMT_CHECK"]["ADD_SLICED"]);
                    if (smt_check_res.is_error())
                    {
                        return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
                    }

                    if (smt_check_res.get())
                    {
                        found_valid_idx = true;
                        valid_idx       = idx;

                        if (next_bit_candidates.size() == 1)
                        {
                            return create_verified_candidate(fc, CandidateType::adder, bf_slice);
                        }

                        // TODO check whether we can insert this break
                        break;
                    }
                }

                if (!found_valid_idx)
                {
                    return OK(VerifiedCandidate());
                }

                next_bit_candidates.erase(std::remove(next_bit_candidates.begin(), next_bit_candidates.end(), valid_idx), next_bit_candidates.end());

                ordered_output_functions.push_back(output_functions.at(valid_idx).clone());
                ordered_output_nets.push_back(fc.m_output_nets.at(valid_idx));
            }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> check_add_sub(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions, const std::vector<std::vector<Gate*>>& registers)
        {
            if (fc.m_operands.size() < 2)
            {
                return OK(VerifiedCandidate());
            }

            auto check_add_res = check_add(fc, output_functions);
            if (check_add_res.is_error())
            {
                return check_add_res;
            }
            if (check_add_res.get().is_verified())
            {
                return check_add_res;
            }

            auto check_sub_res = check_sub(fc, output_functions, registers);
            if (check_sub_res.is_error())
            {
                return check_sub_res;
            }
            if (check_sub_res.get().is_verified())
            {
                return check_sub_res;
            }

            // TODO check whether we need this
            // auto check_sliced_add_res = check_sliced_add(fc, output_functions);
            // if (check_sliced_add_res.is_error())
            // {
            //     return check_sliced_add_res;
            // }
            // if (check_sliced_add_res.get().is_verified())
            // {
            //     return check_sliced_add_res;
            // }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> check_value_check(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            auto bf_i = create_operand(fc.m_operands.at(0));

            // create the solver
            auto s      = SMT::Solver();
            auto config = SMT::QueryConfig();
            config.with_model_generation();

#ifdef BITWUZLA_LIBRARY
            auto s_type = SMT::SolverType::Bitwuzla;
            auto s_call = SMT::SolverCall::Library;
            config.with_solver(s_type).with_call(s_call);
#endif

            s = s.with_constraint(SMT::Constraint(output_functions.front().clone()));

            auto first_result_res = s.query(config);
            if (first_result_res.is_error())
            {
                return ERR_APPEND(first_result_res.get_error(), "failed smt run on boolean function");
            }
            auto first_result = first_result_res.get();

            if (!first_result.is_sat())
            {
                return OK(VerifiedCandidate());
            }

            const auto eval_res = first_result.model.value().evaluate(bf_i);
            if (eval_res.is_error())
            {
                return ERR_APPEND(eval_res.get_error(), "failed to evaluate boolean functions");
            }
            auto bf_m = eval_res.get();

            // bf_i == bf_m
            auto bf_eq_res = BooleanFunction::Eq(std::move(bf_i), std::move(bf_m), 1);
            if (bf_eq_res.is_error())
            {
                return ERR_APPEND(bf_eq_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_eq = bf_eq_res.get();

            // bf_i != bf_m
            auto bf_neq_res = BooleanFunction::Not(std::move(bf_eq), 1);
            if (bf_neq_res.is_error())
            {
                return ERR_APPEND(bf_neq_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_neq = bf_neq_res.get();

            s = s.with_constraint(SMT::Constraint(bf_neq.clone()));

            auto second_result_res = s.query(config);
            if (second_result_res.is_error())
            {
                return ERR_APPEND(second_result_res.get_error(), "failed smt run on boolean function");
            }
            auto second_result = second_result_res.get();

            if (!second_result.is_unsat())
            {
                return OK(VerifiedCandidate());
            }

            // TODO this should probably go somewhere else
            // std::map<std::string, std::vector<Net*>> inputs;
            // for (const auto& net : fc.m_operands.at(0))
            // {
            //     auto bf_val_res = first_result.model.value().evaluate(BooleanFunctionNetDecorator(*net).get_boolean_variable());
            //     if (bf_val_res.is_error())
            //     {
            //         return ERR_APPEND(bf_val_res.get_error(), "failed to evaluate boolean function");
            //     }
            //     auto bf_val = bf_val_res.get();

            //     if (bf_val.has_constant_value(0))
            //     {
            //         inputs["ZERO"].push_back(net);
            //     }
            //     else if (bf_val.has_constant_value(1))
            //     {
            //         inputs["ONE"].push_back(net);
            //     }
            // }

            return create_verified_candidate(fc, CandidateType::value_check, BooleanFunction());
        }

        Result<VerifiedCandidate> check_constant_multiplication(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            if (fc.m_operands.size() < 2)
            {
                return OK(VerifiedCandidate());
            }

            auto bf_sum = create_operand(fc.m_operands.at(0));

            // building constraint
            // SUM(operands)
            for (u32 op_idx = 1; op_idx < fc.m_operands.size(); op_idx++)
            {
                auto bf_op      = create_operand(fc.m_operands.at(op_idx));
                auto bf_add_res = BooleanFunction::Add(std::move(bf_sum), std::move(bf_op), bf_sum.size());
                if (bf_add_res.is_error())
                {
                    return ERR_APPEND(bf_add_res.get_error(), "failed to concatenate boolean functions");
                }
                bf_sum = bf_add_res.get();
            }

            auto smt_check_res = smt_check(output_functions, bf_sum.clone(), 2, fc.m_timings["SMT_CHECK"]["CONST_MULT"]);
            if (smt_check_res.is_error())
            {
                return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
            }

            if (smt_check_res.get())
            {
                return create_verified_candidate(fc, CandidateType::constant_multiplication, bf_sum);
            }

            // for candidates with only two oeprands we check for a subtraction larger_operand - smaller_operand
            if (fc.m_operands.size() == 2)
            {
                // a single shift to the right with a subtraction is equivalent to a counter with the lowest bit omitted, therefore we dont check for constant multiplication in that case
                // if (auto it = fc.m_additional_data.find("OPERAND_SHIFTS"); it != fc.m_additional_data.end())
                // {
                //     if (it->second == "-1")
                //     {
                //         return OK(VerifiedCandidate());
                //     }
                // }

                auto sum_check_res_1 = check_sum_helper(fc, output_functions, fc.m_operands, {false, true}, CandidateType::constant_multiplication, fc.m_timings["SMT_CHECK"]["CONST_MULT"]);
                if (sum_check_res_1.is_error())
                {
                    return ERR_APPEND(sum_check_res_1.get_error(), "failed to execute sum_check_helper in first variant");
                }
                if (const auto c = sum_check_res_1.get(); c.is_verified())
                {
                    return OK(c);
                }

                auto sum_check_res_2 = check_sum_helper(fc, output_functions, fc.m_operands, {true, false}, CandidateType::constant_multiplication, fc.m_timings["SMT_CHECK"]["CONST_MULT"]);
                if (sum_check_res_2.is_error())
                {
                    return ERR_APPEND(sum_check_res_2.get_error(), "failed to execute sum_check_helper in first variant");
                }
                if (const auto c = sum_check_res_2.get(); c.is_verified())
                {
                    return OK(c);
                }
            }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> check_counter(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            auto bf_i = create_operand(fc.m_operands.at(0));
            auto bf_o = output_functions.at(0);
            for (u32 i = 1; i < output_functions.size(); i++)
            {
                auto bf_res = BooleanFunction::Concat(output_functions.at(i).clone(), std::move(bf_o), bf_o.size() + 1);
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(), "cannot check for counter: failed to build conactenated output function");
                }
                bf_o = bf_res.get();
            }

            // figure out operand
            std::set<std::string> input_operands = bf_i.get_variable_names();
            std::map<std::string, BooleanFunction> eval_mapping;
            for (const auto& operand : input_operands)
            {
                eval_mapping.insert({operand, BooleanFunction::Const(0, 1)});
            }

            const auto substitution_res = bf_o.substitute(eval_mapping);
            if (substitution_res.is_error())
            {
                return ERR_APPEND(substitution_res.get_error(), "cannot check for counter: failed to evaluate output functions for zero input");
            }
            const auto eval = substitution_res.get().simplify_local();

            const auto increment_res = eval.get_constant_value();
            if (increment_res.is_error())
            {
                return ERR_APPEND(increment_res.get_error(), "cannot check for counter: failed to get constant value of substituted boolean function");
            }
            const auto increment_vec = increment_res.get();
            const auto increment     = BooleanFunction::Const(increment_vec);

            auto counter_check_res = check_count_n(output_functions, bf_i.clone(), increment_vec, fc.m_timings["SMT_CHECK"]["COUNTER"]);
            if (counter_check_res.is_error())
            {
                return ERR_APPEND(counter_check_res.get_error(), "failed to run counter check for value: " + increment.to_string());
            }
            auto [is_counter, bf_count] = counter_check_res.get();
            if (is_counter)
            {
                fc.add_additional_data("COUNTER_INCREMENT", increment.to_string());
                return create_verified_candidate(fc, CandidateType::counter, bf_count);
            }

            auto check_negation_res = check_negation(output_functions, bf_i.clone(), fc.m_timings["SMT_CHECK"]["NEGATION"]);
            if (check_negation_res.is_error())
            {
                return ERR_APPEND(check_negation_res.get_error(), "failed checknegation during check_counter");
            }
            auto [is_negation, bf_neg] = check_negation_res.get();
            if (is_negation)
            {
                return create_verified_candidate(fc, CandidateType::negation, bf_neg);
            }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> check_absolute(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            auto bf_i = create_operand(fc.m_operands.at(0));

            // building constraint
            // !a
            auto bf_inv_res = BooleanFunction::Not(std::move(bf_i), bf_i.size());
            if (bf_inv_res.is_error())
            {
                return ERR_APPEND(bf_inv_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_inv = bf_inv_res.get();

            // !a + 1
            auto bf_neg_res = BooleanFunction::Add(std::move(bf_inv), BooleanFunction::Const(1, bf_inv.size()), bf_inv.size());
            if (bf_neg_res.is_error())
            {
                return ERR_APPEND(bf_neg_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_neg = bf_neg_res.get();

            // a[MSB : MSB]
            auto bf_a_msb_res = BooleanFunction::Slice(bf_i.clone(), BooleanFunction::Index(bf_i.size() - 1, bf_i.size()), BooleanFunction::Index(bf_i.size() - 1, bf_i.size()), 1);
            if (bf_a_msb_res.is_error())
            {
                return ERR_APPEND(bf_a_msb_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_a_msb = bf_a_msb_res.get();

            // if a[MSB:MSB] == 1 ? (!a + 1) : a
            auto bf_a_abs_res = BooleanFunction::Ite(bf_a_msb.clone(), bf_neg.clone(), bf_i.clone(), bf_i.size());
            if (bf_a_abs_res.is_error())
            {
                return ERR_APPEND(bf_a_abs_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_a_abs = bf_a_abs_res.get();

            auto smt_check_res = smt_check(output_functions, bf_a_abs.clone(), 2, fc.m_timings["SMT_CHECK"]["ABSOLUTE"]);
            if (smt_check_res.is_error())
            {
                return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
            }

            if (smt_check_res.get())
            {
                return create_verified_candidate(fc, CandidateType::absolute, bf_a_abs);
            }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> check_equal(FunctionalCandidate& fc, const std::vector<BooleanFunction>& output_functions)
        {
            // try and find the correct order for input
            std::unordered_map<std::string, BooleanFunction::Value> input_mapping;
            std::set<std::string> vars = output_functions[0].get_variable_names();
            // create mapping from name to Net*
            Netlist* nl = fc.m_input_nets.front()->get_netlist();
            std::unordered_map<Net*, std::string> net_mapping;
            std::vector<Net*> input_nets;
            for (std::string cur_name : vars)
            {
                Net* net = BooleanFunctionNetDecorator::get_net_from(nl, cur_name).get();
                input_nets.push_back(net);
                net_mapping.insert(std::make_pair<Net*, std::string>(std::move(net), std::move(cur_name)));
            }

            // set all inputs to zero
            for (std::string cur_var_name : vars)
            {
                input_mapping.insert(std::make_pair<std::string, BooleanFunction::Value>(std::move(cur_var_name), BooleanFunction::Value::ZERO));
            }
            std::vector<std::vector<Net*>> operands;
            operands.push_back(std::vector<Net*>());
            operands.push_back(std::vector<Net*>());

            std::vector<Net*> unused_inputs;
            unused_inputs.insert(unused_inputs.end(), input_nets.begin(), input_nets.end());
            for (Net* cur_input : input_nets)
            {
                if (std::find(unused_inputs.begin(), unused_inputs.end(), cur_input) == unused_inputs.end())
                {
                    // element was already used
                    continue;
                }
                auto firstit = std::find(unused_inputs.begin(), unused_inputs.end(), cur_input);
                unused_inputs.erase(firstit);

                operands.at(0).push_back(cur_input);
                // set input mapping of Net to true to find partner;
                input_mapping[net_mapping[cur_input]] = BooleanFunction::Value::ONE;
                bool found_b                          = false;
                for (Net* cur_b_candidate : unused_inputs)
                {
                    input_mapping[net_mapping[cur_b_candidate]] = BooleanFunction::Value::ONE;
                    Result<BooleanFunction::Value> res          = output_functions[0].evaluate(input_mapping);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "failed to evaluate boolean function");
                    }

                    if (res.get() == BooleanFunction::Value::ONE)
                    {
                        found_b = true;
                        // operand_b.push_back(cur_b_candidate);

                        operands.at(1).push_back(cur_b_candidate);
                        auto it = std::find(unused_inputs.begin(), unused_inputs.end(), cur_b_candidate);
                        unused_inputs.erase(it);
                        break;
                    }
                    else
                    {
                        input_mapping[net_mapping[cur_b_candidate]] = BooleanFunction::Value::ZERO;
                    }
                }
                if (found_b == false)
                {
                    return OK(VerifiedCandidate());
                }
            }

            // now we found an input order for a and b
            // time to evaluate

            auto bf_a = create_operand(operands.at(0));
            auto bf_b = create_operand(operands.at(1));

            if (bf_a.is_empty() || bf_b.is_empty())
            {
                return ERR("bit vectors of boolean function could not be generated, aborting...");
            }

            // building constraint
            // a == b
            auto bf_eq_res = BooleanFunction::Eq(std::move(bf_a), std::move(bf_b), 1);
            if (bf_eq_res.is_error())
            {
                return ERR_APPEND(bf_eq_res.get_error(), "failed to concatenate boolean functions");
            }
            auto bf_eq = bf_eq_res.get();

            auto smt_check_res = smt_check(output_functions, bf_eq.clone(), 2, fc.m_timings["SMT_CHECK"]["EQUAL"]);
            if (smt_check_res.is_error())
            {
                return ERR_APPEND(smt_check_res.get_error(), "failed equal check for constant multiplication candidate");
            }

            if (smt_check_res.get())
            {
                return create_verified_candidate(fc, CandidateType::equal, bf_eq);
            }

            return OK(VerifiedCandidate());
        }

        Result<VerifiedCandidate> FunctionalCandidate::check(const std::vector<BooleanFunction>& output_functions, const std::vector<std::vector<Gate*>>& registers)
        {
            switch (m_candidate_type)
            {
                case module_identification::CandidateType::equal:
                    return check_equal(*this, output_functions);
                    break;
                case module_identification::CandidateType::less_equal:
                    return check_leq(*this, output_functions);
                    break;
                case module_identification::CandidateType::adder:
                    return check_add_sub(*this, output_functions, registers);
                    break;
                case module_identification::CandidateType::absolute:
                    return check_absolute(*this, output_functions);
                    break;
                case module_identification::CandidateType::value_check:
                    return check_value_check(*this, output_functions);
                    break;
                case module_identification::CandidateType::constant_multiplication:
                    return check_constant_multiplication(*this, output_functions);
                    break;
                case module_identification::CandidateType::counter:
                    return check_counter(*this, output_functions);
                    break;
                default:
                    break;
            }
            return ERR("No check function implemented for type" + enum_to_string(m_candidate_type));
        }
    }    // namespace module_identification
}    // namespace hal