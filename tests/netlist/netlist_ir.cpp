#include "hal_core/netlist/netlist_ir/netlist_ir.h"

#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace netlist_ir;

    class NetlistIRTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }

        /**
         * A two-level design: `top` instantiates `sub` twice and an AND2 gate, with an alias between an internal
         * signal and a port bit, and a constant on one gate pin.
         */
        Design create_design()
        {
            Design d;
            d.source = "test.v";

            netlist_ir::Module& sub = d.add_module("sub");
            Port& i                 = sub.add_port("i", PinDirection::input, {{1, 0}});
            Port& o                 = sub.add_port("o", PinDirection::output);
            Instance& g             = sub.add_instance("g", "AND2", InstanceKind::Gate);
            g.add_connection("I0", {i.bits.at(1)});    // i[0]
            g.add_connection("I1", {i.bits.at(0)});    // i[1]
            g.add_connection("O", {o.bits.front()});

            netlist_ir::Module& top = d.add_module("top");
            Port& a                 = top.add_port("a", PinDirection::input, {{3, 0}});
            Port& y                 = top.add_port("y", PinDirection::output);
            Signal& m               = top.add_signal("m", {{1, 0}});
            Signal& n               = top.add_signal("n");

            Instance& s0 = top.add_instance("s0", "sub", InstanceKind::Module);
            s0.add_connection("i", {a.bits.at(0), a.bits.at(1)});    // a[3:2]
            s0.add_connection("o", {m.bits.at(1)});                  // m[0]

            Instance& s1 = top.add_instance("s1", "sub", InstanceKind::Module);
            s1.add_connection("i", {a.bits.at(2), ZERO});    // {a[1], 1'b0}
            s1.add_connection("o", {m.bits.at(0)});          // m[1]

            Instance& g2 = top.add_instance("g2", "AND2", InstanceKind::Gate);
            g2.add_connection("I0", {m.bits.at(1)});
            g2.add_connection("I1", {n.bits.front()});
            g2.add_connection("O", {y.bits.front()});

            top.add_alias(n.bits.front(), m.bits.at(0));    // assign n = m[1];

            return d;
        }
    };

    /**
     * Testing the index arithmetic of a range in both directions.
     *
     * Functions: Range::size, Range::is_descending, Range::index_at, Range::offset_of
     */
    TEST_F(NetlistIRTest, check_range)
    {
        TEST_START
        {
            const Range down{7, 0};
            EXPECT_EQ(down.size(), 8);
            EXPECT_TRUE(down.is_descending());
            EXPECT_EQ(down.index_at(0), 7);
            EXPECT_EQ(down.index_at(7), 0);
            EXPECT_EQ(down.offset_of(7), 0);
            EXPECT_EQ(down.offset_of(0), 7);
            EXPECT_EQ(down.offset_of(3), 4);
            EXPECT_EQ(down.offset_of(8), std::nullopt);
            EXPECT_EQ(down.offset_of(-1), std::nullopt);
        }
        {
            const Range up{2, 5};
            EXPECT_EQ(up.size(), 4);
            EXPECT_FALSE(up.is_descending());
            EXPECT_EQ(up.index_at(0), 2);
            EXPECT_EQ(up.index_at(3), 5);
            EXPECT_EQ(up.offset_of(2), 0);
            EXPECT_EQ(up.offset_of(5), 3);
            EXPECT_EQ(up.offset_of(1), std::nullopt);
        }
        {
            const Range one{3, 3};
            EXPECT_EQ(one.size(), 1);
            EXPECT_FALSE(one.is_descending());
            EXPECT_EQ(one.offset_of(3), 0);
        }
        {
            const Range negative{-1, 0};
            EXPECT_EQ(negative.size(), 2);
            EXPECT_EQ(negative.index_at(0), -1);
            EXPECT_EQ(negative.offset_of(0), 1);
        }
        EXPECT_EQ((Range{7, 0}), (Range{7, 0}));
        EXPECT_NE((Range{7, 0}), (Range{0, 7}));
        TEST_END
    }

    /**
     * Testing the bit layout of signals: scalars, vectors in both directions, and a two-dimensional signal.
     *
     * Functions: Module::add_signal, Signal::width, Signal::bit_at, Signal::slice, Signal::bit_name
     */
    TEST_F(NetlistIRTest, check_signal_bits)
    {
        TEST_START
        netlist_ir::Module m;
        m.name = "m";

        const Signal& scalar = m.add_signal("s");
        EXPECT_EQ(scalar.width(), 1);
        EXPECT_TRUE(scalar.dims.empty());
        EXPECT_EQ(scalar.bits.front(), FIRST_USER_BIT);
        EXPECT_EQ(scalar.bit_name(0), "s");
        ASSERT_TRUE(scalar.bit_at({}).is_ok());
        EXPECT_EQ(scalar.bit_at({}).get(), FIRST_USER_BIT);
        EXPECT_TRUE(scalar.bit_at({0}).is_error());
        EXPECT_TRUE(scalar.slice({0, 0}).is_error());

        // [3:0]: bits[0] is index 3
        const Signal& down = m.add_signal("d", {{3, 0}});
        EXPECT_EQ(down.width(), 4);
        EXPECT_EQ(down.bits.front(), FIRST_USER_BIT + 1);
        EXPECT_EQ(down.bit_at({3}).get(), down.bits.at(0));
        EXPECT_EQ(down.bit_at({0}).get(), down.bits.at(3));
        EXPECT_TRUE(down.bit_at({4}).is_error());
        EXPECT_TRUE(down.bit_at({0, 0}).is_error());
        EXPECT_EQ(down.bit_name(0), "d(3)");
        EXPECT_EQ(down.bit_name(3), "d(0)");

        // a slice lists the bits in the order of the given range
        ASSERT_TRUE(down.slice({2, 1}).is_ok());
        EXPECT_EQ(down.slice({2, 1}).get(), (std::vector<BitId>{down.bits.at(1), down.bits.at(2)}));
        EXPECT_EQ(down.slice({1, 2}).get(), (std::vector<BitId>{down.bits.at(2), down.bits.at(1)}));
        EXPECT_EQ(down.slice({3, 0}).get(), down.bits);
        EXPECT_TRUE(down.slice({4, 0}).is_error());

        // [0:3]: bits[0] is index 0
        const Signal& up = m.add_signal("u", {{0, 3}});
        EXPECT_EQ(up.bit_at({0}).get(), up.bits.at(0));
        EXPECT_EQ(up.bit_at({3}).get(), up.bits.at(3));
        EXPECT_EQ(up.bit_name(0), "u(0)");

        // [0:1][2:3]: declaration order, innermost fastest
        const Signal& two = m.add_signal("t", {{0, 1}, {3, 2}});
        EXPECT_EQ(two.width(), 4);
        EXPECT_EQ(two.bit_at({0, 3}).get(), two.bits.at(0));
        EXPECT_EQ(two.bit_at({0, 2}).get(), two.bits.at(1));
        EXPECT_EQ(two.bit_at({1, 3}).get(), two.bits.at(2));
        EXPECT_EQ(two.bit_at({1, 2}).get(), two.bits.at(3));
        EXPECT_EQ(two.bit_name(0), "t(0)(3)");
        EXPECT_EQ(two.bit_name(3), "t(1)(2)");
        EXPECT_TRUE(two.slice({0, 1}).is_error());

        // every allocated bit is unique and below next_bit
        EXPECT_EQ(m.next_bit(), FIRST_USER_BIT + 1 + 4 + 4 + 4);
        TEST_END
    }

    /**
     * Testing the builder helpers and the lookups of a module and a design, including reference stability.
     *
     * Functions: Design::add_module, Design::find_module, Module::add_port, Module::add_instance, Module::find_port,
     *            Module::find_signal, Module::find_instance, Instance::add_connection, Instance::find_connection
     */
    TEST_F(NetlistIRTest, check_builders_and_lookups)
    {
        TEST_START
        Design d = create_design();

        ASSERT_EQ(d.modules.size(), 2);
        const netlist_ir::Module* top = d.find_module("top");
        const netlist_ir::Module* sub = d.find_module("sub");
        ASSERT_NE(top, nullptr);
        ASSERT_NE(sub, nullptr);
        EXPECT_EQ(d.find_module("nope"), nullptr);

        // ports and signals share one name space for lookups, but ports are not repeated in the signal list
        ASSERT_NE(top->find_port("a"), nullptr);
        EXPECT_EQ(top->find_port("a")->direction, PinDirection::input);
        EXPECT_EQ(top->find_port("a")->width(), 4);
        EXPECT_EQ(top->find_port("m"), nullptr);
        EXPECT_EQ(top->find_signal("a"), top->find_port("a"));
        ASSERT_NE(top->find_signal("m"), nullptr);
        EXPECT_EQ(top->find_signal("m")->width(), 2);
        EXPECT_EQ(top->signals.size(), 2);
        EXPECT_EQ(top->ports.size(), 2);

        ASSERT_NE(top->find_instance("s0"), nullptr);
        EXPECT_EQ(top->find_instance("s0")->kind, InstanceKind::Module);
        EXPECT_EQ(top->find_instance("s0")->type, "sub");
        EXPECT_EQ(top->find_instance("nope"), nullptr);
        const Connection* c = top->find_instance("s0")->find_connection("i");
        ASSERT_NE(c, nullptr);
        EXPECT_EQ(c->bits.size(), 2);
        EXPECT_EQ(c->bits.front(), top->find_port("a")->bits.at(0));
        EXPECT_EQ(top->find_instance("s0")->find_connection("nope"), nullptr);

        // the constant bit is shared by every module
        EXPECT_EQ(top->find_instance("s1")->find_connection("i")->bits.back(), ZERO);

        // references handed out by the builders stay valid while more elements are added
        netlist_ir::Module& m = d.add_module("more");
        Port& first           = m.add_port("p0", PinDirection::input);
        const BitId b0        = first.bits.front();
        for (u32 i = 1; i < 100; i++)
        {
            m.add_port("p" + std::to_string(i), PinDirection::input);
        }
        EXPECT_EQ(first.name, "p0");
        EXPECT_EQ(first.bits.front(), b0);
        EXPECT_EQ(m.ports.size(), 100);
        TEST_END
    }

    /**
     * Testing the top module rules: an explicit top wins, otherwise the single uninstantiated module, otherwise an error
     * that names the candidates.
     *
     * Functions: Design::find_top
     */
    TEST_F(NetlistIRTest, check_find_top){TEST_START{Design d = create_design();
    ASSERT_TRUE(d.find_top().is_ok());
    EXPECT_EQ(d.find_top().get(), "top");

    // an explicit top wins even if it is instantiated somewhere
    d.top = "sub";
    ASSERT_TRUE(d.find_top().is_ok());
    EXPECT_EQ(d.find_top().get(), "sub");

    // an explicit top that does not exist is an error
    d.top = "nope";
    EXPECT_TRUE(d.find_top().is_error());
}    // namespace hal
{
    NO_COUT_TEST_BLOCK;
    Design d = create_design();
    d.add_module("unused");
    auto res = d.find_top();
    ASSERT_TRUE(res.is_error());
    EXPECT_NE(res.get_error().get().find("top"), std::string::npos);
    EXPECT_NE(res.get_error().get().find("unused"), std::string::npos);
}
{
    // every module instantiated by another one: a cycle without a root
    NO_COUT_TEST_BLOCK;
    Design d;
    netlist_ir::Module& a = d.add_module("a");
    netlist_ir::Module& b = d.add_module("b");
    a.add_instance("ib", "b", InstanceKind::Module);
    b.add_instance("ia", "a", InstanceKind::Module);
    EXPECT_TRUE(d.find_top().is_error());
}
{
    // gate instances do not count as instantiations of a module of the same name
    Design d;
    netlist_ir::Module& a = d.add_module("a");
    a.add_instance("g", "b", InstanceKind::Gate);
    d.add_module("b");
    EXPECT_TRUE(d.find_top().is_error());
}
TEST_END
}

/**
     * Testing that a well-formed design validates and that every rule of the validation rejects its violation.
     *
     * Functions: Design::validate
     */
TEST_F(NetlistIRTest, check_validate)
{
    TEST_START
    {
        Design d = create_design();
        auto res = d.validate();
        EXPECT_TRUE(res.is_ok()) << (res.is_error() ? res.get_error().get() : "");
    }

    NO_COUT_TEST_BLOCK;
    const auto expect_invalid = [](Design d, const std::string& what) {
        auto res = d.validate();
        EXPECT_TRUE(res.is_error()) << "accepted a design with " << what;
    };

    {
        Design d = create_design();
        d.add_module("top");
        expect_invalid(d, "two modules of the same name");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_signal("a");
        expect_invalid(d, "a signal named like a port");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_port("m", PinDirection::input);
        expect_invalid(d, "a port named like a signal");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_port("bad", PinDirection::none);
        expect_invalid(d, "a port without a direction");
    }
    {
        Design d = create_design();
        d.find_module("top")->signals.front().bits.push_back(9999);
        expect_invalid(d, "a signal owning a bit that was never allocated");
    }
    {
        Design d = create_design();
        d.find_module("top")->signals.front().bits.pop_back();
        expect_invalid(d, "a signal narrower than its dimensions");
    }
    {
        Design d                          = create_design();
        netlist_ir::Module* top           = d.find_module("top");
        const BitId taken                 = top->ports.front().bits.front();
        top->signals.front().bits.front() = taken;
        expect_invalid(d, "a bit owned by a port and a signal");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_alias(FIRST_USER_BIT, FIRST_USER_BIT);
        expect_invalid(d, "an alias of a bit with itself");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_alias(FIRST_USER_BIT, 9999);
        expect_invalid(d, "an alias to a bit that was never allocated");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_alias(FIRST_USER_BIT, OPEN);
        expect_invalid(d, "an alias to an open position");
    }
    {
        // an open position in a connection is fine
        Design d                                                            = create_design();
        d.find_module("top")->find_instance("g2")->connections.front().bits = {OPEN};
        EXPECT_TRUE(d.validate().is_ok());
    }
    {
        Design d = create_design();
        d.find_module("top")->add_instance("s0", "sub", InstanceKind::Module);
        expect_invalid(d, "two instances of the same name");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_instance("x", "nope", InstanceKind::Module);
        expect_invalid(d, "a module instance of an unknown module");
    }
    {
        Design d = create_design();
        d.find_module("top")->add_instance("x", "top", InstanceKind::Module);
        expect_invalid(d, "a module instantiating itself");
    }
    {
        Design d = create_design();
        d.find_module("top")->find_instance("s0")->add_connection("i", {ZERO, ONE});
        expect_invalid(d, "a port connected twice");
    }
    {
        Design d = create_design();
        d.find_module("top")->find_instance("s0")->add_connection("i", {ZERO}, Range{0, 0});
        expect_invalid(d, "a port connected as a whole and as a slice");
    }
    {
        Design d           = create_design();
        Connection& c      = d.find_module("top")->find_instance("s0")->add_connection("nope", {ZERO, ONE});
        c.port             = "i";
        c.replicate        = true;
        d.find_module("top")->find_instance("s0")->connections.erase(d.find_module("top")->find_instance("s0")->connections.begin());    // drop the original 'i'
        expect_invalid(d, "a replicated connection with two bits");
    }
    {
        Design d = create_design();
        d.find_module("top")->find_instance("s0")->add_connection("nope", {ZERO});
        expect_invalid(d, "a connection to a port the module does not have");
    }
    {
        // a connection wider or narrower than the module port is not invalid: the low bits pair up at instantiation
        Design d = create_design();
        d.find_module("top")->find_instance("s0")->connections.front().bits.push_back(ONE);
        EXPECT_TRUE(d.validate().is_ok());
        d.find_module("top")->find_instance("s0")->connections.front().bits = {ZERO};
        EXPECT_TRUE(d.validate().is_ok());
    }
    {
        Design d = create_design();
        d.find_module("top")->find_instance("s0")->add_connection("", {ZERO});
        expect_invalid(d, "named and positional connections mixed");
    }
    {
        Design d = create_design();
        d.find_module("top")->find_instance("g2")->connections.front().bits.push_back(9999);
        expect_invalid(d, "a gate connection to a bit that was never allocated");
    }
    {
        Design d    = create_design();
        Instance& i = d.find_module("top")->add_instance("s2", "sub", InstanceKind::Module);
        i.add_connection("", {ZERO, ONE});
        i.add_connection("", {ZERO});
        i.add_connection("", {ZERO});
        expect_invalid(d, "more positional connections than ports");
    }
    {
        // a slice of a module port must lie inside the port and the width must match the slice
        Design d                                                       = create_design();
        d.find_module("top")->find_instance("s0")->connections.front() = Connection{"i", Range{1, 1}, {ZERO}};
        EXPECT_TRUE(d.validate().is_ok());
        d.find_module("top")->find_instance("s0")->connections.front() = Connection{"i", Range{2, 1}, {ZERO, ONE}};
        expect_invalid(d, "a port slice outside the port");
    }
    {
        Design d = create_design();
        d.top    = "nope";
        expect_invalid(d, "an explicit top that does not exist");
    }
    {
        Design d     = create_design();
        const auto p = Parameter::Integer("W", "4").get();
        d.find_module("sub")->parameters.push_back({p, "4"});
        d.find_module("sub")->parameters.push_back({p, "8"});
        expect_invalid(d, "a parameter declared twice");
    }
    {
        Design d        = create_design();
        const auto keep = Parameter::Boolean("keep", "false").get();
        d.find_module("top")->find_instance("g2")->attributes.push_back({keep, "true"});
        d.find_module("top")->find_instance("g2")->attributes.push_back({keep, "false"});
        expect_invalid(d, "an attribute given twice");
    }
    {
        Design d = create_design();
        d.find_module("top")->signals.front().attributes.push_back({Parameter::Integer("n", "0").get(), "abc"});
        expect_invalid(d, "an attribute value that does not fit its type");
    }
    {
        // an attribute and a parameter may share a name
        Design d        = create_design();
        const auto keep = Parameter::Boolean("keep", "false").get();
        d.find_module("top")->find_instance("g2")->attributes.push_back({keep, "true"});
        d.find_module("top")->find_instance("g2")->parameters.push_back({keep, "false"});
        EXPECT_TRUE(d.validate().is_ok());
    }
    {
        // gate instances are not checked against anything but the bit handles: the gate library is not known here
        Design d = create_design();
        d.find_module("top")->find_instance("g2")->add_connection("NOT_A_PIN", {ONE});
        EXPECT_TRUE(d.validate().is_ok());
    }
    TEST_END
}
}    // namespace hal
