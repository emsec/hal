#include "verilog_parser/verilog_syntax.h"

#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace verilog;
    using ast::Expr;

    class VerilogSyntaxTest : public ::testing::Test
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

        ast::SourceFile parse(const std::string& text)
        {
            auto res = parse_string(text, "test.v");
            if (res.is_error())
            {
                ADD_FAILURE() << res.get_error().get();
                return {};
            }
            return res.get();
        }

        /**
         * Render an expression back to text, in a canonical form that makes the tree visible.
         */
        static std::string show(const Expr& e)
        {
            switch (e.kind)
            {
                case Expr::Kind::Empty:
                    return "<empty>";
                case Expr::Kind::Identifier:
                    return e.text;
                case Expr::Kind::Number:
                    return e.text;
                case Expr::Kind::String:
                    return "\"" + e.text + "\"";
                case Expr::Kind::Index:
                    return show(e.children.at(0)) + "[" + show(e.children.at(1)) + "]";
                case Expr::Kind::Slice:
                    return show(e.children.at(0)) + "[" + show(e.children.at(1)) + ":" + show(e.children.at(2)) + "]";
                case Expr::Kind::Concat: {
                    std::string s = "{";
                    for (u32 i = 0; i < e.children.size(); i++)
                    {
                        s += (i ? "," : "") + show(e.children.at(i));
                    }
                    return s + "}";
                }
                case Expr::Kind::Replicate: {
                    std::string s = "{" + show(e.children.at(0)) + "{";
                    for (u32 i = 1; i < e.children.size(); i++)
                    {
                        s += (i > 1 ? "," : "") + show(e.children.at(i));
                    }
                    return s + "}}";
                }
                case Expr::Kind::Unary:
                    return "(" + e.text + show(e.children.at(0)) + ")";
                case Expr::Kind::Binary:
                    return "(" + show(e.children.at(0)) + e.text + show(e.children.at(1)) + ")";
                case Expr::Kind::Conditional:
                    return "(" + show(e.children.at(0)) + "?" + show(e.children.at(1)) + ":" + show(e.children.at(2)) + ")";
            }
            return "?";
        }

        static std::string show(const ast::Range& r)
        {
            return "[" + show(r.left) + ":" + show(r.right) + "]";
        }
    };

    /**
     * Testing the two header styles, header port expressions, and the declarations of the body.
     */
    TEST_F(VerilogSyntaxTest, check_headers_and_declarations){TEST_START{auto file = parse("module top (a, b, .sum({\\<const0> ,\\^sum [1:0]}), .alias(x));\n"
                                                                                           "  input a, b;\n"
                                                                                           "  output [1:0] \\^sum ;\n"
                                                                                           "  output \\<const0> ;\n"
                                                                                           "  wire x;\n"
                                                                                           "  wire [3:0] v, w [1:0];\n"
                                                                                           "  tri t;\n"
                                                                                           "  supply0 gnd;\n"
                                                                                           "  wire d = a;\n"
                                                                                           "  input signed [7:0] s;\n"
                                                                                           "endmodule : top");
    ASSERT_EQ(file.modules.size(), 1);
    const ast::Module& m = file.modules.front();
    EXPECT_EQ(m.name, "top");
    EXPECT_FALSE(m.ansi_ports);
    ASSERT_EQ(m.header_ports.size(), 4);
    EXPECT_EQ(m.header_ports.at(0).name, "a");
    EXPECT_FALSE(m.header_ports.at(0).expression.has_value());
    EXPECT_EQ(m.header_ports.at(2).name, "sum");
    ASSERT_TRUE(m.header_ports.at(2).expression.has_value());
    EXPECT_EQ(show(m.header_ports.at(2).expression.value()), "{<const0>,^sum[1:0]}");
    EXPECT_EQ(show(m.header_ports.at(3).expression.value()), "x");

    ASSERT_EQ(m.ports.size(), 5);
    EXPECT_EQ(m.ports.at(0).name, "a");
    EXPECT_EQ(m.ports.at(0).direction, PinDirection::input);
    EXPECT_EQ(m.ports.at(1).name, "b");
    EXPECT_EQ(m.ports.at(2).name, "^sum");
    EXPECT_EQ(m.ports.at(2).direction, PinDirection::output);
    ASSERT_EQ(m.ports.at(2).packed_dims.size(), 1);
    EXPECT_EQ(show(m.ports.at(2).packed_dims.front()), "[1:0]");
    EXPECT_EQ(m.ports.at(3).name, "<const0>");
    EXPECT_EQ(m.ports.at(4).name, "s");
    EXPECT_EQ(show(m.ports.at(4).packed_dims.front()), "[7:0]");

    ASSERT_EQ(m.nets.size(), 6);
    EXPECT_EQ(m.nets.at(0).name, "x");
    EXPECT_EQ(m.nets.at(0).net_type, "wire");
    EXPECT_EQ(m.nets.at(1).name, "v");
    EXPECT_EQ(show(m.nets.at(1).packed_dims.front()), "[3:0]");
    EXPECT_TRUE(m.nets.at(1).unpacked_dims.empty());
    EXPECT_EQ(m.nets.at(2).name, "w");
    EXPECT_EQ(show(m.nets.at(2).packed_dims.front()), "[3:0]");
    ASSERT_EQ(m.nets.at(2).unpacked_dims.size(), 1);
    EXPECT_EQ(show(m.nets.at(2).unpacked_dims.front()), "[1:0]");
    EXPECT_EQ(m.nets.at(3).net_type, "tri");
    EXPECT_EQ(m.nets.at(4).net_type, "supply0");
    ASSERT_TRUE(m.nets.at(5).initializer.has_value());
    EXPECT_EQ(show(m.nets.at(5).initializer.value()), "a");
    EXPECT_EQ(m.nets.at(5).location.line, 9);
}    // namespace hal
{
    auto file = parse("module top #(parameter W = 8, D = 2, localparam H = W / 2) (input wire a, b, output reg [W-1:0] y, inout io, input [1:0] p, q);\n"
                      "  parameter integer X = 3;\n"
                      "  localparam Y = X + 1, Z = \"str\";\n"
                      "endmodule");
    ASSERT_EQ(file.modules.size(), 1);
    const ast::Module& m = file.modules.front();
    EXPECT_TRUE(m.ansi_ports);
    ASSERT_EQ(m.parameters.size(), 6);
    EXPECT_EQ(m.parameters.at(0).name, "W");
    EXPECT_EQ(show(m.parameters.at(0).value), "8");
    EXPECT_FALSE(m.parameters.at(0).is_local);
    EXPECT_EQ(m.parameters.at(1).name, "D");
    EXPECT_EQ(m.parameters.at(2).name, "H");
    EXPECT_TRUE(m.parameters.at(2).is_local);
    EXPECT_EQ(show(m.parameters.at(2).value), "(W/2)");
    EXPECT_EQ(m.parameters.at(3).name, "X");
    EXPECT_EQ(show(m.parameters.at(4).value), "(X+1)");
    EXPECT_EQ(show(m.parameters.at(5).value), "\"str\"");

    ASSERT_EQ(m.ports.size(), 6);
    ASSERT_EQ(m.header_ports.size(), 6);
    EXPECT_EQ(m.ports.at(0).name, "a");
    EXPECT_EQ(m.ports.at(0).net_type, "wire");
    EXPECT_EQ(m.ports.at(1).name, "b");
    EXPECT_EQ(m.ports.at(1).direction, PinDirection::input);
    EXPECT_EQ(m.ports.at(1).net_type, "wire");
    EXPECT_EQ(m.ports.at(2).name, "y");
    EXPECT_EQ(m.ports.at(2).direction, PinDirection::output);
    EXPECT_EQ(m.ports.at(2).net_type, "reg");
    EXPECT_EQ(show(m.ports.at(2).packed_dims.front()), "[(W-1):0]");
    EXPECT_EQ(m.ports.at(3).direction, PinDirection::inout);
    EXPECT_TRUE(m.ports.at(3).packed_dims.empty());
    EXPECT_EQ(m.ports.at(5).name, "q");
    EXPECT_EQ(show(m.ports.at(5).packed_dims.front()), "[1:0]");
}
{
    auto file = parse("module a; endmodule module b (); endmodule");
    ASSERT_EQ(file.modules.size(), 2);
    EXPECT_EQ(file.modules.at(1).name, "b");
    EXPECT_TRUE(file.modules.at(1).header_ports.empty());
}
TEST_END
}

/**
     * Testing instantiations with parameters and both connection styles, assignments, defparams, and attributes.
     */
TEST_F(VerilogSyntaxTest, check_body_items)
{
    TEST_START
    auto file = parse("(* top = 1, keep *) module top (a, y);\n"
                      "  input a; output y;\n"
                      "  (* note = \"a//b\" *) wire w;\n"
                      "  BUF g0 (.I(a), .O(w));\n"
                      "  (* keep *) AND2 #(.P(1'b1), .S(\"s\")) g1 (w, , y), g2 ();\n"
                      "  sub #(3, 4) s (.i({{2{a}}, w[0], w[3:1]}), .o());\n"
                      "  BUF #5 delayed (.I(a), .O());\n"
                      "  assign {x, y} = {p, q}, z = 1'b0;\n"
                      "  assign v = a & b;\n"
                      "  defparam s.W = 8, g1.Q = 2;\n"
                      "  (* attr = 4'b1010 *) assign k = a[1];\n"
                      "endmodule");
    ASSERT_EQ(file.modules.size(), 1);
    const ast::Module& m = file.modules.front();

    ASSERT_EQ(m.attributes.size(), 2);
    EXPECT_EQ(m.attributes.at(0).name, "top");
    EXPECT_EQ(show(m.attributes.at(0).value.value()), "1");
    EXPECT_EQ(m.attributes.at(1).name, "keep");
    EXPECT_FALSE(m.attributes.at(1).value.has_value());
    ASSERT_EQ(m.nets.size(), 1);
    ASSERT_EQ(m.nets.front().attributes.size(), 1);
    EXPECT_EQ(show(m.nets.front().attributes.front().value.value()), "\"a//b\"");

    ASSERT_EQ(m.instantiations.size(), 5);
    const ast::Instantiation& g0 = m.instantiations.at(0);
    EXPECT_EQ(g0.type, "BUF");
    EXPECT_EQ(g0.name, "g0");
    ASSERT_EQ(g0.connections.size(), 2);
    EXPECT_EQ(g0.connections.at(0).port, "I");
    EXPECT_EQ(show(g0.connections.at(0).expr), "a");
    EXPECT_EQ(g0.location.line, 4);

    const ast::Instantiation& g1 = m.instantiations.at(1);
    EXPECT_EQ(g1.type, "AND2");
    ASSERT_EQ(g1.attributes.size(), 1);
    ASSERT_EQ(g1.parameters.size(), 2);
    EXPECT_EQ(g1.parameters.at(0).name, "P");
    EXPECT_EQ(show(g1.parameters.at(0).value), "1'b1");
    EXPECT_EQ(show(g1.parameters.at(1).value), "\"s\"");
    ASSERT_EQ(g1.connections.size(), 3);
    EXPECT_TRUE(g1.connections.at(0).port.empty());
    EXPECT_EQ(show(g1.connections.at(0).expr), "w");
    EXPECT_TRUE(g1.connections.at(1).expr.is_empty());
    EXPECT_EQ(show(g1.connections.at(2).expr), "y");

    const ast::Instantiation& g2 = m.instantiations.at(2);
    EXPECT_EQ(g2.name, "g2");
    EXPECT_EQ(g2.type, "AND2");
    EXPECT_EQ(g2.parameters.size(), 2);    // shared with g1
    EXPECT_TRUE(g2.connections.empty());

    const ast::Instantiation& s = m.instantiations.at(3);
    ASSERT_EQ(s.parameters.size(), 2);
    EXPECT_TRUE(s.parameters.at(0).name.empty());
    EXPECT_EQ(show(s.parameters.at(1).value), "4");
    ASSERT_EQ(s.connections.size(), 2);
    EXPECT_EQ(show(s.connections.at(0).expr), "{{2{a}},w[0],w[3:1]}");
    EXPECT_TRUE(s.connections.at(1).expr.is_empty());

    EXPECT_EQ(m.instantiations.at(4).name, "delayed");

    ASSERT_EQ(m.assignments.size(), 4);
    EXPECT_EQ(show(m.assignments.at(0).lhs), "{x,y}");
    EXPECT_EQ(show(m.assignments.at(0).rhs), "{p,q}");
    EXPECT_EQ(show(m.assignments.at(1).lhs), "z");
    EXPECT_EQ(show(m.assignments.at(1).rhs), "1'b0");
    EXPECT_EQ(show(m.assignments.at(2).rhs), "(a&b)");
    ASSERT_EQ(m.assignments.at(3).attributes.size(), 1);
    EXPECT_EQ(show(m.assignments.at(3).attributes.front().value.value()), "4'b1010");
    EXPECT_EQ(show(m.assignments.at(3).lhs), "k");

    ASSERT_EQ(m.defparams.size(), 2);
    EXPECT_EQ(m.defparams.at(0).path, (std::vector<std::string>{"s", "W"}));
    EXPECT_EQ(show(m.defparams.at(0).value), "8");
    EXPECT_EQ(m.defparams.at(1).path, (std::vector<std::string>{"g1", "Q"}));
    TEST_END
}

/**
     * Testing the expression grammar: precedence, associativity, unary operators, the conditional, nested
     * concatenations and replications, and chained index suffixes.
     */
TEST_F(VerilogSyntaxTest, check_expressions)
{
    TEST_START
    auto file = parse("module m; assign x = a + b * c - d;"
                      " assign y = a & b | c ^ d == e;"
                      " assign z = ~a ? b : c ? d : e;"
                      " assign w = {a, {b, c}, {3{d, e}}};"
                      " assign v = m[1][2:0];"
                      " assign u = (a + b) * -c;"
                      " assign t = a.b.c;"
                      " assign s = 1 << 2 >> 3; endmodule");
    ASSERT_EQ(file.modules.size(), 1);
    const auto& as = file.modules.front().assignments;
    ASSERT_EQ(as.size(), 8);
    EXPECT_EQ(show(as.at(0).rhs), "((a+(b*c))-d)");
    EXPECT_EQ(show(as.at(1).rhs), "((a&b)|(c^(d==e)))");
    EXPECT_EQ(show(as.at(2).rhs), "((~a)?b:(c?d:e))");
    EXPECT_EQ(show(as.at(3).rhs), "{a,{b,c},{3{d,e}}}");
    EXPECT_EQ(show(as.at(4).rhs), "m[1][2:0]");
    EXPECT_EQ(show(as.at(5).rhs), "((a+b)*(-c))");
    EXPECT_EQ(show(as.at(6).rhs), "a.b.c");
    EXPECT_EQ(show(as.at(7).rhs), "((1<<2)>>3)");
    TEST_END
}

/**
     * Testing that behavioral constructs and malformed input are rejected with an error that names the place.
     */
TEST_F(VerilogSyntaxTest, check_errors)
{
    TEST_START
    NO_COUT_TEST_BLOCK;
    const auto rejects = [](const std::string& text, const std::string& needle) {
        auto res = parse_string(text, "t.v");
        EXPECT_TRUE(res.is_error()) << "accepted: " << text;
        if (res.is_error())
        {
            EXPECT_NE(res.get_error().get().find(needle), std::string::npos) << res.get_error().get();
        }
    };
    rejects("", "no module");
    rejects("wire a;", "expected 'module'");
    rejects("module ( a );\nendmodule", "module name");
    rejects("module m;\n  initial begin y = 1'b0; end\nendmodule", "'initial'");
    rejects("module m;\n  always @(posedge a) y <= a;\nendmodule", "'always'");
    rejects("module m;\n  generate if (1) begin end endgenerate\nendmodule", "'generate'");
    rejects("module m;\n  function f; input x; f = x; endfunction\nendmodule", "'function'");
    rejects("module m;\n  specify (a => y) = 1; endspecify\nendmodule", "'specify'");
    rejects("primitive p (o, i); endprimitive", "primitive");
    rejects("module m;\n  BUF g (.I(a), b);\nendmodule", "mixed");
    rejects("module m;\n  BUF g[1:0] (.I(a));\nendmodule", "arrays of instances");
    rejects("module m;\n  wire [3 0] a;\nendmodule", "line 2");
    rejects("module m;\n  assign a = ;\nendmodule", "expected an expression");
    rejects("module m;\n  BUF g (.I(a)\nendmodule", "expected ')'");
    rejects("module m;\n  assign a = $random;\nendmodule", "system functions");
    rejects("module m;\n  wire a;\n", "expected 'endmodule'");
    rejects("module m;\n  defparam W = 1;\nendmodule", "instance.parameter");
    TEST_END
}
}    // namespace hal
