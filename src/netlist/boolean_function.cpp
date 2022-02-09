#include "hal_core/netlist/boolean_function.h"

#include "hal_core/netlist/boolean_function/parser.h"
#include "hal_core/netlist/boolean_function/simplification.h"
#include "hal_core/netlist/boolean_function/symbolic_execution.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <algorithm>
#include <bitset>
#include <boost/spirit/home/x3.hpp>
#include <chrono>
#include <map>
#include <variant>

#define ERROR(MESSAGE)       \
    std::stringstream error; \
    error << MESSAGE;        \
    return error.str();

namespace hal
{
    template<>
    std::map<BooleanFunction::Value, std::string> EnumStrings<BooleanFunction::Value>::data = {{BooleanFunction::Value::ZERO, "0"},
                                                                                               {BooleanFunction::Value::ONE, "1"},
                                                                                               {BooleanFunction::Value::X, "X"},
                                                                                               {BooleanFunction::Value::Z, "Z"}};

    std::string BooleanFunction::to_string(Value v)
    {
        switch (v)
        {
            case ZERO:
                return std::string("0");
            case ONE:
                return std::string("1");
            case X:
                return std::string("X");
            case Z:
                return std::string("Z");
        }

        return std::string("X");
    }

    namespace
    {
        static std::vector<char> char_map = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        // namespace

        static Result<std::string> to_bin(const std::vector<BooleanFunction::Value>& value)
        {
            if (value.size() == 0)
            {
                return ERR("bit-vector is empty");
            }

            std::string res = "";
            res.reserve(value.size());

            for (auto v : value)
            {
                res += enum_to_string<BooleanFunction::Value>(v);
            }

            return OK(res);
        }

        static Result<std::string> to_oct(const std::vector<BooleanFunction::Value>& value)
        {
            int bitsize = value.size();
            if (bitsize == 0)
            {
                return ERR("bit-vector is empty");
            }

            u8 first_bits = bitsize % 3;

            u8 index = 0;
            u8 mask  = 0;

            u8 v1, v2, v3;

            // result string prep
            std::string res = "";
            res.reserve((bitsize + 2) / 3);

            // deal with 0-3 leading bits
            for (u8 i = 0; i < first_bits; i++)
            {
                v1    = value.at(i);
                index = (index << 1) | v1;
                mask |= v1;
            }
            mask = -((mask >> 1) & 0x1);
            if (first_bits)
            {
                res += (char_map[index] & ~mask) | ('X' & mask);
            }

            // deal with 4-bit blocks (left to right)
            for (int i = bitsize % 3; i < bitsize; i += 3)
            {
                v1 = value[i];
                v2 = value[i + 1];
                v3 = value[i + 2];

                index = (v1 << 2) | (v2 << 1) | v3;    // cannot exceed char_map range as index always < 16, no further check required
                mask  = -(((v1 | v2 | v3) >> 1) & 0x1);

                res += (char_map[index] & ~mask) | ('X' & mask);
            }
            return OK(res);
        }

        static Result<std::string> to_dec(const std::vector<BooleanFunction::Value>& value)
        {
            int bitsize = value.size();
            if (bitsize == 0)
            {
                return ERR("bit-vector is empty");
            }

            if (bitsize > 64)
            {
                return ERR("bit-vector has length " + std::to_string(bitsize) + ", but only up to 64 bits are supported for decimal conversion");
            }

            u64 tmp   = 0;
            u8 x_flag = 0;
            for (auto v : value)
            {
                x_flag |= v >> 1;
                tmp = (tmp << 1) | v;
            }

            if (x_flag)
            {
                return OK(std::string("X"));
            }
            return OK(std::to_string(tmp));
        }

        static Result<std::string> to_hex(const std::vector<BooleanFunction::Value>& value)
        {
            int bitsize = value.size();
            if (bitsize == 0)
            {
                return ERR("bit-vector is empty");
            }

            u8 first_bits = bitsize & 0x3;

            u8 index = 0;
            u8 mask  = 0;

            u8 v1, v2, v3, v4;

            // result string prep
            std::string res = "";
            res.reserve((bitsize + 3) / 4);

            // deal with 0-3 leading bits
            for (u8 i = 0; i < first_bits; i++)
            {
                v1    = value.at(i);
                index = (index << 1) | v1;
                mask |= v1;
            }
            mask = -((mask >> 1) & 0x1);
            if (first_bits)
            {
                res += (char_map[index] & ~mask) | ('X' & mask);
            }

            // deal with 4-bit blocks (left to right)
            for (int i = bitsize & 0x3; i < bitsize; i += 4)
            {
                v1 = value[i];
                v2 = value[i + 1];
                v3 = value[i + 2];
                v4 = value[i + 3];

                index = ((v1 << 3) | (v2 << 2) | (v3 << 1) | v4) & 0xF;
                mask  = -(((v1 | v2 | v3 | v4) >> 1) & 0x1);

                res += (char_map[index] & ~mask) | ('X' & mask);
            }

            return OK(res);
        }
    }    // namespace

    Result<std::string> BooleanFunction::to_string(const std::vector<BooleanFunction::Value>& value, u8 base)
    {
        switch (base)
        {
            case 2:
                return to_bin(value);
            case 8:
                return to_oct(value);
            case 10:
                return to_dec(value);
            case 16:
                return to_hex(value);
            default:
                return ERR("invalid value '" + std::to_string(base) + "' provided for base");
        }
    }

    std::ostream& operator<<(std::ostream& os, BooleanFunction::Value v)
    {
        return os << BooleanFunction::to_string(v);
    }

    BooleanFunction::BooleanFunction()
    {
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::build(std::vector<BooleanFunction::Node>&& nodes)
    {
        return BooleanFunction::validate(BooleanFunction(std::move(nodes)));
    }

    BooleanFunction BooleanFunction::Var(const std::string& name, u16 size)
    {
        return BooleanFunction(Node::Variable(name, size));
    }

    BooleanFunction BooleanFunction::Const(const BooleanFunction::Value& value)
    {
        return BooleanFunction(Node::Constant({value}));
    }

    BooleanFunction BooleanFunction::Const(const std::vector<BooleanFunction::Value>& values)
    {
        return BooleanFunction(Node::Constant(values));
    }

    BooleanFunction BooleanFunction::Const(u64 value, u16 size)
    {
        auto values = std::vector<BooleanFunction::Value>();
        values.reserve(size);
        for (auto i = 0; i < size; i++)
        {
            values.emplace_back((value & (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
        }

        return BooleanFunction::Const(values);
    }

    BooleanFunction BooleanFunction::Index(u16 index, u16 size)
    {
        return BooleanFunction(Node::Index(index, size));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::And(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            ERROR("Mismatching bit-sizes for AND operation (p0 = " << p0.size() << ", p1 = " << p1.size() << ", size = " << size << ").");
        }

        return BooleanFunction(Node::Operation(NodeType::And, size), std::move(p0), std::move(p1));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Or(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            ERROR("Mismatching bit-sizes for Or operation (p0 = " << p0.size() << ", p1 = " << p1.size() << ", size = " << size << ").");
        }

        return BooleanFunction(Node::Operation(NodeType::Or, size), std::move(p0), std::move(p1));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Not(BooleanFunction&& p0, u16 size)
    {
        if (p0.size() != size)
        {
            ERROR("Mismatching bit-sizes for Not operation (p0 = " << p0.size() << ", size = " << size << ").");
        }

        return BooleanFunction(Node::Operation(NodeType::Not, size), std::move(p0));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Xor(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            ERROR("Mismatching bit-sizes for Xor operation (p0 = " << p0.size() << ", p1 = " << p1.size() << ", size = " << size << ").");
        }

        return BooleanFunction(Node::Operation(NodeType::Xor, size), std::move(p0), std::move(p1));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Slice(BooleanFunction&& p0, BooleanFunction&& p1, BooleanFunction&& p2, u16 size)
    {
        if (!p1.is_index() || !p2.is_index())
        {
            ERROR("Mismatching function types for Slice operation (= p1 and p2 must be an 'BooleanFunctino::Index').");
        }
        if ((p0.size() != p1.size()) || (p1.size() != p2.size()))
        {
            ERROR("Mismatching bit-sizes for Slice operation (p0 = " << p0.size() << ", p1 = " << p1.size() << ", p2 = " << p2.size() << " - sizes must be equal).");
        }

        auto start = p1.get_top_level_node().index, end = p2.get_top_level_node().index;

        if ((start > end) || (start >= p0.size()) || (end >= p0.size()) || (end - start + 1) != size)
        {
            ERROR("Mismatching bit-sizes for Slice operation (p0 = " << p0.size() << ", p1 = " << start << ", p2 = " << end << ").");
        }

        return BooleanFunction(Node::Operation(NodeType::Slice, size), std::move(p0), std::move(p1), std::move(p2));
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::Concat(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() + p1.size()) != size)
        {
            ERROR("Mismatch function input width (p0 = " << p0.size() << "-bit, p1 = " << p1.size() << "-bit, size = " << size << ").");
        }

        return BooleanFunction(Node::Operation(NodeType::Concat, size), std::move(p0), std::move(p1));
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
        if (this->m_nodes.size() != other.m_nodes.size())
        {
            return false;
        }

        for (auto i = 0ul; i < this->m_nodes.size(); i++)
        {
            if (this->m_nodes[i] != other.m_nodes[i])
            {
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
        if (this->m_nodes.size() < other.m_nodes.size())
        {
            return true;
        }
        if (this->m_nodes.size() > other.m_nodes.size())
        {
            return false;
        }

        return this->to_string_in_reverse_polish_notation() < other.to_string_in_reverse_polish_notation();
    }

    bool BooleanFunction::is_empty() const
    {
        return this->m_nodes.empty();
    }

    BooleanFunction BooleanFunction::clone() const
    {
        auto function = BooleanFunction();
        function.m_nodes.reserve(this->m_nodes.size());

        for (const auto& node : this->m_nodes)
        {
            function.m_nodes.emplace_back(node);
        }

        return function;
    }

    u16 BooleanFunction::size() const
    {
        return this->m_nodes.back().size;
    }

    bool BooleanFunction::is(u16 type) const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is(type);
    }

    bool BooleanFunction::is_variable() const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is_variable();
    }

    bool BooleanFunction::is_constant() const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is_constant();
    }

    bool BooleanFunction::has_constant_value(u64 value) const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().has_constant_value(value);
    }

    bool BooleanFunction::is_index() const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is_index();
    }

    bool BooleanFunction::has_index_value(u16 value) const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().has_index_value(value);
    }

    const BooleanFunction::Node& BooleanFunction::get_top_level_node() const
    {
        return this->m_nodes.back();
    }

    u32 BooleanFunction::length() const
    {
        return this->m_nodes.size();
    }

    const std::vector<BooleanFunction::Node>& BooleanFunction::get_nodes() const
    {
        return this->m_nodes;
    }

    std::vector<BooleanFunction> BooleanFunction::get_parameters() const
    {
        /// # Developer Note
        /// Instead of iterating the whole Boolean function and decomposing the
        /// abstract syntax tree, we simple iterate the Boolean function once
        /// and compute the coverage, i.e. how many nodes are covered below the
        /// node in the tree, and based on these indices assemble the Boolean
        /// function node vector.

        auto coverage = this->compute_node_coverage();
        switch (this->get_top_level_node().get_arity())
        {
            case 0: {
                return {};
            }
            case 1: {
                return {BooleanFunction(std::vector<Node>({this->m_nodes.begin(), this->m_nodes.end() - 1}))};
            }
            case 2: {
                auto index = this->length() - coverage[this->length() - 2] - 1;

                return {BooleanFunction(std::vector<Node>({this->m_nodes.begin(), this->m_nodes.begin() + index})),
                        BooleanFunction(std::vector<Node>({this->m_nodes.begin() + index, this->m_nodes.end() - 1}))};
            }

            default:
                assert(false && "not implemented reached.");
        }

        return {};
    }

    std::set<std::string> BooleanFunction::get_variable_names() const
    {
        auto variable_names = std::set<std::string>();
        for (const auto& node : this->m_nodes)
        {
            if (node.is_variable())
            {
                variable_names.insert(node.variable);
            }
        }
        return variable_names;
    }

    Result<std::string> BooleanFunction::default_printer(const BooleanFunction::Node& node, std::vector<std::string>&& operands)
    {
        if (node.get_arity() != operands.size())
        {
            return ERR("node arity of " + std::to_string(node.get_arity()) + " does not match number of operands of " + std::to_string(operands.size()));
        }

        switch (node.type)
        {
            case BooleanFunction::NodeType::Constant:
            case BooleanFunction::NodeType::Index:
            case BooleanFunction::NodeType::Variable:
                return OK(node.to_string());

            case BooleanFunction::NodeType::And:
                return OK("(" + operands[0] + " & " + operands[1] + ")");
            case BooleanFunction::NodeType::Not:
                return OK("(! " + operands[0] + ")");
            case BooleanFunction::NodeType::Or:
                return OK("(" + operands[0] + " | " + operands[1] + ")");
            case BooleanFunction::NodeType::Xor:
                return OK("(" + operands[0] + " ^ " + operands[1] + ")");

            case BooleanFunction::NodeType::Add:
                return OK("(" + operands[0] + " + " + operands[1] + ")");

            case BooleanFunction::NodeType::Concat:
                return OK("(" + operands[0] + " ++ " + operands[1] + ")");
            case BooleanFunction::NodeType::Slice:
                return OK("Slice(" + operands[0] + ", " + operands[1] + ", " + operands[2] + ")");
            case BooleanFunction::NodeType::Zext:
                return OK("Zext(" + operands[0] + ", " + operands[1] + ")");

            default:
                return ERR("unsupported node type '" + std::to_string(node.type) + "'");
        }
    }

    std::string BooleanFunction::to_string(std::function<Result<std::string>(const BooleanFunction::Node& node, std::vector<std::string>&& operands)>&& printer) const
    {
        // (1) early termination in case the Boolean function is empty
        if (this->m_nodes.empty())
        {
            return "<empty>";
        }

        // (2) iterate the list of nodes and setup string from leafs to root
        std::vector<std::string> stack;
        for (const auto& node : this->m_nodes)
        {
            std::vector<std::string> operands;

            if (stack.size() < node.get_arity())
            {
                // log_error("netlist", "Cannot fetch {} nodes from the stack (= imbalanced stack with {} parts - {}).", node->get_arity(), stack.size(), this->to_string_in_reverse_polish_notation());
                return "";
            }

            std::move(stack.end() - static_cast<u64>(node.get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<u64>(node.get_arity()), stack.end());

            if (auto res = printer(node, std::move(operands)); res.is_ok())
            {
                stack.emplace_back(res.get());
            }
            else
            {
                log_error("netlist", "Cannot translate BooleanFunction::Node '{}' to a string: {}.", node.to_string(), res.get_error().get());
                return "";
            }
        }

        switch (stack.size())
        {
            case 1:
                return stack.back();
            default: {
                // log_error("netlist", "Cannot translate BooleanFunction (= imbalanced stack with {} remaining parts).", stack.size());
                return "";
            }
        }
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::from_string(const std::string& expression)
    {
        using BooleanFunctionParser::ParserType;
        using BooleanFunctionParser::Token;

        static const std::vector<std::tuple<ParserType, std::function<std::variant<std::vector<Token>, std::string>(const std::string&)>>> parsers = {
            {ParserType::Standard, BooleanFunctionParser::parse_with_standard_grammar},
            {ParserType::Liberty, BooleanFunctionParser::parse_with_liberty_grammar},
        };

        for (const auto& [parser_type, parser] : parsers)
        {
            auto tokens = parser(expression);
            // (1) skip if parser cannot translate to tokens
            if (std::get_if<std::vector<Token>>(&tokens) == nullptr)
            {
                continue;
            }

            // (2) skip if cannot translate to valid reverse-polish notation
            tokens = BooleanFunctionParser::reverse_polish_notation(std::move(std::get<std::vector<Token>>(tokens)), expression, parser_type);
            if (std::get_if<std::vector<Token>>(&tokens) == nullptr)
            {
                continue;
            }
            // (3) skip if reverse-polish notation tokens are no valid Boolean function
            auto function = BooleanFunctionParser::translate(std::move(std::get<std::vector<Token>>(tokens)), expression);
            if (std::get_if<BooleanFunction>(&function) == nullptr)
            {
                continue;
            }
            return std::get<BooleanFunction>(function);
        }
        return "No parser available to parser '" + expression + "'.";
    }

    BooleanFunction BooleanFunction::simplify() const
    {
        auto simplified = Simplification::local_simplification(*this)
                              .map<BooleanFunction>([](const auto& simplified) { return Simplification::abc_simplification(simplified); })
                              .map<BooleanFunction>([](const auto& simplified) { return Simplification::local_simplification(simplified); });

        return (simplified.is_ok()) ? simplified.get() : this->clone();
    }

    BooleanFunction BooleanFunction::substitute(const std::string& old_variable_name, const std::string& new_variable_name) const
    {
        auto function = this->clone();
        for (auto i = 0u; i < this->m_nodes.size(); i++)
        {
            if (this->m_nodes[i].has_variable_name(old_variable_name))
            {
                function.m_nodes[i] = Node::Variable(new_variable_name, this->m_nodes[i].size);
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
        /// @param[in] var_name - Variable name to check for replacement.
        /// @param[in] repl - Replacement Boolean function.
        /// @returns AST replacement.
        auto substitute_variable = [](const auto& node, auto&& operands, auto var_name, auto repl) -> std::variant<BooleanFunction, std::string> {
            if (node.has_variable_name(var_name))
            {
                return repl.clone();
            }
            return BooleanFunction(node.clone(), std::move(operands));
        };

        std::vector<BooleanFunction> stack;
        for (const auto& node : this->m_nodes)
        {
            std::vector<BooleanFunction> operands;
            std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

            auto substituted = substitute_variable(node, std::move(operands), name, replacement);
            if (std::get_if<std::string>(&substituted) != nullptr)
            {
                return substituted;
            }
            stack.emplace_back(std::get<BooleanFunction>(substituted));
        }

        switch (stack.size())
        {
            case 1:
                return stack.back();
            default:
                return "Cannot replace '" + name + "' with '" + replacement.to_string() + "' (= validation failed, so the operations may be imbalanced).";
        }
    }

    std::variant<BooleanFunction::Value, std::string> BooleanFunction::evaluate(const std::unordered_map<std::string, Value>& inputs) const
    {
        // (0) workaround to preserve the API functionality
        if (this->m_nodes.empty())
        {
            return BooleanFunction::Value::X;
        }

        // (1) validate whether the input sizes match the boolean function
        if (this->size() != 1)
        {
            return "Cannot use the single-bit evaluate() on '" + this->to_string() + "' (= " + std::to_string(this->size()) + "-bit).";
        }

        // (2) translate the input to n-bit to use the generic function
        auto generic_inputs = std::unordered_map<std::string, std::vector<Value>>();
        for (const auto& [name, value] : inputs)
        {
            generic_inputs.emplace(name, std::vector<Value>({value}));
        }

        auto value = this->evaluate(generic_inputs);
        if (std::get_if<std::vector<Value>>(&value) != nullptr)
        {
            return std::get<std::vector<Value>>(value)[0];
        }

        return std::get<std::string>(value);
    }

    std::variant<std::vector<BooleanFunction::Value>, std::string> BooleanFunction::evaluate(const std::unordered_map<std::string, std::vector<Value>>& inputs) const
    {
        // (0) workaround to preserve the API functionality
        if (this->m_nodes.empty())
        {
            return std::vector<BooleanFunction::Value>({BooleanFunction::Value::X});
        }

        // (1) validate whether the input sizes match the boolean function
        for (const auto& [name, value] : inputs)
        {
            for (const auto& node : this->m_nodes)
            {
                if (node.has_variable_name(name) && node.size != value.size())
                {
                    return "Cannot use evaluate() on '" + this->to_string() + " as the '" + node.to_string() + " is " + std::to_string(node.size) + "-bit vs. " + std::to_string(value.size())
                           + "-bit in the input.";
                }
            }
        }

        // (2) initialize the symbolic state using the input variables
        auto symbolic_execution = SMT::SymbolicExecution();
        for (const auto& [name, value] : inputs)
        {
            symbolic_execution.state.set(BooleanFunction::Var(name, value.size()), BooleanFunction::Const(value));
        }

        // (3) analyze the evaluation result and check whether the result is a
        //     constant boolean function
        auto result = symbolic_execution.evaluate(*this);
        if (std::get_if<BooleanFunction>(&result) != nullptr)
        {
            auto value = std::get<BooleanFunction>(result);
            if (value.is_constant())
            {
                return value.get_top_level_node().constant;
            }
            else
            {
                return std::vector<BooleanFunction::Value>(this->size(), BooleanFunction::Value::X);
            }
        }
        return std::get<std::string>(result);
    }

    std::variant<std::vector<std::vector<BooleanFunction::Value>>, std::string> BooleanFunction::compute_truth_table(const std::vector<std::string>& ordered_variables,
                                                                                                                     bool remove_unknown_variables) const
    {
        auto variable_names_in_function = this->get_variable_names();

        // (1) check that each variable is just a single bit, otherwise we do
        //     not generate a truth-table
        for (const auto& node : this->m_nodes)
        {
            if (node.is_variable() && node.size != 1)
            {
                return "Cannot generate a truth-table for a Boolean function with variables of > 1-bit (e.g., = '" + node.to_string() + "').";
            }
        }

        // (2) select either parameter or the Boolean function variables
        auto variables = ordered_variables;
        if (variables.empty())
        {
            variables = std::vector<std::string>(variable_names_in_function.begin(), variable_names_in_function.end());
        }

        // (3) remove any unknown variables from the truth table
        if (remove_unknown_variables)
        {
            variables.erase(
                std::remove_if(variables.begin(), variables.end(), [&variable_names_in_function](const auto& s) { return variable_names_in_function.find(s) == variable_names_in_function.end(); }),
                variables.end());
        }

        // (4.1) check that the function is not empty, otherwise we return a
        //       Boolean function with a truth-table with 'X' values
        if (this->m_nodes.empty())
        {
            return std::vector<std::vector<Value>>(1, std::vector<Value>(1 << variables.size(), Value::X));
        }

        // (4.2) safety-check in case the number of variables is too large to process
        if (variables.size() > 10)
        {
            return "Cannot generate a truth-table with > 10 variables (increase threshold or simplify expression beforehand).";
        }

        std::vector<std::vector<Value>> truth_table(this->size(), std::vector<Value>(1 << variables.size(), Value::ZERO));

        // (5) iterate the truth-table rows and set each column accordingly
        for (auto value = 0u; value < ((u32)1 << variables.size()); value++)
        {
            std::unordered_map<std::string, std::vector<Value>> input;
            auto tmp = value;
            for (const auto& variable : variables)
            {
                input[variable] = ((tmp & 1) == 0) ? std::vector<Value>({Value::ZERO}) : std::vector<Value>({Value::ONE});
                tmp >>= 1;
            }
            auto result = this->evaluate(input);
            if (std::get_if<std::string>(&result) != nullptr)
            {
                return std::get<std::string>(result);
            }
            auto output = std::get<std::vector<Value>>(result);
            for (auto index = 0u; index < truth_table.size(); index++)
            {
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
        auto reduce_to_z3 = [&context, &var2expr](const auto& node, auto&& p) -> std::tuple<bool, z3::expr> {
            if (node.get_arity() != p.size())
            {
                return {false, z3::expr(context)};
            }

            switch (node.type)
            {
                case BooleanFunction::NodeType::Index:
                    return {true, context.bv_val(node.index, node.size)};
                case BooleanFunction::NodeType::Constant: {
                    // since our constants are defined as arbitrary bit-vectors,
                    // we have to concat each bit just to be on the safe side
                    auto constant = context.bv_val(node.constant.front(), 1);
                    for (u32 i = 1; i < node.constant.size(); i++)
                    {
                        const auto bit = node.constant.at(i);
                        constant = z3::concat(context.bv_val(bit, 1), constant);
                    }
                    return {true, constant};
                }
                case BooleanFunction::NodeType::Variable: {
                    if (auto it = var2expr.find(node.variable); it != var2expr.end())
                    {
                        return {true, it->second};
                    }
                    return {true, context.bv_const(node.variable.c_str(), node.size)};
                }

                case BooleanFunction::NodeType::And:
                    return {true, p[0] & p[1]};
                case BooleanFunction::NodeType::Or:
                    return {true, p[0] | p[1]};
                case BooleanFunction::NodeType::Not:
                    return {true, ~p[0]};
                case BooleanFunction::NodeType::Xor:
                    return {true, p[0] ^ p[1]};

                default:
                    return {false, z3::expr(context)};
            }
        };

        std::vector<z3::expr> stack;
        for (const auto& node : this->m_nodes)
        {
            std::vector<z3::expr> operands;
            std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

            if (auto [ok, reduction] = reduce_to_z3(node, std::move(operands)); ok)
            {
                stack.emplace_back(reduction);
            }
            else
            {
                return z3::expr(context);
            }
        }

        switch (stack.size())
        {
            case 1:
                return stack.back();
            default:
                return z3::expr(context);
        }
    }

    BooleanFunction::BooleanFunction(std::vector<BooleanFunction::Node>&& nodes) : m_nodes(nodes)
    {
    }

    BooleanFunction::BooleanFunction(BooleanFunction::Node&& node, std::vector<BooleanFunction>&& p) : BooleanFunction()
    {
        auto size = 1;
        for (const auto& parameter : p)
        {
            size += parameter.size();
        }
        this->m_nodes.reserve(size);

        for (auto&& parameter : p)
        {
            this->m_nodes.insert(this->m_nodes.end(), parameter.m_nodes.begin(), parameter.m_nodes.end());
        }
        this->m_nodes.emplace_back(node);
    }

    std::string BooleanFunction::to_string_in_reverse_polish_notation() const
    {
        std::string s;
        for (const auto& node : this->m_nodes)
        {
            s += node.to_string() + " ";
        }
        return s;
    }

    std::variant<BooleanFunction, std::string> BooleanFunction::validate(BooleanFunction&& function)
    {
        /// # Note
        /// In order to validate correctness of a Boolean function, we analyze
        /// the arity of each node and whether its value matches the number of
        /// parameters and covered nodes in the abstract syntax tree.
        if (auto coverage = function.compute_node_coverage(); coverage.back() != function.length())
        {
            auto str = function.to_string_in_reverse_polish_notation();
            return "Cannot validate '" + str + "' (= imbalanced function with coverage '" + std::to_string(coverage.back()) + " != " + std::to_string(function.length()) + ").";
        }

        return std::move(function);
    }

    std::vector<u32> BooleanFunction::compute_node_coverage() const
    {
        auto coverage = std::vector<u32>(this->m_nodes.size(), (u32)-1);

        /// Short-hand function to safely access a coverage value from a vector
        ///
        /// @param[in] cov - Coverage vector.
        /// @param[in] index - Index into coverage vector.
        /// @returns Value at coverage[index] or -1 in case index is invalid.
        auto get = [](const auto& cov, size_t index) -> u32 { return (index < cov.size()) ? cov[index] : -1; };

        /// Short-hand function to safely set a coverage value at an index.
        ///
        /// @param[in,out] cov - Coverage vector to be modified.
        /// @param[in] index - Valid index into coverage vector.
        /// @param[in] x - 1st part of coverage value (= 0 on default)
        /// @param[in] y - 2nc part of coverage value (= 0 on default)
        /// @param[in] z - 3rd part of coverage value (= 0 on default)
        auto set = [](auto& cov, size_t index, u32 x = 0, u32 y = 0, u32 z = 0) { cov[index] = ((x != (u32)-1) && (y != (u32)-1) && (z != (u32)-1)) ? (x + y + z + 1) : (u32)-1; };

        for (auto i = 0ul; i < this->m_nodes.size(); i++)
        {
            auto arity = this->m_nodes[i].get_arity();

            switch (arity)
            {
                case 0: {
                    set(coverage, i);
                    break;
                }
                case 1: {
                    auto x = get(coverage, i - 1);
                    set(coverage, i, x);
                    break;
                }
                case 2: {
                    auto x = get(coverage, i - 1);
                    auto y = get(coverage, i - 1 - x);
                    set(coverage, i, x, y);
                    break;
                }
                case 3: {
                    auto x = get(coverage, i - 1);
                    auto y = get(coverage, i - 1 - x);
                    auto z = get(coverage, i - 1 - x - y);
                    set(coverage, i, x, y, z);
                    break;
                }
            }
        }

        return coverage;
    }

    BooleanFunction::Node BooleanFunction::Node::Operation(u16 _type, u16 _size)
    {
        return Node(_type, _size, {}, {}, {});
    }

    BooleanFunction::Node BooleanFunction::Node::Constant(const std::vector<BooleanFunction::Value> _constant)
    {
        return Node(NodeType::Constant, _constant.size(), _constant, {}, {});
    }

    BooleanFunction::Node BooleanFunction::Node::Index(u16 _index, u16 _size)
    {
        return Node(NodeType::Index, _size, {}, _index, {});
    }

    BooleanFunction::Node BooleanFunction::Node::Variable(const std::string _variable, u16 _size)
    {
        return Node(NodeType::Variable, _size, {}, {}, _variable);
    }

    bool BooleanFunction::Node::operator==(const Node& other) const
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) == std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    bool BooleanFunction::Node::operator!=(const Node& other) const
    {
        return !(*this == other);
    }

    bool BooleanFunction::Node::operator<(const Node& other) const
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) < std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    BooleanFunction::Node BooleanFunction::Node::clone() const
    {
        return Node(this->type, this->size, this->constant, this->index, this->variable);
    }

    std::string BooleanFunction::Node::to_string() const
    {
        switch (this->type)
        {
            case NodeType::Constant: {
                std::string str;
                for (const auto& value : this->constant)
                {
                    str = enum_to_string(value) + str;
                }
                return "0b" + str;
            }

            case NodeType::Index:
                return std::to_string(this->index);
            case NodeType::Variable:
                return this->variable;

            case NodeType::And:
                return "&";
            case NodeType::Or:
                return "|";
            case NodeType::Not:
                return "~";
            case NodeType::Xor:
                return "^";

            case NodeType::Add:
                return "+";

            case NodeType::Concat:
                return "++";
            case NodeType::Slice:
                return "Slice";
            case NodeType::Zext:
                return "Zext";

            default:
                return "unsupported node type '" + std::to_string(this->type) + "'.";
        }
    }

    u16 BooleanFunction::Node::get_arity() const
    {
        return BooleanFunction::Node::get_arity_of_type(this->type);
    }

    u16 BooleanFunction::Node::get_arity_of_type(u16 type)
    {
        static const std::map<u16, u16> type2arity = {
            {BooleanFunction::NodeType::And, 2},
            {BooleanFunction::NodeType::Or, 2},
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

    bool BooleanFunction::Node::has_constant_value(u64 value) const
    {
        if (!this->is_constant())
        {
            return false;
        }

        auto bv_value = std::vector<BooleanFunction::Value>({});
        bv_value.reserve(this->size);
        for (auto i = 0u; i < this->constant.size(); i++)
        {
            bv_value.emplace_back((value << (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
        }
        return this->constant == bv_value;
    }

    bool BooleanFunction::Node::is_index() const
    {
        return this->is(BooleanFunction::NodeType::Index);
    }

    bool BooleanFunction::Node::has_index_value(u16 value) const
    {
        return this->is_index() && (this->index == value);
    }

    bool BooleanFunction::Node::is_variable() const
    {
        return this->is(BooleanFunction::NodeType::Variable);
    }

    bool BooleanFunction::Node::has_variable_name(const std::string& value) const
    {
        return this->is_variable() && (this->variable == value);
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
        return (this->type == NodeType::And) || (this->type == NodeType::Or) || (this->type == NodeType::Xor) || (this->type == NodeType::Add);
    }

    BooleanFunction::Node::Node(u16 _type, u16 _size, std::vector<BooleanFunction::Value> _constant, u16 _index, std::string _variable)
        : type(_type), size(_size), constant(_constant), index(_index), variable(_variable)
    {
    }

}    // namespace hal
