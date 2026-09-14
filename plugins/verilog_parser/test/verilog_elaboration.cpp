#include "verilog_parser/verilog_elaboration.h"

#include "netlist_test_utils.h"
#include "verilog_parser/verilog_syntax.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace verilog;
    using netlist_ir::BitId;
    using netlist_ir::Design;
    using netlist_ir::InstanceKind;
    using netlist_ir::ONE;
    using netlist_ir::OPEN;
    using netlist_ir::ZERO;

    class VerilogElaborationTest : public ::testing::Test
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

        Design elaborate_text(const std::string& text)
        {
            auto parsed = parse_string(text, "test.v");
            if (parsed.is_error())
            {
                ADD_FAILURE() << parsed.get_error().get();
                return {};
            }
            auto res = elaborate(parsed.get());
            if (res.is_error())
            {
                ADD_FAILURE() << res.get_error().get();
                return {};
            }
            return res.get();
        }

        static bool elaboration_fails(const std::string& text, const std::string& needle)
        {
            auto parsed = parse_string(text, "test.v");
            if (parsed.is_error())
            {
                ADD_FAILURE() << "did not even parse: " << parsed.get_error().get();
                return false;
            }
            auto res = elaborate(parsed.get());
            if (res.is_ok())
            {
                return false;
            }
            EXPECT_NE(res.get_error().get().find(needle), std::string::npos) << res.get_error().get();
            return true;
        }

        static const netlist_ir::TypedValue* find_value(const std::vector<netlist_ir::TypedValue>& values, const std::string& name)
        {
            for (const auto& v : values)
            {
                if (v.declaration.get_name() == name)
                {
                    return &v;
                }
            }
            return nullptr;
        }

        static bool has_alias(const netlist_ir::Module& m, BitId a, BitId b)
        {
            for (const auto& [x, y] : m.aliases)
            {
                if ((x == a && y == b) || (x == b && y == a))
                {
                    return true;
                }
            }
            return false;
        }
    };

    /**
     * Testing the number decoder on every literal form.
     *
     * Functions: parse_number
     */
    TEST_F(VerilogElaborationTest, check_numbers)
    {
        TEST_START
        const auto n = [](const std::string& text) {
            auto res = parse_number(text);
            if (res.is_error())
            {
                ADD_FAILURE() << res.get_error().get();
                return Number();
            }
            return res.get();
        };
        EXPECT_EQ(n("12").bits, "1100");
        EXPECT_FALSE(n("12").sized);
        EXPECT_FALSE(n("12").based);
        EXPECT_EQ(n("0").bits, "0");
        EXPECT_EQ(n("1_000").to_u64().get(), 1000);
        EXPECT_EQ(n("4'b10_1x").bits, "101x");
        EXPECT_TRUE(n("4'b10_1x").sized);
        EXPECT_FALSE(n("4'b10_1x").is_defined());
        EXPECT_EQ(n("8'hZZ").bits, "zzzzzzzz");
        EXPECT_EQ(n("'habc").bits, "101010111100");
        EXPECT_EQ(n("'habc").width, 12);
        EXPECT_EQ(n("'habc").to_hex(), "0xABC");
        EXPECT_EQ(n("'d 2748").bits, "101010111100");
        EXPECT_EQ(n("'o5274").bits, "101010111100");
        EXPECT_EQ(n("2'sb01").is_signed, true);
        EXPECT_EQ(n("8'b101").bits, "00000101");    // zero-extended to the width
        EXPECT_EQ(n("8'bx1").bits, "xxxxxxx1");     // x-extended
        EXPECT_EQ(n("8'h0F").bits, "00001111");
        EXPECT_EQ(n("4'h0F").bits, "1111");    // leading zeros may be dropped
        EXPECT_EQ(n("'0").bits, "0");
        EXPECT_EQ(n("'1").bits, "1");
        EXPECT_EQ(n("'x").bits, "x");
        EXPECT_EQ(n("6'o7?").bits, "111zzz");
        EXPECT_TRUE(n("1.5").is_real);
        EXPECT_TRUE(n("1e3").is_real);
        EXPECT_EQ(n("64'hFFFF_FFFF_FFFF_FFFF").to_u64().get(), UINT64_MAX);
        EXPECT_EQ(n("16'hCAFE").to_hex(), "0xCAFE");
        EXPECT_EQ(n("1'b0").to_hex(), "0x0");
        EXPECT_EQ(n("16'h000A").to_hex(), "0xA");

        NO_COUT_TEST_BLOCK;
        EXPECT_TRUE(parse_number("4'hFF").is_error());     // does not fit
        EXPECT_TRUE(parse_number("4'b012").is_error());    // bad digit
        EXPECT_TRUE(parse_number("4'q0").is_error());      // bad base
        EXPECT_TRUE(parse_number("0'b0").is_error());      // zero width
        EXPECT_TRUE(parse_number("'h").is_error());        // no digits
        EXPECT_TRUE(parse_number("12a").is_error());
        EXPECT_TRUE(n("4'bx").to_u64().is_error());
        EXPECT_TRUE(n("1.5").to_u64().is_error());
        EXPECT_TRUE(parse_number("65'h1_FFFF_FFFF_FFFF_FFFF").get().to_u64().is_error());
        TEST_END
    }

    /**
     * Testing ports and signals: both header styles, header port expressions, shared ranges, multi-dimensional
     * signals, re-declared ports, and the ranges evaluated from parameters.
     */
    TEST_F(VerilogElaborationTest, check_ports_and_signals){TEST_START{Design d = elaborate_text("module top (a, b, .sum({\\<const0> ,\\^sum [1:0]}), io);\n"
                                                                                                 "  input a, b;\n"
                                                                                                 "  output [1:0] \\^sum ;\n"
                                                                                                 "  output \\<const0> ;\n"
                                                                                                 "  inout io;\n"
                                                                                                 "  wire a;\n"
                                                                                                 "  wire [3:0] v, w [1:0];\n"
                                                                                                 "  wire [0:1][2:3] m;\n"
                                                                                                 "endmodule");
    ASSERT_EQ(d.modules.size(), 1);
    const netlist_ir::Module& m = d.modules.front();
    EXPECT_EQ(m.name, "top");
    ASSERT_EQ(m.ports.size(), 4);
    EXPECT_EQ(m.ports.at(0).name, "a");
    EXPECT_EQ(m.ports.at(0).direction, PinDirection::input);
    EXPECT_EQ(m.ports.at(0).width(), 1);
    EXPECT_EQ(m.ports.at(2).name, "sum");
    EXPECT_EQ(m.ports.at(2).direction, PinDirection::output);
    EXPECT_EQ(m.ports.at(2).width(), 3);
    EXPECT_EQ(m.ports.at(2).dims.front(), (netlist_ir::Range{2, 0}));
    EXPECT_EQ(m.ports.at(3).name, "io");
    EXPECT_EQ(m.ports.at(3).direction, PinDirection::inout);

    // the signals of the port expression are internal signals, and the port bits are aliased to them
    const netlist_ir::Signal* const0 = m.find_signal("<const0>");
    const netlist_ir::Signal* sum    = m.find_signal("^sum");
    ASSERT_NE(const0, nullptr);
    ASSERT_NE(sum, nullptr);
    EXPECT_EQ(m.find_port("^sum"), nullptr);
    EXPECT_EQ(sum->width(), 2);
    EXPECT_TRUE(has_alias(m, m.ports.at(2).bits.at(0), const0->bits.front()));    // sum[2] = <const0>
    EXPECT_TRUE(has_alias(m, m.ports.at(2).bits.at(1), sum->bits.at(0)));         // sum[1] = ^sum[1]
    EXPECT_TRUE(has_alias(m, m.ports.at(2).bits.at(2), sum->bits.at(1)));         // sum[0] = ^sum[0]

    ASSERT_NE(m.find_signal("v"), nullptr);
    EXPECT_EQ(m.find_signal("v")->width(), 4);
    ASSERT_NE(m.find_signal("w"), nullptr);
    EXPECT_EQ(m.find_signal("w")->width(), 8);
    EXPECT_EQ(m.find_signal("w")->dims, (std::vector<netlist_ir::Range>{{1, 0}, {3, 0}}));    // unpacked first
    EXPECT_EQ(m.find_signal("m")->dims, (std::vector<netlist_ir::Range>{{0, 1}, {2, 3}}));
    EXPECT_EQ(m.signals.size(), 5);    // <const0>, ^sum, v, w, m; the re-declared `wire a` adds nothing
}    // namespace hal
{
    Design d = elaborate_text("module top #(parameter W = 8, localparam H = W / 2) (input wire [W-1:0] a, output reg [H-1:0] y, input [1:0] p, q);\n"
                              "  wire [W*2-1:0] wide;\n"
                              "endmodule");
    ASSERT_EQ(d.modules.size(), 1);
    const netlist_ir::Module& m = d.modules.front();
    ASSERT_EQ(m.ports.size(), 4);
    EXPECT_EQ(m.ports.at(0).width(), 8);
    EXPECT_EQ(m.ports.at(1).width(), 4);
    EXPECT_EQ(m.ports.at(2).width(), 2);
    EXPECT_EQ(m.ports.at(3).width(), 2);
    EXPECT_EQ(m.find_signal("wide")->width(), 16);
    ASSERT_EQ(m.parameters.size(), 1);    // the localparam is not a parameter of the module
    EXPECT_EQ(m.parameters.front().declaration.get_name(), "W");
    EXPECT_EQ(m.parameters.front().declaration.get_type(), Parameter::Type::Integer);
    EXPECT_EQ(m.parameters.front().value, "8");
}
{
    NO_COUT_TEST_BLOCK;
    EXPECT_TRUE(elaboration_fails("module top (a); wire a; endmodule", "no direction declaration"));
    EXPECT_TRUE(elaboration_fails("module top (a); input a; wire w; wire w; endmodule", "declared twice"));
    EXPECT_TRUE(elaboration_fails("module top (a); input a; input a; endmodule", "declared twice"));
    EXPECT_TRUE(elaboration_fails("module top (a); input [1:0] a; wire a; endmodule", "different width"));
    EXPECT_TRUE(elaboration_fails("module top (input a); output b; endmodule", "not in the header"));
    EXPECT_TRUE(elaboration_fails("module top (a, a); input a; endmodule", "appears twice"));
    EXPECT_TRUE(elaboration_fails("module top (a); input [W:0] a; endmodule", "not a parameter"));
    EXPECT_TRUE(elaboration_fails("module top (.p({a, b})); input a; output b; endmodule", "different directions"));
}
TEST_END
}

/**
     * Testing the wiring expressions and the aliases they produce: literals with x and z, indices and slices in both
     * directions, multi-dimensional selects, concatenation, replication, width rules, supply nets and initializers.
     */
TEST_F(VerilogElaborationTest, check_wiring_and_aliases)
{
    TEST_START
    Design d = elaborate_text("module top (a, v, y);\n"
                              "  input a;\n"
                              "  input [3:0] v;\n"
                              "  output [3:0] y;\n"
                              "  wire [0:3] up;\n"
                              "  wire [1:0][1:0] m;\n"
                              "  wire zero, one, flt, s, t, d = a;\n"
                              "  wire [1:0] narrow, pair;\n"
                              "  wire [5:0] wide;\n"
                              "  supply0 gnd;\n"
                              "  supply1 vcc;\n"
                              "  assign zero = 1'b0;\n"
                              "  assign one = 1'b1;\n"
                              "  assign flt = 1'bz;\n"
                              "  assign {s, t} = {a, v[2]};\n"
                              "  assign narrow = v;\n"
                              "  assign wide = 2'b10;\n"
                              "  assign y = {v[1:0], up[2:3]};\n"
                              "  assign pair = m[1];\n"
                              "  BUF g0 (.I(m[0][1]), .O(up[0]));\n"
                              "  RAM r (.DATA_IN({{2{a}}, 2'b1x}), .ADDR(v[0]));\n"
                              "  RAM q (.DATA_IN(m[0:1][1:0]));\n"
                              "endmodule");
    ASSERT_EQ(d.modules.size(), 1);
    const netlist_ir::Module& m = d.modules.front();
    const auto bit              = [&m](const std::string& name, u32 i = 0) { return m.find_signal(name)->bits.at(i); };

    EXPECT_TRUE(has_alias(m, bit("zero"), ZERO));
    EXPECT_TRUE(has_alias(m, bit("one"), ONE));
    EXPECT_FALSE(std::any_of(m.aliases.begin(), m.aliases.end(), [&](const auto& p) { return p.first == bit("flt") || p.second == bit("flt"); }));
    EXPECT_TRUE(has_alias(m, bit("s"), bit("a")));
    EXPECT_TRUE(has_alias(m, bit("t"), bit("v", 1)));    // v[2] is position 1 of [3:0]
    EXPECT_TRUE(has_alias(m, bit("d"), bit("a")));
    EXPECT_TRUE(has_alias(m, bit("gnd"), ZERO));
    EXPECT_TRUE(has_alias(m, bit("vcc"), ONE));

    // narrow = v: the low bits pair up
    EXPECT_TRUE(has_alias(m, bit("narrow", 0), bit("v", 2)));    // narrow[1] = v[1]
    EXPECT_TRUE(has_alias(m, bit("narrow", 1), bit("v", 3)));    // narrow[0] = v[0]
    // wide = 2'b10: zero-extended
    EXPECT_TRUE(has_alias(m, bit("wide", 5), ZERO));    // wide[0]
    EXPECT_TRUE(has_alias(m, bit("wide", 4), ONE));     // wide[1]
    EXPECT_TRUE(has_alias(m, bit("wide", 0), ZERO));    // wide[5]
    // y = {v[1:0], up[2:3]}: y[3] = v[1], y[2] = v[0], y[1] = up[2], y[0] = up[3]
    const netlist_ir::Port* y = m.find_port("y");
    EXPECT_TRUE(has_alias(m, y->bits.at(0), bit("v", 2)));
    EXPECT_TRUE(has_alias(m, y->bits.at(1), bit("v", 3)));
    EXPECT_TRUE(has_alias(m, y->bits.at(2), bit("up", 2)));
    EXPECT_TRUE(has_alias(m, y->bits.at(3), bit("up", 3)));
    // pair = m[1]: the inner block of the two-dimensional signal
    EXPECT_TRUE(has_alias(m, bit("pair", 0), m.find_signal("m")->bit_at({1, 1}).get()));
    EXPECT_TRUE(has_alias(m, bit("pair", 1), m.find_signal("m")->bit_at({1, 0}).get()));

    // a range in every dimension, in the order the ranges list the indices
    const netlist_ir::Instance* q = m.find_instance("q");
    ASSERT_NE(q, nullptr);
    const netlist_ir::Signal* mm = m.find_signal("m");
    EXPECT_EQ(q->find_connection("DATA_IN")->bits, (std::vector<BitId>{mm->bit_at({0, 1}).get(), mm->bit_at({0, 0}).get(), mm->bit_at({1, 1}).get(), mm->bit_at({1, 0}).get()}));

    const netlist_ir::Instance* g0 = m.find_instance("g0");
    ASSERT_NE(g0, nullptr);
    EXPECT_EQ(g0->kind, InstanceKind::Gate);
    EXPECT_EQ(g0->find_connection("I")->bits, (std::vector<BitId>{m.find_signal("m")->bit_at({0, 1}).get()}));
    EXPECT_EQ(g0->find_connection("O")->bits, (std::vector<BitId>{bit("up", 0)}));

    const netlist_ir::Instance* r = m.find_instance("r");
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->find_connection("DATA_IN")->bits, (std::vector<BitId>{bit("a"), bit("a"), ONE, OPEN}));
    EXPECT_EQ(r->find_connection("ADDR")->bits, (std::vector<BitId>{bit("v", 3)}));

    NO_COUT_TEST_BLOCK;
    EXPECT_TRUE(elaboration_fails("module top (a, b, y); input a, b; output y; assign y = a & b; endmodule", "logic expression"));
    EXPECT_TRUE(elaboration_fails("module top (a, y); input a; output y; assign y = a ? 1'b0 : 1'b1; endmodule", "logic expression"));
    EXPECT_TRUE(elaboration_fails("module top (a, y); input a; output y; assign y = b; endmodule", "not a declared signal"));
    EXPECT_TRUE(elaboration_fails("module top (a, y); input [1:0] a; output y; assign y = a[2]; endmodule", "invalid index"));
    EXPECT_TRUE(elaboration_fails("module top (a, y); input [1:0] a; output y; assign y = a[0][0]; endmodule", "fewer dimensions"));
    EXPECT_TRUE(elaboration_fails("module top (a, y); input a; output y; BUF g (.I(a), .I(a), .O(y)); endmodule", "connected twice"));
    EXPECT_TRUE(elaboration_fails("module top (a, y); input a; output y; BUF g (.I(a)); BUF g (.O(y)); endmodule", "used twice"));
    TEST_END
}

/**
     * Testing instances of modules and gates, parameters in every form, defparams, and attributes as typed values.
     */
TEST_F(VerilogElaborationTest, check_instances_parameters_attributes)
{
    TEST_START
    Design d = elaborate_text("(* top = 1, vendor = \"acme\" *)\n"
                              "module top (a, y);\n"
                              "  input a;\n"
                              "  (* keep, LOC = SLICE_X0Y0 *) output y;\n"
                              "  (* mark = 4'hA *) wire w;\n"
                              "  sub #(.W(16), .NAME(\"x\")) s0 (.i(a), .o(w));\n"
                              "  sub #(2, 3.5) s1 (a, y);\n"
                              "  (* placed *) BUF #(.INIT(8'hAB), .MASK(4'b1x0z), .N(-3), .F(1.5), .Q(W_UNKNOWN)) g (.I(w), .O());\n"
                              "  defparam s1.W = 32, g.N = 7;\n"
                              "endmodule\n"
                              "module sub #(parameter W = 8, parameter real R = 1.0, localparam L = 2, parameter NAME = \"n\") (input i, output o);\n"
                              "  BUF b (.I(i), .O(o));\n"
                              "endmodule");
    ASSERT_EQ(d.modules.size(), 2);
    EXPECT_EQ(d.top, "top");
    ASSERT_TRUE(d.find_top().is_ok());
    const netlist_ir::Module& top = *d.find_module("top");
    const netlist_ir::Module& sub = *d.find_module("sub");

    // module attributes and defaults
    ASSERT_NE(find_value(top.attributes, "top"), nullptr);
    EXPECT_EQ(find_value(top.attributes, "top")->declaration.get_type(), Parameter::Type::Integer);
    EXPECT_EQ(find_value(top.attributes, "vendor")->value, "acme");
    EXPECT_EQ(find_value(top.attributes, "vendor")->declaration.get_type(), Parameter::Type::String);
    ASSERT_EQ(sub.parameters.size(), 3);
    EXPECT_EQ(sub.parameters.at(0).declaration.get_name(), "W");
    EXPECT_EQ(sub.parameters.at(1).declaration.get_name(), "R");
    EXPECT_EQ(sub.parameters.at(1).declaration.get_type(), Parameter::Type::Float);
    EXPECT_EQ(sub.parameters.at(2).declaration.get_name(), "NAME");
    EXPECT_EQ(sub.parameters.at(2).value, "n");

    // port and signal attributes
    const netlist_ir::Port* y = top.find_port("y");
    ASSERT_NE(find_value(y->attributes, "keep"), nullptr);
    EXPECT_EQ(find_value(y->attributes, "keep")->declaration.get_type(), Parameter::Type::Boolean);
    EXPECT_EQ(find_value(y->attributes, "keep")->value, "true");
    EXPECT_EQ(find_value(y->attributes, "LOC")->value, "SLICE_X0Y0");
    EXPECT_EQ(find_value(y->attributes, "LOC")->declaration.get_type(), Parameter::Type::String);
    EXPECT_EQ(find_value(top.find_signal("w")->attributes, "mark")->value, "0xA");
    EXPECT_EQ(find_value(top.find_signal("w")->attributes, "mark")->declaration.get_size(), 4);

    // module instances: named and positional parameters, defparam override
    const netlist_ir::Instance* s0 = top.find_instance("s0");
    ASSERT_NE(s0, nullptr);
    EXPECT_EQ(s0->kind, InstanceKind::Module);
    ASSERT_EQ(s0->parameters.size(), 2);
    EXPECT_EQ(s0->parameters.at(0).declaration.get_name(), "W");
    EXPECT_EQ(s0->parameters.at(0).value, "16");
    EXPECT_EQ(s0->parameters.at(1).value, "x");
    ASSERT_EQ(s0->connections.size(), 2);
    EXPECT_EQ(s0->connections.at(0).port, "i");

    const netlist_ir::Instance* s1 = top.find_instance("s1");
    ASSERT_NE(s1, nullptr);
    ASSERT_EQ(s1->parameters.size(), 2);
    EXPECT_EQ(find_value(s1->parameters, "W")->value, "32");    // defparam wins
    EXPECT_EQ(find_value(s1->parameters, "R")->value, "3.5");
    ASSERT_EQ(s1->connections.size(), 2);
    EXPECT_TRUE(s1->connections.at(0).port.empty());

    // gate instance: typed values of every literal form, an open output
    const netlist_ir::Instance* g = top.find_instance("g");
    ASSERT_NE(g, nullptr);
    EXPECT_EQ(g->kind, InstanceKind::Gate);
    EXPECT_EQ(find_value(g->parameters, "INIT")->declaration.get_type(), Parameter::Type::BitVector);
    EXPECT_EQ(find_value(g->parameters, "INIT")->declaration.get_size(), 8);
    EXPECT_EQ(find_value(g->parameters, "INIT")->value, "0xAB");
    EXPECT_EQ(find_value(g->parameters, "MASK")->declaration.get_type(), Parameter::Type::LogicVector);
    EXPECT_EQ(find_value(g->parameters, "MASK")->value, "0b1x0z");
    EXPECT_EQ(find_value(g->parameters, "N")->declaration.get_type(), Parameter::Type::Integer);
    EXPECT_EQ(find_value(g->parameters, "N")->value, "7");
    EXPECT_EQ(find_value(g->parameters, "F")->declaration.get_type(), Parameter::Type::Float);
    EXPECT_EQ(find_value(g->parameters, "Q")->value, "W_UNKNOWN");
    EXPECT_EQ(find_value(g->attributes, "placed")->value, "true");
    ASSERT_EQ(g->connections.size(), 1);    // .O() connects nothing
    EXPECT_EQ(g->connections.front().port, "I");

    NO_COUT_TEST_BLOCK;
    EXPECT_TRUE(elaboration_fails("module top (a); input a; BUF #(1) g (.I(a)); endmodule", "positional parameters"));
    EXPECT_TRUE(elaboration_fails("module top (a); input a; sub #(1, 2) s (.i(a)); endmodule module sub #(parameter W = 1) (input i); endmodule", "more positional parameters"));
    EXPECT_TRUE(elaboration_fails("module top (a); input a; BUF g (.I(a)); defparam h.X = 1; endmodule", "does not exist"));
    EXPECT_TRUE(elaboration_fails("module top (a); input a; BUF g (.I(a)); defparam g.x.X = 1; endmodule", "hierarchical"));
    EXPECT_TRUE(elaboration_fails("(* top = 1 *) module a; endmodule (* top = 1 *) module b; endmodule", "more than one module is marked"));
    EXPECT_TRUE(elaboration_fails("module a; endmodule module a; endmodule", "declared twice"));
    EXPECT_TRUE(elaboration_fails("module top (a); input a; (* k, k *) BUF g (.I(a)); endmodule", "given twice"));
    TEST_END
}
}    // namespace hal
