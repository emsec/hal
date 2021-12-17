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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/symbolic_state.h"
#include "hal_core/netlist/boolean_function/types.h"

#include <optional>

namespace hal {
namespace SMT {

/**
 * SymbolicExecution represents the symbolic execution engine that handles the 
 * evaluation and simplification of Boolean function abstract syntax trees.
 */
class SymbolicExecution final {
 public:
    ////////////////////////////////////////////////////////////////////////////
    // Members
    ////////////////////////////////////////////////////////////////////////////
 
    /// refers to the symbolic state map
    SymbolicState state;

 	////////////////////////////////////////////////////////////////////////////
    // Constructors, Destructors, Operators
    ////////////////////////////////////////////////////////////////////////////

    /**
     * Creates a SymbolicExecution instance and initializes available variables.
     * 
     * @param[in] variables - List of variables.
     * @returns Initialized symbolic execution engine.
     */ 
    explicit SymbolicExecution(const std::vector<BooleanFunction>& variables = {});

    ////////////////////////////////////////////////////////////////////////////
    // Interface
    ////////////////////////////////////////////////////////////////////////////

    /**
     * Evaluates and simplifies a Boolean function.
     * 
     * @param[in] function - Boolean function to evaluate.
     * @returns Boolean function on success, error message string otherwise.
     */
    std::variant<BooleanFunction, std::string> evaluate(const BooleanFunction& function) const;

    /**
     * Evaluates and simplifies a Boolean function constraint.
     * 
     * @param[in] constraint - Boolean function constraint to evaluate.
     * @returns Nullopt on success, error message string otherwise.
     */
    std::optional<std::string> evaluate(const Constraint& constraint);

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
	 * @returns Boolean function on success, error message string otherwise.
	 */
	std::variant<BooleanFunction, std::string> simplify(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p) const;

	/**
	 * Propagates constants in a sub-expression in the Boolean function abstract syntax tree.
	 * 
	 * @param[in] node - Boolean function node.
	 * @param[in] p - List of node parameters.
	 * @returns Boolean function on success, error message string otherwise.
	 */
	static std::variant<BooleanFunction, std::string> constant_propagation(const BooleanFunction::Node& node, std::vector<BooleanFunction>&& p);
};

}  // namespace SMT
}  // namespace hal
