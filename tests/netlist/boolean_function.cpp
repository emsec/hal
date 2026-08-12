#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/boolean_function/types.h"

#include <functional>
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

    namespace {
        /**
         * Translates an integer into the little endian vector of values that BooleanFunction::evaluate() takes
         * and returns, so that the expectations below can be written as plain bit patterns.
         */
        std::vector<BooleanFunction::Value> bits(u64 value, u16 size) {
            std::vector<BooleanFunction::Value> res;
            res.reserve(size);
            for (u16 i = 0; i < size; i++) {
                res.push_back(((value >> i) & 1) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
            }
            return res;
        }
    }

    TEST(BooleanFunction, EvaluateShiftsAndRotates) {
        const auto a  = BooleanFunction::Var("A", 4);
        const auto i0 = BooleanFunction::Index(0, 4),
                   i1 = BooleanFunction::Index(1, 4),
                   i3 = BooleanFunction::Index(3, 4),
                   i4 = BooleanFunction::Index(4, 4),
                   i5 = BooleanFunction::Index(5, 4);

        // description, function, value of A, expected result
        const std::vector<std::tuple<std::string, BooleanFunction, u64, u64>> data = {
            {"Shl 0b1011 by 0",  BooleanFunction::Shl(a.clone(), i0.clone(), 4).get(),  0b1011, 0b1011},
            {"Shl 0b1011 by 1",  BooleanFunction::Shl(a.clone(), i1.clone(), 4).get(),  0b1011, 0b0110},
            {"Shl 0b1011 by 3",  BooleanFunction::Shl(a.clone(), i3.clone(), 4).get(),  0b1011, 0b1000},

            {"Lshr 0b1011 by 1", BooleanFunction::Lshr(a.clone(), i1.clone(), 4).get(), 0b1011, 0b0101},
            {"Lshr 0b1011 by 3", BooleanFunction::Lshr(a.clone(), i3.clone(), 4).get(), 0b1011, 0b0001},

            // arithmetic shift right replicates the sign bit, in contrast to the logical one
            {"Ashr 0b1011 by 1", BooleanFunction::Ashr(a.clone(), i1.clone(), 4).get(), 0b1011, 0b1101},
            {"Ashr 0b1011 by 3", BooleanFunction::Ashr(a.clone(), i3.clone(), 4).get(), 0b1011, 0b1111},
            {"Ashr 0b0110 by 1", BooleanFunction::Ashr(a.clone(), i1.clone(), 4).get(), 0b0110, 0b0011},

            {"Rol 0b1011 by 1",  BooleanFunction::Rol(a.clone(), i1.clone(), 4).get(),  0b1011, 0b0111},
            {"Rol 0b1011 by 3",  BooleanFunction::Rol(a.clone(), i3.clone(), 4).get(),  0b1011, 0b1101},
            {"Ror 0b1011 by 1",  BooleanFunction::Ror(a.clone(), i1.clone(), 4).get(),  0b1011, 0b1101},
            {"Ror 0b1011 by 3",  BooleanFunction::Ror(a.clone(), i3.clone(), 4).get(),  0b1011, 0b0111},

            // shifting by at least the bit width leaves nothing of the original value behind, while a
            // rotation wraps around and is the identity for a full turn
            {"Shl 0b1011 by 4",  BooleanFunction::Shl(a.clone(), i4.clone(), 4).get(),  0b1011, 0b0000},
            {"Shl 0b1011 by 5",  BooleanFunction::Shl(a.clone(), i5.clone(), 4).get(),  0b1011, 0b0000},
            {"Lshr 0b1011 by 4", BooleanFunction::Lshr(a.clone(), i4.clone(), 4).get(), 0b1011, 0b0000},
            {"Ashr 0b1011 by 4", BooleanFunction::Ashr(a.clone(), i4.clone(), 4).get(), 0b1011, 0b1111},
            {"Ashr 0b0110 by 4", BooleanFunction::Ashr(a.clone(), i4.clone(), 4).get(), 0b0110, 0b0000},
            {"Rol 0b1011 by 4",  BooleanFunction::Rol(a.clone(), i4.clone(), 4).get(),  0b1011, 0b1011},
            {"Rol 0b1011 by 5",  BooleanFunction::Rol(a.clone(), i5.clone(), 4).get(),  0b1011, 0b0111},
            {"Ror 0b1011 by 4",  BooleanFunction::Ror(a.clone(), i4.clone(), 4).get(),  0b1011, 0b1011},
            {"Ror 0b1011 by 5",  BooleanFunction::Ror(a.clone(), i5.clone(), 4).get(),  0b1011, 0b1101},
        };

        for (const auto& [description, function, input, expected] : data) {
            const auto value = function.evaluate({{"A", bits(input, 4)}});
            ASSERT_TRUE(value.is_ok()) << description;
            EXPECT_EQ(bits(expected, 4), value.get()) << description;
        }
    }

    TEST(BooleanFunction, EvaluateSliceConcatAndExtend) {
        const auto a2 = BooleanFunction::Var("A", 2),
                   b2 = BooleanFunction::Var("B", 2),
                   a4 = BooleanFunction::Var("A", 4);

        {
            // slice keeps the bits from index p1 up to and including index p2
            const std::vector<std::tuple<std::string, BooleanFunction, u64, u16, u64>> data = {
                {"Slice 0b1010 [1:1]", BooleanFunction::Slice(a4.clone(), BooleanFunction::Index(1, 4), BooleanFunction::Index(1, 4), 1).get(), 0b1010, 1, 0b1},
                {"Slice 0b1010 [2:2]", BooleanFunction::Slice(a4.clone(), BooleanFunction::Index(2, 4), BooleanFunction::Index(2, 4), 1).get(), 0b1010, 1, 0b0},
                {"Slice 0b1010 [1:2]", BooleanFunction::Slice(a4.clone(), BooleanFunction::Index(1, 4), BooleanFunction::Index(2, 4), 2).get(), 0b1010, 2, 0b01},
                {"Slice 0b1010 [0:3]", BooleanFunction::Slice(a4.clone(), BooleanFunction::Index(0, 4), BooleanFunction::Index(3, 4), 4).get(), 0b1010, 4, 0b1010},
            };

            for (const auto& [description, function, input, size, expected] : data) {
                const auto value = function.evaluate({{"A", bits(input, 4)}});
                ASSERT_TRUE(value.is_ok()) << description;
                EXPECT_EQ(bits(expected, size), value.get()) << description;
            }
        }

        {
            // the first parameter of a concatenation holds the most significant bits
            const auto function = BooleanFunction::Concat(a2.clone(), b2.clone(), 4).get();
            const auto value    = function.evaluate({{"A", bits(0b10, 2)}, {"B", bits(0b01, 2)}});
            ASSERT_TRUE(value.is_ok());
            EXPECT_EQ(bits(0b1001, 4), value.get());
        }

        {
            const auto zext = BooleanFunction::Zext(a2.clone(), BooleanFunction::Index(4, 4), 4).get();
            const auto sext = BooleanFunction::Sext(a2.clone(), BooleanFunction::Index(4, 4), 4).get();

            // zero extension always pads with zeroes, sign extension replicates the most significant bit
            const std::vector<std::tuple<std::string, BooleanFunction, u64, u64>> data = {
                {"Zext 0b01 to 4", zext.clone(), 0b01, 0b0001},
                {"Zext 0b11 to 4", zext.clone(), 0b11, 0b0011},
                {"Sext 0b01 to 4", sext.clone(), 0b01, 0b0001},
                {"Sext 0b11 to 4", sext.clone(), 0b11, 0b1111},
            };

            for (const auto& [description, function, input, expected] : data) {
                const auto value = function.evaluate({{"A", bits(input, 2)}});
                ASSERT_TRUE(value.is_ok()) << description;
                EXPECT_EQ(bits(expected, 4), value.get()) << description;
            }
        }
    }

    TEST(BooleanFunction, EvaluateComparisons) {
        const auto a = BooleanFunction::Var("A", 4),
                   b = BooleanFunction::Var("B", 4);

        const auto eq  = BooleanFunction::Eq(a.clone(), b.clone(), 1).get(),
                   ule = BooleanFunction::Ule(a.clone(), b.clone(), 1).get(),
                   ult = BooleanFunction::Ult(a.clone(), b.clone(), 1).get(),
                   sle = BooleanFunction::Sle(a.clone(), b.clone(), 1).get(),
                   slt = BooleanFunction::Slt(a.clone(), b.clone(), 1).get();

        // description, function, A, B, expected. 0b1111 is 15 unsigned but -1 signed, 0b1000 is 8 but -8.
        const std::vector<std::tuple<std::string, BooleanFunction, u64, u64, u64>> data = {
            {"0b1010 == 0b1010", eq.clone(),  0b1010, 0b1010, 1},
            {"0b1010 == 0b0101", eq.clone(),  0b1010, 0b0101, 0},

            {"1 <=u 15", ule.clone(), 0b0001, 0b1111, 1},
            {"15 <=u 15", ule.clone(), 0b1111, 0b1111, 1},
            {"15 <=u 1", ule.clone(), 0b1111, 0b0001, 0},
            {"1 <u 15", ult.clone(), 0b0001, 0b1111, 1},
            {"15 <u 15", ult.clone(), 0b1111, 0b1111, 0},
            {"15 <u 1", ult.clone(), 0b1111, 0b0001, 0},

            // the same bit patterns compare the other way round once they are read as signed
            {"-1 <=s 1", sle.clone(), 0b1111, 0b0001, 1},
            {"1 <=s -1", sle.clone(), 0b0001, 0b1111, 0},
            {"-8 <=s -8", sle.clone(), 0b1000, 0b1000, 1},
            {"-1 <s 1", slt.clone(), 0b1111, 0b0001, 1},
            {"1 <s -1", slt.clone(), 0b0001, 0b1111, 0},
            {"-8 <s 7", slt.clone(), 0b1000, 0b0111, 1},
            {"-8 <s -8", slt.clone(), 0b1000, 0b1000, 0},
        };

        for (const auto& [description, function, lhs, rhs, expected] : data) {
            const auto value = function.evaluate({{"A", bits(lhs, 4)}, {"B", bits(rhs, 4)}});
            ASSERT_TRUE(value.is_ok()) << description;
            EXPECT_EQ(bits(expected, 1), value.get()) << description;
        }
    }

    TEST(BooleanFunction, EvaluateIte) {
        const auto c = BooleanFunction::Var("C", 1),
                   a = BooleanFunction::Var("A", 4),
                   b = BooleanFunction::Var("B", 4);

        const auto function = BooleanFunction::Ite(c.clone(), a.clone(), b.clone(), 4).get();

        {
            const auto value = function.evaluate({{"C", bits(1, 1)}, {"A", bits(0b1010, 4)}, {"B", bits(0b0101, 4)}});
            ASSERT_TRUE(value.is_ok());
            EXPECT_EQ(bits(0b1010, 4), value.get());
        }
        {
            const auto value = function.evaluate({{"C", bits(0, 1)}, {"A", bits(0b1010, 4)}, {"B", bits(0b0101, 4)}});
            ASSERT_TRUE(value.is_ok());
            EXPECT_EQ(bits(0b0101, 4), value.get());
        }
    }

    TEST(BooleanFunction, EvaluateUndefinedPropagation) {
        using Value = BooleanFunction::Value;

        const auto a = BooleanFunction::Var("A"),
                   b = BooleanFunction::Var("B");

        // an undefined input only stays undefined where the other operand does not already determine the result
        const std::vector<std::tuple<std::string, BooleanFunction, Value, Value, Value>> data = {
            {"0 & X", a & b, Value::ZERO, Value::X, Value::ZERO},
            {"1 & X", a & b, Value::ONE, Value::X, Value::X},
            {"X & X", a & b, Value::X, Value::X, Value::X},

            {"1 | X", a | b, Value::ONE, Value::X, Value::ONE},
            {"0 | X", a | b, Value::ZERO, Value::X, Value::X},
            {"X | X", a | b, Value::X, Value::X, Value::X},

            {"0 ^ X", a ^ b, Value::ZERO, Value::X, Value::X},
            {"1 ^ X", a ^ b, Value::ONE, Value::X, Value::X},
        };

        for (const auto& [description, function, lhs, rhs, expected] : data) {
            const std::unordered_map<std::string, std::vector<Value>> input = {{"A", {lhs}}, {"B", {rhs}}};
            const auto value = function.evaluate(input);
            ASSERT_TRUE(value.is_ok()) << description;
            EXPECT_EQ(std::vector<Value>({expected}), value.get()) << description;
        }

        {
            const std::unordered_map<std::string, std::vector<Value>> input = {{"A", {Value::X}}};
            const auto value = (~a).evaluate(input);
            ASSERT_TRUE(value.is_ok());
            EXPECT_EQ(std::vector<Value>({Value::X}), value.get()) << "~X";
        }

        {
            const auto a4 = BooleanFunction::Var("A", 4),
                       b4 = BooleanFunction::Var("B", 4);

            // 0b00X1 and 0b0001
            const std::unordered_map<std::string, std::vector<Value>> input = {
                {"A", {Value::ONE, Value::X, Value::ZERO, Value::ZERO}},
                {"B", {Value::ONE, Value::ZERO, Value::ZERO, Value::ZERO}}};

            const auto undefined = std::vector<Value>(4, Value::X);

            // a single undefined bit makes the whole arithmetic result undefined, while the bitwise
            // operations stay per bit
            EXPECT_EQ(undefined, (a4 + b4).evaluate(input).get()) << "0b00X1 + 0b0001";
            EXPECT_EQ(undefined, (a4 - b4).evaluate(input).get()) << "0b00X1 - 0b0001";
            EXPECT_EQ(undefined, (a4 * b4).evaluate(input).get()) << "0b00X1 * 0b0001";
            EXPECT_EQ(bits(0b0001, 4), (a4 & b4).evaluate(input).get()) << "0b00X1 & 0b0001";

            EXPECT_EQ(std::vector<Value>({Value::X}), BooleanFunction::Ult(a4.clone(), b4.clone(), 1).get().evaluate(input).get()) << "0b00X1 <u 0b0001";

            // an undefined bit hides whether the two are equal, just as it does for the other comparisons
            EXPECT_EQ(std::vector<Value>({Value::X}), BooleanFunction::Eq(a4.clone(), b4.clone(), 1).get().evaluate(input).get()) << "0b00X1 == 0b0001";

            // unless another bit already tells them apart
            const std::unordered_map<std::string, std::vector<Value>> distinguishable = {
                {"A", {Value::ONE, Value::X, Value::ZERO, Value::ZERO}},
                {"B", {Value::ZERO, Value::ZERO, Value::ZERO, Value::ZERO}}};
            EXPECT_EQ(bits(0, 1), BooleanFunction::Eq(a4.clone(), b4.clone(), 1).get().evaluate(distinguishable).get()) << "0b00X1 == 0b0000";
        }
    }

    TEST(BooleanFunction, DivisionFactories) {
        const auto a = BooleanFunction::Var("A", 4),
                   b = BooleanFunction::Var("B", 4),
                   c = BooleanFunction::Var("C", 8);

        struct Factory
        {
            std::string name;
            std::function<Result<BooleanFunction>(BooleanFunction&&, BooleanFunction&&, u16)> create;
            u16 node_type;
            std::string symbol;
        };

        const std::vector<Factory> factories = {
            {"Sdiv", &BooleanFunction::Sdiv, BooleanFunction::NodeType::Sdiv, "/s"},
            {"Udiv", &BooleanFunction::Udiv, BooleanFunction::NodeType::Udiv, "/"},
            {"Srem", &BooleanFunction::Srem, BooleanFunction::NodeType::Srem, "%s"},
            {"Urem", &BooleanFunction::Urem, BooleanFunction::NodeType::Urem, "%"},
        };

        for (const auto& factory : factories)
        {
            {
                // operands and result of equal size are accepted
                auto res = factory.create(a.clone(), b.clone(), 4);
                ASSERT_TRUE(res.is_ok()) << factory.name;

                const auto function = res.get();
                EXPECT_EQ(4, function.size()) << factory.name;
                EXPECT_EQ(factory.node_type, function.get_top_level_node().type) << factory.name;
                EXPECT_EQ(std::set<std::string>({"A", "B"}), function.get_variable_names()) << factory.name;
                EXPECT_NE(std::string::npos, function.to_string().find(factory.symbol)) << factory.name << " prints as " << function.to_string();
            }

            // operands of different size are rejected
            EXPECT_TRUE(factory.create(a.clone(), c.clone(), 4).is_error()) << factory.name << " with mismatched operands";
            EXPECT_TRUE(factory.create(c.clone(), a.clone(), 4).is_error()) << factory.name << " with mismatched operands";

            // a result size that differs from the operands is rejected
            EXPECT_TRUE(factory.create(a.clone(), b.clone(), 8).is_error()) << factory.name << " with mismatched result size";
        }
    }

    TEST(BooleanFunction, EvaluateWideArithmetic) {
        // Addition and subtraction used to mask their result to 32 bit before truncating it to the width of
        // the operands, so anything wider than that silently lost its upper bits.
        for (const u16 size : {16, 32, 33, 40, 64}) {
            const auto a = BooleanFunction::Var("A", size),
                       b = BooleanFunction::Var("B", size);

            const u64 mask    = (size >= 64) ? ~0ull : ((1ull << size) - 1);
            const u64 operand = 1ull << (size - 2);

            {
                // the sum carries into the most significant bit
                const std::unordered_map<std::string, std::vector<BooleanFunction::Value>> input = {{"A", bits(operand, size)}, {"B", bits(operand, size)}};
                const auto value = (a + b).evaluate(input);
                ASSERT_TRUE(value.is_ok()) << "addition of " << size << " bit operands";
                EXPECT_EQ(bits((operand + operand) & mask, size), value.get()) << "addition of " << size << " bit operands";
            }
            {
                // the difference underflows and wraps around over the full width
                const std::unordered_map<std::string, std::vector<BooleanFunction::Value>> input = {{"A", bits(0, size)}, {"B", bits(1, size)}};
                const auto value = (a - b).evaluate(input);
                ASSERT_TRUE(value.is_ok()) << "subtraction of " << size << " bit operands";
                EXPECT_EQ(bits(mask, size), value.get()) << "subtraction of " << size << " bit operands";
            }
        }
    }

    TEST(BooleanFunction, EvaluateDivisionAndRemainder) {
        // The division operations are translated to bvudiv, bvurem, bvsdiv and bvsrem when handed to an SMT
        // solver, so constant folding has to agree with the SMT-LIB definitions of those. Reference
        // implementations of exactly those definitions, against which the folding is checked exhaustively.
        const auto mask = [](u16 size) { return (size >= 64) ? ~0ull : ((1ull << size) - 1); };
        const auto negate = [&](u64 value, u16 size) { return (~value + 1) & mask(size); };
        const auto is_negative = [](u64 value, u16 size) { return ((value >> (size - 1)) & 1) == 1; };

        const auto ref_udiv = [&](u64 s, u64 t, u16 size) { return (t == 0) ? mask(size) : (s / t); };
        const auto ref_urem = [&](u64 s, u64 t, u16 size) { return (t == 0) ? s : (s % t); };

        const auto ref_sdiv = [&](u64 s, u64 t, u16 size) {
            const auto ms = is_negative(s, size), mt = is_negative(t, size);
            if (!ms && !mt) return ref_udiv(s, t, size);
            if (ms && !mt) return negate(ref_udiv(negate(s, size), t, size), size);
            if (!ms && mt) return negate(ref_udiv(s, negate(t, size), size), size);
            return ref_udiv(negate(s, size), negate(t, size), size);
        };
        const auto ref_srem = [&](u64 s, u64 t, u16 size) {
            const auto ms = is_negative(s, size), mt = is_negative(t, size);
            if (!ms && !mt) return ref_urem(s, t, size);
            if (ms && !mt) return negate(ref_urem(negate(s, size), t, size), size);
            if (!ms && mt) return ref_urem(s, negate(t, size), size);
            return negate(ref_urem(negate(s, size), negate(t, size), size), size);
        };

        constexpr u16 size = 4;
        const auto a = BooleanFunction::Var("A", size),
                   b = BooleanFunction::Var("B", size);

        const auto udiv = BooleanFunction::Udiv(a.clone(), b.clone(), size).get(),
                   urem = BooleanFunction::Urem(a.clone(), b.clone(), size).get(),
                   sdiv = BooleanFunction::Sdiv(a.clone(), b.clone(), size).get(),
                   srem = BooleanFunction::Srem(a.clone(), b.clone(), size).get();

        // every pair of 4 bit operands, including every division by zero
        for (u64 lhs = 0; lhs < 16; lhs++) {
            for (u64 rhs = 0; rhs < 16; rhs++) {
                const std::unordered_map<std::string, std::vector<BooleanFunction::Value>> input = {{"A", bits(lhs, size)}, {"B", bits(rhs, size)}};

                const auto quotient_unsigned = udiv.evaluate(input);
                ASSERT_TRUE(quotient_unsigned.is_ok()) << lhs << " udiv " << rhs;
                EXPECT_EQ(bits(ref_udiv(lhs, rhs, size) & mask(size), size), quotient_unsigned.get()) << lhs << " udiv " << rhs;

                const auto remainder_unsigned = urem.evaluate(input);
                ASSERT_TRUE(remainder_unsigned.is_ok()) << lhs << " urem " << rhs;
                EXPECT_EQ(bits(ref_urem(lhs, rhs, size) & mask(size), size), remainder_unsigned.get()) << lhs << " urem " << rhs;

                const auto quotient_signed = sdiv.evaluate(input);
                ASSERT_TRUE(quotient_signed.is_ok()) << lhs << " sdiv " << rhs;
                EXPECT_EQ(bits(ref_sdiv(lhs, rhs, size) & mask(size), size), quotient_signed.get()) << lhs << " sdiv " << rhs;

                const auto remainder_signed = srem.evaluate(input);
                ASSERT_TRUE(remainder_signed.is_ok()) << lhs << " srem " << rhs;
                EXPECT_EQ(bits(ref_srem(lhs, rhs, size) & mask(size), size), remainder_signed.get()) << lhs << " srem " << rhs;
            }
        }
    }

    TEST(BooleanFunction, EvaluateDivisionEdgeCases) {
        using Value = BooleanFunction::Value;

        {
            // an undefined bit in either operand makes the whole result undefined, as it does for the other
            // arithmetic operations
            const auto a = BooleanFunction::Var("A", 4),
                       b = BooleanFunction::Var("B", 4);
            const std::unordered_map<std::string, std::vector<Value>> input = {
                {"A", {Value::ONE, Value::X, Value::ZERO, Value::ZERO}},
                {"B", bits(0b0010, 4)}};

            const auto undefined = std::vector<Value>(4, Value::X);
            EXPECT_EQ(undefined, BooleanFunction::Udiv(a.clone(), b.clone(), 4).get().evaluate(input).get());
            EXPECT_EQ(undefined, BooleanFunction::Urem(a.clone(), b.clone(), 4).get().evaluate(input).get());
            EXPECT_EQ(undefined, BooleanFunction::Sdiv(a.clone(), b.clone(), 4).get().evaluate(input).get());
            EXPECT_EQ(undefined, BooleanFunction::Srem(a.clone(), b.clone(), 4).get().evaluate(input).get());
        }

        {
            // wider than 64 bit, where the operands no longer fit into an integer
            constexpr u16 size = 80;
            const auto a = BooleanFunction::Var("A", size),
                       b = BooleanFunction::Var("B", size);

            auto dividend = std::vector<Value>(size, Value::ZERO);
            auto divisor  = std::vector<Value>(size, Value::ZERO);
            auto expected = std::vector<Value>(size, Value::ZERO);
            dividend[70]  = Value::ONE;    // 2^70
            divisor[35]   = Value::ONE;    // 2^35
            expected[35]  = Value::ONE;    // 2^35

            const std::unordered_map<std::string, std::vector<Value>> input = {{"A", dividend}, {"B", divisor}};

            EXPECT_EQ(expected, BooleanFunction::Udiv(a.clone(), b.clone(), size).get().evaluate(input).get()) << "2^70 udiv 2^35";
            EXPECT_EQ(std::vector<Value>(size, Value::ZERO), BooleanFunction::Urem(a.clone(), b.clone(), size).get().evaluate(input).get()) << "2^70 urem 2^35";
        }

        {
            // a division by zero yields all ones for the quotient and the dividend for the remainder, which
            // is what bvudiv and bvurem are defined to do
            const auto a = BooleanFunction::Var("A", 4),
                       b = BooleanFunction::Var("B", 4);
            const std::unordered_map<std::string, std::vector<Value>> input = {{"A", bits(0b0101, 4)}, {"B", bits(0, 4)}};

            EXPECT_EQ(bits(0b1111, 4), BooleanFunction::Udiv(a.clone(), b.clone(), 4).get().evaluate(input).get()) << "5 udiv 0";
            EXPECT_EQ(bits(0b0101, 4), BooleanFunction::Urem(a.clone(), b.clone(), 4).get().evaluate(input).get()) << "5 urem 0";
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
