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
 *  boolean function class
 *
 * @ingroup netlist
 */
class boolean_function
{
public:
    enum class operation
    {
        AND,
        OR,
        XOR
    };
    static std::string to_string(const operation& op);
    friend std::ostream& operator<<(std::ostream& os, const operation& op);

    enum value
    {
        X    = -1,
        ZERO = 0,
        ONE  = 1
    };
    static std::string to_string(const value& v);
    friend std::ostream& operator<<(std::ostream& os, const value& v);

    /*
     * Constructor for a function of the form "term1 op term2 op term3 op ..."
     * Empty terms behaves like constant X.
     * If there is only a single term, this constructor simply copies said term.
     */
    boolean_function(operation op, const std::vector<boolean_function>& operands, bool invert_result = false);

    /*
     * Constructor for a variable, usable in other functions.
     * Variable name must not be empty.
     */
    boolean_function(const std::string& variable_name, bool invert_result = false);

    /*
     * Constructor for a constant, usable in other functions.
     */
    boolean_function(value constant);

    /*
     * Evaluates the function on the given inputs.
     * Inputs are a map from variable name to value.
     */
    value evaluate(const std::map<std::string, value>& inputs = {}) const;
    value operator()(const std::map<std::string, value>& inputs = {}) const;

    /*
     * Checks whether the function is the constant one.
     */
    bool is_constant_one() const;

    /*
     * Checks whether the function is the constant zero.
     */
    bool is_constant_zero() const;

    /*
     * Get all variable names used in this function.
     */
    std::set<std::string> get_variables() const;

    /*
     * Parse a function from a string representation.
     * Allowed notation:
     * NOT: !
     * AND: & * or space between terms
     * OR: | +
     * XOR: ^
     */
    static boolean_function from_string(std::string expression);

    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const boolean_function& f);

    // combine several existing functions
    boolean_function combine(operation op, const boolean_function& other) const;
    boolean_function operator&(const boolean_function& other) const;
    boolean_function operator|(const boolean_function& other) const;
    boolean_function operator^(const boolean_function& other) const;
    boolean_function& operator&=(const boolean_function& other);
    boolean_function& operator|=(const boolean_function& other);
    boolean_function& operator^=(const boolean_function& other);

    // negate function
    boolean_function operator!() const;

    // get the DNF representation of the function
    boolean_function to_dnf() const;

    /*
     * Get the truth table outputs of the function
     * WARNING: Exponential runtime in the number of variables!
     *
     * Output is the vector of output values when walking the truth table in ascending order.
     * The variable values are changed in order of appearance, i.e.:
     * first_var second_var | output_vector_index
     *     0         0      |   0
     *     1         0      |   1
     *     0         1      |   2
     *     1         1      |   3
     */
    std::vector<value> get_truth_table() const;

private:
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

    bool m_invert;

    bool m_holds_variable;
    bool m_holds_constant;

    std::string m_variable;

    value m_constant;

    operation m_op;
    std::vector<boolean_function> m_operands;
};

#endif
