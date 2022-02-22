#include "hal_core/netlist/boolean_function/parser.h"

#include "hal_core/utilities/log.h"

#include <stack>
#include <deque>

namespace hal {
namespace BooleanFunctionParser {
    Token Token::And() {
        return Token(TokenType::And, {}, {});
    }

    Token Token::Not() {
        return Token(TokenType::Not, {}, {});
    }

    Token Token::Or() {
        return Token(TokenType::Or, {}, {});
    }

    Token Token::Xor() {
        return Token(TokenType::Xor, {}, {});
    }

    Token Token::Variable(std::string name, u16 size) {
        return Token(TokenType::Variable, std::make_tuple(name, size), {});
    }

    Token Token::Constant(std::vector<BooleanFunction::Value> constant) {
        return Token(TokenType::Constant, std::make_tuple("", 0), constant);
    }

    Token Token::BracketOpen() {
        return Token(TokenType::BracketOpen, {}, {});
    }

    Token Token::BracketClose() {
        return Token(TokenType::BracketClose, {}, {});
    }

    unsigned Token::precedence(const ParserType& parser) const {
        static const std::map<ParserType, std::map<TokenType, unsigned>> parser2precedence = {
            {ParserType::Liberty, std::map<TokenType, unsigned>({
                {TokenType::Not, 4},
                {TokenType::And, 3},
                {TokenType::Xor, 2},
                {TokenType::Or,  1},
            })},
            {ParserType::Standard, std::map<TokenType, unsigned>({
                {TokenType::Not, 4},
                {TokenType::And, 3},
                {TokenType::Xor, 2},
                {TokenType::Or,  1},
            })},
        };

        const auto& precedence = parser2precedence.at(parser);
        if (auto iter = precedence.find(this->type); iter != precedence.end()) {
            return iter->second;
        }

        return 5;
    }

    bool Token::is(TokenType _type) const {
        return this->type == _type;
    }

    std::ostream& operator<<(std::ostream& os, const Token& token) {
        static const std::map<TokenType, std::string> type2str = {
            {TokenType::And, "And"},
            {TokenType::Or, "Or"},
            {TokenType::Not, "Not"},
            {TokenType::Xor, "Xor"},
            
            {TokenType::BracketOpen, "BracketOpen"},
            {TokenType::BracketClose, "BracketClose"},

            {TokenType::Variable, "Variable"},
            {TokenType::Constant, "Constant"},
        };

        os << "Token { " << type2str.at(token.type);

        if (token.type == TokenType::Variable) {
            os << ", " << std::get<0>(token.variable) << " (" << std::get<1>(token.variable) << ")";
        }
        if (token.type == TokenType::Constant) {
            os << ", " << "0b";
            for (auto i = token.constant.size(); i-- != 0;) {
                os << token.constant[i];   
            }
        }

        os << " }";
        return os;
    }

    std::string Token::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    Token::Token(TokenType _type, std::tuple<std::string, u16> _variable, std::vector<BooleanFunction::Value> _constant)
        : type(_type), variable(_variable), constant(_constant) {}

    Result<std::vector<Token>> reverse_polish_notation(std::vector<Token>&& tokens, const std::string& expression, const ParserType& parser) {
        std::stack<Token> operator_stack;
        std::vector<Token> output;

        for (auto&& token : tokens) {
            switch (token.type) {
                // (1) constants / variables are always pushed into output queue
                case TokenType::Constant: 
                case TokenType::Variable: {
                    output.emplace_back(token); break;
                }

                // (2) operations are pushed into the operator stack with respect
                //     to the operator precedence and bracket level
                case TokenType::And:
                case TokenType::Or:
                case TokenType::Not:
                case TokenType::Xor: {
                    while (!operator_stack.empty() && !operator_stack.top().is(TokenType::BracketOpen) && operator_stack.top().precedence(parser) > token.precedence(parser)) {
                        output.emplace_back(operator_stack.top()); 
                        operator_stack.pop();
                    }
                    operator_stack.push(token);
                    break;
                }

                // (3) open brackets are always pushed into the operator stack
                case TokenType::BracketOpen: {
                    operator_stack.push(token); break;
                }
                
                // (4) close brackets push remaining operations into the output
                //     queue until the bracket level is closed
                case TokenType::BracketClose: {
                    while (!operator_stack.empty() && !operator_stack.top().is(TokenType::BracketOpen)) {
                        output.emplace_back(operator_stack.top()); 
                        operator_stack.pop();
                    }

                    operator_stack.pop();

                    break;
                }
            }
        }

        while (!operator_stack.empty()) {
            if (operator_stack.top().is(TokenType::BracketOpen)) {
                return ERR("Cannot translate '" + expression + "' to reverse polish notation as bracket level is invalid.");
            }

            output.emplace_back(operator_stack.top()); 
            operator_stack.pop();
        }
        return OK(output);
    }

    Result<BooleanFunction> translate(std::vector<Token>&& tokens, const std::string& expression) {
        using TokenType = BooleanFunctionParser::TokenType;

        auto nodes = std::vector<BooleanFunction::Node>(); nodes.reserve(tokens.size());

        for (auto&& token : tokens) {
            if ((token.is(TokenType::And) || token.is(TokenType::Or) || token.is(TokenType::Not) || token.is(TokenType::Xor))
                && nodes.empty()) {
                return ERR("Cannot translate tokens (= tokens are imbalanced, i.e. operation before operation nodes)");
            }

            switch (token.type) {
                case TokenType::And:
                    nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::And, nodes.back().size)); break;
                case TokenType::Or:
                    nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Or, nodes.back().size)); break;
                case TokenType::Not:
                    nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Not, nodes.back().size)); break;
                case TokenType::Xor:
                    nodes.emplace_back(BooleanFunction::Node::Operation(BooleanFunction::NodeType::Xor, nodes.back().size)); break;

                case TokenType::Variable:
                    nodes.emplace_back(BooleanFunction::Node::Variable(std::get<0>(token.variable), std::get<1>(token.variable))); break;
                case TokenType::Constant:
                    nodes.emplace_back(BooleanFunction::Node::Constant(token.constant)); break;

                default: return ERR("Cannot handle '" + token.to_string() + "' in '" + expression + "'.");
            }
        }

        return BooleanFunction::build(std::move(nodes));
    }
}  // namespace BooleanFunctionParser
}  // namespace hal
