#include "verilog_parser/verilog_syntax.h"

#include <set>

namespace hal
{
    namespace verilog
    {
        namespace
        {
            using namespace ast;

            const std::set<std::string> NET_TYPES = {"wire", "tri", "tri0", "tri1", "triand", "trior", "trireg", "wand", "wor", "supply0", "supply1", "reg", "logic", "uwire"};

            const std::set<std::string> REJECTED = {"initial", "always", "always_comb", "always_ff", "always_latch", "function",  "task",      "generate", "genvar",
                                                    "for",     "if",     "case",        "begin",     "specify",      "specparam", "primitive", "table",    "integer",
                                                    "real",    "time",   "event",       "class",     "package",      "interface", "program"};

            /**
             * Precedence of the binary operators, higher binds tighter.
             */
            int precedence(const std::string& op)
            {
                if (op == "||")
                {
                    return 1;
                }
                if (op == "&&")
                {
                    return 2;
                }
                if (op == "|")
                {
                    return 3;
                }
                if (op == "^")
                {
                    return 4;
                }
                if (op == "&")
                {
                    return 5;
                }
                if (op == "==" || op == "!=")
                {
                    return 6;
                }
                if (op == "<" || op == ">" || op == "<=" || op == ">=")
                {
                    return 7;
                }
                if (op == "<<" || op == ">>")
                {
                    return 8;
                }
                if (op == "+" || op == "-")
                {
                    return 9;
                }
                if (op == "*" || op == "/" || op == "%")
                {
                    return 10;
                }
                if (op == "**")
                {
                    return 11;
                }
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
                        std::vector<Attribute> attributes;
                        if (auto res = parse_attributes(attributes); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        if (peek().is("module") || peek().is("macromodule"))
                        {
                            auto res = parse_module();
                            if (res.is_error())
                            {
                                return ERR(res.get_error());
                            }
                            Module m     = res.get();
                            m.attributes = attributes;
                            out.modules.push_back(std::move(m));
                        }
                        else if (peek().is("primitive"))
                        {
                            return error<SourceFile>(peek(), "user-defined primitives are not supported");
                        }
                        else
                        {
                            return error<SourceFile>(peek(), "expected 'module'");
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

                bool accept_symbol(const char* symbol)
                {
                    if (peek().is_symbol(symbol))
                    {
                        take();
                        return true;
                    }
                    return false;
                }

                bool accept_keyword(const char* keyword)
                {
                    if (peek().is(keyword))
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

                Result<std::monostate> expect_symbol(const char* symbol)
                {
                    if (!accept_symbol(symbol))
                    {
                        return error(peek(), std::string("expected '") + symbol + "'");
                    }
                    return OK({});
                }

                Result<std::monostate> expect_keyword(const char* keyword)
                {
                    if (!accept_keyword(keyword))
                    {
                        return error(peek(), std::string("expected '") + keyword + "'");
                    }
                    return OK({});
                }

                bool is_identifier(const Token& t) const
                {
                    return t.kind == TokenKind::Identifier || t.kind == TokenKind::EscapedIdentifier;
                }

                Result<std::string> expect_identifier(const char* what)
                {
                    if (!is_identifier(peek()))
                    {
                        return error<std::string>(peek(), std::string("expected ") + what);
                    }
                    return OK(take().text);
                }

                bool is_net_type(const Token& t) const
                {
                    return t.kind == TokenKind::Identifier && NET_TYPES.count(t.text) > 0;
                }

                bool is_direction(const Token& t) const
                {
                    return t.is("input") || t.is("output") || t.is("inout");
                }

                static PinDirection direction_of(const std::string& keyword)
                {
                    if (keyword == "input")
                    {
                        return PinDirection::input;
                    }
                    if (keyword == "output")
                    {
                        return PinDirection::output;
                    }
                    return PinDirection::inout;
                }

                // ---- attributes ----------------------------------------------------------------------------------

                /**
                 * Parse any number of `(* ... *)` groups in front of an item.
                 */
                Result<std::monostate> parse_attributes(std::vector<Attribute>& out)
                {
                    while (accept_symbol("(*"))
                    {
                        do
                        {
                            Attribute a;
                            a.location = location_of(peek());
                            auto name  = expect_identifier("an attribute name");
                            if (name.is_error())
                            {
                                return ERR(name.get_error());
                            }
                            a.name = name.get();
                            if (accept_symbol("="))
                            {
                                auto value = parse_expression();
                                if (value.is_error())
                                {
                                    return ERR(value.get_error());
                                }
                                a.value = value.get();
                            }
                            out.push_back(std::move(a));
                        } while (accept_symbol(","));
                        if (auto res = expect_symbol("*)"); res.is_error())
                        {
                            return res;
                        }
                    }
                    return OK({});
                }

                // ---- expressions ---------------------------------------------------------------------------------

                Result<Expr> parse_expression()
                {
                    auto lhs = parse_binary(0);
                    if (lhs.is_error())
                    {
                        return lhs;
                    }
                    if (accept_symbol("?"))
                    {
                        Expr cond;
                        cond.kind     = Expr::Kind::Conditional;
                        cond.location = lhs.get().location;
                        cond.children.push_back(lhs.get());
                        auto a = parse_expression();
                        if (a.is_error())
                        {
                            return a;
                        }
                        if (auto res = expect_symbol(":"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        auto b = parse_expression();
                        if (b.is_error())
                        {
                            return b;
                        }
                        cond.children.push_back(a.get());
                        cond.children.push_back(b.get());
                        return OK(cond);
                    }
                    return lhs;
                }

                Result<Expr> parse_binary(int min_precedence)
                {
                    auto lhs = parse_unary();
                    if (lhs.is_error())
                    {
                        return lhs;
                    }
                    Expr left = lhs.get();
                    while (peek().kind == TokenKind::Symbol && precedence(peek().text) > min_precedence)
                    {
                        const Token& op = take();
                        auto rhs        = parse_binary(precedence(op.text));
                        if (rhs.is_error())
                        {
                            return rhs;
                        }
                        Expr bin;
                        bin.kind     = Expr::Kind::Binary;
                        bin.text     = op.text;
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
                    if (t.kind == TokenKind::Symbol && (t.text == "~" || t.text == "!" || t.text == "-" || t.text == "+" || t.text == "&" || t.text == "|" || t.text == "^"))
                    {
                        take();
                        auto operand = parse_unary();
                        if (operand.is_error())
                        {
                            return operand;
                        }
                        Expr u;
                        u.kind     = Expr::Kind::Unary;
                        u.text     = t.text;
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

                    if (t.kind == TokenKind::Number)
                    {
                        take();
                        e.kind = Expr::Kind::Number;
                        e.text = t.text;
                        return OK(e);
                    }
                    if (t.kind == TokenKind::String)
                    {
                        take();
                        e.kind = Expr::Kind::String;
                        e.text = t.text;
                        return OK(e);
                    }
                    if (t.is_symbol("("))
                    {
                        take();
                        auto inner = parse_expression();
                        if (inner.is_error())
                        {
                            return inner;
                        }
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        return inner;
                    }
                    if (t.is_symbol("{"))
                    {
                        take();
                        // `{count{items}}` or `{items}`
                        auto first = parse_expression();
                        if (first.is_error())
                        {
                            return first;
                        }
                        if (peek().is_symbol("{"))
                        {
                            take();
                            e.kind = Expr::Kind::Replicate;
                            e.children.push_back(first.get());
                            do
                            {
                                auto item = parse_expression();
                                if (item.is_error())
                                {
                                    return item;
                                }
                                e.children.push_back(item.get());
                            } while (accept_symbol(","));
                            if (auto res = expect_symbol("}"); res.is_error())
                            {
                                return ERR(res.get_error());
                            }
                            if (auto res = expect_symbol("}"); res.is_error())
                            {
                                return ERR(res.get_error());
                            }
                            return OK(e);
                        }
                        e.kind = Expr::Kind::Concat;
                        e.children.push_back(first.get());
                        while (accept_symbol(","))
                        {
                            auto item = parse_expression();
                            if (item.is_error())
                            {
                                return item;
                            }
                            e.children.push_back(item.get());
                        }
                        if (auto res = expect_symbol("}"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        return OK(e);
                    }
                    if (is_identifier(t))
                    {
                        take();
                        e.kind = Expr::Kind::Identifier;
                        e.text = t.text;
                        // a hierarchical name stays one identifier with dots
                        while (peek().is_symbol(".") && is_identifier(peek(1)))
                        {
                            take();
                            e.text += "." + take().text;
                        }
                        // any number of index and slice suffixes
                        while (peek().is_symbol("["))
                        {
                            take();
                            auto first = parse_expression();
                            if (first.is_error())
                            {
                                return first;
                            }
                            Expr suffix;
                            suffix.location = e.location;
                            suffix.children.push_back(std::move(e));
                            if (accept_symbol(":"))
                            {
                                auto second = parse_expression();
                                if (second.is_error())
                                {
                                    return second;
                                }
                                suffix.kind = Expr::Kind::Slice;
                                suffix.children.push_back(first.get());
                                suffix.children.push_back(second.get());
                            }
                            else
                            {
                                suffix.kind = Expr::Kind::Index;
                                suffix.children.push_back(first.get());
                            }
                            if (auto res = expect_symbol("]"); res.is_error())
                            {
                                return ERR(res.get_error());
                            }
                            e = std::move(suffix);
                        }
                        return OK(e);
                    }
                    if (t.kind == TokenKind::SystemIdentifier)
                    {
                        return error<Expr>(t, "system functions are not supported in a netlist");
                    }
                    return error<Expr>(t, "expected an expression");
                }

                /**
                 * An expression that may be absent: `.a()`, an empty positional slot, or `()`.
                 */
                Result<Expr> parse_optional_expression()
                {
                    if (peek().is_symbol(")") || peek().is_symbol(","))
                    {
                        Expr empty;
                        empty.location = location_of(peek());
                        return OK(empty);
                    }
                    return parse_expression();
                }

                // ---- ranges and declarations ---------------------------------------------------------------------

                Result<std::vector<Range>> parse_ranges()
                {
                    std::vector<Range> dims;
                    while (peek().is_symbol("["))
                    {
                        take();
                        Range r;
                        auto left = parse_expression();
                        if (left.is_error())
                        {
                            return ERR(left.get_error());
                        }
                        r.left = left.get();
                        if (auto res = expect_symbol(":"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        auto right = parse_expression();
                        if (right.is_error())
                        {
                            return ERR(right.get_error());
                        }
                        r.right = right.get();
                        if (auto res = expect_symbol("]"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        dims.push_back(std::move(r));
                    }
                    return OK(dims);
                }

                /**
                 * `input [7:0] a, b;` and the like in the body.
                 */
                Result<std::monostate> parse_port_declaration(Module& m, std::vector<Attribute>& attributes)
                {
                    const Token& dir_token = take();
                    const PinDirection dir = direction_of(dir_token.text);
                    std::string net_type;
                    if (is_net_type(peek()))
                    {
                        net_type = take().text;
                    }
                    accept_keyword("signed");
                    auto dims_res = parse_ranges();
                    if (dims_res.is_error())
                    {
                        return ERR(dims_res.get_error());
                    }
                    const std::vector<Range> dims = dims_res.get();    // get() moves, so take it once
                    do
                    {
                        PortDecl p;
                        p.location = location_of(peek());
                        auto name  = expect_identifier("a port name");
                        if (name.is_error())
                        {
                            return ERR(name.get_error());
                        }
                        p.name        = name.get();
                        p.direction   = dir;
                        p.net_type    = net_type;
                        p.packed_dims = dims;
                        auto unpacked = parse_ranges();
                        if (unpacked.is_error())
                        {
                            return ERR(unpacked.get_error());
                        }
                        p.unpacked_dims = unpacked.get();
                        p.attributes    = attributes;
                        m.ports.push_back(std::move(p));
                    } while (accept_symbol(","));
                    return expect_symbol(";");
                }

                /**
                 * `wire [7:0] a = b, c;` and the like in the body.
                 */
                Result<std::monostate> parse_net_declaration(Module& m, std::vector<Attribute>& attributes)
                {
                    const std::string net_type = take().text;
                    accept_keyword("signed");
                    auto dims_res = parse_ranges();
                    if (dims_res.is_error())
                    {
                        return ERR(dims_res.get_error());
                    }
                    const std::vector<Range> dims = dims_res.get();    // get() moves, so take it once
                    do
                    {
                        NetDecl n;
                        n.location = location_of(peek());
                        auto name  = expect_identifier("a net name");
                        if (name.is_error())
                        {
                            return ERR(name.get_error());
                        }
                        n.name        = name.get();
                        n.net_type    = net_type;
                        n.packed_dims = dims;
                        auto unpacked = parse_ranges();
                        if (unpacked.is_error())
                        {
                            return ERR(unpacked.get_error());
                        }
                        n.unpacked_dims = unpacked.get();
                        if (accept_symbol("="))
                        {
                            auto init = parse_expression();
                            if (init.is_error())
                            {
                                return ERR(init.get_error());
                            }
                            n.initializer = init.get();
                        }
                        n.attributes = attributes;
                        m.nets.push_back(std::move(n));
                    } while (accept_symbol(","));
                    return expect_symbol(";");
                }

                /**
                 * `parameter W = 8, D = 2` in a header or, with `;`, in the body.
                 */
                Result<std::monostate> parse_parameter_declaration(Module& m, bool in_header)
                {
                    const bool is_local = take().is("localparam");
                    // an optional type or range before the name
                    accept_keyword("integer");
                    accept_keyword("real");
                    accept_keyword("string");
                    accept_keyword("signed");
                    if (peek().is_symbol("["))
                    {
                        auto dims = parse_ranges();
                        if (dims.is_error())
                        {
                            return ERR(dims.get_error());
                        }
                    }
                    do
                    {
                        ParameterDecl p;
                        p.location = location_of(peek());
                        p.is_local = is_local;
                        auto name  = expect_identifier("a parameter name");
                        if (name.is_error())
                        {
                            return ERR(name.get_error());
                        }
                        p.name = name.get();
                        if (auto res = expect_symbol("="); res.is_error())
                        {
                            return res;
                        }
                        auto value = parse_expression();
                        if (value.is_error())
                        {
                            return ERR(value.get_error());
                        }
                        p.value = value.get();
                        m.parameters.push_back(std::move(p));
                        // in a header the list continues with ',' but a ',' may also introduce the next 'parameter' keyword
                        if (in_header && peek().is_symbol(",") && (peek(1).is("parameter") || peek(1).is("localparam")))
                        {
                            take();
                            return OK({});
                        }
                    } while (accept_symbol(","));
                    if (in_header)
                    {
                        return OK({});
                    }
                    return expect_symbol(";");
                }

                Result<std::monostate> parse_defparam(Module& m)
                {
                    take();    // defparam
                    do
                    {
                        Defparam d;
                        d.location = location_of(peek());
                        do
                        {
                            auto part = expect_identifier("an instance or parameter name");
                            if (part.is_error())
                            {
                                return ERR(part.get_error());
                            }
                            d.path.push_back(part.get());
                        } while (accept_symbol("."));
                        if (d.path.size() < 2)
                        {
                            return error(peek(), "expected 'instance.parameter' after 'defparam'");
                        }
                        if (auto res = expect_symbol("="); res.is_error())
                        {
                            return res;
                        }
                        auto value = parse_expression();
                        if (value.is_error())
                        {
                            return ERR(value.get_error());
                        }
                        d.value = value.get();
                        m.defparams.push_back(std::move(d));
                    } while (accept_symbol(","));
                    return expect_symbol(";");
                }

                Result<std::monostate> parse_assignment(Module& m, std::vector<Attribute>& attributes)
                {
                    take();    // assign
                    // drive strength and delay are not part of a netlist, skip them if present
                    if (accept_symbol("("))
                    {
                        while (!at_end() && !peek().is_symbol(")"))
                        {
                            take();
                        }
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return res;
                        }
                    }
                    if (accept_symbol("#"))
                    {
                        take();
                    }
                    do
                    {
                        Assignment a;
                        a.location = location_of(peek());
                        auto lhs   = parse_expression();
                        if (lhs.is_error())
                        {
                            return ERR(lhs.get_error());
                        }
                        a.lhs = lhs.get();
                        if (auto res = expect_symbol("="); res.is_error())
                        {
                            return res;
                        }
                        auto rhs = parse_expression();
                        if (rhs.is_error())
                        {
                            return ERR(rhs.get_error());
                        }
                        a.rhs        = rhs.get();
                        a.attributes = attributes;
                        m.assignments.push_back(std::move(a));
                    } while (accept_symbol(","));
                    return expect_symbol(";");
                }

                /**
                 * `TYPE #(params) name (connections), name2 (connections);`
                 */
                Result<std::monostate> parse_instantiation(Module& m, std::vector<Attribute>& attributes)
                {
                    const Token& type_token = take();
                    std::vector<ParameterAssignment> parameters;

                    if (accept_symbol("#("))
                    {
                        if (!peek().is_symbol(")"))
                        {
                            do
                            {
                                ParameterAssignment p;
                                p.location = location_of(peek());
                                if (peek().is_symbol("."))
                                {
                                    take();
                                    auto name = expect_identifier("a parameter name");
                                    if (name.is_error())
                                    {
                                        return ERR(name.get_error());
                                    }
                                    p.name = name.get();
                                    if (auto res = expect_symbol("("); res.is_error())
                                    {
                                        return res;
                                    }
                                    auto value = parse_optional_expression();
                                    if (value.is_error())
                                    {
                                        return ERR(value.get_error());
                                    }
                                    p.value = value.get();
                                    if (auto res = expect_symbol(")"); res.is_error())
                                    {
                                        return res;
                                    }
                                }
                                else
                                {
                                    auto value = parse_expression();
                                    if (value.is_error())
                                    {
                                        return ERR(value.get_error());
                                    }
                                    p.value = value.get();
                                }
                                parameters.push_back(std::move(p));
                            } while (accept_symbol(","));
                        }
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return res;
                        }
                    }
                    else if (accept_symbol("#"))
                    {
                        // a delay: no meaning in a netlist
                        if (peek().kind != TokenKind::Number)
                        {
                            return error(peek(), "expected a delay after '#'");
                        }
                        take();
                    }

                    do
                    {
                        Instantiation inst;
                        inst.location = location_of(peek());
                        inst.type     = type_token.text;
                        auto name     = expect_identifier("an instance name");
                        if (name.is_error())
                        {
                            return ERR(name.get_error());
                        }
                        inst.name       = name.get();
                        inst.parameters = parameters;
                        inst.attributes = attributes;
                        if (peek().is_symbol("["))
                        {
                            return error(peek(), "arrays of instances are not supported");
                        }
                        if (auto res = expect_symbol("("); res.is_error())
                        {
                            return res;
                        }
                        if (!peek().is_symbol(")"))
                        {
                            bool named      = false;
                            bool positional = false;
                            do
                            {
                                Connection c;
                                c.location = location_of(peek());
                                if (peek().is_symbol("."))
                                {
                                    take();
                                    named     = true;
                                    auto port = expect_identifier("a port name");
                                    if (port.is_error())
                                    {
                                        return ERR(port.get_error());
                                    }
                                    c.port = port.get();
                                    if (auto res = expect_symbol("("); res.is_error())
                                    {
                                        return res;
                                    }
                                    auto expr = parse_optional_expression();
                                    if (expr.is_error())
                                    {
                                        return ERR(expr.get_error());
                                    }
                                    c.expr = expr.get();
                                    if (auto res = expect_symbol(")"); res.is_error())
                                    {
                                        return res;
                                    }
                                }
                                else
                                {
                                    positional = true;
                                    auto expr  = parse_optional_expression();
                                    if (expr.is_error())
                                    {
                                        return ERR(expr.get_error());
                                    }
                                    c.expr = expr.get();
                                }
                                if (named && positional)
                                {
                                    return error(peek(), "named and positional connections are mixed in instance '" + inst.name + "'");
                                }
                                inst.connections.push_back(std::move(c));
                            } while (accept_symbol(","));
                        }
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return res;
                        }
                        m.instantiations.push_back(std::move(inst));
                    } while (accept_symbol(","));
                    return expect_symbol(";");
                }

                // ---- module ---------------------------------------------------------------------------------------

                /**
                 * The port list of the header: empty, plain names and `.name(expr)` entries, or ANSI declarations.
                 */
                Result<std::monostate> parse_header_ports(Module& m)
                {
                    if (!accept_symbol("("))
                    {
                        return OK({});
                    }
                    if (accept_symbol(")"))
                    {
                        return OK({});
                    }

                    // the first entry decides the style
                    std::vector<Attribute> attributes;
                    if (auto res = parse_attributes(attributes); res.is_error())
                    {
                        return res;
                    }
                    if (is_direction(peek()) || (is_net_type(peek()) && !peek(1).is_symbol(",") && !peek(1).is_symbol(")")))
                    {
                        m.ansi_ports           = true;
                        PinDirection direction = PinDirection::none;
                        std::string net_type;
                        std::vector<Range> dims;
                        do
                        {
                            if (attributes.empty())
                            {
                                if (auto res = parse_attributes(attributes); res.is_error())
                                {
                                    return res;
                                }
                            }
                            if (is_direction(peek()))
                            {
                                direction = direction_of(take().text);
                                net_type.clear();
                                dims.clear();
                                if (is_net_type(peek()))
                                {
                                    net_type = take().text;
                                }
                                accept_keyword("signed");
                                auto d = parse_ranges();
                                if (d.is_error())
                                {
                                    return ERR(d.get_error());
                                }
                                dims = d.get();
                            }
                            if (direction == PinDirection::none)
                            {
                                return error(peek(), "expected 'input', 'output' or 'inout'");
                            }
                            PortDecl p;
                            p.location = location_of(peek());
                            auto name  = expect_identifier("a port name");
                            if (name.is_error())
                            {
                                return ERR(name.get_error());
                            }
                            p.name        = name.get();
                            p.direction   = direction;
                            p.net_type    = net_type;
                            p.packed_dims = dims;
                            auto unpacked = parse_ranges();
                            if (unpacked.is_error())
                            {
                                return ERR(unpacked.get_error());
                            }
                            p.unpacked_dims = unpacked.get();
                            p.attributes    = attributes;
                            attributes.clear();

                            HeaderPort h;
                            h.name     = p.name;
                            h.location = p.location;
                            m.header_ports.push_back(std::move(h));
                            m.ports.push_back(std::move(p));
                        } while (accept_symbol(","));
                    }
                    else
                    {
                        if (!attributes.empty())
                        {
                            return error(peek(), "attributes on a header port require an ANSI declaration");
                        }
                        do
                        {
                            if (peek().is_symbol(")"))
                            {
                                break;    // a trailing comma, which some writers leave behind
                            }
                            HeaderPort h;
                            h.location = location_of(peek());
                            if (accept_symbol("."))
                            {
                                auto name = expect_identifier("a port name");
                                if (name.is_error())
                                {
                                    return ERR(name.get_error());
                                }
                                h.name = name.get();
                                if (auto res = expect_symbol("("); res.is_error())
                                {
                                    return res;
                                }
                                auto expr = parse_optional_expression();
                                if (expr.is_error())
                                {
                                    return ERR(expr.get_error());
                                }
                                h.expression = expr.get();
                                if (auto res = expect_symbol(")"); res.is_error())
                                {
                                    return res;
                                }
                            }
                            else
                            {
                                auto name = expect_identifier("a port name");
                                if (name.is_error())
                                {
                                    return ERR(name.get_error());
                                }
                                h.name = name.get();
                                if (peek().is_symbol("["))
                                {
                                    return error(peek(), "a range on a header port requires an ANSI declaration");
                                }
                            }
                            m.header_ports.push_back(std::move(h));
                        } while (accept_symbol(","));
                    }
                    return expect_symbol(")");
                }

                Result<Module> parse_module()
                {
                    Module m;
                    m.location = location_of(peek());
                    take();    // module
                    auto name = expect_identifier("a module name");
                    if (name.is_error())
                    {
                        return ERR(name.get_error());
                    }
                    m.name = name.get();

                    if (accept_symbol("#("))
                    {
                        while (peek().is("parameter") || peek().is("localparam"))
                        {
                            if (auto res = parse_parameter_declaration(m, true); res.is_error())
                            {
                                return ERR(res.get_error());
                            }
                        }
                        if (auto res = expect_symbol(")"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                    }

                    if (auto res = parse_header_ports(m); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    if (auto res = expect_symbol(";"); res.is_error())
                    {
                        return ERR(res.get_error());
                    }

                    while (!accept_keyword("endmodule"))
                    {
                        if (at_end())
                        {
                            return error<Module>(peek(), "expected 'endmodule' for module '" + m.name + "'");
                        }
                        std::vector<Attribute> attributes;
                        if (auto res = parse_attributes(attributes); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                        const Token& t             = peek();
                        Result<std::monostate> res = OK({});
                        if (is_direction(t))
                        {
                            res = parse_port_declaration(m, attributes);
                        }
                        else if (is_net_type(t))
                        {
                            res = parse_net_declaration(m, attributes);
                        }
                        else if (t.is("assign"))
                        {
                            res = parse_assignment(m, attributes);
                        }
                        else if (t.is("parameter") || t.is("localparam"))
                        {
                            res = parse_parameter_declaration(m, false);
                        }
                        else if (t.is("defparam"))
                        {
                            res = parse_defparam(m);
                        }
                        else if (t.kind == TokenKind::Identifier && REJECTED.count(t.text) > 0)
                        {
                            return error<Module>(t, "'" + t.text + "' is not part of a structural netlist");
                        }
                        else if (t.is("module") || t.is("macromodule"))
                        {
                            return error<Module>(t, "expected 'endmodule' for module '" + m.name + "'");
                        }
                        else if (is_identifier(t))
                        {
                            res = parse_instantiation(m, attributes);
                        }
                        else
                        {
                            return error<Module>(t, "expected a declaration, an assignment or an instantiation");
                        }
                        if (res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                    }

                    // `endmodule : name`
                    if (accept_symbol(":"))
                    {
                        if (auto res = expect_identifier("the module name"); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                    }
                    return OK(m);
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
            if (res.get().modules.empty())
            {
                return ERR("could not parse '" + file + "': the file contains no module");
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
    }    // namespace verilog
}    // namespace hal
