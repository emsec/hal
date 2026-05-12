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
                if (c == 'b' || c == 'B')
                {
                    base = 2;
                }
                else if (c == 'o' || c == 'O')
                {
                    base = 8;
                }
                else if (c == 'x' || c == 'X')
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

        // Returns true if `c` (already lowercased) is one of the 9 VHDL std_logic
        // state characters: 0, 1, X (forcing unknown), Z (high-impedance),
        // U (uninitialized), L (weak 0), H (weak 1), W (weak unknown), - (don't-care).
        bool is_logic_state_char(char c)
        {
            return c == '0' || c == '1' || c == 'x' || c == 'z' || c == 'u' || c == 'l' || c == 'h' || c == 'w' || c == '-';
        }

        // Parse a 9-state logic-vector literal "0b<bits>", "0o<digits>", or "0x<digits>".
        // Each digit is expanded to its bits (MSB first). A state character (any of
        // X / Z / U / L / H / W / -, case-insensitive) expands to N copies of itself
        // where N is 1, 3, or 4 depending on the base. Returns the equivalent
        // per-bit string (lowercase, characters in {`0`, `1`, `x`, `z`, `u`, `l`,
        // `h`, `w`, `-`}).
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
            const char base_char = static_cast<char>(std::tolower(static_cast<unsigned char>(value[1])));
            int bits_per_digit;
            std::string numeric_lower;
            switch (base_char)
            {
                case 'b':
                    bits_per_digit = 1;
                    numeric_lower  = "01";
                    break;
                case 'o':
                    bits_per_digit = 3;
                    numeric_lower  = "01234567";
                    break;
                case 'x':
                    bits_per_digit = 4;
                    numeric_lower  = "0123456789abcdef";
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
                // '-' has no case; everything else lowercases cleanly.
                const char l          = (c == '-') ? '-' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                const bool is_numeric = numeric_lower.find(l) != std::string::npos;
                const bool is_state   = is_logic_state_char(l) && l != '0' && l != '1';
                if (!is_numeric && !is_state)
                {
                    return ERR("logic-vector value '" + value + "' contains invalid digit '" + std::string(1, c) + "'");
                }
                if (is_state)
                {
                    out.append(static_cast<size_t>(bits_per_digit), l);
                }
                else
                {
                    const int n = (l >= '0' && l <= '9') ? (l - '0') : (10 + (l - 'a'));
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
        param.default_value = default_value;

        if (!default_value.empty() && !param.validate(default_value))
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

        auto param          = Parameter();
        param.type          = Parameter::Type::LogicVector;
        param.name          = name;
        param.size          = size;
        param.default_value = default_value;

        if (!default_value.empty() && !param.validate(default_value))
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
        switch (type)
        {
            case Type::Boolean: {
                return value == "true" || value == "false";
            }
            case Type::BitVector: {
                auto parsed = parse_bit_vector(value);
                if (parsed.is_error())
                {
                    return false;
                }
                const u64 v = parsed.get();
                if (size < 64 && (v >> size) != 0)
                {
                    return false;
                }
                return true;
            }
            case Type::LogicVector: {
                auto parsed = parse_logic_vector(value);
                if (parsed.is_error())
                {
                    return false;
                }
                return parsed.get().size() <= size;
            }
            case Type::Integer: {
                return parse_integer(value).is_ok();
            }
            case Type::String: {
                return true;
            }
            case Type::Float: {
                return parse_float(value).is_ok();
            }
            case Type::Time: {
                return parse_time(value).is_ok();
            }
            case Type::Enum: {
                if (std::find(enum_values.begin(), enum_values.end(), value) == enum_values.end())
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
        switch (type)
        {
            case Type::Boolean: {
                if (value == "false")
                {
                    return OK(0u);
                }
                if (value == "true")
                {
                    return OK(1u);
                }
                return ERR("value '" + value + "' is not a valid boolean");
            }
            case Type::BitVector: {
                auto parsed = parse_bit_vector(value);
                if (parsed.is_error())
                {
                    return ERR_APPEND(parsed.get_error(), "value '" + value + "' is not a valid bit-vector");
                }
                const u64 v = parsed.get();
                if (size < 64 && (v >> size) != 0)
                {
                    return ERR("value '" + value + "' does not fit in " + std::to_string(size) + " bits");
                }
                return OK(v);
            }
            case Type::Integer: {
                auto parsed = parse_integer(value);
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
