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
#include "hal_core/utilities/enums.h"
#include "z3++.h"

#include <algorithm>
#include <cassert>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hal
{
    /**
     * Boolean function class.
     *
     * @ingroup netlist
     */
    class BooleanFunction
    {
    public:
        /**
         * Represents the logic value that a boolean function operates on.
         */
        enum Value
        {
            ZERO = 0, /**< Represents a logical 0. */
            ONE  = 1, /**< Represents a logical 1 */
            Z,        /**< Represents an undefined value. */
            X         /**< Represents an undefined value. */
        };

        /**
         * Get the value as a string.
         *
         * @param[in] v - the value.
         * @returns A string describing the value.
         */
        static std::string to_string(Value v);

        /**
         * ostream operator that forwards to_string of a value.
         *
         * @param[in] os - the stream to write to.
         * @param[in] v - the value.
         * @returns A reference to os.
         */
        friend std::ostream& operator<<(std::ostream& os, Value v);

        /**
         * Construct an empty Boolean function and thus evaluates to X (undefined).
         */
        BooleanFunction();

        /**
         * Construct a Boolean function comprising a single variable.
         * The name of the variable must not be empty.
         *
         * @param[in] variable_name - The name of the variable.
         */
        BooleanFunction(const std::string& variable_name);

        /**
         * Construct a Boolean function from a single constant value.
         *
         * @param[in] constant - The constant value.
         */
        BooleanFunction(Value constant);

        /**
         * Substitute a variable with another one and thus renames the variable.
         * The operation is applied to all instances of the variable in the function.
         *
         * @param[in] old_variable_name - The old variable to substitute.
         * @param[in] new_variable_name - The new variable.
         * @returns The resulting Boolean function.
         */
        BooleanFunction substitute(const std::string& old_variable_name, const std::string& new_variable_name) const;

        /**
         * Substitute a variable with another function.
         * The operation is applied to all instances of the variable in the function.
         *
         * @param[in] variable_name - The variable to substitute.
         * @param[in] function - The function replace the variable with.
         * @returns The resulting Boolean function.
         */
        BooleanFunction substitute(const std::string& variable_name, const BooleanFunction& function) const;

        /**
         * Evaluate the Boolean function on the given inputs and returns the result.
         *
         * @param[in] inputs - A map from variable names to values.
         * @returns The value that the function evaluates to.
         */
        Value evaluate(const std::unordered_map<std::string, Value>& inputs = {}) const;

        /**
         * Evaluate the function on the given inputs and returns the result.
         *
         * @param[in] inputs - A map from variable names to values.
         * @returns The value that the function evaluates to.
         */
        Value operator()(const std::unordered_map<std::string, Value>& inputs = {}) const;

        /**
         * Check whether the Boolean function always evaluates to ONE.
         *
         * @returns True if function is constant ONE, false otherwise.
         */
        bool is_constant_one() const;

        /**
         * Check whether the Boolean function always evaluates to ZERO.
         *
         * @returns True if function is constant ZERO, false otherwise.
         */
        bool is_constant_zero() const;

        /**
         * Check whether the function is empty.
         *
         * @returns True if function is empty, false otherwise.
         */
        bool is_empty() const;

        /**
         * Get all variable names utilized in this Boolean function.
         *
         * @returns A vector of all variable names.
         */
        std::vector<std::string> get_variables() const;

        /**
         * Parse a function from a string representation.
         * Supported operators are  NOT ("!", "'"), AND ("&", "*", " "), OR ("|", "+"), XOR ("^") and brackets ("(", ")").
         * Operator precedence is ! > & > ^ > |.
         *
         * Since, for example, '(' is interpreted as a new term, but might also be an intended part of a variable, a vector of known variable names can be supplied, which are extracted before parsing.
         *
         * If there is an error during bracket matching, X is returned for that part.
         *
         * @param[in] expression - String containing a Boolean function.
         * @param[in] variable_names - List of variable names.
         * @returns The Boolean function extracted from the string.
         */
        static BooleanFunction from_string(std::string expression, const std::vector<std::string>& variable_names = {});

        /**
         * Get the boolean function as a string.
         *
         * @returns A string describing the boolean function.
         */
        std::string to_string() const;

        /**
         * The ostream operator that forwards to_string of a boolean function.
         *
         * @param[in] os - the stream to write to.
         * @param[in] f - the function.
         * @returns A reference to os.
         */
        friend std::ostream& operator<<(std::ostream& os, const BooleanFunction& f);

        /**
         * Combine two Boolean functions using an AND operator.
         *
         * @param[in] other - The other Boolean function to combine with.
         * @returns The combined Boolean function.
         */
        BooleanFunction operator&(const BooleanFunction& other) const;

        /**
         * Combine two Boolean functions using an OR operator.
         *
         * @param[in] other - The other Boolean function to combine with.
         * @returns The combined Boolean function.
         */
        BooleanFunction operator|(const BooleanFunction& other) const;

        /**
         * Combine two Boolean functions using an XOR operator.
         *
         * @param[in] other - The other Boolean function to combine with.
         * @returns The combined Boolean function.
         */
        BooleanFunction operator^(const BooleanFunction& other) const;

        /**
         * Combine two Boolean functions using an AND operator in-place.
         *
         * @param[in] other - The other Boolean function to combine with.
         * @returns The combined Boolean function.
         */
        BooleanFunction& operator&=(const BooleanFunction& other);

        /**
         * Combine two boolean functions using an OR operator in-place.
         *
         * @param[in] other - The other Boolean function to combine with.
         * @returns The combined Boolean function.
         */
        BooleanFunction& operator|=(const BooleanFunction& other);

        /**
         * Combine two Boolean functions using an XOR operator in-place.
         *
         * @param[in] other - The other Boolean function to combine with.
         * @returns The combined Boolean function.
         */
        BooleanFunction& operator^=(const BooleanFunction& other);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Negate the Boolean function.
         *
         * @returns The negated Boolean function.
         */
        [[deprecated("Use operator~() instead.")]] BooleanFunction operator!() const;

        /**
         * Negate the Boolean function.
         *
         * @returns The negated Boolean function.
         */
        BooleanFunction operator~() const;

        /**
         * Check whether two Boolean functions are equal.
         *
         * @param[in] other - The Boolean function to compare against.
         * @returns True if both Boolean functions are equal, false otherwise.
         */
        bool operator==(const BooleanFunction& other) const;

        /**
         * Check whether two Boolean functions are unequal.
         *
         * @param[in] other - The Boolean function to compare to.
         * @returns True if both Boolean functions are unequal, false otherwise.
         */
        bool operator!=(const BooleanFunction& other) const;

        /**
         * Check whether the Boolean function is in disjunctive normal form (DNF).
         *
         * @returns True if in DNF, false otherwise.
         */
        bool is_dnf() const;

        /**
         * Get the plain disjunctive normal form (DNF) representation of the Boolean function.
         *
         * @returns The DNF as a Boolean function.
         */
        BooleanFunction to_dnf() const;

        /**
         * Get the disjunctive normal form (DNF) clauses of the function.
         *
         * Each clause is a vector of pairs (variable name, Boolean value).
         *
         * Returns an empty vector if the Boolean function is empty.
         *
         * @returns The DNF clauses as a vector of vectors of pairs (string, bool).
         */
        std::vector<std::vector<std::pair<std::string, bool>>> get_dnf_clauses() const;

        /**
         * Optimizes the Boolean function by first converting it to disjunctive normal form (DNF) and then applying the Quine-McCluskey algorithm.
         *
         * @returns The optimized Boolean function.
         */
        BooleanFunction optimize() const;

        /**
         * Removes constant values whenever possible.
         * 
         * @return The optimized Boolean function.
         */
        BooleanFunction optimize_constants() const;

        /**
         * Get the truth table outputs of the function.
         *
         * WARNING: Exponential runtime in the number of variables!
         *
         * Output is the vector of output values when walking the truth table from the least significant bit to the most significant one.
         *
         * If ordered_variables is empty, all included variables are used and ordered alphabetically.
         *
         * @param[in] ordered_variables - Variables in the order of the inputs.
         * @param[in] remove_unknown_variables - If true, all given variables that are not found in the function are removed from the truth table.
         * @returns The vector of output values.
         */
        std::vector<Value> get_truth_table(std::vector<std::string> ordered_variables = {}, bool remove_unknown_variables = false) const;

        // TODO figure out how to test this
        /**
         * Get the z3 representation of the Boolean function.
         *
         * @param[in,out] context - The z3 context.
         * @returns The z3 representation of the Boolean function.
         */
        z3::expr to_z3(z3::context& context) const;

    protected:
        enum class operation
        {
            AND,
            OR,
            XOR
        };
        static std::string to_string(const operation& op);
        friend std::ostream& operator<<(std::ostream& os, const operation& op);

        static BooleanFunction from_string_internal(std::string expression, const std::vector<std::string>& variable_names);

        /*
        * Constructor for a function of the form "term1 op term2 op term3 op ..."
        * Empty terms behaves like constant X.
        * If there is only a single term, this constructor simply copies said term.
        */
        BooleanFunction(operation op, const std::vector<BooleanFunction>& operands, bool invert_result = false);

        BooleanFunction combine(operation op, const BooleanFunction& other) const;

        std::string to_string_internal() const;

        // replaces a^b with (a & !b | (!a & b)
        BooleanFunction replace_xors() const;

        // propagates negations down to the variables
        BooleanFunction propagate_negations(bool negate_term = false) const;

        // expands ands, i.e., a & (b | c) -> a&b | a&c
        BooleanFunction expand_ands() const;
        // helper function 1
        std::vector<BooleanFunction> expand_AND_of_functions(const std::vector<std::vector<BooleanFunction>>& AND_terms_to_expand) const;
        // helper function 2
        std::vector<BooleanFunction> get_AND_terms() const;

        // merges nested expressions of the same operands
        static std::vector<std::vector<Value>> qmc(std::vector<std::vector<Value>> terms);

        // helper to allow for substitution with reduced amount of copies
        static void substitute_helper(BooleanFunction& f, const std::string& v, const BooleanFunction& s);

        z3::expr to_z3_internal(z3::context& context, const std::unordered_map<std::string, z3::expr>& input2expr) const;

        bool m_invert;

        enum class content_type
        {
            VARIABLE,
            CONSTANT,
            TERMS
        } m_content;

        std::string m_variable;

        Value m_constant;

        operation m_op;
        std::vector<BooleanFunction> m_operands;
    };

    template<>
    std::vector<std::string> EnumStrings<BooleanFunction::Value>::data;
}    // namespace hal
