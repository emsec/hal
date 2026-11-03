#include "hal_core/netlist/boolean_function/parser.h"
#include "hal_core/utilities/log.h"

#include <deque>
#include <stack>

namespace hal
{
    namespace BooleanFunctionParser
    {
        Token Token::And()
        {
            return Token(TokenType::And, {}, {});
        }

        Token Token::Not()
        {
            return Token(TokenType::Not, {}, {});
        }

        Token Token::NotSuffix()
        {
            return Token(TokenType::NotSuffix, {}, {});
        }

        Token Token::Or()
        {
            return Token(TokenType::Or, {}, {});
        }

        Token Token::Xor()
        {
            return Token(TokenType::Xor, {}, {});
        }

        Token Token::Eq()
        {
            return Token(TokenType::Eq, {}, {});
        }

        Token Token::Question()
        {
            return Token(TokenType::Question, {}, {});
        }

        Token Token::Colon()
        {
            return Token(TokenType::Colon, {}, {});
        }

        Token Token::Add()
        {
            return Token(TokenType::Add, {}, {});
        }

        Token Token::Sub()
        {
            return Token(TokenType::Sub, {}, {});
        }

        Token Token::Mul()
        {
            return Token(TokenType::Mul, {}, {});
        }

        Token Token::Variable(std::string name, u16 size)
        {
            return Token(TokenType::Variable, std::make_tuple(name, size), {});
        }

        Token Token::Constant(std::vector<BooleanFunction::Value> constant)
        {
            return Token(TokenType::Constant, std::make_tuple("", 0), constant);
        }

        Token Token::BracketOpen()
        {
            return Token(TokenType::BracketOpen, {}, {});
        }

        Token Token::BracketClose()
        {
            return Token(TokenType::BracketClose, {}, {});
        }

        unsigned Token::precedence(const ParserType& parser) const
        {
            static const std::map<ParserType, std::map<TokenType, unsigned>> parser2precedence = {
                {ParserType::Liberty,
                 std::map<TokenType, unsigned>({
                     {TokenType::NotSuffix, 4},
                     {TokenType::Not, 4},
                     {TokenType::And, 3},
                     {TokenType::Xor, 2},
                     {TokenType::Or, 1},
                 })},
                {ParserType::Standard,
                 std::map<TokenType, unsigned>({
                     {TokenType::Not, 8},
                     {TokenType::Mul, 7},
                     {TokenType::Add, 6},
                     {TokenType::Sub, 6},
                     {TokenType::Eq, 5},
                     {TokenType::And, 4},
                     {TokenType::Xor, 3},
                     {TokenType::Or, 2},
                     {TokenType::Question, 1},
                     {TokenType::Colon, 1},
                 })},
            };

            const auto& precedence = parser2precedence.at(parser);
            if (auto iter = precedence.find(this->type); iter != precedence.end())
            {
                return iter->second;
            }

            return 9;
        }

        bool Token::is_left_associative() const
        {
            // Arithmetic operators follow C-style left-associativity. All other operators
            // remain right-associative (the long-standing default for the BF parser).
            return this->type == TokenType::Add || this->type == TokenType::Sub || this->type == TokenType::Mul;
        }

        bool Token::is(TokenType _type) const
        {
            return this->type == _type;
        }

        std::ostream& operator<<(std::ostream& os, const Token& token)
        {
            static const std::map<TokenType, std::string> type2str = {
                {TokenType::And, "And"},
                {TokenType::Or, "Or"},
                {TokenType::Not, "Not"},
                {TokenType::NotSuffix, "NotSuffix"},
                {TokenType::Xor, "Xor"},
                {TokenType::Eq, "Eq"},
                {TokenType::Question, "Question"},
                {TokenType::Colon, "Colon"},
                {TokenType::Add, "Add"},
                {TokenType::Sub, "Sub"},
                {TokenType::Mul, "Mul"},

                {TokenType::BracketOpen, "BracketOpen"},
                {TokenType::BracketClose, "BracketClose"},

                {TokenType::Variable, "Variable"},
                {TokenType::Constant, "Constant"},
            };

            os << "Token { " << type2str.at(token.type);

            if (token.type == TokenType::Variable)
            {
                os << ", " << std::get<0>(token.variable) << " (" << std::get<1>(token.variable) << ")";
            }
            if (token.type == TokenType::Constant)
            {
                os << ", "
                   << "0b";
                for (auto i = token.constant.size(); i-- != 0;)
                {
                    os << token.constant[i];
                }
            }

            os << " }";
            return os;
        }

        std::string Token::to_string() const
        {
            std::stringstream ss;
            ss << *this;
            return ss.str();
        }

        Token::Token(TokenType _type, std::tuple<std::string, u16> _variable, std::vector<BooleanFunction::Value> _constant) : type(_type), variable(_variable), constant(_constant)
        {
        }

        Result<std::vector<Token>> reverse_polish_notation(std::vector<Token>&& tokens, const std::string& expression, const ParserType& parser)
        {
            // Validate ternary structure: every `:` must be preceded (in source order) by an
            // unmatched `?`, and every `?` must eventually have a matching `:`. This catches
            // isolated `?`/`:`, wrong order (e.g. `a : b ? c`), and unbalanced/incomplete
            // ternaries before the Shunting Yard step silently accepts them.
            int ternary_depth = 0;
            for (const auto& tok : tokens)
            {
                if (tok.is(TokenType::Question))
                {
                    ++ternary_depth;
                }
                else if (tok.is(TokenType::Colon))
                {
                    if (ternary_depth == 0)
                    {
                        return ERR("could not translate '" + expression + "' to reverse polish notation: unmatched ':' in ternary expression");
                    }
                    --ternary_depth;
                }
            }
            if (ternary_depth != 0)
            {
                return ERR("could not translate '" + expression + "' to reverse polish notation: unmatched '?' in ternary expression");
            }

            std::stack<Token> operator_stack;
            std::vector<Token> output;

            for (auto&& token : tokens)
            {
                switch (token.type)
                {
                    // (1) constants / variables are always pushed into output queue
                    case TokenType::Constant:
                    case TokenType::Variable: {
                        output.emplace_back(token);
                        break;
                    }

                    // (2) operations are pushed into the operator stack with respect
                    //     to the operator precedence and bracket level
                    case TokenType::And:
                    case TokenType::Or:
                    case TokenType::Not:
                    case TokenType::Xor:
                    case TokenType::Eq:
                    case TokenType::Question:
                    case TokenType::Colon:
                    case TokenType::Add:
                    case TokenType::Sub:
                    case TokenType::Mul: {
                        const bool left_assoc = token.is_left_associative();
                        while (!operator_stack.empty() && !operator_stack.top().is(TokenType::BracketOpen)
                               && (operator_stack.top().precedence(parser) > token.precedence(parser)
                                   || (left_assoc && operator_stack.top().precedence(parser) == token.precedence(parser))))
                        {
                            output.emplace_back(operator_stack.top());
                            operator_stack.pop();
                        }
                        operator_stack.push(token);
                        break;
                    }

                    // (3) the liberty grammar contains suffix not operations that
                    //     invert the previous operation, i.e, acts similiar to  
                    //     a "not" operation in reverse-polish notation. hence, 
                    //     we simply push a "not" the output as the previous
                    //     expression has been already translated to the reverse-
                    //     polish notation in the output vector.
                    case TokenType::NotSuffix: {
                        output.emplace_back(Token::Not());
                        break;
                    }

                    // (4) open brackets are always pushed into the operator stack
                    case TokenType::BracketOpen: {
                        operator_stack.push(token);
                        break;
                    }

                    // (5) close brackets push remaining operations into the output
                    //     queue until the bracket level is closed
                    case TokenType::BracketClose: {
                        while (!operator_stack.empty() && !operator_stack.top().is(TokenType::BracketOpen))
                        {
                            output.emplace_back(operator_stack.top());
                            operator_stack.pop();
                        }

                        operator_stack.pop();

                        break;
                    }
                }
            }

            while (!operator_stack.empty())
            {
                if (operator_stack.top().is(TokenType::BracketOpen))
                {
                    return ERR("could not translate '" + expression + "' to reverse polish notation: bracket level is invalid");
                }

                output.emplace_back(operator_stack.top());
                operator_stack.pop();
            }
            return OK(output);
        }

        Result<BooleanFunction> translate(std::vector<Token>&& tokens, const std::string& expression)
        {
            auto nodes = std::vector<BooleanFunction::Node>();
            nodes.reserve(tokens.size());

            for (auto&& token : tokens)
            {
                if ((token.is(BooleanFunctionParser::TokenType::And) || token.is(BooleanFunctionParser::TokenType::Or) || token.is(BooleanFunctionParser::TokenType::Not) || token.is(BooleanFunctionParser::TokenType::Xor)
                     || token.is(BooleanFunctionParser::TokenType::Eq) || token.is(BooleanFunctionParser::TokenType::Colon)
                     || token.is(BooleanFunctionParser::TokenType::Add) || token.is(BooleanFunctionParser::TokenType::Sub) || token.is(BooleanFunctionParser::TokenType::Mul))
                    && nodes.empty())
                {
                    return ERR("could not translate tokens: tokens are imbalanced, i.e., the operation comes before the operand nodes");
                }

                switch (token.type)
                {
                    case BooleanFunctionParser::TokenType::And:
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::And, nodes.back().size));
                        break;
                    case BooleanFunctionParser::TokenType::Or:
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Or, nodes.back().size));
                        break;
                    case BooleanFunctionParser::TokenType::Not:
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Not, nodes.back().size));
                        break;
                    case BooleanFunctionParser::TokenType::Xor:
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Xor, nodes.back().size));
                        break;
                    case BooleanFunctionParser::TokenType::Add:
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Add, nodes.back().size));
                        break;
                    case BooleanFunctionParser::TokenType::Sub:
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Sub, nodes.back().size));
                        break;
                    case BooleanFunctionParser::TokenType::Mul:
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Mul, nodes.back().size));
                        break;
                    case BooleanFunctionParser::TokenType::Eq:
                        // Eq returns a 1-bit boolean result regardless of operand width.
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Eq, 1));
                        break;
                    case BooleanFunctionParser::TokenType::Question:
                        // `?` is a syntactic marker; the actual Ite node is emitted on `:`.
                        break;
                    case BooleanFunctionParser::TokenType::Colon: {
                        // The result size of Ite matches the size of the then/else branches.
                        // The branches are the two top-most nodes (then below else on the stack).
                        if (nodes.size() < 3)
                        {
                            return ERR("could not translate tokens: ternary `:` requires three preceding operands");
                        }
                        const u16 ite_size = nodes.back().size;
                        nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Ite, ite_size));
                        break;
                    }

                    case BooleanFunctionParser::TokenType::Variable:
                        nodes.emplace_back(BooleanFunction::Node::Variable(std::get<0>(token.variable), std::get<1>(token.variable)));
                        break;
                    case BooleanFunctionParser::TokenType::Constant:
                        nodes.emplace_back(BooleanFunction::Node::Constant(token.constant));
                        break;

                    default:
                        return ERR("could not translate tokens: unable to handle '" + token.to_string() + "' in '" + expression + "'");
                }
            }

            if (auto res = BooleanFunction::build(std::move(nodes)); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not translate tokens: unable to build Boolean function from vector of nodes");
            }
            else
            {
                return res;
            }
        }
    }    // namespace BooleanFunctionParser
}    // namespace hal
