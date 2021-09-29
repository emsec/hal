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
	std::tuple<bool, std::string> query_binary_path() 
	{
		static const std::vector<std::string> z3_binary_paths = 
		{
			"/usr/bin/z3",
			"/usr/local/bin/z3",
		};

		for (const auto& path : z3_binary_paths) 
		{
			if (std::filesystem::exists(path)) 
			{
				return {true, path};
			}
		}

		return {false, ""};
	}

	/**
	 * Queries Z3 with an SMT-LIB input and a query configuration.
	 *
	 * @param[in] input - SMT-LIB input.
	 * @param[in] config - SMT query configuration.
	 * @returns (1) status (true on success, false otherwise),  
	 *          (2) was_killed (true in case process was killed),
	 *          (3) stdout Stdout of Z3 process.
	 */
	std::tuple<bool, bool, std::string> query(std::string input, const QueryConfig& config) 
	{
		auto [binary_path_ok, binary_path] = query_binary_path();
		if (!binary_path_ok) 
		{
			return {false, false, ""};
		}	

		auto z3 = subprocess::Popen(
			{
				binary_path, 
				// read SMT2LIB formula from stdin
				"-in",
				// kill execution after a given time
				 "-t:" + std::to_string(config.timeout_in_seconds)
			},
			subprocess::output{subprocess::PIPE}, 
			subprocess::input{subprocess::PIPE}
		);

		z3.send(input.c_str(), input.size());

		// TODO:
		// check whether process was terminated (i.e. killed) via the subprocess
		// API to channel this to the caller
		return {true, false, z3.communicate().first.buf.data()};
	}
}  // namespace Z3

namespace Boolector {
	/// Checks whether a Boolector binary is available on the system.
	std::tuple<bool, std::string> query_binary_path() 
	{
		static const std::vector<std::string> boolector_binary_paths = 
		{
			"/usr/bin/boolector",
			"/usr/local/bin/boolector",
		};

		for (const auto& path : boolector_binary_paths) 
		{
			if (std::filesystem::exists(path)) 
			{
				return {true, path};
			}
		}

		return {false, ""};
	}

	/**
	 * Queries Boolector with an SMT-LIB input and a query configuration.
	 *
	 * @param[in] input - SMT-LIB input.
	 * @param[in] config - SMT query configuration.
	 * @returns (1) status (true on success, false otherwise),  
	 *          (2) was_killed (true in case process was killed),
	 *          (3) stdout Stdout of Boolector process.
	 */
	std::tuple<bool, bool, std::string> query(std::string input, const QueryConfig& config) 
	{
		auto [binary_path_ok, binary_path] = query_binary_path();
		if (!binary_path_ok) 
		{
			return {false, false, ""};
		}	

		auto boolector = subprocess::Popen(
			{
				binary_path,
				// kill execution after a given time 
				"--time=" + std::to_string(config.timeout_in_seconds),
				// generate SMT-LIB v2 compatible output
				"--output-format=smt2",
				// set model generation if required
				std::string("--model-gen=") + ((config.generate_model) ? "1" : "0"),
			},
			subprocess::output{subprocess::PIPE}, 
			subprocess::input{subprocess::PIPE}
		);

		boolector.send(input.c_str(), input.size());

		// TODO:
		// check whether process was terminated (i.e. killed) via the subprocess
		// API to channel this to the caller
		return {true, false, boolector.communicate().first.buf.data()};
	}
}  // namespace Boolector

	Solver::Solver(std::vector<Constraint>&& constraints) : 
		m_constraints(constraints) {}

	Solver& Solver::with_constraint(Constraint&& constraint) 
	{
		this->m_constraints.emplace_back(constraint);
		return *this;
	}

	Solver& Solver::with_constraints(std::vector<Constraint>&& constraints) 
	{
		for (auto&& constraint : constraints) {
			this->m_constraints.emplace_back(constraint);
		}
		return *this;
	}

	const std::vector<Constraint>& Solver::get_constraints() const 
	{
		return m_constraints;
	}

	bool Solver::has_local_solver_for(SolverType solver) 
	{
		static const std::map<SolverType, std::function<std::tuple<bool, std::string>()>> type2query = 
		{
			{SolverType::Z3, Z3::query_binary_path},
			{SolverType::Boolector, Boolector::query_binary_path},
		};

		switch (auto it = type2query.find(solver); it != type2query.end()) {
			case true: return std::get<0>(it->second());
			default:   return false;
		}
	}

	std::tuple<bool, Result> Solver::query(const QueryConfig& config) const 
	{
		return (config.local) ? this->query_local(config) : this->query_remote(config);
	}

	std::tuple<bool, Result> Solver::query_local(const QueryConfig& config) const 
	{
		static const std::map<SolverType, std::function<std::tuple<bool, bool, std::string>(std::string, const QueryConfig&)>> type2query = 
		{
			{SolverType::Z3, Z3::query},
			{SolverType::Boolector, Boolector::query},
		};

		auto [input_ok, input] = Solver::translate_to_smt2(this->m_constraints, config);
		if (!input_ok) 
		{
			return {false, Result::Unknown()};
		}

		switch (auto [output_ok, was_killed, output] = type2query.at(config.solver)(input, config); output_ok) {
			case true: return Solver::translate_from_smt2(was_killed, output, config);
			default:   return {false, Result::Unknown()};
		}
	}

	std::tuple<bool, Result> Solver::query_remote(const QueryConfig& /* config */) const 
	{
		// unimplemented as this is feature not required at the moment
		return {false, Result::Unknown()};
	}

	std::tuple<bool, std::string> Solver::translate_to_smt2(const std::vector<Constraint>& constraints, const QueryConfig& config) 
	{
		auto translate_declarations = [] (const std::vector<Constraint>& _constraints) -> std::string {
			std::set<std::tuple<std::string, u16>> inputs;
			for (const auto& constraint : _constraints) {
				for (const auto& node : constraint.lhs.get_reverse_polish_notation()) {
					if (auto variable = node->get_as<BooleanFunction::OperandNode>(); node->is_variable()) {
						inputs.insert(std::make_tuple(variable->variable, node->size));
					}
				}
			}

			return std::accumulate(inputs.begin(), inputs.end(), std::string(), 
				[] (auto accumulator, auto entry) -> std::string {
					return accumulator + "(declare-fun " + std::get<0>(entry) + " () (_ BitVec " + std::to_string(std::get<1>(entry)) + "))\n";
				}
			);
		};

		auto translate_constraints = [] (const std::vector<Constraint>& _constraints) -> std::tuple<bool, std::string> {				
			return std::accumulate(_constraints.begin(), _constraints.end(), std::make_tuple(true, std::string()),
				[] (auto state , auto constraint) -> std::tuple<bool, std::string> {
					auto [ok, accumulator] = state;

					auto [lhs_ok, lhs] = Translator::translate_to_smt2(constraint.lhs);
					auto [rhs_ok, rhs] = Translator::translate_to_smt2(constraint.rhs);

					switch (ok && lhs_ok && rhs_ok) {
						case true: return {true, accumulator + "(assert (= " + lhs + " " + rhs + "))\n"};
						default:   return {false, ""};
					}
				}
			);
		};

		auto theory = std::string("(set-logic QF_ABV)");
		auto declarations = translate_declarations(constraints);
		auto [constraints_ok, constraints_str] = translate_constraints(constraints);
		auto epilogue = std::string("(check-sat)") + ((config.generate_model) ? "\n(get-model)" : "");

		switch (constraints_ok) {
			case true: return {true, theory + "\n" + declarations + "\n" + constraints_str + "\n" + epilogue};
			default:   return {false, ""};
		}		
	}

	std::tuple<bool, Result> Solver::translate_from_smt2(bool was_killed, std::string stdout, const QueryConfig& config) {		
		if (was_killed) {
			return {true, Result::Unknown()};
		}

		auto position = stdout.find_first_of('\n');
		auto [result, model_str] = std::make_tuple(std::string(stdout, 0, position), std::string(stdout, position + 1));

		auto to_lowercase = [] (const auto& s) -> std::string {
			auto lowercase = s;
			std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
			return lowercase;
		};

		if (to_lowercase(result) == "sat")
		{
			switch (config.generate_model) {
				case true: {
					auto [ok, model] = Model::parse(model_str, config.solver);
					return {ok, Result::Sat(model)};
				}
				default: return {true, Result::Sat()};
			}
		}
		if (to_lowercase(result) == "unsat")
		{
			return {true, Result::UnSat()};
		}

		if ((to_lowercase(result) == "unknown")
			|| result.rfind("[btor>main] ALARM TRIGGERED: time limit", 0)) 
		{
			return {true, Result::Unknown()};
		}

		return {false, Result::Unknown()};
	}
}  // namespace SMT
}  // namespace hal