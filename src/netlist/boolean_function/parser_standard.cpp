#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/parser.h"

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/spirit/home/x3.hpp>
#include <sstream>

namespace hal
{
    namespace BooleanFunctionParser
    {
        Result<std::vector<Token>> parse_with_standard_grammar(const std::string& expression, const std::map<std::string, u16>& var_sizes)
        {
            // stores the list of tokens that are generated and filled during the
            // parsing process adn the different semantic actions
            std::vector<Token> tokens;

            // Looks up a variable's bit-width from the caller-supplied map; defaults to 1 when absent.
            const auto var_size = [&var_sizes](const std::string& name) -> u16 {
                if (auto it = var_sizes.find(name); it != var_sizes.end())
                {
                    return it->second;
                }
                return 1;
            };

            ////////////////////////////////////////////////////////////////////////
            // (1) Semantic actions to generate tokens
            ////////////////////////////////////////////////////////////////////////

            const auto AndAction      = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::And()); };
            const auto NotAction      = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Not()); };
            const auto OrAction       = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Or()); };
            const auto XorAction      = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Xor()); };
            const auto EqAction       = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Eq()); };
            const auto QuestionAction = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Question()); };
            const auto ColonAction    = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Colon()); };
            const auto AddAction      = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Add()); };
            const auto SubAction      = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Sub()); };
            const auto MulAction      = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::Mul()); };

            const auto BracketOpenAction  = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::BracketOpen()); };
            const auto BracketCloseAction = [&tokens](auto& /* ctx */) { tokens.emplace_back(BooleanFunctionParser::Token::BracketClose()); };

            const auto MultiBitBinAction = [&tokens](auto& ctx) {
                // _attr(ctx) is a vector<char>/std::string of binary digits (MSB first).
                const auto& bits = _attr(ctx);
                std::vector<BooleanFunction::Value> values;
                values.reserve(bits.size());
                // BooleanFunction stores constants LSB first, so iterate in reverse.
                for (auto it = bits.rbegin(); it != bits.rend(); ++it)
                {
                    values.push_back((*it == '0') ? BooleanFunction::Value::ZERO : BooleanFunction::Value::ONE);
                }
                tokens.emplace_back(BooleanFunctionParser::Token::Constant(values));
            };

            const auto MultiBitHexAction = [&tokens](auto& ctx) {
                // Each hex digit contributes 4 bits; LSB first.
                const auto& digits = _attr(ctx);
                std::vector<BooleanFunction::Value> values;
                values.reserve(digits.size() * 4);
                for (auto it = digits.rbegin(); it != digits.rend(); ++it)
                {
                    const char c = *it;
                    u8 nibble    = 0;
                    if (c >= '0' && c <= '9')
                        nibble = c - '0';
                    else if (c >= 'a' && c <= 'f')
                        nibble = c - 'a' + 10;
                    else
                        nibble = c - 'A' + 10;
                    for (int i = 0; i < 4; ++i)
                    {
                        values.push_back((nibble & (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
                    }
                }
                tokens.emplace_back(BooleanFunctionParser::Token::Constant(values));
            };

            const auto MultiBitOctAction = [&tokens](auto& ctx) {
                // Each octal digit contributes 3 bits; LSB first.
                const auto& digits = _attr(ctx);
                std::vector<BooleanFunction::Value> values;
                values.reserve(digits.size() * 3);
                for (auto it = digits.rbegin(); it != digits.rend(); ++it)
                {
                    const u8 oct = *it - '0';
                    for (int i = 0; i < 3; ++i)
                    {
                        values.push_back((oct & (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
                    }
                }
                tokens.emplace_back(BooleanFunctionParser::Token::Constant(values));
            };

            const auto VariableAction = [&tokens, &var_size](auto& ctx) {
                // # Developer Note
                // We combine the first matched character with the remaining
                // string and do not remove any preceding '/' character.
                std::stringstream name;
                name << std::string(1, boost::fusion::at_c<0>(_attr(ctx)));
                name << boost::fusion::at_c<1>(_attr(ctx));

                tokens.emplace_back(BooleanFunctionParser::Token::Variable(name.str(), var_size(name.str())));
            };
            const auto VariableIndexAction = [&tokens, &var_size](auto& ctx) {
                // # Developer Note
                // Since the first character is an optional '\' character and
                // generally escaped a.k.a. removed within HAL, we also do not
                // touch the part and only assemble the remaining string.
                std::stringstream name;
                name << std::string(1, boost::fusion::at_c<1>(_attr(ctx)));
                name << boost::fusion::at_c<2>(_attr(ctx));
                name << boost::fusion::at_c<3>(_attr(ctx));
                name << boost::fusion::at_c<4>(_attr(ctx));
                name << boost::fusion::at_c<5>(_attr(ctx));
                tokens.emplace_back(BooleanFunctionParser::Token::Variable(name.str(), var_size(name.str())));
            };
            const auto ConstantAction = [&tokens](auto& ctx) {
                const auto value = (_attr(ctx) == '0') ? BooleanFunction::Value::ZERO : BooleanFunction::Value::ONE;
                tokens.emplace_back(BooleanFunctionParser::Token::Constant({value}));
            };

            ////////////////////////////////////////////////////////////////////////
            // (2) Rules
            ////////////////////////////////////////////////////////////////////////

            namespace x3 = boost::spirit::x3;

            const auto AndRule      = x3::lit("&")[AndAction];
            const auto NotRule      = x3::char_("!~")[NotAction];
            const auto OrRule       = x3::lit("|")[OrAction];
            const auto XorRule      = x3::lit("^")[XorAction];
            const auto EqRule       = x3::lit("==")[EqAction];
            const auto QuestionRule = x3::lit("?")[QuestionAction];
            const auto ColonRule    = x3::lit(":")[ColonAction];
            const auto AddRule      = x3::lit("+")[AddAction];
            const auto SubRule      = x3::lit("-")[SubAction];
            const auto MulRule      = x3::lit("*")[MulAction];

            const auto BracketOpenRule  = x3::lit("(")[BracketOpenAction];
            const auto BracketCloseRule = x3::lit(")")[BracketCloseAction];

            const auto VariableRule = x3::lexeme[(x3::char_("a-zA-Z") >> *x3::char_("a-zA-Z0-9_"))][VariableAction];
            const auto VariableIndexRoundBracketRule =
                x3::lexeme[(-(x3::char_("\\")) >> x3::char_("a-zA-Z") >> *x3::char_("a-zA-Z0-9_") >> x3::char_("(") >> x3::int_ >> x3::char_(")"))][VariableIndexAction];
            const auto VariableIndexSquareBracketRule =
                x3::lexeme[(-(x3::char_("\\")) >> x3::char_("a-zA-Z") >> *x3::char_("a-zA-Z0-9_") >> x3::char_("[") >> x3::int_ >> x3::char_("]"))][VariableIndexAction];
            const auto VariableIndexRule = VariableIndexRoundBracketRule | VariableIndexSquareBracketRule;

            const auto ConstantRule       = x3::lexeme[x3::char_("0-1")][ConstantAction];
            const auto ConstantBinRule    = x3::lit("0b") >> x3::lexeme[+x3::char_("0-1")][MultiBitBinAction];
            const auto ConstantHexRule    = x3::lit("0x") >> x3::lexeme[+x3::char_("0-9a-fA-F")][MultiBitHexAction];
            const auto ConstantOctRule    = x3::lit("0o") >> x3::lexeme[+x3::char_("0-7")][MultiBitOctAction];
            const auto ConstantSuffixRule = x3::lexeme[x3::char_("0-1") >> x3::lit("'b1")][ConstantAction];

            auto iter     = expression.begin();
            const auto ok = x3::phrase_parse(
                iter,
                expression.end(),
                ////////////////////////////////////////////////////////////////////
                // (3) Parsing Expression Grammar
                ////////////////////////////////////////////////////////////////////
                +(AndRule | NotRule | OrRule | XorRule | EqRule | QuestionRule | ColonRule | AddRule | SubRule | MulRule | VariableIndexRule | VariableRule | ConstantSuffixRule | ConstantHexRule
                  | ConstantOctRule | ConstantBinRule | ConstantRule | BracketOpenRule | BracketCloseRule),
                x3::space    // skips any whitespace in between boolean function
            );

            if (!ok || (iter != expression.end()))
            {
                return ERR("could not parse Boolean function '" + expression + "': " + std::string(iter, expression.end()));
            }

            return OK(tokens);
        }
    }    // namespace BooleanFunctionParser
}    // namespace hal