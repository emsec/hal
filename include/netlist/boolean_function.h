//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include "pragma_once.h"
#ifndef __HAL_BOOLEAN_FUNCTION_H__
#define __HAL_BOOLEAN_FUNCTION_H__

#include "def.h"
#include <map>
#include <ostream>
#include <set>

/**
 * Boolean function class.
 *
 * @ingroup netlist
 */
class boolean_function
{
public:
    enum value
    {
        X    = -1,
        ZERO = 0,
        ONE  = 1
    };

    /**
     * Returns the value as a string.
     * 
     * @returns A string describing the value.
     */
    static std::string to_string(const value& v);

    // TODO description
    friend std::ostream& operator<<(std::ostream& os, const value& v);

    /**
     * Constructor for an empty function.
     * Evaluates to X (undefined).
     * Combining a function with an empty function leaves the other one unchanged.
     */
    boolean_function();

    /**
     * Constructor for a variable, usable in other functions.
     * Variable name must not be empty.
     * 
     * @param[in] variable_name - Name of the variable.
     * @param[in] invert_result - True to invert the variable. 
     */
    boolean_function(const std::string& variable_name, bool invert_result = false);

    /**
     * Constructor for a constant, usable in other functions.
     * The constant can be either X, Zero, or ONE.
     * 
     * @param[in] constant - A constant value.
     */
    boolean_function(value constant);

    /**
     * Substitutes a variable with another function (can again be a single variable).
     * Applies to all instances of the variable in the function.
     *
     * @param[in] variable_name - The variable to substitute
     * @param[in] function - The function to take the place of the varible
     * @returns The new boolean function.
     */
    boolean_function substitute(const std::string& variable_name, const boolean_function& function) const;

    /**
     * Evaluates the function on the given inputs and returns the result.
     * 
     * @param[in] inputs - A map from variable names to values.
     * @returns The value that the function evaluates to.
     */
    value evaluate(const std::map<std::string, value>& inputs = {}) const;

    /**
     * Evaluates the function on the given inputs and returns the result.
     * 
     * @param[in] inputs - A map from variable names to values.
     * @returns The value that the function evaluates to.
     */
    value operator()(const std::map<std::string, value>& inputs = {}) const;

    /**
     * Checks whether the function is the constant ONE.
     * 
     * @returns True if function is constant ONE, false otherwise.
     */
    bool is_constant_one() const;

    /**
     * Checks whether the function is the constant ZERO.
     * 
     * @returns True if function is constant ZERO, false otherwise.
     */
    bool is_constant_zero() const;

    /**
     * Checks whether the function is empty.
     * 
     * @returns True if function is empty, false otherwise.
     */
    bool is_empty() const;

    /**
     * Get all variable names used in this boolean function.
     * 
     * @returns A set of all variable names.
     */
    std::set<std::string> get_variables() const;

    /**
     * Parse a function from a string representation.
     * Supported operators are  NOT ("!", "'"), AND ("&", "*", " "), OR ("|", "+"), XOR ("^") and brackets ("(", ")").
     * 
     * @param[in] expression - String containing a boolean function.
     * @returns The boolean function extracted from the string.
     */
    static boolean_function from_string(std::string expression);

    /**
     * Returns the boolean function as a string.
     * 
     * @returns A string describing the boolean function.
     */
    std::string to_string() const;

    // TODO description
    friend std::ostream& operator<<(std::ostream& os, const boolean_function& f);

    /**
     * Combines two boolean functions using an AND operator.
     * 
     * @returns The combined boolean function.
     */
    boolean_function operator&(const boolean_function& other) const;

    /**
     * Combines two boolean functions using an OR operator.
     * 
     * @returns The combined boolean function.
     */
    boolean_function operator|(const boolean_function& other) const;

    /**
     * Combines two boolean functions using an XOR operator.
     * 
     * @returns The combined boolean function.
     */
    boolean_function operator^(const boolean_function& other) const;

    /**
     * Combines two boolean functions using an AND operator.
     * 
     * @returns The combined boolean function.
     */
    boolean_function& operator&=(const boolean_function& other);

    /**
     * Combines two boolean functions using an OR operator.
     * 
     * @returns The combined boolean function.
     */
    boolean_function& operator|=(const boolean_function& other);

    /**
     * Combines two boolean functions using an XOR operator.
     * 
     * @returns The combined boolean function.
     */
    boolean_function& operator^=(const boolean_function& other);

    /**
     * Negates the boolean function.
     * 
     * @returns The negated boolean function.
     */
    boolean_function operator!() const;

    /**
     * Tests whether two boolean functions are equal.
     * 
     * @param[in] other - Boolean function to compare to.
     * @returns True when both boolean functions are equal, false otherwise.
     */
    bool operator==(const boolean_function& other) const;

    /**
     * Tests whether two boolean functions are unequal.
     * 
     * @param[in] other - Boolean function to compare to.
     * @returns True when both boolean functions are unequal, false otherwise.
     */
    bool operator!=(const boolean_function& other) const;

    // TODO return description
    /**
     * Tests whether two boolean functions are equal.
     * 
     * @param[in] other - Boolean function to compare to.
     * @returns 
     */
    bool operator<(const boolean_function& other) const;

    /**
     * Tests whether two boolean functions are equal.
     * 
     * @param[in] other - Boolean function to compare to.
     * @returns 
     */
    bool operator>(const boolean_function& other) const;

    /**
     * Tests whether the function is in DNF.
     * 
     * @returns True if in DNF, false otherwise.
     */
    bool is_dnf() const;

    /**
     * Gets the DNF representation of the function.
     * 
     * @returns The DNF as a boolean function.
     */
    boolean_function to_dnf() const;

    /**
     * Optimizes the function by first converting it to DNF and then applying the Quine-McCluskey algorithm.
     * 
     * @returns The optimized boolean function.
     */
    boolean_function optimize() const;

    /**
     * Get the truth table outputs of the function.
     * WARNING: Exponential runtime in the number of variables!
     *
     * Output is the vector of output values when walking the truth table in ascending order.
     * The variable values are changed in order of appearance, i.e.:
     * first_var second_var | output_vector_index
     *     0         0      |   0
     *     1         0      |   1
     *     0         1      |   2
     *     1         1      |   3
     *
     * If ordered_variables is empty, all included variables are used and ordered alphabetically.
     * 
     * @param[in] ordered_variables - Specific order in which the inputs shall be structured in the truth table.
     * @returns The vector of output values.
     */
    std::vector<value> get_truth_table(const std::vector<std::string>& ordered_variables = {}) const;

private:
    enum class operation
    {
        AND,
        OR,
        XOR
    };
    static std::string to_string(const operation& op);
    friend std::ostream& operator<<(std::ostream& os, const operation& op);

    /*
     * Constructor for a function of the form "term1 op term2 op term3 op ..."
     * Empty terms behaves like constant X.
     * If there is only a single term, this constructor simply copies said term.
     */
    boolean_function(operation op, const std::vector<boolean_function>& operands, bool invert_result = false);

    boolean_function combine(operation op, const boolean_function& other) const;

    std::string to_string_internal() const;

    // replaces a^b with (a & !b | (!a & b)
    boolean_function replace_xors() const;

    // propagates negations down to the variables
    boolean_function propagate_negations(bool negate_term = false) const;

    // expands ands, i.e., a & (b | c) -> a&b | a&c
    boolean_function expand_ands() const;
    // helper function 1
    std::vector<boolean_function> expand_ands_internal(const std::vector<std::vector<boolean_function>>& sub_primitives, u32 i) const;
    // helper function 2
    std::vector<boolean_function> get_primitives() const;

    // merges constants if possible and resolves duplicates
    boolean_function optimize_constants() const;

    // merges nested expressions of the same operands
    boolean_function flatten() const;

    // merges nested expressions of the same operands
    static std::vector<std::vector<value>> qmc(const std::vector<std::vector<value>>& terms);

    bool m_invert;

    enum class content_type
    {
        VARIABLE,
        CONSTANT,
        TERMS
    } m_content;

    std::string m_variable;

    value m_constant;

    operation m_op;
    std::vector<boolean_function> m_operands;
};

#endif
