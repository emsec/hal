#include "z3_utils/include/z3_utils.h"

#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"

#include <bitset>
#include <filesystem>

namespace hal
{

    class Z3UtilsTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
        }

        virtual void TearDown()
        {
            test_utils::remove_sandbox_directory();
        }
    };

    TEST_F(Z3UtilsTest, check_bf_translation)
    {
        TEST_START
        {
            auto ctx             = z3::context();
            const auto bit_sizes = std::vector({1, 2, 8, 16});

            for (const auto bits : bit_sizes)
            {
                auto a    = BooleanFunction::Var("A", bits);
                auto b    = BooleanFunction::Var("B", bits);
                auto c    = BooleanFunction::Var("C", bits);
                auto cond = BooleanFunction::Eq(a.clone(), b.clone(), 1).get();

                auto bf_not_res = BooleanFunction::Not(a.clone(), bits);
                ASSERT_TRUE(bf_not_res.is_ok());
                auto bf_not      = bf_not_res.get();
                auto z3_not      = z3_utils::from_bf(bf_not, ctx);
                auto bft_not_res = z3_utils::to_bf(z3_not);
                ASSERT_TRUE(bft_not_res.is_ok());
                auto bft_not = bft_not_res.get();
                EXPECT_EQ(bf_not, bft_not);

                auto bf_and_res = BooleanFunction::And(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_and_res.is_ok());
                auto bf_and      = bf_and_res.get();
                auto z3_and      = z3_utils::from_bf(bf_and, ctx);
                auto bft_and_res = z3_utils::to_bf(z3_and);
                ASSERT_TRUE(bft_and_res.is_ok());
                auto bft_and = bft_and_res.get();
                EXPECT_EQ(bf_and, bft_and);

                auto bf_or_res = BooleanFunction::Or(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_or_res.is_ok());
                auto bf_or      = bf_or_res.get();
                auto z3_or      = z3_utils::from_bf(bf_or, ctx);
                auto bft_or_res = z3_utils::to_bf(z3_or);
                ASSERT_TRUE(bft_or_res.is_ok());
                auto bft_or = bft_or_res.get();
                EXPECT_EQ(bf_or, bft_or);

                auto bf_xor_res = BooleanFunction::Xor(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_xor_res.is_ok());
                auto bf_xor      = bf_xor_res.get();
                auto z3_xor      = z3_utils::from_bf(bf_xor, ctx);
                auto bft_xor_res = z3_utils::to_bf(z3_xor);
                ASSERT_TRUE(bft_xor_res.is_ok());
                auto bft_xor = bft_xor_res.get();
                EXPECT_EQ(bf_xor, bft_xor);

                auto bf_add_res = BooleanFunction::Add(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_add_res.is_ok());
                auto bf_add      = bf_add_res.get();
                auto z3_add      = z3_utils::from_bf(bf_add, ctx);
                auto bft_add_res = z3_utils::to_bf(z3_add);
                ASSERT_TRUE(bft_add_res.is_ok());
                auto bft_add = bft_add_res.get();
                EXPECT_EQ(bf_add, bft_add);

                auto bf_sub_res = BooleanFunction::Sub(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_sub_res.is_ok());
                auto bf_sub      = bf_sub_res.get();
                auto z3_sub      = z3_utils::from_bf(bf_sub, ctx);
                auto bft_sub_res = z3_utils::to_bf(z3_sub);
                ASSERT_TRUE(bft_sub_res.is_ok());
                auto bft_sub = bft_sub_res.get();
                EXPECT_EQ(bf_sub, bft_sub);

                auto bf_mul_res = BooleanFunction::Mul(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_mul_res.is_ok());
                auto bf_mul      = bf_mul_res.get();
                auto z3_mul      = z3_utils::from_bf(bf_mul, ctx);
                auto bft_mul_res = z3_utils::to_bf(z3_mul);
                ASSERT_TRUE(bft_mul_res.is_ok());
                auto bft_mul = bft_mul_res.get();
                EXPECT_EQ(bf_mul, bft_mul);

                auto bf_sdiv_res = BooleanFunction::Sdiv(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_sdiv_res.is_ok());
                auto bf_sdiv      = bf_sdiv_res.get();
                auto z3_sdiv      = z3_utils::from_bf(bf_sdiv, ctx);
                auto bft_sdiv_res = z3_utils::to_bf(z3_sdiv);
                ASSERT_TRUE(bft_sdiv_res.is_ok());
                auto bft_sdiv = bft_sdiv_res.get();
                EXPECT_EQ(bf_sdiv, bft_sdiv);

                auto bf_udiv_res = BooleanFunction::Udiv(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_udiv_res.is_ok());
                auto bf_udiv      = bf_udiv_res.get();
                auto z3_udiv      = z3_utils::from_bf(bf_udiv, ctx);
                auto bft_udiv_res = z3_utils::to_bf(z3_udiv);
                ASSERT_TRUE(bft_udiv_res.is_ok());
                auto bft_udiv = bft_udiv_res.get();
                EXPECT_EQ(bf_udiv, bft_udiv);

                auto bf_srem_res = BooleanFunction::Srem(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_srem_res.is_ok());
                auto bf_srem      = bf_srem_res.get();
                auto z3_srem      = z3_utils::from_bf(bf_srem, ctx);
                auto bft_srem_res = z3_utils::to_bf(z3_srem);
                ASSERT_TRUE(bft_srem_res.is_ok());
                auto bft_srem = bft_srem_res.get();
                EXPECT_EQ(bf_srem, bft_srem);

                auto bf_urem_res = BooleanFunction::Urem(a.clone(), b.clone(), bits);
                ASSERT_TRUE(bf_urem_res.is_ok());
                auto bf_urem      = bf_urem_res.get();
                auto z3_urem      = z3_utils::from_bf(bf_urem, ctx);
                auto bft_urem_res = z3_utils::to_bf(z3_urem);
                ASSERT_TRUE(bft_urem_res.is_ok());
                auto bft_urem = bft_urem_res.get();
                EXPECT_EQ(bf_urem, bft_urem);

                auto bf_concat_res = BooleanFunction::Concat(a.clone(), b.clone(), 2 * bits);
                ASSERT_TRUE(bf_concat_res.is_ok());
                auto bf_concat      = bf_concat_res.get();
                auto z3_concat      = z3_utils::from_bf(bf_concat, ctx);
                auto bft_concat_res = z3_utils::to_bf(z3_concat);
                ASSERT_TRUE(bft_concat_res.is_ok());
                auto bft_concat = bft_concat_res.get();
                EXPECT_EQ(bf_concat, bft_concat);

                auto bf_eq_res = BooleanFunction::Eq(a.clone(), b.clone(), 1);
                ASSERT_TRUE(bf_eq_res.is_ok());
                auto bf_eq      = bf_eq_res.get();
                auto z3_eq      = z3_utils::from_bf(bf_eq, ctx);
                auto bft_eq_res = z3_utils::to_bf(z3_eq);
                ASSERT_TRUE(bft_eq_res.is_ok());
                auto bft_eq = bft_eq_res.get();
                EXPECT_EQ(bf_eq, bft_eq);

                auto bf_sle_res = BooleanFunction::Sle(a.clone(), b.clone(), 1);
                ASSERT_TRUE(bf_sle_res.is_ok());
                auto bf_sle      = bf_sle_res.get();
                auto z3_sle      = z3_utils::from_bf(bf_sle, ctx);
                auto bft_sle_res = z3_utils::to_bf(z3_sle);
                ASSERT_TRUE(bft_sle_res.is_ok());
                auto bft_sle = bft_sle_res.get();
                EXPECT_EQ(bf_sle, bft_sle);

                auto bf_slt_res = BooleanFunction::Slt(a.clone(), b.clone(), 1);
                ASSERT_TRUE(bf_slt_res.is_ok());
                auto bf_slt      = bf_slt_res.get();
                auto z3_slt      = z3_utils::from_bf(bf_slt, ctx);
                auto bft_slt_res = z3_utils::to_bf(z3_slt);
                ASSERT_TRUE(bft_slt_res.is_ok());
                auto bft_slt = bft_slt_res.get();
                EXPECT_EQ(bf_slt, bft_slt);

                auto bf_ule_res = BooleanFunction::Ule(a.clone(), b.clone(), 1);
                ASSERT_TRUE(bf_ule_res.is_ok());
                auto bf_ule      = bf_ule_res.get();
                auto z3_ule      = z3_utils::from_bf(bf_ule, ctx);
                auto bft_ule_res = z3_utils::to_bf(z3_ule);
                ASSERT_TRUE(bft_ule_res.is_ok());
                auto bft_ule = bft_ule_res.get();
                EXPECT_EQ(bf_ule, bft_ule);

                auto bf_ult_res = BooleanFunction::Ult(a.clone(), b.clone(), 1);
                ASSERT_TRUE(bf_ult_res.is_ok());
                auto bf_ult      = bf_ult_res.get();
                auto z3_ult      = z3_utils::from_bf(bf_ult, ctx);
                auto bft_ult_res = z3_utils::to_bf(z3_ult);
                ASSERT_TRUE(bft_ult_res.is_ok());
                auto bft_ult = bft_ult_res.get();
                EXPECT_EQ(bf_ult, bft_ult);

                auto bf_ite_res = BooleanFunction::Ite(cond.clone(), b.clone(), c.clone(), bits);
                ASSERT_TRUE(bf_ite_res.is_ok());
                auto bf_ite      = bf_ite_res.get();
                auto z3_ite      = z3_utils::from_bf(bf_ite, ctx);
                auto bft_ite_res = z3_utils::to_bf(z3_ite);
                ASSERT_TRUE(bft_ite_res.is_ok());
                auto bft_ite = bft_ite_res.get();
                EXPECT_EQ(bf_ite, bft_ite);
            }
        }
        {
            auto ctx = z3::context();
            auto a   = BooleanFunction::Var("A", 16);
            auto i0  = BooleanFunction::Index(3, 16);
            auto i1  = BooleanFunction::Index(6, 16);

            auto bf_slice_res = BooleanFunction::Slice(a.clone(), i0.clone(), i1.clone(), 4);
            ASSERT_TRUE(bf_slice_res.is_ok());
            auto bf_slice      = bf_slice_res.get();
            auto z3_slice      = z3_utils::from_bf(bf_slice, ctx);
            auto bft_slice_res = z3_utils::to_bf(z3_slice);
            ASSERT_TRUE(bft_slice_res.is_ok());
            auto bft_slice = bft_slice_res.get();
            EXPECT_EQ(bf_slice, bft_slice);
        }
        {
            auto ctx      = z3::context();
            auto a        = BooleanFunction::Var("A", 5);
            auto ext_size = BooleanFunction::Index(16, 16);

            auto bf_zext_res = BooleanFunction::Zext(a.clone(), ext_size.clone(), 16);
            ASSERT_TRUE(bf_zext_res.is_ok());
            auto bf_zext      = bf_zext_res.get();
            auto z3_zext      = z3_utils::from_bf(bf_zext, ctx);
            auto bft_zext_res = z3_utils::to_bf(z3_zext);
            ASSERT_TRUE(bft_zext_res.is_ok());
            auto bft_zext = bft_zext_res.get();
            EXPECT_EQ(bf_zext, bft_zext);

            auto bf_sext_res = BooleanFunction::Sext(a.clone(), ext_size.clone(), 16);
            ASSERT_TRUE(bf_sext_res.is_ok());
            auto bf_sext      = bf_sext_res.get();
            auto z3_sext      = z3_utils::from_bf(bf_sext, ctx);
            auto bft_sext_res = z3_utils::to_bf(z3_sext);
            ASSERT_TRUE(bft_sext_res.is_ok());
            auto bft_sext = bft_sext_res.get();
            EXPECT_EQ(bf_sext, bft_sext);
        }
        {
            auto ctx      = z3::context();
            auto a        = BooleanFunction::Var("A", 16);
            auto num_bits = BooleanFunction::Index(5, 16);

            auto bf_shl_res = BooleanFunction::Shl(a.clone(), num_bits.clone(), 16);
            ASSERT_TRUE(bf_shl_res.is_ok());
            auto bf_shl      = bf_shl_res.get();
            auto z3_shl      = z3_utils::from_bf(bf_shl, ctx);
            auto bft_shl_res = z3_utils::to_bf(z3_shl);
            ASSERT_TRUE(bft_shl_res.is_ok());
            auto bft_shl = bft_shl_res.get();
            EXPECT_EQ(bf_shl, bft_shl);

            auto bf_lshr_res = BooleanFunction::Lshr(a.clone(), num_bits.clone(), 16);
            ASSERT_TRUE(bf_lshr_res.is_ok());
            auto bf_lshr      = bf_lshr_res.get();
            auto z3_lshr      = z3_utils::from_bf(bf_lshr, ctx);
            auto bft_lshr_res = z3_utils::to_bf(z3_lshr);
            ASSERT_TRUE(bft_lshr_res.is_ok());
            auto bft_lshr = bft_lshr_res.get();
            EXPECT_EQ(bf_lshr, bft_lshr);

            auto bf_ashr_res = BooleanFunction::Ashr(a.clone(), num_bits.clone(), 16);
            ASSERT_TRUE(bf_ashr_res.is_ok());
            auto bf_ashr      = bf_ashr_res.get();
            auto z3_ashr      = z3_utils::from_bf(bf_ashr, ctx);
            auto bft_ashr_res = z3_utils::to_bf(z3_ashr);
            ASSERT_TRUE(bft_ashr_res.is_ok());
            auto bft_ashr = bft_ashr_res.get();
            EXPECT_EQ(bf_ashr, bft_ashr);

            auto bf_rol_res = BooleanFunction::Rol(a.clone(), num_bits.clone(), 16);
            ASSERT_TRUE(bf_rol_res.is_ok());
            auto bf_rol      = bf_rol_res.get();
            auto z3_rol      = z3_utils::from_bf(bf_rol, ctx);
            auto bft_rol_res = z3_utils::to_bf(z3_rol);
            ASSERT_TRUE(bft_rol_res.is_ok());
            auto bft_rol = bft_rol_res.get();
            EXPECT_EQ(bf_rol, bft_rol);

            auto bf_ror_res = BooleanFunction::Ror(a.clone(), num_bits.clone(), 16);
            ASSERT_TRUE(bf_ror_res.is_ok());
            auto bf_ror      = bf_ror_res.get();
            auto z3_ror      = z3_utils::from_bf(bf_ror, ctx);
            auto bft_ror_res = z3_utils::to_bf(z3_ror);
            ASSERT_TRUE(bft_ror_res.is_ok());
            auto bft_ror = bft_ror_res.get();
            EXPECT_EQ(bf_ror, bft_ror);
        }

        TEST_END
    }
}    // namespace hal
