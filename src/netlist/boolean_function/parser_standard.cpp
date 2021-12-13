#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/parser.h"

#include <boost/spirit/home/x3.hpp>

namespace hal {
namespace BooleanFunctionParser {
	std::variant<std::vector<Token>, std::string> parse_with_standard_grammar(const std::string& expression) {
        // stores the list of tokens that are generated and filled during the 
        // parsing process adn the different semantic actions
        std::vector<Token> tokens;
        
        ////////////////////////////////////////////////////////////////////////
        // (1) Semantic actions to generate tokens
        ////////////////////////////////////////////////////////////////////////

        auto AndAction = [&tokens] (auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::And()); };
        auto NotAction = [&tokens] (auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Not()); };
        auto  OrAction = [&tokens] (auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Or()); };
        auto XorAction = [&tokens] (auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Xor()); };
        
        auto BracketOpenAction = [&tokens] (auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::BracketOpen()); };
        auto BracketCloseAction = [&tokens] (auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::BracketClose()); };
        
        auto VariableAction = [&tokens] (auto& ctx) { 
            // combines the first matched character with the remaining string
            std::stringstream name; name << std::string(1, at_c<0>(_attr(ctx))) << at_c<1>(_attr(ctx));

            tokens.emplace_back(BooleanFunctionParser::Token::Variable(name.str(), 1));
        };
        auto VariableIndexAction = [&tokens] (auto& ctx) {
            // combines the first matched character with the remaining string
            std::stringstream name; name << std::string(1, at_c<0>(_attr(ctx))) << at_c<1>(_attr(ctx)) << at_c<2>(_attr(ctx)) << at_c<3>(_attr(ctx)) << at_c<4>(_attr(ctx));
            tokens.emplace_back(BooleanFunctionParser::Token::Variable(name.str(), 1));
        };
        auto ConstantAction = [&tokens] (auto& ctx) { 
            auto value = (_attr(ctx) == '0') ? BooleanFunction::Value::ZERO : BooleanFunction::Value::ONE;
            tokens.emplace_back(BooleanFunctionParser::Token::Constant({value}));
        };

        ////////////////////////////////////////////////////////////////////////
        // (2) Rules
        ////////////////////////////////////////////////////////////////////////

        namespace x3 = boost::spirit::x3;

        auto AndRule = x3::lit("&") [AndAction];
        auto NotRule = x3::lit("!") [NotAction];
        auto OrRule  = x3::lit("|") [OrAction];
        auto XorRule = x3::lit("^") [XorAction];
        
        auto BracketOpenRule = x3::lit("(") [BracketOpenAction];
        auto BracketCloseRule = x3::lit(")") [BracketCloseAction];
        
        auto VariableRule = x3::lexeme[(x3::char_("a-zA-Z") >> *x3::char_("a-zA-Z0-9"))] [VariableAction];
        auto VariableIndexRule = x3::lexeme[(x3::char_("a-zA-Z") >> *x3::char_("a-zA-Z0-9") >> x3::char_("(") >> x3::int_ >> x3::char_(")"))] [VariableIndexAction];
        auto ConstantRule = x3::lexeme[x3::char_("0-1")] [ConstantAction];

        auto iter = expression.begin();
        auto ok = x3::phrase_parse(iter, expression.end(),
            ////////////////////////////////////////////////////////////////////
            // (3) Parsing Expression Grammar
            ////////////////////////////////////////////////////////////////////
            +(
                AndRule | NotRule | OrRule | XorRule 
                | VariableIndexRule | VariableRule | ConstantRule 
                | BracketOpenRule | BracketCloseRule
            ), x3::space // skips any whitespace in between boolean function
        );

        if (!ok || (iter != expression.end())) {
            return "Unable to parse Boolean function '" + expression +"' (= " +  std::string(iter, expression.end()) + ").";
        }
        return tokens;
    }
}  // namespace BooleanFunctionParser
}  // namespace hal