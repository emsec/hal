#include "test_def.h"
#include "gate_library_test_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/netlist_manipulation_decorator.h"
#include "netlist_test_utils.h"


#include "gtest/gtest.h"

namespace hal {

    class DecoratorTest : public ::testing::Test {
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
     * Test BooleanFunctionDecorator.
     */
    TEST_F(DecoratorTest, check_boolean_function_decorator)
    {
        TEST_START
        {
            // test BooleanFunctionDecorator::substitute_power_ground_nets(const Netlist*)
            std::unique_ptr<Netlist> nl_owner = test_utils::create_empty_netlist();
            auto* nl = nl_owner.get();
            ASSERT_NE(nl, nullptr);
            const auto* gl = nl->get_gate_library();

            auto* gnd = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            ASSERT_NE(gnd, nullptr);
            auto* vcc = nl->create_gate(gl->get_gate_type_by_name("VCC"), "vcc");
            ASSERT_NE(vcc, nullptr);
            auto* and0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "and0");
            ASSERT_NE(and0, nullptr);
            auto* or0 = nl->create_gate(gl->get_gate_type_by_name("OR2"), "or0");
            ASSERT_NE(or0, nullptr);
            auto* or1 = nl->create_gate(gl->get_gate_type_by_name("OR2"), "or1");
            ASSERT_NE(or1, nullptr);

            Net* n0 = test_utils::connect_global_in(nl, and0, "I0", "in_0");
            ASSERT_NE(n0, nullptr);
            Net* n1 = test_utils::connect(nl, vcc, "O", and0, "I1", "vcc_net");
            ASSERT_NE(n1, nullptr);
            Net* n2 = test_utils::connect(nl, and0, "O", or0, "I0", "internal_1");
            ASSERT_NE(n2, nullptr);
            Net* n3 = test_utils::connect_global_in(nl, or0, "I1", "in_1");
            ASSERT_NE(n3, nullptr);
            Net* n4 = test_utils::connect(nl, or0, "O", or1, "I0", "internal_2");
            ASSERT_NE(n4, nullptr);
            Net* n5 = test_utils::connect(nl, gnd, "O", or1, "I1", "gnd_net");
            ASSERT_NE(n5, nullptr);
            Net* n6 = test_utils::connect_global_out(nl, or1, "O", "out_0");
            ASSERT_NE(n6, nullptr);

            auto bf_res = BooleanFunction::from_string("((" + BooleanFunctionNetDecorator(*n0).get_boolean_variable_name() + " & " + BooleanFunctionNetDecorator(*n1).get_boolean_variable_name() + ") | " + BooleanFunctionNetDecorator(*n3).get_boolean_variable_name() + ") | " + BooleanFunctionNetDecorator(*n5).get_boolean_variable_name());
            ASSERT_TRUE(bf_res.is_ok());
            const auto bf_dec = BooleanFunctionDecorator(bf_res.get());
            auto subs_res = bf_dec.substitute_power_ground_nets(nl);
            ASSERT_TRUE(subs_res.is_ok());
            EXPECT_EQ(subs_res.get().get_variable_names(), std::set<std::string>({BooleanFunctionNetDecorator(*n0).get_boolean_variable_name(), BooleanFunctionNetDecorator(*n3).get_boolean_variable_name()}));
        }
        {
            // test BooleanFunctionDecorator::get_boolean_function_from(const std::vector<Net*>&, u32, bool)
            std::unique_ptr<Netlist> nl_owner = test_utils::create_empty_netlist();
            auto* nl = nl_owner.get();
            ASSERT_NE(nl, nullptr);
            const auto* gl = nl->get_gate_library();

            Net* n0 = nl->create_net(1, "n0");
            ASSERT_NE(n0, nullptr);
            Net* n1 = nl->create_net(2, "n1");
            ASSERT_NE(n1, nullptr);
            Net* n2 = nl->create_net(3, "n2");
            ASSERT_NE(n2, nullptr);

            auto bf_n0 = BooleanFunctionNetDecorator(*n0).get_boolean_variable();
            auto bf_n1 = BooleanFunctionNetDecorator(*n1).get_boolean_variable();
            auto bf_n2 = BooleanFunctionNetDecorator(*n2).get_boolean_variable();

            auto bf_concat = BooleanFunctionDecorator::get_boolean_function_from({n0, n1, n2});
            ASSERT_TRUE(bf_concat.is_ok());
            EXPECT_EQ(bf_concat.get(), BooleanFunction::Concat(BooleanFunction::Concat(bf_n0.clone(), bf_n1.clone(), 2).get(), bf_n2.clone(), 3).get());

            const auto bf_concat_zext = BooleanFunctionDecorator::get_boolean_function_from({n0, n1, n2}, 6, false);
            ASSERT_TRUE(bf_concat_zext.is_ok());
            EXPECT_EQ(bf_concat_zext.get(), BooleanFunction::Zext(BooleanFunction::Concat(BooleanFunction::Concat(bf_n0.clone(), bf_n1.clone(), 2).get(), bf_n2.clone(), 3).get(), BooleanFunction::Index(6, 6), 6).get());

            const auto bf_concat_sext = BooleanFunctionDecorator::get_boolean_function_from({n0, n1, n2}, 6, true);
            ASSERT_TRUE(bf_concat_sext.is_ok());
            EXPECT_EQ(bf_concat_sext.get(), BooleanFunction::Sext(BooleanFunction::Concat(BooleanFunction::Concat(bf_n0.clone(), bf_n1.clone(), 2).get(), bf_n2.clone(), 3).get(), BooleanFunction::Index(6, 6), 6).get());
        }
        {
            // test BooleanFunctionDecorator::get_boolean_function_from(const std::vector<BooleanFunction>&, u32, bool)
            auto bf_a = BooleanFunction::Var("A", 1);
            auto bf_b = BooleanFunction::Var("B", 1);
            auto bf_c = BooleanFunction::Var("C", 1);

            auto bf_concat = BooleanFunctionDecorator::get_boolean_function_from({bf_a, bf_b, bf_c});
            ASSERT_TRUE(bf_concat.is_ok());
            EXPECT_EQ(bf_concat.get(), BooleanFunction::Concat(BooleanFunction::Concat(bf_a.clone(), bf_b.clone(), 2).get(), bf_c.clone(), 3).get());

            const auto bf_concat_zext = BooleanFunctionDecorator::get_boolean_function_from({bf_a, bf_b, bf_c}, 6, false);
            ASSERT_TRUE(bf_concat_zext.is_ok());
            EXPECT_EQ(bf_concat_zext.get(), BooleanFunction::Zext(BooleanFunction::Concat(BooleanFunction::Concat(bf_a.clone(), bf_b.clone(), 2).get(), bf_c.clone(), 3).get(), BooleanFunction::Index(6, 6), 6).get());

            const auto bf_concat_sext = BooleanFunctionDecorator::get_boolean_function_from({bf_a, bf_b, bf_c}, 6, true);
            ASSERT_TRUE(bf_concat_sext.is_ok());
            EXPECT_EQ(bf_concat_sext.get(), BooleanFunction::Sext(BooleanFunction::Concat(BooleanFunction::Concat(bf_a.clone(), bf_b.clone(), 2).get(), bf_c.clone(), 3).get(), BooleanFunction::Index(6, 6), 6).get());
        }
        TEST_END
    }

    /**
     * Test BooleanFunctionDecorator.
     */
    TEST_F(DecoratorTest, check_netlist_manipulation_decorator)
    {
        TEST_START
        {
            // test NetlistManipulationDecorator::delete_modules()
            auto nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            auto gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            auto a0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A0");
            auto a1 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A1");
            auto a2 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A2");
            auto a3 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A3");            
            auto a4 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A4");
            auto a5 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A5");
            auto a6 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A6");
            auto a7 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "A7");

            ASSERT_NE(a0, nullptr);
            ASSERT_NE(a1, nullptr);
            ASSERT_NE(a2, nullptr);
            ASSERT_NE(a3, nullptr);
            ASSERT_NE(a4, nullptr);
            ASSERT_NE(a5, nullptr);
            ASSERT_NE(a6, nullptr);
            ASSERT_NE(a7, nullptr);

            auto m0 = nl->create_module("m0", nl->get_top_module(), {a0, a1, a2, a3});
            auto m1 = nl->create_module("m1", nl->get_top_module(), {a4, a5});
            auto m2 = nl->create_module("m2", m1, {a6, a7});

            ASSERT_NE(m0, nullptr);
            ASSERT_NE(m1, nullptr);
            ASSERT_NE(m2, nullptr);

            EXPECT_EQ(nl->get_gates().size(), 8);
            EXPECT_EQ(nl->get_modules().size(), 4);
            EXPECT_EQ(m0->get_parent_module(), nl->get_top_module());
            EXPECT_EQ(m1->get_parent_module(), nl->get_top_module());
            EXPECT_EQ(m2->get_parent_module(), m1);

            EXPECT_TRUE(NetlistManipulationDecorator(*(nl.get())).delete_modules([](const Module* m) { return m->get_name() == "m0"; }).is_ok());

            EXPECT_EQ(nl->get_modules().size(), 3);

            EXPECT_TRUE(NetlistManipulationDecorator(*(nl.get())).delete_modules().is_ok());

            EXPECT_EQ(nl->get_modules().size(), 1);
        }
        TEST_END
    }
}