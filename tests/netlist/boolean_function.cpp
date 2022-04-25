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
            {"117", {BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::ONE}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::Z, BooleanFunction::Value::X}},
            {"X", {BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE, BooleanFunction::Value::X, BooleanFunction::Value::Z}},
        };

        for (auto&& [expected, value]: data) 
        {
            EXPECT_EQ(expected, BooleanFunction::to_string(value, 10).get());
        }
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

            if (function.is_error()) {
                log_error("netlist", "{}", function.get_error().get());   
            }

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

        EXPECT_TRUE((a.clone() | _1.clone()).simplify().has_constant_value(1));
        EXPECT_TRUE((a.clone() ^ a.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((a.clone() & _0.clone()).simplify().has_constant_value(0));
        EXPECT_TRUE((a.clone() - a.clone()).simplify().has_constant_value(0));

        {
            {
                auto res = BooleanFunction::Slice(_A.clone(), i1.clone(), i1.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(res.get().simplify().has_constant_value(0));
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
            if (value.is_error()) {
                log_error("netlist", "{}", value.get_error().get());
            }
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
        };
        
        for (const auto& [function, input, expected]: data) {
            auto value = function.evaluate(input);
            if (value.is_error()) {
                log_error("netlist", "{}", value.get_error().get());
            }
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
                .with_remote_solver()
                .without_model_generation();

            EXPECT_EQ(config.solver, SMT::SolverType::Boolector);
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
            {
                SMT::Constraint(a.clone() & b.clone(), _1.clone())
            },
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
        });

        for (auto&& constraints : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));

            for (auto&& solver_type : {SMT::SolverType::Z3}) {
                if (!SMT::Solver::has_local_solver_for(solver_type)) {
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
            }
        });

        for (auto&& constraints : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));
            for (auto&& solver_type : {SMT::SolverType::Z3}) {
                if (!SMT::Solver::has_local_solver_for(solver_type)) {
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

            for (auto&& solver_type : {SMT::SolverType::Z3}) {
                if (!SMT::Solver::has_local_solver_for(solver_type)) {
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
        });

        for (auto&& [constraints, model] : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));

            for (auto&& solver_type : {SMT::SolverType::Z3}) {
                if (!SMT::Solver::has_local_solver_for(solver_type)) {
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
