#include "hal_core/netlist/boolean_function.h"

#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "hal_core/netlist/boolean_function/parser.h"
#include "hal_core/netlist/boolean_function/symbolic_execution.h"

#include <algorithm>
#include <bitset>
#include <map>
#include <variant>

#include <boost/spirit/home/x3.hpp>

#define ERROR(MESSAGE) std::stringstream error; error << MESSAGE; return error.str(); 

namespace hal
{
    template<>
    std::vector<std::string> EnumStrings<BooleanFunction::Value>::data = {"0", "1", "Z", "X"};

    std::string BooleanFunction::to_string(Value v)
    {
        return enum_to_string<Value>(v);
    }

    std::ostream& operator<<(std::ostream& os, BooleanFunction::Value v)
    {
        return os << BooleanFunction::to_string(v);
    }

    BooleanFunction::BooleanFunction() {}

    std::variant<BooleanFunction, std::string> BooleanFunction::build(std::shared_ptr<Node>&& node, std::vector<BooleanFunction>&& parameters) {
        auto function = BooleanFunction(node->clone(), std::move(parameters));
        if (function.to_string() != "") {
            return function;
        }

        // TODO: call a generic validate function
        return "Cannot build a Boolean function for '" + node->to_string() + "'";
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::build(std::vector<std::shared_ptr<BooleanFunction::Node>>&& nodes) {
        auto function = BooleanFunction(std::move(nodes));
        if (function.to_string() != "") {
            return function;
        }
        // TODO: call a generic validate function
        return "Cannot build a boolean function ...";
    }

    BooleanFunction BooleanFunction::Var(const std::string& name, u16 size) {
        return BooleanFunction(OperandNode::make(name, size));
    }

    BooleanFunction BooleanFunction::Const(const BooleanFunction::Value& value) {
        return BooleanFunction(OperandNode::make({value}));
    }

    BooleanFunction BooleanFunction::Const(const std::vector<BooleanFunction::Value>& values) {
        return BooleanFunction(OperandNode::make(values));
    }

    BooleanFunction BooleanFunction::Const(u64 value, u16 size) {
        auto values = std::vector<BooleanFunction::Value>(); values.reserve(size);
        for (auto i = 0; i < size; i++) {
            values.emplace_back((value & (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
        }

        return BooleanFunction::Const(values);
    }

    BooleanFunction BooleanFunction::Index(u16 index, u16 size) {
        return BooleanFunction(OperationNode::make(index, size));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::And(BooleanFunction&& p0, BooleanFunction&& p1, u16 size) {
        if ((p0.size() != p1.size()) || (p0.size() != size)) {
            ERROR("Mismatching bit-sizes for AND operation (p0 = " << p0.size() << ", p1 = " << p1.size() << ", size = " << size << ").");
        }

        return BooleanFunction(OperationNode::make(NodeType::And, size), std::move(p0), std::move(p1));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Or(BooleanFunction&& p0, BooleanFunction&& p1, u16 size) {
        if ((p0.size() != p1.size()) || (p0.size() != size)) {
            ERROR("Mismatching bit-sizes for Or operation (p0 = " << p0.size() << ", p1 = " << p1.size() << ", size = " << size << ").");
        }

        return BooleanFunction(OperationNode::make(NodeType::Or, size), std::move(p0), std::move(p1));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Not(BooleanFunction&& p0, u16 size) {
        if (p0.size() != size) {
            ERROR("Mismatching bit-sizes for Not operation (p0 = " << p0.size() << ", size = " << size << ").");
        }

        return BooleanFunction(OperationNode::make(NodeType::Not, size), std::move(p0));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Xor(BooleanFunction&& p0, BooleanFunction&& p1, u16 size) {
        if ((p0.size() != p1.size()) || (p0.size() != size)) {
            ERROR("Mismatching bit-sizes for Xor operation (p0 = " << p0.size() << ", p1 = " << p1.size() << ", size = " << size << ").");
        }

       return BooleanFunction(OperationNode::make(NodeType::Xor, size), std::move(p0), std::move(p1));
    }

    std::ostream& operator<<(std::ostream& os, const BooleanFunction& function)
    {
        return os << function.to_string();
    }

    BooleanFunction BooleanFunction::operator&(const BooleanFunction& other) const
    {
        return std::get<BooleanFunction>(BooleanFunction::And(this->clone(), other.clone(), this->size()));
    }
    
    BooleanFunction& BooleanFunction::operator&=(const BooleanFunction& other)
    {
        *this = std::get<BooleanFunction>(BooleanFunction::And(this->clone(), other.clone(), this->size()));
        return *this;
    }

    BooleanFunction BooleanFunction::operator!() const
    {
        return this->operator~();
    }

    BooleanFunction BooleanFunction::operator~() const
    {
        return std::get<BooleanFunction>(BooleanFunction::Not(this->clone(), this->size()));
    }

    BooleanFunction BooleanFunction::operator|(const BooleanFunction& other) const
    {
        return std::get<BooleanFunction>(BooleanFunction::Or(this->clone(), other.clone(), this->size()));
    }

    BooleanFunction& BooleanFunction::operator|=(const BooleanFunction& other)
    {
        *this = std::get<BooleanFunction>(BooleanFunction::Or(this->clone(), other.clone(), this->size()));
        return *this;
    }

    BooleanFunction BooleanFunction::operator^(const BooleanFunction& other) const
    {
        return std::get<BooleanFunction>(BooleanFunction::Xor(this->clone(), other.clone(), this->size()));
    }

    BooleanFunction& BooleanFunction::operator^=(const BooleanFunction& other)
    {
        *this = std::get<BooleanFunction>(BooleanFunction::Xor(this->clone(), other.clone(), this->size()));
        return *this;
    }

    bool BooleanFunction::operator==(const BooleanFunction& other) const
    {
        if (this->m_nodes.size() != other.m_nodes.size()) {
            return false;
        }

        for (auto i = 0ul; i < this->m_nodes.size(); i++) {
            if (*this->m_nodes[i] != *other.m_nodes[i]) {
                return false;
            }
        }
        return true;
    }

    bool BooleanFunction::operator!=(const BooleanFunction& other) const
    {
        return !(*this == other);
    }

    bool BooleanFunction::operator<(const BooleanFunction& other) const
    {
        if (this->m_nodes.size() < other.m_nodes.size()) {
            return true;
        }
        if (this->m_nodes.size() > other.m_nodes.size()) {
            return false;
        }

        if (*this == other) {
            return false;
        }

        return this->to_string() < other.to_string();
    }
 
    bool BooleanFunction::is_empty() const
    {
        return this->m_nodes.empty();
    }

    BooleanFunction BooleanFunction::clone() const 
    {
        auto function = BooleanFunction(); function.m_nodes.reserve(this->m_nodes.size());

        for (const auto& node : this->m_nodes) {
            function.append(node->clone());
        }

        return function;
    }
    
    u16 BooleanFunction::size() const 
    {
        return this->m_nodes.back()->size;
    }

    bool BooleanFunction::is(u16 type) const 
    {
        return (this->is_empty()) ? false : this->get_top_level_node()->is(type);
    }

    bool BooleanFunction::is_variable() const 
    {
        return (this->is_empty()) ? false : this->get_top_level_node()->is_variable();
    }

    bool BooleanFunction::is_constant() const 
    {
        return (this->is_empty()) ? false : this->get_top_level_node()->is_constant();
    }

    bool BooleanFunction::is_constant(u64 value) const 
    {
        return (this->is_empty()) ? false : this->get_top_level_node()->is_constant(value);
    }

    const BooleanFunction::Node* BooleanFunction::get_top_level_node() const 
    {
        return (this->m_nodes.empty()) ? nullptr : this->m_nodes.back().get();
    }

    unsigned BooleanFunction::length() const
    {
        return this->m_nodes.size();
    }

    std::vector<const BooleanFunction::Node*> BooleanFunction::get_nodes() const 
    {
        auto nodes = std::vector<const BooleanFunction::Node*>(); nodes.reserve(this->m_nodes.size());
        for (const auto& node : this->m_nodes) {
            nodes.emplace_back(node.get());
        }
        return nodes;
    }

    std::vector<BooleanFunction> BooleanFunction::get_parameters() const {
        std::vector<BooleanFunction> stack;
        for (auto i = 0u; i < this->m_nodes.size() - 1; i++) {
            auto node = this->m_nodes[i];

            std::vector<BooleanFunction> operands;
            std::move(stack.end() - static_cast<u64>(node->get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<u64>(node->get_arity()), stack.end());
            stack.emplace_back(BooleanFunction(node->clone(), std::move(operands)));
        }
        return stack;
    }

    std::set<std::string> BooleanFunction::get_variable_names() const
    {
        auto variable_names = std::set<std::string>();
        for (const auto& node : this->m_nodes) {
            if (node->is_variable()) {
                variable_names.insert(node->get_as<BooleanFunction::OperandNode>()->variable);
            }
        }
        return variable_names;
    }

    std::string BooleanFunction::to_string() const
    {
        /// Local function to translate a given BooleanFunction::Node to a string
        ///
        /// @param[in] node Node of BooleanFunction.
        /// @param[in] p Node operands.
        /// @returns (0) status (true on success, false otherwise),
        ///          (1) human-readable string that represents the node
        auto node2string = [] (auto node, auto&& o) -> std::tuple<bool, std::string>
        {
            if (node->get_arity() != o.size()) {
                return {false, ""};
            }

            switch (node->type) {
                case BooleanFunction::NodeType::Constant:
                case BooleanFunction::NodeType::Index:
                case BooleanFunction::NodeType::Variable:
                    return {true, node->to_string()};


                case BooleanFunction::NodeType::And: return {true, "(" + o[0] + " & " + o[1] + ")"};
                case BooleanFunction::NodeType::Not: return {true, "(! " + o[0] + ")"};
                case BooleanFunction::NodeType::Or:  return {true, "(" + o[0] + " | " + o[1] + ")"};
                case BooleanFunction::NodeType::Xor: return {true, "(" + o[0] + " ^ " + o[1] + ")"};

                case BooleanFunction::NodeType::Add: return {true, "(" + o[0] + " + " + o[1] + ")"};

                case BooleanFunction::NodeType::Concat: return {true, "(" + o[0] + " ++ " + o[1] + ")"};
                case BooleanFunction::NodeType::Slice:  return {true, "Slice(" + o[0] + ", " + o[1] + ", " + o[2] + ")"};
                case BooleanFunction::NodeType::Zext:   return {true, "Zext(" + o[0] + ", " + o[1] + ")"};

                default: return {false, ""};
            }
        };

        // (1) early termination in case the Boolean function is empty
        if (this->m_nodes.empty()) {
            return "<empty>";
        }

        // (2) iterate the list of nodes and setup string from leafs to root
        std::vector<std::string> stack;
        for (const auto& node: this->m_nodes) {
            std::vector<std::string> operands;

            if (stack.size() < node->get_arity()) {
                // log_error("netlist", "Cannot fetch {} nodes from the stack (= imbalanced stack with {} parts - {}).", node->get_arity(), stack.size(), this->to_string_in_reverse_polish_notation());
                return "";
            }

            std::move(stack.end() - static_cast<u64>(node->get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<u64>(node->get_arity()), stack.end());

            if (auto [ok, reduction] = node2string(node.get(), std::move(operands)); ok) {
                stack.emplace_back(reduction);
            } else {
                // log_error("netlist", "Cannot translate BooleanFunction::Node '{}' to a string.", node->to_string());
                return "";
            }
        }

        switch (stack.size()) {
            case 1: return stack.back();
            default: {
                // log_error("netlist", "Cannot translate BooleanFunction (= imbalanced stack with {} remaining parts).", stack.size());
                return "";
            }
        }
    }

   std::variant<BooleanFunction, std::string> BooleanFunction::from(const std::string& expression) {
        using BooleanFunctionParser::ParserType;
        using BooleanFunctionParser::Token;

        static const std::vector<std::tuple<ParserType, std::function<std::variant<std::vector<Token>, std::string>(const std::string&)>>> parsers = {
            {ParserType::Liberty, BooleanFunctionParser::parse_with_liberty_grammar},
            {ParserType::Standard, BooleanFunctionParser::parse_with_standard_grammar},
        };

        for (const auto& [parser_type, parser]: parsers) {
            auto tokens = parser(expression);
            // (1) skip if parser cannot translate to tokens
            if (std::get_if<std::vector<Token>>(&tokens) == nullptr) {
                continue;
            }

            // (2) skip if cannot translate to valid reverse-polish notation
            tokens = BooleanFunctionParser::reverse_polish_notation(
                std::move(std::get<std::vector<Token>>(tokens)), 
                expression,
                parser_type
            );
            if (std::get_if<std::vector<Token>>(&tokens) == nullptr) {
                continue;
            }
            // (3) skip if reverse-polish notation tokens are no valid Boolean function
            auto function = BooleanFunctionParser::translate(
                std::move(std::get<std::vector<Token>>(tokens)), 
                expression
            );
            if (std::get_if<BooleanFunction>(&function) == nullptr) {
                continue;
            } 
            return std::get<BooleanFunction>(function);
        }
        return "No parser available to parser '" + expression + "'.";
    }

    BooleanFunction BooleanFunction::simplify() const 
    {
        auto symbolic_execution = SMT::SymbolicExecution();

        auto current = this->clone(),
             before = BooleanFunction();

        do {
            before = current.clone();
            auto simplified = symbolic_execution.evaluate(current); 
            if (std::get_if<std::string>(&simplified) != nullptr) {
                log_error("netlist", "{}", std::get<std::string>(simplified));
                return this->clone();
            }
            current = std::get<BooleanFunction>(simplified);
        } while (before != current);

        return current;
    }

    BooleanFunction BooleanFunction::substitute(const std::string& old_variable_name, const std::string& new_variable_name) const 
    {
        auto function = this->clone();
        for (auto i = 0u; i < this->m_nodes.size(); i++) {
            if (this->m_nodes[i]->is_variable(old_variable_name)) {
                function.m_nodes[i] = OperandNode::make(new_variable_name, this->m_nodes[i]->size);
            }
        }

        return function;
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::substitute(const std::string& name, const BooleanFunction& replacement) const 
    {
        /// Helper function to substitute a variable with a Boolean function.
        ///
        /// @param[in] node - Node.
	    /// @param[in] operands - Operands of node.
        /// @param[in] name - Variable name to check for replacement.
        /// @param[in] replacement - Replacement Boolean function.
        /// @returns AST replacement.
        auto substitute_variable = [] (auto node, auto&& operands, auto name, auto replacement) -> std::variant<BooleanFunction, std::string> {
            if (node->is_variable(name)) {
                return replacement.clone();
            }
            return BooleanFunction(std::move(node), std::move(operands));
        };

        std::vector<BooleanFunction> stack;
		for (const auto& node : this->m_nodes) {
			std::vector<BooleanFunction> operands;
			std::move(stack.end() - static_cast<i64>(node->get_arity()), stack.end(), std::back_inserter(operands));
        	stack.erase(stack.end() - static_cast<i64>(node->get_arity()), stack.end());
        	
            auto substituted = substitute_variable(node, std::move(operands), name, replacement);
            if (std::get_if<std::string>(&substituted) != nullptr) {
                return substituted;
            }
    		stack.emplace_back(std::get<BooleanFunction>(substituted));
		}

		switch (stack.size()) {
			case 1: return stack.back();
			default:   return "Cannot replace '" + name + "' with '" + replacement.to_string() + "' (= validation failed, so the operations may be imbalanced).";
		}
    }

    std::variant<BooleanFunction::Value, std::string> BooleanFunction::evaluate(const std::unordered_map<std::string, Value>& inputs) const {
        // (1) validate whether the input sizes match the boolean function
        if (this->size() != 1) {
            return "Cannot use the single-bit evaluate() on '" + this->to_string() + "' (= " + std::to_string(this->size()) + "-bit).";
        }

        // (2) translate the input to n-bit to use the generic function
        auto generic_inputs = std::unordered_map<std::string, std::vector<Value>>();
        for (const auto& [name, value]: inputs) {
            generic_inputs.emplace(name, std::vector<Value>({ value }));
        }

        auto value = this->evaluate(generic_inputs);
        if (std::get_if<std::vector<Value>>(&value) != nullptr) {
            return std::get<std::vector<Value>>(value)[0];
        }

        return std::get<std::string>(value);
    }

    std::variant<std::vector<BooleanFunction::Value>, std::string> BooleanFunction::evaluate(const std::unordered_map<std::string, std::vector<Value>>& inputs) const {
        // (1) validate whether the input sizes match the boolean function
        for (const auto& [name, value]: inputs) {
            for (const auto& node: this->m_nodes) {
                if (node->is_variable(name) && node->size != value.size()) {
                    return "Cannot use evaluate() on '" + this->to_string() + " as the '" + node->to_string() + " is " + std::to_string(node->size) + "-bit vs. " + std::to_string(value.size()) + "-bit in the input.";
                }
            }
        }

        // (2) initialize the symbolic state using the input variables
        auto symbolic_execution = SMT::SymbolicExecution();
        for (const auto& [name, value]: inputs) {
            symbolic_execution.state.set(BooleanFunction::Var(name, value.size()), BooleanFunction::Const(value));
        }

        // (3) analyze the evaluation result and check whether the result is a 
        //     constant boolean function
        auto result = symbolic_execution.evaluate(*this);
        if (std::get_if<BooleanFunction>(&result) != nullptr) {
            auto value = std::get<BooleanFunction>(result);
            if (value.is_constant()) {
                return value.get_top_level_node()->get_as<OperandNode>()->constant;
            }
            return "Cannot evaluate '" + this->to_string() + "' as the result is '" + value.to_string() + "'.";
        }
        return std::get<std::string>(result);
    }

    std::variant<std::vector<std::vector<BooleanFunction::Value>>, std::string> BooleanFunction::compute_truth_table(const std::vector<std::string>& ordered_variables, bool remove_unknown_variables) const {
        auto variable_names_in_function = this->get_variable_names();

        // (1) check that each variable is just a single bit, otherwise we do 
        //     not generate a truth-table
        for (const auto& node : this->m_nodes) {
            if (node->is_variable() && node->size != 1) {
                return "Cannot generate a truth-table for a Boolean function with variables of > 1-bit (e.g., = '" + node->to_string() + "').";
            }
        }

        // (2) select either parameter or the Boolean function variables
        auto variables = ordered_variables;
        if (variables.empty()) {
            variables = std::vector<std::string>(variable_names_in_function.begin(), variable_names_in_function.end());
        }

        // (3) remove any unknown variables from the truth table
        if (remove_unknown_variables) {
            variables.erase(std::remove_if(variables.begin(), variables.end(), 
                [&variable_names_in_function] (const auto& s) { return variable_names_in_function.find(s) == variable_names_in_function.end(); }),
                variables.end());
        }

        // (4.1) check that the function is not empty, otherwise we return a 
        //       Boolean function with a truth-table with 'X' values
        if (this->m_nodes.empty()) {
            return std::vector<std::vector<Value>>(1, std::vector<Value>(1 << variables.size(), Value::X));
        }

        // (4.2) safety-check in case the number of variables is too large to process
        if (variables.size() > 10) {
            return "Cannot generate a truth-table with > 10 variables (increase threshold or simplify expression beforehand).";
        }

        std::vector<std::vector<Value>> truth_table(
            this->size(),
            std::vector<Value>(1 << variables.size(), Value::ZERO)
        );

        // (5) iterate the truth-table rows and set each column accordingly
        for (auto value = 0u; value < ((u32)1 << variables.size()); value++) {
            std::unordered_map<std::string, std::vector<Value>> input;
            auto tmp = value;
            for (const auto& variable: variables) {
                input[variable] = ((tmp & 1) == 0) ? std::vector<Value>({Value::ZERO}) : std::vector<Value>({Value::ONE});
                tmp >>= 1;
            }
            auto result = this->evaluate(input);
            if (std::get_if<std::string>(&result) != nullptr) {
                return std::get<std::string>(result);
            }
            auto output = std::get<std::vector<Value>>(result);
            for (auto index = 0u; index < truth_table.size(); index++) {
                truth_table[index][value] = output[index];
            }
        }
        
        return truth_table;
    }

    z3::expr BooleanFunction::to_z3(z3::context& context, const std::map<std::string, z3::expr>& var2expr) const
    {
        /// Helper function to reduce a abstract syntax subtree to z3 expressions
        ///
        /// @param[in] node - Boolean function node.
        /// @param[in] p - Boolean function node parameters.
        /// @returns (1) status (true on success, false otherwise),
        ///          (2) SMT-LIB string representation of node and operands.
        auto reduce_to_z3 = [&context, &var2expr] (const auto& node, auto&& p) -> std::tuple<bool, z3::expr> {
            if (node->get_arity() != p.size()) {
                return {false, z3::expr(context)};
            }

            auto operand = node->template get_as<OperandNode>();

            switch (node->type) {
                case BooleanFunction::NodeType::Index: return {true, context.bv_val(operand->index, node->size)};
                case BooleanFunction::NodeType::Constant: {
                    // since our constants are defined as arbitrary bit-vectors,
                    // we have to concat each bit just to be on the safe side
                    auto constant = z3::expr(context);
                    for (const auto& bit : operand->constant) {
                        constant = z3::concat(context.bv_val(bit, 1), constant);
                    }
                    return {false, z3::expr(context)};
                }
                case BooleanFunction::NodeType::Variable: {
                    if (auto it = var2expr.find(operand->variable); it != var2expr.end()) {
                        return {true, it->second};
                    }
                    return {true, context.bv_const(operand->variable.c_str(), node->size)};
                }

                case BooleanFunction::NodeType::And: return {true, p[0] & p[1]};
                case BooleanFunction::NodeType::Or:  return {true, p[0] | p[1]};
                case BooleanFunction::NodeType::Not: return {true, ~p[0]};
                case BooleanFunction::NodeType::Xor: return {true, p[0] ^ p[1]};

                default: return {false, z3::expr(context)};
            }
        };

        std::vector<z3::expr> stack;
        for (const auto& node : this->m_nodes) {
            std::vector<z3::expr> operands;
            std::move(stack.end() - static_cast<i64>(node->get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<i64>(node->get_arity()), stack.end());
            
            if (auto [ok, reduction] = reduce_to_z3(node, std::move(operands)); ok) {
                stack.emplace_back(reduction);
            } else {
                return z3::expr(context);
            }
        }

        switch (stack.size()) {
            case 1:  return stack.back();
            default: return z3::expr(context);
        }
    }

    BooleanFunction::BooleanFunction(std::vector<std::shared_ptr<BooleanFunction::Node>>&& nodes) : BooleanFunction() 
    {
        m_nodes = std::move(nodes);
    }

    BooleanFunction::BooleanFunction(std::shared_ptr<BooleanFunction::Node>&& node, std::vector<BooleanFunction>&& p) : BooleanFunction() 
    {
        for (auto&& parameter : p) {
            this->append(std::move(parameter.m_nodes));
        }
        this->append(std::move(node));
    }

    void BooleanFunction::append(std::shared_ptr<BooleanFunction::Node>&& node) {
        this->m_nodes.emplace_back(node);
    }

    void BooleanFunction::append(std::vector<std::shared_ptr<BooleanFunction::Node>>&& nodes) {
        this->m_nodes.insert(m_nodes.end(), std::make_move_iterator(nodes.begin()), std::make_move_iterator(nodes.end()));
    }

    std::string BooleanFunction::to_string_in_reverse_polish_notation() const {
        std::string s;
        for (const auto& node : this->m_nodes) {
            s += node->to_string() + " ";
        }
        return s;
    }
/*
    std::vector<std::vector<BooleanFunction::Value>> BooleanFunction::qmc(std::vector<std::vector<Value>> terms)
    {
        if (terms.empty())
        {
            return {};
        }

        u32 num_variables = terms[0].size();

        // repeatedly merge all groups that only differ in a single value
        while (true)
        {
            bool any_changes = false;
            std::vector<std::vector<Value>> new_merged_terms;
            std::vector<bool> removed_by_merge(terms.size(), false);
            // pairwise compare all terms...
            for (u32 i = 0; i < terms.size(); ++i)
            {
                for (u32 j = i + 1; j < terms.size(); ++j)
                {
                    // ...merge their values and count differences...
                    std::vector<Value> merged(num_variables, Value::X);
                    u32 cnt = 0;
                    for (u32 k = 0; k < num_variables; ++k)
                    {
                        if (terms[i][k] == terms[j][k])
                        {
                            merged[k] = terms[i][k];
                        }
                        else
                        {
                            ++cnt;
                        }
                    }
                    // ...and if they differ only in a single value, replace them with the merged term
                    if (cnt == 1)
                    {
                        removed_by_merge[i] = removed_by_merge[j] = true;
                        new_merged_terms.push_back(merged);
                        any_changes = true;
                    }
                }
            }
            if (!any_changes)
            {
                break;
            }
            for (u32 i = 0; i < terms.size(); ++i)
            {
                if (!removed_by_merge[i])
                {
                    new_merged_terms.push_back(terms[i]);
                }
            }
            std::sort(new_merged_terms.begin(), new_merged_terms.end());
            new_merged_terms.erase(std::unique(new_merged_terms.begin(), new_merged_terms.end()), new_merged_terms.end());
            terms = new_merged_terms;
        }

        std::vector<std::vector<Value>> output;

        // build ON-set minterms to later identify essential implicants
        std::map<u32, std::vector<u32>> table;
        for (u32 i = 0; i < terms.size(); ++i)
        {
            // find all inputs that are covered by term i
            // Example: term=01-1 --> all inputs covered are 0101 and 0111
            std::vector<u32> covered_inputs;
            for (u32 j = 0; j < num_variables; ++j)
            {
                if (terms[i][j] != Value::X)
                {
                    if (covered_inputs.empty())
                    {
                        covered_inputs.push_back(terms[i][j]);
                    }
                    else
                    {
                        for (auto& v : covered_inputs)
                        {
                            v = (v << 1) | terms[i][j];
                        }
                    }
                }
                else
                {
                    if (covered_inputs.empty())
                    {
                        covered_inputs.push_back(0);
                        covered_inputs.push_back(1);
                    }
                    else
                    {
                        std::vector<u32> tmp;
                        for (auto v : covered_inputs)
                        {
                            tmp.push_back((v << 1) | 0);
                            tmp.push_back((v << 1) | 1);
                        }
                        covered_inputs = tmp;
                    }
                }
            }

            std::sort(covered_inputs.begin(), covered_inputs.end());
            covered_inputs.erase(std::unique(covered_inputs.begin(), covered_inputs.end()), covered_inputs.end());

            // now memorize that all these inputs are covered by term i
            for (auto v : covered_inputs)
            {
                table[v].push_back(i);
            }
        }

        // helper function to add a term to the output and remove it from the table
        auto add_to_output = [&](u32 term_index) {
            output.push_back(terms[term_index]);
            for (auto it2 = table.cbegin(); it2 != table.cend();)
            {
                if (std::find(it2->second.begin(), it2->second.end(), term_index) != it2->second.end())
                {
                    it2 = table.erase(it2);
                }
                else
                {
                    ++it2;
                }
            }
        };

        // finally, identify essential implicants and add them to the output
        while (!table.empty())
        {
            bool no_change = true;

            for (auto& it : table)
            {
                if (it.second.size() == 1)
                {
                    no_change = false;
                    add_to_output(it.second[0]);
                    break;    // 'add_to_output' invalidates table iterator, so break and restart in next iteration
                }
            }

            if (no_change)
            {
                // none of the remaining terms is essential, just pick the one that covers most input values
                std::unordered_map<u32, u32> counter;
                for (auto it : table)
                {
                    for (auto term_index : it.second)
                    {
                        counter[term_index]++;
                    }
                }

                u32 index = std::max_element(counter.begin(), counter.end(), [](const auto& p1, const auto& p2) { return p1.second < p2.second; })->first;
                add_to_output(index);
            }
        }

        // sort output terms for deterministic output order (no impact on functionality)
        std::sort(output.begin(), output.end());

        return output;
    }
    
   
*/

    BooleanFunction::Node::Node(u16 _type, u16 _size) : 
        type(_type), size(_size) {}

    bool BooleanFunction::Node::operator==(const Node& other) const
    {
        if (this->is_operation() && other.is_operation()) {
        return static_cast<const OperationNode&>(*this) == static_cast<const OperationNode&>(other);
        }
        if (this->is_operand() && other.is_operand()) {
            return static_cast<const OperandNode&>(*this) == static_cast<const OperandNode&>(other);
        }

        return false;
    }
    
    bool BooleanFunction::Node::operator!=(const Node& other) const
    {
        return !(*this == other);
    }

    bool BooleanFunction::Node::operator <(const Node& other) const
    {
        if (this->is_operation() && other.is_operation()) {
            return static_cast<const OperationNode&>(*this) < static_cast<const OperationNode&>(other);
        }
        if (this->is_operand() && other.is_operand()) {
            return static_cast<const OperandNode&>(*this) < static_cast<const OperandNode&>(other);
        }
        return std::tie(type, size) < std::tie(other.type, other.size);
    }

    u16 BooleanFunction::Node::get_arity() const {
        return BooleanFunction::Node::get_arity(this->type);
    }

    u16 BooleanFunction::Node::get_arity(u16 type) {
        static const std::map<u16, u16> type2arity = {
            {BooleanFunction::NodeType::And, 2},
            {BooleanFunction::NodeType::Or,  2},
            {BooleanFunction::NodeType::Not, 1},
            {BooleanFunction::NodeType::Xor, 2},

            {BooleanFunction::NodeType::Add, 2},

            {BooleanFunction::NodeType::Concat, 2},
            {BooleanFunction::NodeType::Slice, 3},
            {BooleanFunction::NodeType::Zext, 2},

            {BooleanFunction::NodeType::Constant, 0},
            {BooleanFunction::NodeType::Index, 0},
            {BooleanFunction::NodeType::Variable, 0},
        };

        return type2arity.at(type);
    }

    bool BooleanFunction::Node::is(u16 _type) const 
    {
        return this->type == _type;   
    }

    bool BooleanFunction::Node::is_constant() const 
    {
        return this->is(BooleanFunction::NodeType::Constant);
    }

    bool BooleanFunction::Node::is_constant(u64 value) const 
    {
        if (!this->is_constant()) {
            return false;
        }

        auto bv_value = std::vector<BooleanFunction::Value>({}); bv_value.reserve(this->size);
        for (auto i = 0u; i < this->get_as<OperandNode>()->constant.size(); i++) {
            bv_value.emplace_back((value << (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
        }
        return this->get_as<OperandNode>()->constant == bv_value;
    }

    bool BooleanFunction::Node::is_index() const 
    {
        return this->is(BooleanFunction::NodeType::Index);
    }

    bool BooleanFunction::Node::is_index(u16 index) const 
    {
        return this->is_index() && (this->get_as<OperandNode>()->index == index);
    }

    bool BooleanFunction::Node::is_variable() const 
    { 
        return this->is(BooleanFunction::NodeType::Variable); 
    }

    bool BooleanFunction::Node::is_variable(const std::string& variable) const 
    {
        return this->is_variable() && (this->get_as<OperandNode>()->variable == variable);
    }

    bool BooleanFunction::Node::is_operation() const 
    {
        return !this->is_operand(); 
    }
    
    bool BooleanFunction::Node::is_operand() const 
    {
        return this->is_constant() || this->is_variable() || this->is_index();
    }

    bool BooleanFunction::Node::is_commutative() const 
    {
        return (this->type == NodeType::And) || (this->type == NodeType::Or) || (this->type == NodeType::Xor)
            || (this->type == NodeType::Add);
    }

    std::shared_ptr<BooleanFunction::Node> BooleanFunction::OperationNode::make(u16 type, u16 size)
    {
        return std::shared_ptr<BooleanFunction::Node>(new OperationNode(type, size));
    }

    bool BooleanFunction::OperationNode::operator==(const BooleanFunction::OperationNode& other) const 
    {
        return std::tie(this->type, this->size) == std::tie(other.type, other.size);
    }

    bool BooleanFunction::OperationNode::operator!=(const BooleanFunction::OperationNode& other) const 
    {
        return !(*this == other);
    }

    bool BooleanFunction::OperationNode::operator <(const BooleanFunction::OperationNode& other) const 
    {
        return std::tie(this->type, this->size) < std::tie(other.type, other.size);
    }

    std::shared_ptr<BooleanFunction::Node> BooleanFunction::OperationNode::clone() const 
    {
        return std::shared_ptr<BooleanFunction::Node>(new OperationNode(*this));
    }

    std::string BooleanFunction::OperationNode::to_string() const 
    {
        static const std::map<u16, std::string>& operation2name = {
            {BooleanFunction::NodeType::And, "&"},
            {BooleanFunction::NodeType::Or, "|"},
            {BooleanFunction::NodeType::Not, "~"},
            {BooleanFunction::NodeType::Xor, "^"},

            {BooleanFunction::NodeType::Add, "+"},

            {BooleanFunction::NodeType::Concat, "++"},
            {BooleanFunction::NodeType::Slice, "Slice"},
            {BooleanFunction::NodeType::Zext, "Zext"},
        };

        return operation2name.at(this->type);
    }

    BooleanFunction::OperationNode::OperationNode(u16 _type, u16 _size) :
        BooleanFunction::Node(_type, _size) {}

    std::shared_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::make(const std::vector<BooleanFunction::Value>& _constant)
    {
        return std::shared_ptr<BooleanFunction::Node>(new OperandNode(NodeType::Constant, _constant.size(), _constant, 0, ""));
    }
 
    std::shared_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::make(u16 _index, u16 _size) 
    {
        return std::shared_ptr<BooleanFunction::Node>(new OperandNode(NodeType::Index, _size, {}, _index, ""));
    }

    std::shared_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::make(const std::string& _name, u16 _size) 
    {
        return std::shared_ptr<BooleanFunction::Node>(new OperandNode(NodeType::Variable, _size, {}, 0, _name));
    }

    bool BooleanFunction::OperandNode::operator==(const BooleanFunction::OperandNode& other) const 
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) == std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    bool BooleanFunction::OperandNode::operator!=(const BooleanFunction::OperandNode& other) const 
    {
        return !(*this == other);
    }

    bool BooleanFunction::OperandNode::operator <(const BooleanFunction::OperandNode& other) const 
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) < std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    std::shared_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::clone() const 
    {
        return std::shared_ptr<BooleanFunction::Node>(new OperandNode(*this));
    }

    std::string BooleanFunction::OperandNode::to_string() const 
    {
        static const std::map<u16, std::function<std::string(const OperandNode*)>> operand2name = 
        {
            {BooleanFunction::NodeType::Constant, [] (const OperandNode* node) { 
                std::string str;
                for (const auto& value : node->constant) {
                    str = enum_to_string(value) + str;
                }
                return "0b" + str;
            }},
            {BooleanFunction::NodeType::Index, [] (const OperandNode* node) { return std::to_string(node->index); }},
            {BooleanFunction::NodeType::Variable, [] (const OperandNode* node) { return node->variable; }},
        };

        return operand2name.at(this->type)(this);
    }

    BooleanFunction::OperandNode::OperandNode(u16 _type, u16 _size, std::vector<BooleanFunction::Value> _constant, u16 _index, const std::string& _variable) :
        BooleanFunction::Node(_type, _size), constant(_constant), index(_index), variable(_variable) {}
}    // namespace hal

