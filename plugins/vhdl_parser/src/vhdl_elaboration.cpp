#include "vhdl_parser/vhdl_elaboration.h"

#include "hal_core/utilities/log.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <map>
#include <set>

namespace hal
{
    namespace vhdl
    {
        using namespace netlist_ir;

        namespace
        {
            std::string lower(const std::string& s)
            {
                std::string r = s;
                for (char& c : r)
                {
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                return r;
            }

            std::string at(const Location& loc)
            {
                return " (line " + std::to_string(loc.line) + ", column " + std::to_string(loc.column) + ")";
            }

            std::string strip_underscores(const std::string& s)
            {
                std::string r;
                for (char c : s)
                {
                    if (c != '_')
                    {
                        r += c;
                    }
                }
                return r;
            }

            i32 digit_value(char c)
            {
                if (c >= '0' && c <= '9')
                    return c - '0';
                if (c >= 'a' && c <= 'f')
                    return 10 + c - 'a';
                if (c >= 'A' && c <= 'F')
                    return 10 + c - 'A';
                return -1;
            }

            bool is_real_literal(const std::string& text)
            {
                const auto hash = text.find('#');
                if (hash != std::string::npos)
                {
                    const auto close = text.find('#', hash + 1);
                    return text.find('.', hash) < close;
                }
                return text.find('.') != std::string::npos;
            }
        }    // namespace

        Result<i64> parse_integer(const std::string& text)
        {
            if (is_real_literal(text))
            {
                return ERR("'" + text + "' is a real, not an integer");
            }
            const std::string s = strip_underscores(text);
            i64 base            = 10;
            std::string digits  = s;
            std::string exponent;
            const auto hash = s.find('#');
            if (hash != std::string::npos)
            {
                const auto close = s.find('#', hash + 1);
                if (close == std::string::npos)
                {
                    return ERR("invalid based literal '" + text + "'");
                }
                base     = std::stoll(s.substr(0, hash));
                digits   = s.substr(hash + 1, close - hash - 1);
                exponent = s.substr(close + 1);
                if (base < 2 || base > 16)
                {
                    return ERR("invalid base in '" + text + "'");
                }
            }
            else
            {
                const auto e = s.find_first_of("eE");
                if (e != std::string::npos)
                {
                    digits   = s.substr(0, e);
                    exponent = s.substr(e);
                }
            }
            if (digits.empty())
            {
                return ERR("invalid integer literal '" + text + "'");
            }
            i64 value = 0;
            for (char c : digits)
            {
                const i32 d = digit_value(c);
                if (d < 0 || d >= base)
                {
                    return ERR("invalid digit '" + std::string(1, c) + "' in '" + text + "'");
                }
                if (value > (std::numeric_limits<i64>::max() - d) / base)
                {
                    return ERR("'" + text + "' does not fit in 64 bits");
                }
                value = value * base + d;
            }
            if (!exponent.empty())
            {
                std::string e = exponent;
                if (e.front() == 'e' || e.front() == 'E')
                {
                    e = e.substr(1);
                }
                if (e.empty() || e.front() == '-' || e.find_first_not_of("+0123456789") != std::string::npos)
                {
                    return ERR("invalid exponent in integer literal '" + text + "'");
                }
                const i64 n = std::stoll(e);
                for (i64 i = 0; i < n; i++)
                {
                    if (value > std::numeric_limits<i64>::max() / base)
                    {
                        return ERR("'" + text + "' does not fit in 64 bits");
                    }
                    value *= base;
                }
            }
            return OK(value);
        }

        Result<std::string> expand_bit_string(const std::string& text)
        {
            const auto quote = text.find('"');
            if (quote == std::string::npos || text.back() != '"')
            {
                return ERR("invalid bit string literal '" + text + "'");
            }
            std::string prefix = lower(text.substr(0, quote));
            std::string body   = strip_underscores(text.substr(quote + 1, text.size() - quote - 2));

            std::optional<u32> size;
            u32 i = 0;
            while (i < prefix.size() && std::isdigit(static_cast<unsigned char>(prefix[i])))
            {
                i++;
            }
            if (i > 0)
            {
                size = static_cast<u32>(std::stoul(prefix.substr(0, i)));
            }
            bool is_signed = false;
            if (i < prefix.size() && (prefix[i] == 'u' || prefix[i] == 's'))
            {
                is_signed = prefix[i] == 's';
                i++;
            }
            if (i + 1 != prefix.size())
            {
                return ERR("invalid bit string literal '" + text + "'");
            }
            const char base = prefix[i];

            std::string bits;
            if (base == 'd')
            {
                if (body.empty() || body.find_first_not_of("0123456789") != std::string::npos)
                {
                    return ERR("invalid decimal bit string literal '" + text + "'");
                }
                // repeated division by two, so that any length works
                std::string number = body;
                while (!(number.size() == 1 && number.front() == '0'))
                {
                    std::string quotient;
                    i32 remainder = 0;
                    for (char c : number)
                    {
                        const i32 cur = remainder * 10 + (c - '0');
                        quotient += static_cast<char>('0' + cur / 2);
                        remainder = cur % 2;
                    }
                    bits.insert(bits.begin(), static_cast<char>('0' + remainder));
                    const auto nz = quotient.find_first_not_of('0');
                    number        = nz == std::string::npos ? "0" : quotient.substr(nz);
                }
                if (bits.empty())
                {
                    bits = "0";
                }
            }
            else
            {
                const u32 per_digit = base == 'b' ? 1 : base == 'o' ? 3 : 4;
                for (char c : body)
                {
                    const i32 d = digit_value(c);
                    if (d >= 0 && d < (1 << per_digit))
                    {
                        for (i32 k = static_cast<i32>(per_digit) - 1; k >= 0; k--)
                        {
                            bits += ((d >> k) & 1) ? '1' : '0';
                        }
                    }
                    else if (std::strchr("xzuw-lh", std::tolower(static_cast<unsigned char>(c))) != nullptr && d < 0)
                    {
                        bits += std::string(per_digit, static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                    }
                    else
                    {
                        return ERR("invalid digit '" + std::string(1, c) + "' in bit string literal '" + text + "'");
                    }
                }
            }

            if (size.has_value())
            {
                if (bits.size() > size.value())
                {
                    const std::string dropped = bits.substr(0, bits.size() - size.value());
                    const char fill           = is_signed ? bits.at(bits.size() - size.value()) : '0';
                    if (dropped.find_first_not_of(fill) != std::string::npos)
                    {
                        return ERR("bit string literal '" + text + "' does not fit in " + std::to_string(size.value()) + " bits");
                    }
                    bits = bits.substr(bits.size() - size.value());
                }
                else if (bits.size() < size.value())
                {
                    const char fill = (is_signed && !bits.empty()) ? bits.front() : '0';
                    bits.insert(0, size.value() - bits.size(), fill);
                }
            }
            return OK(bits);
        }

        namespace
        {
            using namespace ast;

            struct ArchitectureChoice
            {
                const Entity* entity            = nullptr;
                const Architecture* architecture = nullptr;
            };

            /**
             * A bit of a literal or a signal in wiring: `0`, `1`, or nothing for `X`, `Z`, `U`, `W`, `-`.
             */
            Result<BitId> bit_of_char(char c)
            {
                switch (std::toupper(static_cast<unsigned char>(c)))
                {
                    case '0':
                    case 'L':
                        return OK(ZERO);
                    case '1':
                    case 'H':
                        return OK(ONE);
                    case 'X':
                    case 'Z':
                    case 'U':
                    case 'W':
                    case '-':
                        return OK(OPEN);
                    default:
                        return ERR(std::string("'") + c + "' is not a std_logic value");
                }
            }

            /**
             * What the file declares outside of any entity: packages, configurations, and the entities themselves.
             */
            struct GlobalScope
            {
                std::map<std::string, const Entity*> entities;                       /**< By key. */
                std::map<std::string, std::vector<const Architecture*>> architectures; /**< By entity key. */
                std::map<std::string, const ComponentDecl*> components;               /**< From packages, by key. */
                std::map<std::string, const ObjectDecl*> constants;                   /**< From packages, by key. */
                std::map<std::string, const TypeDecl*> types;                         /**< From packages, by key. */
                std::map<std::string, const AttributeDecl*> attribute_decls;          /**< From packages, by key. */
                std::map<std::string, Configuration::Binding> bindings;               /**< Component key to binding, from configurations. */
                std::map<std::string, std::string> default_module;                    /**< Entity key to the IR module of its default architecture, if that one is in use. */
                std::map<std::pair<std::string, std::string>, std::string> modules;   /**< (entity key, architecture key) to the IR module name. */

                /**
                 * The IR module an instantiation of an entity refers to: the one of the named architecture, else the
                 * default one; empty if it does not exist.
                 */
                std::string module_of(const std::string& entity_key, const std::optional<std::string>& architecture_key) const
                {
                    if (architecture_key.has_value())
                    {
                        const auto it = modules.find({entity_key, architecture_key.value()});
                        return it == modules.end() ? "" : it->second;
                    }
                    const auto it = default_module.find(entity_key);
                    return it == default_module.end() ? "" : it->second;
                }
            };

            /**
             * Elaborates one entity with its architecture into an IR module.
             */
            class ModuleElaborator
            {
            public:
                ModuleElaborator(const GlobalScope& global, const Entity& entity, const Architecture* arch, netlist_ir::Module& dst)
                    : m_global(global), m_entity(entity), m_arch(arch), m_dst(dst)
                {
                }

                Result<std::monostate> run()
                {
                    if (auto res = collect_declarations(); res.is_error())
                        return res;
                    if (auto res = elaborate_generics(); res.is_error())
                        return res;
                    if (auto res = elaborate_ports(); res.is_error())
                        return res;
                    if (m_arch == nullptr)
                    {
                        return OK({});
                    }
                    if (auto res = elaborate_signals(); res.is_error())
                        return res;
                    if (auto res = elaborate_instantiations(); res.is_error())
                        return res;
                    if (auto res = elaborate_assignments(); res.is_error())
                        return res;
                    if (auto res = elaborate_attributes(m_entity.declarations); res.is_error())
                        return res;
                    if (auto res = elaborate_attributes(m_arch->declarations); res.is_error())
                        return res;
                    return OK({});
                }

            private:
                const GlobalScope& m_global;
                const Entity& m_entity;
                const Architecture* m_arch;
                netlist_ir::Module& m_dst;

                std::map<std::string, const ComponentDecl*> m_components;
                std::map<std::string, const ObjectDecl*> m_constants;
                std::map<std::string, const TypeDecl*> m_types;
                std::map<std::string, const AttributeDecl*> m_attribute_decls;
                std::map<std::string, const InterfaceDecl*> m_generics;
                std::map<std::string, i64> m_integer_values;        /**< Generics and constants with an integer value, by key. */
                std::map<std::string, TypedValue> m_typed_values;   /**< Generics and constants with a typed value, by key. */
                std::map<std::string, Signal*> m_signals;           /**< Ports and signals, by key. */
                std::map<std::string, Instance*> m_instances;       /**< By key. */

                std::string where(const std::string& msg, const Location& loc) const
                {
                    return "entity '" + m_entity.name.text + "': " + msg + at(loc);
                }

                // ---- scopes ---------------------------------------------------------------------------------------

                void collect(const Declarations& d)
                {
                    for (const ComponentDecl& c : d.components)
                        m_components[c.name.key()] = &c;
                    for (const ObjectDecl& o : d.objects)
                        if (o.is_constant)
                            m_constants[o.name.key()] = &o;
                    for (const TypeDecl& t : d.types)
                        m_types[t.name.key()] = &t;
                    for (const AttributeDecl& a : d.attribute_decls)
                        m_attribute_decls[a.name.key()] = &a;
                }

                Result<std::monostate> collect_declarations()
                {
                    m_components      = m_global.components;
                    m_constants       = m_global.constants;
                    m_types           = m_global.types;
                    m_attribute_decls = m_global.attribute_decls;
                    collect(m_entity.declarations);
                    if (m_arch != nullptr)
                    {
                        collect(m_arch->declarations);
                    }
                    for (const InterfaceDecl& g : m_entity.generics)
                    {
                        if (!m_generics.emplace(g.name.key(), &g).second)
                        {
                            return ERR(where("generic '" + g.name.text + "' is declared twice", g.location));
                        }
                    }
                    return OK({});
                }

                const ComponentDecl* find_component(const Name& n) const
                {
                    const auto it = m_components.find(n.key());
                    return it == m_components.end() ? nullptr : it->second;
                }

                const TypeDecl* find_type(const Name& n) const
                {
                    const auto it = m_types.find(n.key());
                    return it == m_types.end() ? nullptr : it->second;
                }

                Signal* find_signal(const Name& n) const
                {
                    const auto it = m_signals.find(n.key());
                    return it == m_signals.end() ? nullptr : it->second;
                }

                // ---- constant expressions -----------------------------------------------------------------------

                Result<i64> evaluate(const Expr& e) const
                {
                    using K = Expr::Kind;
                    switch (e.kind)
                    {
                        case K::Number:
                            return parse_integer(e.text);
                        case K::Identifier: {
                            const auto it = m_integer_values.find(e.name.key());
                            if (it != m_integer_values.end())
                            {
                                return OK(it->second);
                            }
                            const auto c = m_constants.find(e.name.key());
                            if (c != m_constants.end() && c->second->initializer.has_value())
                            {
                                return evaluate(c->second->initializer.value());
                            }
                            return ERR("'" + e.name.text + "' is not a generic or constant with an integer value" + at(e.location));
                        }
                        case K::Unary: {
                            auto v = evaluate(e.children.at(0));
                            if (v.is_error())
                                return v;
                            if (e.text == "-")
                                return OK(-v.get());
                            if (e.text == "+")
                                return v;
                            if (e.text == "abs")
                                return OK(std::abs(v.get()));
                            return ERR("operator '" + e.text + "' is not an integer operator" + at(e.location));
                        }
                        case K::Binary: {
                            auto l = evaluate(e.children.at(0));
                            if (l.is_error())
                                return l;
                            auto r = evaluate(e.children.at(1));
                            if (r.is_error())
                                return r;
                            const i64 a = l.get();
                            const i64 b = r.get();
                            if (e.text == "+")
                                return OK(a + b);
                            if (e.text == "-")
                                return OK(a - b);
                            if (e.text == "*")
                                return OK(a * b);
                            if (e.text == "/" || e.text == "mod" || e.text == "rem")
                            {
                                if (b == 0)
                                    return ERR("division by zero" + at(e.location));
                                if (e.text == "/")
                                    return OK(a / b);
                                if (e.text == "rem")
                                    return OK(a % b);
                                const i64 m = a % b;
                                return OK((m != 0 && ((m < 0) != (b < 0))) ? m + b : m);
                            }
                            if (e.text == "**")
                            {
                                if (b < 0)
                                    return ERR("negative exponent" + at(e.location));
                                i64 p = 1;
                                for (i64 i = 0; i < b; i++)
                                    p *= a;
                                return OK(p);
                            }
                            return ERR("operator '" + e.text + "' is not an integer operator" + at(e.location));
                        }
                        case K::Attribute: {
                            const Expr& base = e.children.at(0);
                            if (base.kind != K::Identifier)
                            {
                                return ERR("attribute '" + e.text + "' of an expression is not supported" + at(e.location));
                            }
                            std::vector<netlist_ir::Range> dims;
                            if (const Signal* s = find_signal(base.name); s != nullptr)
                            {
                                dims = s->dims;
                            }
                            else if (const TypeDecl* t = find_type(base.name); t != nullptr)
                            {
                                TypeMark tm;
                                tm.name  = base.name;
                                auto res = resolve_dims(tm);
                                if (res.is_error())
                                    return ERR(res.get_error());
                                dims = res.get();
                            }
                            else
                            {
                                return ERR("'" + base.name.text + "' is not a signal or type" + at(e.location));
                            }
                            if (dims.empty())
                            {
                                return ERR("'" + base.name.text + "' has no index range" + at(e.location));
                            }
                            const netlist_ir::Range& r = dims.front();
                            if (e.text == "length")
                                return OK(static_cast<i64>(r.size()));
                            if (e.text == "left")
                                return OK(static_cast<i64>(r.left));
                            if (e.text == "right")
                                return OK(static_cast<i64>(r.right));
                            if (e.text == "high")
                                return OK(static_cast<i64>(std::max(r.left, r.right)));
                            if (e.text == "low")
                                return OK(static_cast<i64>(std::min(r.left, r.right)));
                            return ERR("attribute '" + e.text + "' is not supported in a constant expression" + at(e.location));
                        }
                        default:
                            return ERR("not a constant integer expression" + at(e.location));
                    }
                }

                Result<netlist_ir::Range> evaluate_range(const ast::Range& r) const
                {
                    auto l = evaluate(r.left);
                    if (l.is_error())
                        return ERR(l.get_error());
                    auto h = evaluate(r.right);
                    if (h.is_error())
                        return ERR(h.get_error());
                    if (l.get() < std::numeric_limits<i32>::min() || l.get() > std::numeric_limits<i32>::max() || h.get() < std::numeric_limits<i32>::min() || h.get() > std::numeric_limits<i32>::max())
                    {
                        return ERR("range bound out of the supported range" + at(r.left.location));
                    }
                    const bool descending_written = r.descending;
                    if ((descending_written && l.get() < h.get()) || (!descending_written && l.get() > h.get()))
                    {
                        return ERR("null range " + std::to_string(l.get()) + (descending_written ? " downto " : " to ") + std::to_string(h.get()) + at(r.left.location));
                    }
                    return OK(netlist_ir::Range{static_cast<i32>(l.get()), static_cast<i32>(h.get())});
                }

                // ---- types ----------------------------------------------------------------------------------------

                static bool is_scalar_type(const std::string& key)
                {
                    return key == "std_logic" || key == "std_ulogic" || key == "bit" || key == "boolean";
                }

                static u32 vector_dimensions(const std::string& key)
                {
                    if (key == "std_logic_vector" || key == "std_ulogic_vector" || key == "bit_vector" || key == "signed" || key == "unsigned")
                        return 1;
                    if (key == "std_logic_vector2")
                        return 2;
                    if (key == "std_logic_vector3")
                        return 3;
                    return 0;
                }

                /**
                 * The dimensions of a signal of the given type: none for a scalar, one range per dimension otherwise.
                 */
                Result<std::vector<netlist_ir::Range>> resolve_dims(const TypeMark& tm, u32 depth = 0) const
                {
                    if (depth > 16)
                    {
                        return ERR("type '" + tm.name.text + "' refers to itself" + at(tm.location));
                    }
                    std::vector<netlist_ir::Range> constraints;
                    for (const ast::Range& r : tm.constraints)
                    {
                        auto res = evaluate_range(r);
                        if (res.is_error())
                        {
                            return ERR_APPEND(res.get_error(), "invalid constraint of type '" + tm.name.text + "'" + at(tm.location));
                        }
                        constraints.push_back(res.get());
                    }
                    const std::string key = tm.name.key();
                    if (is_scalar_type(key))
                    {
                        if (!constraints.empty())
                        {
                            return ERR("scalar type '" + tm.name.text + "' cannot have an index constraint" + at(tm.location));
                        }
                        return OK(constraints);
                    }
                    if (const u32 n = vector_dimensions(key); n > 0)
                    {
                        if (constraints.size() != n)
                        {
                            return ERR("type '" + tm.name.text + "' needs " + std::to_string(n) + " index range(s), " + std::to_string(constraints.size()) + " given" + at(tm.location));
                        }
                        return OK(constraints);
                    }
                    const TypeDecl* decl = find_type(tm.name);
                    if (decl == nullptr)
                    {
                        if (key == "integer" || key == "natural" || key == "positive" || key == "real" || key == "string" || key == "time")
                        {
                            return ERR("type '" + tm.name.text + "' is not a bit type; a signal of it has no place in a netlist" + at(tm.location));
                        }
                        return ERR("unknown type '" + tm.name.text + "'" + at(tm.location));
                    }
                    if (decl->subtype.has_value())
                    {
                        auto base = resolve_dims(decl->subtype.value(), depth + 1);
                        if (base.is_error())
                            return base;
                        if (!constraints.empty())
                        {
                            return ERR("subtype '" + tm.name.text + "' cannot be constrained again" + at(tm.location));
                        }
                        return base;
                    }
                    if (!decl->is_array)
                    {
                        return ERR("type '" + tm.name.text + "' is not an array of bits" + at(tm.location));
                    }
                    std::vector<netlist_ir::Range> dims;
                    if (decl->ranges.empty())
                    {
                        if (constraints.empty())
                        {
                            return ERR("unconstrained array type '" + tm.name.text + "' needs an index constraint" + at(tm.location));
                        }
                        dims = constraints;
                    }
                    else
                    {
                        if (!constraints.empty())
                        {
                            return ERR("constrained array type '" + tm.name.text + "' cannot be constrained again" + at(tm.location));
                        }
                        for (const ast::Range& r : decl->ranges)
                        {
                            auto res = evaluate_range(r);
                            if (res.is_error())
                            {
                                return ERR_APPEND(res.get_error(), "invalid range of type '" + decl->name.text + "'" + at(decl->location));
                            }
                            dims.push_back(res.get());
                        }
                    }
                    auto element = resolve_dims(decl->element, depth + 1);
                    if (element.is_error())
                    {
                        return ERR_APPEND(element.get_error(), "invalid element type of '" + decl->name.text + "'" + at(decl->location));
                    }
                    dims.insert(dims.end(), element.get().begin(), element.get().end());
                    return OK(dims);
                }

                // ---- typed values ---------------------------------------------------------------------------------

                static Result<TypedValue> make_typed(Result<Parameter> decl, const std::string& value)
                {
                    if (decl.is_error())
                    {
                        return ERR(decl.get_error());
                    }
                    TypedValue tv;
                    tv.declaration = decl.get();
                    tv.value       = value;
                    return OK(tv);
                }

                static std::string bits_to_hex(const std::string& bits)
                {
                    std::string hex;
                    const u32 pad = (4 - bits.size() % 4) % 4;
                    const std::string padded = std::string(pad, '0') + bits;
                    for (u32 i = 0; i < padded.size(); i += 4)
                    {
                        i32 nibble = 0;
                        for (u32 k = 0; k < 4; k++)
                        {
                            nibble = nibble * 2 + (padded[i + k] - '0');
                        }
                        if (nibble != 0 || !hex.empty())
                        {
                            hex += "0123456789ABCDEF"[nibble];
                        }
                    }
                    return "0x" + (hex.empty() ? std::string("0") : hex);
                }

                static Result<TypedValue> typed_bits(const std::string& name, const std::string& bits)
                {
                    if (bits.find_first_not_of("01") == std::string::npos)
                    {
                        return make_typed(Parameter::BitVector(name, static_cast<u16>(bits.size()), ""), bits_to_hex(bits));
                    }
                    std::string logic;
                    for (char c : bits)
                    {
                        // LogicVector knows 0, 1, x and z
                        logic += (c == '0' || c == '1' || c == 'x' || c == 'z') ? c : c == 'l' ? '0' : c == 'h' ? '1' : 'x';
                    }
                    return make_typed(Parameter::LogicVector(name, static_cast<u16>(bits.size()), ""), "0b" + logic);
                }

                static std::string normalize_time(const std::string& text)
                {
                    // `1.234 sec` -> `1.234s`, `10 ns` -> `10ns`, `1 hr` -> `1h`
                    const auto space   = text.find(' ');
                    std::string number = strip_underscores(text.substr(0, space));
                    std::string unit   = space == std::string::npos ? "" : text.substr(space + 1);
                    if (unit == "sec")
                        unit = "s";
                    else if (unit == "hr")
                        unit = "h";
                    return number + unit;
                }

                /**
                 * The typed value of a generic or attribute from its literal form, or from the declared type when the
                 * target declares one (an entity generic, or an attribute declaration).
                 */
                Result<TypedValue> typed_value(const std::string& name, const Expr& e, const std::optional<std::string>& declared_type, const std::string& what, bool allow_enum_literal = true) const
                {
                    using K               = Expr::Kind;
                    const std::string key = declared_type.value_or("");

                    if (key == "string")
                    {
                        if (e.kind == K::String || e.kind == K::Character || e.kind == K::Number || e.kind == K::BitString)
                        {
                            return make_typed(Parameter::String(name, ""), e.text);
                        }
                        if (e.kind == K::Identifier)
                        {
                            if (const auto it = m_typed_values.find(e.name.key()); it != m_typed_values.end())
                            {
                                return make_typed(Parameter::String(name, ""), it->second.value);
                            }
                            return make_typed(Parameter::String(name, ""), e.name.text);
                        }
                    }
                    if (key == "boolean")
                    {
                        if (e.kind == K::Identifier && (e.name.key() == "true" || e.name.key() == "false"))
                        {
                            return make_typed(Parameter::Boolean(name, "false"), e.name.key());
                        }
                        return ERR(what + ": '" + name + "' is a boolean, the value must be 'true' or 'false'" + at(e.location));
                    }
                    if (key == "integer" || key == "natural" || key == "positive")
                    {
                        auto v = evaluate(e);
                        if (v.is_error())
                        {
                            return ERR_APPEND(v.get_error(), what + ": '" + name + "' is an integer, the value is not a constant integer expression" + at(e.location));
                        }
                        return make_typed(Parameter::Integer(name, "0"), std::to_string(v.get()));
                    }
                    if (key == "real")
                    {
                        if (e.kind == K::Number)
                        {
                            return make_typed(Parameter::Float(name, "0"), strip_underscores(e.text));
                        }
                        if (e.kind == K::Unary && e.text == "-" && e.children.at(0).kind == K::Number)
                        {
                            return make_typed(Parameter::Float(name, "0"), "-" + strip_underscores(e.children.at(0).text));
                        }
                        return ERR(what + ": '" + name + "' is a real, the value must be a numeric literal" + at(e.location));
                    }
                    if (key == "time")
                    {
                        if (e.kind == K::Number && e.text.find(' ') != std::string::npos)
                        {
                            return make_typed(Parameter::Time(name, "0s"), normalize_time(e.text));
                        }
                        return ERR(what + ": '" + name + "' is a time, the value must be a physical literal such as '10 ns'" + at(e.location));
                    }
                    if (key == "bit" || key == "std_logic" || key == "std_ulogic")
                    {
                        if (e.kind == K::Character)
                        {
                            return typed_bits(name, lower(e.text));
                        }
                        return ERR(what + ": '" + name + "' is a bit, the value must be a character literal" + at(e.location));
                    }
                    if (vector_dimensions(key) == 1)
                    {
                        if (e.kind == K::String)
                        {
                            return typed_bits(name, lower(e.text));
                        }
                        if (e.kind == K::BitString)
                        {
                            auto bits = expand_bit_string(e.text);
                            if (bits.is_error())
                                return ERR_APPEND(bits.get_error(), what + ": invalid value for '" + name + "'" + at(e.location));
                            return typed_bits(name, bits.get());
                        }
                        if (e.kind == K::Identifier)
                        {
                            if (const auto it = m_typed_values.find(e.name.key()); it != m_typed_values.end())
                            {
                                TypedValue tv   = it->second;
                                auto redeclared = Parameter::BitVector(name, tv.declaration.get_size(), "");
                                if (tv.declaration.get_type() == Parameter::Type::BitVector && redeclared.is_ok())
                                {
                                    tv.declaration = redeclared.get();
                                    return OK(tv);
                                }
                            }
                        }
                        return ERR(what + ": '" + name + "' is a bit vector, the value must be a string or bit string literal" + at(e.location));
                    }

                    // no declared type: infer from the literal
                    switch (e.kind)
                    {
                        case K::String:
                            return make_typed(Parameter::String(name, ""), e.text);
                        case K::Character:
                            if (e.text == "0" || e.text == "1")
                            {
                                return typed_bits(name, e.text);
                            }
                            return make_typed(Parameter::String(name, ""), e.text);
                        case K::BitString: {
                            auto bits = expand_bit_string(e.text);
                            if (bits.is_error())
                                return ERR_APPEND(bits.get_error(), what + ": invalid value for '" + name + "'" + at(e.location));
                            return typed_bits(name, bits.get());
                        }
                        case K::Number: {
                            if (e.text.find(' ') != std::string::npos)
                            {
                                return make_typed(Parameter::Time(name, "0s"), normalize_time(e.text));
                            }
                            if (is_real_literal(e.text))
                            {
                                return make_typed(Parameter::Float(name, "0"), strip_underscores(e.text));
                            }
                            auto v = parse_integer(e.text);
                            if (v.is_error())
                                return ERR_APPEND(v.get_error(), what + ": invalid value for '" + name + "'" + at(e.location));
                            return make_typed(Parameter::Integer(name, "0"), std::to_string(v.get()));
                        }
                        case K::Identifier: {
                            if (e.name.key() == "true" || e.name.key() == "false")
                            {
                                return make_typed(Parameter::Boolean(name, "false"), e.name.key());
                            }
                            if (const auto it = m_typed_values.find(e.name.key()); it != m_typed_values.end())
                            {
                                // a generic or constant: the same value under the new name
                                TypedValue tv = it->second;
                                const Parameter& d = tv.declaration;
                                Result<Parameter> redeclared = ERR("");
                                switch (d.get_type())
                                {
                                    case Parameter::Type::Boolean:
                                        redeclared = Parameter::Boolean(name, "false");
                                        break;
                                    case Parameter::Type::BitVector:
                                        redeclared = Parameter::BitVector(name, d.get_size(), "");
                                        break;
                                    case Parameter::Type::LogicVector:
                                        redeclared = Parameter::LogicVector(name, d.get_size(), "");
                                        break;
                                    case Parameter::Type::Integer:
                                        redeclared = Parameter::Integer(name, "0");
                                        break;
                                    case Parameter::Type::String:
                                        redeclared = Parameter::String(name, "");
                                        break;
                                    case Parameter::Type::Float:
                                        redeclared = Parameter::Float(name, "0");
                                        break;
                                    case Parameter::Type::Time:
                                        redeclared = Parameter::Time(name, "0s");
                                        break;
                                    case Parameter::Type::Enum:
                                        redeclared = Parameter::Enum(name, d.get_enum_values(), d.get_default_value());
                                        break;
                                }
                                return make_typed(redeclared, tv.value);
                            }
                            if (e.prefix.empty() && allow_enum_literal)
                            {
                                return make_typed(Parameter::String(name, ""), e.name.text);    // an enumeration literal of a declared type
                            }
                            return ERR(what + ": '" + name + "' refers to '" + e.name.text + "', which is not a generic or constant of this file" + at(e.location));
                        }
                        case K::Unary:
                        case K::Binary:
                        case K::Attribute: {
                            if (e.kind == K::Unary && e.text == "-" && e.children.at(0).kind == K::Number && is_real_literal(e.children.at(0).text))
                            {
                                return make_typed(Parameter::Float(name, "0"), "-" + strip_underscores(e.children.at(0).text));
                            }
                            auto v = evaluate(e);
                            if (v.is_error())
                            {
                                return ERR_APPEND(v.get_error(), what + ": the value of '" + name + "' is neither a literal nor a constant integer expression" + at(e.location));
                            }
                            return make_typed(Parameter::Integer(name, "0"), std::to_string(v.get()));
                        }
                        default:
                            return ERR(what + ": the value of '" + name + "' is not a literal" + at(e.location));
                    }
                }

                /**
                 * The declared type key of a generic of the entity or component, if it has one that types know.
                 */
                static std::optional<std::string> declared_type_of(const InterfaceDecl* d)
                {
                    if (d == nullptr)
                    {
                        return std::nullopt;
                    }
                    return d->type.name.key();
                }

                // ---- generics -------------------------------------------------------------------------------------

                Result<std::monostate> elaborate_generics()
                {
                    // package constants first: generics may refer to them
                    for (const auto& [key, c] : m_constants)
                    {
                        if (!c->initializer.has_value())
                        {
                            continue;
                        }
                        if (auto v = evaluate(c->initializer.value()); v.is_ok())
                        {
                            m_integer_values[key] = v.get();
                        }
                        if (auto tv = typed_value(c->name.text, c->initializer.value(), c->type.name.key(), ""); tv.is_ok())
                        {
                            m_typed_values[key] = tv.get();
                        }
                    }
                    for (const InterfaceDecl& g : m_entity.generics)
                    {
                        if (!g.default_value.has_value())
                        {
                            continue;    // no default: nothing to record, ranges that use it fail with a clear message
                        }
                        const std::string what = where("generic '" + g.name.text + "'", g.location);
                        auto tv                = typed_value(g.name.text, g.default_value.value(), g.type.name.key(), what);
                        if (tv.is_error())
                        {
                            return ERR(tv.get_error());
                        }
                        if (auto v = evaluate(g.default_value.value()); v.is_ok())
                        {
                            m_integer_values[g.name.key()] = v.get();
                        }
                        const TypedValue value       = tv.get();
                        m_typed_values[g.name.key()] = value;
                        m_dst.parameters.push_back(value);
                    }
                    return OK({});
                }

                // ---- ports and signals ----------------------------------------------------------------------------

                Result<std::monostate> elaborate_ports()
                {
                    for (const InterfaceDecl& p : m_entity.ports)
                    {
                        if (find_signal(p.name) != nullptr)
                        {
                            return ERR(where("port '" + p.name.text + "' is declared twice", p.location));
                        }
                        auto dims = resolve_dims(p.type);
                        if (dims.is_error())
                        {
                            return ERR_APPEND(dims.get_error(), where("cannot declare port '" + p.name.text + "'", p.location));
                        }
                        Port& port    = m_dst.add_port(p.name.text, p.mode, dims.get());
                        port.location = p.location;
                        m_signals[p.name.key()] = &port;
                    }
                    return OK({});
                }

                Result<std::monostate> elaborate_signals()
                {
                    for (const ObjectDecl& o : m_arch->declarations.objects)
                    {
                        if (o.is_constant)
                        {
                            if (o.initializer.has_value())
                            {
                                if (auto v = evaluate(o.initializer.value()); v.is_ok())
                                {
                                    m_integer_values[o.name.key()] = v.get();
                                }
                                if (auto tv = typed_value(o.name.text, o.initializer.value(), o.type.name.key(), ""); tv.is_ok())
                                {
                                    m_typed_values[o.name.key()] = tv.get();
                                }
                            }
                            continue;
                        }
                        if (find_signal(o.name) != nullptr)
                        {
                            return ERR(where("signal '" + o.name.text + "' is declared twice or shadows a port", o.location));
                        }
                        auto dims = resolve_dims(o.type);
                        if (dims.is_error())
                        {
                            return ERR_APPEND(dims.get_error(), where("cannot declare signal '" + o.name.text + "'", o.location));
                        }
                        Signal& s  = m_dst.add_signal(o.name.text, dims.get());
                        s.location = o.location;
                        m_signals[o.name.key()] = &s;
                    }
                    return OK({});
                }

                // ---- wiring ---------------------------------------------------------------------------------------

                /**
                 * One dimension of a selection: a single index, a range, or (absent) the whole declared range.
                 */
                struct Selector
                {
                    std::optional<i32> index;
                    std::optional<netlist_ir::Range> range;
                };

                /**
                 * The bits of a signal under the given per-dimension selectors, outer dimensions first; trailing
                 * dimensions without a selector are taken whole.
                 */
                Result<std::vector<BitId>> select(const Signal& s, const std::vector<Selector>& selectors, const Location& loc) const
                {
                    if (selectors.size() > s.dims.size())
                    {
                        return ERR("'" + s.name + "' has " + std::to_string(s.dims.size()) + " dimension(s), " + std::to_string(selectors.size()) + " indexed" + at(loc));
                    }
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

                /**
                 * A constant used in wiring: its bits under its declared dimensions, as a temporary signal.
                 */
                Result<Signal> constant_as_signal(const ObjectDecl& c) const
                {
                    if (!c.initializer.has_value())
                    {
                        return ERR("constant '" + c.name.text + "' has no value" + at(c.location));
                    }
                    Signal s;
                    s.name = c.name.text;
                    if (!c.type.constraints.empty() || vector_dimensions(c.type.name.key()) == 0 || find_type(c.type.name) != nullptr)
                    {
                        auto dims = resolve_dims(c.type);
                        if (dims.is_error())
                        {
                            return ERR_APPEND(dims.get_error(), "constant '" + c.name.text + "' is not a bit type" + at(c.location));
                        }
                        s.dims = dims.get();
                    }
                    std::optional<u32> expected;
                    if (!s.dims.empty())
                    {
                        expected = 1;
                        for (const netlist_ir::Range& r : s.dims)
                        {
                            expected = expected.value() * r.size();
                        }
                    }
                    auto bits = wiring_bits(c.initializer.value(), expected);
                    if (bits.is_error())
                    {
                        return ERR_APPEND(bits.get_error(), "cannot resolve the value of constant '" + c.name.text + "'" + at(c.location));
                    }
                    s.bits = bits.get();
                    if (s.dims.empty())
                    {
                        // an unconstrained vector constant takes the width of its value, indexed downto
                        if (s.bits.size() != 1 || vector_dimensions(c.type.name.key()) == 1)
                        {
                            s.dims.push_back(netlist_ir::Range{static_cast<i32>(s.bits.size()) - 1, 0});
                        }
                    }
                    else if (expected.value() != s.bits.size())
                    {
                        return ERR("constant '" + c.name.text + "' has " + std::to_string(expected.value()) + " bits but its value has " + std::to_string(s.bits.size()) + at(c.location));
                    }
                    return OK(s);
                }

                /**
                 * The base of an index or slice: a signal, or a constant materialized as one.
                 */
                Result<Signal> wiring_base(const Expr& e, std::optional<Signal>& storage) const
                {
                    if (e.kind != Expr::Kind::Identifier)
                    {
                        return ERR("only a signal or constant can be indexed" + at(e.location));
                    }
                    if (const Signal* s = find_signal(e.name); s != nullptr)
                    {
                        return OK(*s);
                    }
                    if (const auto c = m_constants.find(e.name.key()); c != m_constants.end())
                    {
                        auto s = constant_as_signal(*c->second);
                        if (s.is_error())
                            return s;
                        storage = s.get();
                        return OK(storage.value());
                    }
                    return ERR("'" + e.name.text + "' is not a declared signal, port or constant" + at(e.location));
                }

                /**
                 * The bits a wiring expression stands for, MSB first. `expected` is the width the context asks for,
                 * which `others` and `open` need.
                 */
                Result<std::vector<BitId>> wiring_bits(const Expr& e, std::optional<u32> expected = std::nullopt) const
                {
                    using K = Expr::Kind;
                    switch (e.kind)
                    {
                        case K::Empty:
                            return OK(std::vector<BitId>{});
                        case K::Open:
                            return OK(std::vector<BitId>(expected.value_or(1), OPEN));
                        case K::Identifier: {
                            if (const Signal* s = find_signal(e.name); s != nullptr)
                            {
                                return OK(s->bits);
                            }
                            if (const auto c = m_constants.find(e.name.key()); c != m_constants.end())
                            {
                                auto s = constant_as_signal(*c->second);
                                if (s.is_error())
                                    return ERR(s.get_error());
                                return OK(s.get().bits);
                            }
                            if (const auto g = m_generics.find(e.name.key()); g != m_generics.end())
                            {
                                return ERR("'" + e.name.text + "' is a generic; a generic cannot be wired" + at(e.location));
                            }
                            return ERR("'" + e.name.text + "' is not a declared signal, port or constant" + at(e.location));
                        }
                        case K::Character: {
                            auto b = bit_of_char(e.text.empty() ? '?' : e.text.front());
                            if (b.is_error())
                                return ERR_APPEND(b.get_error(), "invalid character literal" + at(e.location));
                            return OK(std::vector<BitId>{b.get()});
                        }
                        case K::String: {
                            std::vector<BitId> out;
                            for (char c : e.text)
                            {
                                auto b = bit_of_char(c);
                                if (b.is_error())
                                    return ERR_APPEND(b.get_error(), "invalid string literal \"" + e.text + "\"" + at(e.location));
                                out.push_back(b.get());
                            }
                            return OK(out);
                        }
                        case K::BitString: {
                            auto bits = expand_bit_string(e.text);
                            if (bits.is_error())
                                return ERR_APPEND(bits.get_error(), "invalid bit string literal" + at(e.location));
                            std::vector<BitId> out;
                            for (char c : bits.get())
                            {
                                out.push_back(bit_of_char(c).get());
                            }
                            return OK(out);
                        }
                        case K::Number:
                            return ERR("the integer '" + e.text + "' is not a wiring value; use a bit string such as \"0101\"" + at(e.location));
                        case K::Index: {
                            std::optional<Signal> storage;
                            auto base = wiring_base(e.children.at(0), storage);
                            if (base.is_error())
                                return ERR(base.get_error());
                            std::vector<Selector> selectors;
                            for (u32 i = 1; i < e.children.size(); i++)
                            {
                                const Expr& idx = e.children.at(i);
                                if (idx.kind == K::Slice && idx.children.front().is_empty())
                                {
                                    ast::Range r;
                                    r.left       = idx.children.at(1);
                                    r.right      = idx.children.at(2);
                                    r.descending = idx.text == "downto";
                                    auto range   = evaluate_range(r);
                                    if (range.is_error())
                                        return ERR_APPEND(range.get_error(), "invalid range" + at(idx.location));
                                    selectors.push_back(Selector{std::nullopt, range.get()});
                                    continue;
                                }
                                auto v = evaluate(idx);
                                if (v.is_error())
                                    return ERR_APPEND(v.get_error(), "invalid index" + at(idx.location));
                                selectors.push_back(Selector{static_cast<i32>(v.get()), std::nullopt});
                            }
                            return select(base.get(), selectors, e.location);
                        }
                        case K::Slice: {
                            std::optional<Signal> storage;
                            auto base = wiring_base(e.children.at(0), storage);
                            if (base.is_error())
                                return ERR(base.get_error());
                            ast::Range r;
                            r.left       = e.children.at(1);
                            r.right      = e.children.at(2);
                            r.descending = e.text == "downto";
                            auto range   = evaluate_range(r);
                            if (range.is_error())
                                return ERR_APPEND(range.get_error(), "invalid slice" + at(e.location));
                            return select(base.get(), {Selector{std::nullopt, range.get()}}, e.location);
                        }
                        case K::Concat: {
                            std::vector<BitId> out;
                            for (const Expr& c : e.children)
                            {
                                auto bits = wiring_bits(c);
                                if (bits.is_error())
                                    return bits;
                                out.insert(out.end(), bits.get().begin(), bits.get().end());
                            }
                            return OK(out);
                        }
                        case K::Aggregate: {
                            std::vector<BitId> out;
                            const Expr* others = nullptr;
                            for (const Expr& c : e.children)
                            {
                                if (c.choices.empty())
                                {
                                    auto bits = wiring_bits(c);
                                    if (bits.is_error())
                                        return bits;
                                    out.insert(out.end(), bits.get().begin(), bits.get().end());
                                }
                                else if (c.choices.size() == 1 && c.choices.front().kind == K::Others)
                                {
                                    others = &c;
                                }
                                else
                                {
                                    return ERR("only positional elements and 'others' are supported in an aggregate" + at(c.location));
                                }
                            }
                            if (others != nullptr)
                            {
                                if (!expected.has_value())
                                {
                                    return ERR("'others' needs a context with a known width" + at(others->location));
                                }
                                auto fill = wiring_bits(*others);
                                if (fill.is_error())
                                    return fill;
                                if (fill.get().size() != 1)
                                {
                                    return ERR("'others' must be a single bit" + at(others->location));
                                }
                                if (out.size() > expected.value())
                                {
                                    return ERR("the aggregate has more elements than the " + std::to_string(expected.value()) + " bits of its context" + at(e.location));
                                }
                                out.insert(out.end(), expected.value() - out.size(), fill.get().front());
                            }
                            return OK(out);
                        }
                        case K::Attribute:
                            return ERR("attribute '" + e.text + "' is not a wiring value" + at(e.location));
                        case K::Unary:
                        case K::Binary:
                            return ERR("operator '" + e.text + "' is a logic expression, which a netlist cannot contain; only wiring is supported" + at(e.location));
                        case K::Others:
                            return ERR("'others' outside of an aggregate" + at(e.location));
                    }
                    return ERR("unexpected expression" + at(e.location));
                }

                bool all_constant(const std::vector<BitId>& bits) const
                {
                    return !bits.empty() && std::all_of(bits.begin(), bits.end(), [](BitId b) { return b == ZERO || b == ONE || b == OPEN; });
                }

                /**
                 * Alias the bits of a target with those of a source, low bits first; a narrower constant source is
                 * zero-extended, a narrower signal leaves the high target bits alone, a wider source drops its high bits.
                 */
                void alias_bits(const std::vector<BitId>& target, const std::vector<BitId>& source, const std::string& what, const Location& loc)
                {
                    if (target.size() != source.size() && !(source.size() < target.size() && all_constant(source)))
                    {
                        log_warning("vhdl_parser", "{}: {} bits are assigned to {} bits, the low bits are paired{}", what, source.size(), target.size(), at(loc));
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
                            log_warning("vhdl_parser", "{}: a constant is assigned to, the assignment is ignored{}", what, at(loc));
                            continue;
                        }
                        m_dst.add_alias(t, s);
                    }
                }

                Result<std::monostate> elaborate_assignments()
                {
                    for (const Assignment& a : m_arch->assignments)
                    {
                        auto lhs = wiring_bits(a.target);
                        if (lhs.is_error())
                        {
                            return ERR_APPEND(lhs.get_error(), where("cannot resolve the target of an assignment", a.location));
                        }
                        auto rhs = wiring_bits(a.value, static_cast<u32>(lhs.get().size()));
                        if (rhs.is_error())
                        {
                            return ERR_APPEND(rhs.get_error(), where("cannot resolve the value of an assignment", a.location));
                        }
                        alias_bits(lhs.get(), rhs.get(), where("assignment", a.location), a.location);
                    }
                    return OK({});
                }

                // ---- instantiations -------------------------------------------------------------------------------

                /**
                 * What an instantiation refers to after components, configurations and library prefixes are resolved.
                 */
                struct Target
                {
                    std::string type;                       /**< The IR type name: the entity's or component's declared spelling, or the name as written. */
                    bool is_module = false;
                    const Entity* entity          = nullptr; /**< The entity of a module instance. */
                    const ComponentDecl* component = nullptr; /**< The component declaration, if any. */
                };

                Result<Target> resolve_target(const Instantiation& inst) const
                {
                    Target t;
                    if (inst.kind == Instantiation::Kind::Configuration)
                    {
                        return ERR("instantiating a configuration is not supported" + at(inst.location));
                    }
                    std::string entity_key = inst.unit.key();
                    std::optional<std::string> architecture_key;
                    if (inst.architecture.has_value())
                    {
                        architecture_key = inst.architecture->key();
                    }
                    if (inst.kind == Instantiation::Kind::Component)
                    {
                        // a component: a declaration may exist, a configuration may bind it, or it names an entity of the file
                        t.component = find_component(inst.unit);
                        if (const auto b = m_global.bindings.find(inst.unit.key()); b != m_global.bindings.end())
                        {
                            entity_key = b->second.entity.key();
                            if (b->second.architecture.has_value())
                            {
                                architecture_key = b->second.architecture->key();
                            }
                        }
                    }
                    const auto e = m_global.entities.find(entity_key);
                    if (e == m_global.entities.end())
                    {
                        // not an entity of this file: a gate type, named by its component declaration if there is one
                        t.type = t.component != nullptr ? t.component->name.text : inst.unit.text;
                        return OK(t);
                    }
                    t.entity                 = e->second;
                    const std::string module = m_global.module_of(entity_key, architecture_key);
                    if (module.empty())
                    {
                        if (architecture_key.has_value())
                        {
                            return ERR("entity '" + t.entity->name.text + "' has no architecture '" + architecture_key.value() + "'" + at(inst.location));
                        }
                        t.type = t.entity->name.text;    // an entity without an architecture: a gate type
                        return OK(t);
                    }
                    t.type      = module;
                    t.is_module = true;
                    return OK(t);
                }

                /**
                 * The fill value of an aggregate that consists of `others => value` alone, else null.
                 */
                static const Expr* others_only(const Expr& e)
                {
                    if (e.kind == Expr::Kind::Aggregate && e.children.size() == 1 && e.children.front().choices.size() == 1 && e.children.front().choices.front().kind == Expr::Kind::Others)
                    {
                        return &e.children.front();
                    }
                    return nullptr;
                }

                /**
                 * The declaration of the formal a port or generic map names: from the entity of a module instance,
                 * else from the component declaration; null for a gate without a component declaration.
                 */
                static const InterfaceDecl* find_interface(const std::vector<InterfaceDecl>& list, const Name& n)
                {
                    for (const InterfaceDecl& d : list)
                    {
                        if (d.name == n)
                        {
                            return &d;
                        }
                    }
                    return nullptr;
                }

                Result<std::monostate> elaborate_instantiations()
                {
                    for (const Instantiation& inst : m_arch->instantiations)
                    {
                        const std::string what = where("instance '" + inst.label.text + "' of '" + inst.unit.text + "'", inst.location);
                        if (m_instances.count(inst.label.key()) > 0)
                        {
                            return ERR(where("instance label '" + inst.label.text + "' is used twice", inst.location));
                        }
                        auto target = resolve_target(inst);
                        if (target.is_error())
                        {
                            return ERR_APPEND(target.get_error(), what);
                        }
                        const Target& t = target.get();
                        Instance& out = m_dst.add_instance(inst.label.text, t.type, t.is_module ? InstanceKind::Module : InstanceKind::Gate);
                        out.location  = inst.location;
                        m_instances[inst.label.key()] = &out;

                        const std::vector<InterfaceDecl>* ports    = t.is_module ? &t.entity->ports : t.component != nullptr ? &t.component->ports : nullptr;
                        const std::vector<InterfaceDecl>* generics = t.is_module ? &t.entity->generics : t.component != nullptr ? &t.component->generics : nullptr;

                        // ports
                        u32 positional = 0;
                        for (const Association& a : inst.port_map)
                        {
                            std::string port_name;
                            std::optional<netlist_ir::Range> port_slice;
                            const InterfaceDecl* decl = nullptr;
                            if (a.formal.has_value())
                            {
                                const Expr& f = a.formal.value();
                                const Expr& n = f.kind == Expr::Kind::Identifier ? f : f.children.at(0);
                                if (n.kind != Expr::Kind::Identifier)
                                {
                                    return ERR(what + ": the formal of a port association must be a port name" + at(f.location));
                                }
                                if (ports != nullptr)
                                {
                                    decl = find_interface(*ports, n.name);
                                    if (decl == nullptr)
                                    {
                                        return ERR(what + ": '" + t.type + "' has no port '" + n.name.text + "'" + at(f.location));
                                    }
                                    port_name = decl->name.text;
                                }
                                else
                                {
                                    port_name = n.name.text;
                                }
                                if (f.kind == Expr::Kind::Index)
                                {
                                    if (f.children.size() != 2)
                                    {
                                        return ERR(what + ": only one index is supported on the formal '" + port_name + "'" + at(f.location));
                                    }
                                    auto v = evaluate(f.children.at(1));
                                    if (v.is_error())
                                        return ERR_APPEND(v.get_error(), what + ": invalid index on the formal '" + port_name + "'" + at(f.location));
                                    port_slice = netlist_ir::Range{static_cast<i32>(v.get()), static_cast<i32>(v.get())};
                                }
                                else if (f.kind == Expr::Kind::Slice)
                                {
                                    ast::Range r;
                                    r.left       = f.children.at(1);
                                    r.right      = f.children.at(2);
                                    r.descending = f.text == "downto";
                                    auto range   = evaluate_range(r);
                                    if (range.is_error())
                                        return ERR_APPEND(range.get_error(), what + ": invalid slice on the formal '" + port_name + "'" + at(f.location));
                                    port_slice = range.get();
                                }
                            }
                            else
                            {
                                if (ports != nullptr)
                                {
                                    if (positional >= ports->size())
                                    {
                                        return ERR(what + ": more positional port associations than '" + t.type + "' has ports" + at(a.location));
                                    }
                                    decl = &ports->at(positional);
                                }
                                positional++;
                            }

                            std::optional<u32> expected;
                            if (port_slice.has_value())
                            {
                                expected = port_slice->size();
                            }
                            else if (decl != nullptr)
                            {
                                if (auto dims = resolve_dims(decl->type); dims.is_ok())
                                {
                                    u32 n = 1;
                                    for (const netlist_ir::Range& r : dims.get())
                                        n *= r.size();
                                    expected = n;
                                }
                            }
                            if (a.actual.kind == Expr::Kind::Open && a.formal.has_value())
                            {
                                continue;    // unconnected; a positional `open` keeps its place in the list
                            }
                            const Expr* fill = expected.has_value() ? nullptr : others_only(a.actual);
                            auto bits        = wiring_bits(fill != nullptr ? *fill : a.actual, expected);
                            if (bits.is_error())
                            {
                                return ERR_APPEND(bits.get_error(), what + ": cannot resolve the connection of port '" + (port_name.empty() ? std::to_string(positional) : port_name) + "'");
                            }
                            if (bits.get().empty())
                            {
                                continue;
                            }
                            if (!port_name.empty() && !port_slice.has_value() && out.find_connection(port_name) != nullptr)
                            {
                                return ERR(what + ": port '" + port_name + "' is connected twice" + at(a.location));
                            }
                            if (fill != nullptr && bits.get().size() != 1)
                            {
                                return ERR(what + ": 'others' must be a single bit" + at(a.location));
                            }
                            Connection& c = out.add_connection(port_name, bits.get(), port_slice);
                            c.replicate   = fill != nullptr;
                        }

                        // generics
                        positional = 0;
                        for (const Association& a : inst.generic_map)
                        {
                            std::string name;
                            const InterfaceDecl* decl = nullptr;
                            if (a.formal.has_value())
                            {
                                const Expr& f = a.formal.value();
                                if (f.kind != Expr::Kind::Identifier)
                                {
                                    return ERR(what + ": the formal of a generic association must be a generic name" + at(f.location));
                                }
                                if (generics != nullptr)
                                {
                                    decl = find_interface(*generics, f.name);
                                    if (decl == nullptr)
                                    {
                                        return ERR(what + ": '" + t.type + "' has no generic '" + f.name.text + "'" + at(f.location));
                                    }
                                    name = decl->name.text;
                                }
                                else
                                {
                                    name = f.name.text;
                                }
                            }
                            else
                            {
                                if (generics == nullptr)
                                {
                                    return ERR(what + ": positional generics need a component or entity declaration" + at(a.location));
                                }
                                if (positional >= generics->size())
                                {
                                    return ERR(what + ": more positional generic associations than '" + t.type + "' has generics" + at(a.location));
                                }
                                decl = &generics->at(positional);
                                name = decl->name.text;
                                positional++;
                            }
                            if (a.actual.kind == Expr::Kind::Open)
                            {
                                continue;
                            }
                            // an unquoted word is an enumeration literal only when the generic declares a type for it
                            auto tv = typed_value(name, a.actual, declared_type_of(decl), what, decl != nullptr);
                            if (tv.is_error())
                            {
                                return ERR(tv.get_error());
                            }
                            if (std::any_of(out.parameters.begin(), out.parameters.end(), [&name](const TypedValue& v) { return v.declaration.get_source() == Parameter::Source::Generic && v.declaration.get_name() == name; }))
                            {
                                return ERR(what + ": generic '" + name + "' is set twice" + at(a.location));
                            }
                            out.parameters.push_back(tv.get());
                        }
                    }
                    return OK({});
                }

                // ---- attributes -----------------------------------------------------------------------------------

                Result<std::monostate> elaborate_attributes(const Declarations& d)
                {
                    for (const AttributeSpec& spec : d.attribute_specs)
                    {
                        std::optional<std::string> declared_type;
                        if (const auto it = m_attribute_decls.find(spec.attribute.key()); it != m_attribute_decls.end())
                        {
                            const std::string key = it->second->type.key();
                            if (key == "string" || key == "boolean" || key == "integer" || key == "natural" || key == "positive" || key == "real" || key == "time" || key == "bit" || key == "std_logic"
                                || key == "std_ulogic" || vector_dimensions(key) == 1)
                            {
                                declared_type = key;
                            }
                            // any other type, such as a user enumeration: the literal form of the value decides
                        }
                        const std::string what = where("attribute '" + spec.attribute.text + "'", spec.location);
                        auto typed             = typed_value(spec.attribute.text, spec.value, declared_type, what);
                        if (typed.is_error())
                        {
                            return ERR(typed.get_error());
                        }
                        TypedValue tv  = typed.get();
                        tv.declaration = tv.declaration.with_source(Parameter::Source::Attribute);
                        for (const Name& target : spec.targets)
                        {
                            std::vector<TypedValue>* dst = nullptr;
                            if (spec.entity_class == "signal")
                            {
                                Signal* s = find_signal(target);
                                if (s == nullptr)
                                {
                                    return ERR(what + ": there is no signal or port '" + target.text + "'");
                                }
                                dst = &s->parameters;
                            }
                            else if (spec.entity_class == "label")
                            {
                                const auto it = m_instances.find(target.key());
                                if (it == m_instances.end())
                                {
                                    return ERR(what + ": there is no instance labeled '" + target.text + "'");
                                }
                                dst = &it->second->parameters;
                            }
                            else if (spec.entity_class == "entity" || spec.entity_class == "architecture")
                            {
                                if (!(target == m_entity.name) && !(m_arch != nullptr && target == m_arch->name))
                                {
                                    return ERR(what + ": '" + target.text + "' is not this entity or its architecture");
                                }
                                dst = &m_dst.parameters;
                            }
                            else
                            {
                                log_warning("vhdl_parser", "{}", what + ": attributes of class '" + spec.entity_class + "' have no place in the netlist and are dropped");
                                continue;
                            }
                            if (std::any_of(dst->begin(), dst->end(), [&tv](const TypedValue& v) { return v.declaration == tv.declaration || (v.declaration.get_source() == Parameter::Source::Attribute && v.declaration.get_name() == tv.declaration.get_name()); }))
                            {
                                return ERR(what + ": the attribute is given twice for '" + target.text + "'");
                            }
                            dst->push_back(tv);
                        }
                    }
                    return OK({});
                }
            };
        }    // namespace

        Result<netlist_ir::Design> elaborate(const ast::SourceFile& file)
        {
            Design design;
            design.source = file.file;

            GlobalScope global;
            for (const Entity& e : file.entities)
            {
                if (!global.entities.emplace(e.name.key(), &e).second)
                {
                    return ERR("entity '" + e.name.text + "' is declared twice in '" + file.file + "'" + at(e.location));
                }
            }
            for (const Architecture& a : file.architectures)
            {
                if (global.entities.count(a.entity.key()) == 0)
                {
                    return ERR("architecture '" + a.name.text + "' belongs to entity '" + a.entity.text + "', which is not declared in '" + file.file + "'" + at(a.location));
                }
                for (const Architecture* other : global.architectures[a.entity.key()])
                {
                    if (other->name == a.name)
                    {
                        return ERR("architecture '" + a.name.text + "' of entity '" + a.entity.text + "' is declared twice in '" + file.file + "'" + at(a.location));
                    }
                }
                global.architectures[a.entity.key()].push_back(&a);
            }
            for (const Package& p : file.packages)
            {
                for (const ComponentDecl& c : p.declarations.components)
                    global.components[c.name.key()] = &c;
                for (const ObjectDecl& o : p.declarations.objects)
                    if (o.is_constant)
                        global.constants[o.name.key()] = &o;
                for (const TypeDecl& t : p.declarations.types)
                    global.types[t.name.key()] = &t;
                for (const AttributeDecl& a : p.declarations.attribute_decls)
                    global.attribute_decls[a.name.key()] = &a;
            }
            for (const Configuration& c : file.configurations)
            {
                for (const Configuration::Binding& b : c.bindings)
                {
                    global.bindings[b.component.key()] = b;
                }
            }

            // which (entity, architecture) pairs are in use: every architecture a direct instantiation or a
            // configuration names, and the default architecture (the last one declared) for every entity that is
            // instantiated without an architecture name or not at all
            std::set<std::pair<std::string, std::string>> requested;
            std::set<std::string> plainly_instantiated;
            for (const Architecture& a : file.architectures)
            {
                for (const Instantiation& inst : a.instantiations)
                {
                    std::string entity_key = inst.unit.key();
                    std::optional<std::string> architecture_key;
                    if (inst.architecture.has_value())
                    {
                        architecture_key = inst.architecture->key();
                    }
                    if (inst.kind == Instantiation::Kind::Component)
                    {
                        if (const auto b = global.bindings.find(inst.unit.key()); b != global.bindings.end())
                        {
                            entity_key = b->second.entity.key();
                            if (b->second.architecture.has_value())
                            {
                                architecture_key = b->second.architecture->key();
                            }
                        }
                    }
                    if (global.entities.count(entity_key) == 0)
                    {
                        continue;
                    }
                    if (architecture_key.has_value())
                    {
                        requested.insert({entity_key, architecture_key.value()});
                    }
                    else
                    {
                        plainly_instantiated.insert(entity_key);
                    }
                }
            }

            struct Unit
            {
                const Entity* entity;
                const Architecture* architecture;
                std::string name;
            };
            std::vector<Unit> units;
            for (const auto& [key, archs] : global.architectures)
            {
                const Entity& e = *global.entities.at(key);
                const Architecture* default_arch = archs.back();
                bool default_used                = plainly_instantiated.count(key) > 0;
                for (const Architecture* a : archs)
                {
                    const bool is_requested = requested.count({key, a->name.key()}) > 0;
                    if (a == default_arch && (is_requested || default_used || std::none_of(archs.begin(), archs.end(), [&](const Architecture* o) { return requested.count({key, o->name.key()}) > 0; })))
                    {
                        // the default architecture carries the entity's name
                        default_used = true;
                        global.modules[{key, a->name.key()}] = e.name.text;
                        global.default_module[key]           = e.name.text;
                        units.push_back({&e, a, e.name.text});
                    }
                    else if (is_requested)
                    {
                        const std::string name               = e.name.text + "(" + a->name.text + ")";
                        global.modules[{key, a->name.key()}] = name;
                        units.push_back({&e, a, name});
                    }
                }
                if (archs.size() > 1 && default_used && requested.count({key, default_arch->name.key()}) == 0)
                {
                    log_warning("vhdl_parser", "entity '{}' has {} architectures, the last one declared ('{}') is used where none is named", e.name.text, archs.size(), default_arch->name.text);
                }
            }

            // in file order of the entities, so that the top module comes where the file puts it
            for (const Entity& e : file.entities)
            {
                for (const Unit& u : units)
                {
                    if (u.entity != &e)
                    {
                        continue;
                    }
                    netlist_ir::Module& dst = design.add_module(u.name);
                    dst.location            = e.location;
                    ModuleElaborator elaborator(global, e, u.architecture, dst);
                    if (auto res = elaborator.run(); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "could not elaborate '" + file.file + "'");
                    }
                }
            }

            if (design.modules.empty())
            {
                return ERR("could not elaborate '" + file.file + "': no entity has an architecture");
            }
            if (auto res = design.validate(); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not elaborate '" + file.file + "'");
            }
            return OK(design);
        }
    }    // namespace vhdl
}    // namespace hal
