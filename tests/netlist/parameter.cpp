#include "hal_core/netlist/parameter.h"

#include "hal_core/utilities/enums.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    class ParameterTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }
    };

    /**
     * Testing the Boolean factory: accepts only the values ``"true"`` and ``"false"``.
     *
     * Functions: Parameter::Boolean
     */
    TEST_F(ParameterTest, check_boolean_factory)
    {
        TEST_START
        {
            // Both canonical spellings are accepted; size is fixed to 1.
            auto res = Parameter::Boolean("flag", "false");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "flag");
            EXPECT_EQ(p.get_type(), Parameter::Type::Boolean);
            EXPECT_EQ(p.get_size(), 1u);
            EXPECT_EQ(p.get_default_value(), "false");
            EXPECT_TRUE(p.get_enum_values().empty());

            EXPECT_TRUE(Parameter::Boolean("flag", "true").is_ok());
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Boolean("", "true").is_error());
        }
        {
            // Boolean is strict: only the lower-case spellings are accepted.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Boolean("flag", "True").is_error());
            EXPECT_TRUE(Parameter::Boolean("flag", "0").is_error());
            EXPECT_TRUE(Parameter::Boolean("flag", "1").is_error());
        }
        TEST_END
    }

    /**
     * Testing the BitVector factory: well-formed declarations succeed and
     * ill-formed ones (empty name, default outside the bit-width) fail.
     *
     * Functions: Parameter::BitVector
     */
    TEST_F(ParameterTest, check_bit_vector_factory)
    {
        TEST_START
        {
            // Positive: typical 16-bit declaration with a hex default.
            auto res = Parameter::BitVector("width", 16, "0xCAFE");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "width");
            EXPECT_EQ(p.get_type(), Parameter::Type::BitVector);
            EXPECT_EQ(p.get_size(), 16u);
            EXPECT_EQ(p.get_default_value(), "0xCAFE");
            EXPECT_TRUE(p.get_enum_values().empty());
        }
        {
            // The full 64-bit range is allowed.
            auto res = Parameter::BitVector("full", 64, "0xFFFFFFFFFFFFFFFF");
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().get_size(), 64u);
        }
        {
            // Single-bit declarations are allowed.
            EXPECT_TRUE(Parameter::BitVector("flag", 1, "0b1").is_ok());
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::BitVector("", 8, "0").is_error());
        }
        {
            // Default value doesn't fit in the declared width.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::BitVector("overflow", 4, "0x10").is_error());
        }
        {
            // Default value is not parseable as a bit-vector.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::BitVector("garbage", 8, "not_a_number").is_error());
        }
        TEST_END
    }

    /**
     * Testing the LogicVector factory: accepts 4-state literals across binary,
     * octal, and hex bases; ``X`` and ``Z`` digits expand to the appropriate
     * number of state bits.
     *
     * Functions: Parameter::LogicVector
     */
    TEST_F(ParameterTest, check_logic_vector_factory)
    {
        TEST_START
        {
            // Binary literal with mixed 0/1/X/Z bits.
            auto res = Parameter::LogicVector("bus", 4, "0b10XZ");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "bus");
            EXPECT_EQ(p.get_type(), Parameter::Type::LogicVector);
            EXPECT_EQ(p.get_size(), 4u);
            EXPECT_EQ(p.get_default_value(), "0b10XZ");
            EXPECT_TRUE(p.get_enum_values().empty());
        }
        {
            // Octal and hex literals: each digit expands to 3 / 4 bits.
            EXPECT_TRUE(Parameter::LogicVector("oct", 6, "0o7X").is_ok());        // 6 bits
            EXPECT_TRUE(Parameter::LogicVector("hex", 8, "0xFZ").is_ok());        // 8 bits
            EXPECT_TRUE(Parameter::LogicVector("hex_mix", 12, "0x1XZ").is_ok());  // 12 bits
        }
        {
            // Lowercase x/z digits work and pure 0/1 literals are also fine.
            EXPECT_TRUE(Parameter::LogicVector("a", 4, "0b10xz").is_ok());
            EXPECT_TRUE(Parameter::LogicVector("b", 4, "0b1010").is_ok());
        }
        {
            // All 9 std_logic state characters (U / L / H / W / -) are accepted,
            // both directly as bits and as expanding state-digits in octal/hex.
            EXPECT_TRUE(Parameter::LogicVector("states_b", 9, "0bULHW-XZ01").is_ok());
            EXPECT_TRUE(Parameter::LogicVector("states_o", 6, "0oUL").is_ok());     // 6 bits (2 octal digits)
            EXPECT_TRUE(Parameter::LogicVector("states_x", 8, "0xH-").is_ok());     // 8 bits (2 hex digits)
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::LogicVector("", 4, "0b0").is_error());
        }
        {
            // Default value is longer than the declared bit-width.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::LogicVector("narrow", 4, "0b10101").is_error());
            EXPECT_TRUE(Parameter::LogicVector("narrow_hex", 4, "0xFF").is_error());    // 8 bits in a 4-bit decl
        }
        {
            // Missing base prefix and unparseable digits are rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::LogicVector("no_prefix", 4, "10XZ").is_error());
            EXPECT_TRUE(Parameter::LogicVector("garbage", 4, "0b10!1").is_error());
        }
        TEST_END
    }

    /**
     * Testing the Integer factory: signed decimal values are accepted up to the
     * 64-bit range; ill-formed defaults are rejected.
     *
     * Functions: Parameter::Integer
     */
    TEST_F(ParameterTest, check_integer_factory)
    {
        TEST_START
        {
            // Positive defaults; size is fixed to 64.
            auto res = Parameter::Integer("count", "42");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "count");
            EXPECT_EQ(p.get_type(), Parameter::Type::Integer);
            EXPECT_EQ(p.get_size(), 64u);
            EXPECT_EQ(p.get_default_value(), "42");
            EXPECT_TRUE(p.get_enum_values().empty());
        }
        {
            // Leading sign, zero, and the i64 extremes are accepted.
            EXPECT_TRUE(Parameter::Integer("delta", "-7").is_ok());
            EXPECT_TRUE(Parameter::Integer("plus", "+7").is_ok());
            EXPECT_TRUE(Parameter::Integer("zero", "0").is_ok());
            EXPECT_TRUE(Parameter::Integer("max", "9223372036854775807").is_ok());
            EXPECT_TRUE(Parameter::Integer("min", "-9223372036854775808").is_ok());
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Integer("", "0").is_error());
        }
        {
            // Out-of-range values are rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Integer("oob", "9999999999999999999").is_error());
        }
        {
            // Unparseable / hex / empty defaults are rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Integer("garbage", "abc").is_error());
            EXPECT_TRUE(Parameter::Integer("hex", "0xCAFE").is_error());
        }
        TEST_END
    }

    /**
     * Testing the String factory: any value (including the empty string) is
     * accepted as default; only the empty-name case fails.
     *
     * Functions: Parameter::String
     */
    TEST_F(ParameterTest, check_string_factory)
    {
        TEST_START
        {
            // Non-empty default is preserved verbatim.
            auto res = Parameter::String("note", "hello world");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "note");
            EXPECT_EQ(p.get_type(), Parameter::Type::String);
            EXPECT_EQ(p.get_size(), 0u);
            EXPECT_EQ(p.get_default_value(), "hello world");
            EXPECT_TRUE(p.get_enum_values().empty());
        }
        {
            // Empty string is a valid default.
            auto res = Parameter::String("blank", "");
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().get_default_value(), "");
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::String("", "x").is_error());
        }
        TEST_END
    }

    /**
     * Testing the Float factory: defaults parseable by ``std::stod`` succeed.
     *
     * Functions: Parameter::Float
     */
    TEST_F(ParameterTest, check_float_factory)
    {
        TEST_START
        {
            // Decimal, exponent, and negative defaults are accepted.
            auto res = Parameter::Float("pi", "3.14");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "pi");
            EXPECT_EQ(p.get_type(), Parameter::Type::Float);
            EXPECT_EQ(p.get_size(), 64u);
            EXPECT_EQ(p.get_default_value(), "3.14");
            EXPECT_TRUE(p.get_enum_values().empty());

            EXPECT_TRUE(Parameter::Float("exp", "1e-5").is_ok());
            EXPECT_TRUE(Parameter::Float("neg", "-2.5").is_ok());
            EXPECT_TRUE(Parameter::Float("int_form", "42").is_ok());
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Float("", "0.0").is_error());
        }
        {
            // Unparseable / empty defaults are rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Float("garbage", "abc").is_error());
            EXPECT_TRUE(Parameter::Float("trailing", "3.14ns").is_error());
        }
        TEST_END
    }

    /**
     * Testing the Time factory: values of the form ``<number><unit>`` are accepted
     * for the standard SystemVerilog units.
     *
     * Functions: Parameter::Time
     */
    TEST_F(ParameterTest, check_time_factory)
    {
        TEST_START
        {
            // Typical value with a unit suffix.
            auto res = Parameter::Time("delay", "10ns");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "delay");
            EXPECT_EQ(p.get_type(), Parameter::Type::Time);
            EXPECT_EQ(p.get_size(), 0u);
            EXPECT_EQ(p.get_default_value(), "10ns");
            EXPECT_TRUE(p.get_enum_values().empty());
        }
        {
            // All standard units are accepted; fractional and zero values OK.
            EXPECT_TRUE(Parameter::Time("a", "100fs").is_ok());
            EXPECT_TRUE(Parameter::Time("b", "1.5ps").is_ok());
            EXPECT_TRUE(Parameter::Time("c", "250ns").is_ok());
            EXPECT_TRUE(Parameter::Time("d", "5us").is_ok());
            EXPECT_TRUE(Parameter::Time("e", "1ms").is_ok());
            EXPECT_TRUE(Parameter::Time("f", "2s").is_ok());
            EXPECT_TRUE(Parameter::Time("g", "0ps").is_ok());
            EXPECT_TRUE(Parameter::Time("h", "30min").is_ok());
            EXPECT_TRUE(Parameter::Time("i", "1.5h").is_ok());
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Time("", "1ns").is_error());
        }
        {
            // Missing or unknown unit suffixes are rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Time("no_unit", "10").is_error());
            EXPECT_TRUE(Parameter::Time("bad_unit", "10sec").is_error());
            EXPECT_TRUE(Parameter::Time("upper_unit", "10NS").is_error());
        }
        {
            // Whitespace between the number and the unit is not allowed.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Time("spaced", "10 ns").is_error());
        }
        {
            // Garbage / empty defaults are rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Time("garbage", "abc").is_error());
        }
        TEST_END
    }

    /**
     * Testing the Enum factory: well-formed declarations succeed; size is
     * auto-derived from the value count; ill-formed declarations (empty name,
     * default not among values) fail.
     *
     * Functions: Parameter::Enum
     */
    TEST_F(ParameterTest, check_enum_factory)
    {
        TEST_START
        {
            // Two values fit in 1 bit.
            auto res = Parameter::Enum("mode", {"normal", "inverted"}, "normal");
            ASSERT_TRUE(res.is_ok());
            const Parameter p = res.get();
            EXPECT_EQ(p.get_name(), "mode");
            EXPECT_EQ(p.get_type(), Parameter::Type::Enum);
            EXPECT_EQ(p.get_size(), 1u);
            EXPECT_EQ(p.get_default_value(), "normal");
            EXPECT_EQ(p.get_enum_values(), std::vector<std::string>({"normal", "inverted"}));
        }
        {
            // Four values fit in 2 bits.
            auto res = Parameter::Enum("speed", {"slow", "fast", "very_fast", "blazing"}, "fast");
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().get_size(), 2u);
        }
        {
            // Non-power-of-two value counts round up.
            auto res = Parameter::Enum("trit", {"a", "b", "c"}, "a");
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().get_size(), 2u);
        }

        // Negative
        {
            // Empty name is rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Enum("", {"a", "b"}, "a").is_error());
        }
        {
            // Default value must be among the declared values.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Enum("color", {"red", "green"}, "blue").is_error());
        }
        TEST_END
    }

    /**
     * Testing Parameter::validate for every type tag. Each type's accepted-value
     * grammar is exercised positively and negatively.
     *
     * Functions: Parameter::validate
     */
    TEST_F(ParameterTest, check_validate)
    {
        TEST_START
        {
            // Boolean validation accepts exactly "true" or "false".
            const Parameter p = Parameter::Boolean("flag", "false").get();
            EXPECT_TRUE(p.validate("true"));
            EXPECT_TRUE(p.validate("false"));
            EXPECT_FALSE(p.validate(""));
            EXPECT_FALSE(p.validate("True"));     // case-sensitive
            EXPECT_FALSE(p.validate("0"));
            EXPECT_FALSE(p.validate("1"));
        }
        {
            // Bit-vector validation across bases.
            const Parameter p = Parameter::BitVector("v", 16, "0b0").get();
            EXPECT_TRUE(p.validate("0b1010"));
            EXPECT_TRUE(p.validate("0o17"));
            EXPECT_TRUE(p.validate("0xCAFE"));
            EXPECT_TRUE(p.validate("0x0"));

            // Out-of-range / unparseable values fail.
            EXPECT_FALSE(p.validate("0x10000"));    // overflow
            EXPECT_FALSE(p.validate(""));
            EXPECT_FALSE(p.validate("garbage"));
            EXPECT_FALSE(p.validate("0xZZ"));
            EXPECT_FALSE(p.validate("0b"));         // base prefix with no digits
        }
        {
            // 64-bit-wide bit-vector accepts the full unsigned range.
            const Parameter p = Parameter::BitVector("full", 64, "0b0").get();
            EXPECT_TRUE(p.validate("0xFFFFFFFFFFFFFFFF"));
        }
        {
            // LogicVector accepts 9-state literals across all three bases, sized to fit.
            const Parameter p = Parameter::LogicVector("bus", 9, "0b0").get();
            EXPECT_TRUE(p.validate("0b10XZ"));
            EXPECT_TRUE(p.validate("0b10xz"));
            EXPECT_TRUE(p.validate("0b1010"));
            EXPECT_TRUE(p.validate("0b0"));      // shorter than size is fine
            EXPECT_TRUE(p.validate("0o7"));      // 3 bits
            EXPECT_TRUE(p.validate("0xF"));      // 4 bits
            EXPECT_TRUE(p.validate("0xZ"));      // expands to 4 Z bits

            // All 9 std_logic states are valid bits.
            EXPECT_TRUE(p.validate("0bULHW-XZ01"));    // 9 bits in size=9
            EXPECT_TRUE(p.validate("0bu"));
            EXPECT_TRUE(p.validate("0bl"));
            EXPECT_TRUE(p.validate("0bh"));
            EXPECT_TRUE(p.validate("0bw"));
            EXPECT_TRUE(p.validate("0b-"));

            // Length overflows, missing prefix, garbage, empty all fail.
            EXPECT_FALSE(p.validate("0bULHW-XZ010"));   // 10 bits doesn't fit in size=9
            EXPECT_FALSE(p.validate("10XZ"));
            EXPECT_FALSE(p.validate("0b!"));
            EXPECT_FALSE(p.validate(""));
        }
        {
            // Integer validation accepts decimal numbers (with optional sign).
            const Parameter p = Parameter::Integer("count", "0").get();
            EXPECT_TRUE(p.validate("0"));
            EXPECT_TRUE(p.validate("42"));
            EXPECT_TRUE(p.validate("-1"));
            EXPECT_TRUE(p.validate("+7"));
            EXPECT_TRUE(p.validate("9223372036854775807"));     // i64 max
            EXPECT_TRUE(p.validate("-9223372036854775808"));    // i64 min

            // Hex literals, garbage, trailing chars, and the empty string are rejected.
            EXPECT_FALSE(p.validate(""));
            EXPECT_FALSE(p.validate("0x10"));
            EXPECT_FALSE(p.validate("abc"));
            EXPECT_FALSE(p.validate("42abc"));
            EXPECT_FALSE(p.validate("9999999999999999999"));    // overflow
        }
        {
            // String validation accepts any value, including the empty string.
            const Parameter p = Parameter::String("note", "").get();
            EXPECT_TRUE(p.validate(""));
            EXPECT_TRUE(p.validate("hello"));
            EXPECT_TRUE(p.validate("with spaces"));
            EXPECT_TRUE(p.validate("0xCAFE"));     // numeric-looking strings still accepted
        }
        {
            // Float validation accepts decimals, exponents, and integer-form literals.
            const Parameter p = Parameter::Float("pi", "0").get();
            EXPECT_TRUE(p.validate("3.14"));
            EXPECT_TRUE(p.validate("-2.5"));
            EXPECT_TRUE(p.validate("1e-5"));
            EXPECT_TRUE(p.validate("0"));

            // Empty / non-numeric / trailing characters fail.
            EXPECT_FALSE(p.validate(""));
            EXPECT_FALSE(p.validate("abc"));
            EXPECT_FALSE(p.validate("3.14ns"));
        }
        {
            // Time validation requires a contiguous ``<number><unit>``.
            const Parameter p = Parameter::Time("delay", "0ps").get();
            EXPECT_TRUE(p.validate("10ns"));
            EXPECT_TRUE(p.validate("1.5us"));
            EXPECT_TRUE(p.validate("0fs"));
            EXPECT_TRUE(p.validate("100ms"));
            EXPECT_TRUE(p.validate("1s"));
            EXPECT_TRUE(p.validate("45min"));
            EXPECT_TRUE(p.validate("2h"));

            // Missing unit, wrong unit, whitespace, and garbage fail.
            EXPECT_FALSE(p.validate("10"));
            EXPECT_FALSE(p.validate("10sec"));
            EXPECT_FALSE(p.validate("10 ns"));
            EXPECT_FALSE(p.validate("10NS"));
            EXPECT_FALSE(p.validate("ns"));
            EXPECT_FALSE(p.validate(""));
            EXPECT_FALSE(p.validate("10m"));      // "m" alone is not a valid unit
            EXPECT_FALSE(p.validate("10hr"));     // "hr" is not the canonical hour unit
        }
        {
            // Enum validation accepts exactly the declared values.
            const Parameter p = Parameter::Enum("mode", {"normal", "inverted"}, "normal").get();
            EXPECT_TRUE(p.validate("normal"));
            EXPECT_TRUE(p.validate("inverted"));
            EXPECT_FALSE(p.validate(""));
            EXPECT_FALSE(p.validate("Normal"));        // case-sensitive
            EXPECT_FALSE(p.validate("unknown"));
        }
        TEST_END
    }

    /**
     * Testing Parameter::encode_as_int across all type tags.
     *
     * Functions: Parameter::encode_as_int
     */
    TEST_F(ParameterTest, check_encode_as_int)
    {
        TEST_START
        {
            // Boolean encodes to 0 or 1; anything else is an error.
            const Parameter p = Parameter::Boolean("flag", "false").get();
            EXPECT_EQ(p.encode_as_int("false").get(), 0u);
            EXPECT_EQ(p.encode_as_int("true").get(), 1u);
            EXPECT_TRUE(p.encode_as_int("True").is_error());
            EXPECT_TRUE(p.encode_as_int("0").is_error());
            EXPECT_TRUE(p.encode_as_int("").is_error());
        }
        {
            // Bit-vector encoding parses the value across bases.
            const Parameter p = Parameter::BitVector("v", 16, "0b0").get();
            EXPECT_EQ(p.encode_as_int("0b1010").get(), 0b1010u);
            EXPECT_EQ(p.encode_as_int("0o17").get(), 017u);
            EXPECT_EQ(p.encode_as_int("0xCAFE").get(), 0xCAFEu);
            EXPECT_EQ(p.encode_as_int("0x0").get(), 0u);

            // Values that overflow or fail to parse are rejected.
            EXPECT_TRUE(p.encode_as_int("0x10000").is_error());
            EXPECT_TRUE(p.encode_as_int("garbage").is_error());
        }
        {
            // Integer encoding reinterprets the i64 bit-pattern as u64. Negative
            // values therefore appear as two's-complement u64s.
            const Parameter p = Parameter::Integer("count", "0").get();
            EXPECT_EQ(p.encode_as_int("0").get(), 0u);
            EXPECT_EQ(p.encode_as_int("42").get(), 42u);
            EXPECT_EQ(p.encode_as_int("-1").get(), 0xFFFFFFFFFFFFFFFFu);
            EXPECT_EQ(p.encode_as_int("9223372036854775807").get(), 0x7FFFFFFFFFFFFFFFu);    // i64 max
            EXPECT_TRUE(p.encode_as_int("abc").is_error());
            EXPECT_TRUE(p.encode_as_int("").is_error());
        }
        {
            // LogicVector / String / Float / Time have no canonical integer encoding.
            const Parameter pl = Parameter::LogicVector("bus", 4, "0b0").get();
            EXPECT_TRUE(pl.encode_as_int("0b1010").is_error());    // even pure 0/1 values error
            EXPECT_TRUE(pl.encode_as_int("0b10XZ").is_error());

            const Parameter ps = Parameter::String("note", "").get();
            EXPECT_TRUE(ps.encode_as_int("").is_error());
            EXPECT_TRUE(ps.encode_as_int("anything").is_error());

            const Parameter pf = Parameter::Float("pi", "0").get();
            EXPECT_TRUE(pf.encode_as_int("3.14").is_error());

            const Parameter pt = Parameter::Time("delay", "0ps").get();
            EXPECT_TRUE(pt.encode_as_int("10ns").is_error());
        }
        {
            // Enum encoding returns the zero-based index in the value list.
            const Parameter p = Parameter::Enum("speed", {"slow", "fast", "very_fast"}, "slow").get();
            EXPECT_EQ(p.encode_as_int("slow").get(), 0u);
            EXPECT_EQ(p.encode_as_int("fast").get(), 1u);
            EXPECT_EQ(p.encode_as_int("very_fast").get(), 2u);
            EXPECT_TRUE(p.encode_as_int("unknown").is_error());
        }
        TEST_END
    }

    /**
     * Testing the empty-default policy: every factory accepts an empty
     * ``default_value`` (meaning "no default") without validating it. A
     * non-empty default still has to satisfy the type's value grammar.
     *
     * Functions: Parameter::Boolean, Parameter::BitVector, Parameter::LogicVector,
     *            Parameter::Integer, Parameter::String, Parameter::Float,
     *            Parameter::Time, Parameter::Enum
     */
    TEST_F(ParameterTest, check_empty_default)
    {
        TEST_START
        {
            // Every factory accepts an empty default_value.
            EXPECT_TRUE(Parameter::Boolean("flag", "").is_ok());
            EXPECT_TRUE(Parameter::BitVector("w", 16, "").is_ok());
            EXPECT_TRUE(Parameter::LogicVector("s", 4, "").is_ok());
            EXPECT_TRUE(Parameter::Integer("n", "").is_ok());
            EXPECT_TRUE(Parameter::String("note", "").is_ok());
            EXPECT_TRUE(Parameter::Float("pi", "").is_ok());
            EXPECT_TRUE(Parameter::Time("delay", "").is_ok());
            EXPECT_TRUE(Parameter::Enum("color", {"red", "green"}, "").is_ok());

            // The empty default round-trips through the struct.
            const Parameter p = Parameter::BitVector("w", 16, "").get();
            EXPECT_EQ(p.get_default_value(), "");
        }

        // Negative
        {
            // Non-empty but invalid defaults are still rejected.
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(Parameter::Boolean("flag", "yes").is_error());
            EXPECT_TRUE(Parameter::BitVector("w", 4, "0x10").is_error());
            EXPECT_TRUE(Parameter::LogicVector("s", 4, "0b10101").is_error());
            EXPECT_TRUE(Parameter::Integer("n", "abc").is_error());
            EXPECT_TRUE(Parameter::Float("pi", "abc").is_error());
            EXPECT_TRUE(Parameter::Time("delay", "10").is_error());
            EXPECT_TRUE(Parameter::Enum("color", {"red", "green"}, "blue").is_error());
        }
        TEST_END
    }

    /**
     * Testing equality: two parameters compare equal iff every field matches.
     *
     * Functions: operator==, operator!=
     */
    TEST_F(ParameterTest, check_equality)
    {
        TEST_START
        {
            // Identical declarations compare equal.
            const Parameter a = Parameter::BitVector("w", 16, "0b0").get();
            const Parameter b = Parameter::BitVector("w", 16, "0b0").get();
            EXPECT_TRUE(a == b);
            EXPECT_FALSE(a != b);
        }
        {
            // Any single mismatched field makes them unequal.
            const Parameter base = Parameter::BitVector("w", 16, "0b0").get();
            EXPECT_TRUE(base != Parameter::BitVector("w2", 16, "0b0").get());    // name
            EXPECT_TRUE(base != Parameter::BitVector("w", 8, "0b0").get());      // size
            EXPECT_TRUE(base != Parameter::BitVector("w", 16, "0x1").get());     // default
        }
        {
            // A BitVector and an Enum with the same name are never equal.
            const Parameter bv = Parameter::BitVector("x", 1, "0b0").get();
            const Parameter en = Parameter::Enum("x", {"a", "b"}, "a").get();
            EXPECT_TRUE(bv != en);
        }
        {
            // Enum value list participates in equality.
            const Parameter a = Parameter::Enum("c", {"r", "g", "b"}, "r").get();
            const Parameter b = Parameter::Enum("c", {"r", "g", "y"}, "r").get();
            EXPECT_TRUE(a != b);
        }
        TEST_END
    }

    /**
     * Testing the EnumStrings specialization that maps Parameter::Type to its
     * serialized name and back. Used by HGL / .hal round-trips.
     *
     * Functions: enum_to_string, enum_from_string
     */
    TEST_F(ParameterTest, check_type_enum_strings)
    {
        TEST_START
        EXPECT_EQ(enum_to_string(Parameter::Type::Boolean), "boolean");
        EXPECT_EQ(enum_to_string(Parameter::Type::BitVector), "bit_vector");
        EXPECT_EQ(enum_to_string(Parameter::Type::LogicVector), "logic_vector");
        EXPECT_EQ(enum_to_string(Parameter::Type::Integer), "integer");
        EXPECT_EQ(enum_to_string(Parameter::Type::String), "string");
        EXPECT_EQ(enum_to_string(Parameter::Type::Float), "float");
        EXPECT_EQ(enum_to_string(Parameter::Type::Time), "time");
        EXPECT_EQ(enum_to_string(Parameter::Type::Enum), "enum");

        EXPECT_EQ(enum_from_string<Parameter::Type>("boolean"), Parameter::Type::Boolean);
        EXPECT_EQ(enum_from_string<Parameter::Type>("bit_vector"), Parameter::Type::BitVector);
        EXPECT_EQ(enum_from_string<Parameter::Type>("logic_vector"), Parameter::Type::LogicVector);
        EXPECT_EQ(enum_from_string<Parameter::Type>("integer"), Parameter::Type::Integer);
        EXPECT_EQ(enum_from_string<Parameter::Type>("string"), Parameter::Type::String);
        EXPECT_EQ(enum_from_string<Parameter::Type>("float"), Parameter::Type::Float);
        EXPECT_EQ(enum_from_string<Parameter::Type>("time"), Parameter::Type::Time);
        EXPECT_EQ(enum_from_string<Parameter::Type>("enum"), Parameter::Type::Enum);
        TEST_END
    }
}    // namespace hal
