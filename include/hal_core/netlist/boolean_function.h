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
     * (or even a complex subcircuit consisting of multiple gates) in a formal 
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

        struct Node;     /// represents an abstract syntax tree node
        struct NodeType; /// represents the type of the node

        struct OperationNode; /// represents an operation node (e.g., AND)
        struct OperandNode;   /// reprsents an operand node (e.g., a variable)

        /// Represents the logic value that a boolean function operates on.
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

        ////////////////////////////////////////////////////////////////////////
        // Constructors / Factories, Destructors, Operators
        ////////////////////////////////////////////////////////////////////////

        /// Construct an empty / invalid Boolean function.
        explicit BooleanFunction();

        /**
         * Builds a Boolean function from a node and list of operands.
         * 
         * @param[in] node Boolean function node.
         * @param[in] parameters Boolean function node parameters.
         * @returns Boolean function on success, error message otherwise.
         */
        static std::variant<BooleanFunction, std::string> build(std::shared_ptr<Node>&& node, std::vector<BooleanFunction>&& parameters);

        /**
         * Builds a Boolean function from a list of nodes.
         * 
         * @param[in] nodes List of Boolean function nodes.
         * @returns Boolean function on success, Err() otherwise.
         */
        static std::variant<BooleanFunction, std::string> build(std::vector<std::shared_ptr<Node>>&& nodes);

        /// Creates a 'Variable' Boolean function.
        static BooleanFunction Var(const std::string& name, u16 size = 1);

        /// Creates a 'Constant' Boolean function.
        static BooleanFunction Const(const BooleanFunction::Value& value);
        /// Creates a 'Constant' Boolean function.
        static BooleanFunction Const(const std::vector<BooleanFunction::Value>& value);
        /// Creates a 'Constant' Boolean function.
        static BooleanFunction Const(u64 value, u16 size);

        /// Creates an 'Index' Boolean function.
        static BooleanFunction Index(u16 index, u16 size);

        /**
         * Creates an 'AND' Boolean function.
         * 
         * @param[in] p0 - Parameter 0 of bit-size 'size'.
         * @param[in] p1 - Parameter 1 of bit-size 'size'.
         * @param[in] size - Bit-size of AND operation.
         * @returns Boolean function on success, error message string otherwise.
         */
        static std::variant<BooleanFunction, std::string> And(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Creates an 'OR' Boolean function.
         * 
         * @param[in] p0 - Parameter 0 of bit-size 'size'.
         * @param[in] p1 - Parameter 1 of bit-size 'size'.
         * @param[in] size - Bit-size of Or operation.
         * @returns Boolean function on success, error message string otherwise.
         */
        static std::variant<BooleanFunction, std::string> Or(BooleanFunction&& p0, BooleanFunction&& p1, u16 size);

        /**
         * Creates an 'Not' Boolean function.
         * 
         * @param[in] p0 - Parameter 0 of bit-size 'size'.
         * @param[in] size - Bit-size of Not operation.
         * @returns  Boolean function on success, error message string otherwise.
         */
        static std::variant<BooleanFunction, std::string> Not(BooleanFunction&& p0, u16 size);

        /**
         * Creates an 'Xor' Boolean function.
         * 
         * @param[in] p0 - Parameter 0 of bit-size 'size'.
         * @param[in] p1 - Parameter 1 of bit-size 'size'.
         * @param[in] size - Bit-size of Xor operation.
         * @returns Boolean function on success, error message string otherwise.
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

        /// Short-hand Boolean function AND operation (may fail in case bit-sizes are not equal).
        BooleanFunction operator&(const BooleanFunction& other) const;
        /// Short-hand Boolean function AND operation assignment (may fail in case bit-sizes are not equal).
        BooleanFunction& operator&=(const BooleanFunction& other); 

        /**
         * \deprecated
         * DEPRECATED <br>
         * Negate the Boolean function.
         *
         * @returns The negated Boolean function.
         */
        [[deprecated("Use operator~() instead.")]] BooleanFunction operator!() const;

        /// Short-hand Boolean function NOT operation.
        BooleanFunction operator~() const;

        /// Short-hand Boolean function OR operation (may fail in case bit-sizes are not equal).
        BooleanFunction operator|(const BooleanFunction& other) const;
        /// Short-hand Boolean function OR operation assignment (may fail in case bit-sizes are not equal).
        BooleanFunction& operator|=(const BooleanFunction& other); 

        /// Short-hand Boolean function XOR operation (may fail in case bit-sizes are not equal).
        BooleanFunction operator^(const BooleanFunction& other) const;
        /// Short-hand Boolean function XOR operation assignment (may fail in case bit-sizes are not equal).
        BooleanFunction& operator^=(const BooleanFunction& other); 

        /// Short-hand equality operator to compare Boolean functions.
        bool operator==(const BooleanFunction& other) const;
        /// Short-hand in-equality operator to compare Boolean functions.
        bool operator!=(const BooleanFunction& other) const;
        /// Short-hand less-than operator to compare Boolean functions.
        bool operator <(const BooleanFunction& other) const;

        /// Short-hand check to test whether the instance is empty.
        bool is_empty() const;

        ////////////////////////////////////////////////////////////////////////
        // Interface: Nodes, Sizes, and Checks
        ////////////////////////////////////////////////////////////////////////

        /// Short-hand function to clone the instance.
        BooleanFunction clone() const;

        /// Short-hand function to get the bit-size of the instance.
        u16 size() const;

        /// Short-hand check whether the top-level node is a specific type.
        bool is(u16 type) const;

        /// Short-hand check whether the Boolean function is variable.
        bool is_variable() const;

        /// Short-hand check whether the Boolean function is a constant.
        bool is_constant() const;
        /// Short-hand check whether the Boolean function is a constant with a specific value.
        bool is_constant(u64 value) const;

        /// Short-hand function to query the top-level Boolean function node.
        const BooleanFunction::Node* get_top_level_node() const;

        /// Short-hand functoin to get the number of nodes in the Boolean function.
        unsigned length() const;

        /**
         * Returns the reverse-polish notation list of Boolean function nodes.
         *
         * @returns List of non-owning pointers to nodes.
         */
        std::vector<const BooleanFunction::Node*> get_nodes() const;

        /**
         * Returns the parameter list of the top-level node.
         * 
         * @returns List of parameters.
         */
        std::vector<BooleanFunction> get_parameters() const;

        /**
         *  Returns the set of variable names utilizes in the Boolean function.
         *
         * @returns Set of variable names.
         */
        std::set<std::string> get_variable_names() const;

        ////////////////////////////////////////////////////////////////////////
        // Interface: String Translation 
        ////////////////////////////////////////////////////////////////////////

        /// Transforms the BooleanFunction into a human-readable string.
        std::string to_string() const;

        /**
         * Parses a boolean function from a string representation.
         * 
         * @param[in] expression - Boolean function string.
         * @returns The Boolean function or a string with the error message.
         */
        static std::variant<BooleanFunction, std::string> from(const std::string& expression);

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
        explicit BooleanFunction(std::vector<std::shared_ptr<BooleanFunction::Node>>&& nodes);

        /** 
         * Constructs a Boolean function from a single node and an arbitrary list
         * of Boolean function parameters.
         * 
         * @param[in] node - Boolean function node.
         * @param[in] p - Boolean function node parameters.
         * @returns Initialized Boolean function.
         */
        template<typename ...T, typename = std::enable_if_t<std::conjunction_v<std::is_same<T, BooleanFunction>...>>>
        explicit BooleanFunction(std::shared_ptr<BooleanFunction::Node>&& node, T&&... p) {
            (this->append(std::move(p.m_nodes)), ...);
            this->append(std::move(node));
        }

        /** 
         * Constructs a Boolean function from a single node and a list of parameters.
         * 
         * @param[in] node - Boolean function node.
         * @param[in] p - Boolean function node parameters.
         * @returns Initialized Boolean function.
         */
        explicit BooleanFunction(std::shared_ptr<BooleanFunction::Node>&& node, std::vector<BooleanFunction>&& p);

        ////////////////////////////////////////////////////////////////////////
        // Internal Interface
        ////////////////////////////////////////////////////////////////////////

        /// Appends a Boolean function node to the instance.
        void append(std::shared_ptr<BooleanFunction::Node>&& node);
        
        /// Appends a list of Boolan function nodes to the instance.
        void append(std::vector<std::shared_ptr<BooleanFunction::Node>>&& nodes);

        /// Returns the Boolean function in reverse-polish notation.
        std::string to_string_in_reverse_polish_notation() const;

        // static std::vector<std::vector<Value>> qmc(std::vector<std::vector<Value>> terms);

        ////////////////////////////////////////////////////////////////////////
        // Member
        ////////////////////////////////////////////////////////////////////////

        /// refers to the list of nodes in reverse polish notation
        ///
        /// # TODO
        /// We want to change the shared_ptr into a unique_ptr to prevent any 
        /// accidental copy, however, this has implications for various HAL 
        /// components as they need to have explicit move / clone semantics. 
        /// Since this rework affects various components, we opted to postpone 
        /// this change for later and stick with a shared_ptr for now.
        std::vector<std::shared_ptr<BooleanFunction::Node>> m_nodes{};
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
        virtual std::shared_ptr<Node> clone() const = 0;
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
        /// Checks whether node is of type 'Constant' and has specific value.
        bool is_constant(u64 value) const;

        /// Checks whether node is of type 'Index'.
        bool is_index() const;
        /// Checks whether node is of type 'Index' and has specifif value.
        bool is_index(u16 value) const;

        /// Checks whether node is of type 'Variable'.
        bool is_variable() const;
        /// Checks whether node is of type 'Variable' and has specific value.
        bool is_variable(const std::string& variable) const;

        /// Checks whether node is an operation.
        bool is_operation() const;
        /// Checks whether node is an operand, i.e. 'Constant' or 'Variable'.
        bool is_operand() const;

        /// Short-hand check whether the node is a commutative operator.
        bool is_commutative() const;
    };

    /**
     * List of available node types in a Boolean function.
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
        static std::shared_ptr<BooleanFunction::Node> make(u16 type, u16 size);

        /// Comparison operators
        bool operator==(const OperationNode& other) const;
        bool operator!=(const OperationNode& other) const;
        bool operator <(const OperationNode& other) const;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

        /// Clones instance
        std::shared_ptr<Node> clone() const override;
        /// Human-readable description of node for debugging / logging
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
        const std::vector<BooleanFunction::Value> constant{};
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
         * @returns An initialized base-class node.
         */
        static std::shared_ptr<BooleanFunction::Node> make(const std::vector<BooleanFunction::Value>& _constant);

        /**
         * Creates an index 'OperandNode'.
         *
         * @param[in] index - Index value.
         * @param[in] size - Node bit-size.
         * @returns An initialized base-class node.
         */
        static std::shared_ptr<BooleanFunction::Node> make(u16 _index, u16 _size);

        /**
         * Creates a variable 'OperandNode'.
         *
         * @param[in] name - Variable name.
         * @param[in] size - Node bit-size.
         * @returns An initialized base-class node.
         */
        static std::shared_ptr<BooleanFunction::Node> make(const std::string& _name, u16 _size);

        /// Comparison operators
        bool operator==(const OperandNode& other) const;
        bool operator!=(const OperandNode& other) const;
        bool operator <(const OperandNode& other) const;

        ////////////////////////////////////////////////////////////////////////
        // Interface
        ////////////////////////////////////////////////////////////////////////

        /// Clones instance
        std::shared_ptr<Node> clone() const override;
        /// Human-readable description of node for debugging / logging
        std::string to_string() const override;

    private:
        /// Constructor to initialize an 'OperandNode'.
        OperandNode(u16 _type, u16 _size, std::vector<BooleanFunction::Value> _constant, u16 _index, const std::string& _variable);
    };
}    // namespace hal
