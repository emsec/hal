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
#include <map>
#include <ostream>
#include <set>
#include <unordered_map>
#include <variant>
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
    class BooleanFunction final
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

        /// Represents the logic value that a Boolean function operates on.
        enum Value
        {
            ZERO = 0, /**< Represents a logical 0. */
            ONE  = 1, /**< Represents a logical 1 */
            Z,        /**< Represents a high-impedance value. */
            X         /**< Represents an undefined value. */
        };

        /**
         * Get the value as a string.
         *
         * @param[in] value - The value.
         * @returns A string describing the value.
         */
        static std::string to_string(Value value);

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
         * @returns The Boolean function on success, a string error message otherwise.
         */
        static std::variant<BooleanFunction, std::string> build(std::vector<Node>&& nodes);

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
         * Creates a constant multi-bit  Boolean function from a vector of values.
         * 
         * @param[in] values - The vector of values.
         * @returns The Boolean function.
         */
        static BooleanFunction Const(const std::vector<BooleanFunction::Value>& values);

        /**
         * Creates a constant multi-bit Boolean function of the given bit-size from an integer value.
         * 
         * @param[in] value - The integer value.
         * @param[in] size - The bit-size.
         * @returns The Boolean function.
         */
        static BooleanFunction Const(u64 value, u16 size);

        /**
         * Creates a Boolean function index of the given bit-size from an integer value.
         * 
         * @param[in] index - The integer value.
         * @param[in] size - The bit-size.
         * @returns The Boolean function.
         */
        static BooleanFunction Index(u16 index, u16 size);

        /**
         * Joins two Boolean functions by an 'AND' operation. 
         * Requires both Boolean functions to be of the specified bit-size.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns The joined Boolean function on success, a string error message otherwise.
         */
        static std::variant<BooleanFunction, std::string> And(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Joins two Boolean functions by an 'OR' operation. 
         * Requires both Boolean functions to be of the specified bit-size.
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns The joined Boolean function on success, a string error message otherwise.
         */
        static std::variant<BooleanFunction, std::string> Or(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Negates the given Boolean function. 
         * Requires the Boolean function to be of the specified bit-size.
         * 
         * @param[in] p0 - The Boolean function to negate.
         * @param[in] size - Bit-size of the operation.
         * @returns  The negated Boolean function on success, a string error message otherwise.
         */
        static std::variant<BooleanFunction, std::string> Not(BooleanFunction&& p0, u16 size);

        /**
         * Joins two Boolean functions by an 'XOR' operation. 
         * Requires both Boolean functions to be of the specified bit-size.
         * 
         * 
         * @param[in] p0 - First Boolean function.
         * @param[in] p1 - Second Boolean function.
         * @param[in] size - Bit-size of the operation.
         * @returns The joined Boolean function on success, a string error message otherwise.
         */
        static std::variant<BooleanFunction, std::string> Xor(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

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
         * @returns `true` if this Boolean functions is 'smaller', `false` otherwise.
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
         * @param type - The type to check for.
         * @returns `true` if the node is of the given type, `false` otherwise.
         */
        bool is(u16 type) const;

        /**
         * Checks whether the Boolean function is a variable.
         * 
         * @returns `true` if the Boolean function is a variable, `false` otherwise.
         */
        bool is_variable() const;

        /**
         * Checks whether the Boolean function is a constant.
         * 
         * @returns `true` if the Boolean function is a constant, `false` otherwise.
         */
        bool is_constant() const;

        /**
         * Checks whether the Boolean function is a constant with a specific value.
         * 
         * @param value - The value to check for.
         * @returns `true` if the Boolean function is a constant with the given value, `false` otherwise.
         */
        bool is_constant(u64 value) const;

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
         * @returns The Boolean function as a string.
         */
        std::string to_string() const;

        /**
         * Parses a Boolean function from a string expression.
         * 
         * @param[in] expression - Boolean function string.
         * @returns The Boolean function on success, a string error message otherwise.
         */
        static std::variant<BooleanFunction, std::string> from_string(const std::string& expression);

        ////////////////////////////////////////////////////////////////////////
        // Interface: Simplification / Substitution / Evaluation
        ////////////////////////////////////////////////////////////////////////

        /**
         * Simplifies a given Boolean function (e.g., "A & 0" to "0").
         * 
         * @returns The simplified Boolean function.
         */
        BooleanFunction simplify() const;

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
         * @returns The resulting Boolean function or a string with the error message.
         */
        std::variant<BooleanFunction, std::string> substitute(const std::string& variable_name, const BooleanFunction& function) const;

        /**
         * Short-hand Boolean function evaluation that symbolically evaluates
         * the function given the concrete single-bit variable values.
         * 
         * @param[in] inputs Maps a variable name to its concrete value.
         * @returns Boolean function value or error message string.
         */
        std::variant<Value, std::string> evaluate(const std::unordered_map<std::string, Value>& inputs) const;

        /**
         * Short-hand Boolean function evaluation that symbolically evaluates
         * the function given the concrete multi-bit variable values.
         * 
         * @param[in] inputs Maps a variable name to its concrete value.
         * @returns Boolean function value or error message string.
         */
        std::variant<std::vector<Value>, std::string> evaluate(const std::unordered_map<std::string, std::vector<Value>>& inputs) const;

        /**
         * Computes the truth table outputs for a Boolean function that only
         * consists of <= 10 variables with 1-bit, as the generation of a truth
         * table is exponential in the number of variables.
         * 
         * @param[in] variables - Varible in order of truth table input.
         * @param[in] remove_unknown_variables - Switch to remove variables from the truth table if not found in function.
         * @returns Truth table where entry 0 refers to the bit 0, error message otherwise.
         */
        std::variant<std::vector<std::vector<Value>>, std::string> compute_truth_table(const std::vector<std::string>& ordered_variables = {}, bool remove_unknown_variables = false) const;

        /**
         * Translates the Boolean function into the z3 expression representation.
         *
         * @param[in,out] context - Z3 context to generate expressions.
         * @param[in] var2expr - Maps input variables to expression.
         * @returns Z3 representation of the Boolean function.
         */
        z3::expr to_z3(z3::context& context, const std::map<std::string, z3::expr>& var2expr = {}) const;

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

        /// Checks whether the Boolean function is valid.
        ///
        /// @returns Validated Boolean function on success, error message string otherwise.
        static std::variant<BooleanFunction, std::string> validate(BooleanFunction&& function);

        /// Computes the coverage value of each node in the Boolean function.
        std::vector<u32> compute_node_coverage() const;

        /// Implements the Quine-McCluskey algorithm to simplify Boolean functions.
        ///
        /// @param[in] function - Boolean function to simplify.
        /// @returns Simplified boolean function on success, error message string otherwise.
        static std::variant<BooleanFunction, std::string> quine_mccluskey(const BooleanFunction& function);

        ////////////////////////////////////////////////////////////////////////
        // Member
        ////////////////////////////////////////////////////////////////////////

        /// refers to the list of nodes in reverse polish notation
        std::vector<BooleanFunction::Node> m_nodes{};
    };

    template<>
    std::vector<std::string> EnumStrings<BooleanFunction::Value>::data;

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

        /// store node type of Boolean function
        u16 type;
        /// stores bit-size of Boolean function node
        u16 size;
        /// stores constant value
        std::vector<BooleanFunction::Value> constant{};
        /// stores index value
        u16 index{};
        /// stores variable name
        std::string variable{};

        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /// constructor to generate an operation node
        static Node Operation(u16 _type, u16 _size);
        /// constructor to generate a constant node
        static Node Constant(const std::vector<BooleanFunction::Value> constant);
        /// constructor to generate an index node
        static Node Index(u16 _index, u16 _size);
        /// constructor to generate a constant node
        static Node Variable(const std::string variable, u16 _size);

        /// comparison operators
        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
        bool operator<(const Node& other) const;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

        /// Clones the Boolean function node
        Node clone() const;

        /// Human-readable description of node for debugging / logging
        std::string to_string() const;

        /// Returns arity of node, see Node::get_arity(u16 type) for details.
        u16 get_arity() const;

        /**
         * Returns arity of node, i.e how many operand nodes does the node have.
         * For example, an 'And' node has an arity of 2 as it has 2 operands, a 
         * 'Not' node has an arity of 1 respectively. Operand nodes such as a 
         * 'Constant' or 'Variable' have an arity of 0.
         *
         * @returns Arity of node.
         */
        static u16 get_arity(u16 type);

        /// Checks whether node is of a specific type.
        bool is(u16 _type) const;

        /// Checks whether node is of type 'Constant'.
        bool is_constant() const;
        /// Checks whether node is of type 'Constant' and has specific value.
        bool is_constant(u64 value) const;

        /// Checks whether node is of type 'Index'.
        bool is_index() const;
        /// Checks whether node is of type 'Index' and has specifif value.
        bool is_index(u16 value) const;

        /// Checks whether node is of type 'Variable'.
        bool is_variable() const;
        /// Checks whether node is of type 'Variable' and has specific value.
        bool is_variable(const std::string& value) const;

        /// Checks whether node is an operation.
        bool is_operation() const;
        /// Checks whether node is an operand, i.e. 'Constant' or 'Variable'.
        bool is_operand() const;

        /// Short-hand check whether the node is a commutative operator.
        bool is_commutative() const;

    private:
        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /// constructor to initialize all Node fields
        Node(u16 _type, u16 _size, std::vector<BooleanFunction::Value> _constant, u16 _index, std::string variable);
    };

    /**
     * List of available node types in a Boolean function.
     *
     * @ingroup netlist
     */
    struct BooleanFunction::NodeType final
    {
        static constexpr u16 And = 0x0000;
        static constexpr u16 Or  = 0x0001;
        static constexpr u16 Not = 0x0002;
        static constexpr u16 Xor = 0x0003;

        static constexpr u16 Add = 0x0010;

        static constexpr u16 Concat = 0x0100;
        static constexpr u16 Slice  = 0x0101;
        static constexpr u16 Zext   = 0x0102;

        static constexpr u16 Constant = 0x1000;
        static constexpr u16 Index    = 0x1001;
        static constexpr u16 Variable = 0x1002;
    };
}    // namespace hal
