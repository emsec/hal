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

    // empty operands behaves like constant X
    // single operand behaves like copy constructor
    boolean_function(operation op, const std::vector<boolean_function>& operands, bool invert_result = false);
    boolean_function(const std::string& variable, bool invert_result = false);
    boolean_function(value constant);

    value evaluate(const std::map<std::string, value>& inputs = {}) const;
    value operator()(const std::map<std::string, value>& inputs = {}) const;

    bool is_constant_one() const;
    bool is_constant_zero() const;

    std::set<std::string> get_variables() const;

    static boolean_function from_string(std::string expression);
    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const boolean_function& f);

    boolean_function combine(operation op, const boolean_function& other) const;
    boolean_function operator&(const boolean_function& other) const;
    boolean_function operator|(const boolean_function& other) const;
    boolean_function operator^(const boolean_function& other) const;

    boolean_function operator!() const;

    boolean_function to_dnf() const;

    std::vector<value> get_truth_table() const;

private:
    std::string to_string_internal() const;

    boolean_function replace_xors() const;

    boolean_function propagate_negations(bool negate_term = false) const;

    std::vector<boolean_function> expand_ands_internal(const std::vector<std::vector<boolean_function>>& sub_primitives, u32 i) const;
    std::vector<boolean_function> get_primitives() const;
    boolean_function expand_ands() const;

    boolean_function optimize_constants() const;

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
