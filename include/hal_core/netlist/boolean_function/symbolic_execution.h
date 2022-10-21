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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/symbolic_state.h"
#include "hal_core/netlist/boolean_function/types.h"

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

            /// The current symbolic state.
            SymbolicState state;

            ////////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////////

            /**
             * Creates a symbolic execution engine and initializes the variables.
             * 
             * @param[in] variables - The (optional) list of variables.
             */
            explicit SymbolicExecution(const std::vector<BooleanFunction>& variables = {});

            ////////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////////

            /**
             * Evaluates a Boolean function within the symbolic state of the symbolic execution.
             * 
             * @param[in] function - The Boolean function to evaluate.
             * @returns Ok() and the evaluated Boolean function on success, Err() otherwise.
             */
            Result<BooleanFunction> evaluate(const BooleanFunction& function) const;

            /**
             * Evaluates an equality constraint and applies it to the symbolic state of the symbolic execution.
             * 
             * @param[in] constraint - The equality constraint to evaluate.
             * @returns Ok() on success, Err() otherwise.
             */
            Result<std::monostate> evaluate(const Constraint& constraint);

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
            static std::vector<BooleanFunction> normalize(std::vector<BooleanFunction>&& p);

            /**
             * Simplifies a sub-expression in the Boolean function abstract syntax tree.
             * 
             * @param[in] node - Boolean function node.
             * @param[in] p - List of node parameters.
             * @returns Ok() and Boolean function on success, Err() otherwise.
             */
            Result<BooleanFunction> simplify(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p) const;

            /**
             * Propagates constants in a sub-expression in the Boolean function abstract syntax tree.
             * 
             * @param[in] node - Boolean function node.
             * @param[in] p - List of node parameters.
             * @returns Ok() and Boolean function on success, Err() otherwise.
             */
            static Result<BooleanFunction> constant_propagation(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p);
        };

    }    // namespace SMT
}    // namespace hal
