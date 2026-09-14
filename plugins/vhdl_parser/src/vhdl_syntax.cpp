#include "vhdl_parser/vhdl_syntax.h"

#include "hal_core/utilities/log.h"

#include <set>

namespace hal
{
    namespace vhdl
    {
        std::string ast::Name::key() const
        {
            return fold(text, extended);
        }

        namespace
        {
            using namespace ast;

            const std::set<std::string> REJECTED_STATEMENTS = {"process", "block", "generate", "assert", "with", "postponed", "wait"};
            const std::set<std::string> REJECTED_DECLARATIONS = {"variable", "shared", "file", "alias", "group", "disconnect"};
            const std::set<std::string> SUBPROGRAM_STARTS    = {"function", "procedure", "impure", "pure"};
            const std::set<std::string> TIME_UNITS           = {"fs", "ps", "ns", "us", "ms", "sec", "min", "hr"};

            /**
             * Precedence of the binary operators, higher binds tighter.
             */
            int precedence(const Token& t)
            {
                if (t.kind == TokenKind::Identifier)
                {
                    if (t.is("and") || t.is("or") || t.is("xor") || t.is("nand") || t.is("nor") || t.is("xnor"))
                        return 1;
                    if (t.is("mod") || t.is("rem"))
                        return 5;
                    return 0;
                }
                if (t.kind != TokenKind::Symbol)
                {
                    return 0;
                }
                const std::string& s = t.text;
                if (s == "=" || s == "/=" || s == "<" || s == "<=" || s == ">" || s == ">=" || s == "?=")
                    return 2;
                if (s == "+" || s == "-" || s == "&")
                    return 4;
                if (s == "*" || s == "/")
                    return 5;
                if (s == "**")
                    return 6;
                return 0;
            }

            class Parser
            {
            public:
                Parser(const std::vector<Token>& tokens, const std::string& file) : m_tokens(tokens), m_file(file)
                {
                }

                Result<SourceFile> run()
                {
                    SourceFile out;
                    out.file = m_file;

                    while (!at_end())
                    {
                        const Token& t = peek();
                        Result<std::monostate> res = OK({});
                        if (t.is("library") || t.is("use"))
                        {
                            res = parse_context(out.context);
                        }
                        else if (t.is("entity"))
                        {
                            res = parse_entity(out);
                        }
                        else if (t.is("architecture"))
                        {
                            res = parse_architecture(out);
                        }
                        else if (t.is("package"))
                        {
                            res = parse_package(out);
                        }
                        else if (t.is("configuration"))
                        {
                            res = parse_configuration(out);
                        }
                        else if (t.is("context"))
                        {
                            res = error(t, "context declarations are not supported");
                        }
                        else
                        {
                            res = error(t, "expected 'library', 'use', 'entity', 'architecture', 'package' or 'configuration'");
                        }
                        if (res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                    }
                    return OK(out);
                }

            private:
                const std::vector<Token>& m_tokens;
                std::string m_file;
                u32 m_pos = 0;

                // ---- token access -------------------------------------------------------------------------------

                const Token& peek(u32 offset = 0) const
                {
                    const u32 i = m_pos + offset;
                    return i < m_tokens.size() ? m_tokens.at(i) : m_tokens.back();
                }

                bool at_end() const
                {
                    return peek().kind == TokenKind::EndOfFile;
                }

                const Token& take()
                {
                    const Token& t = peek();
                    if (m_pos < m_tokens.size() - 1)
                    {
                        m_pos++;
                    }
                    return t;
                }

                bool accept(const char* keyword)
                {
                    if (peek().is(keyword))
                    {
                        take();
                        return true;
                    }
                    return false;
                }

                bool accept_symbol(const char* symbol)
                {
                    if (peek().is_symbol(symbol))
                    {
                        take();
                        return true;
                    }
                    return false;
                }

                static Location location_of(const Token& t)
                {
                    return {t.line, t.column};
                }

                template<typename T>
                Result<T> error(const Token& t, const std::string& message) const
                {
                    if (t.kind == TokenKind::EndOfFile)
                    {
                        return ERR(message + " but reached the end of '" + m_file + "'");
                    }
                    return ERR(message + " at " + t.location() + " of '" + t.file_name() + "', found '" + t.text + "'");
                }

                Result<std::monostate> error(const Token& t, const std::string& message) const
                {
                    return error<std::monostate>(t, message);
                }

                Result<std::monostate> expect(const char* keyword)
                {
                    if (!accept(keyword))
                    {
                        return error(peek(), std::string("expected '") + keyword + "'");
                    }
                    return OK({});
                }

                Result<std::monostate> expect_symbol(const char* symbol)
                {
                    if (!accept_symbol(symbol))
                    {
                        return error(peek(), std::string("expected '") + symbol + "'");
                    }
                    return OK({});
                }

                static Name name_of(const Token& t)
                {
                    Name n;
                    n.text     = t.text;
                    n.extended = t.kind == TokenKind::ExtendedIdentifier;
                    return n;
                }

                Result<Name> expect_name(const char* what)
                {
                    if (!peek().is_identifier())
                    {
                        return error<Name>(peek(), std::string("expected ") + what);
                    }
                    return OK(name_of(take()));
                }

                /**
                 * `end [keyword] [name];` with the name optional and unchecked.
                 */
                Result<std::monostate> expect_end(const char* keyword, const Name& name)
                {
                    if (auto res = expect("end"); res.is_error())
                    {
                        return res;
                    }
                    accept(keyword);
                    if (peek().is_identifier())
                    {
                        const Token& t   = peek();
                        const Name given = name_of(take());
                        if (!(given == name))
                        {
                            log_warning("vhdl_parser", "'end {} {}' does not match the name '{}' at {} of '{}'", keyword, given.text, name.text, t.location(), t.file_name());
                        }
                    }
                    return expect_symbol(";");
                }

                /**
                 * Skip to the end of the current statement, including the `;`.
                 */
                void skip_statement()
                {
                    while (!at_end() && !peek().is_symbol(";"))
                    {
                        take();
                    }
                    accept_symbol(";");
                }

                // ---- names and expressions --------------------------------------------------------------------

                /**
                 * A selected name `a.b.c`: the parts.
                 */
                Result<std::vector<Name>> parse_selected_name(const char* what)
                {
                    std::vector<Name> parts;
                    do
                    {
                        if (peek().is("all"))
                        {
                            parts.push_back(name_of(take()));
                            break;
                        }
                        auto n = expect_name(what);
                        if (n.is_error())
                        {
                            return ERR(n.get_error());
                        }
                        parts.push_back(n.get());
                    } while (accept_symbol("."));
                    return OK(parts);
                }

                Result<Expr> parse_expression()
                {
                    return parse_binary(0);
                }

                Result<Expr> parse_binary(int min_precedence)
                {
                    auto lhs = parse_unary();
                    if (lhs.is_error())
                    {
                        return lhs;
                    }
                    Expr left = lhs.get();
                    while (precedence(peek()) > min_precedence)
                    {
                        const Token& op = take();
                        auto rhs        = parse_binary(precedence(op));
                        if (rhs.is_error())
                        {
                            return rhs;
                        }
                        // concatenations are flattened into one node, which is what a port map needs
                        if (op.is_symbol("&") && left.kind == Expr::Kind::Concat)
                        {
                            left.children.push_back(rhs.get());
                            continue;
                        }
                        Expr bin;
                        bin.kind     = op.is_symbol("&") ? Expr::Kind::Concat : Expr::Kind::Binary;
                        bin.text     = op.kind == TokenKind::Identifier ? fold(op.text) : op.text;
                        bin.location = left.location;
                        bin.children.push_back(std::move(left));
                        bin.children.push_back(rhs.get());
                        left = std::move(bin);
                    }
                    return OK(left);
                }

                Result<Expr> parse_unary()
                {
                    const Token& t = peek();
                    if (t.is("not") || t.is("abs") || t.is_symbol("-") || t.is_symbol("+"))
                    {
                        take();
                        auto operand = parse_unary();
                        if (operand.is_error())
                        {
                            return operand;
                        }
                        Expr u;
                        u.kind     = Expr::Kind::Unary;
                        u.text     = t.kind == TokenKind::Identifier ? fold(t.text) : t.text;
                        u.location = location_of(t);
                        u.children.push_back(operand.get());
                        return OK(u);
                    }
                    return parse_primary();
                }

                Result<Expr> parse_primary()
                {
                    const Token& t = peek();
                    Expr e;
                    e.location = location_of(t);

                    switch (t.kind)
                    {
                        case TokenKind::Number:
                            take();
                            e.kind = Expr::Kind::Number;
                            e.text = t.text;
                            if (peek().kind == TokenKind::Identifier && TIME_UNITS.count(fold(peek().text)) > 0)
                            {
                                e.text += " " + fold(take().text);    // a physical literal such as `1 ns`
                            }
                            return OK(e);
                        case TokenKind::Character:
                            take();
                            e.kind = Expr::Kind::Character;
                            e.text = t.text;
                            return OK(e);
                        case TokenKind::String:
                            take();
                            e.kind = Expr::Kind::String;
                            e.text = t.text;
                            return OK(e);
                        case TokenKind::BitString:
                            take();
                            e.kind = Expr::Kind::BitString;
                            e.text = t.text;
                            return OK(e);
                        default:
                            break;
                    }

                    if (t.is("open"))
                    {
                        take();
                        e.kind = Expr::Kind::Open;
                        return OK(e);
                    }
                    if (t.is("others"))
                    {
                        take();
                        e.kind = Expr::Kind::Others;
                        return OK(e);
                    }
                    if (t.is_symbol("("))
                    {
                        return parse_parenthesized();
                    }
                    if (t.is_identifier())
                    {
                        auto parts = parse_selected_name("a name");
                        if (parts.is_error())
                        {
                            return ERR(parts.get_error());
                        }
                        e.kind   = Expr::Kind::Identifier;
                        e.name   = parts.get().back();
                        e.prefix = parts.get();
                        e.prefix.pop_back();
                        return parse_suffixes(std::move(e));
                    }
                    return error<Expr>(t, "expected an expression");
                }

                /**
                 * `( ... )`: an aggregate, or a parenthesized expression when it holds one positional element.
                 */
                Result<Expr> parse_parenthesized()
                {
                    Expr agg;
                    agg.location = location_of(peek());
                    agg.kind     = Expr::Kind::Aggregate;
                    take();    // (
                    bool any_choice = false;
                    do
                    {
                        auto elem = parse_aggregate_element();
                        if (elem.is_error())
                        {
                            return elem;
                        }
                        any_choice = any_choice || !elem.get().choices.empty();
                        agg.children.push_back(elem.get());
                    } while (accept_symbol(","));
                    if (auto res = expect_symbol(")"); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    if (agg.children.size() == 1 && !any_choice)
                    {
                        return OK(agg.children.front());
                    }
                    return OK(agg);
                }

                /**
                 * `[choice {| choice} =>] expression`; a choice may be a range.
                 */
                Result<Expr> parse_aggregate_element()
                {
                    auto first = parse_expression_or_range();
                    if (first.is_error())
                    {
                        return first;
                    }
                    if (!peek().is_symbol("=>") && !peek().is_symbol("|"))
                    {
                        return first;
                    }
                    std::vector<Expr> choices{first.get()};
                    while (accept_symbol("|"))
                    {
                        auto c = parse_expression_or_range();
                        if (c.is_error())
                        {
                            return c;
                        }
                        choices.push_back(c.get());
                    }
                    if (auto res = expect_symbol("=>"); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    auto value = parse_expression();
                    if (value.is_error())
                    {
                        return value;
                    }
                    Expr elem    = value.get();
                    elem.choices = std::move(choices);
                    return OK(elem);
                }

                /**
                 * An expression, or `l to r` / `l downto r` as a slice with an empty base.
                 */
                Result<Expr> parse_expression_or_range()
                {
                    auto left = parse_expression();
                    if (left.is_error())
                    {
                        return left;
                    }
                    if (peek().is("to") || peek().is("downto"))
                    {
                        Expr range;
                        range.kind     = Expr::Kind::Slice;
                        range.text     = fold(take().text);
                        range.location = left.get().location;
                        auto right     = parse_expression();
                        if (right.is_error())
                        {
                            return right;
                        }
                        Expr base;
                        range.children.push_back(base);
                        range.children.push_back(left.get());
                        range.children.push_back(right.get());
                        return OK(range);
                    }
                    return left;
                }

                /**
                 * Indexing, slicing and attributes after a name: `a(1)`, `a(3 downto 0)`, `a(1, 2)`, `a'length`.
                 */
                Result<Expr> parse_suffixes(Expr base)
                {
                    while (true)
                    {
                        if (peek().is_symbol("("))
                        {
                            const Location loc = location_of(peek());
                            take();
                            std::vector<Expr> args;
                            do
                            {
                                auto a = parse_expression_or_range();
                                if (a.is_error())
                                {
                                    return a;
                                }
                                args.push_back(a.get());
                            } while (accept_symbol(","));
                            if (auto res = expect_symbol(")"); res.is_error())
                            {
                                return ERR(res.get_error());
                            }
                            Expr suffix;
                            suffix.location = loc;
                            if (args.size() == 1 && args.front().kind == Expr::Kind::Slice && args.front().children.front().is_empty())
                            {
                                suffix.kind = Expr::Kind::Slice;
                                suffix.text = args.front().text;
                                suffix.children.push_back(std::move(base));
                                suffix.children.push_back(args.front().children.at(1));
                                suffix.children.push_back(args.front().children.at(2));
                            }
                            else
                            {
                                suffix.kind = Expr::Kind::Index;
                                suffix.children.push_back(std::move(base));
                                for (Expr& a : args)
                                {
                                    suffix.children.push_back(std::move(a));
                                }
                            }
                            base = std::move(suffix);
                        }
                        else if (peek().is_symbol("'") && peek(1).is_identifier())
                        {
                            take();
                            Expr attr;
                            attr.kind     = Expr::Kind::Attribute;
                            attr.text     = fold(take().text);
                            attr.location = base.location;
                            attr.children.push_back(std::move(base));
                            base = std::move(attr);
                        }
                        else
                        {
                            return OK(base);
                        }
                    }
                }

                Result<Range> parse_range()
                {
                    Range r;
                    auto left = parse_expression();
                    if (left.is_error())
                    {
                        return ERR(left.get_error());
                    }
                    r.left = left.get();
                    if (accept("downto"))
                    {
                        r.descending = true;
                    }
                    else if (!accept("to"))
                    {
                        return error<Range>(peek(), "expected 'to' or 'downto'");
                    }
                    auto right = parse_expression();
                    if (right.is_error())
                    {
                        return ERR(right.get_error());
                    }
                    r.right = right.get();
                    return OK(r);
                }

                /**
                 * `std_logic`, `std_logic_vector(7 downto 0)`, `work.t_bus`, `std_logic_vector2(0 to 1, 2 to 3)`,
                 * `integer range 0 to 7`.
                 */
                Result<TypeMark> parse_type_mark()
                {
                    TypeMark tm;
                    tm.location = location_of(peek());
                    auto parts  = parse_selected_name("a type name");
                    if (parts.is_error())
                    {
                        return ERR(parts.get_error());
                    }
                    tm.name   = parts.get().back();
                    tm.prefix = parts.get();
                    tm.prefix.pop_back();
                    if (accept_symbol("("))
                    {
                        do
                        {
                            auto r = parse_range();
                            if (r.is_error())
                            {
                                return ERR(r.get_error());
                            }
                            tm.constraints.push_back(r.get());
                        } while (accept_symbol(","));
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                    }
                    else if (accept("range"))
                    {
                        // `integer range 0 to 7`: the constraint says nothing about bits, skip it
                        auto r = parse_range();
                        if (r.is_error())
                        {
                            return ERR(r.get_error());
                        }
                    }
                    return OK(tm);
                }

                // ---- interface lists --------------------------------------------------------------------------

                /**
                 * `( name, name : [mode] type [:= default]; ... )`
                 */
                Result<std::vector<InterfaceDecl>> parse_interface_list(bool ports)
                {
                    std::vector<InterfaceDecl> out;
                    if (auto res = expect_symbol("("); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    while (!peek().is_symbol(")"))
                    {
                        accept("signal");
                        accept("constant");
                        std::vector<Name> names;
                        std::vector<Location> locations;
                        do
                        {
                            locations.push_back(location_of(peek()));
                            auto n = expect_name(ports ? "a port name" : "a generic name");
                            if (n.is_error())
                            {
                                return ERR(n.get_error());
                            }
                            names.push_back(n.get());
                        } while (accept_symbol(","));
                        if (auto res = expect_symbol(":"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        PinDirection mode = PinDirection::none;
                        if (ports)
                        {
                            if (accept("in"))
                                mode = PinDirection::input;
                            else if (accept("out") || accept("buffer"))
                                mode = PinDirection::output;
                            else if (accept("inout") || accept("linkage"))
                                mode = PinDirection::inout;
                            else if (peek().is_identifier() && !peek(1).is_symbol(":") && (peek(1).is_symbol(";") || peek(1).is_symbol(")") || peek(1).is_symbol("(") || peek(1).is_symbol(":=") || peek(1).is("range")
                                                                                          || peek(1).is_symbol(".")))
                                mode = PinDirection::input;    // no mode given: `in` is the default
                            else
                                return error<std::vector<InterfaceDecl>>(peek(), "expected a port mode 'in', 'out', 'inout', 'buffer' or 'linkage'");
                        }
                        auto tm = parse_type_mark();
                        if (tm.is_error())
                        {
                            return ERR(tm.get_error());
                        }
                        const TypeMark type = tm.get();
                        std::optional<Expr> def;
                        if (accept_symbol(":="))
                        {
                            auto d = parse_expression();
                            if (d.is_error())
                            {
                                return ERR(d.get_error());
                            }
                            def = d.get();
                        }
                        for (u32 i = 0; i < names.size(); i++)
                        {
                            InterfaceDecl d;
                            d.name          = names.at(i);
                            d.mode          = mode;
                            d.type          = type;
                            d.default_value = def;
                            d.location      = locations.at(i);
                            out.push_back(std::move(d));
                        }
                        if (!accept_symbol(";"))
                        {
                            break;    // the last item has no ';'
                        }
                    }
                    if (auto res = expect_symbol(")"); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    return OK(out);
                }

                /**
                 * `[generic (...);] [port (...);]`
                 */
                Result<std::monostate> parse_header(std::vector<InterfaceDecl>& generics, std::vector<InterfaceDecl>& ports)
                {
                    if (accept("generic"))
                    {
                        auto g = parse_interface_list(false);
                        if (g.is_error())
                        {
                            return ERR(g.get_error());
                        }
                        generics = g.get();
                        if (auto res = expect_symbol(";"); res.is_error())
                        {
                            return res;
                        }
                    }
                    if (accept("port"))
                    {
                        auto p = parse_interface_list(true);
                        if (p.is_error())
                        {
                            return ERR(p.get_error());
                        }
                        ports = p.get();
                        if (auto res = expect_symbol(";"); res.is_error())
                        {
                            return res;
                        }
                    }
                    return OK({});
                }

                // ---- declarations -----------------------------------------------------------------------------

                /**
                 * The declarative part of an entity, architecture or package, up to `begin` or `end`.
                 */
                Result<std::monostate> parse_declarations(Declarations& out)
                {
                    while (!peek().is("begin") && !peek().is("end") && !peek().is("generic") && !peek().is("port") && !at_end())
                    {
                        const Token& t = peek();
                        Result<std::monostate> res = OK({});
                        if (t.is("signal") || t.is("constant"))
                        {
                            res = parse_object_declaration(out);
                        }
                        else if (t.is("component"))
                        {
                            res = parse_component(out);
                        }
                        else if (t.is("type"))
                        {
                            res = parse_type_declaration(out);
                        }
                        else if (t.is("subtype"))
                        {
                            res = parse_subtype_declaration(out);
                        }
                        else if (t.is("attribute"))
                        {
                            res = parse_attribute(out);
                        }
                        else if (t.is("use"))
                        {
                            skip_statement();
                        }
                        else if (t.is("for"))
                        {
                            skip_statement();    // a configuration specification inside an architecture
                        }
                        else if (t.kind == TokenKind::Identifier && SUBPROGRAM_STARTS.count(fold(t.text)) > 0)
                        {
                            skip_subprogram();    // never called by a structural netlist
                        }
                        else if (t.kind == TokenKind::Identifier && REJECTED_DECLARATIONS.count(fold(t.text)) > 0)
                        {
                            return error(t, "'" + t.text + "' is not part of a structural netlist");
                        }
                        else
                        {
                            return error(t, "expected a declaration");
                        }
                        if (res.is_error())
                        {
                            return res;
                        }
                    }
                    return OK({});
                }

                Result<std::monostate> parse_object_declaration(Declarations& out)
                {
                    const bool is_constant = take().is("constant");
                    std::vector<Name> names;
                    std::vector<Location> locations;
                    do
                    {
                        locations.push_back(location_of(peek()));
                        auto n = expect_name(is_constant ? "a constant name" : "a signal name");
                        if (n.is_error())
                        {
                            return ERR(n.get_error());
                        }
                        names.push_back(n.get());
                    } while (accept_symbol(","));
                    if (auto res = expect_symbol(":"); res.is_error())
                    {
                        return res;
                    }
                    auto tm = parse_type_mark();
                    if (tm.is_error())
                    {
                        return ERR(tm.get_error());
                    }
                    const TypeMark type = tm.get();
                    accept("register");
                    accept("bus");
                    std::optional<Expr> init;
                    if (accept_symbol(":="))
                    {
                        auto i = parse_expression();
                        if (i.is_error())
                        {
                            return ERR(i.get_error());
                        }
                        init = i.get();
                    }
                    for (u32 i = 0; i < names.size(); i++)
                    {
                        ObjectDecl d;
                        d.name        = names.at(i);
                        d.is_constant = is_constant;
                        d.type        = type;
                        d.initializer = init;
                        d.location    = locations.at(i);
                        out.objects.push_back(std::move(d));
                    }
                    return expect_symbol(";");
                }

                Result<std::monostate> parse_component(Declarations& out)
                {
                    ComponentDecl c;
                    c.location = location_of(take());    // component
                    auto n     = expect_name("a component name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    c.name = n.get();
                    accept("is");
                    if (auto res = parse_header(c.generics, c.ports); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = expect_end("component", c.name); res.is_error())
                    {
                        return res;
                    }
                    out.components.push_back(std::move(c));
                    return OK({});
                }

                Result<std::monostate> parse_type_declaration(Declarations& out)
                {
                    TypeDecl d;
                    d.location = location_of(take());    // type
                    auto n     = expect_name("a type name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    d.name = n.get();
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    if (accept("array"))
                    {
                        d.is_array = true;
                        if (auto res = expect_symbol("("); res.is_error())
                        {
                            return res;
                        }
                        do
                        {
                            if (peek().is_identifier() && peek(1).is("range") && peek(2).is_symbol("<>"))
                            {
                                take();
                                take();
                                take();    // unconstrained: `natural range <>`
                                continue;
                            }
                            auto r = parse_range();
                            if (r.is_error())
                            {
                                return ERR(r.get_error());
                            }
                            d.ranges.push_back(r.get());
                        } while (accept_symbol(","));
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return res;
                        }
                        if (auto res = expect("of"); res.is_error())
                        {
                            return res;
                        }
                        auto tm = parse_type_mark();
                        if (tm.is_error())
                        {
                            return ERR(tm.get_error());
                        }
                        d.element = tm.get();
                        if (auto res = expect_symbol(";"); res.is_error())
                        {
                            return res;
                        }
                    }
                    else if (peek().is("record"))
                    {
                        // skip to `end record`
                        while (!at_end() && !(peek().is("end") && peek(1).is("record")))
                        {
                            take();
                        }
                        take();
                        take();
                        skip_statement();
                    }
                    else
                    {
                        skip_statement();    // enumeration, integer, ... : recorded by name, rejected when used for a signal
                    }
                    out.types.push_back(std::move(d));
                    return OK({});
                }

                Result<std::monostate> parse_subtype_declaration(Declarations& out)
                {
                    TypeDecl d;
                    d.location = location_of(take());    // subtype
                    auto n     = expect_name("a subtype name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    d.name = n.get();
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    auto tm = parse_type_mark();
                    if (tm.is_error())
                    {
                        return ERR(tm.get_error());
                    }
                    d.subtype = tm.get();
                    out.types.push_back(std::move(d));
                    return expect_symbol(";");
                }

                Result<std::monostate> parse_attribute(Declarations& out)
                {
                    const Token& start = take();    // attribute
                    auto n             = expect_name("an attribute name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    if (accept_symbol(":"))
                    {
                        AttributeDecl d;
                        d.location = location_of(start);
                        d.name     = n.get();
                        auto tn    = parse_selected_name("a type name");
                        if (tn.is_error())
                        {
                            return ERR(tn.get_error());
                        }
                        d.type = tn.get().back();
                        out.attribute_decls.push_back(std::move(d));
                        return expect_symbol(";");
                    }
                    if (!accept("of"))
                    {
                        return error(peek(), "expected ':' or 'of' after the attribute name");
                    }
                    AttributeSpec s;
                    s.location  = location_of(start);
                    s.attribute = n.get();
                    do
                    {
                        if (peek().is("others") || peek().is("all"))
                        {
                            return error(peek(), "attribute specifications for 'others' or 'all' are not supported");
                        }
                        auto target = expect_name("an attribute target");
                        if (target.is_error())
                        {
                            return ERR(target.get_error());
                        }
                        s.targets.push_back(target.get());
                    } while (accept_symbol(","));
                    if (auto res = expect_symbol(":"); res.is_error())
                    {
                        return res;
                    }
                    if (!peek().is_identifier())
                    {
                        return error(peek(), "expected an entity class such as 'signal' or 'label'");
                    }
                    s.entity_class = fold(take().text);
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    auto v = parse_expression();
                    if (v.is_error())
                    {
                        return ERR(v.get_error());
                    }
                    s.value = v.get();
                    out.attribute_specs.push_back(std::move(s));
                    return expect_symbol(";");
                }

                // ---- statements -------------------------------------------------------------------------------

                Result<std::vector<Association>> parse_association_list()
                {
                    std::vector<Association> out;
                    if (auto res = expect_symbol("("); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    if (accept_symbol(")"))
                    {
                        return OK(out);
                    }
                    do
                    {
                        Association a;
                        a.location = location_of(peek());
                        auto first = parse_expression();
                        if (first.is_error())
                        {
                            return ERR(first.get_error());
                        }
                        if (accept_symbol("=>"))
                        {
                            const Expr::Kind k = first.get().kind;
                            if (k != Expr::Kind::Identifier && k != Expr::Kind::Index && k != Expr::Kind::Slice)
                            {
                                return error<std::vector<Association>>(peek(), "the formal part of an association must be a name, an index or a slice");
                            }
                            a.formal    = first.get();
                            auto actual = parse_expression();
                            if (actual.is_error())
                            {
                                return ERR(actual.get_error());
                            }
                            a.actual = actual.get();
                        }
                        else
                        {
                            a.actual = first.get();
                        }
                        out.push_back(std::move(a));
                    } while (accept_symbol(","));
                    if (auto res = expect_symbol(")"); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    return OK(out);
                }

                Result<std::monostate> parse_instantiation(Architecture& arch, const Name& label, const Location& loc)
                {
                    Instantiation inst;
                    inst.label    = label;
                    inst.location = loc;
                    if (accept("entity"))
                    {
                        inst.kind = Instantiation::Kind::Entity;
                    }
                    else if (accept("configuration"))
                    {
                        inst.kind = Instantiation::Kind::Configuration;
                    }
                    else
                    {
                        accept("component");
                    }
                    auto parts = parse_selected_name("a component or entity name");
                    if (parts.is_error())
                    {
                        return ERR(parts.get_error());
                    }
                    inst.unit   = parts.get().back();
                    inst.prefix = parts.get();
                    inst.prefix.pop_back();
                    if (inst.kind == Instantiation::Kind::Entity && accept_symbol("("))
                    {
                        auto a = expect_name("an architecture name");
                        if (a.is_error())
                        {
                            return ERR(a.get_error());
                        }
                        inst.architecture = a.get();
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return res;
                        }
                    }
                    if (accept("generic"))
                    {
                        if (auto res = expect("map"); res.is_error())
                        {
                            return res;
                        }
                        auto g = parse_association_list();
                        if (g.is_error())
                        {
                            return ERR(g.get_error());
                        }
                        inst.generic_map = g.get();
                    }
                    if (accept("port"))
                    {
                        if (auto res = expect("map"); res.is_error())
                        {
                            return res;
                        }
                        auto p = parse_association_list();
                        if (p.is_error())
                        {
                            return ERR(p.get_error());
                        }
                        inst.port_map = p.get();
                    }
                    arch.instantiations.push_back(std::move(inst));
                    return expect_symbol(";");
                }

                Result<std::monostate> parse_assignment(Architecture& arch, const std::optional<Name>& label, const Location& loc)
                {
                    Assignment a;
                    a.label    = label;
                    a.location = loc;
                    auto target = parse_primary();    // a name or an aggregate, never a full expression, which would swallow the '<='
                    if (target.is_error())
                    {
                        return ERR(target.get_error());
                    }
                    a.target = target.get();
                    if (auto res = expect_symbol("<="); res.is_error())
                    {
                        return res;
                    }
                    accept("guarded");
                    if (peek().is("transport") || peek().is("reject") || peek().is("inertial"))
                    {
                        take();
                    }
                    auto value = parse_expression();
                    if (value.is_error())
                    {
                        return ERR(value.get_error());
                    }
                    a.value = value.get();
                    if (peek().is("after") || peek().is("when") || peek().is_symbol(","))
                    {
                        return error(peek(), "only a plain 'target <= value;' assignment is part of a structural netlist");
                    }
                    arch.assignments.push_back(std::move(a));
                    return expect_symbol(";");
                }

                Result<std::monostate> parse_statements(Architecture& arch)
                {
                    while (!peek().is("end") && !at_end())
                    {
                        const Token& t = peek();
                        const Location loc = location_of(t);
                        std::optional<Name> label;
                        if (t.is_identifier() && peek(1).is_symbol(":"))
                        {
                            label = name_of(take());
                            take();    // :
                        }
                        const Token& s = peek();
                        if (s.kind == TokenKind::Identifier && REJECTED_STATEMENTS.count(fold(s.text)) > 0)
                        {
                            return error(s, "'" + s.text + "' is not part of a structural netlist");
                        }
                        if (s.is("for") || s.is("if") || s.is("case"))
                        {
                            return error(s, "a generate statement is not part of a structural netlist");
                        }
                        Result<std::monostate> res = OK({});
                        if (label.has_value() && (s.is("entity") || s.is("configuration") || s.is("component") || (s.is_identifier() && !is_assignment_ahead())))
                        {
                            res = parse_instantiation(arch, label.value(), loc);
                        }
                        else
                        {
                            res = parse_assignment(arch, label, loc);
                        }
                        if (res.is_error())
                        {
                            return res;
                        }
                    }
                    return OK({});
                }

                /**
                 * After `label :`, decide between an instantiation and an assignment by looking for `<=` before the
                 * next `;` at bracket depth zero.
                 */
                bool is_assignment_ahead() const
                {
                    int depth = 0;
                    for (u32 i = m_pos; i < m_tokens.size(); i++)
                    {
                        const Token& t = m_tokens.at(i);
                        if (t.kind == TokenKind::EndOfFile)
                        {
                            return false;
                        }
                        if (t.is_symbol("("))
                        {
                            depth++;
                        }
                        else if (t.is_symbol(")"))
                        {
                            depth--;
                        }
                        else if (depth == 0 && t.is_symbol("<="))
                        {
                            return true;
                        }
                        else if (depth == 0 && (t.is_symbol(";") || t.is("port") || t.is("generic")))
                        {
                            return false;
                        }
                    }
                    return false;
                }

                // ---- design units ------------------------------------------------------------------------------

                Result<std::monostate> parse_context(std::vector<ContextClause>& out)
                {
                    ContextClause c;
                    c.location = location_of(peek());
                    c.is_use   = take().is("use");
                    do
                    {
                        auto parts = parse_selected_name("a library or package name");
                        if (parts.is_error())
                        {
                            return ERR(parts.get_error());
                        }
                        ContextClause one = c;
                        one.parts         = parts.get();
                        out.push_back(std::move(one));
                    } while (accept_symbol(","));
                    return expect_symbol(";");
                }

                Result<std::monostate> parse_entity(SourceFile& out)
                {
                    Entity e;
                    e.location = location_of(take());    // entity
                    auto n     = expect_name("an entity name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    e.name = n.get();
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    // the header comes first in the standard; declarations before it are tolerated
                    if (auto res = parse_declarations(e.declarations); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = parse_header(e.generics, e.ports); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = parse_declarations(e.declarations); res.is_error())
                    {
                        return res;
                    }
                    if (accept("begin"))
                    {
                        // passive statements only; a netlist has none
                        if (!peek().is("end"))
                        {
                            return error(peek(), "statements in an entity are not part of a structural netlist");
                        }
                    }
                    if (auto res = expect_end("entity", e.name); res.is_error())
                    {
                        return res;
                    }
                    out.entities.push_back(std::move(e));
                    return OK({});
                }

                Result<std::monostate> parse_architecture(SourceFile& out)
                {
                    Architecture a;
                    a.location = location_of(take());    // architecture
                    auto n     = expect_name("an architecture name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    a.name = n.get();
                    if (auto res = expect("of"); res.is_error())
                    {
                        return res;
                    }
                    auto en = parse_selected_name("an entity name");
                    if (en.is_error())
                    {
                        return ERR(en.get_error());
                    }
                    a.entity = en.get().back();
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = parse_declarations(a.declarations); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = expect("begin"); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = parse_statements(a); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = expect_end("architecture", a.name); res.is_error())
                    {
                        return res;
                    }
                    out.architectures.push_back(std::move(a));
                    return OK({});
                }

                Result<std::monostate> parse_package(SourceFile& out)
                {
                    const Token& start = take();    // package
                    if (accept("body"))
                    {
                        return skip_package_body();
                    }
                    Package p;
                    p.location = location_of(start);
                    auto n     = expect_name("a package name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    p.name = n.get();
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = parse_declarations(p.declarations); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = expect_end("package", p.name); res.is_error())
                    {
                        return res;
                    }
                    out.packages.push_back(std::move(p));
                    return OK({});
                }

                /**
                 * Skip a subprogram declaration `function f (...) return t;` or a body `... is ... begin ... end;`,
                 * counting the constructs that close with `end`.
                 */
                void skip_subprogram()
                {
                    int depth    = 0;
                    bool pending = true;    // the next `is` opens the body of this subprogram
                    while (!at_end())
                    {
                        const Token& t = take();
                        if (depth == 0)
                        {
                            if (t.is_symbol(";"))
                            {
                                return;    // a declaration only
                            }
                            if (t.is("is"))
                            {
                                depth   = 1;
                                pending = false;
                            }
                            continue;
                        }
                        if (t.is("end"))
                        {
                            depth--;
                            skip_statement();    // `end if;`, `end loop;`, `end function f;`
                            if (depth == 0)
                            {
                                return;
                            }
                        }
                        else if (t.is("function") || t.is("procedure"))
                        {
                            pending = true;
                        }
                        else if (t.is("is"))
                        {
                            if (pending)
                            {
                                depth++;
                                pending = false;
                            }
                        }
                        else if (t.is("if") || t.is("loop") || t.is("case") || t.is("record") || t.is("block") || t.is("process"))
                        {
                            depth++;
                        }
                    }
                }

                /**
                 * A package body holds subprograms, which a netlist does not need: skip to its end.
                 */
                Result<std::monostate> skip_package_body()
                {
                    auto n = expect_name("a package name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    Declarations unused;
                    while (!at_end())
                    {
                        const Token& t = peek();
                        if (t.is("end"))
                        {
                            take();
                            accept("package");
                            accept("body");
                            if (peek().is_identifier())
                            {
                                take();
                            }
                            return expect_symbol(";");
                        }
                        if (t.kind == TokenKind::Identifier && SUBPROGRAM_STARTS.count(fold(t.text)) > 0)
                        {
                            skip_subprogram();
                        }
                        else if (t.is("type"))
                        {
                            if (auto res = parse_type_declaration(unused); res.is_error())
                            {
                                return res;
                            }
                        }
                        else
                        {
                            skip_statement();
                        }
                    }
                    return error(peek(), "expected the end of the package body");
                }

                Result<std::monostate> parse_configuration(SourceFile& out)
                {
                    Configuration c;
                    c.location = location_of(take());    // configuration
                    auto n     = expect_name("a configuration name");
                    if (n.is_error())
                    {
                        return ERR(n.get_error());
                    }
                    c.name = n.get();
                    if (auto res = expect("of"); res.is_error())
                    {
                        return res;
                    }
                    auto en = parse_selected_name("an entity name");
                    if (en.is_error())
                    {
                        return ERR(en.get_error());
                    }
                    c.entity = en.get().back();
                    if (auto res = expect("is"); res.is_error())
                    {
                        return res;
                    }
                    // `for arch ... end for;` with nested `for all : comp use entity ...; end for;`
                    int depth = 0;
                    while (!at_end())
                    {
                        if (accept("for"))
                        {
                            depth++;
                            std::vector<Name> labels;
                            bool all = false;
                            if (accept("all"))
                            {
                                all = true;
                            }
                            else
                            {
                                do
                                {
                                    auto l = expect_name("a label or architecture name");
                                    if (l.is_error())
                                    {
                                        return ERR(l.get_error());
                                    }
                                    labels.push_back(l.get());
                                } while (accept_symbol(","));
                            }
                            if (accept_symbol(":"))
                            {
                                auto comp = expect_name("a component name");
                                if (comp.is_error())
                                {
                                    return ERR(comp.get_error());
                                }
                                if (accept("use") && accept("entity"))
                                {
                                    Configuration::Binding b;
                                    b.component = comp.get();
                                    auto parts  = parse_selected_name("an entity name");
                                    if (parts.is_error())
                                    {
                                        return ERR(parts.get_error());
                                    }
                                    b.entity = parts.get().back();
                                    b.prefix = parts.get();
                                    b.prefix.pop_back();
                                    if (accept_symbol("("))
                                    {
                                        auto a = expect_name("an architecture name");
                                        if (a.is_error())
                                        {
                                            return ERR(a.get_error());
                                        }
                                        b.architecture = a.get();
                                        if (auto res = expect_symbol(")"); res.is_error())
                                        {
                                            return res;
                                        }
                                    }
                                    if (all || !labels.empty())
                                    {
                                        c.bindings.push_back(std::move(b));
                                    }
                                }
                                skip_statement();    // the rest of the binding indication
                            }
                            (void)all;
                        }
                        else if (peek().is("end") && peek(1).is("for"))
                        {
                            take();
                            take();
                            if (auto res = expect_symbol(";"); res.is_error())
                            {
                                return res;
                            }
                            depth--;
                        }
                        else if (peek().is("end") && depth == 0)
                        {
                            break;
                        }
                        else
                        {
                            skip_statement();
                        }
                    }
                    if (auto res = expect_end("configuration", c.name); res.is_error())
                    {
                        return res;
                    }
                    out.configurations.push_back(std::move(c));
                    return OK({});
                }
            };
        }    // namespace

        Result<ast::SourceFile> parse_tokens(const std::vector<Token>& tokens, const std::string& file)
        {
            if (tokens.empty())
            {
                return ERR("no tokens to parse for '" + file + "'");
            }
            Parser parser(tokens, file);
            auto res = parser.run();
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse '" + file + "'");
            }
            if (res.get().entities.empty())
            {
                return ERR("could not parse '" + file + "': the file contains no entity");
            }
            return res;
        }

        Result<ast::SourceFile> parse_string(const std::string& text, const std::filesystem::path& file)
        {
            auto tokens = lex_string(text, file);
            if (tokens.is_error())
            {
                return ERR(tokens.get_error());
            }
            return parse_tokens(tokens.get(), file.string());
        }

        Result<ast::SourceFile> parse_file(const std::filesystem::path& file)
        {
            auto tokens = lex_file(file);
            if (tokens.is_error())
            {
                return ERR(tokens.get_error());
            }
            return parse_tokens(tokens.get(), file.string());
        }
    }    // namespace vhdl
}    // namespace hal
