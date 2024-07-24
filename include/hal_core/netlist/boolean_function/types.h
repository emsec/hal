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
        /**
         * Identifier for the SMT solver type.
         */
        enum class SolverType : int
        {
            Z3 = 0,    /**< Z3 SMT solver. */
            Boolector, /**< Boolector SMT solver. */
            Bitwuzla,  /**< Bitwuzla SMT solver. */
            Unknown,   /**< Unknown (unsupported) SMT solver. */
        };

        enum class SolverCall : int
        {
            Binary,  /**< Call binary in subprocess*/
            Library, /**< Call linked library */
        };

        /**
		 * Represents the data structure to configure an SMT query.
		 */
        struct QueryConfig final
        {
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// The SMT solver identifier.
            SolverType solver = SolverType::Z3;
            /// The calling format for the SMT solver
            SolverCall call = SolverCall::Binary;
            /// Controls whether the SMT query is performed on a local or a remote machine.
            bool local = true;
            /// Controls whether the SMT solver should generate a model in case formula is satisfiable.
            bool generate_model = false;
            /// The timeout after which the SMT solver is killed in seconds.
            u64 timeout_in_seconds = 10;

            ////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////

            /**
             * Sets the solver type to the desired SMT solver.
             *
             * @param[in] solver - The solver type identifier.
             * @returns The updated SMT query configuration.
             */
            QueryConfig& with_solver(SolverType solver);

            /**
             * Sets the solver type to the desired SMT solver.
             *
             * @param[in] call - The solver call.
             * @returns The updated SMT query configuration.
             */
            QueryConfig& with_call(SolverCall call);

            /**
             * Activates local SMT solver execution.
             *
             * @returns The updated SMT query configuration.
             */
            QueryConfig& with_local_solver();

            /**
             * Indicates that the SMT solver runs on a remote machine.
             *
             * @returns The updated SMT query configuration.
             */
            QueryConfig& with_remote_solver();

            /**
             * Indicates that the SMT solver should generate a model in case the formula is satisfiable.
             *
             * @returns The updated SMT query configuration.
             */
            QueryConfig& with_model_generation();

            /**
             * Indicates that the SMT solver should not generate a model.
             *
             * @returns The updated SMT query configuration.
             */
            QueryConfig& without_model_generation();

            /**
             * Sets a timeout in seconds that terminates an SMT query after the specified time has passed.
             *
             * @param[in] seconds - The timeout in seconds.
             * @returns The updated SMT query configuration.
             */
            QueryConfig& with_timeout(u64 seconds);

            /**
             * Passes a human-readable description of the SMT query configuration to the output stream.
             *
             * @param[in] out - The output stream to write to.
             * @param[in] config - The SMT query configuration.
             * @returns A reference to the output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const QueryConfig& config);
        };

        /**
		 * Represents a constraint to the SMT query.
         * A constraint is either an assignment of two Boolean functions or a single Boolean function, e.g., an equality check or similar.
		 */
        struct Constraint final
        {
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// A constraint that is either an assignment of two Boolean functions or a single Boolean function, e.g., an equality check or similar.
            std::variant<BooleanFunction, std::pair<BooleanFunction, BooleanFunction>> constraint;

            ////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////

            /**
             * Constructs a new constraint from one Boolean function that evaluates to a single bit.
             * 
             * @param[in] constraint - The constraint function.
             */
            explicit Constraint(BooleanFunction&& constraint);

            /**
             * Constructs a new equality constraint from two Boolean functions.
             * 
             * @param[in] lhs - The left-hand side of the equality constraint
             * @param[in] rhs - The right-hand side of the equality constraint
             */
            explicit Constraint(BooleanFunction&& lhs, BooleanFunction&& rhs);

            /**
             * Passes a human-readable description of the SMT constraint to the output stream.
             *
             * @param[in] out - The output stream to write to.
             * @param[in] constraint - The SMT constraint.
             * @returns A reference to the output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const Constraint& constraint);

            /**
             * Translate the SMT constraint into its string representation.
             * 
             * @returns A string representing the SMT constraint.
             */
            std::string to_string() const;

            /**
             * Checks whether the constraint is an assignment constraint.
             * 
             * @returns `true` if the constraint is an assignment, `false` otherwise.
             */
            bool is_assignment() const;

            /**
             * Returns the assignment constraint as a pair of Boolean functions.
             * 
             * @returns The assignment constraint on success, an error otherwise.
             */
            Result<const std::pair<BooleanFunction, BooleanFunction>*> get_assignment() const;

            /**
             * Returns the function constraint.
             * 
             * @returns The function constraint on success, an error otherwise.
             */
            Result<const BooleanFunction*> get_function() const;
        };

        /**
         * Result type of an SMT solver query.
         */
        enum class SolverResultType
        {
            Sat,     /**< The list of constraints is satisfiable. */
            UnSat,   /**< The list of constraints is not satisfiable. */
            Unknown, /**< A result could not be obtained, e.g., due to a time-out. */
        };

        /**
		 * Represents a list of assignments for variable nodes that yield a satisfiable assignment for a given list of constraints.
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

            /**
             * Constructs a new model from a map of variable names to value and bit-size.
             * 
             * @param[in] model - A map from variable name to value and bit-size.
             */
            Model(const std::map<std::string, std::tuple<u64, u16>>& model = {});

            /**
             * Checks whether two SMT models are equal.
             *
             * @param[in] other - The model to compare again.
             * @returns `true` if both models are equal, `false` otherwise.
             */
            bool operator==(const Model& other) const;

            /**
             * Checks whether two SMT models are unequal.
             *
             * @param[in] other - The model to compare again.
             * @returns True if both models are unequal, false otherwise.
             */
            bool operator!=(const Model& other) const;

            /**
             * Passes a human-readable description of the SMT model to the output stream.
             *
             * @param[in] out - The output stream to write to.
             * @param[in] model - The SMT model.
             * @returns A reference to the output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const Model& model);

            ////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////

            /**
			 * Parses an SMT-Lib model from a string output by a solver of the given type.
			 *
			 * @param[in] model_str - The SMT-Lib model string.
			 * @param[in] solver - The solver that computed the model.
			 * @returns The model on success, an error otherwise.
			 */
            static Result<Model> parse(const std::string& model_str, const SolverType& solver);

            /**
			 * Evaluates the given Boolean function by replacing all variables contained in the model with their corresponding value and simplifying the result.
			 *
			 * @param[in] bf - The Boolean function to evaluate.
			 * @returns The evaluated function on success, an error otherwise.
			 */
            Result<BooleanFunction> evaluate(const BooleanFunction& bf) const;
        };

        /**
		 * Represents the result of an SMT query.
		 */
        struct SolverResult final
        {
            ////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////

            /// Result type of the SMT query.
            SolverResultType type;
            /// The (optional) model that is only available if `type == SMT::ResultType::Sat` and model generation is enabled.
            std::optional<Model> model;

            ////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////

            /// Default constructor (required for Result<T> initialization)
            SolverResult() : type(SolverResultType::Unknown), model({})
            {
            }

            /**
             * Creates a satisfiable result with an optional model.
             *
             * @param[in] model - Optional model for satisfiable formula.
             * @returns The result.
             */
            static SolverResult Sat(const std::optional<Model>& model = {});

            /**
             * Creates an unsatisfiable result.
             *
             * @returns The result.
             */
            static SolverResult UnSat();

            /**
             * Creates an unknown result.
             *
             * @returns The result.
             */
            static SolverResult Unknown();

            /**
             * Checks whether the result is of a specific type.
             *
             * @param[in] type - The type to check.
             * @returns `true` in case result matches the given type, `false` otherwise.
             */
            bool is(const SolverResultType& type) const;

            /**
             * Checks whether the result is satisfiable.
             *
             * @returns `true` in case result is satisfiable, `false` otherwise.
             */
            bool is_sat() const;

            /**
             * Checks whether the result is unsatisfiable.
             *
             * @returns `true` in case result is unsatisfiable, `false` otherwise.
             */
            bool is_unsat() const;

            /**
             * Checks whether the result is unknown.
             *
             * @returns `true` in case result is unknown, `false` otherwise.
             */
            bool is_unknown() const;

            /**
             * Human-readable description of SMT result.
             *
             * @param[in] out - Stream to write to.
             * @param[in] result - SMT result.
             * @returns A reference to output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const SolverResult& result);

        private:
            /// Constructor to initialize a 'Result.
            SolverResult(SolverResultType _type, std::optional<Model> _model);
        };

    }    // namespace SMT

    template<>
    std::map<SMT::SolverType, std::string> EnumStrings<SMT::SolverType>::data;

    template<>
    std::map<SMT::SolverResultType, std::string> EnumStrings<SMT::SolverResultType>::data;
}    // namespace hal