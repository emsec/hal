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

#include <map>

namespace hal {
namespace SMT {

/**
 * SymbolicState represents the data structure that keeps track of symbolic
 * variable values (e.g., required for symbolic simplification).
 */
class SymbolicState final {
 public:
    ////////////////////////////////////////////////////////////////////////////
    // Constructors, Destructors, Operators
    ////////////////////////////////////////////////////////////////////////////

    /**
     * Creates a SymbolicState instance and initializes the available variables.
     * 
     * @param[in] variables - List of variables.
     * @returns Initialized symbolic state.
     */ 
    explicit SymbolicState(const std::vector<BooleanFunction>& variables = {});

    ////////////////////////////////////////////////////////////////////////////
    // Interface
    ////////////////////////////////////////////////////////////////////////////

    /**
     * Look-up a Boolean function in the symbolic state.
     * 
     * @param[in] key - Boolean function.
     * @returns Boolean function from state or key in case key is not in state.
     */
    const BooleanFunction& get(const BooleanFunction& key) const;

    /**
     * Set a Boolean function in the symbolic state.
     * 
     * @param[in] key - Boolean function variable key.
     * @param[in] value - Boolean function variable value.
     */
    void set(BooleanFunction&& key, BooleanFunction&& value);

    ////////////////////////////////////////////////////////////////////////////
    // Members
    ////////////////////////////////////////////////////////////////////////////
 private:
    /// refers to the symbolic state map
    std::map<BooleanFunction, BooleanFunction> variable;
};

}  // namespace SMT
}  // namespace hal