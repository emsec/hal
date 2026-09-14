#include "vhdl_parser/vhdl_lexer.h"

#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace vhdl;

    class VHDLLexerTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
        }

        virtual void TearDown()
        {
            test_utils::remove_sandbox_directory();
        }

        std::vector<Token> lex(const std::string& text)
        {
            auto res = lex_string(text, "test.vhd");
            if (res.is_error())
            {
                ADD_FAILURE() << res.get_error().get();
                return {};
            }
            auto tokens = res.get();
            tokens.pop_back();
            return tokens;
        }

        static std::vector<std::string> texts(const std::vector<Token>& tokens)
        {
            std::vector<std::string> res;
            for (const Token& t : tokens)
            {
                res.push_back(t.text);
            }
            return res;
        }

        static std::vector<TokenKind> kinds(const std::vector<Token>& tokens)
        {
            std::vector<TokenKind> res;
            for (const Token& t : tokens)
            {
                res.push_back(t.kind);
            }
            return res;
        }
    };

    /**
     * Testing identifiers and keywords in any case, extended identifiers with their escapes, and the folded keys.
     */
    TEST_F(VHDLLexerTest, check_identifiers)
    {
        TEST_START
        auto tokens = lex("ENTITY Top_1 is \\Mixed Case\\ \\a\\\\b\\ \\NLW_x/y[3]\\ \\entity\\ end");
        ASSERT_EQ(tokens.size(), 8);
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"ENTITY", "Top_1", "is", "Mixed Case", "a\\b", "NLW_x/y[3]", "entity", "end"}));
        EXPECT_EQ(kinds(tokens),
                  (std::vector<TokenKind>{TokenKind::Identifier,
                                          TokenKind::Identifier,
                                          TokenKind::Identifier,
                                          TokenKind::ExtendedIdentifier,
                                          TokenKind::ExtendedIdentifier,
                                          TokenKind::ExtendedIdentifier,
                                          TokenKind::ExtendedIdentifier,
                                          TokenKind::Identifier}));
        EXPECT_TRUE(tokens.at(0).is("entity"));
        EXPECT_TRUE(tokens.at(2).is("is"));
        EXPECT_FALSE(tokens.at(6).is("entity"));    // extended, never a keyword
        EXPECT_EQ(tokens.at(1).folded(), "top_1");
        EXPECT_EQ(tokens.at(3).folded(), "\\Mixed Case\\");
        EXPECT_EQ(fold("Top_1"), "top_1");
        EXPECT_EQ(fold("A", true), "\\A\\");
        EXPECT_NE(fold("A", true), fold("a", true));
        EXPECT_NE(fold("a", true), fold("a"));
        TEST_END
    }

    /**
     * Testing literals: characters versus attribute ticks, strings with doubled quotes, bit strings in every base and
     * with a size, integers, reals and based numbers.
     */
    TEST_F(VHDLLexerTest, check_literals)
    {
        TEST_START
        auto tokens = lex("'0' '1' 'Z' a'length x(1)'event a xor '1' \"str\" \"say \"\"hi\"\"\" \"\" X\"AB\" b\"01_10\" O\"17\" 8X\"F\" UX\"1\" 12 1_000 1.5 1e3 2.5E-2 16#FF# 2#1010#E1");
        EXPECT_EQ(texts(tokens),
                  (std::vector<std::string>{"0",  "1", "Z", "a", "'",  "length", "x", "(", "1", ")", "'", "event", "a", "xor", "1", "str", "say \"hi\"", "", "X\"AB\"", "b\"01_10\"", "O\"17\"", "8X\"F\"", "UX\"1\"", "12",
                                            "1_000", "1.5", "1e3", "2.5E-2", "16#FF#", "2#1010#E1"}));
        EXPECT_EQ(tokens.at(0).kind, TokenKind::Character);
        EXPECT_EQ(tokens.at(2).kind, TokenKind::Character);
        EXPECT_TRUE(tokens.at(4).is_symbol("'"));
        EXPECT_TRUE(tokens.at(10).is_symbol("'"));
        EXPECT_EQ(tokens.at(14).kind, TokenKind::Character);    // after an operator keyword
        EXPECT_EQ(tokens.at(15).kind, TokenKind::String);
        EXPECT_EQ(tokens.at(17).kind, TokenKind::String);
        for (u32 i = 18; i <= 22; i++)
        {
            EXPECT_EQ(tokens.at(i).kind, TokenKind::BitString) << tokens.at(i).text;
        }
        for (u32 i = 23; i <= 29; i++)
        {
            EXPECT_EQ(tokens.at(i).kind, TokenKind::Number) << tokens.at(i).text;
        }
        // a character literal right after a name is not one: `=> '0'` is, `a'` is a tick
        tokens = lex("I => '0', O => open);");
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"I", "=>", "0", ",", "O", "=>", "open", ")", ";"}));
        EXPECT_EQ(tokens.at(2).kind, TokenKind::Character);

        NO_COUT_TEST_BLOCK;
        EXPECT_TRUE(lex_string("\"unterminated", "t.vhd").is_error());
        EXPECT_TRUE(lex_string("\\unterminated", "t.vhd").is_error());
        EXPECT_TRUE(lex_string("X\"unterminated", "t.vhd").is_error());
        EXPECT_TRUE(lex_string("16#FF", "t.vhd").is_error());
        EXPECT_TRUE(lex_string("a $ b", "t.vhd").is_error());
        TEST_END
    }

    /**
     * Testing symbols, comments in every position, line endings, a byte order mark, and positions.
     */
    TEST_F(VHDLLexerTest, check_symbols_comments_positions)
    {
        TEST_START
        auto tokens = lex("\xEF\xBB\xBF-- head\r\n"
                          "port map ( I0 => a, -- trailing\r\n"
                          "  O <= b & c ); x := 1; /* block\r\n"
                          "  comment */ y /= z ** 2 <> <= >= = < >");
        EXPECT_EQ(texts(tokens),
                  (std::vector<std::string>{"port", "map", "(", "I0", "=>", "a", ",", "O", "<=", "b", "&", "c", ")", ";", "x", ":=", "1", ";", "y", "/=", "z", "**", "2", "<>", "<=", ">=", "=", "<", ">"}));
        EXPECT_EQ(tokens.at(0).line, 2);
        EXPECT_EQ(tokens.at(0).column, 1);
        EXPECT_EQ(tokens.at(7).line, 3);
        EXPECT_EQ(tokens.at(7).column, 3);
        EXPECT_EQ(tokens.at(18).line, 4);    // y after the block comment
        EXPECT_EQ(tokens.at(0).file_name(), "test.vhd");
        EXPECT_EQ(tokens.at(0).location(), "line 2, column 1");

        auto file = test_utils::create_sandbox_file("t.vhd", "entity e is end e;");
        auto res  = lex_file(file);
        ASSERT_TRUE(res.is_ok());
        EXPECT_EQ(res.get().size(), 7);    // six tokens and the end-of-file token

        NO_COUT_TEST_BLOCK;
        EXPECT_TRUE(lex_string("a /* never closed", "t.vhd").is_error());
        EXPECT_TRUE(lex_file("/nonexistent/file.vhd").is_error());
        TEST_END
    }
}    // namespace hal
