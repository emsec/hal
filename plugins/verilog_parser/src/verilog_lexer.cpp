#include "verilog_parser/verilog_lexer.h"

#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace hal
{
    namespace verilog
    {
        // ---------------------------------------------------------------------------------------------------------
        // preprocessor
        // ---------------------------------------------------------------------------------------------------------

        namespace
        {
            bool is_identifier_start(char c)
            {
                return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
            }

            bool is_identifier_char(char c)
            {
                return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$';
            }

            std::string trim(const std::string& s)
            {
                const auto begin = s.find_first_not_of(" \t\r");
                if (begin == std::string::npos)
                {
                    return "";
                }
                const auto end = s.find_last_not_of(" \t\r");
                return s.substr(begin, end - begin + 1);
            }

            struct Conditional
            {
                bool parent_active;    // whether the enclosing region is active
                bool taken;            // whether any branch of this conditional has been taken
                bool active;           // whether the current branch is active
            };

            class Preprocessor
            {
            public:
                Preprocessor(std::vector<Line>& out) : m_out(out)
                {
                }

                Result<std::monostate> run(const std::string& text, const std::filesystem::path& file, u32 depth)
                {
                    if (depth > 32)
                    {
                        return ERR("`include nested deeper than 32 levels at '" + file.string() + "'");
                    }

                    std::istringstream in(text);
                    std::string raw;
                    u32 line_number       = 0;
                    bool in_block_comment = false;

                    while (std::getline(in, raw))
                    {
                        line_number++;
                        if (!raw.empty() && raw.back() == '\r')
                        {
                            raw.pop_back();
                        }

                        // a `define body continues on the next line after a trailing backslash
                        std::string line = raw;
                        if (!in_block_comment && trim(line).rfind("`define", 0) == 0)
                        {
                            u32 consumed = 0;
                            while (!line.empty() && line.back() == '\\')
                            {
                                line.pop_back();
                                std::string next;
                                if (!std::getline(in, next))
                                {
                                    break;
                                }
                                if (!next.empty() && next.back() == '\r')
                                {
                                    next.pop_back();
                                }
                                line += " " + next;
                                consumed++;
                            }
                            auto res = handle_line(line, line_number, file, depth, in_block_comment);
                            if (res.is_error())
                            {
                                return res;
                            }
                            for (u32 i = 0; i < consumed; i++)
                            {
                                line_number++;
                                m_out.push_back({"", line_number, file.string()});
                            }
                            continue;
                        }

                        auto res = handle_line(line, line_number, file, depth, in_block_comment);
                        if (res.is_error())
                        {
                            return res;
                        }
                    }

                    return OK({});
                }

                Result<std::monostate> finish(const std::filesystem::path& file)
                {
                    if (!m_conditionals.empty())
                    {
                        return ERR("`ifdef without matching `endif in '" + file.string() + "'");
                    }
                    return OK({});
                }

            private:
                std::vector<Line>& m_out;
                std::map<std::string, std::string> m_macros;
                std::vector<Conditional> m_conditionals;

                bool active() const
                {
                    return m_conditionals.empty() || m_conditionals.back().active;
                }

                static std::string where(const std::filesystem::path& file, u32 line)
                {
                    return "'" + file.string() + "' line " + std::to_string(line);
                }

                /**
                 * Expand macros and evaluate directives in one line; comments and strings are copied through.
                 */
                Result<std::monostate> handle_line(const std::string& line, u32 line_number, const std::filesystem::path& file, u32 depth, bool& in_block_comment)
                {
                    std::string out;
                    bool emit = active();
                    u32 i     = 0;

                    while (i < line.size())
                    {
                        const char c = line[i];

                        if (in_block_comment)
                        {
                            if (c == '*' && i + 1 < line.size() && line[i + 1] == '/')
                            {
                                in_block_comment = false;
                                if (emit)
                                {
                                    out += "*/";
                                }
                                i += 2;
                                continue;
                            }
                            if (emit)
                            {
                                out += c;
                            }
                            i++;
                            continue;
                        }

                        if (c == '/' && i + 1 < line.size() && line[i + 1] == '/')
                        {
                            if (emit)
                            {
                                out += line.substr(i);
                            }
                            break;
                        }
                        if (c == '/' && i + 1 < line.size() && line[i + 1] == '*')
                        {
                            in_block_comment = true;
                            if (emit)
                            {
                                out += "/*";
                            }
                            i += 2;
                            continue;
                        }
                        if (c == '"')
                        {
                            // copy the string through, escapes included
                            u32 j = i + 1;
                            while (j < line.size() && line[j] != '"')
                            {
                                if (line[j] == '\\' && j + 1 < line.size())
                                {
                                    j++;
                                }
                                j++;
                            }
                            if (emit)
                            {
                                out += line.substr(i, j - i + 1);
                            }
                            i = j + 1;
                            continue;
                        }
                        if (c == '\\')
                        {
                            // an escaped identifier runs to the next whitespace and may contain a backtick
                            u32 j = i + 1;
                            while (j < line.size() && !std::isspace(static_cast<unsigned char>(line[j])))
                            {
                                j++;
                            }
                            if (emit)
                            {
                                out += line.substr(i, j - i);
                            }
                            i = j;
                            continue;
                        }
                        if (c != '`')
                        {
                            if (emit)
                            {
                                out += c;
                            }
                            i++;
                            continue;
                        }

                        // a directive or a macro use
                        u32 j = i + 1;
                        while (j < line.size() && is_identifier_char(line[j]))
                        {
                            j++;
                        }
                        const std::string name = line.substr(i + 1, j - i - 1);
                        if (name.empty())
                        {
                            return ERR("stray '`' at " + where(file, line_number));
                        }
                        std::string rest = line.substr(j);
                        i                = static_cast<u32>(line.size());    // directives consume the rest of the line unless stated otherwise

                        if (name == "ifdef" || name == "ifndef")
                        {
                            std::string symbol = trim(rest);
                            const auto sp      = symbol.find_first_of(" \t");
                            if (sp != std::string::npos)
                            {
                                symbol = symbol.substr(0, sp);
                            }
                            if (symbol.empty())
                            {
                                return ERR("`" + name + " without a symbol at " + where(file, line_number));
                            }
                            const bool defined = m_macros.find(symbol) != m_macros.end();
                            const bool take    = (name == "ifdef") == defined;
                            m_conditionals.push_back({active(), take && active(), take && active()});
                            emit = active();
                        }
                        else if (name == "elsif")
                        {
                            if (m_conditionals.empty())
                            {
                                return ERR("`elsif without `ifdef at " + where(file, line_number));
                            }
                            std::string symbol = trim(rest);
                            const auto sp      = symbol.find_first_of(" \t");
                            if (sp != std::string::npos)
                            {
                                symbol = symbol.substr(0, sp);
                            }
                            Conditional& cond = m_conditionals.back();
                            const bool take   = cond.parent_active && !cond.taken && m_macros.find(symbol) != m_macros.end();
                            cond.active       = take;
                            cond.taken        = cond.taken || take;
                            emit              = active();
                        }
                        else if (name == "else")
                        {
                            if (m_conditionals.empty())
                            {
                                return ERR("`else without `ifdef at " + where(file, line_number));
                            }
                            Conditional& cond = m_conditionals.back();
                            cond.active       = cond.parent_active && !cond.taken;
                            cond.taken        = true;
                            emit              = active();
                        }
                        else if (name == "endif")
                        {
                            if (m_conditionals.empty())
                            {
                                return ERR("`endif without `ifdef at " + where(file, line_number));
                            }
                            m_conditionals.pop_back();
                            emit = active();
                        }
                        else if (!active())
                        {
                            // inside an excluded region only the conditionals matter
                        }
                        else if (name == "define")
                        {
                            std::string body = trim(rest);
                            u32 k            = 0;
                            while (k < body.size() && is_identifier_char(body[k]))
                            {
                                k++;
                            }
                            const std::string macro = body.substr(0, k);
                            if (macro.empty() || !is_identifier_start(macro[0]))
                            {
                                return ERR("`define without a name at " + where(file, line_number));
                            }
                            if (k < body.size() && body[k] == '(')
                            {
                                return ERR("`define " + macro + " at " + where(file, line_number) + " takes arguments, which is not supported");
                            }
                            m_macros[macro] = trim(body.substr(k));
                        }
                        else if (name == "undef")
                        {
                            m_macros.erase(trim(rest));
                        }
                        else if (name == "include")
                        {
                            std::string arg = trim(rest);
                            if (arg.size() < 2 || arg.front() != '"' || arg.find('"', 1) == std::string::npos)
                            {
                                return ERR("`include without a quoted file name at " + where(file, line_number));
                            }
                            const std::filesystem::path included = file.parent_path() / arg.substr(1, arg.find('"', 1) - 1);
                            std::ifstream stream(included);
                            if (!stream)
                            {
                                return ERR("`include at " + where(file, line_number) + ": cannot open '" + included.string() + "'");
                            }
                            std::stringstream buffer;
                            buffer << stream.rdbuf();
                            // the included lines carry their own file and line numbers; this line stays empty
                            m_out.push_back({out, line_number, file.string()});
                            out.clear();
                            if (auto res = run(buffer.str(), included, depth + 1); res.is_error())
                            {
                                return res;
                            }
                            return OK({});
                        }
                        else if (name == "timescale" || name == "celldefine" || name == "endcelldefine" || name == "default_nettype" || name == "resetall" || name == "line" || name == "begin_keywords"
                                 || name == "end_keywords" || name == "unconnected_drive" || name == "nounconnected_drive")
                        {
                            // carries nothing structural
                        }
                        else if (const auto it = m_macros.find(name); it != m_macros.end())
                        {
                            // a macro use: substitute and go on with the rest of the line
                            out += it->second;
                            i = j;
                        }
                        else
                        {
                            return ERR("unknown directive or macro `" + name + " at " + where(file, line_number));
                        }
                    }

                    m_out.push_back({out, line_number, file.string()});
                    return OK({});
                }
            };
        }    // namespace

        Result<std::vector<Line>> preprocess(const std::string& text, const std::filesystem::path& file)
        {
            std::vector<Line> lines;
            Preprocessor pp(lines);
            if (auto res = pp.run(text, file, 0); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not preprocess '" + file.string() + "'");
            }
            if (auto res = pp.finish(file); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not preprocess '" + file.string() + "'");
            }
            return OK(lines);
        }

        // ---------------------------------------------------------------------------------------------------------
        // tokens
        // ---------------------------------------------------------------------------------------------------------

        const std::string& Token::file_name() const
        {
            static const std::string none;
            return file ? *file : none;
        }

        bool Token::is(const char* identifier) const
        {
            return kind == TokenKind::Identifier && text == identifier;
        }

        bool Token::is_symbol(const char* symbol) const
        {
            return kind == TokenKind::Symbol && text == symbol;
        }

        std::string Token::location() const
        {
            return "line " + std::to_string(line) + ", column " + std::to_string(column);
        }

        namespace
        {
            // longest first, so that the scan below can take the first match
            const std::vector<std::string> SYMBOLS = {"(*", "*)", "#(", "<=", ">=", "==", "!=", "&&", "||", "<<", ">>", "->", "**", "(", ")", "[", "]", "{", "}", ",",
                                                      ";",  ":",  ".",  "#",  "=",  "@",  "+",  "-",  "*",  "/",  "%",  "&",  "|",  "^", "~", "!", "?", "<", ">", "'"};

            bool is_base_char(char c)
            {
                return c == 'b' || c == 'B' || c == 'o' || c == 'O' || c == 'd' || c == 'D' || c == 'h' || c == 'H';
            }

            bool is_number_value_char(char c)
            {
                return std::isxdigit(static_cast<unsigned char>(c)) || c == 'x' || c == 'X' || c == 'z' || c == 'Z' || c == '?' || c == '_';
            }

            /**
             * Read the part of a number that starts at the apostrophe: `'b1010`, `'sh_ff`, `'0`, `'x`.
             *
             * @returns The number of characters consumed, 0 if there is no such part.
             */
            u32 scan_based_part(const std::string& s, u32 i)
            {
                if (i >= s.size() || s[i] != '\'')
                {
                    return 0;
                }
                u32 j = i + 1;
                if (j < s.size() && (s[j] == 's' || s[j] == 'S'))
                {
                    j++;
                }
                if (j < s.size() && is_base_char(s[j]))
                {
                    j++;
                    while (j < s.size() && std::isspace(static_cast<unsigned char>(s[j])))
                    {
                        j++;    // Verilog allows whitespace between base and value
                    }
                    const u32 value_start = j;
                    while (j < s.size() && is_number_value_char(s[j]))
                    {
                        j++;
                    }
                    if (j == value_start)
                    {
                        return 0;
                    }
                    return j - i;
                }
                // SystemVerilog fill literals '0, '1, 'x, 'z
                if (j == i + 1 && j < s.size() && (s[j] == '0' || s[j] == '1' || s[j] == 'x' || s[j] == 'X' || s[j] == 'z' || s[j] == 'Z'))
                {
                    if (j + 1 < s.size() && is_identifier_char(s[j + 1]))
                    {
                        return 0;
                    }
                    return 2;
                }
                return 0;
            }
        }    // namespace

        Result<std::vector<Token>> tokenize(const std::vector<Line>& lines)
        {
            std::vector<Token> tokens;
            bool in_block_comment = false;
            u32 comment_line      = 0;
            std::map<std::string, std::shared_ptr<const std::string>> files;

            for (const Line& l : lines)
            {
                const std::string& s = l.text;
                u32 i                = 0;
                auto file_it         = files.find(l.file);
                if (file_it == files.end())
                {
                    file_it = files.emplace(l.file, std::make_shared<const std::string>(l.file)).first;
                }

                while (i < s.size())
                {
                    const char c = s[i];

                    if (in_block_comment)
                    {
                        const auto end = s.find("*/", i);
                        if (end == std::string::npos)
                        {
                            i = static_cast<u32>(s.size());
                        }
                        else
                        {
                            in_block_comment = false;
                            i                = static_cast<u32>(end + 2);
                        }
                        continue;
                    }
                    if (std::isspace(static_cast<unsigned char>(c)))
                    {
                        i++;
                        continue;
                    }
                    if (c == '/' && i + 1 < s.size() && s[i + 1] == '/')
                    {
                        break;
                    }
                    if (c == '/' && i + 1 < s.size() && s[i + 1] == '*')
                    {
                        in_block_comment = true;
                        comment_line     = l.line;
                        i += 2;
                        continue;
                    }

                    Token t;
                    t.line   = l.line;
                    t.column = i + 1;
                    t.file   = file_it->second;

                    if (c == '\\')
                    {
                        u32 j = i + 1;
                        while (j < s.size() && !std::isspace(static_cast<unsigned char>(s[j])))
                        {
                            j++;
                        }
                        if (j == i + 1)
                        {
                            return ERR("empty escaped identifier at line " + std::to_string(l.line) + ", column " + std::to_string(i + 1) + " of '" + l.file + "'");
                        }
                        t.kind = TokenKind::EscapedIdentifier;
                        t.text = s.substr(i + 1, j - i - 1);
                        i      = j;
                    }
                    else if (c == '"')
                    {
                        std::string value;
                        u32 j       = i + 1;
                        bool closed = false;
                        while (j < s.size())
                        {
                            if (s[j] == '\\' && j + 1 < s.size())
                            {
                                const char e = s[j + 1];
                                switch (e)
                                {
                                    case 'n':
                                        value += '\n';
                                        break;
                                    case 't':
                                        value += '\t';
                                        break;
                                    default:
                                        value += e;
                                        break;
                                }
                                j += 2;
                                continue;
                            }
                            if (s[j] == '"')
                            {
                                closed = true;
                                break;
                            }
                            value += s[j];
                            j++;
                        }
                        if (!closed)
                        {
                            return ERR("unterminated string at line " + std::to_string(l.line) + ", column " + std::to_string(i + 1) + " of '" + l.file + "'");
                        }
                        t.kind = TokenKind::String;
                        t.text = value;
                        i      = j + 1;
                    }
                    else if (std::isdigit(static_cast<unsigned char>(c)))
                    {
                        // decimal digits, then either a based part (with optional whitespace before the apostrophe),
                        // a fraction, an exponent, or nothing
                        u32 j = i;
                        while (j < s.size() && (std::isdigit(static_cast<unsigned char>(s[j])) || s[j] == '_'))
                        {
                            j++;
                        }
                        u32 k = j;
                        while (k < s.size() && (s[k] == ' ' || s[k] == '\t'))
                        {
                            k++;
                        }
                        // a fill literal such as '0 never takes a size, so only a real base is joined with the digits
                        const bool has_base = k + 1 < s.size() && s[k] == '\'' && (is_base_char(s[k + 1]) || ((s[k + 1] == 's' || s[k + 1] == 'S') && k + 2 < s.size() && is_base_char(s[k + 2])));
                        if (const u32 based = has_base ? scan_based_part(s, k) : 0; based > 0)
                        {
                            t.text = s.substr(i, j - i) + s.substr(k, based);
                            i      = k + based;
                        }
                        else
                        {
                            if (j + 1 < s.size() && s[j] == '.' && std::isdigit(static_cast<unsigned char>(s[j + 1])))
                            {
                                j++;
                                while (j < s.size() && (std::isdigit(static_cast<unsigned char>(s[j])) || s[j] == '_'))
                                {
                                    j++;
                                }
                            }
                            if (j < s.size() && (s[j] == 'e' || s[j] == 'E'))
                            {
                                u32 e = j + 1;
                                if (e < s.size() && (s[e] == '+' || s[e] == '-'))
                                {
                                    e++;
                                }
                                if (e < s.size() && std::isdigit(static_cast<unsigned char>(s[e])))
                                {
                                    while (e < s.size() && std::isdigit(static_cast<unsigned char>(s[e])))
                                    {
                                        e++;
                                    }
                                    j = e;
                                }
                            }
                            t.text = s.substr(i, j - i);
                            i      = j;
                        }
                        t.kind = TokenKind::Number;
                    }
                    else if (c == '\'' && scan_based_part(s, i) > 0)
                    {
                        const u32 n = scan_based_part(s, i);
                        t.kind      = TokenKind::Number;
                        t.text      = s.substr(i, n);
                        i += n;
                    }
                    else if (is_identifier_start(c) || (c == '$' && i + 1 < s.size() && is_identifier_start(s[i + 1])))
                    {
                        u32 j = i + 1;
                        while (j < s.size() && is_identifier_char(s[j]))
                        {
                            j++;
                        }
                        t.kind = c == '$' ? TokenKind::SystemIdentifier : TokenKind::Identifier;
                        t.text = s.substr(i, j - i);
                        i      = j;
                    }
                    else
                    {
                        bool matched = false;
                        for (const std::string& sym : SYMBOLS)
                        {
                            if (s.compare(i, sym.size(), sym) == 0)
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
                            return ERR(std::string("unexpected character '") + c + "' at line " + std::to_string(l.line) + ", column " + std::to_string(i + 1) + " of '" + l.file + "'");
                        }
                    }

                    tokens.push_back(t);
                }
            }

            if (in_block_comment)
            {
                return ERR("unterminated block comment starting at line " + std::to_string(comment_line));
            }

            Token eof;
            eof.kind = TokenKind::EndOfFile;
            if (!lines.empty())
            {
                eof.line = lines.back().line;
                eof.file = files.at(lines.back().file);
            }
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

        Result<std::vector<Token>> lex_string(const std::string& text, const std::filesystem::path& file)
        {
            auto lines = preprocess(text, file);
            if (lines.is_error())
            {
                return ERR(lines.get_error());
            }
            auto tokens = tokenize(lines.get());
            if (tokens.is_error())
            {
                return ERR_APPEND(tokens.get_error(), "could not tokenize '" + file.string() + "'");
            }
            return tokens;
        }
    }    // namespace verilog
}    // namespace hal
