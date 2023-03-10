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
#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/result.h"
#include "z3++.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <ostream>
#include <set>
#include <unordered_map>
#include <vector>

namespace hal
{
    /**
     * A BooleanFunction represents a symbolic expression (e.g., "A & B") in 
     * order to abstract the (semantic) functionality of a single netlist gate 
     * (or even a complex subcircuit comprising multiple gates) in a formal 
     * manner. To this end, the BooleanFunction class is able to construct and 
     * display arbitrarily-nested expressions, enable symbolic simplification
     * (e.g., simplify "A & 0" to "0"), and translate Boolean functions to the
     * SAT / SMT solver domain to use the solve constraint formulas.
     *
     * @ingroup netlist
     */
    class NETLIST_API BooleanFunction final
    {
    public:
        ////////////////////////////////////////////////////////////////////////
        // Forward Declarations
        ////////////////////////////////////////////////////////////////////////

        /*
         * We forward declare the abstract syntax tree nodes and types of the 
         * Boolean function in order to re-use BooleanFunction both as a class 
         * and a namespace for its underlying node data structures.
         */

        struct Node;        /// represents an abstract syntax tree node
        struct NodeType;    /// represents the type of the node

        /**
         * Represents the logic value that a Boolean function operates on.
         */
        enum Value
        {
            ZERO = 0,  /**< Represents a logical 0. */
            ONE  = 1,  /**< Represents a logical 1. */
            X    = -1, /**< Represents an undefined value. */
            Z    = -2  /**< Represents a high-impedance value. */
        };

        /**
         * Get the value as a string.
         *
         * @param[in] value - The value.
         * @returns A string describing the value.
         */
        static std::string to_string(Value value);

        /**
         * Convert the given bit-vector to its string representation in the given base.
         * 
         * @param[in] value - The value as a bit-vector.
         * @param[in] base - The base that the values should be converted to. Valid values are 2 (default), 8, 10, and 16.
         * @returns A string representing the values in the given base on success, an error otherwise.
         */
        static Result<std::string> to_string(const std::vector<BooleanFunction::Value>& value, u8 base = 2);

        /**
         * Convert the given bit-vector to its unsigned 64-bit integer representation.
         * 
         * @param[in] value - The value as a bit-vector.
         * @returns A 64-bit integer representing the values on success, an error otherwise.
         */
        static Result<u64> to_u64(const std::vector<BooleanFunction::Value>& value);

        /**
         * Output stream operator that forwards to_string of a value.
         *
         * @param[in] os - The stream to write to.
         * @param[in] value - The value.
         * @returns A reference to output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, Value value);

        ////////////////////////////////////////////////////////////////////////
        // Constructors / Factories, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /**
         * Constructs an empty / invalid Boolean function.
         */
        explicit BooleanFunction();

        /**
         * Builds and validates a Boolean function from a vector of nodes.
         * 
         * @param[in] nodes - Vector of Boolean function nodes.
         * @returns Ok() and the Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> build(std::vector<Node>&& nodes);

        /**
         * Creates a Boolean function of the given bit-size comprising only a variable of the specified name.
         * 
         * @param[in] name - The name of the variable.
         * @param[in] size - The bit-size. Defaults to 1.
         * @returns The Boolean function.
         */
        static BooleanFunction Var(const std::string& name, u16 size = 1);

        /**
         * Creates a constant single-bit Boolean function from a value.
         * 
         * @param[in] value - The value.
         * @returns The Boolean function.
         */
        static BooleanFunction Const(const BooleanFunction::Value& value);

        /**
         * Creates a constant multi-bit Boolean function from a vector of values.
         * 
         * @param[in] value - The vector of values.
         * @returns The Boolean function.
         */
        static BooleanFunction Const(const std::vector<BooleanFunction::Value>& value);

        /**
         * Creates a constant multi-bit Boolean function of the given bit-size from an integer value.
         * 
         * @param[in] value - The integer value.
         * @param[in] size - The bit-size.
         * @returns The Boolean function.
         */
        static BooleanFunction Const(u64 value, u16 size);

        /**
         * Creates an index for a Boolean function of the given bit-size from an integer value.
         * 
         * @param[in] index - The integer value.
         * @param[in] size - The bit-size.
         * @returns The Boolean function.
         */
        static BooleanFunction Index(u16 index, u16 size);

        /**
         * Joins two Boolean functions by applying an 'AND' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> And(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'OR' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Or(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Negates the given Boolean function. 
         * Requires the Boolean function to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * @param[in] p0 - The Boolean function to negate.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the negated Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Not(BooleanFunction&& p0, u16 size);

        /**
         * Joins two Boolean functions by applying an 'XOR' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Xor(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'ADD' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Add(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'SUB' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Sub(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'MUL' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Mul(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'SDIV' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Sdiv(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'UDIV' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Udiv(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'SREM' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Srem(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by applying an 'UREM' operation. 
         * Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Urem(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Returns the slice `[i:j]` of a Boolean function specified by a start index `i` and an end index `j` beginning at 0.
         * Note that slice `[i:j]` includes positions `i` and `j` as well. 
         * 
         * @param[in] p0 - Boolean function to slice.
         * @param[in] p1 - Boolean function start index.
         * @param[in] p2 - Boolean function end index.
         * @param[in] size - Bit-size of the resulting Boolean function slice, i.e., |p2| - |p1| + 1.
         * @returns OK() and the Boolean function slice on success, an error otherwise.
         */
        static Result<BooleanFunction> Slice(BooleanFunction&& p0, BooleanFunction&& p1, BooleanFunction&& p2, u16 size);

        /**
         * Concatenates two Boolean functions of potentially different bit-sizes `n` and `m` to form a single Boolean function of bit-size `n+m`.
         * 
         * @param[in] p0 - First Boolean function (MSBs).
         * @param[in] p1 - Second Boolean function (LSBs).
         * @param[in] size - Bit-size of the concatenated Boolean function.
         * @returns Ok() and the concatenated Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Concat(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Zero-extends a Boolean function to the specified bit-size.
         * 
         * @param[in] p0 - Boolean function to extend.
         * @param[in] p1 - Constant Boolean function describing the size of the zero-extended result.
         * @param[in] size - Bit-size of the zero-extended Boolean function.
         * @returns Ok() and the extended Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Zext(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Sign-extends a Boolean function to the specified bit-size.
         * 
         * @param[in] p0 - Boolean function to extend.
         * @param[in] p1 - Constant Boolean function describing the size of the sign-extended result.
         * @param[in] size - Bit-size of the sign-extended Boolean function.
         * @returns Ok() and the extended Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Sext(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by an equality check that produces a single-bit result.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation (always =1).
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Eq(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by a signed less-than-equal check that produces a single-bit result.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation (always =1).
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Sle(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by a signed less-than check that produces a single-bit result.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation (always =1).
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Slt(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by an unsigned less-than-equal check that produces a single-bit result.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation (always =1).
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Ule(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by an unsigned less-than check that produces a single-bit result.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation (always =1).
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Ult(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins three Boolean functions by an if-then-else operation with p0 as the condition, p1 as true-case, and p2 as false-case.
         * Requires `p1` to be of bit-size 1, both Boolean functions `p1` and `p2` to be of the specified bit-size, and produces a new Boolean function of the specified bit-size.
         * 
         * @param[in] p0 - Boolean function condition.
         * @param[in] p1 - Boolean function for true-case.
         * @param[in] p2 - Boolean function for false-case.
         * @param[in] size - Bit-size of the operation, i.e., size of p1 and p2.
         * @returns Ok() and the joined Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> Ite(BooleanFunction&& p0, BooleanFunction&& p1, BooleanFunction&& p2, u16 size);

        /**
         * The ostream operator that forwards to_string of a boolean function.
         *
         * @param[in] os - the stream to write to.
         * @param[in] f - the function.
         * @returns A reference to os.
         */
        friend std::ostream& operator<<(std::ostream& os, const BooleanFunction& f);

        /**
         * Joins two Boolean functions by an 'AND' operation. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction operator&(const BooleanFunction& other) const;

        /**
         * Joins two Boolean functions by an 'AND' operation in-place. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction& operator&=(const BooleanFunction& other);

        /**
         * Negates the Boolean function. 
         * 
         * @returns The negated Boolean function.
         */
        BooleanFunction operator~() const;

        /**
         * Joins two Boolean functions by an 'OR' operation. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction operator|(const BooleanFunction& other) const;

        /**
         * Joins two Boolean functions by an 'OR' operation in-place. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction& operator|=(const BooleanFunction& other);

        /**
         * Joins two Boolean functions by an 'XOR' operation. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction operator^(const BooleanFunction& other) const;

        /**
         * Joins two Boolean functions by an 'XOR' operation in-place. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction& operator^=(const BooleanFunction& other);

        /**
         * Joins two Boolean functions by an 'ADD' operation. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction operator+(const BooleanFunction& other) const;

        /**
         * Joins two Boolean functions by an 'ADD' operation in-place. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction& operator+=(const BooleanFunction& other);

        /**
         * Joins two Boolean functions by an 'SUB' operation. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction operator-(const BooleanFunction& other) const;

        /**
         * Joins two Boolean functions by an 'SUB' operation in-place. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction& operator-=(const BooleanFunction& other);

        /**
         * Joins two Boolean functions by an 'MUL' operation. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction operator*(const BooleanFunction& other) const;

        /**
         * Joins two Boolean functions by an 'MUL' operation in-place. 
         * Requires both Boolean functions to be of the same bit-size.
         * \warning Fails if the Boolean functions have different bit-sizes.
         * 
         * @param[in] other - The other Boolean function.
         * @returns The joined Boolean function.
         */
        BooleanFunction& operator*=(const BooleanFunction& other);

        /**
         * Checks whether two Boolean functions are equal.
         * 
         * @param[in] other - The other Boolean function.
         * @returns `true` if the Boolean functions are equal, `false` otherwise.
         */
        bool operator==(const BooleanFunction& other) const;

        /**
         * Checks whether two Boolean functions are unequal.
         * 
         * @param[in] other - The other Boolean function.
         * @returns `true` if the Boolean functions are unequal, `false` otherwise.
         */
        bool operator!=(const BooleanFunction& other) const;

        /**
         * Checks whether this Boolean function is 'smaller' than the `other` Boolean function.
         * 
         * @param[in] other - The other Boolean function.
         * @returns `true` if this Boolean function is 'smaller', `false` otherwise.
         */
        bool operator<(const BooleanFunction& other) const;

        /**
         * Checks whether the Boolean function is empty.
         * 
         * @returns `true` if the Boolean function is empty, `false` otherwise.
         */
        bool is_empty() const;

        ////////////////////////////////////////////////////////////////////////
        // Interface: Nodes, Sizes, and Checks
        ////////////////////////////////////////////////////////////////////////

        /**
         * Clones the Boolean function.
         * 
         * @returns The cloned Boolean function.
         */
        BooleanFunction clone() const;

        /**
         * Returns the bit-size of the Boolean function.
         * 
         * @returns The bit-size of the Boolean function. 
         */
        u16 size() const;

        /**
         * Checks whether the top-level node of the Boolean function is of a specific type.
         * 
         * @param[in] type - The type to check for.
         * @returns `true` if the node is of the given type, `false` otherwise.
         */
        bool is(u16 type) const;

        /**
         * Checks whether the top-level node of the Boolean function is of type `Variable`.
         * 
         * @returns `true` if the top-level node of the Boolean function is of type `Variable`, `false` otherwise.
         */
        bool is_variable() const;

        /**
         * Checks whether the top-level node of the Boolean function is of type `Variable` and holds a specific variable name.
         * 
         * @param variable_name - The variable name to check for.
         * @returns `true` if the Boolean function is of type `Variable` and holds the given variable name, `false` otherwise.
         */
        bool has_variable_name(const std::string& variable_name) const;

        /**
         * Get the variable name of the top-level node of the Boolean function of type `Variable`.
         * 
         * @returns The variable name on success, an error otherwise.
         */
        Result<std::string> get_variable_name() const;

        /**
         * Checks whether the top-level node of the Boolean function is of type `Constant`.
         * 
         * @returns `true` if the top-level node of the Boolean function is of type `Constant`, `false` otherwise.
         */
        bool is_constant() const;

        /**
         * Checks whether the top-level node of the Boolean function is of type `Constant` and holds a specific value.
         * 
         * @param[in] value - The constant value to check for.
         * @returns `true` if the Boolean function is of type `Constant` and holds the given value, `false` otherwise.
         */
        bool has_constant_value(u64 value) const;

        /**
         * Get the constant value of the top-level node of the Boolean function of type `Constant` as long as it has a size <= 64-bit.
         * 
         * @returns The constant value on success, an error otherwise.
         */
        Result<u64> get_constant_value() const;

        /** 
         * Checks whether the top-level node of the Boolean function is of type `Index`. 
         * 
         * @returns `true` if the top-level node of the Boolean function is of type `Index`, `false` otherwise.
         */
        bool is_index() const;

        /**
         * Checks whether the top-level node of the Boolean function is of type `Index` and holds a specific value.
         * 
         * @param[in] index - The index value to check for.
         * @returns `true` if the Boolean function is of type `Index` and holds the given value, `false` otherwise.
         */
        bool has_index_value(u16 index) const;

        /**
         * Get the index value of the top-level node of the Boolean function of type `Index`.
         * 
         * @returns The constant value on success, an error otherwise.
         */
        Result<u16> get_index_value() const;

        /**
         * Returns the top-level node of the Boolean function.
         * \warning Fails if the Boolean function is empty.
         * 
         * @returns The top-level node.
         */
        const BooleanFunction::Node& get_top_level_node() const;

        /**
         * Returns the number of nodes in the Boolean function.
         * 
         * @returns The number of nodes. 
         */
        u32 length() const;

        /**
         * Returns the reverse polish notation list of the Boolean function nodes.
         *
         * @returns A vector of nodes.
         */
        const std::vector<BooleanFunction::Node>& get_nodes() const;

        /**
         * Returns the parameter list of the top-level node of the Boolean function.
         * 
         * @returns A vector of Boolean functions.
         */
        std::vector<BooleanFunction> get_parameters() const;

        /**
         * Returns the set of variable names used by the Boolean function.
         *
         * @returns A set of variable names.
         */
        std::set<std::string> get_variable_names() const;

        ////////////////////////////////////////////////////////////////////////
        // Interface: String Translation
        ////////////////////////////////////////////////////////////////////////

        /**
         * Translates the Boolean function into its string representation.
         * 
         * @param[in] printer - A function a node and its operands as inputs and outputs their string representation. Allows for different grammers to be printed. Defaults to the 'default_printer'.
         * @returns The Boolean function as a string.
         */
        std::string to_string(std::function<Result<std::string>(const BooleanFunction::Node& node, std::vector<std::string>&& operands)>&& printer = default_printer) const;

        /**
         * Parses a Boolean function from a string expression.
         * 
         * @param[in] expression - Boolean function string.
         * @returns Ok() and the Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> from_string(const std::string& expression);

        ////////////////////////////////////////////////////////////////////////
        // Interface: Simplification / Substitution / Evaluation
        ////////////////////////////////////////////////////////////////////////

        /**
         * Simplifies the Boolean function.
         * 
         * @returns The simplified Boolean function.
         */
        BooleanFunction simplify() const;

        /**
         * Simplifies the Boolean function using only the local simplification.
         * 
         * @returns The simplified Boolean function.
         */
        BooleanFunction simplify_local() const;

        /**
         * Substitute a variable name with another one, i.e., renames the variable.
         * The operation is applied to all instances of the variable in the function.
         *
         * @param[in] old_variable_name - The old variable name to substitute.
         * @param[in] new_variable_name - The new variable name.
         * @returns The resulting Boolean function.
         */
        BooleanFunction substitute(const std::string& old_variable_name, const std::string& new_variable_name) const;

        /**
         * Substitute a variable with another Boolean function.
         * The operation is applied to all instances of the variable in the function.
         *
         * @param[in] variable_name - The variable to substitute.
         * @param[in] function - The function replace the variable with.
         * @returns Ok() and the resulting Boolean function on success, an error otherwise.
         */
        Result<BooleanFunction> substitute(const std::string& variable_name, const BooleanFunction& function) const;

        /**
         * Substitute multiple variables with other Boolean functions at once.
         * The operation is applied to all instances of the variable in the function.
         *
         * @param[in] substitutions - A map from the variable name to the function to replace the variable with.
         * @returns Ok() and the resulting Boolean function on success, an error otherwise.
         */
        Result<BooleanFunction> substitute(const std::map<std::string, BooleanFunction>& substitutions) const;

        /**
         * Evaluates a Boolean function comprising only single-bit variables using the given input values.
         * 
         * @param[in] inputs - A map from variable name to input value.
         * @returns Ok() and the resulting value on success, an error otherwise.
         */
        Result<Value> evaluate(const std::unordered_map<std::string, Value>& inputs) const;

        /**
         * Evaluates a Boolean function comprising multi-bit variables using the given input values.
         * 
         * @param[in] inputs - A map from variable name to a vector of input values.
         * @returns Ok() and the resulting value on success, an error otherwise.
         */
        Result<std::vector<Value>> evaluate(const std::unordered_map<std::string, std::vector<Value>>& inputs) const;

        /**
         * Computes the truth table outputs for a Boolean function that comprises <= 10 single-bit variables.
         * \warning The generation of the truth table is exponential in the number of parameters.
         * 
         * @param[in] ordered_variables - A vector describing the order of input variables used to generate the truth table. Defaults to an empty vector.
         * @param[in] remove_unknown_variables - Set `true` to remove variables from the truth table that are not present within the Boolean function, `false` otherwise. Defaults to `false`.
         * @returns Ok() and a vector of values representing the truth table output on success, an error otherwise.
         */
        Result<std::vector<std::vector<Value>>> compute_truth_table(const std::vector<std::string>& ordered_variables = {}, bool remove_unknown_variables = false) const;

        /**
         * Prints the truth table for a Boolean function that comprises <= 10 single-bit variables.
         * \warning The generation of the truth table is exponential in the number of parameters.
         * 
         * @param[in] ordered_variables - A vector describing the order of input variables used to generate the truth table. Defaults to an empty vector.
         * @param[in] function_name - The name of the Boolean function to be printed as header of the output columns.
         * @param[in] remove_unknown_variables - Set `true` to remove variables from the truth table that are not present within the Boolean function, `false` otherwise. Defaults to `false`.
         * @returns Ok() and a string representing the truth table on success, an error otherwise.
         */
        Result<std::string> get_truth_table_as_string(const std::vector<std::string>& ordered_variables = {}, std::string function_name = "", bool remove_unknown_variables = false) const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Translates the Boolean function into the z3 expression representation.
         *
         * @param[in,out] context - Z3 context to generate expressions.
         * @param[in] var2expr - Maps input variables to expression.
         * @returns Z3 representation of the Boolean function.
         */
        [[deprecated("Will be removed in a future version. Use z3_utils::to_z3() in the z3_utils plugin instead.")]] z3::expr to_z3(z3::context& context,
                                                                                                                                    const std::map<std::string, z3::expr>& var2expr = {}) const;

    private:
        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /// Constructs a Boolean function with a reverse-polish notation node list.
        explicit BooleanFunction(std::vector<BooleanFunction::Node>&& nodes);

        /** 
         * Constructs a Boolean function from a single node and an arbitrary list
         * of Boolean function parameters.
         * 
         * @param[in] node - Boolean function node.
         * @param[in] p - Boolean function node parameters.
         * @returns Initialized Boolean function.
         */
        template<typename... T, typename = std::enable_if_t<std::conjunction_v<std::is_same<T, BooleanFunction>...>>>
        explicit BooleanFunction(BooleanFunction::Node&& node, T&&... p)
        {
            auto size = 1;
            ((size += p.size()), ...);
            this->m_nodes.reserve(size);

            (this->m_nodes.insert(this->m_nodes.end(), p.m_nodes.begin(), p.m_nodes.end()), ...);
            this->m_nodes.emplace_back(std::move(node));
        }

        /** 
         * Constructs a Boolean function from a single node and a list of parameters.
         * 
         * @param[in] node - Boolean function node.
         * @param[in] p - Boolean function node parameters.
         * @returns Initialized Boolean function.
         */
        explicit BooleanFunction(BooleanFunction::Node&& node, std::vector<BooleanFunction>&& p);

        ////////////////////////////////////////////////////////////////////////
        // Internal Interface
        ////////////////////////////////////////////////////////////////////////

        /// Returns the Boolean function in reverse-polish notation.
        std::string to_string_in_reverse_polish_notation() const;

        /**
         * Translate a given Boolean function node into a human-readable string.
         *
         * @param[in] node The node of a Boolean function.
         * @param[in] operands The operands of the node .
         * @returns A string on success or an error message otherwise.
         */
        static Result<std::string> default_printer(const BooleanFunction::Node& node, std::vector<std::string>&& operands);

        /// Checks whether the Boolean function is valid.
        ///
        /// @returns Validated Boolean function on success, error message string otherwise.
        static Result<BooleanFunction> validate(BooleanFunction&& function);

        /// Computes the coverage value of each node in the Boolean function.
        std::vector<u32> compute_node_coverage() const;

        ////////////////////////////////////////////////////////////////////////
        // Member
        ////////////////////////////////////////////////////////////////////////

        /// refers to the list of nodes in reverse polish notation
        std::vector<BooleanFunction::Node> m_nodes{};
    };

    template<>
    std::map<BooleanFunction::Value, std::string> EnumStrings<BooleanFunction::Value>::data;

    /**
     * Node refers to an abstract syntax tree node of a Boolean function. A node
     * is an abstract base class for either an operation (e.g., AND, XOR) or an 
     * operand (e.g., a signal name variable).
     *
     * # Developer Note
     * We deliberately opted to have a single (littered) memory space for a node
     * i.e. no separation from operation / operand nodes via inheritance, due to 
     * optimization reasons to keep node data closely together and prevent the 
     * use of smart pointers to manage memory safely.
     *
     * @ingroup netlist
     */
    struct BooleanFunction::Node final
    {
        ////////////////////////////////////////////////////////////////////////
        // Member
        ////////////////////////////////////////////////////////////////////////

        /// The type of the node.
        u16 type;
        /// The bit-size of the node.
        u16 size;
        /// The (optional) constant value of the node.
        std::vector<BooleanFunction::Value> constant{};
        /// The (optional) index value of the node.
        u16 index{};
        /// The (optional) variable name of the node.
        std::string variable{};

        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /**
         * Constructs an 'operation' node.
         * 
         * @param type - The type of the operation.
         * @param size - The bit-size of the operation.
         * @returns The node.
         */
        static Node Operation(u16 type, u16 size);

        /**
         * Constructs a 'constant' node.
         * 
         * @param value - The constant value of the node.
         * @returns The node.
         */
        static Node Constant(const std::vector<BooleanFunction::Value> value);

        /**
         * Constructs an 'index' node.
         * 
         * @param index - The index value of the node.
         * @param size - The bit-size of the node.
         * @returns The node.
         */
        static Node Index(u16 index, u16 size);

        /**
         * Constructs a 'variable' node.
         * 
         * @param variable - The variable name of the node.
         * @param size - The bit-size of the node.
         * @returns The node.
         */
        static Node Variable(const std::string variable, u16 size);

        /**
         * Checks whether two Boolean function nodes are equal.
         * 
         * @param[in] other - The other Boolean function node.
         * @returns `true` if the Boolean function nodes are equal, `false` otherwise.
         */
        bool operator==(const Node& other) const;

        /**
         * Checks whether two Boolean function nodes are unequal.
         * 
         * @param[in] other - The other Boolean function node.
         * @returns `true` if the Boolean function nodes are unequal, `false` otherwise.
         */
        bool operator!=(const Node& other) const;

        /**
         * Checks whether this Boolean function node is 'smaller' than the `other` Boolean function node.
         * 
         * @param[in] other - The other Boolean function node.
         * @returns `true` if this Boolean function node is 'smaller', `false` otherwise.
         */
        bool operator<(const Node& other) const;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

        /**
         * Clones the Boolean function node.
         * 
         * @returns The cloned Boolean function node. 
         */
        Node clone() const;

        /**
         * Translates the Boolean function node into its string representation.
         * 
         * @returns The Boolean function node as a string.
         */
        std::string to_string() const;

        /**
         * Returns the arity of the Boolean function node, i.e., the number of parameters.
         * 
         * @return The arity. 
         */
        u16 get_arity() const;

        /**
         * Returns the arity for a Boolean function node of the given type, i.e., the number of parameters.
         *
         * @returns The arity.
         */
        static u16 get_arity_of_type(u16 type);

        /**
         *  Checks whether the Boolean function node is of a specific type.
         * 
         * @param type - The type to check for.
         * @returns `true` if the node is of the given type, `false` otherwise.
         */
        bool is(u16 type) const;

        /**
         * Checks whether the Boolean function node is of type `Constant`.
         * 
         * @returns `true` if the Boolean function node is of type `Constant`, `false` otherwise.
         */
        bool is_constant() const;

        /**
         * Checks whether the Boolean function node is of type `Constant` and holds a specific value.
         * 
         * @param value - The value to check for.
         * @returns `true` if the Boolean function node is of type `Constant` and holds the given value, `false` otherwise.
         */
        bool has_constant_value(u64 value) const;

        /**
         * Get the constant value of the node of type `Constant` as long as it has a size <= 64-bit.
         * 
         * @returns The constant value on success, an error otherwise.
         */
        Result<u64> get_constant_value() const;

        /**
         * Checks whether the Boolean function node is of type `Index`.
         * 
         * @returns `true` if the Boolean function node is of type `Index`, `false` otherwise.
         */
        bool is_index() const;

        /**
         * Checks whether the Boolean function node is of type `Index` and holds a specific value.
         * 
         * @param value - The value to check for.
         * @returns `true` if the Boolean function node is of type `Index` and holds the given value, `false` otherwise.
         */
        bool has_index_value(u16 value) const;

        /**
         * Get the index value of node of type `Index`.
         * 
         * @returns The index value on success, an error otherwise.
         */
        Result<u16> get_index_value() const;

        /**
         * Checks whether the Boolean function node is of type `Variable`.
         * 
         * @returns `true` if the Boolean function node is of type `Variable`, `false` otherwise.
         */
        bool is_variable() const;

        /**
         * Checks whether the Boolean function node is of type `Variable` and holds a specific variable name.
         * 
         * @param variable_name - The variable name to check for.
         * @returns `true` if the Boolean function node is of type `Variable` and holds the given variable name, `false` otherwise.
         */
        bool has_variable_name(const std::string& variable_name) const;

        /**
         * Get the variable name of node of type `Variable`.
         * 
         * @returns The variable name on success, an error otherwise.
         */
        Result<std::string> get_variable_name() const;

        /**
         * Checks whether the Boolean function node is an operation node.
         * 
         * @returns `true` if the Boolean function node is an operation node, `false` otherwise.
         */
        bool is_operation() const;

        /**
         * Checks whether the Boolean function node is an operand node.
         * 
         * @returns `true` if the Boolean function node is an operand node, `false` otherwise.
         */
        bool is_operand() const;

        /**
         * Checks whether the Boolean function node is commutative.
         * 
         * @returns `true` if the Boolean function node is commutative, `false` otherwise.
         */
        bool is_commutative() const;

    private:
        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /// constructor to initialize all Node fields
        Node(u16 _type, u16 _size, std::vector<BooleanFunction::Value> _constant, u16 _index, std::string variable);
    };

    /**
     * Holds all node types available in a Boolean function.
     *
     * @ingroup netlist
     */
    struct BooleanFunction::NodeType final
    {
        static constexpr u16 And = 0x0000;
        static constexpr u16 Or  = 0x0001;
        static constexpr u16 Not = 0x0002;
        static constexpr u16 Xor = 0x0003;

        static constexpr u16 Add  = 0x0010;
        static constexpr u16 Sub  = 0x0011;
        static constexpr u16 Mul  = 0x0012;
        static constexpr u16 Sdiv = 0x0013;
        static constexpr u16 Udiv = 0x0014;
        static constexpr u16 Srem = 0x0015;
        static constexpr u16 Urem = 0x0016;

        static constexpr u16 Concat = 0x0100;
        static constexpr u16 Slice  = 0x0101;
        static constexpr u16 Zext   = 0x0102;
        static constexpr u16 Sext   = 0x0103;

        static constexpr u16 Eq  = 0x0400;
        static constexpr u16 Sle = 0x0401;
        static constexpr u16 Slt = 0x0402;
        static constexpr u16 Ule = 0x0403;
        static constexpr u16 Ult = 0x0404;
        static constexpr u16 Ite = 0x0405;

        static constexpr u16 Constant = 0x1000;
        static constexpr u16 Index    = 0x1001;
        static constexpr u16 Variable = 0x1002;
    };
}    // namespace hal
