#include "hal_core/netlist/boolean_function/solver.h"

#include "hal_core/netlist/boolean_function/translator.h"
#include "hal_core/netlist/boolean_function/types.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "subprocess/process.h"

#include <boost/thread.hpp>
#include <ctime>
#include <fstream>
#include <numeric>
#include <set>

#ifdef BITWUZLA_LIBRARY
#include "bitwuzla/cpp/bitwuzla.h"
#include "bitwuzla/cpp/parser.h"
#endif
namespace hal
{
    namespace SMT
    {
        namespace Z3
        {
            bool is_linked = false;

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
            Result<std::tuple<bool, std::string>> query_binary(std::string& input, const QueryConfig& config)
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
                // log_info("solver", "\ninput: \n{}\n\noutput:{}\n", input, output);

                return OK({false, output});
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
            Result<std::tuple<bool, std::string>> query_library(std::string& input, const QueryConfig& config)
            {
                // z3::context ctx;

                // z3::solver s = {ctx};
                // s.from_string(input.c_str());

                // const auto res = s.check();

                // if (res == z3::unknown)
                // {
                //     return OK({true, "unknown"});
                // }

                // if (res == z3::unsat)
                // {
                //     return OK({false, "unsat"});
                // }

                // // std::cout << s.get_model().to_string() << std::endl;
                return OK({false, "unsat"});
                // return OK({false, s.get_model().to_string()});
            }
        }    // namespace Z3

        namespace Boolector
        {
            bool is_linked = false;

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
             *      (1) stdout Stdout of Boolector process on success, 
             *      Err() otherwise
			 */
            Result<std::tuple<bool, std::string>> query_binary(std::string& input, const QueryConfig& config)
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

            /**
			 * Queries Boolector with an SMT-LIB input and a query configuration.
			 *
			 * @param[in] input - SMT-LIB input.
			 * @param[in] config - SMT query configuration.
			 * @returns Ok() and status with 
             *      (0) was_killed (true in case process was killed), and 
             *      (1) stdout Stdout of Boolector process on success, 
             *      Err() otherwise
			 */
            Result<std::tuple<bool, std::string>> query_library(std::string& input, const QueryConfig& config)
            {
                UNUSED(input);
                UNUSED(config);
                return ERR("could not call Boolector solver library: Library call not implemented");
            }
        }    // namespace Boolector

        namespace Bitwuzla
        {
#ifdef BITWUZLA_LIBRARY
            bool is_linked = true;
#else
            bool is_linked = false;
#endif

            /// Checks whether a Bitwuzla binary is available on the system.
            Result<std::string> query_binary_path()
            {
                static const std::vector<std::string> bitwuzla_binary_paths = {
                    "/usr/bin/bitwuzla",
                    "/usr/local/bin/bitwuzla",
                    "/opt/homebrew/bin/bitwuzla",
                };

                for (const auto& path : bitwuzla_binary_paths)
                {
                    if (std::filesystem::exists(path))
                    {
                        return OK(path);
                    }
                }

                return ERR("could not query binary path: no binary found for Bitwuzla solver");
            }

            /**
			 * Queries Bitwuzla with an SMT-LIB input and a query configuration.
			 *
			 * @param[in] input - SMT-LIB input.
			 * @param[in] config - SMT query configuration.
			 * @returns Ok() and status with 
             *      (0) was_killed (true in case process was killed), and 
             *      (1) stdout Stdout of Bitwuzla process on success, 
             *      Err() otherwise
			 */
            Result<std::tuple<bool, std::string>> query_library(std::string& input, const QueryConfig& config)
            {
#ifdef BITWUZLA_LIBRARY

                // First, create a Bitwuzla options instance.
                bitwuzla::Options options;
                // We will parse example file `smt2/quickstart.smt2`.
                // Create parser instance.
                // We expect no error to occur.
                const char* smt2_char_string = input.c_str();

                auto in_stream = fmemopen((void*)smt2_char_string, strlen(smt2_char_string), "r");
                std::stringbuf result_string;
                std::ostream output_stream(&result_string);

                std::vector<bitwuzla::Term> all_vars;

                if (config.generate_model)
                {
                    options.set(bitwuzla::Option::PRODUCE_MODELS, true);
                }

                // std::filesystem::path tmp_path = utils::get_unique_temp_directory().get();
                // std::string output_file        = std::string(tmp_path) + "/out.smt2";

                bitwuzla::parser::Parser parser(options, "VIRTUAL_FILE", in_stream, "smt2", &output_stream);
                // Now parse the input file.
                std::string err_msg = parser.parse(false);

                if (!err_msg.empty())
                {
                    return ERR("failed to parse input file: " + err_msg);
                }

                fclose(in_stream);

                std::string output(result_string.str());
                // std::cout << "output" << std::endl;
                // std::cout << output << std::endl;
                return OK({false, output});
#else
                return ERR("Bitwuzla Library not linked!");
#endif
            }

            /**
			 * Queries Bitwuzla with an SMT-LIB input and a query configuration.
			 *
			 * @param[in] input - SMT-LIB input.
			 * @param[in] config - SMT query configuration.
			 * @returns Ok() and status with 
             *      (0) was_killed (true in case process was killed), and 
             *      (1) stdout Stdout of Bitwuzla process on success, 
             *      Err() otherwise
			 */
            Result<std::tuple<bool, std::string>> query_binary(std::string& input, const QueryConfig& config)
            {
                auto binary_path = query_binary_path();
                if (binary_path.is_error())
                {
                    return ERR_APPEND(binary_path.get_error(), "could not query Bitwuzla: unable to locate binary");
                }

                // TODO check how to timeout bitwuzla
                UNUSED(config);

                return ERR("could not query Bitwuzla: binary call not implemented");

                /*
                auto bitwuzla = subprocess::Popen(
                    {
                        binary_path.get(),
                    },
                    subprocess::error{subprocess::PIPE},
                    subprocess::output{subprocess::PIPE},
                    subprocess::input{subprocess::PIPE});

                bitwuzla.send(input.c_str(), input.size());

                auto bitwuzla_communication = bitwuzla.communicate();

                std::vector<char> output_buf = bitwuzla_communication.first.buf;
                std::string output(output_buf.begin(), output_buf.end());

                // TODO:
                // check whether process was terminated (i.e. killed) via the subprocess
                // API to channel this to the caller
                bitwuzla.close_input();
                bitwuzla.close_output();
                bitwuzla.close_error();
                bitwuzla.kill();

                return OK({false, output});
                */
            }
        }    // namespace Bitwuzla

        std::map<SolverType, std::function<Result<std::string>()>> Solver::type2query_binary = {
            {SolverType::Z3, Z3::query_binary_path},
            {SolverType::Boolector, Boolector::query_binary_path},
            {SolverType::Bitwuzla, Bitwuzla::query_binary_path},
        };

        std::map<SolverType, bool> Solver::type2link_status = {
            {SolverType::Z3, Z3::is_linked},
            {SolverType::Boolector, Boolector::is_linked},
            {SolverType::Bitwuzla, Bitwuzla::is_linked},
        };

        std::map<std::pair<SolverType, SolverCall>, std::function<Result<std::tuple<bool, std::string>>(std::string&, const QueryConfig&)>> Solver::spec2query = {
            {{SolverType::Z3, SolverCall::Binary}, Z3::query_binary},
            {{SolverType::Z3, SolverCall::Library}, Z3::query_library},
            {{SolverType::Boolector, SolverCall::Binary}, Boolector::query_binary},
            {{SolverType::Boolector, SolverCall::Library}, Boolector::query_library},
            {{SolverType::Bitwuzla, SolverCall::Binary}, Bitwuzla::query_binary},
            {{SolverType::Bitwuzla, SolverCall::Library}, Bitwuzla::query_library},
        };

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

        bool Solver::has_local_solver_for(SolverType type, SolverCall call)
        {
            if (call == SolverCall::Binary)
            {
                switch (auto it = type2query_binary.find(type); it != type2query_binary.end())
                {
                    case true:
                        return it->second().is_ok();
                    default:
                        return false;
                }
            }
            else if (call == SolverCall::Library)
            {
                switch (auto it = type2link_status.find(type); it != type2link_status.end())
                {
                    case true:
                        return it->second;
                    default:
                        return false;
                }
            }

            return false;
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
            auto input = Solver::translate_to_smt2(this->m_constraints, config);
            if (input.is_error())
            {
                return ERR_APPEND(input.get_error(), "could not query local SMT solver: unable to translate SMT constraints and configuration to string");
            }

            auto input_str = input.get();
            return query_local(config, input_str);
        }

        Result<SolverResult> Solver::query_local(const QueryConfig& config, std::string& smt2)
        {
            auto query = spec2query.at({config.solver, config.call})(smt2, config);
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

        Result<std::string> Solver::to_smt2(const QueryConfig& config) const
        {
            return translate_to_smt2(this->m_constraints, config);
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

            auto preamble        = ((config.generate_model) ? std::string("(set-option :produce-models true)\n") : "");
            auto theory          = std::string("(set-logic QF_ABV)");
            auto declarations    = translate_declarations(constraints);
            auto constraints_str = translate_constraints(constraints);
            auto epilogue        = std::string("(check-sat)") + ((config.generate_model) ? "\n(get-model)" : "");

            if (constraints_str.is_ok())
            {
                return OK(preamble + theory + "\n" + declarations + "\n" + constraints_str.get() + "\n" + epilogue);
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
