#include "test_def.h"
#include "gate_library_test_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
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
     * Test NetlistModificationDecorator.
     */
    TEST_F(DecoratorTest, check_netlist_modification_decorator)
    {
        TEST_START
        {
            // test NetlistModificationDecorator::delete_modules
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

            EXPECT_TRUE(NetlistModificationDecorator(*(nl.get())).delete_modules([](const Module* m) { return m->get_name() == "m0"; }).is_ok());

            EXPECT_EQ(nl->get_modules().size(), 3);

            EXPECT_TRUE(NetlistModificationDecorator(*(nl.get())).delete_modules().is_ok());

            EXPECT_EQ(nl->get_modules().size(), 1);
        }
        {
            // test NetlistModificationDecorator::replace_gate
            auto nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            auto gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            GateType* and_type = gl->get_gate_type_by_name("AND2");
            ASSERT_NE(and_type, nullptr);
            GateType* or_type = gl->get_gate_type_by_name("OR2");
            ASSERT_NE(or_type, nullptr);
            GateType* xor_type = gl->get_gate_type_by_name("XOR2");
            ASSERT_NE(xor_type, nullptr);

            Gate* a0 = nl->create_gate(and_type, "A0");
            Gate* a1 = nl->create_gate(and_type, "A1");
            Gate* a2 = nl->create_gate(and_type, "A2");
            Gate* a3 = nl->create_gate(and_type, "A3");            
            Gate* a4 = nl->create_gate(and_type, "A4");
            Gate* a5 = nl->create_gate(and_type, "A5");
            Gate* a6 = nl->create_gate(or_type, "A6");
            Gate* a7 = nl->create_gate(or_type, "A7");

            ASSERT_NE(a0, nullptr);
            ASSERT_NE(a1, nullptr);
            ASSERT_NE(a2, nullptr);
            ASSERT_NE(a3, nullptr);
            ASSERT_NE(a4, nullptr);
            ASSERT_NE(a5, nullptr);
            ASSERT_NE(a6, nullptr);
            ASSERT_NE(a7, nullptr);

            ASSERT_NE(test_utils::connect_global_in(nl.get(), a0, "I0"), nullptr);
            ASSERT_NE(test_utils::connect_global_in(nl.get(), a0, "I1"), nullptr);
            ASSERT_NE(test_utils::connect_global_in(nl.get(), a1, "I0"), nullptr);
            ASSERT_NE(test_utils::connect_global_in(nl.get(), a1, "I1"), nullptr);
            ASSERT_NE(test_utils::connect_global_in(nl.get(), a2, "I0"), nullptr);
            ASSERT_NE(test_utils::connect_global_in(nl.get(), a2, "I1"), nullptr);
            Net* i0_net = test_utils::connect(nl.get(), a0, "O", a3, "I0");
            ASSERT_NE(i0_net, nullptr);
            Net* i1_net = test_utils::connect(nl.get(), a1, "O", a3, "I1");
            ASSERT_NE(i1_net, nullptr);
            ASSERT_NE(test_utils::connect(nl.get(), a0, "O", a4, "I0"), nullptr);
            ASSERT_NE(test_utils::connect(nl.get(), a2, "O", a4, "I1"), nullptr);
            ASSERT_NE(test_utils::connect(nl.get(), a1, "O", a5, "I0"), nullptr);
            Net* i2_net = test_utils::connect(nl.get(), a2, "O", a5, "I1");
            ASSERT_NE(i2_net, nullptr);
            Net* o0_net = test_utils::connect(nl.get(), a3, "O", a6, "I0");
            ASSERT_NE(o0_net, nullptr);
            Net* o1_net = test_utils::connect(nl.get(), a4, "O", a6, "I1");
            ASSERT_NE(o1_net, nullptr);
            ASSERT_NE(test_utils::connect(nl.get(), a4, "O", a7, "I0"), nullptr);
            Net* o2_net = test_utils::connect(nl.get(), a5, "O", a7, "I1");
            ASSERT_NE(o2_net, nullptr);
            ASSERT_NE(test_utils::connect_global_out(nl.get(), a6, "O"), nullptr);
            ASSERT_NE(test_utils::connect_global_out(nl.get(), a7, "O"), nullptr);

            auto mod = nl->create_module("mod", nl->get_top_module(), {a3, a4, a5});
            ASSERT_NE(mod, nullptr);

            ModulePin* i0 = mod->get_pin_by_net(i0_net);
            ASSERT_NE(i0, nullptr);
            ModulePin* i1 = mod->get_pin_by_net(i1_net);
            ASSERT_NE(i1, nullptr);
            ModulePin* i2 = mod->get_pin_by_net(i2_net);
            ASSERT_NE(i2, nullptr);
            ModulePin* o0 = mod->get_pin_by_net(o0_net);
            ASSERT_NE(o0, nullptr);
            ModulePin* o1 = mod->get_pin_by_net(o1_net);
            ASSERT_NE(o1, nullptr);
            ModulePin* o2 = mod->get_pin_by_net(o2_net);
            ASSERT_NE(o2, nullptr);

            EXPECT_TRUE(mod->set_pin_name(i0, "I0"));
            EXPECT_TRUE(mod->set_pin_name(i1, "I1"));
            EXPECT_TRUE(mod->set_pin_name(i2, "I2"));
            EXPECT_TRUE(mod->set_pin_name(o0, "O0"));
            EXPECT_TRUE(mod->set_pin_name(o1, "O1"));
            EXPECT_TRUE(mod->set_pin_name(o2, "O2"));

            EXPECT_TRUE(mod->create_pin_group("I", {i0, i1, i2}).is_ok());
            EXPECT_TRUE(mod->create_pin_group("O", {o0, o1, o2}).is_ok());

            EXPECT_EQ(nl->get_gates().size(), 8);
            EXPECT_EQ(nl->get_nets().size(), 14);
            EXPECT_EQ(nl->get_modules().size(), 2);


            std::map<GatePin*, GatePin*> pin_map;
            pin_map[and_type->get_pin_by_name("I0")] = xor_type->get_pin_by_name("I0");
            pin_map[and_type->get_pin_by_name("I1")] = xor_type->get_pin_by_name("I1");
            pin_map[and_type->get_pin_by_name("O")] = xor_type->get_pin_by_name("O");

            const auto replace_res = NetlistModificationDecorator(*(nl.get())).replace_gate(a4, xor_type, pin_map);
            ASSERT_TRUE(replace_res.is_ok());
            Gate* new_gate = replace_res.get();
            ASSERT_NE(new_gate, nullptr);

            EXPECT_EQ(new_gate->get_fan_in_net("I0"), i0_net);
            EXPECT_EQ(new_gate->get_fan_in_net("I1"), i2_net);
            EXPECT_EQ(new_gate->get_fan_out_net("O"), o1_net);

            i0 = mod->get_pin_by_net(i0_net);
            ASSERT_NE(i0, nullptr);
            EXPECT_EQ(i0->get_name(), "I0");
            EXPECT_EQ(i0->get_direction(), PinDirection::input);
            i1 = mod->get_pin_by_net(i1_net);
            ASSERT_NE(i1, nullptr);
            EXPECT_EQ(i1->get_name(), "I1");
            EXPECT_EQ(i1->get_direction(), PinDirection::input);
            i2 = mod->get_pin_by_net(i2_net);
            ASSERT_NE(i2, nullptr);
            EXPECT_EQ(i2->get_name(), "I2");
            EXPECT_EQ(i2->get_direction(), PinDirection::input);
            o0 = mod->get_pin_by_net(o0_net);
            ASSERT_NE(o0, nullptr);
            EXPECT_EQ(o0->get_name(), "O0");
            EXPECT_EQ(o0->get_direction(), PinDirection::output);
            o1 = mod->get_pin_by_net(o1_net);
            ASSERT_NE(o1, nullptr);
            EXPECT_EQ(o1->get_name(), "O1");
            EXPECT_EQ(o1->get_direction(), PinDirection::output);
            o2 = mod->get_pin_by_net(o2_net);
            ASSERT_NE(o2, nullptr);
            EXPECT_EQ(o2->get_name(), "O2");
            EXPECT_EQ(o2->get_direction(), PinDirection::output);

            PinGroup<ModulePin>* o_group = mod->get_pin_group_by_name("O");
            EXPECT_TRUE(o_group->contains_pin(o0));
            EXPECT_TRUE(o_group->contains_pin(o1));
            EXPECT_TRUE(o_group->contains_pin(o2));

            PinGroup<ModulePin>* i_group = mod->get_pin_group_by_name("I");
            EXPECT_TRUE(i_group->contains_pin(i0));
            EXPECT_TRUE(i_group->contains_pin(i1));
            EXPECT_TRUE(i_group->contains_pin(i2));
        }
        {
            // test NetlistModificationDecorator::connect_gates
            auto nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            auto gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            GateType* and_type = gl->get_gate_type_by_name("AND2");
            ASSERT_NE(and_type, nullptr);
            GatePin* i0_pin = and_type->get_pin_by_name("I0");
            ASSERT_NE(i0_pin, nullptr);
            GatePin* i1_pin = and_type->get_pin_by_name("I1");
            ASSERT_NE(i1_pin, nullptr);
            GatePin* o_pin = and_type->get_pin_by_name("O");
            ASSERT_NE(o_pin, nullptr);

            Gate* a0 = nl->create_gate(and_type, "A0");
            Gate* a1 = nl->create_gate(and_type, "A1");
            Gate* a2 = nl->create_gate(and_type, "A2");
            Gate* a3 = nl->create_gate(and_type, "A3");            

            ASSERT_NE(a0, nullptr);
            ASSERT_NE(a1, nullptr);
            ASSERT_NE(a2, nullptr);
            ASSERT_NE(a3, nullptr);

            auto nl_dec = NetlistModificationDecorator(*(nl.get()));

            // no connection from A0 exists
            const auto net0_res = nl_dec.connect_gates(a0, o_pin, a1, i0_pin);
            ASSERT_TRUE(net0_res.is_ok());  
            const Net* net0 = net0_res.get();
            ASSERT_NE(net0, nullptr);
            EXPECT_EQ(net0->get_num_of_sources(), 1);
            EXPECT_EQ(net0->get_num_of_destinations(), 1);
            const Endpoint* ep0 = net0->get_sources([a0](const Endpoint* ep) { return ep->get_gate() == a0; }).front();
            ASSERT_NE(ep0, nullptr);
            EXPECT_EQ(ep0->get_gate(), a0);
            EXPECT_EQ(ep0->get_pin(), o_pin);
            const Endpoint* ep1 = net0->get_destinations([a1](const Endpoint* ep) { return ep->get_gate() == a1; }).front();
            ASSERT_NE(ep1, nullptr);
            EXPECT_EQ(ep1->get_gate(), a1);
            EXPECT_EQ(ep1->get_pin(), i0_pin);

            // reuse net0 with A0 as source for connection of A2
            const auto net1_res = nl_dec.connect_gates(a0, o_pin, a2, i0_pin);
            ASSERT_TRUE(net1_res.is_ok());  
            const Net* net1 = net1_res.get();
            ASSERT_NE(net1, nullptr);
            EXPECT_EQ(net0, net1);
            EXPECT_EQ(net0->get_num_of_sources(), 1);
            EXPECT_EQ(net0->get_num_of_destinations(), 2);
            const Endpoint* ep2 = net0->get_sources([a0](const Endpoint* ep) { return ep->get_gate() == a0; }).front();
            ASSERT_NE(ep2, nullptr);
            EXPECT_EQ(ep2->get_gate(), a0);
            EXPECT_EQ(ep2->get_pin(), o_pin);
            const Endpoint* ep3 = net0->get_destinations([a2](const Endpoint* ep) { return ep->get_gate() == a2; }).front();
            ASSERT_NE(ep3, nullptr);
            EXPECT_EQ(ep3->get_gate(), a2);
            EXPECT_EQ(ep3->get_pin(), i0_pin);

            // no connection to A3 exists
            const auto net2_res = nl_dec.connect_gates(a1, o_pin, a3, i0_pin);
            ASSERT_TRUE(net2_res.is_ok());  
            const Net* net2 = net2_res.get();
            ASSERT_NE(net2, nullptr);
            EXPECT_EQ(net2->get_num_of_sources(), 1);
            EXPECT_EQ(net2->get_num_of_destinations(), 1);
            const Endpoint* ep4 = net2->get_sources([a1](const Endpoint* ep) { return ep->get_gate() == a1; }).front();
            ASSERT_NE(ep4, nullptr);
            EXPECT_EQ(ep4->get_gate(), a1);
            EXPECT_EQ(ep4->get_pin(), o_pin);
            const Endpoint* ep5 = net2->get_destinations([a3](const Endpoint* ep) { return ep->get_gate() == a3; }).front();
            ASSERT_NE(ep5, nullptr);
            EXPECT_EQ(ep5->get_gate(), a3);
            EXPECT_EQ(ep5->get_pin(), i0_pin);

            // reuse net2 with A3 as destination for connection of A2
            const auto net3_res = nl_dec.connect_gates(a2, o_pin, a3, i0_pin);
            ASSERT_TRUE(net3_res.is_ok());  
            const Net* net3 = net3_res.get();
            ASSERT_NE(net3, nullptr);
            EXPECT_EQ(net2, net3);
            EXPECT_EQ(net2->get_num_of_sources(), 2);
            EXPECT_EQ(net2->get_num_of_destinations(), 1);
            const Endpoint* ep6 = net2->get_sources([a2](const Endpoint* ep) { return ep->get_gate() == a2; }).front();
            ASSERT_NE(ep6, nullptr);
            EXPECT_EQ(ep6->get_gate(), a2);
            EXPECT_EQ(ep6->get_pin(), o_pin);
            const Endpoint* ep7 = net2->get_destinations([a3](const Endpoint* ep) { return ep->get_gate() == a3; }).front();
            ASSERT_NE(ep7, nullptr);
            EXPECT_EQ(ep7->get_gate(), a3);
            EXPECT_EQ(ep7->get_pin(), i0_pin);

            // try invalid connection as A0 already has fan-out and A3 already has fan-in
            const auto net4_res = nl_dec.connect_gates(a0, o_pin, a3, i0_pin);
            ASSERT_TRUE(net4_res.is_error());  
        }
        TEST_END
    }
}