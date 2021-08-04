//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/netlist/boolean_function/types.h"

namespace hal 
{
namespace SMT {
	/**
	 * SMT::Solver is used to query SMT solvers for a list of constraints, i.e. 
	 * statements that have to be equal. To this end, we translate constraints
	 * to a SMT-LIB v2 string representation and query solvers with a defined 
	 * configuration, i.e. chosen solver, model generation etc.
	 */
	class Solver final {
	public:
        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

		/**
		 * Creates an 'Solver' instance with an optional list of constraints.
		 *
		 * @param[in] constraints - List of constraints.
		 * @returns An initialized SMT solver instance.
		 */
		Solver(std::vector<Constraint>&& constraints = {});

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

		/**
		 * Returns list of available constraints.
		 *
		 * @returns List of available constraints.
		 */
		const std::vector<Constraint>& get_constraints() const;

		/**
		 * Appends a constraint to an SMT solver.
		 *
		 * @param[in] constraint - SMT solver constraint.
		 * @returns Self reference.
		 */
		Solver& with_constraint(Constraint&& constraint);

		/**
		 * Appends a list of constraints to an SMT solver.
		 *
		 * @param[in] constraints - SMT solver constraints.
		 * @returns Self reference.
		 */
		Solver& with_constraints(std::vector<Constraint>&& constraints);

		/**
		 * Checks whether a SMT solver is available on the local machine.
		 *
		 * @param[in] type - SMT solver identifier.
		 * @returns true in case SMT solver is available, false otherwise.
		 */	
 		static bool has_local_solver_for(SolverType type);

 		/**
 		 * Queries an SMT solver with a specified configuration.
 		 *
 		 * Example:
		 *		auto [ok, result] = SMT::Solver()
		 *		.with_constraint(SMT::Constraint(BooleanFunction("A") & BooleanFunction("B"), BooleanFunction::ONE))
		 *		.with_constraint(SMT::Constraint(BooleanFunction("A"), BooleanFunction::ONE))
		 *		.query(SMT::QueryConfig()
		 *    		.with_solver(SMT::SolverType::Z3)
		 *    		.with_model_generation()
		 *    		.with_timeout(5) // seconds
		 *		);
		 *
		 *		if (ok && result.is(SMT::ResultType::Sat)) {}
 		 *
 		 * @param[in] config - SMT solver configuration.
 		 * @returns (1) status (true on success, false otherwise), and 
 		 *          (2) SMT solver result.
 		 */
		std::tuple<bool, Result> query(const QueryConfig& config) const;
		
		/**
 		 * Queries a local SMT solver instance with a specified configuration.
 		 *
 		 * @param[in] config - SMT solver configuration.
 		 * @returns (1) status (true on success, false otherwise), and 
 		 *          (2) SMT solver result.
 		 */
		std::tuple<bool, Result> query_local(const QueryConfig& config) const;

		/**
 		 * Queries a local SMT solver instance with a specified configuration.
 		 *
 		 * WARNING: This function is not implemented and always returns with a 
 		 *          status == false, currently.
 		 *
 		 * @param[in] config - SMT solver configuration.
 		 * @returns (1) status (true on success, false otherwise), and 
 		 *          (2) SMT solver result.
 		 */
		std::tuple<bool, Result> query_remote(const QueryConfig& config) const;

	private:
        ////////////////////////////////////////////////////////////////////////
        // Member
        ////////////////////////////////////////////////////////////////////////

		/// stores list of SMT solver constraints
		std::vector<Constraint> m_constraints;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

		/**
		 * Translate a list of constraint and a configuration to SMT-LIB v2.
		 *
		 * NOTE: For details on the SMT-LIB language standard see: 
		 * 		 https://smtlib.cs.uiowa.edu/language.shtml
		 *
		 * @param[in] constraints - List of constraints.
		 * @param[in] config - SMT solver configuration.
		 * @returns (1) status (true on success, false otherwise), and
		 *          (2) SMT-LIB compatible string representation
		 */
		static std::tuple<bool, std::string> translate_to_smt2(const std::vector<Constraint>& constraints, const QueryConfig& config);

		/**
		 * Translate a SMT solver result from SMT-LIB v2.
		 *
		 * NOTE: For details on the SMT-LIB language standard see: 
		 * 		 https://smtlib.cs.uiowa.edu/language.shtml
		 *
		 * @param[in] was_killed - Indicates whether solver process was killed.
		 * @param[in] stdout - Stdout of solver process.
		 * @param[in] config - SMT solver configuration.
		 * @returns (1) status (true on success, false otherwise), and
		 *          (2) SMT solver result
		 */
		static std::tuple<bool, Result> translate_from_smt2(bool was_killed, std::string stdout, const QueryConfig& config);
	};
}  // namespace SMT
}  // namespace hal