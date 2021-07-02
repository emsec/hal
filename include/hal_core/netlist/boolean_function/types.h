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

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/enums.h"

#include <map>
#include <optional>
#include <string>

namespace hal
{
    namespace SMT
    {
        /// 'SMTSolverType' represents the identifier for an SMT solver.
        enum class SolverType
        {
            Z3,
            Boolector,

            Unknown,
        };

        /**
		 * 'QueryConfig' represents the data structure to configure an SMT query.
		 */
        struct QueryConfig final
        {
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// refers to the SMT solver identifier.
            SolverType solver = SolverType::Z3;
            /// refers to whether SMT query is performed on a local/remote machine.
            bool local = true;
            /// refers to whether the SMT solver should generate a model (in case formula is satisfiable)
            bool generate_model = true;
            /// refers to the timeout after which the SMT solver is killed (seconds)
            u64 timeout_in_seconds = 10;

            ////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////

            /**
             * Appends a solver to the configuration.
             *
             * @param[in] _solver - Solver identifier.
             * @returns Self-reference.
             */
            QueryConfig& with_solver(SolverType _solver);
            
            /**
             * Indicates that the SMT solver runs on a local machine.
             *
             * @returns Self-reference.
             */
            QueryConfig& with_local_solver();
            
            /**
             * Indicates that the SMT solver runs on a remote machine.
             *
             * @returns Self-reference.
             */
            QueryConfig& with_remote_solver();
            
            /**
             * Indicates that the SMT solver should generate a model (in case formula is satisfiable).
             *
             * @returns Self-reference.
             */
            QueryConfig& with_model_generation();
            
            /**
             * Indicates that the SMT solver should not generate a model.
             *
             * @returns Self-reference.
             */
            QueryConfig& without_model_generation();
            
            /**
             * Indicates the timeout in seconds after which SMT query is killed.
             *
             * @param[in] seconds - Timeout in seconds.
             * @returns Self-reference.
             */
            QueryConfig& with_timeout(u64 seconds);

            /**
             * Human-readable description of SMT configuration.
             *
             * @param[in] out - Stream to write to.
             * @param[in] config - SMT query configuration.
             * @returns A reference to output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const QueryConfig& config);
        };

        /**
		 * 'Constraint' represents a constraint a.k.a. assertion to the SMT query
		 * i.e. an assignment of two Boolean function that is true.
		 */
        struct Constraint final
        {
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// Left-hand side of equality constraint.
            BooleanFunction lhs;
            /// Right-hand side of equality constraint.
            BooleanFunction rhs;

            ////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////

            /// Creates a new 'Constraint'
            Constraint(BooleanFunction _lhs, BooleanFunction _rhs);

            /**
             * Human-readable description of an SMT constraint.
             *
             * @param[in] out - Stream to write to.
             * @param[in] constraint - SMT constraint.
             * @returns A reference to output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const Constraint& constraint);
        };

        /// 'ResultType' refers to the result of an SMT solver query.
        enum class ResultType
        {
            Sat,        // indicates that a list of constraints is satisfiable
            UnSat,      // indicates that a list of constraints is unsatisfiable
            Unknown,    // indicates that a result cannot be obtained, i.e. timeout.
        };

        /**
		 * 'Model' represents a list of assignment for variable nodes that yield
		 * a satisfiable assignment for a given list of constraints.
		 */
        struct Model final
        {
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// maps variable identifiers to a (1) value and (2) its bit-size.
            std::map<std::string, std::tuple<u64, u16>> model;

            ////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////

            /// Constructor to initialize an 'Model'.
            Model(std::map<std::string, std::tuple<u64, u16>> _model = {});

            /**
             * Check whether two models are equal.
             *
             * @param[in] other - The model to compare again.
             * @returns True if both models are equal, false otherwise.
             */
            bool operator==(const Model& other) const;

            /**
             * Check whether two models are unequal.
             *
             * @param[in] other - The model to compare again.
             * @returns True if both models are unequal, false otherwise.
             */
            bool operator!=(const Model& other) const;

            /**
             * Human-readable description of an SMT model.
             *
             * @param[in] out - Stream to write to.
             * @param[in] model - SMT model.
             * @returns A reference to output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const Model& model);

            ////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////

            /**
			 * Parses an SMT-LIB model for a given solver.
			 *
			 * @param[in] s - SMT-LIB model string.
			 * @param[in] solver - Solver that computed the model.
			 * @returns (1) status (true on success, false otherwise), and 
			 *          (2) SMT model.
			 */
            static std::tuple<bool, Model> parse(std::string s, const SolverType& solver);
        };

        /**
		 * 'Result' represents the result of an SMT query.
		 */
        struct Result final
        {
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// stores result of SMT query
            ResultType type;
            /// stores optional model (only available if type == Sat) and model generation is enabled in SMTQueryConfig
            std::optional<Model> model;

            ////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////

            
            /**
             * Creates a satisfiable result with an optional model.
             *
             * @param[in] model - Optional model for satisfiable formula.
             * @returns Result.
             */
            static Result Sat(const std::optional<Model>& model = {});
            
            /**
             * Creates an unsatisfiable result.
             *
             * @returns Result.
             */
            static Result UnSat();
            
            /**
             * Creates an unknown result.
             *
             * @returns Result.
             */
            static Result Unknown();

            /**
             * Checks whether result is of specific type.
             *
             * @param[in] _type - Type to check.
             * @returns True in case result matches _type, false otherwise.
             */
            bool is(const ResultType& _type) const;

            /**
             * Checks whether result is of satisfiable type.
             *
             * @returns True in case result is satisfiable, false otherwise.
             */
            bool is_sat() const;

            /**
             * Checks whether result is of unsatisfiable type.
             *
             * @returns True in case result is unsatisfiable, false otherwise.
             */
            bool is_unsat() const;

            /**
             * Checks whether result is of unknown type.
             *
             * @returns True in case result is unknown, false otherwise.
             */
            bool is_unknown() const;

            /**
             * Human-readable description of SMT result.
             *
             * @param[in] out - Stream to write to.
             * @param[in] result - SMT result.
             * @returns A reference to output stream.
             */ 
            friend std::ostream& operator<<(std::ostream& out, const Result& result);

        private:
            /// Constructor to initialize a 'Result.
            Result(ResultType _type, std::optional<Model> _model);
        };

    }    // namespace SMT

    template<>
    std::vector<std::string> EnumStrings<SMT::SolverType>::data;

    template<>
    std::vector<std::string> EnumStrings<SMT::ResultType>::data;
}    // namespace hal