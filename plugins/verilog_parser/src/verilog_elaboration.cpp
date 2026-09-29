#include "verilog_parser/verilog_elaboration.h"

#include "hal_core/utilities/log.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <set>
#include <unordered_map>

namespace hal
{
    namespace verilog
    {
        using namespace netlist_ir;

        // ---------------------------------------------------------------------------------------------------------
        // numbers
        // ---------------------------------------------------------------------------------------------------------

        bool Number::is_defined() const
        {
            return !is_real && bits.find_first_not_of("01") == std::string::npos;
        }

        Result<u64> Number::to_u64() const
        {
            if (is_real)
            {
                return ERR("'" + text + "' is a real number");
            }
            if (!is_defined())
            {
                return ERR("'" + text + "' has x or z bits");
            }
            const auto first_one = bits.find('1');
            if (first_one != std::string::npos && bits.size() - first_one > 64)
            {
                return ERR("'" + text + "' does not fit in 64 bits");
            }
            u64 v = 0;
            for (const char c : bits)
            {
                v = (v << 1) | (c == '1' ? 1u : 0u);
            }
            return OK(v);
        }

        std::string Number::to_hex() const
        {
            // without leading zeros: the declaration carries the width, and that is what the parsers always stored
            std::string padded = bits;
            while (padded.size() % 4 != 0)
            {
                padded = "0" + padded;
            }
            std::string hex;
            for (u32 i = 0; i < padded.size(); i += 4)
            {
                u32 nibble = 0;
                for (u32 k = 0; k < 4; k++)
                {
                    nibble = (nibble << 1) | (padded[i + k] == '1' ? 1u : 0u);
                }
                if (nibble != 0 || !hex.empty())
                {
                    hex += "0123456789ABCDEF"[nibble];
                }
            }
            return "0x" + (hex.empty() ? std::string("0") : hex);
        }

        Result<Number> parse_number(const std::string& text)
        {
            Number n;
            n.text = text;

            std::string s;
            for (const char c : text)
            {
                if (c != '_' && c != ' ' && c != '\t')
                {
                    s += c;
                }
            }
            if (s.empty())
            {
                return ERR("empty number");
            }

            const auto apostrophe = s.find('\'');
            if (apostrophe == std::string::npos)
            {
                // plain decimal or real
                if (s.find_first_of(".eE") != std::string::npos)
                {
                    n.is_real = true;
                    return OK(n);
                }
                if (s.find_first_not_of("0123456789") != std::string::npos)
                {
                    return ERR("'" + text + "' is not a valid number");
                }
                u64 v = 0;
                for (const char c : s)
                {
                    if (v > (UINT64_MAX - 9) / 10)
                    {
                        return ERR("'" + text + "' does not fit in 64 bits");
                    }
                    v = v * 10 + static_cast<u64>(c - '0');
                }
                std::string bits;
                do
                {
                    bits.insert(bits.begin(), (v & 1) ? '1' : '0');
                    v >>= 1;
                } while (v != 0);
                n.bits  = bits;
                n.width = static_cast<u32>(bits.size());
                return OK(n);
            }

            if (apostrophe > 0)
            {
                const std::string size = s.substr(0, apostrophe);
                if (size.find_first_not_of("0123456789") != std::string::npos)
                {
                    return ERR("'" + text + "' has an invalid width");
                }
                n.sized = true;
                n.width = static_cast<u32>(std::stoul(size));
                if (n.width == 0)
                {
                    return ERR("'" + text + "' has width 0");
                }
            }

            std::string rest = s.substr(apostrophe + 1);
            if (rest.empty())
            {
                return ERR("'" + text + "' has no digits");
            }

            // SystemVerilog fill: '0, '1, 'x, 'z
            if (!n.sized && rest.size() == 1 && (rest[0] == '0' || rest[0] == '1' || rest[0] == 'x' || rest[0] == 'X' || rest[0] == 'z' || rest[0] == 'Z'))
            {
                n.based = true;
                n.width = 1;
                n.bits  = std::string(1, static_cast<char>(std::tolower(static_cast<unsigned char>(rest[0]))));
                return OK(n);
            }

            if (rest[0] == 's' || rest[0] == 'S')
            {
                n.is_signed = true;
                rest        = rest.substr(1);
            }
            if (rest.empty())
            {
                return ERR("'" + text + "' has no base");
            }
            const char base          = static_cast<char>(std::tolower(static_cast<unsigned char>(rest[0])));
            const std::string digits = rest.substr(1);
            n.based                  = true;
            if (digits.empty())
            {
                return ERR("'" + text + "' has no digits");
            }

            std::string bits;
            if (base == 'd')
            {
                if (digits.size() == 1 && (digits[0] == 'x' || digits[0] == 'X' || digits[0] == 'z' || digits[0] == 'Z'))
                {
                    bits = std::string(1, static_cast<char>(std::tolower(static_cast<unsigned char>(digits[0]))));
                }
                else
                {
                    if (digits.find_first_not_of("0123456789") != std::string::npos)
                    {
                        return ERR("'" + text + "' has an invalid decimal digit");
                    }
                    u64 v = 0;
                    for (const char c : digits)
                    {
                        if (v > (UINT64_MAX - 9) / 10)
                        {
                            return ERR("'" + text + "' does not fit in 64 bits");
                        }
                        v = v * 10 + static_cast<u64>(c - '0');
                    }
                    do
                    {
                        bits.insert(bits.begin(), (v & 1) ? '1' : '0');
                        v >>= 1;
                    } while (v != 0);
                }
            }
            else
            {
                u32 bits_per_digit = 0;
                if (base == 'b')
                {
                    bits_per_digit = 1;
                }
                else if (base == 'o')
                {
                    bits_per_digit = 3;
                }
                else if (base == 'h')
                {
                    bits_per_digit = 4;
                }
                else
                {
                    return ERR("'" + text + "' has an unknown base '" + std::string(1, base) + "'");
                }

                for (const char raw : digits)
                {
                    const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(raw)));
                    if (c == 'x' || c == 'z' || c == '?')
                    {
                        bits += std::string(bits_per_digit, c == '?' ? 'z' : c);
                        continue;
                    }
                    u32 value = 0;
                    if (c >= '0' && c <= '9')
                    {
                        value = static_cast<u32>(c - '0');
                    }
                    else if (c >= 'a' && c <= 'f')
                    {
                        value = static_cast<u32>(c - 'a' + 10);
                    }
                    else
                    {
                        return ERR("'" + text + "' has an invalid digit '" + std::string(1, raw) + "'");
                    }
                    if (value >= (1u << bits_per_digit))
                    {
                        return ERR("'" + text + "' has a digit '" + std::string(1, raw) + "' that is not valid for its base");
                    }
                    for (u32 k = bits_per_digit; k-- > 0;)
                    {
                        bits += ((value >> k) & 1) ? '1' : '0';
                    }
                }
            }

            if (n.sized)
            {
                if (bits.size() > n.width)
                {
                    // dropping leading zeros is fine, and so is dropping x or z bits that repeat the top kept bit
                    const std::string dropped = bits.substr(0, bits.size() - n.width);
                    const char top            = bits.at(bits.size() - n.width);
                    const bool fill_ok        = (top == 'x' || top == 'z') && dropped.find_first_not_of(top) == std::string::npos;
                    if (dropped.find_first_not_of('0') != std::string::npos && !fill_ok)
                    {
                        return ERR("'" + text + "' does not fit in " + std::to_string(n.width) + " bits");
                    }
                    bits = bits.substr(bits.size() - n.width);
                }
                else if (bits.size() < n.width)
                {
                    // Verilog extends with the top bit if it is x or z, with zeros otherwise
                    const char fill = (bits[0] == 'x' || bits[0] == 'z') ? bits[0] : '0';
                    bits            = std::string(n.width - bits.size(), fill) + bits;
                }
            }
            else
            {
                n.width = static_cast<u32>(bits.size());
            }
            n.bits = bits;
            return OK(n);
        }

        // ---------------------------------------------------------------------------------------------------------
        // elaboration
        // ---------------------------------------------------------------------------------------------------------

        namespace
        {
            std::string at(const ast::Location& l)
            {
                return l.line == 0 ? "" : " (" + l.to_string() + ")";
            }

            /**
             * Everything needed while one module is elaborated.
             */
            class ModuleElaborator
            {
            public:
                ModuleElaborator(const ast::SourceFile& file, const ast::Module& src, netlist_ir::Module& dst, const std::set<std::string>& module_names)
                    : m_file(file), m_src(src), m_dst(dst), m_module_names(module_names)
                {
                }

                Result<std::monostate> run()
                {
                    if (auto res = elaborate_parameters(); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = elaborate_declarations(); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = elaborate_header_ports(); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = elaborate_aliases(); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = elaborate_instantiations(); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = elaborate_defparams(); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = convert_attributes(m_src.attributes, m_dst.parameters, "module '" + m_src.name + "'"); res.is_error())
                    {
                        return res;
                    }
                    return OK({});
                }

            private:
                const ast::SourceFile& m_file;
                const ast::Module& m_src;
                netlist_ir::Module& m_dst;
                const std::set<std::string>& m_module_names;

                std::map<std::string, i64> m_parameter_values;     // for constant expressions
                std::map<std::string, const Signal*> m_signals;    // ports and nets by name

                std::string where(const std::string& what, const ast::Location& l) const
                {
                    return what + " in module '" + m_src.name + "'" + at(l);
                }

                // ---- constant expressions ----------------------------------------------------------------------

                Result<i64> evaluate(const ast::Expr& e) const
                {
                    using K = ast::Expr::Kind;
                    switch (e.kind)
                    {
                        case K::Number: {
                            auto n = parse_number(e.text);
                            if (n.is_error())
                            {
                                return ERR(n.get_error());
                            }
                            if (n.get().is_real)
                            {
                                return ERR("'" + e.text + "' is not an integer" + at(e.location));
                            }
                            auto v = n.get().to_u64();
                            if (v.is_error())
                            {
                                return ERR_APPEND(v.get_error(), "'" + e.text + "' is not a usable integer" + at(e.location));
                            }
                            return OK(static_cast<i64>(v.get()));
                        }
                        case K::Identifier: {
                            if (const auto it = m_parameter_values.find(e.text); it != m_parameter_values.end())
                            {
                                return OK(it->second);
                            }
                            return ERR("'" + e.text + "' is not a parameter with an integer value" + at(e.location));
                        }
                        case K::Unary: {
                            auto v = evaluate(e.children.at(0));
                            if (v.is_error())
                            {
                                return v;
                            }
                            if (e.text == "-")
                            {
                                return OK(-v.get());
                            }
                            if (e.text == "+")
                            {
                                return OK(v.get());
                            }
                            if (e.text == "~")
                            {
                                return OK(~v.get());
                            }
                            if (e.text == "!")
                            {
                                return OK(static_cast<i64>(v.get() == 0));
                            }
                            return ERR("operator '" + e.text + "' is not supported in a constant expression" + at(e.location));
                        }
                        case K::Binary: {
                            auto a = evaluate(e.children.at(0));
                            if (a.is_error())
                            {
                                return a;
                            }
                            auto b = evaluate(e.children.at(1));
                            if (b.is_error())
                            {
                                return b;
                            }
                            const i64 x           = a.get();
                            const i64 y           = b.get();
                            const std::string& op = e.text;
                            if (op == "+")
                            {
                                return OK(x + y);
                            }
                            if (op == "-")
                            {
                                return OK(x - y);
                            }
                            if (op == "*")
                            {
                                return OK(x * y);
                            }
                            if (op == "/" || op == "%")
                            {
                                if (y == 0)
                                {
                                    return ERR("division by zero" + at(e.location));
                                }
                                return OK(op == "/" ? x / y : x % y);
                            }
                            if (op == "<<")
                            {
                                return OK(y >= 0 && y < 64 ? x << y : 0);
                            }
                            if (op == ">>")
                            {
                                return OK(y >= 0 && y < 64 ? x >> y : 0);
                            }
                            if (op == "&")
                            {
                                return OK(x & y);
                            }
                            if (op == "|")
                            {
                                return OK(x | y);
                            }
                            if (op == "^")
                            {
                                return OK(x ^ y);
                            }
                            if (op == "==")
                            {
                                return OK(static_cast<i64>(x == y));
                            }
                            if (op == "!=")
                            {
                                return OK(static_cast<i64>(x != y));
                            }
                            if (op == "<")
                            {
                                return OK(static_cast<i64>(x < y));
                            }
                            if (op == ">")
                            {
                                return OK(static_cast<i64>(x > y));
                            }
                            if (op == "<=")
                            {
                                return OK(static_cast<i64>(x <= y));
                            }
                            if (op == ">=")
                            {
                                return OK(static_cast<i64>(x >= y));
                            }
                            if (op == "&&")
                            {
                                return OK(static_cast<i64>(x != 0 && y != 0));
                            }
                            if (op == "||")
                            {
                                return OK(static_cast<i64>(x != 0 || y != 0));
                            }
                            if (op == "**")
                            {
                                i64 r = 1;
                                for (i64 i = 0; i < y; i++)
                                {
                                    r *= x;
                                }
                                return OK(r);
                            }
                            return ERR("operator '" + op + "' is not supported in a constant expression" + at(e.location));
                        }
                        case K::Conditional: {
                            auto c = evaluate(e.children.at(0));
                            if (c.is_error())
                            {
                                return c;
                            }
                            return evaluate(e.children.at(c.get() != 0 ? 1 : 2));
                        }
                        default:
                            return ERR("expected a constant integer expression" + at(e.location));
                    }
                }

                Result<netlist_ir::Range> evaluate_range(const ast::Range& r) const
                {
                    auto left = evaluate(r.left);
                    if (left.is_error())
                    {
                        return ERR_APPEND(left.get_error(), "cannot evaluate the left bound of a range");
                    }
                    auto right = evaluate(r.right);
                    if (right.is_error())
                    {
                        return ERR_APPEND(right.get_error(), "cannot evaluate the right bound of a range");
                    }
                    return OK(netlist_ir::Range{static_cast<i32>(left.get()), static_cast<i32>(right.get())});
                }

                Result<std::vector<netlist_ir::Range>> evaluate_dims(const std::vector<ast::Range>& unpacked, const std::vector<ast::Range>& packed) const
                {
                    // the unpacked dimensions are the outer ones: `wire [3:0] a [1:0]` is indexed a[1][3]
                    std::vector<netlist_ir::Range> dims;
                    for (const auto* list : {&unpacked, &packed})
                    {
                        for (const ast::Range& r : *list)
                        {
                            auto res = evaluate_range(r);
                            if (res.is_error())
                            {
                                return ERR(res.get_error());
                            }
                            dims.push_back(res.get());
                        }
                    }
                    return OK(dims);
                }

                // ---- typed values ------------------------------------------------------------------------------

                /**
                 * Infer the type of a parameter or attribute value from its literal form.
                 */
                Result<TypedValue> typed_value(const std::string& name, const ast::Expr& e, const std::string& what) const
                {
                    using K         = ast::Expr::Kind;
                    const auto make = [&name](Result<Parameter> decl, const std::string& value) -> Result<TypedValue> {
                        if (decl.is_error())
                        {
                            return ERR(decl.get_error());
                        }
                        TypedValue tv;
                        tv.declaration = decl.get();
                        tv.value       = value;
                        return OK(tv);
                    };

                    if (e.kind == K::String)
                    {
                        return make(Parameter::String(name, ""), e.text);
                    }
                    if (e.kind == K::Identifier && m_parameter_values.find(e.text) == m_parameter_values.end())
                    {
                        // an unquoted word, e.g. `(* ATTR = value *)`
                        return make(Parameter::String(name, ""), e.text);
                    }
                    if (e.kind == K::Number)
                    {
                        auto n = parse_number(e.text);
                        if (n.is_error())
                        {
                            return ERR_APPEND(n.get_error(), what + ": invalid value for '" + name + "'" + at(e.location));
                        }
                        const Number& num = n.get();
                        if (num.is_real)
                        {
                            return make(Parameter::Float(name, "0"), num.text);
                        }
                        if (!num.based)
                        {
                            return make(Parameter::Integer(name, "0"), std::to_string(num.to_u64().get()));
                        }
                        if (!num.is_defined())
                        {
                            return make(Parameter::LogicVector(name, static_cast<u16>(num.width), ""), "0b" + num.bits);
                        }
                        return make(Parameter::BitVector(name, static_cast<u16>(num.width), ""), num.to_hex());
                    }
                    // anything else has to be a constant integer expression, e.g. `-1` or `W / 2`
                    auto v = evaluate(e);
                    if (v.is_error())
                    {
                        return ERR_APPEND(v.get_error(), what + ": the value of '" + name + "' is neither a literal nor a constant expression" + at(e.location));
                    }
                    return make(Parameter::Integer(name, "0"), std::to_string(v.get()));
                }

                /**
                 * Append the attributes as typed values with the attribute source; a generic of the same name may sit
                 * in the same list.
                 */
                Result<std::monostate> convert_attributes(const std::vector<ast::Attribute>& src, std::vector<TypedValue>& dst, const std::string& what) const
                {
                    for (const ast::Attribute& a : src)
                    {
                        if (std::any_of(dst.begin(), dst.end(), [&a](const TypedValue& tv) { return tv.declaration.get_source() == Parameter::Source::Attribute && tv.declaration.get_name() == a.name; }))
                        {
                            return ERR(what + ": attribute '" + a.name + "' is given twice" + at(a.location));
                        }
                        if (!a.value.has_value())
                        {
                            TypedValue tv;
                            tv.declaration = Parameter::Boolean(a.name, "false", Parameter::Source::Attribute).get();
                            tv.value       = "true";
                            dst.push_back(tv);
                            continue;
                        }
                        auto tv = typed_value(a.name, a.value.value(), what);
                        if (tv.is_error())
                        {
                            return ERR(tv.get_error());
                        }
                        TypedValue value  = tv.get();
                        value.declaration = value.declaration.with_source(Parameter::Source::Attribute);
                        dst.push_back(value);
                    }
                    return OK({});
                }

                // ---- parameters ----------------------------------------------------------------------------------

                Result<std::monostate> elaborate_parameters()
                {
                    for (const ast::ParameterDecl& p : m_src.parameters)
                    {
                        if (m_parameter_values.find(p.name) != m_parameter_values.end()
                            || std::any_of(m_dst.parameters.begin(), m_dst.parameters.end(), [&p](const TypedValue& tv) { return tv.declaration.get_source() == Parameter::Source::Generic && tv.declaration.get_name() == p.name; }))
                        {
                            return ERR(where("parameter '" + p.name + "' is declared twice", p.location));
                        }
                        auto tv = typed_value(p.name, p.value, where("parameter '" + p.name + "'", p.location));
                        if (tv.is_error())
                        {
                            return ERR(tv.get_error());
                        }
                        // the integer view for ranges and later parameters
                        if (auto v = evaluate(p.value); v.is_ok())
                        {
                            m_parameter_values[p.name] = v.get();
                        }
                        else if (auto n = parse_number(p.value.text); p.value.kind == ast::Expr::Kind::Number && n.is_ok() && n.get().to_u64().is_ok())
                        {
                            m_parameter_values[p.name] = static_cast<i64>(n.get().to_u64().get());
                        }
                        if (!p.is_local)
                        {
                            m_dst.parameters.push_back(tv.get());
                        }
                    }
                    return OK({});
                }

                // ---- declarations --------------------------------------------------------------------------------

                bool is_header_port(const std::string& name) const
                {
                    return std::any_of(m_src.header_ports.begin(), m_src.header_ports.end(), [&name](const ast::HeaderPort& h) { return h.name == name && !h.expression.has_value(); });
                }

                const ast::PortDecl* find_port_decl(const std::string& name) const
                {
                    for (const ast::PortDecl& p : m_src.ports)
                    {
                        if (p.name == name)
                        {
                            return &p;
                        }
                    }
                    return nullptr;
                }

                /**
                 * Every declared name that is not a plain header port becomes a signal. A body port declaration that
                 * is not in the header (a signal of a header port expression) is a signal with its direction ignored.
                 */
                Result<std::monostate> elaborate_declarations()
                {
                    std::set<std::string> declared_ports;
                    for (const ast::PortDecl& p : m_src.ports)
                    {
                        if (!declared_ports.insert(p.name).second)
                        {
                            return ERR(where("port '" + p.name + "' is declared twice", p.location));
                        }
                        if (is_header_port(p.name))
                        {
                            continue;    // becomes a port in header order
                        }
                        if (m_src.ansi_ports)
                        {
                            return ERR(where("port '" + p.name + "' is declared but not in the header", p.location));
                        }
                        auto dims = evaluate_dims(p.unpacked_dims, p.packed_dims);
                        if (dims.is_error())
                        {
                            return ERR_APPEND(dims.get_error(), where("cannot declare '" + p.name + "'", p.location));
                        }
                        Signal& s  = m_dst.add_signal(p.name, dims.get());
                        s.location = p.location;
                        if (auto res = convert_attributes(p.attributes, s.parameters, where("signal '" + p.name + "'", p.location)); res.is_error())
                        {
                            return res;
                        }
                        m_signals[p.name] = &s;
                    }

                    for (const ast::NetDecl& n : m_src.nets)
                    {
                        auto dims = evaluate_dims(n.unpacked_dims, n.packed_dims);
                        if (dims.is_error())
                        {
                            return ERR_APPEND(dims.get_error(), where("cannot declare '" + n.name + "'", n.location));
                        }
                        if (declared_ports.count(n.name) > 0)
                        {
                            // a net declaration of a port repeats the port; the dimensions have to agree
                            const ast::PortDecl* p = find_port_decl(n.name);
                            auto port_dims         = evaluate_dims(p->unpacked_dims, p->packed_dims);
                            if (port_dims.is_error() || port_dims.get() != dims.get())
                            {
                                return ERR(where("net '" + n.name + "' is declared with a different width than the port of that name", n.location));
                            }
                            if (n.initializer.has_value())
                            {
                                return ERR(where("net '" + n.name + "' is a port and cannot have an initializer", n.location));
                            }
                            continue;
                        }
                        if (m_signals.find(n.name) != m_signals.end())
                        {
                            return ERR(where("net '" + n.name + "' is declared twice", n.location));
                        }
                        Signal& s  = m_dst.add_signal(n.name, dims.get());
                        s.location = n.location;
                        if (auto res = convert_attributes(n.attributes, s.parameters, where("net '" + n.name + "'", n.location)); res.is_error())
                        {
                            return res;
                        }
                        m_signals[n.name] = &s;
                    }
                    return OK({});
                }

                Result<std::monostate> elaborate_header_ports()
                {
                    std::set<std::string> seen;
                    for (const ast::HeaderPort& h : m_src.header_ports)
                    {
                        if (!seen.insert(h.name).second)
                        {
                            return ERR(where("port '" + h.name + "' appears twice in the header", h.location));
                        }
                        if (!h.expression.has_value())
                        {
                            const ast::PortDecl* p = find_port_decl(h.name);
                            if (p == nullptr)
                            {
                                return ERR(where("port '" + h.name + "' of the header has no direction declaration", h.location));
                            }
                            if (m_signals.find(h.name) != m_signals.end())
                            {
                                return ERR(where("port '" + h.name + "' is also declared as a net", h.location));
                            }
                            auto dims = evaluate_dims(p->unpacked_dims, p->packed_dims);
                            if (dims.is_error())
                            {
                                return ERR_APPEND(dims.get_error(), where("cannot declare port '" + h.name + "'", p->location));
                            }
                            Port& port    = m_dst.add_port(h.name, p->direction, dims.get());
                            port.location = p->location;
                            if (auto res = convert_attributes(p->attributes, port.parameters, where("port '" + h.name + "'", p->location)); res.is_error())
                            {
                                return res;
                            }
                            m_signals[h.name] = &port;
                            continue;
                        }

                        // `.name(expr)`: the port is as wide as the expression and aliased to it; the direction comes
                        // from the declarations of the expression's signals
                        auto bits = wiring_bits(h.expression.value());
                        if (bits.is_error())
                        {
                            return ERR_APPEND(bits.get_error(), where("cannot resolve the expression of port '" + h.name + "'", h.location));
                        }
                        const std::vector<BitId> expr_bits = bits.get();
                        if (expr_bits.empty())
                        {
                            return ERR(where("the expression of port '" + h.name + "' is empty", h.location));
                        }
                        auto direction = direction_of_expression(h.expression.value());
                        if (direction.is_error())
                        {
                            return ERR_APPEND(direction.get_error(), where("cannot determine the direction of port '" + h.name + "'", h.location));
                        }
                        std::vector<netlist_ir::Range> dims;
                        if (expr_bits.size() > 1)
                        {
                            dims.push_back({static_cast<i32>(expr_bits.size()) - 1, 0});
                        }
                        Port& port    = m_dst.add_port(h.name, direction.get(), dims);
                        port.location = h.location;
                        // the signals of the expression are the real ones, the port only names them outside: the
                        // signal bit comes first so that the merged net carries the signal's name
                        for (u32 i = 0; i < expr_bits.size(); i++)
                        {
                            if (expr_bits.at(i) != OPEN && expr_bits.at(i) != port.bits.at(i))
                            {
                                m_dst.add_alias(expr_bits.at(i), port.bits.at(i));
                            }
                        }
                    }
                    return OK({});
                }

                /**
                 * The direction of a header port expression: every referenced signal must be declared with the same
                 * direction.
                 */
                Result<PinDirection> direction_of_expression(const ast::Expr& e) const
                {
                    std::vector<std::string> names;
                    collect_identifiers(e, names);
                    PinDirection dir = PinDirection::none;
                    for (const std::string& name : names)
                    {
                        const ast::PortDecl* p = find_port_decl(name);
                        if (p == nullptr)
                        {
                            return ERR("'" + name + "' has no direction declaration");
                        }
                        if (dir != PinDirection::none && dir != p->direction)
                        {
                            return ERR("the signals of the expression have different directions");
                        }
                        dir = p->direction;
                    }
                    if (dir == PinDirection::none)
                    {
                        return ERR("the expression names no signal");
                    }
                    return OK(dir);
                }

                static void collect_identifiers(const ast::Expr& e, std::vector<std::string>& out)
                {
                    if (e.kind == ast::Expr::Kind::Identifier)
                    {
                        out.push_back(e.text);
                    }
                    for (const ast::Expr& c : e.children)
                    {
                        collect_identifiers(c, out);
                    }
                }

                // ---- wiring expressions --------------------------------------------------------------------------

                const Signal* find_signal(const std::string& name) const
                {
                    const auto it = m_signals.find(name);
                    return it == m_signals.end() ? nullptr : it->second;
                }

                /**
                 * The bits an expression stands for, MSB first, `OPEN` for x and z.
                 */
                Result<std::vector<BitId>> wiring_bits(const ast::Expr& e) const
                {
                    using K = ast::Expr::Kind;
                    std::vector<BitId> out;
                    switch (e.kind)
                    {
                        case K::Empty:
                            return OK(out);
                        case K::Number: {
                            auto n = parse_number(e.text);
                            if (n.is_error())
                            {
                                return ERR_APPEND(n.get_error(), "invalid number" + at(e.location));
                            }
                            if (n.get().is_real)
                            {
                                return ERR("a real number cannot be connected" + at(e.location));
                            }
                            // an unsized plain decimal is 32 bits wide in Verilog
                            std::string bits = n.get().bits;
                            if (!n.get().sized && !n.get().based && bits.size() < 32)
                            {
                                bits = std::string(32 - bits.size(), '0') + bits;
                            }
                            for (const char c : bits)
                            {
                                out.push_back(c == '0' ? ZERO : c == '1' ? ONE : OPEN);
                            }
                            return OK(out);
                        }
                        case K::String:
                            return ERR("a string cannot be connected" + at(e.location));
                        case K::Identifier: {
                            const Signal* s = find_signal(e.text);
                            if (s == nullptr)
                            {
                                return ERR("'" + e.text + "' is not a declared signal" + at(e.location));
                            }
                            return OK(s->bits);
                        }
                        case K::Index:
                        case K::Slice: {
                            // peel the suffixes: every dimension may carry an index or a range
                            std::vector<Selector> selectors;
                            const ast::Expr* cur = &e;
                            while (cur->kind == K::Index || cur->kind == K::Slice)
                            {
                                Selector sel;
                                if (cur->kind == K::Slice)
                                {
                                    auto left  = evaluate(cur->children.at(1));
                                    auto right = evaluate(cur->children.at(2));
                                    if (left.is_error() || right.is_error())
                                    {
                                        return ERR("cannot evaluate a range" + at(e.location));
                                    }
                                    sel.range = netlist_ir::Range{static_cast<i32>(left.get()), static_cast<i32>(right.get())};
                                }
                                else
                                {
                                    auto v = evaluate(cur->children.at(1));
                                    if (v.is_error())
                                    {
                                        return ERR_APPEND(v.get_error(), "cannot evaluate an index" + at(e.location));
                                    }
                                    sel.index = static_cast<i32>(v.get());
                                }
                                selectors.insert(selectors.begin(), sel);
                                cur = &cur->children.at(0);
                            }
                            if (cur->kind != K::Identifier)
                            {
                                return ERR("only a signal can be indexed" + at(e.location));
                            }
                            const Signal* s = find_signal(cur->text);
                            if (s == nullptr)
                            {
                                return ERR("'" + cur->text + "' is not a declared signal" + at(e.location));
                            }
                            if (selectors.size() > s->dims.size())
                            {
                                return ERR("'" + s->name + "' has fewer dimensions than indices are given" + at(e.location));
                            }
                            return select(*s, selectors, e.location);
                        }
                        case K::Concat:
                            for (const ast::Expr& c : e.children)
                            {
                                auto bits = wiring_bits(c);
                                if (bits.is_error())
                                {
                                    return bits;
                                }
                                const auto b = bits.get();
                                out.insert(out.end(), b.begin(), b.end());
                            }
                            return OK(out);
                        case K::Replicate: {
                            auto count = evaluate(e.children.at(0));
                            if (count.is_error())
                            {
                                return ERR_APPEND(count.get_error(), "cannot evaluate a replication count" + at(e.location));
                            }
                            if (count.get() < 0)
                            {
                                return ERR("negative replication count" + at(e.location));
                            }
                            std::vector<BitId> once;
                            for (u32 i = 1; i < e.children.size(); i++)
                            {
                                auto bits = wiring_bits(e.children.at(i));
                                if (bits.is_error())
                                {
                                    return bits;
                                }
                                const auto b = bits.get();
                                once.insert(once.end(), b.begin(), b.end());
                            }
                            for (i64 i = 0; i < count.get(); i++)
                            {
                                out.insert(out.end(), once.begin(), once.end());
                            }
                            return OK(out);
                        }
                        case K::Unary:
                        case K::Binary:
                        case K::Conditional:
                            return ERR("operator '" + e.text + "' is a logic expression, which a netlist cannot contain; only wiring is supported" + at(e.location));
                    }
                    return ERR("unexpected expression" + at(e.location));
                }

                /**
                 * One dimension of a selection: a single index, a range, or (absent) the whole declared range.
                 */
                struct Selector
                {
                    std::optional<i32> index;
                    std::optional<netlist_ir::Range> range;
                };

                /**
                 * The bits of a signal under the given per-dimension selectors, outer dimensions first, each in the
                 * order its selector lists the indices; trailing dimensions without a selector are taken whole.
                 */
                Result<std::vector<BitId>> select(const Signal& s, const std::vector<Selector>& selectors, const ast::Location& loc) const
                {
                    std::vector<BitId> out;
                    std::vector<i32> indices(s.dims.size(), 0);
                    std::function<Result<std::monostate>(u32)> walk = [&](u32 d) -> Result<std::monostate> {
                        if (d == s.dims.size())
                        {
                            auto bit = s.bit_at(indices);
                            if (bit.is_error())
                            {
                                return ERR_APPEND(bit.get_error(), "invalid index of '" + s.name + "'" + at(loc));
                            }
                            out.push_back(bit.get());
                            return OK({});
                        }
                        netlist_ir::Range r = s.dims.at(d);
                        if (d < selectors.size())
                        {
                            if (selectors.at(d).index.has_value())
                            {
                                r = netlist_ir::Range{selectors.at(d).index.value(), selectors.at(d).index.value()};
                            }
                            else if (selectors.at(d).range.has_value())
                            {
                                r = selectors.at(d).range.value();
                            }
                        }
                        for (u32 k = 0; k < r.size(); k++)
                        {
                            indices.at(d) = r.index_at(k);
                            if (auto res = walk(d + 1); res.is_error())
                            {
                                return res;
                            }
                        }
                        return OK({});
                    };
                    if (auto res = walk(0); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    return OK(out);
                }

                bool all_constant(const std::vector<BitId>& bits) const
                {
                    return !bits.empty() && std::all_of(bits.begin(), bits.end(), [](BitId b) { return b == ZERO || b == ONE || b == OPEN; });
                }

                /**
                 * Alias the bits of a target with those of a source, low bits first; a narrower constant source is
                 * zero-extended, a narrower signal leaves the high target bits alone, a wider source drops its high bits.
                 */
                void alias_bits(const std::vector<BitId>& target, const std::vector<BitId>& source, const std::string& what, const ast::Location& loc)
                {
                    if (target.size() != source.size() && !(source.size() < target.size() && all_constant(source)))
                    {
                        log_warning("verilog_parser", "{}: {} bits are assigned to {} bits, the low bits are paired{}", what, source.size(), target.size(), at(loc));
                    }
                    for (u32 k = 0; k < target.size(); k++)
                    {
                        const BitId t = target.at(target.size() - 1 - k);
                        BitId s       = OPEN;
                        if (k < source.size())
                        {
                            s = source.at(source.size() - 1 - k);
                        }
                        else if (all_constant(source))
                        {
                            s = ZERO;
                        }
                        if (t == OPEN || s == OPEN || t == s)
                        {
                            continue;
                        }
                        if (t == ZERO || t == ONE)
                        {
                            log_warning("verilog_parser", "{}: a constant is assigned to, the assignment is ignored{}", what, at(loc));
                            continue;
                        }
                        m_dst.add_alias(t, s);
                    }
                }

                Result<std::monostate> elaborate_aliases()
                {
                    for (const ast::NetDecl& n : m_src.nets)
                    {
                        const Signal* s = find_signal(n.name);
                        if (s == nullptr)
                        {
                            continue;    // a re-declared port
                        }
                        if (n.net_type == "supply0" || n.net_type == "supply1")
                        {
                            for (const BitId b : s->bits)
                            {
                                m_dst.add_alias(b, n.net_type == "supply0" ? ZERO : ONE);
                            }
                        }
                        if (n.initializer.has_value())
                        {
                            auto bits = wiring_bits(n.initializer.value());
                            if (bits.is_error())
                            {
                                return ERR_APPEND(bits.get_error(), where("cannot resolve the initializer of '" + n.name + "'", n.location));
                            }
                            alias_bits(s->bits, bits.get(), where("initializer of '" + n.name + "'", n.location), n.location);
                        }
                    }

                    for (const ast::Assignment& a : m_src.assignments)
                    {
                        auto lhs = wiring_bits(a.lhs);
                        if (lhs.is_error())
                        {
                            return ERR_APPEND(lhs.get_error(), where("cannot resolve the left side of an assignment", a.location));
                        }
                        auto rhs = wiring_bits(a.rhs);
                        if (rhs.is_error())
                        {
                            return ERR_APPEND(rhs.get_error(), where("cannot resolve the right side of an assignment", a.location));
                        }
                        if (!a.attributes.empty())
                        {
                            log_warning("verilog_parser", "{}: attributes on an assignment have no place in the netlist and are dropped", where("assignment", a.location));
                        }
                        alias_bits(lhs.get(), rhs.get(), where("assignment", a.location), a.location);
                    }
                    return OK({});
                }

                // ---- instantiations ------------------------------------------------------------------------------

                Result<std::monostate> elaborate_instantiations()
                {
                    for (const ast::Instantiation& inst : m_src.instantiations)
                    {
                        const std::string what = where("instance '" + inst.name + "' of type '" + inst.type + "'", inst.location);
                        if (m_dst.find_instance(inst.name) != nullptr)
                        {
                            return ERR(where("instance name '" + inst.name + "' is used twice", inst.location));
                        }
                        const bool is_module = m_module_names.count(inst.type) > 0;
                        Instance& out        = m_dst.add_instance(inst.name, inst.type, is_module ? InstanceKind::Module : InstanceKind::Gate);
                        out.location         = inst.location;

                        for (const ast::Connection& c : inst.connections)
                        {
                            auto bits = wiring_bits(c.expr);
                            if (bits.is_error())
                            {
                                return ERR_APPEND(bits.get_error(), what + ": cannot resolve the connection of port '" + c.port + "'");
                            }
                            if (bits.get().empty() && !c.port.empty())
                            {
                                continue;    // `.a()` connects nothing
                            }
                            if (!c.port.empty() && out.find_connection(c.port) != nullptr)
                            {
                                return ERR(what + ": port '" + c.port + "' is connected twice" + at(c.location));
                            }
                            out.add_connection(c.port, bits.get());
                        }

                        // parameters: named, or positional against the module's declared order
                        const ast::Module* target = nullptr;
                        if (is_module)
                        {
                            for (const ast::Module& m : m_file.modules)
                            {
                                if (m.name == inst.type)
                                {
                                    target = &m;
                                }
                            }
                        }
                        u32 positional = 0;
                        for (const ast::ParameterAssignment& p : inst.parameters)
                        {
                            std::string name = p.name;
                            if (name.empty())
                            {
                                if (target == nullptr)
                                {
                                    return ERR(what + ": positional parameters are only supported for modules of this file, not for gate types" + at(p.location));
                                }
                                u32 seen = 0;
                                for (const ast::ParameterDecl& d : target->parameters)
                                {
                                    if (!d.is_local && seen++ == positional)
                                    {
                                        name = d.name;
                                    }
                                }
                                if (name.empty())
                                {
                                    return ERR(what + ": more positional parameters than module '" + inst.type + "' declares" + at(p.location));
                                }
                                positional++;
                            }
                            if (p.value.is_empty())
                            {
                                continue;
                            }
                            auto tv = typed_value(name, p.value, what);
                            if (tv.is_error())
                            {
                                return ERR(tv.get_error());
                            }
                            if (std::any_of(out.parameters.begin(), out.parameters.end(), [&name](const TypedValue& t) { return t.declaration.get_source() == Parameter::Source::Generic && t.declaration.get_name() == name; }))
                            {
                                return ERR(what + ": parameter '" + name + "' is set twice" + at(p.location));
                            }
                            out.parameters.push_back(tv.get());
                        }

                        if (auto res = convert_attributes(inst.attributes, out.parameters, what); res.is_error())
                        {
                            return res;
                        }
                    }
                    return OK({});
                }

                Result<std::monostate> elaborate_defparams()
                {
                    for (const ast::Defparam& d : m_src.defparams)
                    {
                        if (d.path.size() != 2)
                        {
                            return ERR(where("defparam with a hierarchical path is not supported", d.location));
                        }
                        Instance* inst = m_dst.find_instance(d.path.front());
                        if (inst == nullptr)
                        {
                            return ERR(where("defparam refers to instance '" + d.path.front() + "', which does not exist", d.location));
                        }
                        auto tv = typed_value(d.path.back(), d.value, where("defparam of '" + d.path.front() + "'", d.location));
                        if (tv.is_error())
                        {
                            return ERR(tv.get_error());
                        }
                        // a defparam overrides an earlier value
                        inst->parameters.erase(std::remove_if(inst->parameters.begin(), inst->parameters.end(), [&d](const TypedValue& t) { return t.declaration.get_source() == Parameter::Source::Generic && t.declaration.get_name() == d.path.back(); }),
                                               inst->parameters.end());
                        inst->parameters.push_back(tv.get());
                    }
                    return OK({});
                }
            };
        }    // namespace

        Result<netlist_ir::Design> elaborate(const ast::SourceFile& file)
        {
            Design design;
            design.source = file.file;

            std::set<std::string> module_names;
            for (const ast::Module& m : file.modules)
            {
                if (!module_names.insert(m.name).second)
                {
                    return ERR("module '" + m.name + "' is declared twice in '" + file.file + "'" + at(m.location));
                }
            }

            std::vector<std::string> marked_top;
            for (const ast::Module& src : file.modules)
            {
                netlist_ir::Module& dst = design.add_module(src.name);
                dst.location            = src.location;
                ModuleElaborator elaborator(file, src, dst, module_names);
                if (auto res = elaborator.run(); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not elaborate '" + file.file + "'");
                }
                for (const ast::Attribute& a : src.attributes)
                {
                    if (a.name == "top" && a.value.has_value() && (a.value->text == "1" || a.value->text == "1'b1" || a.value->text == "true"))
                    {
                        marked_top.push_back(src.name);
                    }
                }
            }

            if (marked_top.size() > 1)
            {
                return ERR("could not elaborate '" + file.file + "': more than one module is marked as the top module: " + [&marked_top]() {
                    std::string s;
                    for (const auto& n : marked_top)
                    {
                        s += (s.empty() ? "" : ", ") + n;
                    }
                    return s;
                }());
            }
            if (marked_top.size() == 1)
            {
                design.top = marked_top.front();
            }

            if (auto res = design.validate(); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not elaborate '" + file.file + "'");
            }
            return OK(design);
        }
    }    // namespace verilog
}    // namespace hal
