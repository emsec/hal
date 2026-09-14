#include "gate_library_test_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_ir/instantiate.h"
#include "hal_core/netlist/netlist_ir/netlist_ir.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace netlist_ir;

    class NetlistIRInstantiateTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }

        Gate* gate_by_name(const Netlist* nl, const std::string& name)
        {
            const auto gates = nl->get_gates([&name](const Gate* g) { return g->get_name() == name; });
            return gates.size() == 1 ? gates.front() : nullptr;
        }

        Net* net_by_name(const Netlist* nl, const std::string& name)
        {
            const auto nets = nl->get_nets([&name](const Net* n) { return n->get_name() == name; });
            return nets.size() == 1 ? nets.front() : nullptr;
        }

        hal::Module* module_by_name(const Netlist* nl, const std::string& name)
        {
            const auto modules = nl->get_modules([&name](const hal::Module* m) { return m->get_name() == name; });
            return modules.size() == 1 ? modules.front() : nullptr;
        }

        /**
         * top(a, b, y): sub s0 and sub s1, each an AND2, plus an AND2 g on top combining both; an alias
         * `assign n = s1_out;` and one sub input tied to 1'b1.
         */
        Design create_design()
        {
            Design d;
            d.source = "hier.v";

            netlist_ir::Module& sub = d.add_module("sub");
            Port& i                 = sub.add_port("i", PinDirection::input, {{1, 0}});
            Port& o                 = sub.add_port("o", PinDirection::output);
            Instance& g             = sub.add_instance("g", "AND2", InstanceKind::Gate);
            g.add_connection("I0", {i.bits.at(1)});    // i[0]
            g.add_connection("I1", {i.bits.at(0)});    // i[1]
            g.add_connection("O", {o.bits.front()});

            netlist_ir::Module& top = d.add_module("top");
            Port& a                 = top.add_port("a", PinDirection::input);
            Port& b                 = top.add_port("b", PinDirection::input);
            Port& y                 = top.add_port("y", PinDirection::output);
            Signal& m               = top.add_signal("m", {{1, 0}});
            Signal& n               = top.add_signal("n");

            Instance& s0 = top.add_instance("s0", "sub", InstanceKind::Module);
            s0.add_connection("i", {a.bits.front(), b.bits.front()});    // {a, b}
            s0.add_connection("o", {m.bits.at(1)});                      // m[0]

            Instance& s1 = top.add_instance("s1", "sub", InstanceKind::Module);
            s1.add_connection("i", {a.bits.front(), ONE});    // {a, 1'b1}
            s1.add_connection("o", {m.bits.at(0)});           // m[1]

            Instance& g2 = top.add_instance("g", "AND2", InstanceKind::Gate);
            g2.add_connection("I0", {m.bits.at(1)});
            g2.add_connection("I1", {n.bits.front()});
            g2.add_connection("O", {y.bits.front()});

            top.add_alias(n.bits.front(), m.bits.at(0));    // assign n = m[1];

            return d;
        }
    };

    /**
     * Testing a two-level hierarchy: aliases resolved across module boundaries, a constant on a module port, module
     * types and pins, global inputs and outputs, and the legacy naming of gates that share a name.
     *
     * Functions: instantiate
     */
    TEST_F(NetlistIRInstantiateTest, check_hierarchy)
    {
        TEST_START
        auto res = instantiate(create_design(), test_utils::get_gate_library());
        ASSERT_TRUE(res.is_ok()) << res.get_error().get();
        auto nl = res.get();

        EXPECT_EQ(nl->get_design_name(), "top");
        EXPECT_EQ(nl->get_top_module()->get_name(), "top_module");
        EXPECT_EQ(nl->get_top_module()->get_type(), "top");
        ASSERT_EQ(nl->get_top_module()->get_submodules().size(), 2);

        hal::Module* s0 = module_by_name(nl.get(), "s0");
        hal::Module* s1 = module_by_name(nl.get(), "s1");
        ASSERT_NE(s0, nullptr);
        ASSERT_NE(s1, nullptr);
        EXPECT_EQ(s0->get_type(), "sub");

        // three AND2 gates plus the VCC gate for the constant; the three gates named 'g' are prefixed with the name of
        // the module they belong to, the top module included
        EXPECT_EQ(nl->get_gates().size(), 4);
        Gate* g    = gate_by_name(nl.get(), "top_module/g");
        Gate* g_s0 = gate_by_name(nl.get(), "s0/g");
        Gate* g_s1 = gate_by_name(nl.get(), "s1/g");
        ASSERT_NE(g, nullptr);
        ASSERT_NE(g_s0, nullptr);
        ASSERT_NE(g_s1, nullptr);
        EXPECT_EQ(g->get_module(), nl->get_top_module());
        EXPECT_EQ(g_s0->get_module(), s0);
        EXPECT_EQ(g_s1->get_module(), s1);

        // a and b reach the submodule gates directly: no net is duplicated at the module boundary
        Net* a = net_by_name(nl.get(), "a");
        Net* b = net_by_name(nl.get(), "b");
        Net* y = net_by_name(nl.get(), "y");
        ASSERT_NE(a, nullptr);
        ASSERT_NE(b, nullptr);
        ASSERT_NE(y, nullptr);
        EXPECT_TRUE(nl->is_global_input_net(a));
        EXPECT_TRUE(nl->is_global_input_net(b));
        EXPECT_TRUE(nl->is_global_output_net(y));
        EXPECT_EQ(g_s0->get_fan_in_net("I0"), b);    // i[0] = b
        EXPECT_EQ(g_s0->get_fan_in_net("I1"), a);    // i[1] = a
        EXPECT_EQ(g_s1->get_fan_in_net("I1"), a);
        EXPECT_EQ(g->get_fan_out_net("O"), y);

        // the constant reaches through the module port and gets a VCC gate
        ASSERT_NE(g_s1->get_fan_in_net("I0"), nullptr);
        EXPECT_TRUE(g_s1->get_fan_in_net("I0")->is_vcc_net());
        EXPECT_EQ(nl->get_vcc_gates().size(), 1);
        EXPECT_TRUE(nl->get_gnd_gates().empty());

        // the alias `assign n = m[1]` merges n with m[1]: one net, named after the receiving side of the assignment
        EXPECT_EQ(g->get_fan_in_net("I0"), g_s0->get_fan_out_net("O"));
        EXPECT_EQ(g->get_fan_in_net("I1"), g_s1->get_fan_out_net("O"));
        EXPECT_EQ(g->get_fan_in_net("I0")->get_name(), "m(0)");
        EXPECT_EQ(g->get_fan_in_net("I1")->get_name(), "n");
        EXPECT_EQ(nl->get_nets().size(), 6);    // a, b, y, m(0), n, '1'

        // module pins carry the port names of the design
        ASSERT_EQ(s0->get_pins().size(), 3);
        EXPECT_EQ(s0->get_pin_by_net(a)->get_name(), "i(1)");
        EXPECT_EQ(s0->get_pin_by_net(b)->get_name(), "i(0)");
        EXPECT_EQ(s0->get_pin_by_net(b)->get_direction(), PinDirection::input);
        EXPECT_EQ(s0->get_pin_by_net(g_s0->get_fan_out_net("O"))->get_name(), "o");
        EXPECT_EQ(s0->get_pin_by_net(g_s0->get_fan_out_net("O"))->get_direction(), PinDirection::output);
        TEST_END
    }

    /**
     * Testing connections to pin groups: expression order against pin indices, a narrower signal, a narrower constant
     * that is zero-extended, a wider expression, a slice of a pin group, positional connections, and a single pin
     * addressed by name.
     *
     * Functions: instantiate
     */
    TEST_F(NetlistIRInstantiateTest, check_pin_groups)
    {
        TEST_START
        Design d;
        netlist_ir::Module& top = d.add_module("top");
        Port& a                 = top.add_port("a", PinDirection::input, {{3, 0}});
        Port& n                 = top.add_port("n", PinDirection::input, {{2, 0}});
        Port& w                 = top.add_port("w", PinDirection::input, {{5, 0}});
        Port& y0                = top.add_port("y0", PinDirection::output, {{3, 0}});
        Port& y1                = top.add_port("y1", PinDirection::output, {{3, 0}});
        Port& y2                = top.add_port("y2", PinDirection::output, {{3, 0}});
        Port& y3                = top.add_port("y3", PinDirection::output, {{3, 0}});
        Port& clk               = top.add_port("clk", PinDirection::input);

        // full width: a[3] is the first bit and lands on ADDR(3)
        Instance& r0 = top.add_instance("r0", "RAM", InstanceKind::Gate);
        r0.add_connection("ADDR", a.bits);
        r0.add_connection("DATA_IN", n.bits);    // narrower signal: DATA_IN(3) stays open
        r0.add_connection("DATA_OUT", y0.bits);
        r0.add_connection("CLK", clk.bits);

        // narrower constant 2'b10 is zero-extended, wider signal keeps its low bits
        Instance& r1 = top.add_instance("r1", "RAM", InstanceKind::Gate);
        r1.add_connection("ADDR", {ONE, ZERO});
        r1.add_connection("DATA_IN", w.bits);
        r1.add_connection("DATA_OUT", y1.bits);

        // a slice of a pin group, in the slice's order
        Instance& r2 = top.add_instance("r2", "RAM", InstanceKind::Gate);
        r2.add_connection("DATA_OUT", {y2.bits.at(3), y2.bits.at(2)}, Range{1, 2});    // DATA_OUT(1) = y2[0], DATA_OUT(2) = y2[1]

        // positional connections follow the pin group order of the library: CLK, EN, ADDR, DATA_IN, DATA_OUT
        Instance& r3 = top.add_instance("r3", "RAM", InstanceKind::Gate);
        r3.add_connection("", clk.bits);
        r3.add_connection("", {ONE});
        r3.add_connection("", a.bits);
        r3.add_connection("", {});
        r3.add_connection("", y3.bits);

        // a single pin addressed by its own name
        Instance& r4 = top.add_instance("r4", "RAM", InstanceKind::Gate);
        r4.add_connection("ADDR(2)", {a.bits.at(0)});

        // open positions: 4'b1x0z leaves ADDR(0) and ADDR(2) unconnected, {a[0], OPEN} on a pin group too
        Instance& r5 = top.add_instance("r5", "RAM", InstanceKind::Gate);
        r5.add_connection("ADDR", {ONE, OPEN, ZERO, OPEN});
        r5.add_connection("DATA_IN", {a.bits.at(3), OPEN});

        auto res = instantiate(d, test_utils::get_gate_library());
        ASSERT_TRUE(res.is_ok()) << res.get_error().get();
        auto nl = res.get();

        Gate* g0 = gate_by_name(nl.get(), "r0");
        ASSERT_NE(g0, nullptr);
        for (u32 i = 0; i < 4; i++)
        {
            ASSERT_NE(g0->get_fan_in_net("ADDR(" + std::to_string(i) + ")"), nullptr);
            EXPECT_EQ(g0->get_fan_in_net("ADDR(" + std::to_string(i) + ")")->get_name(), "a(" + std::to_string(i) + ")");
            ASSERT_NE(g0->get_fan_out_net("DATA_OUT(" + std::to_string(i) + ")"), nullptr);
            EXPECT_EQ(g0->get_fan_out_net("DATA_OUT(" + std::to_string(i) + ")")->get_name(), "y0(" + std::to_string(i) + ")");
        }
        EXPECT_EQ(g0->get_fan_in_net("DATA_IN(0)")->get_name(), "n(0)");
        EXPECT_EQ(g0->get_fan_in_net("DATA_IN(2)")->get_name(), "n(2)");
        EXPECT_EQ(g0->get_fan_in_net("DATA_IN(3)"), nullptr);
        EXPECT_EQ(g0->get_fan_in_net("CLK")->get_name(), "clk");

        Gate* g1 = gate_by_name(nl.get(), "r1");
        ASSERT_NE(g1, nullptr);
        EXPECT_TRUE(g1->get_fan_in_net("ADDR(0)")->is_gnd_net());
        EXPECT_TRUE(g1->get_fan_in_net("ADDR(1)")->is_vcc_net());
        EXPECT_TRUE(g1->get_fan_in_net("ADDR(2)")->is_gnd_net());
        EXPECT_TRUE(g1->get_fan_in_net("ADDR(3)")->is_gnd_net());
        EXPECT_EQ(g1->get_fan_in_net("DATA_IN(0)")->get_name(), "w(0)");
        EXPECT_EQ(g1->get_fan_in_net("DATA_IN(3)")->get_name(), "w(3)");
        EXPECT_NE(net_by_name(nl.get(), "w(4)"), nullptr);    // the dropped high bits are still top ports, so their nets exist
        EXPECT_NE(net_by_name(nl.get(), "w(5)"), nullptr);
        EXPECT_EQ(net_by_name(nl.get(), "w(4)")->get_num_of_destinations(), 0);

        Gate* g2 = gate_by_name(nl.get(), "r2");
        ASSERT_NE(g2, nullptr);
        EXPECT_EQ(g2->get_fan_out_net("DATA_OUT(0)"), nullptr);
        EXPECT_EQ(g2->get_fan_out_net("DATA_OUT(1)")->get_name(), "y2(0)");
        EXPECT_EQ(g2->get_fan_out_net("DATA_OUT(2)")->get_name(), "y2(1)");
        EXPECT_EQ(g2->get_fan_out_net("DATA_OUT(3)"), nullptr);

        Gate* g3 = gate_by_name(nl.get(), "r3");
        ASSERT_NE(g3, nullptr);
        EXPECT_EQ(g3->get_fan_in_net("CLK")->get_name(), "clk");
        EXPECT_TRUE(g3->get_fan_in_net("EN")->is_vcc_net());
        EXPECT_EQ(g3->get_fan_in_net("ADDR(3)")->get_name(), "a(3)");
        EXPECT_EQ(g3->get_fan_in_net("DATA_IN(0)"), nullptr);
        EXPECT_EQ(g3->get_fan_out_net("DATA_OUT(0)")->get_name(), "y3(0)");

        Gate* g4 = gate_by_name(nl.get(), "r4");
        ASSERT_NE(g4, nullptr);
        EXPECT_EQ(g4->get_fan_in_net("ADDR(2)")->get_name(), "a(3)");
        EXPECT_EQ(g4->get_fan_in_nets().size(), 1);

        Gate* g5 = gate_by_name(nl.get(), "r5");
        ASSERT_NE(g5, nullptr);
        EXPECT_EQ(g5->get_fan_in_net("ADDR(0)"), nullptr);
        EXPECT_TRUE(g5->get_fan_in_net("ADDR(1)")->is_gnd_net());
        EXPECT_EQ(g5->get_fan_in_net("ADDR(2)"), nullptr);
        EXPECT_TRUE(g5->get_fan_in_net("ADDR(3)")->is_vcc_net());
        EXPECT_EQ(g5->get_fan_in_net("DATA_IN(0)"), nullptr);
        EXPECT_EQ(g5->get_fan_in_net("DATA_IN(1)")->get_name(), "a(0)");
        EXPECT_EQ(g5->get_fan_in_net("DATA_IN(2)"), nullptr);    // a signal with an open bit is not zero-extended
        TEST_END
    }

    /**
     * Testing what becomes a net: unused internal signals vanish, unused top ports stay, signals with attributes stay,
     * and the option keeps everything; multiple drivers are accepted; attributes of merged signals land on the net.
     *
     * Functions: instantiate
     */
    TEST_F(NetlistIRInstantiateTest, check_net_selection_and_attributes)
    {
        TEST_START
        Design d;
        netlist_ir::Module& top = d.add_module("top");
        Port& a                 = top.add_port("a", PinDirection::input);
        Port& unused_in         = top.add_port("unused_in", PinDirection::input);
        Port& y                 = top.add_port("y", PinDirection::output);
        Port& unused_out        = top.add_port("unused_out", PinDirection::output);
        Signal& dangling        = top.add_signal("dangling");
        Signal& tagged          = top.add_signal("tagged");
        Signal& alias           = top.add_signal("alias_of_a");
        tagged.attributes.push_back({Parameter::Boolean("keep", "false").get(), "true"});
        alias.attributes.push_back({Parameter::String("mark", "").get(), "x"});
        top.add_alias(alias.bits.front(), a.bits.front());

        Instance& g = top.add_instance("g", "BUF", InstanceKind::Gate);
        g.add_connection("I", alias.bits);
        g.add_connection("O", y.bits);
        Instance& h = top.add_instance("h", "BUF", InstanceKind::Gate);
        h.add_connection("I", a.bits);
        h.add_connection("O", y.bits);    // second driver of y
        (void)unused_in;
        (void)unused_out;
        (void)dangling;

        {
            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            auto nl = res.get();

            EXPECT_EQ(net_by_name(nl.get(), "dangling"), nullptr);
            EXPECT_EQ(net_by_name(nl.get(), "tagged"), nullptr);    // an attribute alone does not keep a signal
            ASSERT_NE(net_by_name(nl.get(), "unused_in"), nullptr);
            EXPECT_TRUE(nl->is_global_input_net(net_by_name(nl.get(), "unused_in")));
            ASSERT_NE(net_by_name(nl.get(), "unused_out"), nullptr);
            EXPECT_TRUE(nl->is_global_output_net(net_by_name(nl.get(), "unused_out")));

            // the alias is one net, named after the top port, carrying the signal's attribute
            Net* a_net = net_by_name(nl.get(), "a");
            ASSERT_NE(a_net, nullptr);
            EXPECT_EQ(net_by_name(nl.get(), "alias_of_a"), nullptr);
            EXPECT_EQ(gate_by_name(nl.get(), "g")->get_fan_in_net("I"), a_net);
            EXPECT_EQ(a_net->get_attribute_value("mark").get(), "x");

            Net* y_net = net_by_name(nl.get(), "y");
            ASSERT_NE(y_net, nullptr);
            EXPECT_EQ(y_net->get_num_of_sources(), 2);
            EXPECT_EQ(nl->get_nets().size(), 4);
        }
        {
            InstantiationOptions options;
            options.keep_unconnected_signals = true;
            auto res                         = instantiate(d, test_utils::get_gate_library(), options);
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            auto nl = res.get();
            EXPECT_NE(net_by_name(nl.get(), "dangling"), nullptr);
            ASSERT_NE(net_by_name(nl.get(), "tagged"), nullptr);
            EXPECT_EQ(net_by_name(nl.get(), "tagged")->get_attribute_value("keep").get(), "true");
        }
        TEST_END
    }

    /**
     * Testing parameters and attributes: a parameter declared by the gate type takes the gate type's declaration
     * (the only way an enum is set), others keep the inferred one; module defaults and instance overrides land on the
     * module; attributes go to the attribute store on gates, modules and nets; a bad value is an error.
     *
     * Functions: instantiate
     */
    TEST_F(NetlistIRInstantiateTest, check_parameters_and_attributes)
    {
        TEST_START
        Design d;
        netlist_ir::Module& sub = d.add_module("sub");
        sub.parameters.push_back({Parameter::Integer("WIDTH", "8").get(), "8"});
        sub.parameters.push_back({Parameter::Integer("DEPTH", "2").get(), "2"});
        sub.attributes.push_back({Parameter::String("origin", "").get(), "file"});
        Port& i     = sub.add_port("i", PinDirection::input);
        Port& o     = sub.add_port("o", PinDirection::output);
        Instance& p = sub.add_instance("p", "PARAM_TEST", InstanceKind::Gate);
        p.add_connection("I0", i.bits);
        p.add_connection("I1", i.bits);
        p.add_connection("O", o.bits);
        p.parameters.push_back({Parameter::String("mode", "").get(), "inverted"});          // declared as an enum by the gate type
        p.parameters.push_back({Parameter::BitVector("width", 32, "").get(), "0xBEEF"});    // declared as 16 bits by the gate type
        p.parameters.push_back({Parameter::String("note", "").get(), "free"});              // not declared: inferred declaration stays
        p.attributes.push_back({Parameter::Boolean("keep", "false").get(), "true"});
        p.attributes.push_back({Parameter::String("mode", "").get(), "attr"});    // same name as a parameter

        netlist_ir::Module& top = d.add_module("top");
        Port& a                 = top.add_port("a", PinDirection::input);
        Port& y                 = top.add_port("y", PinDirection::output);
        Instance& s             = top.add_instance("s", "sub", InstanceKind::Module);
        s.add_connection("i", a.bits);
        s.add_connection("o", y.bits);
        s.parameters.push_back({Parameter::Integer("WIDTH", "0").get(), "16"});
        s.attributes.push_back({Parameter::String("loc", "").get(), "here"});

        auto res = instantiate(d, test_utils::get_gate_library());
        ASSERT_TRUE(res.is_ok()) << res.get_error().get();
        auto nl = res.get();

        Gate* g = gate_by_name(nl.get(), "p");
        ASSERT_NE(g, nullptr);
        EXPECT_EQ(g->get_parameter_value("mode").get(), "inverted");
        EXPECT_EQ(g->get_parameter_declaration("mode").get().get_type(), Parameter::Type::Enum);
        EXPECT_EQ(g->get_parameter_value("width").get(), "0xBEEF");
        EXPECT_EQ(g->get_parameter_declaration("width").get().get_size(), 16);
        EXPECT_EQ(g->get_parameter_value("note").get(), "free");
        EXPECT_EQ(g->get_parameter_declaration("note").get().get_type(), Parameter::Type::String);
        EXPECT_EQ(g->get_attribute_value("keep").get(), "true");
        EXPECT_EQ(g->get_attribute_value("mode").get(), "attr");
        EXPECT_TRUE(g->get_data_map().empty());    // nothing lands in the legacy data map

        hal::Module* sm = module_by_name(nl.get(), "s");
        ASSERT_NE(sm, nullptr);
        EXPECT_EQ(sm->get_parameter_value("WIDTH").get(), "16");    // overridden
        EXPECT_EQ(sm->get_parameter_value("DEPTH").get(), "2");     // declared default
        EXPECT_EQ(sm->get_attribute_value("origin").get(), "file");
        EXPECT_EQ(sm->get_attribute_value("loc").get(), "here");
        EXPECT_TRUE(sm->get_data_map().empty());

        // a value the gate type's declaration rejects is an error
        {
            NO_COUT_TEST_BLOCK;
            p.parameters.front().value = "sideways";
            EXPECT_TRUE(instantiate(d, test_utils::get_gate_library()).is_error());
        }
        TEST_END
    }

    /**
     * Testing gate type and pin resolution: exact names first, a unique case-insensitive match second, everything
     * else an error that names the instance.
     *
     * Functions: instantiate
     */
    TEST_F(NetlistIRInstantiateTest, check_name_resolution_and_errors)
    {
        TEST_START
        NO_COUT_TEST_BLOCK;
        {
            Design d;
            netlist_ir::Module& top = d.add_module("top");
            Port& a                 = top.add_port("a", PinDirection::input);
            Port& y                 = top.add_port("y", PinDirection::output);
            Instance& g             = top.add_instance("g", "and2", InstanceKind::Gate);
            g.add_connection("i0", a.bits);
            g.add_connection("I1", a.bits);
            g.add_connection("o", y.bits);
            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            Gate* gate = gate_by_name(res.get().get(), "g");
            ASSERT_NE(gate, nullptr);
            EXPECT_EQ(gate->get_type()->get_name(), "AND2");
            EXPECT_EQ(gate->get_fan_in_net("I0")->get_name(), "a");
            EXPECT_EQ(gate->get_fan_out_net("O")->get_name(), "y");
        }
        {
            Design d;
            netlist_ir::Module& top = d.add_module("top");
            Port& a                 = top.add_port("a", PinDirection::input);
            top.add_instance("g", "NOPE", InstanceKind::Gate).add_connection("I", a.bits);
            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_error());
            EXPECT_NE(res.get_error().get().find("'g'"), std::string::npos);
            EXPECT_NE(res.get_error().get().find("NOPE"), std::string::npos);
        }
        {
            Design d;
            netlist_ir::Module& top = d.add_module("top");
            Port& a                 = top.add_port("a", PinDirection::input);
            Instance& g             = top.add_instance("g", "BUF", InstanceKind::Gate);
            g.location              = {7, 3};
            g.add_connection("NOT_A_PIN", a.bits);
            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_error());
            EXPECT_NE(res.get_error().get().find("NOT_A_PIN"), std::string::npos);
            EXPECT_NE(res.get_error().get().find("line 7"), std::string::npos);
        }
        {
            // a cycle that the top-module rule cannot catch because the top is named explicitly
            Design d;
            netlist_ir::Module& a = d.add_module("a");
            netlist_ir::Module& b = d.add_module("b");
            a.add_instance("ib", "b", InstanceKind::Module);
            b.add_instance("ia", "a", InstanceKind::Module);
            d.top    = "a";
            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_error());
            EXPECT_NE(res.get_error().get().find("instantiates itself"), std::string::npos);
        }
        {
            Design d;
            d.add_module("top");
            EXPECT_TRUE(instantiate(d, nullptr).is_error());
        }
        TEST_END
    }

    /**
     * Testing the constants: the GND and VCC gates exist only when the constants are used, an inout port of the top
     * module is both a global input and output, and a top port aliased to a constant keeps the port name.
     *
     * Functions: instantiate
     */
    TEST_F(NetlistIRInstantiateTest, check_constants_and_inout)
    {
        TEST_START
        {
            Design d;
            netlist_ir::Module& top = d.add_module("top");
            Port& io                = top.add_port("io", PinDirection::inout);
            Port& zero_out          = top.add_port("zero_out", PinDirection::output);
            Instance& g             = top.add_instance("g", "BUF", InstanceKind::Gate);
            g.add_connection("I", io.bits);
            g.add_connection("O", io.bits);
            top.add_alias(zero_out.bits.front(), ZERO);

            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            auto nl = res.get();

            Net* io_net = net_by_name(nl.get(), "io");
            ASSERT_NE(io_net, nullptr);
            EXPECT_TRUE(nl->is_global_input_net(io_net));
            EXPECT_TRUE(nl->is_global_output_net(io_net));

            Net* zero = net_by_name(nl.get(), "zero_out");
            ASSERT_NE(zero, nullptr);
            EXPECT_TRUE(zero->is_gnd_net());
            EXPECT_TRUE(nl->is_global_output_net(zero));
            EXPECT_EQ(nl->get_gnd_gates().size(), 1);
            EXPECT_TRUE(nl->get_vcc_gates().empty());
            EXPECT_EQ(net_by_name(nl.get(), "'0'"), nullptr);
        }
        {
            // an explicit GND instance is marked as a GND gate and drives its own net, apart from any constant
            Design d;
            netlist_ir::Module& top = d.add_module("top");
            Port& y                 = top.add_port("y", PinDirection::output);
            Port& z                 = top.add_port("z", PinDirection::output);
            Instance& gnd           = top.add_instance("gnd_inst", "GND", InstanceKind::Gate);
            gnd.add_connection("O", y.bits);
            top.add_alias(z.bits.front(), ZERO);
            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            auto nl = res.get();
            EXPECT_EQ(nl->get_gnd_gates().size(), 2);
            EXPECT_TRUE(net_by_name(nl.get(), "y")->is_gnd_net());
            EXPECT_TRUE(net_by_name(nl.get(), "z")->is_gnd_net());
            EXPECT_NE(net_by_name(nl.get(), "y"), net_by_name(nl.get(), "z"));
            EXPECT_TRUE(gate_by_name(nl.get(), "gnd_inst")->is_gnd_gate());
        }
        {
            // no constants used: no GND or VCC gate
            Design d;
            netlist_ir::Module& top = d.add_module("top");
            Port& a                 = top.add_port("a", PinDirection::input);
            Port& y                 = top.add_port("y", PinDirection::output);
            Instance& g             = top.add_instance("g", "BUF", InstanceKind::Gate);
            g.add_connection("I", a.bits);
            g.add_connection("O", y.bits);
            auto res = instantiate(d, test_utils::get_gate_library());
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            EXPECT_EQ(res.get().get()->get_gates().size(), 1);
        }
        TEST_END
    }
}    // namespace hal
