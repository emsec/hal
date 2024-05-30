#include "test_def.h"
#include "gate_library_test_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
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

            EXPECT_EQ(nl->get_gates().size(), 8);
            EXPECT_EQ(nl->get_nets().size(), 14);
            EXPECT_EQ(nl->get_modules().size(), 2);

            EXPECT_EQ(new_gate->get_module(), mod);

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
        {
            // test NetlistModificationDecorator::connect_nets
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

            Net* net0 = test_utils::connect_global_in(nl.get(), a0, "I0");
            Net* net1 = test_utils::connect_global_in(nl.get(), a2, "I0");
            Net* net2 = test_utils::connect(nl.get(), a0, "O", a1, "I0");
            Net* net3 = test_utils::connect(nl.get(), a2, "O", a3, "I0");
            Net* net4 = test_utils::connect_global_out(nl.get(), a1, "O");
            Net* net5 = test_utils::connect_global_out(nl.get(), a3, "O");

            ASSERT_NE(net0, nullptr);
            ASSERT_NE(net1, nullptr);
            ASSERT_NE(net2, nullptr);
            ASSERT_NE(net3, nullptr);
            ASSERT_NE(net4, nullptr);
            ASSERT_NE(net5, nullptr);

            auto nl_dec = NetlistModificationDecorator(*(nl.get()));

            // merge global inputs
            const auto merged_net01_res = nl_dec.connect_nets(net0, net1);
            ASSERT_TRUE(merged_net01_res.is_ok());
            Net* m_net01 = merged_net01_res.get();
            ASSERT_NE(m_net01, nullptr);
            EXPECT_EQ(m_net01, net0);
            EXPECT_TRUE(m_net01->is_global_input_net());
            EXPECT_FALSE(m_net01->is_global_output_net());
            EXPECT_EQ(m_net01->get_num_of_sources(), 0);
            EXPECT_EQ(m_net01->get_num_of_destinations(), 2);
            EXPECT_TRUE(m_net01->is_a_destination(a0, "I0"));
            EXPECT_TRUE(m_net01->is_a_destination(a2, "I0"));

            // merge internal nets
            const auto merged_net23_res = nl_dec.connect_nets(net2, net3);
            ASSERT_TRUE(merged_net23_res.is_ok());
            Net* m_net23 = merged_net23_res.get();
            ASSERT_NE(m_net23, nullptr);
            EXPECT_EQ(m_net23, net2);
            EXPECT_FALSE(m_net23->is_global_input_net());
            EXPECT_FALSE(m_net23->is_global_output_net());
            EXPECT_EQ(m_net23->get_num_of_sources(), 2);
            EXPECT_EQ(m_net23->get_num_of_destinations(), 2);
            EXPECT_TRUE(m_net23->is_a_source(a0, "O"));
            EXPECT_TRUE(m_net23->is_a_source(a2, "O"));
            EXPECT_TRUE(m_net23->is_a_destination(a1, "I0"));
            EXPECT_TRUE(m_net23->is_a_destination(a3, "I0"));

            // merge global outputs
            const auto merged_net45_res = nl_dec.connect_nets(net4, net5);
            ASSERT_TRUE(merged_net45_res.is_ok());
            Net* m_net45 = merged_net45_res.get();
            ASSERT_NE(m_net45, nullptr);
            EXPECT_EQ(m_net45, net4);
            EXPECT_FALSE(m_net45->is_global_input_net());
            EXPECT_TRUE(m_net45->is_global_output_net());
            EXPECT_EQ(m_net45->get_num_of_sources(), 2);
            EXPECT_EQ(m_net45->get_num_of_destinations(), 0);
            EXPECT_TRUE(m_net45->is_a_source(a1, "O"));
            EXPECT_TRUE(m_net45->is_a_source(a3, "O"));
        }
        TEST_END
    }

    /**
     * Test NetlistTraversalDecorator.
     */
    TEST_F(DecoratorTest, check_netlist_traversal_decorator)
    {
        TEST_START
        {
            // setup test netlist
            auto nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            auto* nl_raw = nl.get();

            auto gl = nl_raw->get_gate_library();
            ASSERT_NE(gl, nullptr);

            auto dff0 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF0");
            auto dff1 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF1");
            auto dff2 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF2");
            auto dff3 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF3");
            auto dff4 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF4");
            auto dff5 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF5");
            auto dff6 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF6");
            auto dff7 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF7");
            auto dff8 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF8");
            auto dff9 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF9");
            auto dff10 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF10");
            auto dff11 = nl_raw->create_gate(gl->get_gate_type_by_name("DFFRE"), "DFF11");

            auto sff0 = nl_raw->create_gate(gl->get_gate_type_by_name("DFF"), "SFF0");
            auto sff1 = nl_raw->create_gate(gl->get_gate_type_by_name("DFF"), "SFF1");

            auto or0 = nl_raw->create_gate(gl->get_gate_type_by_name("OR2"), "OR0");
            auto or1 = nl_raw->create_gate(gl->get_gate_type_by_name("OR2"), "OR1");
            auto or2 = nl_raw->create_gate(gl->get_gate_type_by_name("OR2"), "OR2");
            auto or3 = nl_raw->create_gate(gl->get_gate_type_by_name("OR2"), "OR3");
            auto or4 = nl_raw->create_gate(gl->get_gate_type_by_name("OR2"), "OR4");
            auto or5 = nl_raw->create_gate(gl->get_gate_type_by_name("OR2"), "OR5");

            auto inv0 = nl_raw->create_gate(gl->get_gate_type_by_name("INV"), "INV0");
            auto inv1 = nl_raw->create_gate(gl->get_gate_type_by_name("INV"), "INV1");
            auto inv2 = nl_raw->create_gate(gl->get_gate_type_by_name("INV"), "INV2");
            auto inv3 = nl_raw->create_gate(gl->get_gate_type_by_name("INV"), "INV3");
            auto inv4 = nl_raw->create_gate(gl->get_gate_type_by_name("INV"), "INV4");
            auto inv5 = nl_raw->create_gate(gl->get_gate_type_by_name("INV"), "INV5");
            auto inv6 = nl_raw->create_gate(gl->get_gate_type_by_name("INV"), "INV6");

            auto and0 = nl_raw->create_gate(gl->get_gate_type_by_name("AND2"), "AND0");
            auto and1 = nl_raw->create_gate(gl->get_gate_type_by_name("AND2"), "AND1");
            auto and2 = nl_raw->create_gate(gl->get_gate_type_by_name("AND2"), "AND2");

            Net* clk = test_utils::connect_global_in(nl_raw, dff0, "CLK", "clk");
            test_utils::connect_global_in(nl_raw, dff1, "CLK");
            test_utils::connect_global_in(nl_raw, dff2, "CLK");
            test_utils::connect_global_in(nl_raw, dff3, "CLK");
            test_utils::connect_global_in(nl_raw, dff4, "CLK");
            test_utils::connect_global_in(nl_raw, dff5, "CLK");
            test_utils::connect_global_in(nl_raw, dff6, "CLK");
            test_utils::connect_global_in(nl_raw, dff7, "CLK");
            test_utils::connect_global_in(nl_raw, dff8, "CLK");
            test_utils::connect_global_in(nl_raw, dff9, "CLK");
            test_utils::connect_global_in(nl_raw, dff10, "CLK");
            test_utils::connect_global_in(nl_raw, dff11, "CLK");
            test_utils::connect_global_in(nl_raw, sff0, "CLK");
            test_utils::connect_global_in(nl_raw, sff1, "CLK");

            Net* net_0 = test_utils::connect(nl_raw, or2, "O", or0, "I0", "net_0");
            Net* in_0 = test_utils::connect_global_in(nl_raw, or0, "I1", "in_0");
            Net* net_1 = test_utils::connect(nl_raw, or0, "O", dff0, "D", "net_1");
            Net* in_1 = test_utils::connect_global_in(nl_raw, dff1, "D", "in_1");
            Net* in_2 = test_utils::connect_global_in(nl_raw, dff2, "D", "in_2");
            Net* in_3 = test_utils::connect_global_in(nl_raw, or1, "I0", "in_3");
            Net* net_2 = test_utils::connect(nl_raw, or5, "O", or1, "I1", "net_2");
            Net* net_3 = test_utils::connect(nl_raw, or1, "O", dff3, "D", "net_3");

            Net* net_4 = test_utils::connect(nl_raw, dff0, "Q", inv0, "I", "net_4");
            Net* net_5 = test_utils::connect(nl_raw, dff0, "Q", and0, "I0", "net_5");
            Net* net_6 = test_utils::connect(nl_raw, dff1, "Q", and0, "I1", "net_6");
            Net* net_7 = test_utils::connect(nl_raw, dff1, "Q", and1, "I0", "net_7");
            Net* net_8 = test_utils::connect(nl_raw, dff2, "Q", and1, "I1", "net_8");
            Net* net_9 = test_utils::connect(nl_raw, dff2, "Q", and2, "I0", "net_9");
            Net* net_10 = test_utils::connect(nl_raw, dff3, "Q", and2, "I1", "net_10");
            Net* net_11 = test_utils::connect(nl_raw, dff3, "Q", inv1, "I", "net_11");

            Net* net_12 = test_utils::connect(nl_raw, inv0, "O", or2, "I0", "net_12");
            Net* net_13 = test_utils::connect(nl_raw, and0, "O", or2, "I1", "net_13");
            Net* net_14 = test_utils::connect(nl_raw, and0, "O", or3, "I0", "net_14");
            Net* net_15 = test_utils::connect(nl_raw, and1, "O", or3, "I1", "net_15");
            Net* net_16 = test_utils::connect(nl_raw, and1, "O", or4, "I0", "net_16");
            Net* net_17 = test_utils::connect(nl_raw, and2, "O", or4, "I1", "net_17");
            Net* net_18 = test_utils::connect(nl_raw, and2, "O", or5, "I0", "net_18");
            Net* net_19 = test_utils::connect(nl_raw, inv1, "O", or5, "I1", "net_19");

            Net* net_20 = test_utils::connect(nl_raw, or2, "O", dff4, "D", "net_20");
            Net* net_21 = test_utils::connect(nl_raw, or3, "O", dff5, "D", "net_21");
            Net* net_22 = test_utils::connect(nl_raw, or4, "O", dff6, "D", "net_22");
            Net* net_23 = test_utils::connect(nl_raw, or5, "O", dff7, "D", "net_23");

            Net* net_24 = test_utils::connect(nl_raw, dff4, "Q", inv2, "I", "net_24");
            Net* net_25 = test_utils::connect(nl_raw, dff5, "Q", inv3, "I", "net_25");
            Net* net_26 = test_utils::connect(nl_raw, dff6, "Q", inv4, "I", "net_26");
            Net* net_27 = test_utils::connect(nl_raw, dff7, "Q", inv5, "I", "net_27");

            Net* net_28 = test_utils::connect(nl_raw, inv2, "O", dff8, "D", "net_28");
            Net* net_29 = test_utils::connect(nl_raw, inv3, "O", dff9, "D", "net_29");
            Net* net_30 = test_utils::connect(nl_raw, inv4, "O", dff10, "D", "net_30");
            Net* net_31 = test_utils::connect(nl_raw, inv5, "O", dff11, "D", "net_31");

            Net* out_0 = test_utils::connect_global_out(nl_raw, dff8, "Q", "out_0");
            Net* out_1 = test_utils::connect_global_out(nl_raw, dff9, "Q", "out_1");
            Net* out_2 = test_utils::connect_global_out(nl_raw, dff10, "Q", "out_2");
            Net* out_3 = test_utils::connect_global_out(nl_raw, dff11, "Q", "out_3");

            Net* in_4 = test_utils::connect_global_in(nl_raw, sff0, "D", "in_4");
            Net* net_32 = test_utils::connect(nl_raw, sff0, "Q", sff1, "D", "net_32");

            Net* en = test_utils::connect(nl_raw, sff0, "Q", dff0, "EN", "en");
            test_utils::connect(nl_raw, sff0, "Q", dff1, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff2, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff3, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff4, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff5, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff6, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff7, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff8, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff9, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff10, "EN");
            test_utils::connect(nl_raw, sff0, "Q", dff11, "EN");

            Net* irst = test_utils::connect(nl_raw, sff1, "Q", inv6, "I", "irst");
            Net* rst = test_utils::connect(nl_raw, inv6, "O", dff0, "R", "rst");
            test_utils::connect(nl_raw, inv6, "O", dff1, "R");
            test_utils::connect(nl_raw, inv6, "O", dff2, "R");
            test_utils::connect(nl_raw, inv6, "O", dff3, "R");
            test_utils::connect(nl_raw, inv6, "O", dff4, "R");
            test_utils::connect(nl_raw, inv6, "O", dff5, "R");
            test_utils::connect(nl_raw, inv6, "O", dff6, "R");
            test_utils::connect(nl_raw, inv6, "O", dff7, "R");
            test_utils::connect(nl_raw, inv6, "O", dff8, "R");
            test_utils::connect(nl_raw, inv6, "O", dff9, "R");
            test_utils::connect(nl_raw, inv6, "O", dff10, "R");
            test_utils::connect(nl_raw, inv6, "O", dff11, "R");


            {
                // test NetlistModificationDecorator::get_next_matching_gates
                const auto trav_dec = NetlistTraversalDecorator(*(nl.get()));

                // successors
                {
                    const auto res = trav_dec.get_next_matching_gates(dff1, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, nullptr, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff4, dff5, dff6}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates(dff1, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, [](const Endpoint* ep, u32 current_depth) { return ep->get_gate()->get_name() != "AND0"; }, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff5, dff6}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates(dff1, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, nullptr, [](const Endpoint* ep, u32 current_depth) { return current_depth < 4; });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff4, dff5, dff6}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_matching_gates(dff2, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, nullptr, nullptr, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({dff5, dff6, dff7, dff3}));
                    const auto res2 = trav_dec.get_next_matching_gates(dff3, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, nullptr, nullptr, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({dff6, dff7, dff3}));
                }

                // predecessors
                {
                    const auto res = trav_dec.get_next_matching_gates(dff5, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, nullptr, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2, sff0, sff1}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates(dff5, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, [](const Endpoint* ep, u32 current_depth) { return ep->get_pin()->get_type() == PinType::data || ep->get_pin()->get_type() == PinType::none; }, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_matching_gates(dff6, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, nullptr, nullptr, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({dff1, dff2, dff3, sff0, sff1}));
                    const auto res2 = trav_dec.get_next_matching_gates(dff7, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }, false, nullptr, nullptr, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({dff2, dff3, sff0, sff1}));
                }
            }
            {
                // test NetlistModificationDecorator::get_next_matching_gates_until
                const auto trav_dec = NetlistTraversalDecorator(*(nl.get()));

                // successors
                {
                    const auto res = trav_dec.get_next_matching_gates_until(dff1, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); }, false, nullptr, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({and0, or2, or0, or3, and1, or4}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until(dff1, true, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); }, true, nullptr, [](const Endpoint* ep, u32 current_depth) { return !ep->get_gate()->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({and0, or2, or0, or3, and1, or4}));
                }

                // predecessors
                {
                    const auto res = trav_dec.get_next_matching_gates_until(dff5, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); }, false, nullptr, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({or3, and0, and1, inv6}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until(dff4, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); }, false, nullptr, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({or2, inv0, and0, inv6}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until(dff5, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); }, true, nullptr, [](const Endpoint* ep, u32 current_depth) { return !ep->get_gate()->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({or3, and0, and1, inv6}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until(dff5, false, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); }, true, [](const Endpoint* ep, u32 current_depth) { return ep->get_pin()->get_type() == PinType::data || ep->get_pin()->get_type() == PinType::none; }, [](const Endpoint* ep, u32 current_depth) { return !ep->get_gate()->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({or3, and0, and1}));
                }
            }
            {
                // test NetlistModificationDecorator::get_next_matching_gates_until_depth
                const auto trav_dec = NetlistTraversalDecorator(*(nl.get()));

                // successors
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff1, true, 2, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff1, true, 3, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff4, dff5, dff6}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff1, true, 4, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff4, dff5, dff6}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff1, true, 5, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff4, dff5, dff6, dff8, dff9, dff10}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff1, true, 6, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff4, dff5, dff6, dff8, dff9, dff10}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff1, true, 100, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff4, dff5, dff6, dff8, dff9, dff10}));
                }
                // predecessors
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff5, false, 2, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({sff0, sff1}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff5, false, 3, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2, sff0, sff1}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff5, false, 4, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2, sff0, sff1}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff5, false, 5, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2, sff0, sff1}));
                }
                {
                    const auto res = trav_dec.get_next_matching_gates_until_depth(dff5, false, 100, [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); });
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2, sff0, sff1}));
                }
            }
            {
                // test NetlistModificationDecorator::get_next_sequential_gates
                const auto trav_dec = NetlistTraversalDecorator(*(nl.get()));

                // successors
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff0, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff4, dff5}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff2, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff5, dff6, dff7, dff3}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff4, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff8}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff8, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>());
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(sff0, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({sff1, dff0, dff1, dff2, dff3, dff4, dff5, dff6, dff7, dff8, dff9, dff10, dff11}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(sff0, true, {PinType::enable, PinType::reset}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({sff1}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(sff1, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2, dff3, dff4, dff5, dff6, dff7, dff8, dff9, dff10, dff11}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(sff1, true, {PinType::enable, PinType::reset}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_sequential_gates(dff1, true, {}, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({dff4, dff5, dff6, dff0}));

                    const auto res2 = trav_dec.get_next_sequential_gates(dff2, true, {}, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({dff5, dff6, dff7, dff3}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_sequential_gates(dff3, true, {}, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({dff6, dff7, dff3}));

                    const auto res2 = trav_dec.get_next_sequential_gates(dff2, true, {}, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({dff5, dff6, dff7, dff3}));
                }

                // predecessors
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff4, false, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, sff0, sff1}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff4, false, {PinType::enable, PinType::reset, PinType::clock}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff5, false, {PinType::enable, PinType::reset, PinType::clock}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1, dff2}));
                }
                {
                    const auto res = trav_dec.get_next_sequential_gates(dff0, false, {PinType::enable, PinType::reset, PinType::clock}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({dff0, dff1}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_sequential_gates(dff5, false, {}, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({dff0, dff1, dff2, sff0, sff1}));

                    const auto res2 = trav_dec.get_next_sequential_gates(dff6, false, {}, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({dff1, dff2, dff3, sff0, sff1}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_sequential_gates(dff5, false, {PinType::enable, PinType::reset, PinType::clock}, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({dff0, dff1, dff2}));

                    const auto res2 = trav_dec.get_next_sequential_gates(dff6, false, {PinType::enable, PinType::reset, PinType::clock}, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({dff1, dff2, dff3}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_sequential_gates(dff6, false, {PinType::enable, PinType::reset, PinType::clock}, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({dff1, dff2, dff3}));

                    const auto res2 = trav_dec.get_next_sequential_gates(dff7, false, {PinType::enable, PinType::reset, PinType::clock}, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({dff2, dff3}));
                }
            }
            {
                // test NetlistModificationDecorator::get_next_sequential_gates_map
                const auto trav_dec = NetlistTraversalDecorator(*(nl.get()));

                // successors
                {
                    std::map<Gate*, std::set<Gate*>> gt;
                    gt[dff0] = {dff4, dff5, dff0};
                    gt[dff1] = {dff4, dff5, dff6, dff0};
                    gt[dff2] = {dff5, dff6, dff7, dff3};
                    gt[dff3] = {dff6, dff7, dff3};
                    gt[dff4] = {dff8};
                    gt[dff5] = {dff9};
                    gt[dff6] = {dff10};
                    gt[dff7] = {dff11};
                    gt[dff8] = {};
                    gt[dff9] = {};
                    gt[dff10] = {};
                    gt[dff11] = {};
                    gt[sff0] = {sff1};
                    gt[sff1] = {};

                    const auto res = trav_dec.get_next_sequential_gates_map(true, {PinType::enable, PinType::reset, PinType::set, PinType::clock});
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), gt);
                }
                {
                    std::map<Gate*, std::set<Gate*>> gt;
                    gt[dff0] = {dff4, dff5, dff0};
                    gt[dff1] = {dff4, dff5, dff6, dff0};
                    gt[dff2] = {dff5, dff6, dff7, dff3};
                    gt[dff3] = {dff6, dff7, dff3};
                    gt[dff4] = {dff8};
                    gt[dff5] = {dff9};
                    gt[dff6] = {dff10};
                    gt[dff7] = {dff11};
                    gt[dff8] = {};
                    gt[dff9] = {};
                    gt[dff10] = {};
                    gt[dff11] = {};
                    gt[sff0] = {dff0, dff1, dff2, dff3, dff4, dff5, dff6, dff7, dff8, dff9, dff10, dff11, sff1};
                    gt[sff1] = {dff0, dff1, dff2, dff3, dff4, dff5, dff6, dff7, dff8, dff9, dff10, dff11};

                    const auto res = trav_dec.get_next_sequential_gates_map(true, {});
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), gt);
                }

                // predecessors
                {
                    std::map<Gate*, std::set<Gate*>> gt;
                    gt[dff0] = {dff0, dff1};
                    gt[dff1] = {};
                    gt[dff2] = {};
                    gt[dff3] = {dff3, dff2};
                    gt[dff4] = {dff0, dff1};
                    gt[dff5] = {dff0, dff1, dff2};
                    gt[dff6] = {dff1, dff2, dff3};
                    gt[dff7] = {dff2, dff3};
                    gt[dff8] = {dff4};
                    gt[dff9] = {dff5};
                    gt[dff10] = {dff6};
                    gt[dff11] = {dff7};
                    gt[sff0] = {};
                    gt[sff1] = {sff0};

                    const auto res = trav_dec.get_next_sequential_gates_map(false, {PinType::enable, PinType::reset, PinType::set, PinType::clock});
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), gt);
                }
                {
                    std::map<Gate*, std::set<Gate*>> gt;
                    gt[dff0] = {dff0, dff1, sff0, sff1};
                    gt[dff1] = {sff0, sff1};
                    gt[dff2] = {sff0, sff1};
                    gt[dff3] = {dff3, dff2, sff0, sff1};
                    gt[dff4] = {dff0, dff1, sff0, sff1};
                    gt[dff5] = {dff0, dff1, dff2, sff0, sff1};
                    gt[dff6] = {dff1, dff2, dff3, sff0, sff1};
                    gt[dff7] = {dff2, dff3, sff0, sff1};
                    gt[dff8] = {dff4, sff0, sff1};
                    gt[dff9] = {dff5, sff0, sff1};
                    gt[dff10] = {dff6, sff0, sff1};
                    gt[dff11] = {dff7, sff0, sff1};
                    gt[sff0] = {};
                    gt[sff1] = {sff0};

                    const auto res = trav_dec.get_next_sequential_gates_map(false, {});
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), gt);
                }
            }
            {
                // test NetlistModificationDecorator::get_next_combinational_gates
                const auto trav_dec = NetlistTraversalDecorator(*(nl.get()));

                // successors
                {
                    const auto res = trav_dec.get_next_combinational_gates(dff4, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({inv2}));
                }
                {
                    const auto res = trav_dec.get_next_combinational_gates(dff0, true, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({inv0, and0, or2, or3, or0}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_combinational_gates(dff1, true, {}, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({and0, or2, or3, and1, or4, or0}));

                    const auto res2 = trav_dec.get_next_combinational_gates(dff2, true, {}, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({and1, or3, or4, and2, or5, or1}));
                }

                // predecessors
                {
                    const auto res = trav_dec.get_next_combinational_gates(dff4, false, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({inv0, and0, or2, inv6}));
                }
                {
                    const auto res = trav_dec.get_next_combinational_gates(dff4, false, {PinType::enable, PinType::reset, PinType::clock, PinType::set}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({inv0, and0, or2}));
                }
                {
                    const auto res = trav_dec.get_next_combinational_gates(dff4, false, {PinType::none}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({}));
                }
                {
                    const auto res = trav_dec.get_next_combinational_gates(dff0, false, {}, nullptr);
                    EXPECT_TRUE(res.is_ok());
                    EXPECT_EQ(res.get(), std::set<Gate*>({inv0, and0, or2, or0, inv6}));
                }
                {
                    std::unordered_map<const Net*, std::set<Gate*>> cache;
                    const auto res1 = trav_dec.get_next_combinational_gates(dff5, false, {}, &cache);
                    EXPECT_TRUE(res1.is_ok());
                    EXPECT_EQ(res1.get(), std::set<Gate*>({or3, and0, and1, inv6}));

                    const auto res2 = trav_dec.get_next_combinational_gates(dff6, false, {}, &cache);
                    EXPECT_TRUE(res2.is_ok());
                    EXPECT_EQ(res2.get(), std::set<Gate*>({or4, and1, and2, inv6}));
                }
            }
        }
        TEST_END
    }
}