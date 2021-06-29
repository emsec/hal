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
        ////////////////////////////////////////////////////////////////////////
        // Forward Declarations
        ////////////////////////////////////////////////////////////////////////

        /*
         * We forward declare the abstract syntax tree nodes and types of the 
         * Boolean function in order to re-use BooleanFunction both as a class 
         * and a namespace for its underlying node data structures.
         */

        struct Node;     /// represents an abstract syntax tree node
        struct NodeType; /// represents the type of the node

        struct OperationNode; /// represents an operation node (e.g., AND)
        struct OperandNode;   /// reprsents an operand node (e.g., a variable)

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

        enum class content_type
        {
            VARIABLE,
            CONSTANT,
            TERMS
        };
        
        enum class operation
        {
            AND,
            OR,
            XOR
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
         * Returns content type of Boolean function.
         *
         * @returns Content type.
         */
        content_type get_type() const;

        /**
         * Returns operation identifier of Boolean function.
         *
         * @returns Operation.
         */
        operation get_operation() const;

        /** 
         * Checks whether Boolean function is negated.
         *
         * @returns True in case Boolean function is negated, false otherwise.
         */
        bool is_neg() const;

        /**
         * Returns list of operands as Boolean function.
         *
         * @returns List of operands.
         */
        const std::vector<BooleanFunction>& get_operands() const;

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
         * The variables can be efficiently substituted by passing in the var_to_expr map. 
         *
         * @param[in] var_to_expr - A mapping from input variable to expression that this variable is substituted with.
         * @param[in,out] context - The z3 context.
         * @returns The z3 representation of the Boolean function.
         */
        z3::expr to_z3(z3::context& context, const std::map<std::string, z3::expr>& var_to_expr = {}) const;

        /**
         * Returns the list of nodes of the Boolean function in reverse-polish
         * notation, i.e. top-level node is at last vector.
         *
         * WARNING: This is a temporary function as long as the underlying 
         *          BooleanFunction is using the recursive data structure.
         *
         * @returns The vector of abstract syntax tree nodes.
         */
        std::vector<std::unique_ptr<Node>> get_reverse_polish_notation() const;

    protected:
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

        std::string m_variable;

        Value m_constant;

        content_type m_content;
        operation m_op;
        std::vector<BooleanFunction> m_operands;
    };

    template<>
    std::vector<std::string> EnumStrings<BooleanFunction::Value>::data;

    /**
     * Node refers to an abstract syntax tree node of a Boolean function. A node
     * is an abstract base class for either an operation (e.g., AND, XOR) or an 
     * operand (e.g., a signal name variable).
     *
     * @ingroup netlist
     */
    struct BooleanFunction::Node {
        ////////////////////////////////////////////////////////////////////////
        // Member
        ////////////////////////////////////////////////////////////////////////

        /// store node type of Boolean function
        const u16 type;
        /// stores bit-size of Boolean function node
        const u16 size;

        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /// constructor to initialize fields
        Node(u16 _type, u16 _size);
        /// default destructor to allow sub-class override
        virtual ~Node() = default;

        /// comparison operators
        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
        bool operator <(const Node& other) const;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

        /**
         * Safe-downcast to return a node as operation / operand sub-class.
         *
         * @tparam T Sub-class of Node, i.e. OperationNode or OperandNode.
         * @returns Pointer to sub-class.
         */
        template<typename T>
        const T* get_as() const {
            if ((this->is_operation() && std::is_same_v<T, BooleanFunction::OperationNode>)
                || (this->is_operand() && std::is_same_v<T, BooleanFunction::OperandNode>)) {
                return static_cast<const T*>(this);
            }
            return nullptr;
        }

        /// Clones an node instance.
        virtual std::unique_ptr<Node> clone() const = 0;
        /// Human-readable description of node for debugging / logging
        virtual std::string to_string() const = 0;

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
        /// Checks whether node is of type 'Index'.
        bool is_index() const;
        /// Checks whether node is of type 'Variable'.
        bool is_variable() const;
        /// Checks whether node is an operation.
        bool is_operation() const;
        /// Checks whether node is an operand, i.e. 'Constant' or 'Variable'.
        bool is_operand() const;
    };

    /**
     * List of node types that are available in a Boolean function.
     *
     * @ingroup netlist
     */
    struct BooleanFunction::NodeType final {
        static constexpr u16 And      = 0x0000;
        static constexpr u16 Or       = 0x0001;
        static constexpr u16 Not      = 0x0002;
        static constexpr u16 Xor      = 0x0003;

        static constexpr u16 Add      = 0x0010;

        static constexpr u16 Concat   = 0x0100;
        static constexpr u16 Slice    = 0x0101;
        static constexpr u16 Zext     = 0x0102;

        static constexpr u16 Constant = 0x1000;
        static constexpr u16 Index    = 0x1001;
        static constexpr u16 Variable = 0x1002;
    };

    /**
     * OperationNode refers to an operation node.
     *
     * @ingroup netlist
     */
    struct BooleanFunction::OperationNode final : public BooleanFunction::Node {
        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /**
         * Creates an 'OperationNode'.
         *
         * @param[in] type - Node type.
         * @param[in] size - Node bit-size.
         * @returns An initialized base-class node.
         */
        static std::unique_ptr<BooleanFunction::Node> make(u16 type, u16 size);

        /// Comparison operators
        bool operator==(const OperationNode& other) const;
        bool operator!=(const OperationNode& other) const;
        bool operator <(const OperationNode& other) const;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

        /// Clones instance
        std::unique_ptr<Node> clone() const override;
        /// Human-readable description of AST node for debugging / logging
        std::string to_string() const override;

    private:
        /// constructor to initialize an operation node.
        OperationNode(u16 type, u16 size);
    };

    /**
     * OperandNode refers to an operand node, i.e. Constant, Index or Variable.
     *
     * @ingroup netlist
     */
    struct BooleanFunction::OperandNode final : public BooleanFunction::Node {
        ////////////////////////////////////////////////////////////////////////
        // Member
        ////////////////////////////////////////////////////////////////////////

        /// stores constant value
        const BooleanFunction::Value constant{};
        /// stores index value
        const u16 index{};
        /// stores variable name 
        const std::string variable{};
        
        ////////////////////////////////////////////////////////////////////////
        // Constructors, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /**
         * Creates a constant 'OperandNode'.
         *
         * @param[in] constant - Constant value.
         * @param[in] size - Node bit-size.
         * @returns An initialized base-class node.
         */
        static std::unique_ptr<BooleanFunction::Node> make(BooleanFunction::Value _constant, u16 _size);

        /**
         * Creates an index 'OperandNode'.
         *
         * @param[in] index - Index value.
         * @param[in] size - Node bit-size.
         * @returns An initialized base-class node.
         */
        static std::unique_ptr<BooleanFunction::Node> make(u16 _index, u16 _size);

        /**
         * Creates a variable 'OperandNode'.
         *
         * @param[in] name - Variable name.
         * @param[in] size - Node bit-size.
         * @returns An initialized base-class node.
         */
        static std::unique_ptr<BooleanFunction::Node> make(const std::string& _name, u16 _size);

        /// Comparison operators
        bool operator==(const OperandNode& other) const;
        bool operator!=(const OperandNode& other) const;
        bool operator <(const OperandNode& other) const;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

        /// Clones instance
        std::unique_ptr<Node> clone() const override;
        /// Human-readable description of AST node for debugging / logging
        std::string to_string() const override;

    private:
        /// Constructor to initialize an 'OperandNode'.
        OperandNode(u16 _type, u16 _size, BooleanFunction::Value _constant, u16 _index, const std::string& _variable);
    };
}    // namespace hal
