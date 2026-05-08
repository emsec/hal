#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/boolean_function/types.h"

#include <iostream>
#include <type_traits>
#include <variant>

namespace hal {
    TEST(BooleanFunction, EnumConstruction) {
        EXPECT_EQ(static_cast<BooleanFunction::Value>(0), BooleanFunction::Value::ZERO);
        EXPECT_EQ(static_cast<BooleanFunction::Value>(1), BooleanFunction::Value::ONE);
    }

    TEST(BooleanFunction, IsEmpty) {
        EXPECT_TRUE(BooleanFunction().is_empty());
        EXPECT_FALSE(BooleanFunction::Var("A").is_empty());
        EXPECT_FALSE(BooleanFunction::Const(0, 1).is_empty());
    }

    TEST(BooleanFunction, GetVariableNames) {
        auto a = BooleanFunction::Var("A"),
             b = BooleanFunction::Var("B"),
             c = BooleanFunction::Var("C"),
            _0 = BooleanFunction::Const(0, 1),
            _1 = BooleanFunction::Const(1, 1);

        EXPECT_EQ((a.clone() & b.clone()).get_variable_names(), std::set<std::string>({"A", "B"}));
        EXPECT_EQ(((a.clone() & b.clone()) | a.clone()).get_variable_names(), std::set<std::string>({"A", "B"}));
        EXPECT_EQ(((a.clone() & b.clone()) & c.clone()).get_variable_names(), std::set<std::string>({"A", "B", "C"}));
        EXPECT_EQ((_0.clone() & b.clone()).get_variable_names(), std::set<std::string>({"B"}));
        EXPECT_EQ((_0.clone() & _1.clone()).get_variable_names(), std::set<std::string>({}));
    }

    TEST(BooleanFunction, CopyMoveSemantics) {
        EXPECT_EQ( std::is_copy_constructible<BooleanFunction>::value, true);
        EXPECT_EQ( std::is_copy_assignable<BooleanFunction>::value, true);
        EXPECT_EQ( std::is_move_constructible<BooleanFunction>::value, true);
        EXPECT_EQ( std::is_move_assignable<BooleanFunction>::value, true);
    }

    TEST(BooleanFunction, Operator) {
        const auto a = BooleanFunction::Var("A"),
                   b = BooleanFunction::Var("B"),
                  _0 = BooleanFunction::Const(0, 1),
                  _1 = BooleanFunction::Const(1, 1);

        EXPECT_TRUE(a == a);
        EXPECT_TRUE(a != b);
        
        EXPECT_TRUE(_0 == _0);
        EXPECT_TRUE(_0 != _1);

        EXPECT_TRUE(a != _0);
    }

    TEST(BooleanFunction, ToString) {
        const auto a = BooleanFunction::Var("A"),
                   b = BooleanFunction::Var("B"),
                   c = BooleanFunction::Var("C"),
                  _0 = BooleanFunction::Const(0, 1),
                  _1 = BooleanFunction::Const(1, 1);

        const auto data = std::vector<std::tuple<std::string, BooleanFunction>>{
            {"<empty>", BooleanFunction()},
            {"(A & B)", a.clone() & b.clone()},
            {"(A & (B | C))", (a.clone() & (b.clone() | c.clone()))},
            {"((A & B) ^ (B & C))", ((a.clone() & b.clone()) ^ (b.clone() & c.clone()))},
            {"(A ^ 0b1)", a.clone() ^ _1.clone()}, 
            {"(A ^ 0b0)", a.clone() ^ _0.clone()},
            {"((A + B) - C)", (a.clone() + b.clone()) - c.clone()},
            {"(! A)", ~a.clone()},
        };

        for (auto&& [expected, function]: data) {
            EXPECT_EQ(expected, function.to_string());
        }
    }

    TEST(BooleanFunction, ValueToBin) {
        EXPECT_TRUE(BooleanFunction::to_string({}, 2).is_error());

        const auto data = std::vector<std::pair<std::string, std::vector<BooleanFunction::Value>>>{
            {"1", {BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::X}},
            {"Z", {BooleanFunction::Value::Z}},
            {"10101", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {"101X1", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::ONE}},
            {"101Z1", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::ONE}},
            {"101ZX", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::X}},
            {"101XZ", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::Z}},
            {std::string(10000, '1'), std::vector<BooleanFunction::Value>(10000, BooleanFunction::Value::ONE)},
        };

        for (auto&& [expected, value]: data) 
        {
            EXPECT_EQ(expected, BooleanFunction::to_string(value, 2).get());
        }
    }

    TEST(BooleanFunction, ValueToOct) {
        EXPECT_TRUE(BooleanFunction::to_string({}, 8).is_error());

        const auto data = std::vector<std::pair<std::string, std::vector<BooleanFunction::Value>>>{
            {"1", {BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::X}},
            {"X", {BooleanFunction::Value::Z}},
            {"25", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {"2X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::ONE}},
            {"2X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::ONE}},
            {"2X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::X}},
            {"2X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::Z}},
            {std::string(3334, '7'), std::vector<BooleanFunction::Value>(10002, BooleanFunction::Value::ONE)},
        };

        for (auto&& [expected, value]: data) 
        {
            EXPECT_EQ(expected, BooleanFunction::to_string(value, 8).get());
        }
    }

    TEST(BooleanFunction, ValueToDec) {
        EXPECT_TRUE(BooleanFunction::to_string({}, 10).is_error());
        EXPECT_TRUE(BooleanFunction::to_string(std::vector<BooleanFunction::Value>(10000, BooleanFunction::Value::ONE), 10).is_error());

        const auto data = std::vector<std::pair<std::string, std::vector<BooleanFunction::Value>>>{
            {"1", {BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::X}},
            {"X", {BooleanFunction::Value::Z}},
            {"21", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {"87", {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::X}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::Z}},
        };

        for (auto&& [expected, value]: data) 
        {
            EXPECT_EQ(expected, BooleanFunction::to_string(value, 10).get());
        }

        const auto res_137 = BooleanFunction::to_string(BooleanFunction::Const(137, 16).get_top_level_node().constant, 10);
        ASSERT_TRUE(res_137.is_ok());
        EXPECT_EQ(res_137.get(), "137");
    }

    TEST(BooleanFunction, ValueToHex) {
        EXPECT_TRUE(BooleanFunction::to_string({}, 10).is_error());

        const auto data = std::vector<std::pair<std::string, std::vector<BooleanFunction::Value>>>{
            {"1", {BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::X}},
            {"X", {BooleanFunction::Value::Z}},
            {"15", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {"1X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::ONE}},
            {"1X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::ONE}},
            {"1X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::X}},
            {"1X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::Z}},
            {std::string(2500, 'F'), std::vector<BooleanFunction::Value>(10000, BooleanFunction::Value::ONE)},
        };

        for (auto&& [expected, value]: data) 
        {
            EXPECT_EQ(expected, BooleanFunction::to_string(value, 16).get());
        }
    }

    TEST(BooleanFunction, Parser) {
        const std::vector<std::tuple<std::string, BooleanFunction>> data = {
            ////////////////////////////////////////////////////////////////////
            // GENERIC PARSER
            ////////////////////////////////////////////////////////////////////
            {"0", 
                BooleanFunction::Const(0, 1)
            },
            {"1", 
                BooleanFunction::Const(1, 1)
            },
            {"0b0", 
                BooleanFunction::Const(0, 1)
            },
            {"0b1", 
                BooleanFunction::Const(1, 1)
            },
            {"A & B", 
                BooleanFunction::Var("A") & BooleanFunction::Var("B")
            },
            {"(a & bb) | (ccc & dddd)", 
                (BooleanFunction::Var("a") & BooleanFunction::Var("bb")) | (BooleanFunction::Var("ccc") & BooleanFunction::Var("dddd"))
            },
            {"A(1) ^ B(1)", 
                BooleanFunction::Var("A(1)") ^ BooleanFunction::Var("B(1)")
            },
            {"!(a ^ a) ^ !(!(b ^ b))", 
                ~(BooleanFunction::Var("a") ^ BooleanFunction::Var("a")) ^ (~(~(BooleanFunction::Var("b") ^ BooleanFunction::Var("b"))))
            },
            {"(!I0 & I1 & I2) | (I0 & I1 & I2)", 
                (~BooleanFunction::Var("I0") & (BooleanFunction::Var("I1") & BooleanFunction::Var("I2"))) | (BooleanFunction::Var("I0") & (BooleanFunction::Var("I1") & BooleanFunction::Var("I2")))
            },
            {"(((0b1 & O[0]) & c3) | (RDATA[0] & (! c3)))",
                ((BooleanFunction::Const(1, 1) & BooleanFunction::Var("O[0]")) & BooleanFunction::Var("c3"))
                | (BooleanFunction::Var("RDATA[0]") & (~ BooleanFunction::Var("c3")))
            },
            {"(((0b1 & \\O[0] ) & c3) | (\\RDATA[0]  & (! c3)))",
                ((BooleanFunction::Const(1, 1) & BooleanFunction::Var("O[0]")) & BooleanFunction::Var("c3"))
                | (BooleanFunction::Var("RDATA[0]") & (~ BooleanFunction::Var("c3")))
            },
            ////////////////////////////////////////////////////////////////////
            // LIBERTY PARSER
            ////////////////////////////////////////////////////////////////////
            {"A B C D(1)",
                BooleanFunction::Var("A") & (BooleanFunction::Var("B") & (BooleanFunction::Var("C") & BooleanFunction::Var("D(1)")))
            },
            {"A'", 
                ~BooleanFunction::Var("A")
            },
            {"RSTB'",
                ~BooleanFunction::Var("RSTB")
            },
            {"(INP)'",
                ~BooleanFunction::Var("INP")
            },
            {"(IN2*IN1)'",
                ~(BooleanFunction::Var("IN2") & BooleanFunction::Var("IN1"))
            },
            {"(D'*CLK*RSTB*SETB')",
                ~BooleanFunction::Var("D") & (BooleanFunction::Var("CLK") & (BooleanFunction::Var("RSTB") & ~BooleanFunction::Var("SETB"))) 
            },
            {"(IN5*(IN2+IN1)*(IN3+IN4))'",
                ~(BooleanFunction::Var("IN5") & ((BooleanFunction::Var("IN2") | BooleanFunction::Var("IN1")) & (BooleanFunction::Var("IN3") | BooleanFunction::Var("IN4"))))
            }
        };

        for (const auto& [s, expected] : data) {
            auto function = BooleanFunction::from_string(s);
            ASSERT_TRUE(function.is_ok());
            ASSERT_EQ(function.get(), expected);
        }
    }

    TEST(BooleanFunction, ParserTernaryAndEq) {
        // Single ternary
        {
            auto fn = BooleanFunction::from_string("a ? b : c");
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Ite(BooleanFunction::Var("a"), BooleanFunction::Var("b"), BooleanFunction::Var("c"), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Equality with constants
        {
            auto fn = BooleanFunction::from_string("a == 1");
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Eq(BooleanFunction::Var("a"), BooleanFunction::Const(1, 1), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Ternary whose condition is a comparison
        {
            auto fn = BooleanFunction::from_string("(p == 1) ? a : b");
            ASSERT_TRUE(fn.is_ok());
            auto cond = BooleanFunction::Eq(BooleanFunction::Var("p"), BooleanFunction::Const(1, 1), 1);
            ASSERT_TRUE(cond.is_ok());
            auto expected = BooleanFunction::Ite(cond.get(), BooleanFunction::Var("a"), BooleanFunction::Var("b"), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Nested ternary with explicit parens — outer cond is `a`, then-branch is the inner ternary
        {
            auto fn = BooleanFunction::from_string("a ? (b ? c : d) : e");
            ASSERT_TRUE(fn.is_ok());
            auto inner = BooleanFunction::Ite(BooleanFunction::Var("b"), BooleanFunction::Var("c"), BooleanFunction::Var("d"), 1);
            ASSERT_TRUE(inner.is_ok());
            auto expected = BooleanFunction::Ite(BooleanFunction::Var("a"), inner.get(), BooleanFunction::Var("e"), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Multi-bit binary constant
        {
            auto fn = BooleanFunction::from_string("0b101");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Const(5, 3));
        }
        // Multi-bit hex constant (each digit = 4 bits)
        {
            auto fn = BooleanFunction::from_string("0xA");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Const(10, 4));
        }
        {
            auto fn = BooleanFunction::from_string("0xff");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Const(0xff, 8));
        }
        // Multi-bit octal constant (each digit = 3 bits)
        {
            auto fn = BooleanFunction::from_string("0o7");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Const(7, 3));
        }
        {
            auto fn = BooleanFunction::from_string("0o17");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Const(0017, 6));
        }
        // Unparenthesized nested ternary parses with right-associativity:
        // `a ? b : c ? d : e` == `a ? b : (c ? d : e)`
        {
            auto fn = BooleanFunction::from_string("a ? b : c ? d : e");
            ASSERT_TRUE(fn.is_ok());
            auto inner = BooleanFunction::Ite(BooleanFunction::Var("c"), BooleanFunction::Var("d"), BooleanFunction::Var("e"), 1);
            ASSERT_TRUE(inner.is_ok());
            auto expected = BooleanFunction::Ite(BooleanFunction::Var("a"), BooleanFunction::Var("b"), inner.get(), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Equality with multi-bit constants
        {
            auto fn = BooleanFunction::from_string("0b101 == 0b101");
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Eq(BooleanFunction::Const(5, 3), BooleanFunction::Const(5, 3), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        {
            auto fn = BooleanFunction::from_string("0xA == 0xA");
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Eq(BooleanFunction::Const(10, 4), BooleanFunction::Const(10, 4), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
    }

    TEST(BooleanFunction, ParserTernarySimplify) {
        // `1 ? a : b` reduces to `a`
        {
            auto fn = BooleanFunction::from_string("1 ? a : b");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Var("a"));
        }
        // `0 ? a : b` reduces to `b`
        {
            auto fn = BooleanFunction::from_string("0 ? a : b");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Var("b"));
        }
        // `(1 == 1) ? a : b` reduces to `a`
        {
            auto fn = BooleanFunction::from_string("(1 == 1) ? a : b");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Var("a"));
        }
        // `(1 == 0) ? a : b` reduces to `b`
        {
            auto fn = BooleanFunction::from_string("(1 == 0) ? a : b");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Var("b"));
        }
        // Substitute a parameter, then simplify reduces the ternary to one branch.
        {
            auto fn = BooleanFunction::from_string("(p == 1) ? a : b");
            ASSERT_TRUE(fn.is_ok());
            auto subst = fn.get().substitute("p", BooleanFunction::Const(1, 1));
            ASSERT_TRUE(subst.is_ok());
            EXPECT_EQ(subst.get().simplify(), BooleanFunction::Var("a"));
        }
        // `cond ? x : x` simplifies to `x` regardless of cond.
        {
            auto fn = BooleanFunction::from_string("cond ? x : x");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Var("x"));
        }
        // Simplification of equality between matching multi-bit constants.
        {
            auto fn = BooleanFunction::from_string("0b101 == 0b101");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Const(1, 1));
        }
        // Simplification of equality between non-matching multi-bit constants.
        {
            auto fn = BooleanFunction::from_string("0xA == 0xB");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Const(0, 1));
        }
    }

    TEST(BooleanFunction, ParserWithVariableSizes) {
        // Without a size map, A == 0xA produces a malformed BF (sizes mismatch).
        // With the map, A is parsed as 4-bit and the comparison is well-formed.
        {
            std::map<std::string, u16> sizes = {{"A", 4}};
            auto fn = BooleanFunction::from_string("A == 0xA", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Eq(BooleanFunction::Var("A", 4), BooleanFunction::Const(10, 4), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Substituting A with a matching constant + simplifying reduces fully.
        {
            std::map<std::string, u16> sizes = {{"mode", 2}};
            auto fn = BooleanFunction::from_string("(mode == 0b10) ? a : b", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto subst = fn.get().substitute("mode", BooleanFunction::Const(2, 2));
            ASSERT_TRUE(subst.is_ok());
            EXPECT_EQ(subst.get().simplify(), BooleanFunction::Var("a"));
        }
        // Variables not in the map default to 1 bit (preserves existing behavior).
        {
            std::map<std::string, u16> sizes = {{"A", 4}};
            auto fn = BooleanFunction::from_string("(A == 0xA) & b", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto eq = BooleanFunction::Eq(BooleanFunction::Var("A", 4), BooleanFunction::Const(10, 4), 1);
            ASSERT_TRUE(eq.is_ok());
            auto expected = eq.get() & BooleanFunction::Var("b");
            EXPECT_EQ(fn.get(), expected);
        }
    }

    TEST(BooleanFunction, ParserMultiBitOps) {
        const std::map<std::string, u16> sizes = {{"A", 4}, {"B", 4}};

        // AND with multi-bit variables
        {
            auto fn = BooleanFunction::from_string("A & B", sizes);
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Var("A", 4) & BooleanFunction::Var("B", 4));
            EXPECT_EQ(fn.get().size(), 4);
        }
        // OR with multi-bit variables
        {
            auto fn = BooleanFunction::from_string("A | B", sizes);
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Var("A", 4) | BooleanFunction::Var("B", 4));
            EXPECT_EQ(fn.get().size(), 4);
        }
        // XOR with multi-bit variables
        {
            auto fn = BooleanFunction::from_string("A ^ B", sizes);
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Var("A", 4) ^ BooleanFunction::Var("B", 4));
            EXPECT_EQ(fn.get().size(), 4);
        }
        // NOT with a multi-bit variable
        {
            auto fn = BooleanFunction::from_string("!A", {{"A", 4}});
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), ~BooleanFunction::Var("A", 4));
            EXPECT_EQ(fn.get().size(), 4);
        }
        // Compound: combination of multi-bit ops
        {
            auto fn = BooleanFunction::from_string("(A & B) ^ !A", sizes);
            ASSERT_TRUE(fn.is_ok());
            const auto a = BooleanFunction::Var("A", 4);
            const auto b = BooleanFunction::Var("B", 4);
            EXPECT_EQ(fn.get(), (a.clone() & b) ^ ~a);
        }
        // AND of two multi-bit constants (no map needed)
        {
            auto fn = BooleanFunction::from_string("0xA & 0xF");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get(), BooleanFunction::Const(0xA, 4) & BooleanFunction::Const(0xF, 4));
        }
    }

    TEST(BooleanFunction, ParserSizeMismatchFailures) {
        // Variable defaults to 1 bit but constant is 4 bit -> Eq operands differ.
        {
            auto fn = BooleanFunction::from_string("A == 0xA");
            EXPECT_TRUE(fn.is_error());
        }
        // 1-bit variable AND'd with 4-bit constant.
        {
            auto fn = BooleanFunction::from_string("A & 0xF");
            EXPECT_TRUE(fn.is_error());
        }
        // A is 4-bit, B defaults to 1-bit; bit-widths don't match in AND.
        {
            std::map<std::string, u16> sizes = {{"A", 4}};
            auto fn = BooleanFunction::from_string("A & B", sizes);
            EXPECT_TRUE(fn.is_error());
        }
        // Octal (3-bit) compared to binary 4-bit constant.
        {
            auto fn = BooleanFunction::from_string("0o7 == 0b1111");
            EXPECT_TRUE(fn.is_error());
        }
        // Mixed sizes inside a ternary: condition is 4-bit, but Ite requires 1-bit cond.
        {
            std::map<std::string, u16> sizes = {{"A", 4}};
            auto fn = BooleanFunction::from_string("A ? a : b", sizes);
            EXPECT_TRUE(fn.is_error());
        }
    }

    TEST(BooleanFunction, ParserArithmeticOps) {
        const std::map<std::string, u16> sizes = {{"a", 4}, {"b", 4}, {"c", 4}};

        // Addition with multi-bit variables.
        {
            auto fn = BooleanFunction::from_string("a + b", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Add(BooleanFunction::Var("a", 4), BooleanFunction::Var("b", 4), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
            EXPECT_EQ(fn.get().size(), 4);
        }
        // Subtraction with multi-bit variables.
        {
            auto fn = BooleanFunction::from_string("a - b", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Sub(BooleanFunction::Var("a", 4), BooleanFunction::Var("b", 4), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Multiplication with multi-bit variables.
        {
            auto fn = BooleanFunction::from_string("a * b", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto expected = BooleanFunction::Mul(BooleanFunction::Var("a", 4), BooleanFunction::Var("b", 4), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Mul has higher precedence than Add: `a + b * c` == `a + (b * c)`.
        {
            auto fn = BooleanFunction::from_string("a + b * c", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto bc = BooleanFunction::Mul(BooleanFunction::Var("b", 4), BooleanFunction::Var("c", 4), 4);
            ASSERT_TRUE(bc.is_ok());
            auto expected = BooleanFunction::Add(BooleanFunction::Var("a", 4), bc.get(), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Mul on the left of Add: `a * b + c` == `(a * b) + c`.
        {
            auto fn = BooleanFunction::from_string("a * b + c", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto ab = BooleanFunction::Mul(BooleanFunction::Var("a", 4), BooleanFunction::Var("b", 4), 4);
            ASSERT_TRUE(ab.is_ok());
            auto expected = BooleanFunction::Add(ab.get(), BooleanFunction::Var("c", 4), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Mul interleaved with Sub on both sides: `a - b * c - d` == `(a - (b * c)) - d`.
        {
            std::map<std::string, u16> sizes_d = {{"a", 4}, {"b", 4}, {"c", 4}, {"d", 4}};
            auto fn = BooleanFunction::from_string("a - b * c - d", sizes_d);
            ASSERT_TRUE(fn.is_ok());
            auto bc = BooleanFunction::Mul(BooleanFunction::Var("b", 4), BooleanFunction::Var("c", 4), 4);
            ASSERT_TRUE(bc.is_ok());
            auto a_bc = BooleanFunction::Sub(BooleanFunction::Var("a", 4), bc.get(), 4);
            ASSERT_TRUE(a_bc.is_ok());
            auto expected = BooleanFunction::Sub(a_bc.get(), BooleanFunction::Var("d", 4), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Mul is left-associative: `a * b * c` == `(a * b) * c`.
        {
            auto fn = BooleanFunction::from_string("a * b * c", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto ab = BooleanFunction::Mul(BooleanFunction::Var("a", 4), BooleanFunction::Var("b", 4), 4);
            ASSERT_TRUE(ab.is_ok());
            auto expected = BooleanFunction::Mul(ab.get(), BooleanFunction::Var("c", 4), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Mul has higher precedence than the bitwise And: `a & b * c` == `a & (b * c)`.
        {
            auto fn = BooleanFunction::from_string("a & b * c", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto bc = BooleanFunction::Mul(BooleanFunction::Var("b", 4), BooleanFunction::Var("c", 4), 4);
            ASSERT_TRUE(bc.is_ok());
            auto expected = BooleanFunction::Var("a", 4) & bc.get();
            EXPECT_EQ(fn.get(), expected);
        }
        // Subtraction is left-associative: `a - b - c` == `(a - b) - c`.
        {
            auto fn = BooleanFunction::from_string("a - b - c", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto ab = BooleanFunction::Sub(BooleanFunction::Var("a", 4), BooleanFunction::Var("b", 4), 4);
            ASSERT_TRUE(ab.is_ok());
            auto expected = BooleanFunction::Sub(ab.get(), BooleanFunction::Var("c", 4), 4);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Addition has higher precedence than equality: `a + b == c` parses
        // as `(a + b) == c` (Eq result is 1-bit, operands share the 4-bit width).
        {
            auto fn = BooleanFunction::from_string("a + b == c", sizes);
            ASSERT_TRUE(fn.is_ok());
            auto ab = BooleanFunction::Add(BooleanFunction::Var("a", 4), BooleanFunction::Var("b", 4), 4);
            ASSERT_TRUE(ab.is_ok());
            auto expected = BooleanFunction::Eq(ab.get(), BooleanFunction::Var("c", 4), 1);
            ASSERT_TRUE(expected.is_ok());
            EXPECT_EQ(fn.get(), expected.get());
        }
        // Constants only — exercises that the parser/validator accept multi-bit constant arithmetic.
        {
            auto fn = BooleanFunction::from_string("0b01 + 0b10");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().size(), 2);
            // simplify reduces to the concrete sum.
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Const(3, 2));
        }
        {
            auto fn = BooleanFunction::from_string("0b11 * 0b10");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Const(6, 2));
        }
        {
            auto fn = BooleanFunction::from_string("0b11 - 0b01");
            ASSERT_TRUE(fn.is_ok());
            EXPECT_EQ(fn.get().simplify(), BooleanFunction::Const(2, 2));
        }
    }

    TEST(BooleanFunction, ParserArithmeticSizeMismatch) {
        // Notes on cross-parser behavior: from_string tries Standard, then Liberty, then
        // LibertyNoSpace. The cases here are picked so that all three parsers fail —
        // Standard by operand-size validation, Liberty by either rejecting the operator
        // ('-') or by ending up with the same mismatched-size structure ('+' => Or,
        // '*' => And).
        {
            // 4-bit + 1-bit default in Standard (Add) and Liberty (Or).
            std::map<std::string, u16> sizes = {{"a", 4}};
            auto fn = BooleanFunction::from_string("a + b", sizes);
            EXPECT_TRUE(fn.is_error());
        }
        {
            // 4-bit - 1-bit default; Liberty has no '-' operator so it cannot tokenize.
            std::map<std::string, u16> sizes = {{"a", 4}};
            auto fn = BooleanFunction::from_string("a - b", sizes);
            EXPECT_TRUE(fn.is_error());
        }
        {
            // 4-bit * 1-bit default in Standard (Mul) and Liberty (And).
            std::map<std::string, u16> sizes = {{"a", 4}};
            auto fn = BooleanFunction::from_string("a * b", sizes);
            EXPECT_TRUE(fn.is_error());
        }
    }

    TEST(BooleanFunction, ParserMalformedTernary) {
        // `?` without matching `:`
        {
            auto fn = BooleanFunction::from_string("a ? b");
            EXPECT_TRUE(fn.is_error());
        }
        // `:` without preceding `?`
        {
            auto fn = BooleanFunction::from_string("a : b");
            EXPECT_TRUE(fn.is_error());
        }
        // `?` in isolation at the start
        {
            auto fn = BooleanFunction::from_string("? a");
            EXPECT_TRUE(fn.is_error());
        }
        // `:` in isolation at the start
        {
            auto fn = BooleanFunction::from_string(": a");
            EXPECT_TRUE(fn.is_error());
        }
        // Wrong order: `:` appears before its `?`
        {
            auto fn = BooleanFunction::from_string("a : b ? c");
            EXPECT_TRUE(fn.is_error());
        }
        // Trailing `?` with nothing after
        {
            auto fn = BooleanFunction::from_string("a ?");
            EXPECT_TRUE(fn.is_error());
        }
        // Missing then-branch: `?:` with no operand between them
        {
            auto fn = BooleanFunction::from_string("a ? : b");
            EXPECT_TRUE(fn.is_error());
        }
    }

    TEST(BooleanFunction, Parameters) {
        const auto a = BooleanFunction::Var("A"),
                   b = BooleanFunction::Var("B"),
                   c = BooleanFunction::Var("C");

        EXPECT_EQ((a.clone() & b.clone()).get_parameters(), std::vector<BooleanFunction>({a.clone(), b.clone()}));
        EXPECT_EQ(((a.clone() & b.clone()) | c.clone()).get_parameters(), std::vector<BooleanFunction>({(a.clone() & b.clone()), c.clone()}));
    }

    TEST(BooleanFunction, ConstantSimplification) {
        const auto _0 = BooleanFunction::Const(0, 1),
                   _1 = BooleanFunction::Const(1, 1),
                   _A = BooleanFunction::Const(0xA, 4),
                    a = BooleanFunction::Var("A"),
                   i1 = BooleanFunction::Index(1, 4),
                   i2 = BooleanFunction::Index(2, 4),
                   i4 = BooleanFunction::Index(4, 4);
                   

        EXPECT_TRUE(_0.has_constant_value(0));
        EXPECT_TRUE(_1.has_constant_value(1));
        EXPECT_FALSE(_0.has_constant_value(1));
        EXPECT_FALSE(_1.has_constant_value(0));

        EXPECT_FALSE(a.is_constant());

        EXPECT_TRUE((~_1.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((~_0.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_0.clone() | _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() | _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_1.clone() | _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_0.clone() & _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() & _1.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_1.clone() & _1.clone()).simplify().has_constant_value(1));    
        EXPECT_TRUE((_0.clone() ^ _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() ^ _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_1.clone() ^ _1.clone()).simplify().has_constant_value(0));

        EXPECT_TRUE((_0.clone() + _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() + _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_1.clone() + _0.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_1.clone() + _1.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() - _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() - _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_1.clone() - _0.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((_1.clone() - _1.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() * _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_0.clone() * _1.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_1.clone() * _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((_1.clone() * _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((BooleanFunction::Const(100, 8) + BooleanFunction::Const(50, 8)).simplify().has_constant_value(150));
        EXPECT_TRUE((BooleanFunction::Const(200, 8) + BooleanFunction::Const(60, 8)).simplify().has_constant_value(4));
        EXPECT_TRUE((BooleanFunction::Const(100, 8) - BooleanFunction::Const(50, 8)).simplify().has_constant_value(50));
        EXPECT_TRUE((BooleanFunction::Const(50, 8) - BooleanFunction::Const(100, 8)).simplify().has_constant_value(206));
        EXPECT_TRUE((BooleanFunction::Const(5, 8) * BooleanFunction::Const(5, 8)).simplify().has_constant_value(25));
        EXPECT_TRUE((BooleanFunction::Const(50, 8) * BooleanFunction::Const(50, 8)).simplify().has_constant_value(196));

        EXPECT_TRUE((a.clone() | _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((a.clone() ^ a.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((a.clone() & _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((a.clone() - a.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((a.clone() * _0.clone()).simplify().has_constant_value(0));

        {
            {
                auto res = BooleanFunction::Slice(_A.clone(), i1.clone(), i1.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Slice(_A.clone(), i2.clone(), i2.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Slice(_A.clone(), i1.clone(), i2.clone(), 2);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Concat(_1.clone(), _0.clone(), 2);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(2));
            }
            {
                auto res = BooleanFunction::Concat(_A.clone(), _0.clone(), 5);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(20));
            }
            {
                auto res = BooleanFunction::Zext(_1.clone(), i4.clone(), 4);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Sext(_0.clone(), i4.clone(), 4);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Sext(_1.clone(), i4.clone(), 4);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(15));
            }
        } 

        {
            {
                auto res = BooleanFunction::Eq(_0.clone(), _0.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Eq(_0.clone(), _1.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Eq(_1.clone(), _0.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Eq(_1.clone(), _1.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }

            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xC, 4), BooleanFunction::Const(0x3, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }

            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xC, 4), BooleanFunction::Const(0x3, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }

            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }

            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }

            {
                auto res = BooleanFunction::Ite(_0.clone(), _1.clone(), _0.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
            }
            {
                auto res = BooleanFunction::Ite(_1.clone(), _1.clone(), _0.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(1));
            }
        }

    }

    TEST(BooleanFunction, SimplificationRules) {
        const auto a = BooleanFunction::Var("A"),
                   b = BooleanFunction::Var("B"),
                   c = BooleanFunction::Var("C"),
                   d = BooleanFunction::Var("D", 16),
                   e = BooleanFunction::Var("E", 16),
                   f = BooleanFunction::Var("F", 16),
                  _0 = BooleanFunction::Const(0, 1),
                  _1 = BooleanFunction::Const(1, 1),
                  i0 = BooleanFunction::Index(0, 1);

        ////////////////////////////////////////////////////////////////////////
        // AND RULES
        ////////////////////////////////////////////////////////////////////////

        // (a & 0)   =>    0
        EXPECT_EQ((a.clone() & _0.clone()).simplify(), _0.clone());
        // (a & 1)   =>    a
        EXPECT_EQ((a.clone() & _1.clone()).simplify(), a.clone());
        // (a & a)   =>    a
        EXPECT_EQ((a.clone() & a.clone()).simplify(), a.clone());
        // (a & ~a)  =>    0
        EXPECT_EQ((a.clone() & ~a.clone()).simplify(), _0.clone());

        // (a & b) & a   =>   a & b
        EXPECT_EQ(((a.clone() & b.clone()) & a.clone()).simplify(), a.clone() & b.clone());
        // (a & b) & b   =>   a & b
        EXPECT_EQ(((a.clone() & b.clone()) & b.clone()).simplify(), a.clone() & b.clone());
        // a & (b & a)   =>   a & b
        EXPECT_EQ((a.clone() & (b.clone() & a.clone())).simplify(), a.clone() & b.clone());
        // b & (b & a)   =>   a & b
        EXPECT_EQ((b.clone() & (b.clone() & a.clone())).simplify(), a.clone() & b.clone());

        // a & (a | b)   =>    a
        EXPECT_EQ((a.clone() & (a.clone() | b.clone())).simplify(), a.clone());
        // b & (a | b)   =>    b
        EXPECT_EQ((b.clone() & (a.clone() | b.clone())).simplify(), b.clone());
        // (a | b) & a   =>    a
        EXPECT_EQ(((a.clone() | b.clone()) & a.clone()).simplify(), a.clone());
        // (a | b) & b   =>    b
        EXPECT_EQ(((a.clone() | b.clone()) & b.clone()).simplify(), b.clone());

        // (~a & b) & a   =>   0
        EXPECT_EQ(((~a.clone() & b.clone()) & a.clone()).simplify(), _0.clone());
        // (a & ~b) & b   =>   0
        EXPECT_EQ(((a.clone() & ~b.clone()) & b.clone()).simplify(), _0.clone());
        // a & (b & ~a)   =>   0
        EXPECT_EQ((a.clone() & (b.clone() & ~a.clone())).simplify(), _0.clone());
        // b & (~b & a)   =>   0
        EXPECT_EQ((b.clone() & (~b.clone() & a.clone())).simplify(), _0.clone());

        // a & (~a | b)   =>    a & b
        EXPECT_EQ((a.clone() & (~a.clone() | b.clone())).simplify(), a.clone() & b.clone());
        // b & (a | ~b)   =>    a & b
        EXPECT_EQ((b.clone() & (a.clone() | ~b.clone())).simplify(), a.clone() & b.clone());
        // (~a | b) & a   =>    a & b
        EXPECT_EQ(((~a.clone() | b.clone()) & a.clone()).simplify(), a.clone() & b.clone());
        // (a | ~b) & b   =>    a & b
        EXPECT_EQ(((a.clone() | ~b.clone()) & b.clone()).simplify(), a.clone() & b.clone());

        ////////////////////////////////////////////////////////////////////////
        // OR RULES
        ////////////////////////////////////////////////////////////////////////

        // (a | 0)   =>    a
        EXPECT_EQ((a.clone() | _0.clone()).simplify(), a.clone());
        // (a | 1)   =>    1
        EXPECT_EQ((a.clone() | _1.clone()).simplify(), _1.clone());
        // (a | a)   =>    a
        EXPECT_EQ((a.clone() | a.clone()).simplify(), a.clone());
        // (a | ~a)  =>    1
        EXPECT_EQ((a.clone() | ~a.clone()).simplify(), _1.clone());

        // a | (a | b)   =>    a | b
        EXPECT_EQ((a.clone() | (a.clone() | b.clone())).simplify(), a.clone() | b.clone());
        // b | (a | b)   =>    a | b
        EXPECT_EQ((b.clone() | (a.clone() | b.clone())).simplify(), a.clone() | b.clone());
        // (a | b) | a   =>    a | b
        EXPECT_EQ(((a.clone() | b.clone()) | a.clone()).simplify(), a.clone() | b.clone());
        // (a | b) | b   =>    a | b
        EXPECT_EQ(((a.clone() | b.clone()) | b.clone()).simplify(), a.clone() | b.clone());

        // (a & b) | a   =>   a
        EXPECT_EQ(((a.clone() & b.clone()) | a.clone()).simplify(), a.clone());
        // (a & b) | b   =>   b
        EXPECT_EQ(((a.clone() & b.clone()) | b.clone()).simplify(), b.clone());
        // a | (b & a)   =>   a
        EXPECT_EQ((a.clone() | (b.clone() & a.clone())).simplify(), a.clone());
        // b | (b & a)   =>   b
        EXPECT_EQ((b.clone() | (b.clone() & a.clone())).simplify(), b.clone());

        // a | (~a | b)   =>   1
        EXPECT_EQ((a.clone() | (~a.clone() | b.clone())).simplify(), _1.clone());
        // b | (a | ~b)   =>   1
        EXPECT_EQ((b.clone() | (a.clone() | ~b.clone())).simplify(), _1.clone());
        // (~a | b) | a   =>   1
        EXPECT_EQ(((~a.clone() | b.clone()) | a.clone()).simplify(), _1.clone());
        // (a | ~b) | b   =>   1
        EXPECT_EQ(((a.clone() | ~b.clone()) | b.clone()).simplify(), _1.clone());

        // (~a & b) | a   =>   a | b
        EXPECT_EQ(((~a.clone() & b.clone()) | a.clone()).simplify(), a.clone() | b.clone());
        // (a & ~b) | b   =>   a | b
        EXPECT_EQ(((a.clone() & ~b.clone()) | b.clone()).simplify(), a.clone() | b.clone());
        // a | (b & ~a)   =>   a | b
        EXPECT_EQ((a.clone() | (b.clone() & ~a.clone())).simplify(), a.clone() | b.clone());
        // b | (~b & a)   =>   a | b
        EXPECT_EQ((b.clone() | (~b.clone() & a.clone())).simplify(), a.clone() | b.clone());

        ////////////////////////////////////////////////////////////////////////
        // NOT RULES
        ////////////////////////////////////////////////////////////////////////

        // ~~a   =>   a
        EXPECT_EQ((~(~a.clone())).simplify(), a.clone());
        // ~(~a | ~b)   =>   a & b
        EXPECT_EQ((~(~a.clone() | ~b.clone())).simplify(), a.clone() & b.clone());
        // ~(~a & ~b)   =>   a | b
        EXPECT_EQ((~(~a.clone() & ~b.clone())).simplify(), a.clone() | b.clone());

        ////////////////////////////////////////////////////////////////////////
        // XOR RULES
        ////////////////////////////////////////////////////////////////////////

        // (a ^ 0)   =>    a
        EXPECT_EQ((a.clone() ^ _0.clone()).simplify(), a.clone());
        // (a ^ 1)   =>    ~a
        EXPECT_EQ((a.clone() ^ _1.clone()).simplify(), ~a.clone());
        // (a ^ a)   =>    0
        EXPECT_EQ((a.clone() ^ a.clone()).simplify(), _0.clone());
        // (a ^ ~a)  =>    1
        EXPECT_EQ((a.clone() ^ ~a.clone()).simplify(), _1.clone());

        ////////////////////////////////////////////////////////////////////////
        // ADD RULES
        ////////////////////////////////////////////////////////////////////////

        // (a + 0)   =>    a
        EXPECT_EQ((a.clone() + _0.clone()).simplify(), a.clone());

        ////////////////////////////////////////////////////////////////////////
        // SUB RULES
        ////////////////////////////////////////////////////////////////////////

        // (a - 0)   =>    a
        EXPECT_EQ((a.clone() - _0.clone()).simplify(), a.clone());
        // (a - a)   =>    0
        EXPECT_EQ((a.clone() - a.clone()).simplify(), _0.clone());

        ////////////////////////////////////////////////////////////////////////
        // MUL RULES
        ////////////////////////////////////////////////////////////////////////

        // (a * 0)   =>    0
        EXPECT_EQ((a.clone() * _0.clone()).simplify(), _0.clone());
        // (a * 1)   =>    a
        EXPECT_EQ((a.clone() * _1.clone()).simplify(), a.clone());

        ////////////////////////////////////////////////////////////////////////
        // SDIV RULES
        ////////////////////////////////////////////////////////////////////////

        // (a /s 1)   =>    a
        {
            auto res = BooleanFunction::Sdiv(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), a.clone());
        }
        // (a /s a)   =>    1
        {
            auto res = BooleanFunction::Sdiv(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _1.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // UDIV RULES
        ////////////////////////////////////////////////////////////////////////

        // (a /s 1)   =>    a
        {
            auto res = BooleanFunction::Udiv(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), a.clone());
        }
        // (a /s a)   =>    1
        {
            auto res = BooleanFunction::Udiv(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _1.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // SREM RULES
        ////////////////////////////////////////////////////////////////////////

        // (a %s 1)   =>    0
        {
            auto res = BooleanFunction::Srem(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _0.clone());
        }
        // (a %s a)   =>    0
        {
            auto res = BooleanFunction::Srem(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _0.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // UREM RULES
        ////////////////////////////////////////////////////////////////////////

        // (a % 1)   =>    0
        {
            auto res = BooleanFunction::Urem(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _0.clone());
        }
        // (a % a)   =>    0
        {
            auto res = BooleanFunction::Urem(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _0.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // SLICE RULES
        ////////////////////////////////////////////////////////////////////////

        // SLICE(p, 0, 0)   =>   p (if p is 1-bit wide)
        {
            auto res = BooleanFunction::Slice(a.clone(), i0.clone(), i0.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), a.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // EQUALITY RULES
        ////////////////////////////////////////////////////////////////////////

        // X == X   =>   1
        {
            auto res = BooleanFunction::Eq(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _1.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // SIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X <=s X   =>   1
        {
            auto res = BooleanFunction::Sle(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _1.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // SIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X <s X   =>   0
        {
            auto res = BooleanFunction::Slt(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _0.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // UNSIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X <= X   =>   1
        {
            auto res = BooleanFunction::Ule(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _1.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // UNSIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X < 0   =>   0
        {
            auto res = BooleanFunction::Ult(a.clone(), _0.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _0.clone());
        }
        // X < X   =>   0
        {
            auto res = BooleanFunction::Ult(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), _0.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // IF-THEN-ELSE RULES
        ////////////////////////////////////////////////////////////////////////

        // ITE(0, a, b)  =>  b
        {
            auto res = BooleanFunction::Ite(_0.clone(), a.clone(), b.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), b.clone());
        }
        // ITE(1, a, b)  =>  a
        {
            auto res = BooleanFunction::Ite(_1.clone(), a.clone(), b.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), a.clone());
        }
        // ITE(a, b, b)  =>  b
        {
            auto res = BooleanFunction::Ite(a.clone(), b.clone(), b.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get().simplify(), b.clone());
        }

        ////////////////////////////////////////////////////////////////////////
        // CONCAT RULES
        ////////////////////////////////////////////////////////////////////////

        // CONCAT(SLICE(X, j+1, k), SLICE(X, i, j)) => SLICE(X, i, k)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(14, d.size()), 7);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(14, d.size()), 15);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), s1.get(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), s3.get());
        }

        // CONCAT(SLICE(X, j, j), SLICE(X, i, j)) => SEXT(SLICE(X, i, j), j-i+1)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(7, d.size()), BooleanFunction::Index(7, d.size()), 1);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto sext1 = BooleanFunction::Sext(s1.get().clone(), BooleanFunction::Index(s1.get().size()+1, s1.get().size()+1), s1.get().size()+1);

            ASSERT_TRUE(sext1.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), s1.get(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), sext1.get());
        }

        // CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), Y)) => CONCAT(CONCAT(SLICE(X, 0, 7), SLICE(X, 8, 15)), Y))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto c1 = BooleanFunction::Concat(s2.get().clone(), a.clone(), s2.get().size() + a.size());
            auto c2 = BooleanFunction::Concat(s1.get().clone(), s2.get().clone(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto c3 = BooleanFunction::Concat(c2.get().clone(), a.clone(), c2.get().size() + a.size());

            auto res = BooleanFunction::Concat(s1.get().clone(), c1.get().clone(), s1.get().size() + c1.get().size());

            ASSERT_TRUE(c3.is_ok());
            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), c3.get());
        }

        // CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), SLICE(Z, 8, 15))) => CONCAT(CONCAT(SLICE(X, 0, 7), SLICE(X, 8, 15)), SLICE(Z, 8, 15)))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);
            auto s3 = BooleanFunction::Slice(e.clone(), BooleanFunction::Index(8, e.size()), BooleanFunction::Index(15, e.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto c1 = BooleanFunction::Concat(s2.get().clone(), s3.get().clone(), s2.get().size() + s3.get().size());
            auto c2 = BooleanFunction::Concat(s1.get().clone(), s2.get().clone(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto c3 = BooleanFunction::Concat(c2.get().clone(), s3.get().clone(), c2.get().size() + s3.get().size());

            auto res = BooleanFunction::Concat(s1.get().clone(), c1.get().clone(), s1.get().size() + c1.get().size());

            ASSERT_TRUE(c3.is_ok());
            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), c3.get());
        }

        // CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), SLICE(X, 8, 15))) => CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), SLICE(X, 8, 15)))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto c1 = BooleanFunction::Concat(s2.get().clone(), s3.get().clone(), s2.get().size() + s3.get().size());

            ASSERT_TRUE(c1.is_ok());

            auto res = BooleanFunction::Concat(s1.get().clone(), c1.get().clone(), s1.get().size() + c1.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), res.get());
        }

        // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), CONCAT(SLICE(Y, m, n), Z))) => CONCAT(CONCAT(SLICE(X, i, j), SLICE(X, k, l)), CONCAT(SLICE(Y, m, n), Z)))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);
            auto s3 = BooleanFunction::Slice(e.clone(), BooleanFunction::Index(8, e.size()), BooleanFunction::Index(15, e.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto c1 = BooleanFunction::Concat(s3.get().clone(), f.clone(), s3.get().size() + f.size());
            auto c2 = BooleanFunction::Concat(s2.get().clone(), c1.get().clone(), s2.get().size() + c1.get().size());

            auto c3 = BooleanFunction::Concat(s1.get().clone(), s2.get().clone(), s1.get().size() + s2.get().size());
            auto c4 = BooleanFunction::Concat(c3.get().clone(), c1.get().clone(), c3.get().size() + c1.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());
            ASSERT_TRUE(c3.is_ok());
            ASSERT_TRUE(c4.is_ok());

            auto res = BooleanFunction::Concat(s1.get().clone(), c2.get().clone(), s1.get().size() + c2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), c4.get());
        }

        // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), CONCAT(SLICE(X, m, n), Z))) => CONCAT(SLICE(X, i, j), CONCAT(CONCAT(SLICE(X, k, l), SLICE(X, m, n)), Z))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(4, d.size()), BooleanFunction::Index(11, d.size()), 8);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto c1 = BooleanFunction::Concat(s3.get().clone(), e.clone(), s3.get().size() + e.size());
            auto c2 = BooleanFunction::Concat(s2.get().clone(), c1.get().clone(), s2.get().size() + c1.get().size());

            auto c3 = BooleanFunction::Concat(s2.get().clone(), s3.get().clone(), s2.get().size() + s3.get().size());
            auto c4 = BooleanFunction::Concat(c3.get().clone(), e.clone(), c3.get().size() + e.size());
            auto c5 = BooleanFunction::Concat(s1.get().clone(), c4.get().clone(), s1.get().size() + c4.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());
            ASSERT_TRUE(c3.is_ok());
            ASSERT_TRUE(c4.is_ok());
            ASSERT_TRUE(c5.is_ok());

            auto res = BooleanFunction::Concat(s1.get().clone(), c2.get().clone(), s1.get().size() + c2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), c5.get());
        }

        // CONCAT(SLICE(X, 8, 15), CONCAT(SLICE(X, 0, 7), SLICE(Y, 0, 7))) => CONCAT(SLICE(X, 15, 0), SLICE(Y, 0, 7))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(14, d.size()), 7);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(14, d.size()), 15);
            auto s4 = BooleanFunction::Slice(e.clone(), BooleanFunction::Index(0, e.size()), BooleanFunction::Index(7, e.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());
            ASSERT_TRUE(s4.is_ok());

            auto c1 = BooleanFunction::Concat(s1.get().clone(), s4.get().clone(), s1.get().size() + s4.get().size());
            auto c2 = BooleanFunction::Concat(s3.get().clone(), s4.get().clone(), s3.get().size() + s4.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto res = BooleanFunction::Concat(s2.get().clone(), c1.get().clone(), s2.get().size() + c1.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), c2.get());
        }

        // CONCAT(SLICE(X, j, j), SEXT(SLICE(X, i, j), j-i+1)) => SEXT(SLICE(X, i, j), j-i+2)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(7, d.size()), BooleanFunction::Index(7, d.size()), 1);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto sext1 = BooleanFunction::Sext(s1.get().clone(), BooleanFunction::Index(s1.get().size()+1, s1.get().size()+1), s1.get().size()+1);
            auto sext2 = BooleanFunction::Sext(s1.get().clone(), BooleanFunction::Index(s1.get().size()+2, s1.get().size()+2), s1.get().size()+2);

            ASSERT_TRUE(sext1.is_ok());
            ASSERT_TRUE(sext2.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), sext1.get(), sext1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), sext2.get());
        }

        // CONCAT(SLICE(X, 7, 7), CONCAT(SEXT(SLICE(X, 0, 7), 9), Y)) => CONCAT(SEXT(SLICE(X, 0, 7), 10), Y)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(7, d.size()), BooleanFunction::Index(7, d.size()), 1);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto sext1 = BooleanFunction::Sext(s1.get().clone(), BooleanFunction::Index(s1.get().size()+1, s1.get().size()+1), s1.get().size()+1);
            auto sext2 = BooleanFunction::Sext(s1.get().clone(), BooleanFunction::Index(s1.get().size()+2, s1.get().size()+2), s1.get().size()+2);

            ASSERT_TRUE(sext1.is_ok());
            ASSERT_TRUE(sext2.is_ok());

            auto c1 = BooleanFunction::Concat(sext1.get().clone(), e.clone(), sext1.get().size() + e.size());
            auto c2 = BooleanFunction::Concat(sext2.get().clone(), e.clone(), sext2.get().size() + e.size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), c1.get(), c1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(res.get().simplify(), c2.get());
        }

        ////////////////////////////////////////////////////////////////////////
        // GENERAL SIMPLIFICATION RULES
        ////////////////////////////////////////////////////////////////////////

        // (a & ~a) | (b & ~b)  =>   0
        EXPECT_EQ(((a.clone() & ~a.clone()) | (b.clone() & ~b.clone())).simplify(), _0.clone());
        // (a & b) | (~a & b)   =>   b
        EXPECT_EQ(((a.clone() & b.clone()) | (~a.clone() & b.clone())).simplify(), b.clone());
        // (a & ~b) | (~a & ~b)  =>  ~b
        EXPECT_EQ(((a.clone() & ~b.clone()) | (~a.clone() & ~b.clone())).simplify(), ~b.clone());

        // (a & b) | (~a & b) | (a & ~b) | (~a & ~b)   =>   1
        EXPECT_EQ(((a.clone() & b.clone()) | (~a.clone() & b.clone()) | (a.clone() & ~b.clone()) | (~a.clone() & ~b.clone())).simplify(), _1.clone());
        // (a | b) | (b & c)   => a | b
        EXPECT_EQ(((a.clone() | b.clone()) | (b.clone() & c.clone())).simplify(), a.clone() | b.clone());    
        // (a & c) | (b & ~c) | (a & b)   =>   (b | c) & (a | ~c)
        EXPECT_EQ(((a.clone() & c.clone()) | (b.clone() & ~c.clone()) | (a.clone() & b.clone())).simplify(), (b.clone() | c.clone()) & (a.clone() | ~c.clone()));
    }

    TEST(BooleanFunction, SimplificationPerformance)
    {
        const auto start = std::chrono::system_clock::now();

        const auto function = BooleanFunction::from_string("((((((((((((((((((((((((((((((((0b0 | (((((I0 & (! I1)) & (! I2)) & (! I3)) & (! I4)) & (! I5))) | ((((((! I0) & I1) & (! I2)) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & I1) & (! I2)) & (! I3)) & (! I4)) & (! I5))) | ((((((! I0) & (! I1)) & I2) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & I1) & I2) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & (! I1)) & (! I2)) & I3) & (! I4)) & (! I5))) | (((((I0 & I1) & (! I2)) & I3) & (! I4)) & (! I5))) | ((((((! I0) & (! I1)) & I2) & I3) & (! I4)) & (! I5))) | (((((I0 & I1) & I2) & I3) & (! I4)) & (! I5))) | ((((((! I0) & (! I1)) & (! I2)) & (! I3)) & I4) & (! I5))) | (((((I0 & (! I1)) & (! I2)) & (! I3)) & I4) & (! I5))) | ((((((! I0) & (! I1)) & (! I2)) & I3) & I4) & (! I5))) | (((((I0 & (! I1)) & (! I2)) & I3) & I4) & (! I5))) | (((((I0 & I1) & (! I2)) & I3) & I4) & (! I5))) | ((((((! I0) & I1) & I2) & I3) & I4) & (! I5))) | ((((((! I0) & I1) & (! I2)) & (! I3)) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & I2) & (! I3)) & (! I4)) & I5)) | ((((((! I0) & I1) & I2) & (! I3)) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & (! I2)) & I3) & (! I4)) & I5)) | (((((I0 & (! I1)) & (! I2)) & I3) & (! I4)) & I5)) | (((((I0 & I1) & (! I2)) & I3) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & I2) & I3) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & (! I2)) & (! I3)) & I4) & I5)) | (((((I0 & (! I1)) & (! I2)) & (! I3)) & I4) & I5)) | ((((((! I0) & I1) & (! I2)) & (! I3)) & I4) & I5)) | ((((((! I0) & I1) & I2) & (! I3)) & I4) & I5)) | (((((I0 & I1) & I2) & (! I3)) & I4) & I5)) | (((((I0 & (! I1)) & (! I2)) & I3) & I4) & I5)) | ((((((! I0) & I1) & (! I2)) & I3) & I4) & I5)) | ((((((! I0) & (! I1)) & I2) & I3) & I4) & I5)) | (((((I0 & (! I1)) & I2) & I3) & I4) & I5)) | (((((I0 & I1) & I2) & I3) & I4) & I5))").get();
        const auto simplified = function.simplify();

        const auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
    }

    TEST(BooleanFunction, Substitution) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                    c = BooleanFunction::Var("C"),
                    d = BooleanFunction::Var("D"),
                   _0 = BooleanFunction::Const(0, 1);

        EXPECT_EQ((a & b & c).substitute("C", "D"), a & b & d);

        EXPECT_EQ((a & b).substitute("B", _0).get(), a & _0);
        EXPECT_EQ((a & b).substitute("B", ~c).get(), a & ~c);
        EXPECT_EQ((a & b).substitute("B", ~c).get(), a & ~c);
        EXPECT_EQ((a & b).substitute("B", b | c | d).get(),  a & (b | c | d));



        EXPECT_EQ((a & b).substitute({{"A", c}, {"B", d}}).get(), c & d);
    }

    TEST(BooleanFunction, EvaluateSingleBit) {
        const auto a = BooleanFunction::Var("A"),
                   b = BooleanFunction::Var("B"),
                  _0 = BooleanFunction::Const(0, 1),
                  _1 = BooleanFunction::Const(1, 1);

        using Value = BooleanFunction::Value;

        const std::vector<std::tuple<BooleanFunction, std::unordered_map<std::string, Value>, Value>> data = {
            {a, {{"A", Value::ZERO}}, Value::ZERO},
            {a, {{"A", Value::ONE}}, Value::ONE},

            {~a, {{"A", Value::ZERO}}, Value::ONE},
            {~a, {{"A", Value::ONE}}, Value::ZERO},
            
            {a & b, {{"A", Value::ZERO}, {"B", Value::ZERO}}, Value::ZERO},
            {a & b, {{"A", Value::ONE}, {"B", Value::ZERO}}, Value::ZERO},
            {a & b, {{"A", Value::ONE}, {"B", Value::ONE}}, Value::ONE},

            {a | b, {{"A", Value::ZERO}, {"B", Value::ZERO}}, Value::ZERO},
            {a | b, {{"A", Value::ONE}, {"B", Value::ZERO}}, Value::ONE},
            {a | b, {{"A", Value::ONE}, {"B", Value::ONE}}, Value::ONE},

            {a ^ b, {{"A", Value::ZERO}, {"B", Value::ZERO}}, Value::ZERO},
            {a ^ b, {{"A", Value::ONE}, {"B", Value::ZERO}}, Value::ONE},
            {a ^ b, {{"A", Value::ONE}, {"B", Value::ONE}}, Value::ZERO},
        };
        
        for (const auto& [function, input, expected]: data) {
            auto value = function.evaluate(input);
            EXPECT_EQ(expected, value.get());
        }
    }

    TEST(BooleanFunction, EvaluateMultiBit) {
        const auto a = BooleanFunction::Var("A", 2),
                   b = BooleanFunction::Var("B", 2),
                  _0 = BooleanFunction::Const(0, 2),
                  _1 = BooleanFunction::Const(1, 2);

        using Value = BooleanFunction::Value;

        const std::vector<std::tuple<BooleanFunction, std::unordered_map<std::string, std::vector<Value>>, std::vector<Value>>> data = {
            {a, {{"A", {Value::ZERO, Value::ZERO}}}, {Value::ZERO, Value::ZERO}},
            {a, {{"A", {Value::ONE, Value::ZERO}}}, {Value::ONE, Value::ZERO}},
            {a, {{"A", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ONE}},

            {~a, {{"A", {Value::ZERO, Value::ZERO}}}, {Value::ONE, Value::ONE}},
            {~a, {{"A", {Value::ONE, Value::ZERO}}}, {Value::ZERO, Value::ONE}},
            {~a, {{"A", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ZERO}},
         
            {a & b, {{"A", {Value::ZERO, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ZERO}},
            {a & b, {{"A", {Value::ONE, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ZERO}},
            {a & b, {{"A", {Value::ZERO, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ONE}},
            {a & b, {{"A", {Value::ONE, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ONE}},

            {a | b, {{"A", {Value::ZERO, Value::ZERO}}, {"B", {Value::ZERO, Value::ZERO}}}, {Value::ZERO, Value::ZERO}},
            {a | b, {{"A", {Value::ONE, Value::ZERO}}, {"B", {Value::ONE, Value::ZERO}}}, {Value::ONE, Value::ZERO}},
            {a | b, {{"A", {Value::ZERO, Value::ONE}}, {"B", {Value::ZERO, Value::ONE}}}, {Value::ZERO, Value::ONE}},
            {a | b, {{"A", {Value::ZERO, Value::ONE}}, {"B", {Value::ONE, Value::ZERO}}}, {Value::ONE, Value::ONE}},

            {a ^ b, {{"A", {Value::ZERO, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ONE}},
            {a ^ b, {{"A", {Value::ONE, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ONE}},
            {a ^ b, {{"A", {Value::ZERO, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ZERO}},
            {a ^ b, {{"A", {Value::ONE, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ZERO}},

            {a + b, {{"A", {Value::ZERO, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ONE}},
            {a + b, {{"A", {Value::ONE, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ZERO}},
            {a + b, {{"A", {Value::ZERO, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ZERO}},
            {a + b, {{"A", {Value::ONE, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ONE}},

            {a - b, {{"A", {Value::ZERO, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ZERO}},
            {a - b, {{"A", {Value::ONE, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ONE}},
            {a - b, {{"A", {Value::ZERO, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ONE}},
            {a - b, {{"A", {Value::ONE, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ZERO}},

            {a * b, {{"A", {Value::ZERO, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ZERO}},
            {a * b, {{"A", {Value::ONE, Value::ZERO}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ONE}},
            {a * b, {{"A", {Value::ZERO, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ZERO, Value::ONE}},
            {a * b, {{"A", {Value::ONE, Value::ONE}}, {"B", {Value::ONE, Value::ONE}}}, {Value::ONE, Value::ZERO}},
        };
        
        for (const auto& [function, input, expected]: data) {
            auto value = function.evaluate(input);
            EXPECT_EQ(expected, value.get());
        }
    }

    TEST(BooleanFunction, TruthTable) {
        const auto a = BooleanFunction::Var("A"),
                   b = BooleanFunction::Var("B"),
                   c = BooleanFunction::Var("C");

        using Value = BooleanFunction::Value;

        const std::vector<std::tuple<BooleanFunction, std::vector<std::vector<Value>>, std::vector<std::string>>> data = {
            {a.clone() & b.clone(), std::vector<std::vector<Value>>({
                {Value::ZERO, Value::ZERO, Value::ZERO, Value::ONE}
            }), {}},
            {a.clone() | b.clone(), std::vector<std::vector<Value>>({
                {Value::ZERO, Value::ONE, Value::ONE, Value::ONE}
            }), {}},
            {a.clone() ^ b.clone(), std::vector<std::vector<Value>>({
                {Value::ZERO, Value::ONE, Value::ONE, Value::ZERO}
            }), {}},
            {~((a & b) | c), std::vector<std::vector<Value>>({
                {Value::ONE, Value::ONE, Value::ONE, Value::ZERO, Value::ZERO, Value::ZERO, Value::ZERO, Value::ZERO}
            }), {}},
            {~((a & b) | c), std::vector<std::vector<Value>>({
                {Value::ONE, Value::ZERO, Value::ONE, Value::ZERO, Value::ONE, Value::ZERO, Value::ZERO, Value::ZERO}
            }), {"C", "B", "A"}},
        };

        for (const auto& [function, expected, variable_order] : data) {
            ASSERT_EQ(expected, function.compute_truth_table(variable_order).get());
        }
    }

    TEST(BooleanFunction, SimplificationVsTruthTable) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                    c = BooleanFunction::Var("C");
        
        const std::vector<BooleanFunction> data = {
            (a & b & c),
            (a | b | c),
        };

        for (const auto& function: data) {
            ASSERT_EQ(function.compute_truth_table().get(), function.simplify().compute_truth_table().get());
        }
    }

    TEST(BooleanFunction, QueryConfig) {
        {
            const auto config = SMT::QueryConfig()
                .with_solver(SMT::SolverType::Z3)
                .with_local_solver()
                .with_model_generation()
                .with_timeout(42);

            EXPECT_EQ(config.solver, SMT::SolverType::Z3);
            EXPECT_EQ(config.local, true);
            EXPECT_EQ(config.generate_model, true);
            EXPECT_EQ(config.timeout_in_seconds, 42);
        }
        {
            const auto config = SMT::QueryConfig()
                .with_solver(SMT::SolverType::Boolector)
                .with_call(SMT::SolverCall::Library)
                .with_remote_solver()
                .without_model_generation();

            EXPECT_EQ(config.solver, SMT::SolverType::Boolector);
            EXPECT_EQ(config.call, SMT::SolverCall::Library);
            EXPECT_EQ(config.local, false);
            EXPECT_EQ(config.generate_model, false);
        }
    }

    TEST(BooleanFunction, SatisfiableConstraint) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                    c = BooleanFunction::Var("C", 4),
                    d = BooleanFunction::Var("D", 4),
                   _0 = BooleanFunction::Const(0, 1),
                   _1 = BooleanFunction::Const(1, 1);

        auto formulas = std::vector<std::vector<SMT::Constraint>>({
            {SMT::Constraint(a.clone() & b.clone(), _1.clone())},
            {
                SMT::Constraint(a.clone() | b.clone(), _1.clone()),
                SMT::Constraint(a.clone(), _1.clone()),
                SMT::Constraint(b.clone(), _0.clone()),
            },
            {
                SMT::Constraint(a.clone() & b.clone(), _1.clone()),
                SMT::Constraint(a.clone(), _1.clone()),
                SMT::Constraint(b.clone(), _1.clone()),
            },
            {
                SMT::Constraint((a.clone() & ~b.clone()) | (~a.clone() & b.clone()), _1.clone()),
                SMT::Constraint(a.clone(), _1.clone()),
            },
            {
                SMT::Constraint(BooleanFunction::Add(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(5, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Sub(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(4, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(6, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Mul(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(4, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(2, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Sdiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(2, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(4, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Sdiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(14, 4)),    // 14 = -2
                SMT::Constraint(c.clone(), BooleanFunction::Const(4, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Udiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(4, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(8, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Srem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(3, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(7, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Srem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(13, 4)),    // 13 = -3
                SMT::Constraint(c.clone(), BooleanFunction::Const(9, 4)),                                                // 9 = -7
            },
            {
                SMT::Constraint(BooleanFunction::Urem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(3, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(7, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Shl(c.clone(), BooleanFunction::Index(1, 4), 4).get(), BooleanFunction::Const(4, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Lshr(c.clone(), BooleanFunction::Index(1, 4), 4).get(), BooleanFunction::Const(2, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Ashr(c.clone(), BooleanFunction::Index(1, 4), 4).get(), BooleanFunction::Const(0xC, 4)),
            },
        });

        for (auto&& constraints : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));

            for (auto&& [solver_type, solver_call] : std::vector<std::pair<SMT::SolverType, SMT::SolverCall>>{{SMT::SolverType::Z3, SMT::SolverCall::Binary}}) {
                if (!SMT::Solver::has_local_solver_for(solver_type, solver_call)) {
                    continue;
                }

                auto result = solver.query(
                    SMT::QueryConfig()
                        .with_solver(solver_type)
                        .with_local_solver()
                        .with_model_generation()
                        .with_timeout(1000)
                );

                ASSERT_TRUE(result.is_ok());
                auto solver_result = result.get();
                EXPECT_EQ(solver_result.type, SMT::SolverResultType::Sat);
                EXPECT_TRUE(solver_result.model.has_value());
            }
        }
    }

    TEST(BooleanFunction, UnSatisfiableConstraint) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                    c = BooleanFunction::Var("C", 4),
                    d = BooleanFunction::Var("D", 4),
                   _0 = BooleanFunction::Const(0, 1),
                   _1 = BooleanFunction::Const(1, 1);

        auto formulas = std::vector<std::vector<SMT::Constraint>>({
            {
                SMT::Constraint(a.clone(), b.clone()),
                SMT::Constraint(a.clone(), _1.clone()),
                SMT::Constraint(b.clone(), _0.clone()),
            },
            {
                SMT::Constraint(a.clone() | b.clone(), _1.clone()),
                SMT::Constraint(a.clone(), _0.clone()),
                SMT::Constraint(b.clone(), _0.clone()),
            },
            {
                SMT::Constraint(a.clone() & b.clone(), _1.clone()),
                SMT::Constraint(a.clone(), _0.clone()),
                SMT::Constraint(b.clone(), _1.clone()),
            },
            {
                SMT::Constraint(a.clone() & b.clone(), _1.clone()),
                SMT::Constraint(a.clone(), _1.clone()),
                SMT::Constraint(b.clone(), _0.clone()),
            },
            {
                SMT::Constraint((a.clone() & ~b.clone()) | (~a.clone() & b.clone()), _1.clone()),
                SMT::Constraint(a.clone(), _1.clone()),
                SMT::Constraint(b.clone(), _1.clone()),
            },
            {
                SMT::Constraint(BooleanFunction::Add(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(0, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Sub(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(0, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Mul(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(5, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Sdiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(5, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Udiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(5, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Srem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(5, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Urem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(5, 4)),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Ult(c.clone(), d.clone(), 1).get()),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Ult(c.clone(), d.clone(), 1).get()),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Ule(c.clone(), d.clone(), 1).get()),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Slt(c.clone(), d.clone(), 1).get()),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Sle(c.clone(), d.clone(), 1).get()),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Eq(c.clone(), d.clone(), 1).get()),
                SMT::Constraint(c.clone(), BooleanFunction::Const(1, 4)),
                SMT::Constraint(d.clone(), BooleanFunction::Const(0, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Shl(c.clone(), BooleanFunction::Index(1, 4), 4).get(), BooleanFunction::Const(1, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Lshr(c.clone(), BooleanFunction::Index(1, 4), 4).get(), BooleanFunction::Const(8, 4)),
            },
            {
                SMT::Constraint(BooleanFunction::Ashr(c.clone(), BooleanFunction::Index(1, 4), 4).get(), BooleanFunction::Const(4, 4)),
            }
        });

        for (auto&& constraints : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));
            for (auto&& [solver_type, solver_call] : std::vector<std::pair<SMT::SolverType, SMT::SolverCall>>{{SMT::SolverType::Z3, SMT::SolverCall::Binary}}) {
                if (!SMT::Solver::has_local_solver_for(solver_type, solver_call)) {
                    continue;
                }

                auto result = solver.query(
                    SMT::QueryConfig()
                        .with_solver(solver_type)
                        .with_local_solver()
                        .with_model_generation()
                        .with_timeout(1000)
                );

                ASSERT_TRUE(result.is_ok());
                auto solver_result = result.get();
                EXPECT_EQ(solver_result.type, SMT::SolverResultType::UnSat);
                EXPECT_FALSE(solver_result.model.has_value());
            }
        }
    }

    TEST(BooleanFunction, FunctionConstraint) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                   _0 = BooleanFunction::Const(0, 1),
                   _1 = BooleanFunction::Const(1, 1);

        auto formulas = std::vector<std::tuple<std::vector<SMT::Constraint>, SMT::Model>>({
            {
                {
                    SMT::Constraint(BooleanFunction::Eq(a.clone(), b.clone(), 1).get()), 
                    SMT::Constraint(a.clone(), _1.clone())
                },
                SMT::Model({{"A", {1, 1}}, {"B", {1, 1}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Eq(a.clone(), b.clone(), 1).get()), 
                    SMT::Constraint(a.clone(), _0.clone())
                },
                SMT::Model({{"A", {0, 1}}, {"B", {0, 1}}})
            },
        });

        for (auto&& [constraints, model] : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));

            for (auto&& [solver_type, solver_call] : std::vector<std::pair<SMT::SolverType, SMT::SolverCall>>{{SMT::SolverType::Z3, SMT::SolverCall::Binary}}) {
                if (!SMT::Solver::has_local_solver_for(solver_type, solver_call)) {
                    continue;
                }

                auto result = solver.query(
                    SMT::QueryConfig()
                        .with_solver(solver_type)
                        .with_local_solver()
                        .with_model_generation()
                        .with_timeout(1000)
                );

                ASSERT_TRUE(result.is_ok());
                auto solver_result = result.get();
                EXPECT_EQ(solver_result.type, SMT::SolverResultType::Sat);
                EXPECT_EQ(*solver_result.model, model);
            }
        }
    }

    TEST(BooleanFunction, Model) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                    c = BooleanFunction::Var("C", 4),
                    d = BooleanFunction::Var("D", 4),
                   _0 = BooleanFunction::Const(0, 1),
                   _1 = BooleanFunction::Const(1, 1);

        auto formulas = std::vector<std::tuple<std::vector<SMT::Constraint>, SMT::Model>>({
            {
                {
                    SMT::Constraint(a.clone() & b.clone(), _1.clone())
                },
                SMT::Model({{"A", {1, 1}}, {"B", {1, 1}}})
            },
            {
                {
                    SMT::Constraint(a.clone() | b.clone(), _1.clone()),
                    SMT::Constraint(b.clone(), _0.clone()),
                },
                SMT::Model({{"A", {1, 1}}, {"B", {0, 1}}})
            },
            {
                {
                    SMT::Constraint(a.clone() & b.clone(), _1.clone()),
                    SMT::Constraint(a.clone(), _1.clone()),
                },
                SMT::Model({{"A", {1, 1}}, {"B", {1, 1}}})
            },
            {
                {
                    SMT::Constraint((~a.clone() & b.clone()) | (a.clone() & ~b.clone()), _1.clone()),
                    SMT::Constraint(a.clone(), _1.clone()),
                },
                SMT::Model({{"A", {1, 1}}, {"B", {0, 1}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Add(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(5, 4)),
                    SMT::Constraint(c.clone(), BooleanFunction::Const(0, 4)),
                },
                SMT::Model({{"C", {0, 4}}, {"D", {5, 4}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Sub(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(4, 4)),
                    SMT::Constraint(c.clone(), BooleanFunction::Const(6, 4)),
                },
                SMT::Model({{"C", {6, 4}}, {"D", {2, 4}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Mul(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(4, 4)),
                    SMT::Constraint(c.clone(), BooleanFunction::Const(2, 4)),
                },
                SMT::Model({{"C", {2, 4}}, {"D", {2, 4}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Sdiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(2, 4)),
                    SMT::Constraint(c.clone(), BooleanFunction::Const(4, 4)),
                },
                SMT::Model({{"C", {4, 4}}, {"D", {2, 4}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Sdiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(14, 4)), // 14 = -2
                    SMT::Constraint(c.clone(), BooleanFunction::Const(4, 4)),
                },
                SMT::Model({{"C", {4, 4}}, {"D", {14, 4}}}) // 14 = -2
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Udiv(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(2, 4)),
                    SMT::Constraint(c.clone(), BooleanFunction::Const(4, 4)),
                },
                SMT::Model({{"C", {4, 4}}, {"D", {2, 4}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Srem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(3, 4)),
                    SMT::Constraint(c.clone(), BooleanFunction::Const(7, 4)),
                    SMT::Constraint(BooleanFunction::Slt(BooleanFunction::Const(0, 4), d.clone(), 1).get()), // d could be 4 and -4, thats why we bind it to be larger than 0
                },
                SMT::Model({{"C", {7, 4}}, {"D", {4, 4}}})
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Srem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(13, 4)), // 13 = -3
                    SMT::Constraint(c.clone(), BooleanFunction::Const(9, 4)), // 9 = -7
                    SMT::Constraint(BooleanFunction::Slt(BooleanFunction::Const(0, 4), d.clone(), 1).get()), // d could be 4 and -4, thats why we bind it to be larger than 0
                },
                SMT::Model({{"C", {9, 4}}, {"D", {4, 4}}}) // 9 = -7
            },
            {
                {
                    SMT::Constraint(BooleanFunction::Urem(c.clone(), d.clone(), 4).get(), BooleanFunction::Const(3, 4)),
                    SMT::Constraint(c.clone(), BooleanFunction::Const(7, 4)),
                },
                SMT::Model({{"C", {7, 4}}, {"D", {4, 4}}})
            },
        });

        for (auto&& [constraints, model] : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));

            for (auto&& [solver_type, solver_call] : std::vector<std::pair<SMT::SolverType, SMT::SolverCall>>{{SMT::SolverType::Z3, SMT::SolverCall::Binary}}) {
                if (!SMT::Solver::has_local_solver_for(solver_type, solver_call)) {
                    continue;
                }

                auto result = solver.query(
                    SMT::QueryConfig()
                        .with_solver(solver_type)
                        .with_local_solver()
                        .with_model_generation()
                        .with_timeout(1000)
                );

                ASSERT_TRUE(result.is_ok());
                auto solver_result = result.get();
                EXPECT_EQ(solver_result.type, SMT::SolverResultType::Sat);
                EXPECT_EQ(*solver_result.model, model);
            }
        }
    }
} //namespace hal
