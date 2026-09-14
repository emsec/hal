#include "verilog_parser/verilog_lexer.h"

#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace verilog;

    class VerilogLexerTest : public ::testing::Test
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

        /**
         * Lex a string and return the tokens without the end-of-file token.
         */
        std::vector<Token> lex(const std::string& text)
        {
            auto res = lex_string(text, "test.v");
            if (res.is_error())
            {
                ADD_FAILURE() << res.get_error().get();
                return {};
            }
            auto tokens = res.get();
            tokens.pop_back();
            return tokens;
        }

        std::vector<std::string> texts(const std::vector<Token>& tokens)
        {
            std::vector<std::string> res;
            for (const Token& t : tokens)
            {
                res.push_back(t.text);
            }
            return res;
        }

        std::vector<TokenKind> kinds(const std::vector<Token>& tokens)
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
     * Testing identifiers: simple ones, escaped ones that end at whitespace and may contain anything, keywords that
     * are ordinary identifiers when escaped, and system identifiers.
     */
    TEST_F(VerilogLexerTest, check_identifiers)
    {
        TEST_START
        auto tokens = lex("wire a_1 $x \\bus[3] \\a(b,c) \\wire \\x//y \t\\p;q ;");
        ASSERT_EQ(tokens.size(), 9);
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"wire", "a_1", "$x", "bus[3]", "a(b,c)", "wire", "x//y", "p;q", ";"}));
        EXPECT_EQ(kinds(tokens),
                  (std::vector<TokenKind>{TokenKind::Identifier,
                                          TokenKind::Identifier,
                                          TokenKind::SystemIdentifier,
                                          TokenKind::EscapedIdentifier,
                                          TokenKind::EscapedIdentifier,
                                          TokenKind::EscapedIdentifier,
                                          TokenKind::EscapedIdentifier,
                                          TokenKind::EscapedIdentifier,
                                          TokenKind::Symbol}));
        EXPECT_TRUE(tokens.at(0).is("wire"));
        EXPECT_FALSE(tokens.at(5).is("wire"));    // escaped, so not the keyword
        EXPECT_TRUE(tokens.at(8).is_symbol(";"));

        // an escaped identifier immediately followed by a range: the space ends the identifier
        tokens = lex("\\^sum [1:0]");
        ASSERT_EQ(tokens.size(), 6);
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"^sum", "[", "1", ":", "0", "]"}));
        TEST_END
    }

    /**
     * Testing numbers in every form a netlist contains.
     */
    TEST_F(VerilogLexerTest, check_numbers)
    {
        TEST_START
        auto tokens = lex("12 1_000 4'b10_1x 8'hZZ 'habc 'd 2748 2'sb01 4 'b0101 'b1 '0 '1 'x 1.234 1e3 2.5E-2 16'bx 3'o7?");
        EXPECT_EQ(texts(tokens),
                  (std::vector<std::string>{"12", "1_000", "4'b10_1x", "8'hZZ", "'habc", "'d 2748", "2'sb01", "4'b0101", "'b1", "'0", "'1", "'x", "1.234", "1e3", "2.5E-2", "16'bx", "3'o7?"}));
        for (const Token& t : tokens)
        {
            EXPECT_EQ(t.kind, TokenKind::Number) << t.text;
        }

        // a number followed by something that is not a base: the apostrophe stays a symbol
        tokens = lex("a[3'd1] 1 'z");
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"a", "[", "3'd1", "]", "1", "'z"}));
        TEST_END
    }

    /**
     * Testing strings and their escapes, and that comment markers inside a string are text.
     */
    TEST_F(VerilogLexerTest, check_strings)
    {
        TEST_START
        auto tokens = lex("\"plain\" \"a//b/*c*/\" \"say \\\"hi\\\"\" \"tab\\tnew\\nline\" \"\"");
        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"plain", "a//b/*c*/", "say \"hi\"", "tab\tnew\nline", ""}));
        for (const Token& t : tokens)
        {
            EXPECT_EQ(t.kind, TokenKind::String);
        }

        NO_COUT_TEST_BLOCK;
        EXPECT_TRUE(lex_string("\"unterminated", "t.v").is_error());
        TEST_END
    }

    /**
     * Testing symbols: the two-character ones are one token, and one-character ones are split apart.
     */
    TEST_F(VerilogLexerTest, check_symbols)
    {
        TEST_START
        auto tokens = lex("(* *) #( ( ) [ ] { } , ; : . # = @ & | ^ ~ ! ? + - * / <= == a?b:c");
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"(*", "*)", "#(", "(", ")", "[", "]", "{", "}", ",",  ";",  ":", ".", "#", "=", "@", "&",
                                                           "|",  "^",  "~",  "!", "?", "+", "-", "*", "/", "<=", "==", "a", "?", "b", ":", "c"}));
        NO_COUT_TEST_BLOCK;
        EXPECT_TRUE(lex_string("a \x01 b", "t.v").is_error());
        TEST_END
    }

    /**
     * Testing comments in every position, CRLF line endings, and the line and column of tokens.
     */
    TEST_F(VerilogLexerTest, check_comments_and_positions)
    {
        TEST_START
        auto tokens = lex("/* head */ module top (a, /* between */ b); // trailing\r\n"
                          "  wire [1 /* inside a range */ :0] v; /* multi\r\n"
                          "  line */ BUF g (.I(a), .O(v[1]));\r\n"
                          "endmodule // last line without newline");
        EXPECT_EQ(texts(tokens), (std::vector<std::string>{"module", "top", "(", "a", ",", "b", ")", ";", "wire", "[", "1", ":", "0", "]", "v", ";", "BUF", "g",
                                                           "(",      ".",   "I", "(", "a", ")", ",", ".", "O",    "(", "v", "[", "1", "]", ")", ")", ";",   "endmodule"}));
        EXPECT_EQ(tokens.at(0).line, 1);
        EXPECT_EQ(tokens.at(0).column, 12);
        EXPECT_EQ(tokens.at(8).line, 2);
        EXPECT_EQ(tokens.at(8).column, 3);
        EXPECT_EQ(tokens.at(16).line, 3);    // BUF after the multi-line comment
        EXPECT_EQ(tokens.at(16).column, 11);
        EXPECT_EQ(tokens.back().line, 4);
        EXPECT_EQ(tokens.at(0).file_name(), "test.v");
        EXPECT_EQ(tokens.at(0).location(), "line 1, column 12");

        NO_COUT_TEST_BLOCK;
        EXPECT_TRUE(lex_string("a /* never closed", "t.v").is_error());
        TEST_END
    }

    /**
     * Testing the preprocessor: ignored directives, macros, conditional compilation in all forms, and the errors.
     */
    TEST_F(VerilogLexerTest, check_preprocessor)
    {
        TEST_START
        {
            auto tokens = lex("`timescale 1ns / 1ps\n"
                              "`default_nettype none\n"
                              "`celldefine\n"
                              "module m; endmodule\n"
                              "`endcelldefine\n"
                              "`resetall\n");
            EXPECT_EQ(texts(tokens), (std::vector<std::string>{"module", "m", ";", "endmodule"}));
            EXPECT_EQ(tokens.at(0).line, 4);
        }
        {
            auto tokens = lex("`define GATE INV\n"
                              "`define WIDE [3:0] \\\n"
                              "   w\n"
                              "`GATE g (.I(a), .O(`GATE));\n"
                              "wire `WIDE;\n"
                              "`undef GATE\n"
                              "\"`GATE stays\" // `GATE in a comment\n");
            EXPECT_TRUE(tokens.at(0).is("INV"));
            EXPECT_EQ(tokens.at(0).line, 4);
            EXPECT_EQ(texts(tokens),
                      (std::vector<std::string>{"INV", "g", "(", ".", "I", "(", "a", ")", ",", ".", "O", "(", "INV", ")", ")", ";", "wire", "[", "3", ":", "0", "]", "w", ";", "`GATE stays"}));
            EXPECT_EQ(tokens.at(16).line, 5);    // the continuation line keeps the numbering intact
        }
        {
            auto tokens = lex("`define A\n"
                              "`ifdef A\n"
                              "  yes_a\n"
                              "`else\n"
                              "  no_a\n"
                              "`endif\n"
                              "`ifdef B\n"
                              "  yes_b\n"
                              "`elsif A\n"
                              "  elsif_a\n"
                              "`else\n"
                              "  no_b\n"
                              "`endif\n"
                              "`ifndef B\n"
                              "  `ifdef A\n"
                              "    nested\n"
                              "  `endif\n"
                              "  `define LATE late\n"
                              "`endif\n"
                              "`LATE\n"
                              "`ifdef B\n"
                              "  `define NEVER 1\n"
                              "  `unknown_directive_is_fine_here\n"
                              "`endif\n"
                              "`ifdef NEVER\n"
                              "  never\n"
                              "`endif\n"
                              "end\n");
            EXPECT_EQ(texts(tokens), (std::vector<std::string>{"yes_a", "elsif_a", "nested", "late", "end"}));
            EXPECT_EQ(tokens.at(0).line, 3);
            EXPECT_EQ(tokens.at(1).line, 10);
            EXPECT_EQ(tokens.at(2).line, 16);
            EXPECT_EQ(tokens.at(3).line, 20);
            EXPECT_EQ(tokens.at(4).line, 28);
        }
        {
            // `include reads the file next to the including one; its tokens carry their own file and line
            auto included = test_utils::create_sandbox_file("inc.vh", "// header\nwire from_include;\n");
            auto main     = test_utils::create_sandbox_file("main.v", "`include \"inc.vh\"\nwire after;\n");
            auto res      = lex_file(main);
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            auto tokens = res.get();
            tokens.pop_back();
            EXPECT_EQ(texts(tokens), (std::vector<std::string>{"wire", "from_include", ";", "wire", "after", ";"}));
            EXPECT_EQ(tokens.at(0).line, 2);
            EXPECT_EQ(tokens.at(0).file_name(), included.string());
            EXPECT_EQ(tokens.at(3).line, 2);
            EXPECT_EQ(tokens.at(3).file_name(), main.string());
        }
        {
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(lex_string("`unknown_directive\n", "t.v").is_error());
            EXPECT_TRUE(lex_string("`ifdef A\nx\n", "t.v").is_error());
            EXPECT_TRUE(lex_string("`endif\n", "t.v").is_error());
            EXPECT_TRUE(lex_string("`else\n", "t.v").is_error());
            EXPECT_TRUE(lex_string("`define F(x) x\n", "t.v").is_error());
            EXPECT_TRUE(lex_string("`include \"does_not_exist.vh\"\n", "t.v").is_error());
            EXPECT_TRUE(lex_string("`ifdef\n`endif\n", "t.v").is_error());
            EXPECT_TRUE(lex_file("/nonexistent/file.v").is_error());
        }
        TEST_END
    }
}    // namespace hal
