#include "test_def.h"
#include "gate_library_test_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
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
            // test BooleanFunctionDecorator::substitute_power_ground_nets()
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
        TEST_END
    }
}