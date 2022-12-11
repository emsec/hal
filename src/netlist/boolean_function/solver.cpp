#include "hal_core/netlist/boolean_function/solver.h"

#include "hal_core/netlist/boolean_function/translator.h"
#include "hal_core/netlist/boolean_function/types.h"
#include "hal_core/utilities/process.h"

#include <numeric>
#include <set>

namespace hal
{
    namespace SMT
    {
        namespace Z3
        {
            /// Checks whether a Z3 binary is available on the system.
            Result<std::string> query_binary_path()
            {
                static const std::vector<std::string> z3_binary_paths = {
                    "/usr/bin/z3",
                    "/usr/local/bin/z3",
                    "/opt/homebrew/bin/z3",
                };

                for (const auto& path : z3_binary_paths)
                {
                    if (std::filesystem::exists(path))
                    {
                        return OK(path);
                    }
                }

                return ERR("could not query binary path: no binary found for Z3 solver");
            }

            /**
			 * Queries Z3 with an SMT-LIB input and a query configuration.
			 *
			 * @param[in] input - SMT-LIB input.
			 * @param[in] config - SMT query configuration.
			 * @returns Ok() and status with 
             *      (0) was_killed (true in case process was killed), and 
             *      (1) stdout Stdout of Z3 process on success, 
             *      Err() otherwise
			 */
            Result<std::tuple<bool, std::string>> query(std::string input, const QueryConfig& config)
            {
                auto binary_path = query_binary_path();
                if (binary_path.is_error())
                {
                    return ERR_APPEND(binary_path.get_error(), "could not query Z3: unable to locate binary");
                }

                auto z3 = subprocess::Popen({binary_path.get(),
                                             // read SMT2LIB formula from stdin
                                             "-in",
                                             // kill execution after a given time
                                             "-T:" + std::to_string(config.timeout_in_seconds)},
                                            subprocess::error{subprocess::PIPE},
                                            subprocess::output{subprocess::PIPE},
                                            subprocess::input{subprocess::PIPE});

                z3.send(input.c_str(), input.size());

                auto z3_communication = z3.communicate();

                std::vector<char> output_buf = z3_communication.first.buf;
                std::string output(output_buf.begin(), output_buf.end());

                // TODO:
                // check whether process was terminated (i.e. killed) via the subprocess
                // API to channel this to the caller
                z3.close_input();
                z3.close_output();
                z3.close_error();
                z3.kill();

                return OK({false, output});
            }
        }    // namespace Z3

        namespace Boolector
        {
            /// Checks whether a Boolector binary is available on the system.
            Result<std::string> query_binary_path()
            {
                static const std::vector<std::string> boolector_binary_paths = {
                    "/usr/bin/boolector",
                    "/usr/local/bin/boolector",
                    "/opt/homebrew/bin/boolector",
                };

                for (const auto& path : boolector_binary_paths)
                {
                    if (std::filesystem::exists(path))
                    {
                        return OK(path);
                    }
                }

                return ERR("could not query binary path: no binary found for Boolector solver");
            }

            /**
			 * Queries Boolector with an SMT-LIB input and a query configuration.
			 *
			 * @param[in] input - SMT-LIB input.
			 * @param[in] config - SMT query configuration.
			 * @returns Ok() and status with 
             *      (0) was_killed (true in case process was killed), and 
             *      (1) stdout Stdout of Z3 process on success, 
             *      Err() otherwise
			 */
            Result<std::tuple<bool, std::string>> query(std::string input, const QueryConfig& config)
            {
                auto binary_path = query_binary_path();
                if (binary_path.is_error())
                {
                    return ERR_APPEND(binary_path.get_error(), "could not query Boolector: unable to locate binary");
                }

                auto boolector = subprocess::Popen(
                    {
                        binary_path.get(),
                        // NOTE the boolector binary provided as package takes different parameters. The ones below do not seem to work.
                        // kill execution after a given time
                        "--time=" + std::to_string(config.timeout_in_seconds),
                        // generate SMT-LIB v2 compatible output
                        "--output-format=smt2",
                        // set model generation if required
                        std::string("--model-gen=") + ((config.generate_model) ? "1" : "0"),
                    },
                    subprocess::output{subprocess::PIPE},
                    subprocess::input{subprocess::PIPE});

                boolector.send(input.c_str(), input.size());
                auto boolector_communication = boolector.communicate();

                std::vector<char> output_buf = boolector_communication.first.buf;
                std::string output(output_buf.begin(), output_buf.end());

                // TODO:
                // check whether process was terminated (i.e. killed) via the subprocess
                // API to channel this to the caller
                boolector.close_input();
                boolector.close_output();
                boolector.close_error();
                boolector.kill();
                return OK({false, output});
            }
        }    // namespace Boolector

        Solver::Solver(const std::vector<Constraint>& constraints) : m_constraints(constraints)
        {
        }

        Solver& Solver::with_constraint(const Constraint& constraint)
        {
            this->m_constraints.emplace_back(std::move(constraint));
            return *this;
        }

        Solver& Solver::with_constraints(const std::vector<Constraint>& constraints)
        {
            for (auto&& constraint : constraints)
            {
                this->m_constraints.emplace_back(std::move(constraint));
            }
            return *this;
        }

        const std::vector<Constraint>& Solver::get_constraints() const
        {
            return m_constraints;
        }

        bool Solver::has_local_solver_for(SolverType solver)
        {
            static const std::map<SolverType, std::function<Result<std::string>()>> type2query = {
                {SolverType::Z3, Z3::query_binary_path},
                {SolverType::Boolector, Boolector::query_binary_path},
            };

            switch (auto it = type2query.find(solver); it != type2query.end())
            {
                case true:
                    return it->second().is_ok();
                default:
                    return false;
            }
        }

        Result<SolverResult> Solver::query(const QueryConfig& config) const
        {
            if (config.local)
            {
                if (auto res = this->query_local(config); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "unable to query SMT solver: local query failed");
                }
                else
                {
                    return res;
                }
            }
            else
            {
                if (auto res = this->query_remote(config); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "unable to query SMT solver: remote query failed");
                }
                else
                {
                    return res;
                }
            }
        }

        Result<SolverResult> Solver::query_local(const QueryConfig& config) const
        {
            static const std::map<SolverType, std::function<Result<std::tuple<bool, std::string>>(std::string, const QueryConfig&)>> type2query = {
                {SolverType::Z3, Z3::query},
                {SolverType::Boolector, Boolector::query},
            };

            auto input = Solver::translate_to_smt2(this->m_constraints, config);
            if (input.is_error())
            {
                return ERR_APPEND(input.get_error(), "could not query local SMT solver: unable to translate SMT constraints and configuration to string");
            }

            auto query = type2query.at(config.solver)(input.get(), config);
            if (query.is_ok())
            {
                auto [was_killed, output] = query.get();
                return Solver::translate_from_smt2(was_killed, output, config);
            }
            return ERR_APPEND(query.get_error(), "could not query local SMT solver: unable to parse SMT result from string");
        }

        Result<SolverResult> Solver::query_remote(const QueryConfig& /* config */) const
        {
            // unimplemented as this is feature not required at the moment
            return ERR("could not query remote SMT solver: currently not supported");
        }

        Result<std::string> Solver::translate_to_smt2(const std::vector<Constraint>& constraints, const QueryConfig& config)
        {
            /// Helper to translate variable declarations to an SMT-LIB v2 string representation.
            ///
            /// @param[in] constraints - List of constraints to translate.
            /// @returns List of variable declarations.
            auto translate_declarations = [](const std::vector<Constraint>& _constraints) -> std::string {
                std::set<std::tuple<std::string, u16>> inputs;
                for (const auto& constraint : _constraints)
                {
                    if (constraint.is_assignment())
                    {
                        for (const auto& node : constraint.get_assignment().get()->first.get_nodes())
                        {
                            if (node.is_variable())
                            {
                                inputs.insert(std::make_tuple(node.variable, node.size));
                            }
                        }
                    }
                    else
                    {
                        for (const auto& node : constraint.get_function().get()->get_nodes())
                        {
                            if (node.is_variable())
                            {
                                inputs.insert(std::make_tuple(node.variable, node.size));
                            }
                        }
                    }
                }

                return std::accumulate(inputs.begin(), inputs.end(), std::string(), [](auto accumulator, auto entry) -> std::string {
                    return accumulator + "(declare-fun " + std::get<0>(entry) + " () (_ BitVec " + std::to_string(std::get<1>(entry)) + "))\n";
                });
            };

            /// Helper to translate constraints to an SMT-LIB v2 string representation.
            ///
            /// @param[in] constraints - List of constraints to translate.
            /// @returns Ok() and constraints as string on success, Err() otherwise.
            auto translate_constraints = [](const std::vector<Constraint>& _constraints) -> Result<std::string> {
                return std::accumulate(_constraints.cbegin(), _constraints.cend(), Result<std::string>::Ok({}), [](auto accumulator, const auto& constraint) -> Result<std::string> {
                    // (1) short-hand termination in case accumulator is an error
                    if (accumulator.is_error())
                    {
                        return ERR(accumulator.get_error());
                    }

                    if (constraint.is_assignment())
                    {
                        const auto assignment = constraint.get_assignment().get();
                        auto lhs              = Translator::translate_to_smt2(assignment->first);
                        auto rhs              = Translator::translate_to_smt2(assignment->second);
                        if (lhs.is_error())
                        {
                            return ERR_APPEND(lhs.get_error(), "could not translate constraint to SMT-LIB v2: '" + constraint.to_string() + "'");
                        }
                        else if (rhs.is_error())
                        {
                            return ERR_APPEND(rhs.get_error(), "could not translate constraint to SMT-LIB v2: '" + constraint.to_string() + "'");
                        }
                        else
                        {
                            return OK(accumulator.get() + "(assert (= " + lhs.get() + " " + rhs.get() + "))\n");
                        }
                    }
                    else
                    {
                        auto lhs = Translator::translate_to_smt2(*constraint.get_function().get());
                        if (lhs.is_ok())
                        {
                            return OK(accumulator.get() + "(assert (= #b1 " + lhs.get() + "))\n");
                        }
                        return ERR_APPEND(lhs.get_error(), "could not translate constraint to SMT-LIB v2: '" + constraint.to_string() + "'");
                    }
                });
            };

            auto theory          = std::string("(set-logic QF_ABV)");
            auto declarations    = translate_declarations(constraints);
            auto constraints_str = translate_constraints(constraints);
            auto epilogue        = std::string("(check-sat)") + ((config.generate_model) ? "\n(get-model)" : "");

            if (constraints_str.is_ok())
            {
                return OK(theory + "\n" + declarations + "\n" + constraints_str.get() + "\n" + epilogue);
            }
            return ERR_APPEND(constraints_str.get_error(), "could not translate constraint to SMT-LIB v2: unable to generate translation constraints");
        }

        Result<SolverResult> Solver::translate_from_smt2(bool was_killed, std::string stdout, const QueryConfig& config)
        {
            if (was_killed)
            {
                return ERR("could not parse SMT result from string: the SMT solver was killed");
            }

            auto position            = stdout.find_first_of('\n');
            auto [result, model_str] = std::make_tuple(std::string(stdout, 0, position), std::string(stdout, position + 1));

            auto to_lowercase = [](const auto& s) -> std::string {
                auto lowercase = s;
                std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
                return lowercase;
            };

            if (to_lowercase(result) == "sat")
            {
                if (config.generate_model)
                {
                    if (auto res = Model::parse(model_str, config.solver).map<SolverResult>([](const auto& model) -> Result<SolverResult> { return OK(SolverResult::Sat(model)); }); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not parse SMT result from string: unable to generate model");
                    }
                    else
                    {
                        return res;
                    }
                }

                return OK(SolverResult::Sat());
            }
            if (to_lowercase(result) == "unsat")
            {
                return OK(SolverResult::UnSat());
            }

            if ((to_lowercase(result) == "unknown") || result.rfind("[btor>main] ALARM TRIGGERED: time limit", 0))
            {
                return OK(SolverResult::Unknown());
            }

            return ERR("could not parse SMT result from string: invalid result");
        }
    }    // namespace SMT
}    // namespace hal