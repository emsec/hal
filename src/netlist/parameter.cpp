#include "hal_core/netlist/parameter.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <utility>

namespace hal
{
    template<>
    std::map<Parameter::Type, std::string> EnumStrings<Parameter::Type>::data = {
        {Parameter::Type::Boolean, "boolean"},
        {Parameter::Type::BitVector, "bit_vector"},
        {Parameter::Type::LogicVector, "logic_vector"},
        {Parameter::Type::Integer, "integer"},
        {Parameter::Type::String, "string"},
        {Parameter::Type::Float, "float"},
        {Parameter::Type::Time, "time"},
        {Parameter::Type::Enum, "enum"},
    };

    namespace
    {
        std::string normalize_value(Parameter::Type type, const std::string& value)
        {
            if (type != Parameter::Type::BitVector && type != Parameter::Type::LogicVector)
                return value;
            if (value.size() < 2 || value[0] != '0')
                return value;
            const char pfx = static_cast<char>(std::tolower(static_cast<unsigned char>(value[1])));
            if (pfx != 'b' && pfx != 'o' && pfx != 'x')
                return value;
            std::string result;
            result.reserve(value.size());
            result += '0';
            result += pfx;
            for (std::size_t i = 2; i < value.size(); ++i)
                result += static_cast<char>(std::toupper(static_cast<unsigned char>(value[i])));
            return result;
        }

        // Number of bits needed to encode ``count`` distinct values, i.e. ``ceil(log2(count))``.
        // Returns 0 for count <= 1, which the enum constructor rejects as invalid.
        u16 enum_bit_width(size_t count)
        {
            if (count <= 1)
            {
                return 0;
            }
            const unsigned long long max = static_cast<unsigned long long>(count - 1);
            return static_cast<u16>(sizeof(unsigned long long) * 8 - __builtin_clzll(max));
        }

        // Parse a bit-vector value string (0b..., 0o..., 0x...) into a u64.
        // Returns an error for empty input or unparseable digits.
        Result<u64> parse_bit_vector(const std::string& value)
        {
            if (value.empty())
            {
                return ERR("empty bit-vector value");
            }
            int base;
            if (value.size() >= 2 && value[0] == '0')
            {
                const char c = value[1];
                if (c == 'b')
                {
                    base = 2;
                }
                else if (c == 'o')
                {
                    base = 8;
                }
                else if (c == 'x')
                {
                    base = 16;
                }
                else
                {
                    return ERR("invalid base");
                }
            }
            else
            {
                return ERR("bit-vector value '" + value + "' has no base prefix (expected 0b, 0o, or 0x)");
            }
            const std::string digits = value.substr(2);
            if (digits.empty())
            {
                return ERR("bit-vector value '" + value + "' has no digits after the base prefix");
            }
            try
            {
                size_t consumed = 0;
                const u64 v     = std::stoull(digits, &consumed, base);
                if (consumed != digits.size())
                {
                    return ERR("bit-vector value '" + value + "' contains non-digit characters");
                }
                return OK(v);
            }
            catch (const std::invalid_argument&)
            {
                return ERR("bit-vector value '" + value + "' is not a valid number");
            }
            catch (const std::out_of_range&)
            {
                return ERR("bit-vector value '" + value + "' exceeds the 64-bit range");
            }
        }

        // Returns true if `c` is one of the 9 VHDL std_logic state characters.
        // Expects canonical uppercase form: X, Z, U, L, H, W; also accepts 0, 1, -.
        bool is_logic_state_char(char c)
        {
            return c == '0' || c == '1' || c == 'X' || c == 'Z' || c == 'U' || c == 'L' || c == 'H' || c == 'W' || c == '-';
        }

        // Parse a 9-state logic-vector literal "0b<bits>", "0o<digits>", or "0x<digits>".
        // Expects canonical form: lowercase prefix, uppercase digits and state characters.
        // Each digit is expanded to its bits (MSB first). A state character (X, Z, U, L,
        // H, W, -) expands to N copies of itself where N is 1, 3, or 4 depending on the base.
        // Returns the equivalent per-bit string (characters in {`0`, `1`, `X`, `Z`, `U`,
        // `L`, `H`, `W`, `-`}).
        Result<std::string> parse_logic_vector(const std::string& value)
        {
            if (value.empty())
            {
                return ERR("empty logic-vector value");
            }
            if (value.size() < 2 || value[0] != '0')
            {
                return ERR("logic-vector value '" + value + "' has no base prefix");
            }
            const char base_char = value[1];
            int bits_per_digit;
            std::string numeric_set;
            switch (base_char)
            {
                case 'b':
                    bits_per_digit = 1;
                    numeric_set    = "01";
                    break;
                case 'o':
                    bits_per_digit = 3;
                    numeric_set    = "01234567";
                    break;
                case 'x':
                    bits_per_digit = 4;
                    numeric_set    = "0123456789ABCDEF";
                    break;
                default:
                    return ERR("logic-vector value '" + value + "' has invalid base");
            }
            const std::string digits = value.substr(2);
            if (digits.empty())
            {
                return ERR("logic-vector value '" + value + "' has no digits after the base prefix");
            }
            std::string out;
            out.reserve(digits.size() * bits_per_digit);
            for (const char c : digits)
            {
                const bool is_numeric = numeric_set.find(c) != std::string::npos;
                const bool is_state   = is_logic_state_char(c) && c != '0' && c != '1';
                if (!is_numeric && !is_state)
                {
                    return ERR("logic-vector value '" + value + "' contains invalid digit '" + std::string(1, c) + "'");
                }
                if (is_state)
                {
                    out.append(static_cast<size_t>(bits_per_digit), c);
                }
                else
                {
                    const int n = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
                    for (int i = bits_per_digit - 1; i >= 0; --i)
                    {
                        out += ((n >> i) & 1) ? '1' : '0';
                    }
                }
            }
            return OK(out);
        }

        // Parse a base-10 signed integer (optional leading +/-).
        Result<i64> parse_integer(const std::string& value)
        {
            if (value.empty())
            {
                return ERR("empty integer value");
            }
            try
            {
                size_t consumed = 0;
                const i64 v     = std::stoll(value, &consumed, 10);
                if (consumed != value.size())
                {
                    return ERR("integer value '" + value + "' contains trailing characters");
                }
                return OK(v);
            }
            catch (const std::invalid_argument&)
            {
                return ERR("integer value '" + value + "' is not a valid number");
            }
            catch (const std::out_of_range&)
            {
                return ERR("integer value '" + value + "' exceeds the 64-bit signed range");
            }
        }

        // Parse a floating-point value (decimal, with optional sign and exponent).
        Result<double> parse_float(const std::string& value)
        {
            if (value.empty())
            {
                return ERR("empty float value");
            }
            try
            {
                size_t consumed = 0;
                const double v  = std::stod(value, &consumed);
                if (consumed != value.size())
                {
                    return ERR("float value '" + value + "' contains trailing characters");
                }
                return OK(v);
            }
            catch (const std::invalid_argument&)
            {
                return ERR("float value '" + value + "' is not a valid number");
            }
            catch (const std::out_of_range&)
            {
                return ERR("float value '" + value + "' exceeds the double range");
            }
        }

        // Parse a time value of the form ``<number><unit>`` with the unit in
        // {fs, ps, ns, us, ms, s, min, h}. The number is a non-negative decimal
        // (no scientific notation; whitespace between the number and unit is rejected).
        Result<std::monostate> parse_time(const std::string& value)
        {
            if (value.empty())
            {
                return ERR("empty time value");
            }
            size_t i = 0;
            while (i < value.size() && (std::isdigit(static_cast<unsigned char>(value[i])) || value[i] == '.'))
            {
                ++i;
            }
            if (i == 0)
            {
                return ERR("time value '" + value + "' has no numeric prefix");
            }
            if (i == value.size())
            {
                return ERR("time value '" + value + "' is missing a unit suffix");
            }
            const std::string num_str                         = value.substr(0, i);
            const std::string unit                            = value.substr(i);
            static const std::vector<std::string> valid_units = {"fs", "ps", "ns", "us", "ms", "s", "min", "h"};
            if (std::find(valid_units.begin(), valid_units.end(), unit) == valid_units.end())
            {
                return ERR("time value '" + value + "' has invalid unit '" + unit + "'");
            }
            try
            {
                size_t consumed = 0;
                (void)std::stod(num_str, &consumed);
                if (consumed != num_str.size())
                {
                    return ERR("time value '" + value + "' has non-numeric prefix '" + num_str + "'");
                }
            }
            catch (const std::invalid_argument&)
            {
                return ERR("time value '" + value + "' has invalid numeric prefix '" + num_str + "'");
            }
            catch (const std::out_of_range&)
            {
                return ERR("time value '" + value + "' has numeric prefix '" + num_str + "' out of range");
            }
            return OK({});
        }
    }    // namespace

    Parameter::Parameter()
    {
    }

    const std::string& Parameter::get_name() const
    {
        return name;
    }

    Parameter::Type Parameter::get_type() const
    {
        return type;
    }

    u16 Parameter::get_size() const
    {
        return size;
    }

    const std::string& Parameter::get_default_value() const
    {
        return default_value;
    }

    const std::vector<std::string>& Parameter::get_enum_values() const
    {
        return enum_values;
    }

    Result<Parameter> Parameter::Boolean(const std::string& name, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'Boolean': parameter name is empty");
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::Boolean;
        param.name          = name;
        param.size          = 1;
        param.default_value = default_value;

        if (!default_value.empty() && !param.validate(default_value))
        {
            return ERR("could not create parameter with name '" + name + "' of type 'Boolean': default value '" + default_value + "' is invalid");
        }

        return OK(param);
    }

    Result<Parameter> Parameter::BitVector(const std::string& name, u16 size, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'BitVector': parameter name is empty");
        }
        if (size == 0)
        {
            return ERR("could not create parameter with name '" + name + "' of type 'BitVector': size must be at least 1");
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::BitVector;
        param.name          = name;
        param.size          = size;
        param.default_value = normalize_value(Type::BitVector, default_value);

        if (!default_value.empty() && !param.validate(param.default_value))
        {
            return ERR("could not create parameter with name '" + name + "' of type 'BitVector': default value '" + param.default_value + "' is invalid");
        }

        return OK(param);
    }

    Result<Parameter> Parameter::LogicVector(const std::string& name, u16 size, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'LogicVector': parameter name is empty");
        }
        if (size == 0)
        {
            return ERR("could not create parameter with name '" + name + "' of type 'LogicVector': size must be at least 1");
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::LogicVector;
        param.name          = name;
        param.size          = size;
        param.default_value = normalize_value(Type::LogicVector, default_value);

        if (!default_value.empty() && !param.validate(param.default_value))
        {
            return ERR("could not create parameter with name '" + name + "' of type 'LogicVector': default value '" + param.default_value + "' is invalid");
        }

        return OK(param);
    }

    Result<Parameter> Parameter::Integer(const std::string& name, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'Integer': parameter name is empty");
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::Integer;
        param.name          = name;
        param.size          = 64;
        param.default_value = default_value;

        if (!default_value.empty() && !param.validate(default_value))
        {
            return ERR("could not create parameter with name '" + name + "' of type 'Integer': default value '" + default_value + "' is invalid");
        }

        return OK(param);
    }

    Result<Parameter> Parameter::String(const std::string& name, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'String': parameter name is empty");
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::String;
        param.name          = name;
        param.size          = 0;
        param.default_value = default_value;

        return OK(param);
    }

    Result<Parameter> Parameter::Float(const std::string& name, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'Float': parameter name is empty");
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::Float;
        param.name          = name;
        param.size          = 64;
        param.default_value = default_value;

        if (!default_value.empty() && !param.validate(default_value))
        {
            return ERR("could not create parameter with name '" + name + "' of type 'Float': default value '" + default_value + "' is invalid");
        }

        return OK(param);
    }

    Result<Parameter> Parameter::Time(const std::string& name, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'Time': parameter name is empty");
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::Time;
        param.name          = name;
        param.size          = 0;
        param.default_value = default_value;

        if (!default_value.empty() && !param.validate(default_value))
        {
            return ERR("could not create parameter with name '" + name + "' of type 'Time': default value '" + default_value + "' is invalid");
        }

        return OK(param);
    }

    Result<Parameter> Parameter::Enum(const std::string& name, const std::vector<std::string>& values, const std::string& default_value)
    {
        if (name.empty())
        {
            return ERR("could not create parameter of type 'Enum': parameter name is empty");
        }
        if (values.size() < 2)
        {
            return ERR("could not create parameter with name '" + name + "' of type 'Enum': at least two enum values are required");
        }
        {
            std::unordered_set<std::string> seen;
            for (const auto& v : values)
            {
                if (!seen.insert(v).second)
                {
                    return ERR("could not create parameter with name '" + name + "' of type 'Enum': duplicate enum value '" + v + "'");
                }
            }
        }

        auto param          = Parameter();
        param.type          = Parameter::Type::Enum;
        param.name          = name;
        param.size          = enum_bit_width(values.size());
        param.enum_values   = values;
        param.default_value = default_value;

        if (!default_value.empty() && !param.validate(default_value))
        {
            return ERR("could not create parameter with name '" + name + "' of type 'Enum': default value '" + param.default_value + "' is invalid");
        }

        return OK(param);
    }

    bool Parameter::validate(const std::string& value) const
    {
        const std::string v = normalize_value(type, value);
        switch (type)
        {
            case Type::Boolean: {
                return v == "true" || v == "false";
            }
            case Type::BitVector: {
                if (size <= 64)
                {
                    auto parsed = parse_bit_vector(v);
                    if (parsed.is_error())
                    {
                        return false;
                    }
                    const u64 n = parsed.get();
                    return size == 64 || (n >> size) == 0;
                }
                // size > 64: validate structurally — count significant bits without
                // converting to u64 (which would overflow for wide values).
                if (v.size() < 3 || v[0] != '0')
                {
                    return false;
                }
                const char bc = v[1];    // lowercase after normalize
                int bpd;
                std::string valid_chars;
                switch (bc)
                {
                    case 'b': bpd = 1; valid_chars = "01"; break;
                    case 'o': bpd = 3; valid_chars = "01234567"; break;
                    case 'x': bpd = 4; valid_chars = "0123456789ABCDEF"; break;
                    default: return false;
                }
                const std::string digits = v.substr(2);
                if (digits.empty())
                {
                    return false;
                }
                for (char c : digits)
                {
                    if (valid_chars.find(c) == std::string::npos)
                    {
                        return false;
                    }
                }
                // skip leading zero digits to find the most-significant non-zero digit
                size_t i = 0;
                while (i < digits.size() && digits[i] == '0')
                {
                    ++i;
                }
                if (i == digits.size())
                {
                    return true;    // value is zero — fits in any size
                }
                const char lead     = digits[i];    // uppercase after normalize
                const int  lead_val = (lead >= 'A') ? (lead - 'A' + 10) : (lead - '0');
                int        lead_bits = 0;
                while ((1 << lead_bits) <= lead_val)
                {
                    ++lead_bits;
                }
                const u64 sig_bits = static_cast<u64>(lead_bits) + static_cast<u64>(digits.size() - i - 1) * static_cast<u64>(bpd);
                return sig_bits <= size;
            }
            case Type::LogicVector: {
                auto parsed = parse_logic_vector(v);
                if (parsed.is_error())
                {
                    return false;
                }
                return parsed.get().size() <= size;
            }
            case Type::Integer: {
                return parse_integer(v).is_ok();
            }
            case Type::String: {
                return true;
            }
            case Type::Float: {
                return parse_float(v).is_ok();
            }
            case Type::Time: {
                return parse_time(v).is_ok();
            }
            case Type::Enum: {
                if (std::find(enum_values.begin(), enum_values.end(), v) == enum_values.end())
                {
                    return false;
                }
                return true;
            }
        }
        return false;
    }

    Result<u64> Parameter::encode_as_int(const std::string& value) const
    {
        const std::string v = normalize_value(type, value);
        switch (type)
        {
            case Type::Boolean: {
                if (v == "false")
                {
                    return OK(0u);
                }
                if (v == "true")
                {
                    return OK(1u);
                }
                return ERR("value '" + value + "' is not a valid boolean");
            }
            case Type::BitVector: {
                auto parsed = parse_bit_vector(v);
                if (parsed.is_error())
                {
                    return ERR_APPEND(parsed.get_error(), "value '" + value + "' is not a valid bit-vector");
                }
                const u64 n = parsed.get();
                if (size < 64 && (n >> size) != 0)
                {
                    return ERR("value '" + value + "' does not fit in " + std::to_string(size) + " bits");
                }
                return OK(n);
            }
            case Type::Integer: {
                auto parsed = parse_integer(v);
                if (parsed.is_error())
                {
                    return ERR_APPEND(parsed.get_error(), "value '" + value + "' is not a valid integer");
                }
                return OK(static_cast<u64>(parsed.get()));
            }
            case Type::LogicVector:
            case Type::String:
            case Type::Float:
            case Type::Time: {
                return ERR("parameter of type '" + enum_to_string<Parameter::Type>(type) + "' has no canonical integer encoding");
            }
            case Type::Enum: {
                for (size_t i = 0; i < enum_values.size(); ++i)
                {
                    if (enum_values[i] == value)
                    {
                        return OK(static_cast<u64>(i));
                    }
                }
                return ERR("value '" + value + "' is not a declared enum value");
            }
        }
        return ERR("unknown parameter type");
    }

    bool Parameter::operator==(const Parameter& other) const
    {
        return name == other.name && type == other.type && size == other.size && default_value == other.default_value && enum_values == other.enum_values;
    }

    bool Parameter::operator!=(const Parameter& other) const
    {
        return !(*this == other);
    }
}    // namespace hal
