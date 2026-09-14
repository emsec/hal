#include "vhdl_parser/vhdl_lexer.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>

namespace hal
{
    namespace vhdl
    {
        namespace
        {
            bool is_letter(char c)
            {
                return std::isalpha(static_cast<unsigned char>(c)) != 0;
            }

            bool is_identifier_char(char c)
            {
                return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
            }

            std::string lower(const std::string& s)
            {
                std::string r = s;
                for (char& c : r)
                {
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                return r;
            }

            // longest first
            const std::vector<std::string> SYMBOLS = {"=>", "<=", ":=", "**", "/=", ">=", "<>", "??", "?=", "(", ")", "[", "]", ",", ";", ":", ".", "&", "|", "'", "=", "<", ">", "+", "-", "*", "/", "@", "^"};

            /**
             * Whether a character literal may start here: not right after a name or a closing bracket, where an
             * apostrophe is an attribute tick (`a'length`, `x(1)'event`).
             */
            bool character_literal_allowed(const std::vector<Token>& tokens)
            {
                if (tokens.empty())
                {
                    return true;
                }
                const Token& p = tokens.back();
                if (p.kind == TokenKind::Identifier)
                {
                    // after a keyword that expects an operand, `'0'` is a literal: `x xor '0'`, `is '1'`
                    static const std::vector<std::string> OPERAND_KEYWORDS = {"and", "or", "xor", "nand", "nor", "xnor", "not", "abs", "mod", "rem", "is", "of", "when", "else", "then", "return", "to", "downto"};
                    const std::string l = lower(p.text);
                    for (const std::string& k : OPERAND_KEYWORDS)
                    {
                        if (l == k)
                        {
                            return true;
                        }
                    }
                    return false;
                }
                if (p.kind == TokenKind::ExtendedIdentifier)
                {
                    return false;
                }
                return !(p.kind == TokenKind::Symbol && (p.text == ")" || p.text == "]"));
            }

            /**
             * Whether the letters at `i` are the base of a bit string literal, like `X`, `sb`, `8x` before a `"`.
             */
            u32 bit_string_prefix_length(const std::string& s, u32 i)
            {
                u32 j = i;
                while (j < s.size() && std::isdigit(static_cast<unsigned char>(s[j])))
                {
                    j++;
                }
                if (j < s.size() && (s[j] == 'u' || s[j] == 'U' || s[j] == 's' || s[j] == 'S') && j + 1 < s.size() && std::strchr("bBoOxXdD", s[j + 1]) != nullptr)
                {
                    j++;
                }
                if (j < s.size() && std::strchr("bBoOxXdD", s[j]) != nullptr && j + 1 < s.size() && s[j + 1] == '"')
                {
                    return j + 1 - i;
                }
                return 0;
            }
        }    // namespace

        const std::string& Token::file_name() const
        {
            static const std::string none;
            return file ? *file : none;
        }

        bool Token::is(const char* keyword) const
        {
            return kind == TokenKind::Identifier && lower(text) == keyword;
        }

        bool Token::is_symbol(const char* symbol) const
        {
            return kind == TokenKind::Symbol && text == symbol;
        }

        bool Token::is_identifier() const
        {
            return kind == TokenKind::Identifier || kind == TokenKind::ExtendedIdentifier;
        }

        std::string Token::folded() const
        {
            return fold(text, kind == TokenKind::ExtendedIdentifier);
        }

        std::string Token::location() const
        {
            return "line " + std::to_string(line) + ", column " + std::to_string(column);
        }

        std::string fold(const std::string& name, bool extended)
        {
            return extended ? "\\" + name + "\\" : lower(name);
        }

        Result<std::vector<Token>> lex_string(const std::string& text, const std::filesystem::path& file)
        {
            std::vector<Token> tokens;
            const auto file_name = std::make_shared<const std::string>(file.string());
            const auto where     = [&](u32 line, u32 column) { return "line " + std::to_string(line) + ", column " + std::to_string(column) + " of '" + file.string() + "'"; };

            u32 i         = 0;
            u32 line      = 1;
            u32 line_start = 0;
            if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF && static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF)
            {
                i = 3;    // byte order mark
            }

            const auto column_of = [&](u32 pos) { return pos - line_start + 1; };

            while (i < text.size())
            {
                const char c = text[i];

                if (c == '\n')
                {
                    line++;
                    i++;
                    line_start = i;
                    continue;
                }
                if (std::isspace(static_cast<unsigned char>(c)))
                {
                    i++;
                    continue;
                }
                if (c == '-' && i + 1 < text.size() && text[i + 1] == '-')
                {
                    while (i < text.size() && text[i] != '\n')
                    {
                        i++;
                    }
                    continue;
                }
                if (c == '/' && i + 1 < text.size() && text[i + 1] == '*')
                {
                    const u32 start_line = line;
                    const u32 start_col  = column_of(i);
                    i += 2;
                    bool closed = false;
                    while (i < text.size())
                    {
                        if (text[i] == '*' && i + 1 < text.size() && text[i + 1] == '/')
                        {
                            i += 2;
                            closed = true;
                            break;
                        }
                        if (text[i] == '\n')
                        {
                            line++;
                            line_start = i + 1;
                        }
                        i++;
                    }
                    if (!closed)
                    {
                        return ERR("unterminated block comment at " + where(start_line, start_col));
                    }
                    continue;
                }

                Token t;
                t.line   = line;
                t.column = column_of(i);
                t.file   = file_name;

                if (c == '\\')
                {
                    // extended identifier, a doubled backslash inside stands for one
                    std::string name;
                    u32 j       = i + 1;
                    bool closed = false;
                    while (j < text.size() && text[j] != '\n')
                    {
                        if (text[j] == '\\')
                        {
                            if (j + 1 < text.size() && text[j + 1] == '\\')
                            {
                                name += '\\';
                                j += 2;
                                continue;
                            }
                            closed = true;
                            break;
                        }
                        name += text[j];
                        j++;
                    }
                    if (!closed || name.empty())
                    {
                        return ERR("unterminated or empty extended identifier at " + where(t.line, t.column));
                    }
                    t.kind = TokenKind::ExtendedIdentifier;
                    t.text = name;
                    i      = j + 1;
                }
                else if (c == '"')
                {
                    std::string value;
                    u32 j       = i + 1;
                    bool closed = false;
                    while (j < text.size() && text[j] != '\n')
                    {
                        if (text[j] == '"')
                        {
                            if (j + 1 < text.size() && text[j + 1] == '"')
                            {
                                value += '"';
                                j += 2;
                                continue;
                            }
                            closed = true;
                            break;
                        }
                        value += text[j];
                        j++;
                    }
                    if (!closed)
                    {
                        return ERR("unterminated string at " + where(t.line, t.column));
                    }
                    t.kind = TokenKind::String;
                    t.text = value;
                    i      = j + 1;
                }
                else if (c == '\'' && i + 2 < text.size() && text[i + 2] == '\'' && character_literal_allowed(tokens))
                {
                    t.kind = TokenKind::Character;
                    t.text = std::string(1, text[i + 1]);
                    i += 3;
                }
                else if (const u32 prefix = (std::isdigit(static_cast<unsigned char>(c)) || is_letter(c)) ? bit_string_prefix_length(text, i) : 0; prefix > 0)
                {
                    // bit string literal: the prefix, then a string
                    u32 j       = i + prefix + 1;
                    bool closed = false;
                    while (j < text.size() && text[j] != '\n')
                    {
                        if (text[j] == '"')
                        {
                            closed = true;
                            break;
                        }
                        j++;
                    }
                    if (!closed)
                    {
                        return ERR("unterminated bit string literal at " + where(t.line, t.column));
                    }
                    t.kind = TokenKind::BitString;
                    t.text = text.substr(i, j + 1 - i);
                    i      = j + 1;
                }
                else if (std::isdigit(static_cast<unsigned char>(c)))
                {
                    u32 j = i;
                    while (j < text.size() && (std::isdigit(static_cast<unsigned char>(text[j])) || text[j] == '_'))
                    {
                        j++;
                    }
                    if (j < text.size() && text[j] == '#')
                    {
                        // based literal 16#FF.8#E2
                        j++;
                        while (j < text.size() && (std::isxdigit(static_cast<unsigned char>(text[j])) || text[j] == '_' || text[j] == '.'))
                        {
                            j++;
                        }
                        if (j >= text.size() || text[j] != '#')
                        {
                            return ERR("unterminated based literal at " + where(t.line, t.column));
                        }
                        j++;
                    }
                    else if (j + 1 < text.size() && text[j] == '.' && std::isdigit(static_cast<unsigned char>(text[j + 1])))
                    {
                        j++;
                        while (j < text.size() && (std::isdigit(static_cast<unsigned char>(text[j])) || text[j] == '_'))
                        {
                            j++;
                        }
                    }
                    if (j < text.size() && (text[j] == 'e' || text[j] == 'E'))
                    {
                        u32 e = j + 1;
                        if (e < text.size() && (text[e] == '+' || text[e] == '-'))
                        {
                            e++;
                        }
                        if (e < text.size() && std::isdigit(static_cast<unsigned char>(text[e])))
                        {
                            while (e < text.size() && std::isdigit(static_cast<unsigned char>(text[e])))
                            {
                                e++;
                            }
                            j = e;
                        }
                    }
                    t.kind = TokenKind::Number;
                    t.text = text.substr(i, j - i);
                    i      = j;
                }
                else if (is_letter(c))
                {
                    u32 j = i + 1;
                    while (j < text.size() && is_identifier_char(text[j]))
                    {
                        j++;
                    }
                    t.kind = TokenKind::Identifier;
                    t.text = text.substr(i, j - i);
                    i      = j;
                }
                else
                {
                    bool matched = false;
                    for (const std::string& sym : SYMBOLS)
                    {
                        if (text.compare(i, sym.size(), sym) == 0)
                        {
                            t.kind = TokenKind::Symbol;
                            t.text = sym;
                            i += static_cast<u32>(sym.size());
                            matched = true;
                            break;
                        }
                    }
                    if (!matched)
                    {
                        return ERR(std::string("unexpected character '") + c + "' at " + where(t.line, t.column));
                    }
                }

                tokens.push_back(t);
            }

            Token eof;
            eof.kind = TokenKind::EndOfFile;
            eof.line = line;
            eof.file = file_name;
            tokens.push_back(eof);
            return OK(tokens);
        }

        Result<std::vector<Token>> lex_file(const std::filesystem::path& file)
        {
            std::ifstream stream(file);
            if (!stream)
            {
                return ERR("could not open '" + file.string() + "'");
            }
            std::stringstream buffer;
            buffer << stream.rdbuf();
            return lex_string(buffer.str(), file);
        }
    }    // namespace vhdl
}    // namespace hal
