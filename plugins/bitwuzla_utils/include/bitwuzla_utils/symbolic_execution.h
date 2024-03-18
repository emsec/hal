#pragma once

#include "bitwuzla_utils/bitwuzla_utils.h"

#include <optional>

namespace hal
{
    namespace SMT
    {

        /**
         * Represents the symbolic execution engine that handles the evaluation and simplification of Boolean function abstract syntax trees.
         */
        class SymbolicExecution final
        {
        public:
            ////////////////////////////////////////////////////////////////////////////
            // Members
            ////////////////////////////////////////////////////////////////////////////

            // /// The current symbolic state.
            // SymbolicState state;

            ////////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////////

            /**
             * Creates a symbolic execution engine and initializes the variables.
             * 
             * @param[in] variables - The (optional) list of variables.
             */
            explicit SymbolicExecution(const std::vector<bitwuzla::Term>& variables = {});

            ////////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////////

            /**
             * Evaluates a Boolean function within the symbolic state of the symbolic execution.
             * 
             * @param[in] function - The Boolean function to evaluate.
             * @returns Ok() and the evaluated Boolean function on success, Err() otherwise.
             */
            Result<bitwuzla::Term> evaluate(const bitwuzla::Term& function, std::map<u64, bitwuzla::Term> id_to_term) const;

            // /**
            //  * Evaluates an equality constraint and applies it to the symbolic state of the symbolic execution.
            //  *
            //  * @param[in] constraint - The equality constraint to evaluate.
            //  * @returns Ok() on success, Err() otherwise.
            //  */
            // Result<std::monostate> evaluate(const Constraint& constraint);

        private:
            ////////////////////////////////////////////////////////////////////////////
            // Internal Interface
            ////////////////////////////////////////////////////////////////////////////

            /**
             * Normalizes a list of (parameter) assignments, i.e. registers before 
             * constants in case an operation is commutative.
             * 
             * @param[in] p - List of Boolean functions.
             * @returns List of normalized Boolean functions.
             */
            static std::vector<bitwuzla::Term> normalize(std::vector<bitwuzla::Term>&& p);

            /**
             * Simplifies a sub-expression in the Boolean function abstract syntax tree.
             * 
             * @param[in] kind - Boolean function node.
             * @param[in] p - List of node parameters.
             * @returns Ok() and Boolean function on success, Err() otherwise.
             */
            static Result<bitwuzla::Term> simplify(const bitwuzla::Term& node, std::vector<bitwuzla::Term>& p);

            /**
             * Propagates constants in a sub-expression in the Boolean function abstract syntax tree.
             * 
             * @param[in] kind - Boolean function node.
             * @param[in] p - List of node parameters.
             * @returns Ok() and Boolean function on success, Err() otherwise.
             */
            static Result<bitwuzla::Term> constant_propagation(const bitwuzla::Term& node, std::vector<bitwuzla::Term>& values);
        };

    }    // namespace SMT
}    // namespace hal
