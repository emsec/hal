// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/netlist/boolean_function/types.h"

namespace hal
{
    namespace SMT
    {
        /**
		 * Provides an interface to query SMT solvers for a list of constraints, i.e. 
		 * statements that have to be equal. To this end, we translate constraints
		 * to a SMT-LIB v2 string representation and query solvers with a defined 
		 * configuration, i.e., chosen solver, model generation etc.
		 */
        class Solver final
        {
        public:
            ////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////

            /**
			 * Constructs an solver with an optional list of constraints.
			 *
			 * @param[in] constraints - The (optional) list of constraints.
			 */
            Solver(const std::vector<Constraint>& constraints = {});

            ////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////

            /**
			 * Returns the vector of constraints.
			 *
			 * @returns The vector of constraints.
			 */
            const std::vector<Constraint>& get_constraints() const;

            /**
			 * Adds a constraint to the SMT solver.
			 *
			 * @param[in] constraint - The constraint.
			 * @returns The updated SMT solver.
			 */
            Solver& with_constraint(const Constraint& constraint);

            /**
			 * Adds a vector of constraints to the SMT solver.
			 *
			 * @param[in] constraints - the constraints.
			 * @returns The updated SMT solver.
			 */
            Solver& with_constraints(const std::vector<Constraint>& constraints);

            /**
			 * Checks whether a SMT solver of the given type is available on the local machine.
			 *
			 * @param[in] type - The SMT solver type.
			 * @param[in] call - The solver call.
			 * @returns `true` if an SMT solver of the requested type is available, `false` otherwise.
			 */
            static bool has_local_solver_for(SolverType type, SolverCall call);

            /**
			 * Queries an SMT solver with the specified query configuration.
			 *
			 * @param[in] config - The SMT solver query configuration.
			 * @returns OK() and the result on success, Err() otherwise.
			 */
            Result<SolverResult> query(const QueryConfig& config = QueryConfig()) const;

            /**
			 * Queries a local SMT solver with the specified query configuration.
			 *
			 * @param[in] config - The SMT solver query configuration.
			 * @returns OK() and the result on success, Err() otherwise.
			 */
            Result<SolverResult> query_local(const QueryConfig& config) const;

			/**
			 * Queries a local SMT solver with the specified query configuration and the provided smt2 representation of the query.
			 *
			 * @param[in] config - The SMT solver query configuration.
			 * @param[in] smt2   - The SMT solver query as smt2 string.
			 * @returns OK() and the result on success, Err() otherwise.
			 */
            static Result<SolverResult> query_local(const QueryConfig& config, std::string& smt2);

            /**
			 * Queries a remote SMT solver with the specified query configuration.
			 *
			 * \warning This function is not yet implemented.
			 *
			 * @param[in] config - The SMT solver query configuration.
			 * @returns Ok() and the result on success, Err() otherwise.
			 */
            Result<SolverResult> query_remote(const QueryConfig& config) const;

            /**
			 * Translate the solver into an SMT-LIB v2 string representation
			 * 
			 * @param[in] config - The SMT solver query configuration.
			 * @returns Ok() and the SMT-LIB v2 string representation, Err() otherwise.
			 */
            Result<std::string> to_smt2(const QueryConfig& config) const;

        private:
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// stores list of SMT solver constraints
            std::vector<Constraint> m_constraints;

            static std::map<std::pair<SolverType, SolverCall>, std::function<Result<std::tuple<bool, std::string>>(std::string&, const QueryConfig&)>> spec2query;
            static std::map<SolverType, std::function<Result<std::string>()>> type2query_binary;
            static std::map<SolverType, bool> type2link_status;

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
			 * @returns Ok() and SMT-LIB compatible string representation on success, Err() otherwise.
			 */
            static Result<std::string> translate_to_smt2(const std::vector<Constraint>& constraints, const QueryConfig& config);

            /**
			 * Translate a SMT solver result from SMT-LIB v2.
			 *
			 * NOTE: For details on the SMT-LIB language standard see: 
			 * 		 https://smtlib.cs.uiowa.edu/language.shtml
			 *
			 * @param[in] was_killed - Indicates whether solver process was killed.
			 * @param[in] stdout - Stdout of solver process.
			 * @param[in] config - SMT solver configuration.
			 * @returns Ok() and SMT solver result on success, Err() otherwise.
			 */
            static Result<SolverResult> translate_from_smt2(bool was_killed, std::string stdout, const QueryConfig& config);
        };
    }    // namespace SMT
}    // namespace hal