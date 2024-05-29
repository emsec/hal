#include "bitwuzla_utils/symbolic_execution.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/boolean_function/types.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include <iostream>
#include <type_traits>
#include <variant>

namespace hal
{

    TEST(BooleanFunction, ConstantSimplification)
    {
        const auto _0 = BooleanFunction::Const(0, 1), _1 = BooleanFunction::Const(1, 1), _A = BooleanFunction::Const(0xA, 4), a = BooleanFunction::Var("A"), i1 = BooleanFunction::Index(1, 4),
                   i2 = BooleanFunction::Index(2, 4), i4 = BooleanFunction::Index(4, 4);  

        
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(~_1).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(~_0).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 | _0).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 | _1).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 | _1).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 & _0).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 & _1).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 & _1).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 ^ _0).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 ^ _1).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 ^ _1).get()).get().is_bv_value_zero());

        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 + _0).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 + _1).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 + _0).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 + _1).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 - _0).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 - _1).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 - _0).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 - _1).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 * _0).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_0 * _1).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 * _0).get()).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(_1 * _1).get()).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(BooleanFunction::Const(100, 8) + BooleanFunction::Const(50, 8)).get()).get().value<std::string>(10) == "150");
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(BooleanFunction::Const(200, 8) + BooleanFunction::Const(60, 8)).get()).get().value<std::string>(10) == "4");
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(BooleanFunction::Const(100, 8) - BooleanFunction::Const(50, 8)).get()).get().value<std::string>(10) == "50");
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(BooleanFunction::Const(50, 8) - BooleanFunction::Const(100, 8)).get()).get().value<std::string>(10) == "206");
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(BooleanFunction::Const(5, 8) * BooleanFunction::Const(5, 8)).get()).get().value<std::string>(10) == "25");
        EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(BooleanFunction::Const(50, 8) * BooleanFunction::Const(50, 8)).get()).get().value<std::string>(10) == "196");

        EXPECT_TRUE(bitwuzla_utils::simplify((bitwuzla_utils::from_bf(a | _1).get())).get().is_bv_value_ones());
        EXPECT_TRUE(bitwuzla_utils::simplify((bitwuzla_utils::from_bf(a ^ a).get())).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify((bitwuzla_utils::from_bf(a & _0).get())).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify((bitwuzla_utils::from_bf(a - a).get())).get().is_bv_value_zero());
        EXPECT_TRUE(bitwuzla_utils::simplify((bitwuzla_utils::from_bf(a * _0).get())).get().is_bv_value_zero());

        {
            {
                auto res = BooleanFunction::Slice(_A.clone(), i1.clone(), i1.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_bv_value_ones());
            }
            {
                auto res = BooleanFunction::Slice(_A.clone(), i2.clone(), i2.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_bv_value_zero());
            }
            {
                auto res = BooleanFunction::Slice(_A.clone(), i1.clone(), i2.clone(), 2);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_bv_value_one());
            }
            {
                auto res = BooleanFunction::Concat(_1.clone(), _0.clone(), 2);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().value<std::string>(10) == "2");
            }
            {
                auto res = BooleanFunction::Concat(_A.clone(), _0.clone(), 5);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().value<std::string>(10) == "20");
            }
            {
                auto res = BooleanFunction::Zext(_1.clone(), i4.clone(), 4);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_bv_value_one());
            }
            {
                auto res = BooleanFunction::Sext(_0.clone(), i4.clone(), 4);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_bv_value_zero());
            }
            {
                auto res = BooleanFunction::Sext(_1.clone(), i4.clone(), 4);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().value<std::string>(10) == "15");
            }
        }

        {
            {
                auto res = BooleanFunction::Eq(_0.clone(), _0.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Eq(_0.clone(), _1.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }
            {
                auto res = BooleanFunction::Eq(_1.clone(), _0.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }
            {
                auto res = BooleanFunction::Eq(_1.clone(), _1.clone(), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }

            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xC, 4), BooleanFunction::Const(0x3, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Slt(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }

            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xC, 4), BooleanFunction::Const(0x3, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Sle(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }

            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Ule(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }

            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0x0, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0xE, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
            }
            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xF, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }
            {
                auto res = BooleanFunction::Ult(BooleanFunction::Const(0xF, 4), BooleanFunction::Const(0xA, 4), 1);
                ASSERT_TRUE(res.is_ok());
                EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
            }

            // {
            //     auto res = BooleanFunction::Ite(_0.clone(), _1.clone(), _0.clone(), 1);
            //     ASSERT_TRUE(res.is_ok());
            //     EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_bv_value_zero());
            // }
            // {
            //     auto res = BooleanFunction::Ite(_1.clone(), _1.clone(), _0.clone(), 1);
            //     ASSERT_TRUE(res.is_ok());
            //     EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_bv_value_ones());
            // }
        }
    }

    TEST(BooleanFunction, SimplificationRules)
    {
        const auto a = BooleanFunction::Var("A"), b = BooleanFunction::Var("B"), c = BooleanFunction::Var("C"), d = BooleanFunction::Var("D", 16), e = BooleanFunction::Var("E", 16),
                   f = BooleanFunction::Var("F", 16), _0 = BooleanFunction::Const(0, 1), _1 = BooleanFunction::Const(1, 1), i0 = BooleanFunction::Index(0, 1);

        ////////////////////////////////////////////////////////////////////////
        // AND RULES
        ////////////////////////////////////////////////////////////////////////

        // (a & 0)   =>    0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & _0).get()).get()).get(), _0);
        // (a & 1)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & _1).get()).get()).get(), a);
        // (a & a)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & a).get()).get()).get(), a);
        // (a & ~a)  =>    0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & ~a).get()).get()).get(), _0);

        // (a & b) & a   =>   a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & b) & a).get()).get()).get(), a & b);
        // (a & b) & b   =>   a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & b) & b).get()).get()).get(), a & b);
        // a & (b & a)   =>   a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & (b & a)).get()).get()).get(), a & b);
        // b & (b & a)   =>   a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b & (b & a)).get()).get()).get(), a & b);

        // a & (a | b)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & (a | b)).get()).get()).get(), a);
        // b & (a | b)   =>    b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b & (a | b)).get()).get()).get(), b);
        // (a | b) & a   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a | b) & a).get()).get()).get(), a);
        // (a | b) & b   =>    b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a | b) & b).get()).get()).get(), b);

        // (~a & b) & a   =>   0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((~a & b) & a).get()).get()).get(), _0);
        // (a & ~b) & b   =>   0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & ~b) & b).get()).get()).get(), _0);
        // a & (b & ~a)   =>   0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & (b & ~a)).get()).get()).get(), _0);
        // b & (~b & a)   =>   0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b & (~b & a)).get()).get()).get(), _0);

        // a & (~a | b)   =>    a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a & (~a | b)).get()).get()).get(), a & b);
        // b & (a | ~b)   =>    a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b & (a | ~b)).get()).get()).get(), a & b);
        // (~a | b) & a   =>    a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((~a | b) & a).get()).get()).get(), a & b);
        // (a | ~b) & b   =>    a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a | ~b) & b).get()).get()).get(), a & b);

        ////////////////////////////////////////////////////////////////////////
        // OR RULES
        ////////////////////////////////////////////////////////////////////////

        // (a | 0)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | _0).get()).get()).get(), a);
        // (a | 1)   =>    1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | _1).get()).get()).get(), _1);
        // (a | a)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | a).get()).get()).get(), a);
        // (a | ~a)  =>    1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | ~a).get()).get()).get(), _1);

        // a | (a | b)   =>    a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | (a | b)).get()).get()).get(), a | b);
        // b | (a | b)   =>    a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b | (a | b)).get()).get()).get(), a | b);
        // (a | b) | a   =>    a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a | b) | a).get()).get()).get(), a | b);
        // (a | b) | b   =>    a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a | b) | b).get()).get()).get(), a | b);

        // (a & b) | a   =>   a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & b) | a).get()).get()).get(), a);
        // (a & b) | b   =>   b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & b) | b).get()).get()).get(), b);
        // a | (b & a)   =>   a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | (b & a)).get()).get()).get(), a);
        // b | (b & a)   =>   b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b | (b & a)).get()).get()).get(), b);

        // a | (~a | b)   =>   1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | (~a | b)).get()).get()).get(), _1);
        // b | (a | ~b)   =>   1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b | (a | ~b)).get()).get()).get(), _1);
        // (~a | b) | a   =>   1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((~a | b) | a).get()).get()).get(), _1);
        // (a | ~b) | b   =>   1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a | ~b) | b).get()).get()).get(), _1);

        // (~a & b) | a   =>   a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((~a & b) | a).get()).get()).get(), a | b);
        // (a & ~b) | b   =>   a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & ~b) | b).get()).get()).get(), a | b);
        // a | (b & ~a)   =>   a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a | (b & ~a)).get()).get()).get(), a | b);
        // b | (~b & a)   =>   a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(b | (~b & a)).get()).get()).get(), a | b);

        ////////////////////////////////////////////////////////////////////////
        // NOT RULES
        ////////////////////////////////////////////////////////////////////////

        // ~~a   =>   a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(~(~a)).get()).get()).get(), a);
        // ~(~a | ~b)   =>   a & b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(~(~a | ~b)).get()).get()).get(), a & b);
        // ~(~a & ~b)   =>   a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(~(~a & ~b)).get()).get()).get(), a | b);

        ////////////////////////////////////////////////////////////////////////
        // XOR RULES
        ////////////////////////////////////////////////////////////////////////

        // (a ^ 0)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a ^ _0).get()).get()).get(), a);
        // (a ^ 1)   =>    ~a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a ^ _1).get()).get()).get(), ~a);
        // (a ^ a)   =>    0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a ^ a).get()).get()).get(), _0);
        // (a ^ ~a)  =>    1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a ^ ~a).get()).get()).get(), _1);

        ////////////////////////////////////////////////////////////////////////
        // ADD RULES
        ////////////////////////////////////////////////////////////////////////

        // (a + 0)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a + _0).get()).get()).get(), a);

        ////////////////////////////////////////////////////////////////////////
        // SUB RULES
        ////////////////////////////////////////////////////////////////////////

        // (a - 0)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a - _0).get()).get()).get(), a);
        // (a - a)   =>    0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a - a).get()).get()).get(), _0);

        ////////////////////////////////////////////////////////////////////////
        // MUL RULES
        ////////////////////////////////////////////////////////////////////////

        // (a * 0)   =>    0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a * _0).get()).get()).get(), _0);
        // (a * 1)   =>    a
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(a * _1).get()).get()).get(), a);

        ////////////////////////////////////////////////////////////////////////
        // SDIV RULES
        ////////////////////////////////////////////////////////////////////////

        // (a /s 1)   =>    a
        {
            auto res = BooleanFunction::Sdiv(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), a);
        }
        // (a /s a)   =>    1
        {
            auto res = BooleanFunction::Sdiv(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), _1);
        }

        ////////////////////////////////////////////////////////////////////////
        // UDIV RULES
        ////////////////////////////////////////////////////////////////////////

        // (a /s 1)   =>    a
        {
            auto res = BooleanFunction::Udiv(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), a);
        }
        // (a /s a)   =>    1
        {
            auto res = BooleanFunction::Udiv(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), _1);
        }

        ////////////////////////////////////////////////////////////////////////
        // SREM RULES
        ////////////////////////////////////////////////////////////////////////

        // (a %s 1)   =>    0
        {
            auto res = BooleanFunction::Srem(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), _0);
        }
        // (a %s a)   =>    0
        {
            auto res = BooleanFunction::Srem(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), _0);
        }

        ////////////////////////////////////////////////////////////////////////
        // UREM RULES
        ////////////////////////////////////////////////////////////////////////

        // (a % 1)   =>    0
        {
            auto res = BooleanFunction::Urem(a.clone(), _1.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), _0);
        }
        // (a % a)   =>    0
        {
            auto res = BooleanFunction::Urem(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), _0);
        }

        ////////////////////////////////////////////////////////////////////////
        // SLICE RULES
        ////////////////////////////////////////////////////////////////////////

        // SLICE(p, 0, 0)   =>   p (if p is 1-bit wide)
        {
            auto res = BooleanFunction::Slice(a.clone(), i0.clone(), i0.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), a);
        }

        ////////////////////////////////////////////////////////////////////////
        // EQUALITY RULES
        ////////////////////////////////////////////////////////////////////////

        // X == X   =>   1
        {
            auto res = BooleanFunction::Eq(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
        }
        ////////////////////////////////////////////////////////////////////////
        // SIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X <=s X   =>   1
        {
            auto res = BooleanFunction::Sle(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
        }

        ////////////////////////////////////////////////////////////////////////
        // SIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X <s X   =>   0
        {
            auto res = BooleanFunction::Slt(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
        }

        ////////////////////////////////////////////////////////////////////////
        // UNSIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X <= X   =>   1
        {
            auto res = BooleanFunction::Ule(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_true());
        }

        ////////////////////////////////////////////////////////////////////////
        // UNSIGNED LESS THAN RULES
        ////////////////////////////////////////////////////////////////////////

        // X < 0   =>   0
        {
            auto res = BooleanFunction::Ult(a.clone(), _0.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
        }
        // X < X   =>   0
        {
            auto res = BooleanFunction::Ult(a.clone(), a.clone(), 1);
            ASSERT_TRUE(res.is_ok());
            EXPECT_TRUE(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get().is_false());
        }

        ////////////////////////////////////////////////////////////////////////
        // IF-THEN-ELSE RULES
        ////////////////////////////////////////////////////////////////////////


        //TODO
        // ITE not implemented

        // // ITE(0, a, b)  =>  b
        // {
        //     auto res = BooleanFunction::Ite(_0.clone(), a.clone(), b.clone(), 1);
        //     ASSERT_TRUE(res.is_ok());
        //     EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), b);
        // }
        // // ITE(1, a, b)  =>  a
        // {
        //     auto res = BooleanFunction::Ite(_1.clone(), a.clone(), b.clone(), 1);
        //     ASSERT_TRUE(res.is_ok());
        //     EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), a);
        // }
        // // ITE(a, b, b)  =>  b
        // {
        //     auto res = BooleanFunction::Ite(a.clone(), b.clone(), b.clone(), 1);
        //     ASSERT_TRUE(res.is_ok());
        //     EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), b);
        // }

        ////////////////////////////////////////////////////////////////////////
        // CONCAT RULES
        ////////////////////////////////////////////////////////////////////////

        // CONCAT(SLICE(X, j+1, k), SLICE(X, i, j)) => SLICE(X, i, k)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(15, d.size()), 16);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), s1.get(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), s3.get());
        }

        // CONCAT(SLICE(X, j, j), SLICE(X, i, j)) => SEXT(SLICE(X, i, j), j-i+1)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(7, d.size()), BooleanFunction::Index(7, d.size()), 1);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto sext1 = BooleanFunction::Sext(s1.get(), BooleanFunction::Index(s1.get().size() + 1, s1.get().size() + 1), s1.get().size() + 1);

            ASSERT_TRUE(sext1.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), s1.get(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), sext1.get());
        }

        // CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), Y)) => CONCAT(CONCAT(SLICE(X, 0, 7), SLICE(X, 8, 15)), Y))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto c1 = BooleanFunction::Concat(s2.get(), a.clone(), s2.get().size() + a.size());
            auto c2 = BooleanFunction::Concat(s1.get(), s2.get(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto c3 = BooleanFunction::Concat(c2.get(), a.clone(), c2.get().size() + a.size());

            auto res = BooleanFunction::Concat(s1.get(), c1.get(), s1.get().size() + c1.get().size());

            ASSERT_TRUE(c3.is_ok());
            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), c3.get());
        }

        // CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), SLICE(Z, 8, 15))) => CONCAT(CONCAT(SLICE(X, 0, 7), SLICE(X, 8, 15)), SLICE(Z, 8, 15)))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);
            auto s3 = BooleanFunction::Slice(e.clone(), BooleanFunction::Index(8, e.size()), BooleanFunction::Index(15, e.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto c1 = BooleanFunction::Concat(s2.get(), s3.get(), s2.get().size() + s3.get().size());
            auto c2 = BooleanFunction::Concat(s1.get(), s2.get(), s1.get().size() + s2.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto c3 = BooleanFunction::Concat(c2.get(), s3.get(), c2.get().size() + s3.get().size());

            auto res = BooleanFunction::Concat(s1.get(), c1.get(), s1.get().size() + c1.get().size());

            ASSERT_TRUE(c3.is_ok());
            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), c3.get());
        }

        // CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), SLICE(X, 8, 15))) => CONCAT(SLICE(X, 0, 7), CONCAT(SLICE(X, 8, 15), SLICE(X, 8, 15)))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto c1 = BooleanFunction::Concat(s2.get(), s3.get(), s2.get().size() + s3.get().size());

            ASSERT_TRUE(c1.is_ok());

            auto res = BooleanFunction::Concat(s1.get(), c1.get(), s1.get().size() + c1.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), res.get());
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
            auto res = BooleanFunction::Concat(s1.get(), c2.get(), s1.get().size() + c2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), c4.get());
        }

        // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), CONCAT(SLICE(X, m, n), Z))) => CONCAT(SLICE(X, i, j), CONCAT(CONCAT(SLICE(X, k, l), SLICE(X, m, n)), Z))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(4, d.size()), BooleanFunction::Index(11, d.size()), 8);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());

            auto c1 = BooleanFunction::Concat(s3.get(), e.clone(), s3.get().size() + e.size());
            auto c2 = BooleanFunction::Concat(s2.get(), c1.get(), s2.get().size() + c1.get().size());

            auto c3 = BooleanFunction::Concat(s2.get(), s3.get(), s2.get().size() + s3.get().size());
            auto c4 = BooleanFunction::Concat(c3.get(), e.clone(), c3.get().size() + e.size());
            auto c5 = BooleanFunction::Concat(s1.get(), c4.get(), s1.get().size() + c4.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());
            ASSERT_TRUE(c3.is_ok());
            ASSERT_TRUE(c4.is_ok());
            ASSERT_TRUE(c5.is_ok());

            auto res = BooleanFunction::Concat(s1.get(), c2.get(), s1.get().size() + c2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), c5.get());
        }

        // CONCAT(SLICE(X, 8, 15), CONCAT(SLICE(X, 0, 7), SLICE(Y, 0, 7))) => CONCAT(SLICE(X, 15, 0), SLICE(Y, 0, 7))
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(8, d.size()), BooleanFunction::Index(15, d.size()), 8);
            auto s3 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(15, d.size()), 16);
            auto s4 = BooleanFunction::Slice(e.clone(), BooleanFunction::Index(0, e.size()), BooleanFunction::Index(7, e.size()), 8);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());
            ASSERT_TRUE(s3.is_ok());
            ASSERT_TRUE(s4.is_ok());

            auto c1 = BooleanFunction::Concat(s1.get(), s4.get(), s1.get().size() + s4.get().size());
            auto c2 = BooleanFunction::Concat(s3.get(), s4.get(), s3.get().size() + s4.get().size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), c1.get(), s2.get().size() + c1.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), c2.get());
        }
        return;

        // CONCAT(SLICE(X, j, j), SEXT(SLICE(X, i, j), j-i+1)) => SEXT(SLICE(X, i, j), j-i+2)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(7, d.size()), BooleanFunction::Index(7, d.size()), 1);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto sext1 = BooleanFunction::Sext(s1.get(), BooleanFunction::Index(s1.get().size() + 1, s1.get().size() + 1), s1.get().size() + 1);
            auto sext2 = BooleanFunction::Sext(s1.get(), BooleanFunction::Index(s1.get().size() + 2, s1.get().size() + 2), s1.get().size() + 2);

            ASSERT_TRUE(sext1.is_ok());
            ASSERT_TRUE(sext2.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), sext1.get(), sext1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), sext2.get());
        }

        // CONCAT(SLICE(X, 7, 7), CONCAT(SEXT(SLICE(X, 0, 7), 9), Y)) => CONCAT(SEXT(SLICE(X, 0, 7), 10), Y)
        {
            auto s1 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(0, d.size()), BooleanFunction::Index(7, d.size()), 8);
            auto s2 = BooleanFunction::Slice(d.clone(), BooleanFunction::Index(7, d.size()), BooleanFunction::Index(7, d.size()), 1);

            ASSERT_TRUE(s1.is_ok());
            ASSERT_TRUE(s2.is_ok());

            auto sext1 = BooleanFunction::Sext(s1.get(), BooleanFunction::Index(s1.get().size() + 1, s1.get().size() + 1), s1.get().size() + 1);
            auto sext2 = BooleanFunction::Sext(s1.get(), BooleanFunction::Index(s1.get().size() + 2, s1.get().size() + 2), s1.get().size() + 2);

            ASSERT_TRUE(sext1.is_ok());
            ASSERT_TRUE(sext2.is_ok());

            auto c1 = BooleanFunction::Concat(sext1.get(), e.clone(), sext1.get().size() + e.size());
            auto c2 = BooleanFunction::Concat(sext2.get(), e.clone(), sext2.get().size() + e.size());

            ASSERT_TRUE(c1.is_ok());
            ASSERT_TRUE(c2.is_ok());

            auto res = BooleanFunction::Concat(s2.get(), c1.get(), c1.get().size() + s2.get().size());

            ASSERT_TRUE(res.is_ok());

            EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf(res.get()).get()).get()).get(), c2.get());
        }

        ////////////////////////////////////////////////////////////////////////
        // GENERAL SIMPLIFICATION RULES
        ////////////////////////////////////////////////////////////////////////

        // (a & ~a) | (b & ~b)  =>   0
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & ~a) | (b & ~b)).get()).get()).get(), _0);
        // (a & b) | (~a & b)   =>   b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & b) | (~a & b)).get()).get()).get(), b);
        // (a & ~b) | (~a & ~b)  =>  ~b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & ~b) | (~a & ~b)).get()).get()).get(), ~b);

        // (a & b) | (~a & b) | (a & ~b) | (~a & ~b)   =>   1
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & b) | (~a & b) | (a & ~b) | (~a & ~b)).get()).get()).get(), _1);
        // (a | b) | (b & c)   => a | b
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a | b) | (b & c)).get()).get()).get(), a | b);
        // (a & c) | (b & ~c) | (a & b)   =>   (b | c) & (a | ~c)
        EXPECT_EQ(bitwuzla_utils::to_bf(bitwuzla_utils::simplify(bitwuzla_utils::from_bf((a & c) | (b & ~c) | (a & b)).get()).get()).get(), (b | c) & (a | ~c));
    }

    TEST(BooleanFunction, SimplificationPerformance)
    {
        const auto function =
            BooleanFunction::from_string(
                "((((((((((((((((((((((((((((((((0b0 | (((((I0 & (! I1)) & (! I2)) & (! I3)) & (! I4)) & (! I5))) | ((((((! I0) & I1) & (! I2)) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & I1) & (! "
                "I2)) & (! I3)) & (! I4)) & (! I5))) | ((((((! I0) & (! I1)) & I2) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & I1) & I2) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & (! I1)) & (! I2)) & "
                "I3) & (! I4)) & (! I5))) | (((((I0 & I1) & (! I2)) & I3) & (! I4)) & (! I5))) | ((((((! I0) & (! I1)) & I2) & I3) & (! I4)) & (! I5))) | (((((I0 & I1) & I2) & I3) & (! I4)) & (! "
                "I5))) | ((((((! I0) & (! I1)) & (! I2)) & (! I3)) & I4) & (! I5))) | (((((I0 & (! I1)) & (! I2)) & (! I3)) & I4) & (! I5))) | ((((((! I0) & (! I1)) & (! I2)) & I3) & I4) & (! I5))) "
                "| (((((I0 & (! I1)) & (! I2)) & I3) & I4) & (! I5))) | (((((I0 & I1) & (! I2)) & I3) & I4) & (! I5))) | ((((((! I0) & I1) & I2) & I3) & I4) & (! I5))) | ((((((! I0) & I1) & (! I2)) "
                "& (! I3)) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & I2) & (! I3)) & (! I4)) & I5)) | ((((((! I0) & I1) & I2) & (! I3)) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & (! I2)) & I3) & (! "
                "I4)) & I5)) | (((((I0 & (! I1)) & (! I2)) & I3) & (! I4)) & I5)) | (((((I0 & I1) & (! I2)) & I3) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & I2) & I3) & (! I4)) & I5)) | ((((((! I0) "
                "& (! I1)) & (! I2)) & (! I3)) & I4) & I5)) | (((((I0 & (! I1)) & (! I2)) & (! I3)) & I4) & I5)) | ((((((! I0) & I1) & (! I2)) & (! I3)) & I4) & I5)) | ((((((! I0) & I1) & I2) & (! "
                "I3)) & I4) & I5)) | (((((I0 & I1) & I2) & (! I3)) & I4) & I5)) | (((((I0 & (! I1)) & (! I2)) & I3) & I4) & I5)) | ((((((! I0) & I1) & (! I2)) & I3) & I4) & I5)) | ((((((! I0) & (! "
                "I1)) & I2) & I3) & I4) & I5)) | (((((I0 & (! I1)) & I2) & I3) & I4) & I5)) | (((((I0 & I1) & I2) & I3) & I4) & I5))")
                .get();
        auto bf_function      = bitwuzla_utils::from_bf(function).get();
        auto start      = std::chrono::system_clock::now();
        const auto simplified = bitwuzla_utils::simplify(bf_function);

        const auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
        start      = std::chrono::system_clock::now();
        const auto bf_simplified = function.simplify();        
        const auto bf_duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
        std::cout << "BW_simplification took "<< duration_in_seconds<< " seconds"<< std::endl;
        std::cout << "BF_simplification took "<< bf_duration_in_seconds<< " seconds"<< std::endl;
    }

#ifdef BITWUZLA_LIBRARY
    TEST(BooleanFunction, BitwuzlaTest)
    {
        const auto a = BooleanFunction::Var("a"), _1 = BooleanFunction::Const(1, 1);

        const auto boolean_function = a | _1;
        const auto constraint       = BooleanFunction::Eq(a, boolean_function, 1);

        auto s      = SMT::Solver();
        auto config = SMT::QueryConfig();
        auto s_type = SMT::SolverType::Bitwuzla;
        auto s_call = SMT::SolverCall::Library;
        config.with_solver(s_type).with_call(s_call).with_model_generation();
        auto result = s.with_constraint(SMT::Constraint(constraint.get())).query(config);

        ASSERT_TRUE(result.is_ok());
        auto solver_result = result.get();
        EXPECT_EQ(solver_result.type, SMT::SolverResultType::Sat);
    }
#endif

}    //namespace hal
