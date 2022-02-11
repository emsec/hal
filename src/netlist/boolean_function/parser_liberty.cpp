#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/parser.h"
#include "hal_core/utilities/log.h"

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/spirit/home/x3.hpp>

namespace hal
{
    namespace BooleanFunctionParser
    {
        Result<std::vector<Token>> parse_with_liberty_grammar(const std::string& expression)
        {
            // stores the list of tokens that are generated and filled during the
            // parsing process adn the different semantic actions
            std::vector<Token> tokens;

            ////////////////////////////////////////////////////////////////////////
            // (1) Semantic actions to generate tokens
            ////////////////////////////////////////////////////////////////////////

            const auto AndAction = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::And()); };
            const auto NotAction = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Not()); };
            const auto OrAction  = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Or()); };
            const auto XorAction = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Xor()); };

            const auto NotSuffixAction = [&tokens](auto ctx) {
                // The liberty format defines a ' character as an inversion of the
                // previous expression. Since the only occurrences in the liberty
                // file format belong to variables (e.g., " TE' "), we limit the
                // ability of suffix not operations to variable expressions only.
                //
                // In case a suffix not operation may occur in complex expressions,
                // we should introduce a special not token type that is then handled
                // by the translation to the reverse polish notation.
                if (tokens.empty() || (tokens.back().type != TokenType::Variable))
                {
                    log_error("netlist", "Cannot handle not suffix character ' in liberty parser as it is required that the predecessor token is a variable.");
                    _pass(ctx) = false;
                    return;
                }
                auto variable = tokens.back();
                tokens.pop_back();

                tokens.emplace_back(Token::Not());
                tokens.emplace_back(variable);
            };

            const auto BracketOpenAction  = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::BracketOpen()); };
            const auto BracketCloseAction = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::BracketClose()); };

            const auto VariableAction = [&tokens](auto& ctx) {
                // combines the first matched character with the remaining string
                std::stringstream name;
                name << std::string(1, boost::fusion::at_c<0>(_attr(ctx))) << boost::fusion::at_c<1>(_attr(ctx));

                tokens.emplace_back(BooleanFunctionParser::Token::Variable(name.str(), 1));
            };
            const auto VariableIndexAction = [&tokens](auto& ctx) {
                // combines the first matched character with the remaining string
                std::stringstream name;
                name << std::string(1, boost::fusion::at_c<0>(_attr(ctx))) << boost::fusion::at_c<1>(_attr(ctx)) << boost::fusion::at_c<2>(_attr(ctx)) << boost::fusion::at_c<3>(_attr(ctx))
                     << boost::fusion::at_c<4>(_attr(ctx));
                tokens.emplace_back(BooleanFunctionParser::Token::Variable(name.str(), 1));
            };
            const auto ConstantAction = [&tokens](auto& ctx) {
                auto value = (_attr(ctx) == '0') ? BooleanFunction::Value::ZERO : BooleanFunction::Value::ONE;
                tokens.emplace_back(BooleanFunctionParser::Token::Constant({value}));
            };

            ////////////////////////////////////////////////////////////////////////
            // (2) Rules
            ////////////////////////////////////////////////////////////////////////

            namespace x3 = boost::spirit::x3;

            const auto AndRule       = x3::char_("& *")[AndAction];
            const auto NotRule       = x3::lit("!")[NotAction];
            const auto NotSuffixRule = x3::lit("'")[NotSuffixAction];
            const auto OrRule        = x3::lit("|+")[OrAction];
            const auto XorRule       = x3::lit("^")[XorAction];

            const auto BracketOpenRule  = x3::lit("(")[BracketOpenAction];
            const auto BracketCloseRule = x3::lit(")")[BracketCloseAction];

            const auto VariableRule      = x3::lexeme[(x3::char_("a-zA-Z") >> *x3::char_("a-zA-Z0-9_"))][VariableAction];
            const auto VariableIndexRule = x3::lexeme[(x3::char_("a-zA-Z") >> *x3::char_("a-zA-Z0-9_") >> x3::char_("(") >> x3::int_ >> x3::char_(")"))][VariableIndexAction];
            const auto ConstantRule      = x3::lexeme[x3::char_("0-1")][ConstantAction];

            auto iter     = expression.begin();
            const auto ok = x3::phrase_parse(iter,
                                             expression.end(),
                                             ////////////////////////////////////////////////////////////////////
                                             // (3) Parsing Expression Grammar
                                             ////////////////////////////////////////////////////////////////////
                                             +(AndRule | NotRule | NotSuffixRule | OrRule | XorRule | VariableIndexRule | VariableRule | ConstantRule | BracketOpenRule | BracketCloseRule),
                                             // we use an invalid a.k.a. non-printable ASCII character in order
                                             // to prevent the skipping of space characters as they are defined
                                             // as an and operation
                                             x3::char_(0x00));

            if (!ok || (iter != expression.end()))
            {
                return ERR("Unable to parse Boolean function '" + expression + "' (= " + std::string(iter, expression.end()) + ").");
            }

            return OK(tokens);
        }
    }    // namespace BooleanFunctionParser
}    // namespace hal