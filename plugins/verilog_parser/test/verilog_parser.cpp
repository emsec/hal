#include "verilog_parser/verilog_parser.h"

#include "netlist_test_utils.h"
#include "gate_library_test_utils.h"

#include "hal_core/utilities/enums.h"

#include <bitset>
#include <cstdlib>
#include <filesystem>

namespace hal {

    class VerilogParserTest : public ::testing::Test {
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

        /**
         * Parse and instantiate a Verilog netlist given as a string against the test gate library.
         */
        Result<std::unique_ptr<Netlist>> parse(const std::string& netlist, const std::string& file_name = "netlist.v")
        {
            VerilogParser parser;
            return parser.parse_and_instantiate(test_utils::create_sandbox_file(file_name, netlist), test_utils::get_gate_library());
        }

        /**
         * The type name and value of a typed parameter, or two empty strings if there is none. Replaces the legacy
         * `get_data("generic", ...)` assertions: the parsers write typed parameters now.
         */
        static std::tuple<std::string, std::string> parameter_of(const DataContainer* c, const std::string& name)
        {
            if (!c->has_parameter(name))
            {
                return std::make_tuple("", "");
            }
            return std::make_tuple(enum_to_string(c->get_parameter_declaration(name).get().get_type()), c->get_parameter_value(name).get());
        }

        /**
         * The type name and value of a typed attribute, or two empty strings if there is none.
         */
        static std::tuple<std::string, std::string> attribute_of(const DataContainer* c, const std::string& name)
        {
            if (!c->has_parameter(name, Parameter::Source::Attribute))
            {
                return std::make_tuple("", "");
            }
            return std::make_tuple(enum_to_string(c->get_parameter_declaration(name, Parameter::Source::Attribute).get().get_type()), c->get_parameter_value(name, Parameter::Source::Attribute).get());
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
    };

    /*                                    net_0
     *                  .--= INV (0) =------.
     *  net_global_in   |                   '-=               net_global_out
     *      ------------|                   .-= AND3 (2) = ----------
     *                  |                   | =
     *                  +--=                |
     *                  |    AND2 (1) =-----'
     *                  '--=              net_1
     */
    /**
     * Testing the correct usage of the verilog parser by parse a small verilog-format string, which describes the netlist
     * shown above.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_main_example) 
    {
        TEST_START
        {
            std::string netlist_input("module top ("
                                    "  net_global_in,"
                                    "  net_global_out "
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "BUF gate_0 ("
                                    "  .I (net_global_in ),"
                                    "  .O (net_0 )"
                                    " ) ;"
                                    "AND2 gate_1 ("
                                    "  .I0 (net_global_in ),"
                                    "  .I1 (net_global_in ),"
                                    "  .O (net_1 )"
                                    " ) ;"
                                    "AND3 gate_2 ("
                                    "  .I0 (net_0 ),"
                                    "  .I1 (net_1 ),"
                                    "  .O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            // Check if the gates are parsed correctly
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("BUF")).size(), 1);
            Gate* gate_0 = *(nl->get_gates(test_utils::gate_type_filter("BUF")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("AND2")).size(), 1);
            Gate* gate_1 = *(nl->get_gates(test_utils::gate_type_filter("AND2")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("AND3")).size(), 1);
            Gate* gate_2 = *(nl->get_gates(test_utils::gate_type_filter("AND3")).begin());

            ASSERT_NE(gate_0, nullptr);
            EXPECT_EQ(gate_0->get_name(), "gate_0");

            ASSERT_NE(gate_1, nullptr);
            EXPECT_EQ(gate_1->get_name(), "gate_1");

            ASSERT_NE(gate_2, nullptr);
            EXPECT_EQ(gate_2->get_name(), "gate_2");

            // Check if the nets are parsed correctly
            Net* net_0 = *(nl->get_nets(test_utils::net_name_filter("net_0")).begin());
            Net* net_1 = *(nl->get_nets(test_utils::net_name_filter("net_1")).begin());
            Net*
                net_global_in = *(nl->get_nets(test_utils::net_name_filter("net_global_in")).begin());
            Net*
                net_global_out = *(nl->get_nets(test_utils::net_name_filter("net_global_out")).begin());

            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "net_0");
            ASSERT_EQ(net_0->get_sources().size(), 1);
            EXPECT_EQ(net_0->get_sources()[0], test_utils::get_endpoint(gate_0, "O"));
            std::vector<Endpoint*> exp_net_0_dsts = {test_utils::get_endpoint(gate_2, "I0")};
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_0->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_2,
                                                                                                                "I0")})));

            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "net_1");
            ASSERT_EQ(net_1->get_sources().size(), 1);
            EXPECT_EQ(net_1->get_sources()[0], test_utils::get_endpoint(gate_1, "O"));
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_1->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_2,
                                                                                                                "I1")})));

            ASSERT_NE(net_global_in, nullptr);
            EXPECT_EQ(net_global_in->get_name(), "net_global_in");
            EXPECT_EQ(net_global_in->get_sources().size(), 0);
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_global_in->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_0,
                                                                                                                "I"),
                                                                                        test_utils::get_endpoint(gate_1,
                                                                                                                "I0"),
                                                                                        test_utils::get_endpoint(gate_1,
                                                                                                                "I1")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "net_global_out");
            ASSERT_EQ(net_global_out->get_sources().size(), 1);
            EXPECT_EQ(net_global_out->get_sources()[0], test_utils::get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_destinations().empty());
            EXPECT_TRUE(nl->is_global_output_net(net_global_out));

            EXPECT_EQ(nl->get_global_input_nets().size(), 1);
            EXPECT_EQ(nl->get_global_output_nets().size(), 1);
        }
        TEST_END
    }

    /**
     * The same test, as the main example, but use white spaces of different types (' ','\n','\t') in various locations (or remove some unnecessary ones)
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_whitespace_chaos) 
    {
        TEST_START
        {
             std::string netlist_input("module top(net_global_in,net_global_out);input net_global_in;\n"
                                    "  output net_global_out;wire net_0;wire net_1;BUF gate_0 (\n"
                                    "  .I (net_global_in),\n"
                                    "\n"
                                    " \t.O (net_0 )\n"
                                    " );\n"
                                    "AND2 gate_1 (.I0\n\t"
                                    "  (\n"
                                    "    net_global_in\n"
                                    "   \t)\n"
                                    "    ,\n"
                                    "  .I1 (net_global_in),.O (net_1));\n"
                                    "AND3 gate_2 (.I0 (net_0 ),.I1 (net_1 ),.O (net_global_out ));endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            // Check if the gates are parsed correctly
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("BUF")).size(), 1);
            Gate* gate_0 = *(nl->get_gates(test_utils::gate_type_filter("BUF")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("AND2")).size(), 1);
            Gate* gate_1 = *(nl->get_gates(test_utils::gate_type_filter("AND2")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("AND3")).size(), 1);
            Gate* gate_2 = *(nl->get_gates(test_utils::gate_type_filter("AND3")).begin());

            ASSERT_NE(gate_0, nullptr);
            EXPECT_EQ(gate_0->get_name(), "gate_0");

            ASSERT_NE(gate_1, nullptr);
            EXPECT_EQ(gate_1->get_name(), "gate_1");

            ASSERT_NE(gate_2, nullptr);
            EXPECT_EQ(gate_2->get_name(), "gate_2");

            // Check if the nets are parsed correctly
            Net* net_0 = *(nl->get_nets(test_utils::net_name_filter("net_0")).begin());
            Net* net_1 = *(nl->get_nets(test_utils::net_name_filter("net_1")).begin());
            Net* net_global_in = *(nl->get_nets(test_utils::net_name_filter("net_global_in")).begin());
            Net* net_global_out = *(nl->get_nets(test_utils::net_name_filter("net_global_out")).begin());

            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "net_0");
            ASSERT_EQ(net_0->get_sources().size(), 1);
            EXPECT_EQ(net_0->get_sources()[0], test_utils::get_endpoint(gate_0, "O"));
            std::vector<Endpoint*> exp_net_0_dsts = {test_utils::get_endpoint(gate_2, "I0")};
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_0->get_destinations(), std::vector<Endpoint*>({test_utils::get_endpoint(gate_2, "I0")})));

            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "net_1");
            ASSERT_EQ(net_1->get_sources().size(), 1);
            EXPECT_EQ(net_1->get_sources()[0], test_utils::get_endpoint(gate_1, "O"));
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_1->get_destinations(), std::vector<Endpoint*>({test_utils::get_endpoint(gate_2, "I1")})));

            ASSERT_NE(net_global_in, nullptr);
            EXPECT_EQ(net_global_in->get_name(), "net_global_in");
            EXPECT_EQ(net_global_in->get_sources().size(), 0);
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_global_in->get_destinations(), std::vector<Endpoint*>({test_utils::get_endpoint(gate_0, "I"),
                                                                                                                            test_utils::get_endpoint(gate_1, "I0"),
                                                                                                                            test_utils::get_endpoint(gate_1, "I1")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "net_global_out");
            ASSERT_EQ(net_global_out->get_sources().size(), 1);
            EXPECT_EQ(net_global_out->get_sources()[0], test_utils::get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_destinations().empty());
            EXPECT_TRUE(nl->is_global_output_net(net_global_out));

            EXPECT_EQ(nl->get_global_input_nets().size(), 1);
            EXPECT_EQ(nl->get_global_output_nets().size(), 1);
        }
        TEST_END
    }

    /**
     * Test different variants to declare pins.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_pins) 
    {
        TEST_START
        {
            const GateLibrary* gl = test_utils::get_gate_library();

            std::string netlist_input(  "module top (a, b, c, d); "
                                        "   input a, b;"
                                        "   output c, d;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   OR2 gate_1 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (d)"
                                        "   );"
                                        "endmodule");

            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            EXPECT_EQ(nl->get_gates().size(), 2);
            EXPECT_EQ(nl->get_nets().size(), 4);
            EXPECT_EQ(nl->get_modules().size(), 1);

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).size(), 1);
            Gate* gate_0 = nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).front();
            Endpoint* ep_g0_net_a = gate_0->get_fan_in_endpoint("I0");
            Endpoint* ep_g0_net_b = gate_0->get_fan_in_endpoint("I1");
            Endpoint* ep_g0_net_c = gate_0->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g0_net_a, nullptr);
            ASSERT_NE(ep_g0_net_b, nullptr);
            ASSERT_NE(ep_g0_net_c, nullptr);
            Net* net_g0_a = ep_g0_net_a->get_net();
            Net* net_g0_b = ep_g0_net_b->get_net();
            Net* net_g0_c = ep_g0_net_c->get_net();
            ASSERT_NE(net_g0_a, nullptr);
            ASSERT_NE(net_g0_b, nullptr);
            ASSERT_NE(net_g0_c, nullptr);
            EXPECT_EQ(net_g0_a->get_name(), "a");
            EXPECT_EQ(net_g0_b->get_name(), "b");
            EXPECT_EQ(net_g0_c->get_name(), "c");

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).size(), 1);
            Gate* gate_1 = nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).front();
            Endpoint* ep_g1_net_a = gate_1->get_fan_in_endpoint("I0");
            Endpoint* ep_g1_net_b = gate_1->get_fan_in_endpoint("I1");
            Endpoint* ep_g1_net_d = gate_1->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g1_net_a, nullptr);
            ASSERT_NE(ep_g1_net_b, nullptr);
            ASSERT_NE(ep_g1_net_d, nullptr);
            Net* net_g1_a = ep_g1_net_a->get_net();
            Net* net_g1_b = ep_g1_net_b->get_net();
            Net* net_g1_d = ep_g1_net_d->get_net();
            ASSERT_NE(net_g1_a, nullptr);
            ASSERT_NE(net_g1_b, nullptr);
            ASSERT_NE(net_g1_d, nullptr);
            EXPECT_EQ(net_g1_a->get_name(), "a");
            EXPECT_EQ(net_g1_b->get_name(), "b");
            EXPECT_EQ(net_g1_d->get_name(), "d");

            ASSERT_EQ(net_g0_a, net_g1_a);
            ASSERT_EQ(net_g0_b, net_g1_b);

            Net* net_a = net_g0_a;
            Net* net_b = net_g0_b;
            Net* net_c = net_g0_c;
            Net* net_d = net_g1_d;

            Module* top = nl->get_top_module();
            ASSERT_NE(top, nullptr);
            EXPECT_TRUE(top->is_input_net(net_a));
            EXPECT_TRUE(top->is_input_net(net_b));
            EXPECT_TRUE(top->is_output_net(net_c));
            EXPECT_TRUE(top->is_output_net(net_d));

            EXPECT_TRUE(net_a->is_global_input_net());
            EXPECT_TRUE(net_b->is_global_input_net());
            EXPECT_TRUE(net_c->is_global_output_net());
            EXPECT_TRUE(net_d->is_global_output_net());
        }
        {
            const GateLibrary* gl = test_utils::get_gate_library();

            std::string netlist_input(  "module top (input a, b, output c, d); "
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   OR2 gate_1 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (d)"
                                        "   );"
                                        "endmodule");

            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            EXPECT_EQ(nl->get_gates().size(), 2);
            EXPECT_EQ(nl->get_nets().size(), 4);
            EXPECT_EQ(nl->get_modules().size(), 1);

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).size(), 1);
            Gate* gate_0 = nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).front();
            Endpoint* ep_g0_net_a = gate_0->get_fan_in_endpoint("I0");
            Endpoint* ep_g0_net_b = gate_0->get_fan_in_endpoint("I1");
            Endpoint* ep_g0_net_c = gate_0->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g0_net_a, nullptr);
            ASSERT_NE(ep_g0_net_b, nullptr);
            ASSERT_NE(ep_g0_net_c, nullptr);
            Net* net_g0_a = ep_g0_net_a->get_net();
            Net* net_g0_b = ep_g0_net_b->get_net();
            Net* net_g0_c = ep_g0_net_c->get_net();
            ASSERT_NE(net_g0_a, nullptr);
            ASSERT_NE(net_g0_b, nullptr);
            ASSERT_NE(net_g0_c, nullptr);
            EXPECT_EQ(net_g0_a->get_name(), "a");
            EXPECT_EQ(net_g0_b->get_name(), "b");
            EXPECT_EQ(net_g0_c->get_name(), "c");

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).size(), 1);
            Gate* gate_1 = nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).front();
            Endpoint* ep_g1_net_a = gate_1->get_fan_in_endpoint("I0");
            Endpoint* ep_g1_net_b = gate_1->get_fan_in_endpoint("I1");
            Endpoint* ep_g1_net_d = gate_1->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g1_net_a, nullptr);
            ASSERT_NE(ep_g1_net_b, nullptr);
            ASSERT_NE(ep_g1_net_d, nullptr);
            Net* net_g1_a = ep_g1_net_a->get_net();
            Net* net_g1_b = ep_g1_net_b->get_net();
            Net* net_g1_d = ep_g1_net_d->get_net();
            ASSERT_NE(net_g1_a, nullptr);
            ASSERT_NE(net_g1_b, nullptr);
            ASSERT_NE(net_g1_d, nullptr);
            EXPECT_EQ(net_g1_a->get_name(), "a");
            EXPECT_EQ(net_g1_b->get_name(), "b");
            EXPECT_EQ(net_g1_d->get_name(), "d");

            ASSERT_EQ(net_g0_a, net_g1_a);
            ASSERT_EQ(net_g0_b, net_g1_b);

            Net* net_a = net_g0_a;
            Net* net_b = net_g0_b;
            Net* net_c = net_g0_c;
            Net* net_d = net_g1_d;

            Module* top = nl->get_top_module();
            ASSERT_NE(top, nullptr);
            EXPECT_TRUE(top->is_input_net(net_a));
            EXPECT_TRUE(top->is_input_net(net_b));
            EXPECT_TRUE(top->is_output_net(net_c));
            EXPECT_TRUE(top->is_output_net(net_d));

            EXPECT_TRUE(net_a->is_global_input_net());
            EXPECT_TRUE(net_b->is_global_input_net());
            EXPECT_TRUE(net_c->is_global_output_net());
            EXPECT_TRUE(net_d->is_global_output_net());
        }
        {
            const GateLibrary* gl = test_utils::get_gate_library();

            std::string netlist_input(  "module top (input a, b, output c, d); "
                                        "   wire a, b;"
                                        "   wire c, d;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   OR2 gate_1 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (d)"
                                        "   );"
                                        "endmodule");

            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            EXPECT_EQ(nl->get_gates().size(), 2);
            EXPECT_EQ(nl->get_nets().size(), 4);
            EXPECT_EQ(nl->get_modules().size(), 1);

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).size(), 1);
            Gate* gate_0 = nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).front();
            Endpoint* ep_g0_net_a = gate_0->get_fan_in_endpoint("I0");
            Endpoint* ep_g0_net_b = gate_0->get_fan_in_endpoint("I1");
            Endpoint* ep_g0_net_c = gate_0->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g0_net_a, nullptr);
            ASSERT_NE(ep_g0_net_b, nullptr);
            ASSERT_NE(ep_g0_net_c, nullptr);
            Net* net_g0_a = ep_g0_net_a->get_net();
            Net* net_g0_b = ep_g0_net_b->get_net();
            Net* net_g0_c = ep_g0_net_c->get_net();
            ASSERT_NE(net_g0_a, nullptr);
            ASSERT_NE(net_g0_b, nullptr);
            ASSERT_NE(net_g0_c, nullptr);
            EXPECT_EQ(net_g0_a->get_name(), "a");
            EXPECT_EQ(net_g0_b->get_name(), "b");
            EXPECT_EQ(net_g0_c->get_name(), "c");

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).size(), 1);
            Gate* gate_1 = nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).front();
            Endpoint* ep_g1_net_a = gate_1->get_fan_in_endpoint("I0");
            Endpoint* ep_g1_net_b = gate_1->get_fan_in_endpoint("I1");
            Endpoint* ep_g1_net_d = gate_1->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g1_net_a, nullptr);
            ASSERT_NE(ep_g1_net_b, nullptr);
            ASSERT_NE(ep_g1_net_d, nullptr);
            Net* net_g1_a = ep_g1_net_a->get_net();
            Net* net_g1_b = ep_g1_net_b->get_net();
            Net* net_g1_d = ep_g1_net_d->get_net();
            ASSERT_NE(net_g1_a, nullptr);
            ASSERT_NE(net_g1_b, nullptr);
            ASSERT_NE(net_g1_d, nullptr);
            EXPECT_EQ(net_g1_a->get_name(), "a");
            EXPECT_EQ(net_g1_b->get_name(), "b");
            EXPECT_EQ(net_g1_d->get_name(), "d");

            ASSERT_EQ(net_g0_a, net_g1_a);
            ASSERT_EQ(net_g0_b, net_g1_b);

            Net* net_a = net_g0_a;
            Net* net_b = net_g0_b;
            Net* net_c = net_g0_c;
            Net* net_d = net_g1_d;

            Module* top = nl->get_top_module();
            ASSERT_NE(top, nullptr);
            EXPECT_TRUE(top->is_input_net(net_a));
            EXPECT_TRUE(top->is_input_net(net_b));
            EXPECT_TRUE(top->is_output_net(net_c));
            EXPECT_TRUE(top->is_output_net(net_d));

            EXPECT_TRUE(net_a->is_global_input_net());
            EXPECT_TRUE(net_b->is_global_input_net());
            EXPECT_TRUE(net_c->is_global_output_net());
            EXPECT_TRUE(net_d->is_global_output_net());
        }
        {
            const GateLibrary* gl = test_utils::get_gate_library();

            std::string netlist_input(  "module top (a, b, c, d); "
                                        "   input a, b;"
                                        "   output c, d;"
                                        "   wire a, b;"
                                        "   wire c, d;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   OR2 gate_1 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (d)"
                                        "   );"
                                        "endmodule");

            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            EXPECT_EQ(nl->get_gates().size(), 2);
            EXPECT_EQ(nl->get_nets().size(), 4);
            EXPECT_EQ(nl->get_modules().size(), 1);

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).size(), 1);
            Gate* gate_0 = nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).front();
            Endpoint* ep_g0_net_a = gate_0->get_fan_in_endpoint("I0");
            Endpoint* ep_g0_net_b = gate_0->get_fan_in_endpoint("I1");
            Endpoint* ep_g0_net_c = gate_0->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g0_net_a, nullptr);
            ASSERT_NE(ep_g0_net_b, nullptr);
            ASSERT_NE(ep_g0_net_c, nullptr);
            Net* net_g0_a = ep_g0_net_a->get_net();
            Net* net_g0_b = ep_g0_net_b->get_net();
            Net* net_g0_c = ep_g0_net_c->get_net();
            ASSERT_NE(net_g0_a, nullptr);
            ASSERT_NE(net_g0_b, nullptr);
            ASSERT_NE(net_g0_c, nullptr);
            EXPECT_EQ(net_g0_a->get_name(), "a");
            EXPECT_EQ(net_g0_b->get_name(), "b");
            EXPECT_EQ(net_g0_c->get_name(), "c");

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).size(), 1);
            Gate* gate_1 = nl->get_gates(test_utils::gate_filter("OR2", "gate_1")).front();
            Endpoint* ep_g1_net_a = gate_1->get_fan_in_endpoint("I0");
            Endpoint* ep_g1_net_b = gate_1->get_fan_in_endpoint("I1");
            Endpoint* ep_g1_net_d = gate_1->get_fan_out_endpoint("O");
            ASSERT_NE(ep_g1_net_a, nullptr);
            ASSERT_NE(ep_g1_net_b, nullptr);
            ASSERT_NE(ep_g1_net_d, nullptr);
            Net* net_g1_a = ep_g1_net_a->get_net();
            Net* net_g1_b = ep_g1_net_b->get_net();
            Net* net_g1_d = ep_g1_net_d->get_net();
            ASSERT_NE(net_g1_a, nullptr);
            ASSERT_NE(net_g1_b, nullptr);
            ASSERT_NE(net_g1_d, nullptr);
            EXPECT_EQ(net_g1_a->get_name(), "a");
            EXPECT_EQ(net_g1_b->get_name(), "b");
            EXPECT_EQ(net_g1_d->get_name(), "d");

            ASSERT_EQ(net_g0_a, net_g1_a);
            ASSERT_EQ(net_g0_b, net_g1_b);

            Net* net_a = net_g0_a;
            Net* net_b = net_g0_b;
            Net* net_c = net_g0_c;
            Net* net_d = net_g1_d;

            Module* top = nl->get_top_module();
            ASSERT_NE(top, nullptr);
            EXPECT_TRUE(top->is_input_net(net_a));
            EXPECT_TRUE(top->is_input_net(net_b));
            EXPECT_TRUE(top->is_output_net(net_c));
            EXPECT_TRUE(top->is_output_net(net_d));

            EXPECT_TRUE(net_a->is_global_input_net());
            EXPECT_TRUE(net_b->is_global_input_net());
            EXPECT_TRUE(net_c->is_global_output_net());
            EXPECT_TRUE(net_d->is_global_output_net());
        }
        TEST_END
    }

    /**
     * Test different variants of pin assignments to modules and gates.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_pin_assignments) {

        TEST_START
            {   // test gate pin assignment by name
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module top (a, b, c); "
                                        "   input a, b;"
                                        "   output c;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   AND2 gate_1 ("
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   AND2 gate_2 ("
                                        "       .O (c)"
                                        "   );"
                                        "endmodule");
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).size(), 1);
                Gate* gate_0 = nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).front();
                EXPECT_EQ(gate_0->get_fan_in_endpoint("I0")->get_net()->get_name(), "a");
                EXPECT_EQ(gate_0->get_fan_in_endpoint("I1")->get_net()->get_name(), "b");
                EXPECT_EQ(gate_0->get_fan_out_endpoint("O")->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_1")).size(), 1);
                Gate* gate_1 = nl->get_gates(test_utils::gate_filter("AND2", "gate_1")).front();
                EXPECT_EQ(gate_1->get_fan_in_endpoint("I0"), nullptr);
                EXPECT_EQ(gate_1->get_fan_in_endpoint("I1")->get_net()->get_name(), "b");
                EXPECT_EQ(gate_1->get_fan_out_endpoint("O")->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_2")).size(), 1);
                Gate* gate_2 = nl->get_gates(test_utils::gate_filter("AND2", "gate_2")).front();
                EXPECT_EQ(gate_2->get_fan_in_endpoint("I0"), nullptr);
                EXPECT_EQ(gate_2->get_fan_in_endpoint("I1"), nullptr);
                EXPECT_EQ(gate_2->get_fan_out_endpoint("O")->get_net()->get_name(), "c");
            }
            {   // test gate pin assignment by name with empty assignments
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module top (a, b, c); "
                                        "   input a, b;"
                                        "   output c;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (a),"
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   AND2 gate_1 ("
                                        "       .I0 (),"
                                        "       .I1 (b),"
                                        "       .O (c)"
                                        "   );"
                                        ""
                                        "   AND2 gate_2 ("
                                        "       .I0 (),"
                                        "       .I1 (),"
                                        "       .O (c)"
                                        "   );"
                                        "endmodule");
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).size(), 1);
                Gate* gate_0 = nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).front();
                EXPECT_EQ(gate_0->get_fan_in_endpoint("I0")->get_net()->get_name(), "a");
                EXPECT_EQ(gate_0->get_fan_in_endpoint("I1")->get_net()->get_name(), "b");
                EXPECT_EQ(gate_0->get_fan_out_endpoint("O")->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_1")).size(), 1);
                Gate* gate_1 = nl->get_gates(test_utils::gate_filter("AND2", "gate_1")).front();
                EXPECT_EQ(gate_1->get_fan_in_endpoint("I0"), nullptr);
                EXPECT_EQ(gate_1->get_fan_in_endpoint("I1")->get_net()->get_name(), "b");
                EXPECT_EQ(gate_1->get_fan_out_endpoint("O")->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_2")).size(), 1);
                Gate* gate_2 = nl->get_gates(test_utils::gate_filter("AND2", "gate_2")).front();
                EXPECT_EQ(gate_2->get_fan_in_endpoint("I0"), nullptr);
                EXPECT_EQ(gate_2->get_fan_in_endpoint("I1"), nullptr);
                EXPECT_EQ(gate_2->get_fan_out_endpoint("O")->get_net()->get_name(), "c");
            }
            {   // test gate pin assignment by order
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module top (a, b, c); "
                                        "   input a, b;"
                                        "   output c;"
                                        ""
                                        "   AND2 gate_0 (a, b, c);"
                                        ""
                                        "   AND2 gate_1 (a, b);"
                                        ""
                                        "   AND2 gate_2 (a);"
                                        "endmodule");
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).size(), 1);
                Gate* gate_0 = nl->get_gates(test_utils::gate_filter("AND2", "gate_0")).front();
                EXPECT_EQ(gate_0->get_fan_in_endpoint("I0")->get_net()->get_name(), "a");
                EXPECT_EQ(gate_0->get_fan_in_endpoint("I1")->get_net()->get_name(), "b");
                EXPECT_EQ(gate_0->get_fan_out_endpoint("O")->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_1")).size(), 1);
                Gate* gate_1 = nl->get_gates(test_utils::gate_filter("AND2", "gate_1")).front();
                EXPECT_EQ(gate_1->get_fan_in_endpoint("I0")->get_net()->get_name(), "a");
                EXPECT_EQ(gate_1->get_fan_in_endpoint("I1")->get_net()->get_name(), "b");
                EXPECT_EQ(gate_1->get_fan_out_endpoint("O"), nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND2", "gate_2")).size(), 1);
                Gate* gate_2 = nl->get_gates(test_utils::gate_filter("AND2", "gate_2")).front();
                EXPECT_EQ(gate_2->get_fan_in_endpoint("I0")->get_net()->get_name(), "a");
                EXPECT_EQ(gate_2->get_fan_in_endpoint("I1"), nullptr);
                EXPECT_EQ(gate_2->get_fan_out_endpoint("O"), nullptr);
            }
            {   // test module pin assignment by name
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module sub_mod (as, bs, cs);"
                                        "   input as, bs;"
                                        "   output cs;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (as),"
                                        "       .I1 (bs),"
                                        "       .O (cs)"
                                        "   );"
                                        "endmodule"
                                        "\n"
                                        "module top (a, b, c); "
                                        "   input a, b;"
                                        "   output c;"
                                        ""
                                        "   sub_mod inst_0 ("
                                        "       .as(a),"
                                        "       .bs(b),"
                                        "       .cs(c)"
                                        "   );"
                                        ""
                                        "   sub_mod inst_1 ("
                                        "       .bs(b),"
                                        "       .cs(c)"
                                        "   );"
                                        ""
                                        "   sub_mod inst_2 ("
                                        "       .cs(c)"
                                        "   );"
                                        "endmodule"
                                        );
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_0")).size(), 1);
                Module* inst_0 = nl->get_modules(test_utils::module_name_filter("inst_0")).front();
                const auto pins_0 = inst_0->get_pins();
                EXPECT_EQ(pins_0.size(), 3);
                EXPECT_EQ(pins_0.at(0)->get_name(), "as");
                EXPECT_EQ(pins_0.at(0)->get_net()->get_name(), "a");
                EXPECT_EQ(pins_0.at(1)->get_name(), "bs");
                EXPECT_EQ(pins_0.at(1)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_0.at(2)->get_name(), "cs");
                EXPECT_EQ(pins_0.at(2)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_1")).size(), 1);
                Module* inst_1 = nl->get_modules(test_utils::module_name_filter("inst_1")).front();
                const auto pins_1 = inst_1->get_pins();
                EXPECT_EQ(pins_1.size(), 2);
                EXPECT_EQ(pins_1.at(0)->get_name(), "bs");
                EXPECT_EQ(pins_1.at(0)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_1.at(1)->get_name(), "cs");
                EXPECT_EQ(pins_1.at(1)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_2")).size(), 1);
                Module* inst_2 = nl->get_modules(test_utils::module_name_filter("inst_2")).front();
                const auto pins_2 = inst_2->get_pins();
                EXPECT_EQ(pins_2.size(), 1);
                EXPECT_EQ(pins_2.at(0)->get_name(), "cs");
                EXPECT_EQ(pins_2.at(0)->get_net()->get_name(), "c");
            }
            {    // test module pin aliasing
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input("module sub_mod (as, .bs(bs_int), cs);"
                                          "   input as, bs_int;"
                                          "   output cs;"
                                          ""
                                          "   AND2 gate_0 ("
                                          "       .I0 (as),"
                                          "       .I1 (bs_int),"
                                          "       .O (cs)"
                                          "   );"
                                          "endmodule"
                                          "\n"
                                          "module top (a, b, c); "
                                          "   input a, b;"
                                          "   output c;"
                                          ""
                                          "   sub_mod inst_0 ("
                                          "       .as(a),"
                                          "       .bs(b),"
                                          "       .cs(c)"
                                          "   );"
                                          ""
                                          "   sub_mod inst_1 ("
                                          "       .bs(b),"
                                          "       .cs(c)"
                                          "   );"
                                          ""
                                          "   sub_mod inst_2 ("
                                          "       .cs(c)"
                                          "   );"
                                          "endmodule");

                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file           = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_0")).size(), 1);
                Module* inst_0    = nl->get_modules(test_utils::module_name_filter("inst_0")).front();
                const auto pins_0 = inst_0->get_pins();
                EXPECT_EQ(pins_0.size(), 3);
                EXPECT_EQ(pins_0.at(0)->get_name(), "as");
                EXPECT_EQ(pins_0.at(0)->get_net()->get_name(), "a");
                EXPECT_EQ(pins_0.at(1)->get_name(), "bs");
                EXPECT_EQ(pins_0.at(1)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_0.at(2)->get_name(), "cs");
                EXPECT_EQ(pins_0.at(2)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_1")).size(), 1);
                Module* inst_1    = nl->get_modules(test_utils::module_name_filter("inst_1")).front();
                const auto pins_1 = inst_1->get_pins();
                EXPECT_EQ(pins_1.size(), 2);
                EXPECT_EQ(pins_1.at(0)->get_name(), "bs");
                EXPECT_EQ(pins_1.at(0)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_1.at(1)->get_name(), "cs");
                EXPECT_EQ(pins_1.at(1)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_2")).size(), 1);
                Module* inst_2    = nl->get_modules(test_utils::module_name_filter("inst_2")).front();
                const auto pins_2 = inst_2->get_pins();
                EXPECT_EQ(pins_2.size(), 1);
                EXPECT_EQ(pins_2.at(0)->get_name(), "cs");
                EXPECT_EQ(pins_2.at(0)->get_net()->get_name(), "c");
            }
            {   // test module pin assignment by name with empty assignments
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module sub_mod (as, bs, cs);"
                                        "   input as, bs;"
                                        "   output cs;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (as),"
                                        "       .I1 (bs),"
                                        "       .O (cs)"
                                        "   );"
                                        "endmodule"
                                        "\n"
                                        "module top (a, b, c); "
                                        "   input a, b;"
                                        "   output c;"
                                        ""
                                        "   sub_mod inst_0 ("
                                        "       .as(a),"
                                        "       .bs(b),"
                                        "       .cs(c)"
                                        "   );"
                                        ""
                                        "   sub_mod inst_1 ("
                                        "       .as(),"
                                        "       .bs(b),"
                                        "       .cs(c)"
                                        "   );"
                                        ""
                                        "   sub_mod inst_2 ("
                                        "       .as(),"
                                        "       .bs(),"
                                        "       .cs(c)"
                                        "   );"
                                        "endmodule"
                                        );
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_0")).size(), 1);
                Module* inst_0 = nl->get_modules(test_utils::module_name_filter("inst_0")).front();
                const auto pins_0 = inst_0->get_pins();
                EXPECT_EQ(pins_0.size(), 3);
                EXPECT_EQ(pins_0.at(0)->get_name(), "as");
                EXPECT_EQ(pins_0.at(0)->get_net()->get_name(), "a");
                EXPECT_EQ(pins_0.at(1)->get_name(), "bs");
                EXPECT_EQ(pins_0.at(1)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_0.at(2)->get_name(), "cs");
                EXPECT_EQ(pins_0.at(2)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_1")).size(), 1);
                Module* inst_1 = nl->get_modules(test_utils::module_name_filter("inst_1")).front();
                const auto pins_1 = inst_1->get_pins();
                EXPECT_EQ(pins_1.size(), 2);
                EXPECT_EQ(pins_1.at(0)->get_name(), "bs");
                EXPECT_EQ(pins_1.at(0)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_1.at(1)->get_name(), "cs");
                EXPECT_EQ(pins_1.at(1)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_2")).size(), 1);
                Module* inst_2 = nl->get_modules(test_utils::module_name_filter("inst_2")).front();
                const auto pins_2 = inst_2->get_pins();
                EXPECT_EQ(pins_2.size(), 1);
                EXPECT_EQ(pins_2.at(0)->get_name(), "cs");
                EXPECT_EQ(pins_2.at(0)->get_net()->get_name(), "c");
            }
            {   // test module pin assignment by order
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module sub_mod (as, bs, cs);"
                                        "   input as, bs;"
                                        "   output cs;"
                                        ""
                                        "   AND2 gate_0 (as, bs, cs);"
                                        "endmodule"
                                        "\n"
                                        "module top (a, b, c); "
                                        "   input a, b;"
                                        "   output c;"
                                        ""
                                        "   sub_mod inst_0 (a, b, c);"
                                        ""
                                        "   sub_mod inst_1 (a, b);"
                                        ""
                                        "   sub_mod inst_2 (a);"
                                        "endmodule"
                                        );
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_0")).size(), 1);
                Module* inst_0 = nl->get_modules(test_utils::module_name_filter("inst_0")).front();
                const auto pins_0 = inst_0->get_pins();
                EXPECT_EQ(pins_0.size(), 3);
                EXPECT_EQ(pins_0.at(0)->get_name(), "as");
                EXPECT_EQ(pins_0.at(0)->get_net()->get_name(), "a");
                EXPECT_EQ(pins_0.at(1)->get_name(), "bs");
                EXPECT_EQ(pins_0.at(1)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_0.at(2)->get_name(), "cs");
                EXPECT_EQ(pins_0.at(2)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_1")).size(), 1);
                Module* inst_1 = nl->get_modules(test_utils::module_name_filter("inst_1")).front();
                const auto pins_1 = inst_1->get_pins();
                EXPECT_EQ(pins_1.size(), 2);
                EXPECT_EQ(pins_1.at(0)->get_name(), "as");
                EXPECT_EQ(pins_1.at(0)->get_net()->get_name(), "a");
                EXPECT_EQ(pins_1.at(1)->get_name(), "bs");
                EXPECT_EQ(pins_1.at(1)->get_net()->get_name(), "b");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_2")).size(), 1);
                Module* inst_2 = nl->get_modules(test_utils::module_name_filter("inst_2")).front();
                const auto pins_2 = inst_2->get_pins();
                EXPECT_EQ(pins_2.size(), 1);
                EXPECT_EQ(pins_2.at(0)->get_name(), "as");
                EXPECT_EQ(pins_2.at(0)->get_net()->get_name(), "a");
            }
            {   // test pass-through module
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module sub_sub_mod (ass, bss, css);"
                                        "   input ass, bss;"
                                        "   output css;"
                                        ""
                                        "   AND2 gate_0 ("
                                        "       .I0 (ass),"
                                        "       .I1 (bss),"
                                        "       .O (css)"
                                        "   );"
                                        "endmodule"
                                        "\n"
                                        "module sub_mod (as, bs, cs);"
                                        "   input as, bs;"
                                        "   output cs;"
                                        ""
                                        "   sub_sub_mod mid ("
                                        "       .ass (as),"
                                        "       .bss (bs),"
                                        "       .css (cs)"
                                        "   );"
                                        "endmodule"
                                        "\n"
                                        "module top (a, b, c); "
                                        "   input a, b;"
                                        "   output c;"
                                        ""
                                        "   sub_mod inst_0 ("
                                        "       .as(a),"
                                        "       .bs(b),"
                                        "       .cs(c)"
                                        "   );"
                                        ""
                                        "   sub_mod inst_1 ("
                                        "       .bs(b),"
                                        "       .cs(c)"
                                        "   );"
                                        ""
                                        "   sub_mod inst_2 ("
                                        "       .cs(c)"
                                        "   );"
                                        "endmodule"
                                        );
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_0")).size(), 1);
                Module* inst_0 = nl->get_modules(test_utils::module_name_filter("inst_0")).front();
                const auto pins_0 = inst_0->get_pins();
                EXPECT_EQ(pins_0.size(), 3);
                EXPECT_EQ(pins_0.at(0)->get_name(), "as");
                EXPECT_EQ(pins_0.at(0)->get_net()->get_name(), "a");
                EXPECT_EQ(pins_0.at(1)->get_name(), "bs");
                EXPECT_EQ(pins_0.at(1)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_0.at(2)->get_name(), "cs");
                EXPECT_EQ(pins_0.at(2)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_1")).size(), 1);
                Module* inst_1 = nl->get_modules(test_utils::module_name_filter("inst_1")).front();
                const auto pins_1 = inst_1->get_pins();
                EXPECT_EQ(pins_1.size(), 2);
                EXPECT_EQ(pins_1.at(0)->get_name(), "bs");
                EXPECT_EQ(pins_1.at(0)->get_net()->get_name(), "b");
                EXPECT_EQ(pins_1.at(1)->get_name(), "cs");
                EXPECT_EQ(pins_1.at(1)->get_net()->get_name(), "c");

                ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("inst_2")).size(), 1);
                Module* inst_2 = nl->get_modules(test_utils::module_name_filter("inst_2")).front();
                const auto pins_2 = inst_2->get_pins();
                EXPECT_EQ(pins_2.size(), 1);
                EXPECT_EQ(pins_2.at(0)->get_name(), "cs");
                EXPECT_EQ(pins_2.at(0)->get_net()->get_name(), "c");
            }
        TEST_END
    }

    /**
     * Testing the correct storage of data of the following data types:
     * integer, floating_point, string, bit_vector (hexadecimal, decimal, octal, binary)
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_parameters) 
    {
        TEST_START
        {
            // Store an instance of all possible data types in one Gate + some special cases
            std::string netlist_input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "BUF #("
                                    ".key_integer(1234),"
                                    ".key_floating_point(1.234),"
                                    ".key_string(\"test_string\"),"
                                    ".key_bit_vector_hex('habc),"    // All values are 'ABC' in hex
                                    ".key_bit_vector_dec('d2748),"
                                    ".key_bit_vector_oct('o5274),"
                                    ".key_bit_vector_bin('b1010_1011_1100),"
                                    ".key_negative_comma_string(\"test,1,2,3\"),"
                                    ".key_negative_float_string(\"1.234\")) "
                                    "gate_0 ("
                                    "  .I (global_in ),"
                                    "  .O (global_out )"
                                    " ) ;"
                                    "defparam gate_0.external_int = 3;"
                                    "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            // std::cout << nl_res.get_error().get() << std::endl;
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).size(), 1);
            Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).begin();

            // Integers are stored in their hex representation
            EXPECT_EQ(parameter_of(gate_0, "key_integer"), std::make_tuple("integer", "1234"));
            EXPECT_EQ(parameter_of(gate_0, "key_floating_point"), std::make_tuple("float", "1.234"));
            EXPECT_EQ(parameter_of(gate_0, "key_string"), std::make_tuple("string", "test_string"));
            EXPECT_EQ(parameter_of(gate_0, "key_bit_vector_hex"), std::make_tuple("bit_vector", "0xABC"));
            EXPECT_EQ(parameter_of(gate_0, "key_bit_vector_dec"), std::make_tuple("bit_vector", "0xABC"));
            EXPECT_EQ(parameter_of(gate_0, "key_bit_vector_oct"), std::make_tuple("bit_vector", "0xABC"));
            EXPECT_EQ(parameter_of(gate_0, "key_bit_vector_bin"), std::make_tuple("bit_vector", "0xABC"));
            EXPECT_EQ(parameter_of(gate_0, "external_int"), std::make_tuple("integer", "3"));

            // Special Characters
            EXPECT_EQ(parameter_of(gate_0, "key_negative_comma_string"), std::make_tuple("string", "test,1,2,3"));
            EXPECT_EQ(parameter_of(gate_0, "key_negative_float_string"), std::make_tuple("string", "1.234"));
        }
        {
            // Port map gets multiple nets (should only assign right-most one)
            std::string netlist_input("module top ("
                                    "  global_in,"
                                    "  global_out "
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "  wire net_0;"
                                    "BUF gate_0 ("
                                    "  .I ({net_0, global_in} )"
                                    " ) ;"
                                    "BUF gate_1 ("
                                    "  .I (net_0 ),"
                                    "  .O (global_out )"
                                    " ) ;"
                                    "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();

            Gate* gate;

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).empty());
            gate = *(nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).begin());

            ASSERT_NE(gate->get_fan_in_net("I"), nullptr);
            EXPECT_EQ(gate->get_fan_in_net("I")->get_name(), "global_in");
        }
        TEST_END
    }

    /**
     * Testing the handling of Net-vectors in dimension 1-3
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_net_vectors) {

        TEST_START
            {
                // Use two logic vectors with dimension 1. One is declared with ascending indices, the other with
                // descending indices. ([0:3] and [3:0])
                /*
                 *                           n_vec_1              n_vec_2
                 *                        =-----------=        =-----------=
                 *                        =-----------=        =-----------=
                 *  global_in ---= gate_0 =-----------= gate_1 =-----------= gate_2 =--- global_out
                 *                        =-----------=        =-----------=
                 *
                 */

                std::string netlist_input("module top ( "
                                        "  global_in, "
                                        "  global_out "
                                        " ) ; "
                                        "  input global_in ; "
                                        "  output global_out ; "
                                        "  wire [0:3] n_vec_1 ; "
                                        "  wire [3:0] n_vec_2 ; "
                                        " COMB14 gate_0 ( "
                                        "  .I (global_in ), "
                                        "  .O0 (n_vec_1[0]), "
                                        "  .O1 (n_vec_1[1]), "
                                        "  .O2 (n_vec_1[2]), "
                                        "  .O3 (n_vec_1[3]) "
                                        " ) ; "
                                        " COMB44 gate_1 ( "
                                        "   .I0 (n_vec_1[0]), "
                                        "   .I1 (n_vec_1[1]), "
                                        "   .I2 (n_vec_1[2]), "
                                        "   .I3 (n_vec_1[3]), "
                                        "   .O0 (n_vec_2[0]), "
                                        "   .O1 (n_vec_2[1]), "
                                        "   .O2 (n_vec_2[2]), "
                                        "   .O3 (n_vec_2[3]) "
                                        "  ) ; "
                                        "  COMB41 gate_2 ( "
                                        "    .I0 (n_vec_2[0]), "
                                        "    .I1 (n_vec_2[1]), "
                                        "    .I2 (n_vec_2[2]), "
                                        "    .I3 (n_vec_2[3]), "
                                        "    .O (global_out ) "
                                        "   ) ; "
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                // Check that all nets are created and connected correctly
                EXPECT_EQ(nl->get_nets().size(),
                          10);    // net_global_in + net_global_out + 4 nets in n_vec_1 + 4 nets in n_vec_2
                for (auto net_name : std::set<std::string>({"n_vec_1(0)", "n_vec_1(1)", "n_vec_1(2)", "n_vec_1(3)",
                                                            "n_vec_2(0)", "n_vec_2(1)", "n_vec_2(2)", "n_vec_2(3)"})) {
                    ASSERT_EQ(nl->get_nets(test_utils::net_name_filter(net_name)).size(), 1);
                }
                for (unsigned i = 0; i < 4; i++) {
                    std::string i_str = std::to_string(i);
                    Net* n_vec_1_i = *nl->get_nets(test_utils::net_name_filter("n_vec_1(" + i_str + ")")).begin();
                    Net* n_vec_2_i = *nl->get_nets(test_utils::net_name_filter("n_vec_2(" + i_str + ")")).begin();
                    ASSERT_EQ(n_vec_1_i->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_1_i->get_sources()[0]->get_pin()->get_name(), "O" + i_str);
                    EXPECT_EQ((*n_vec_1_i->get_destinations().begin())->get_pin()->get_name(), "I" + i_str);
                    ASSERT_EQ(n_vec_2_i->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_2_i->get_sources()[0]->get_pin()->get_name(), "O" + i_str);
                    EXPECT_EQ((*n_vec_2_i->get_destinations().begin())->get_pin()->get_name(), "I" + i_str);
                }
            }
            {
                // Use a logic vector of dimension two
                std::string netlist_input("module top ( "
                                        "  global_in, "
                                        "  global_out "
                                        " ) ; "
                                        "  input global_in ; "
                                        "  output global_out ; "
                                        "  wire [0:1][2:3] n_vec ; "
                                        "COMB14 gate_0 ( "
                                        "  .I (global_in ), "
                                        "  .O0 (n_vec[0][2]), "
                                        "  .O1 (n_vec[0][3]), "
                                        "  .O2 (n_vec[1][2]), "
                                        "  .O3 (n_vec[1][3]) "
                                        " ) ; "
                                        "COMB41 gate_1 ( "
                                        "  .I0 (n_vec[0][2]), "
                                        "  .I1 (n_vec[0][3]), "
                                        "  .I2 (n_vec[1][2]), "
                                        "  .I3 (n_vec[1][3]), "
                                        "  .O (global_out ) "
                                        " ) ; "
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                // Check that all nets are created and connected correctly
                EXPECT_EQ(nl->get_nets().size(), 6);    // net_global_in + global_out + 4 nets in n_vec
                unsigned pin = 0;
                for (auto n : std::vector<std::string>({"n_vec(0)(2)", "n_vec(0)(3)", "n_vec(1)(2)", "n_vec(1)(3)"})) {
                    ASSERT_FALSE(nl->get_nets(test_utils::net_name_filter(n)).empty());
                    Net* n_vec_i_j = *nl->get_nets(test_utils::net_name_filter(n)).begin();
                    ASSERT_EQ(n_vec_i_j->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_i_j->get_sources()[0]->get_pin()->get_name(), "O" + std::to_string(pin));
                    EXPECT_EQ((*n_vec_i_j->get_destinations().begin())->get_pin()->get_name(), "I" + std::to_string(pin));
                    pin++;
                }
            }
            {
                // Use a Net vector of dimension three
                std::string netlist_input("module top ( "
                                        "  global_in, "
                                        "  global_out "
                                        " ) ; "
                                        "  input global_in ; "
                                        "  output global_out ; "
                                        "  wire [0:1][1:0][0:1] n_vec ; "
                                        "COMB18 gate_0 ( "
                                        "  .I (global_in ), "
                                        "  .O0 (n_vec[0][0][0]), "
                                        "  .O1 (n_vec[0][0][1]), "
                                        "  .O2 (n_vec[0][1][0]), "
                                        "  .O3 (n_vec[0][1][1]), "
                                        "  .O4 (n_vec[1][0][0]), "
                                        "  .O5 (n_vec[1][0][1]), "
                                        "  .O6 (n_vec[1][1][0]), "
                                        "  .O7 (n_vec[1][1][1]) "
                                        " ) ; "
                                        "COMB81 gate_1 ( "
                                        "  .I0 (n_vec[0][0][0]), "
                                        "  .I1 (n_vec[0][0][1]), "
                                        "  .I2 (n_vec[0][1][0]), "
                                        "  .I3 (n_vec[0][1][1]), "
                                        "  .I4 (n_vec[1][0][0]), "
                                        "  .I5 (n_vec[1][0][1]), "
                                        "  .I6 (n_vec[1][1][0]), "
                                        "  .I7 (n_vec[1][1][1]), "
                                        "  .O (global_out ) "
                                        " ) ; "
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                // Check that all nets are created and connected correctly
                EXPECT_EQ(nl->get_nets().size(), 10);    // net_global_in + net_global_out + 8 nets in n_vec
                std::vector<std::string> net_idx
                    ({"(0)(0)(0)", "(0)(0)(1)", "(0)(1)(0)", "(0)(1)(1)", "(1)(0)(0)", "(1)(0)(1)", "(1)(1)(0)",
                      "(1)(1)(1)"});
                for (size_t idx = 0; idx < 8; idx++) 
                {
                    ASSERT_FALSE(nl->get_nets(test_utils::net_name_filter("n_vec" + net_idx[idx])).empty());
                    Net* n_vec_i_j = nl->get_nets(test_utils::net_name_filter("n_vec" + net_idx[idx])).front();
                    ASSERT_EQ(n_vec_i_j->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_i_j->get_sources()[0]->get_pin()->get_name(), "O" + std::to_string(idx));
                    EXPECT_EQ((*n_vec_i_j->get_destinations().begin())->get_pin()->get_name(), "I" + std::to_string(idx));
                }
            }
        TEST_END
    }

    /**
     * Test zero parsing and (implicit) zero padding.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_zero_padding) {

        TEST_START
            {
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module top ();\n "
                                        "   wire [3:0] gate_0_in, gate_1_in;\n"
                                        "   RAM gate_0 (\n "
                                        "       .DATA_IN (gate_0_in ) \n"
                                        "   ) ; \n"
                                        "   RAM gate_1 (\n "
                                        "       .DATA_IN (gate_1_in ) \n"
                                        "   ) ;\n "
                                        "   assign gate_0_in = 4'd0;\n"
                                        "   assign gate_1_in = 1'd0;\n"
                                        "endmodule");
                
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                ASSERT_TRUE(!nl->get_gnd_gates().empty());
                EXPECT_EQ(nl->get_gnd_gates().front()->get_successors().size(), 8);
                for (Gate* gate : nl->get_gates([](const Gate* gate) { return !gate->is_gnd_gate(); })) 
                {
                    EXPECT_EQ(gate->get_fan_in_endpoints().size(), 4);
                    for (Endpoint* ep : gate->get_fan_in_endpoints()) 
                    {
                        EXPECT_TRUE(ep->get_net()->is_gnd_net());
                    }
                }
            }
        TEST_END
    }

    /**
     * Testing assignment of '0', '1', 'Z', and 'X'.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_special_nets) {

        TEST_START
            {
                std::string netlist_input("module top ("
                                        "  global_out_0,"
                                        "  global_out_1,"
                                        "  global_out_2,"
                                        "  global_out_3 "
                                        " ) ;"
                                        "  output global_out_0 ;"
                                        "  output global_out_1 ;"
                                        "  output global_out_2 ;"
                                        "  output global_out_3 ;"
                                        "BUF gate_0 ("
                                        "  .I ('b0 ),"
                                        "  .O (global_out_0)"
                                        " ) ;"
                                        "BUF gate_1 ("
                                        "  .I ('b1 ),"
                                        "  .O (global_out_1)"
                                        " ) ;"
                                        "BUF gate_2 ("
                                        "  .I ('bZ ),"
                                        "  .O (global_out_2)"
                                        " ) ;"
                                        "BUF gate_3 ("
                                        "  .I ('bX ),"
                                        "  .O (global_out_3)"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);

                Gate* gate_0 = nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).front();
                ASSERT_NE(gate_0, nullptr);
                Gate* gate_1 = nl->get_gates(test_utils::gate_filter("BUF", "gate_1")).front();
                ASSERT_NE(gate_1, nullptr);
                Gate* gate_2 = nl->get_gates(test_utils::gate_filter("BUF", "gate_2")).front();
                ASSERT_NE(gate_2, nullptr);
                Gate* gate_3 = nl->get_gates(test_utils::gate_filter("BUF", "gate_3")).front();
                ASSERT_NE(gate_3, nullptr);

                // check whether net '0' was created and is connected to a GND gate through input pin "I"
                Net* net_gnd = gate_0->get_fan_in_net("I");
                ASSERT_NE(net_gnd, nullptr);
                EXPECT_EQ(net_gnd->get_name(), "'0'");
                ASSERT_EQ(net_gnd->get_sources().size(), 1);
                ASSERT_NE(net_gnd->get_sources()[0]->get_gate(), nullptr);
                EXPECT_TRUE(net_gnd->get_sources()[0]->get_gate()->is_gnd_gate());

                // check whether net '1' was created and is connected to a VCC gate through input pin "I"
                Net* net_vcc = gate_1->get_fan_in_net("I");
                ASSERT_NE(net_vcc, nullptr);
                EXPECT_EQ(net_vcc->get_name(), "'1'");
                ASSERT_EQ(net_vcc->get_sources().size(), 1);
                ASSERT_NE(net_vcc->get_sources()[0]->get_gate(), nullptr);
                EXPECT_TRUE(net_vcc->get_sources()[0]->get_gate()->is_vcc_gate());

                // check whether input pin "I" remains unconnected for 'Z' assignment
                ASSERT_EQ(gate_2->get_fan_in_net("I"), nullptr);

                // check whether input pin "I" remains unconnected for 'X' assignment
                ASSERT_EQ(gate_2->get_fan_in_net("I"), nullptr);
            }
        TEST_END
    }

    /**
     * Testing the usage of additional entities. Entities that are used by the main entity (the last one) are recursively
     * split in gates that are part of the Gate library, while the original entity hierarchy is represented by the Module-
     * hierarchy of the netlist. Therefore, there can be multiple gates with the same name, so names that occur twice or
     * more will be extended by a unique suffix.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_multiple_entities) 
    {
        TEST_START
        {
            // Create a new entity with an attribute that is used once by the main entity
            /*                               ---------------------------------------------.
            *                              | child_mod                                   |
            *                              |                                             |
            *  global_in ---=| gate_0 |=---=---=| gate_0_child |=---=| gate_1_child |=---=---=| gate_1 |=--- global_out
            *                              |                                             |
            *                              '---------------------------------------------'
            */
            std::string netlist_input("(* child_attribute = \"child_attribute_value\" *)"
                                    "module MODULE_CHILD ("
                                    "  child_in,"
                                    "  child_out"
                                    " ) ;"
                                    "  (* child_net_attribute = \"child_net_attribute_value\" *)"
                                    "  input child_in ;"
                                    "  output child_out ;"
                                    "  wire net_0_child ;"
                                    "BUF gate_0_child ("
                                    "  .I (child_in ),"
                                    "  .O (net_0_child )"
                                    " ) ;"
                                    "BUF gate_1_child ("
                                    "  .I (net_0_child ),"
                                    "  .O (child_out )"
                                    " ) ;"
                                    "endmodule"
                                    "\n"
                                    "module MODULE_TOP ("
                                    "  net_global_in,"
                                    "  net_global_out"
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "BUF gate_0 ("
                                    "  .I (net_global_in ),"
                                    "  .O (net_0 )"
                                    " ) ;"
                                    "MODULE_CHILD child_mod ("
                                    "  .\\child_in (net_0 ),"
                                    "  .\\child_out (net_1 )"
                                    " ) ;"
                                    "BUF gate_1 ("
                                    "  .I (net_1 ), "
                                    "  .O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            // check gates
            Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).begin();
            ASSERT_NE(gate_0, nullptr);
            Gate* gate_1 = *nl->get_gates(test_utils::gate_filter("BUF", "gate_1")).begin();
            ASSERT_NE(gate_1, nullptr);
            Gate* gate_0_child = *nl->get_gates(test_utils::gate_filter("BUF", "gate_0_child")).begin();
            ASSERT_NE(gate_0_child, nullptr);
            Gate* gate_1_child = *nl->get_gates(test_utils::gate_filter("BUF", "gate_1_child")).begin();
            ASSERT_NE(gate_1_child, nullptr);

            // check nets
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_1")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_in")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_out")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0_child")).size(), 1);
            Net* net_0 = *nl->get_nets(test_utils::net_name_filter("net_0")).begin();
            Net* net_1 = *nl->get_nets(test_utils::net_name_filter("net_1")).begin();
            Net* net_global_in = *nl->get_nets(test_utils::net_name_filter("net_global_in")).begin();
            Net* net_global_out = *nl->get_nets(test_utils::net_name_filter("net_global_out")).begin();
            Net* net_0_child = *nl->get_nets(test_utils::net_name_filter("net_0_child")).begin();

            // check connections
            EXPECT_EQ(gate_0->get_fan_in_net("I"), net_global_in);
            EXPECT_EQ(gate_0->get_fan_out_net("O"), net_0);
            EXPECT_EQ(gate_1->get_fan_in_net("I"), net_1);
            EXPECT_EQ(gate_1->get_fan_out_net("O"), net_global_out);
            EXPECT_EQ(gate_0_child->get_fan_in_net("I"), net_0);
            EXPECT_EQ(gate_0_child->get_fan_out_net("O"), net_0_child);
            EXPECT_EQ(gate_1_child->get_fan_in_net("I"), net_0_child);
            EXPECT_EQ(gate_1_child->get_fan_out_net("O"), net_1);

            // check modules
            Module* top_mod = nl->get_top_module();
            ASSERT_NE(top_mod, nullptr);
            EXPECT_TRUE(top_mod->is_top_module());
            EXPECT_EQ(top_mod->get_name(), "top_module");
            EXPECT_EQ(top_mod->get_type(), "MODULE_TOP");
            ASSERT_EQ(top_mod->get_submodules().size(), 1);
            Module* child_mod = *top_mod->get_submodules().begin();
            ASSERT_NE(child_mod, nullptr);
            EXPECT_EQ(child_mod->get_name(), "child_mod");
            EXPECT_EQ(child_mod->get_type(), "MODULE_CHILD");
            EXPECT_EQ(top_mod->get_gates(), std::vector<Gate*>({gate_0, gate_1}));
            EXPECT_EQ(child_mod->get_gates(), std::vector<Gate*>({gate_0_child, gate_1_child}));

            // check attributes
            EXPECT_EQ(attribute_of(net_0, "child_net_attribute"), std::make_tuple("string", "child_net_attribute_value"));
            EXPECT_EQ(attribute_of(child_mod, "child_attribute"), std::make_tuple("string", "child_attribute_value"));
        }
        {
            // Create a netlist with the following MODULE hierarchy (assigned gates in '()'):
            /*
                        *                               .---- CHILD_TWO --- (gate_child_two)
                        *                               |
                        *              .----- CHILD_ONE-+
                        *              |                |
                        *  TOP_MODULE -+                +---- CHILD_TWO --- (gate_child_two)
                        *              |                |
                        *              |                '---- (gate_child_one)
                        *              |
                        *              +----- CHILD_TWO --- (gate_child_two)
                        *              |
                        *              '---- (gate_top)
                        *
                        */
            // Testing the correct build of the Module hierarchy. Moreover the correct substitution of Gate and Net names,
            // which would be added twice (because an entity can be used multiple times) is tested as well.

            std::string netlist_input("module ENT_CHILD_TWO ( "
                                    "  I_c2, "
                                    "  O_c2 "
                                    " ) ; "
                                    "  input I_c2 ; "
                                    "  output O_c2 ; "
                                    "BUF gate_child_two ( "
                                    "  .I (I_c2 ), "
                                    "  .O (O_c2 ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module ENT_CHILD_ONE ( "
                                    "  I_c1, "
                                    "  O_c1 "
                                    " ) ; "
                                    "  input I_c1 ; "
                                    "  output O_c1 ; "
                                    "  wire net_child_0 ; "
                                    "  wire net_child_1 ; "
                                    "ENT_CHILD_TWO gate_0_ent_two ( "
                                    "  .\\I_c2 (I_c1 ), "
                                    "  .\\O_c2 (net_child_0 ) "
                                    " ) ; "
                                    "ENT_CHILD_TWO gate_1_ent_two ( "
                                    "  .\\I_c2 (net_child_0 ), "
                                    "  .\\O_c2 (net_child_1 ) "
                                    " ) ; "
                                    "BUF gate_child_one ( "
                                    "  .I (net_child_1 ), "
                                    "  .O (O_c1 ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module ENT_TOP ("
                                    "  net_global_in,"
                                    "  net_global_out"
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "ENT_CHILD_ONE #("
                                    "  .child_one_mod_key(1234)"
                                    ") child_one_mod ("
                                    "  .\\I_c1 (net_global_in ),"
                                    "  .\\O_c1 (net_0 )"
                                    " ) ;"
                                    "ENT_CHILD_TWO child_two_mod ("
                                    "  .\\I_c2 (net_0 ),"
                                    "  .\\O_c2 (net_1 )"
                                    " ) ;"
                                    "BUF gate_top ("
                                    "  .I (net_1 ),"
                                    "  .O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gates().size(), 5);      // 3 * gate_child_two + gate_child_one + gate_top
            EXPECT_EQ(nl->get_modules().size(), 5);    // 3 * ENT_CHILD_TWO + ENT_CHILD_ONE + ENT_TOP
            Module* top_module = nl->get_top_module();

            ASSERT_EQ(top_module->get_submodules().size(), 2);
            Module* top_child_one = *top_module->get_submodules().begin();
            Module* top_child_two = *(++top_module->get_submodules().begin());
            if (top_child_one->get_submodules().empty()) {
                std::swap(top_child_one, top_child_two);
            }

            ASSERT_EQ(top_child_one->get_submodules().size(), 2);
            Module* one_child_0 = *(top_child_one->get_submodules().begin());
            Module* one_child_1 = *(++top_child_one->get_submodules().begin());

            // Test if all names that are used multiple times are substituted correctly
            std::string module_suffix = "";

            EXPECT_EQ(top_child_one->get_name(), "child_one_mod");

            EXPECT_TRUE(utils::starts_with(top_child_two->get_name(), "child_two_mod" + module_suffix));
            EXPECT_TRUE(utils::starts_with(one_child_0->get_name(), "gate_0_ent_two" + module_suffix));
            EXPECT_TRUE(utils::starts_with(one_child_1->get_name(), "gate_1_ent_two" + module_suffix));
            // All 3 names should be unique
            EXPECT_EQ(std::set<std::string>({top_child_two->get_name(), one_child_0->get_name(),
                                                one_child_1->get_name()}).size(), 3);

            // Test if the Gate names are substituted correctly as well (gate_child_two is used multiple times)
            std::string gate_suffix = "";

            ASSERT_EQ(top_module->get_gates().size(), 1);
            EXPECT_EQ((*top_module->get_gates().begin())->get_name(), "gate_top");

            ASSERT_EQ(top_child_one->get_gates().size(), 1);
            EXPECT_EQ((*top_child_one->get_gates().begin())->get_name(), "gate_child_one");

            ASSERT_EQ(top_child_two->get_gates().size(), 1);
            ASSERT_EQ(one_child_0->get_gates().size(), 1);
            ASSERT_EQ(one_child_1->get_gates().size(), 1);
            Gate* gate_child_two_0 = *top_child_two->get_gates().begin();
            Gate* gate_child_two_1 = *one_child_0->get_gates().begin();
            Gate* gate_child_two_2 = *one_child_1->get_gates().begin();

            EXPECT_TRUE(utils::starts_with(gate_child_two_0->get_name(), "child_two_mod/gate_child_two" + gate_suffix));
            EXPECT_TRUE(utils::starts_with(gate_child_two_1->get_name(), "gate_0_ent_two/gate_child_two" + gate_suffix));
            EXPECT_TRUE(utils::starts_with(gate_child_two_2->get_name(), "gate_1_ent_two/gate_child_two" + gate_suffix));
            // All 3 names should be unique
            EXPECT_EQ(std::set<std::string>({gate_child_two_0->get_name(), gate_child_two_1->get_name(),
                                                gate_child_two_2->get_name()}).size(), 3);

            // Test the creation on generic data of the Module child_one_mod
            EXPECT_EQ(parameter_of(top_child_one, "child_one_mod_key"),
                        std::make_tuple("integer", "1234"));
        }
        {
            // Create a netlist as follows and test its creation (due to request):
            /*                     - - - - - - - - - - - - - - - - - - - - - - .
                *                    ' mod                                        '
                *                    '                       mod_inner/mod_out    '
                *                    '                     .------------------.   '
                *                    'mod_in               |                  |   'net_0
                *  net_global_in ----=------=| gate_a |=---+---=| gate_b |=   '---=----=| gate_top |=---- net_global_out
                *                    '                                            '
                *                    '                                            '
                *                    '- - - - - - - - - - - - - - - - - - - - - - '
            */

            std::string netlist_input("module ENT_MODULE ( "
                                    "  mod_in, "
                                    "  mod_out "
                                    " ) ; "
                                    "  input mod_in ; "
                                    "  output mod_out ; "
                                    "  wire mod_inner ; "
                                    "  assign mod_out = mod_inner ; "
                                    "BUF gate_a ( "
                                    "  .I (mod_in ), "
                                    "  .O (mod_inner ) "
                                    " ) ; "
                                    "BUF gate_b ( "
                                    "  .I (mod_inner ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    " "
                                    "module ENT_TOP ( "
                                    "  net_global_in, "
                                    "  net_global_out "
                                    " ) ; "
                                    "  input net_global_in ; "
                                    "  output net_global_out ; "
                                    "  wire net_0 ; "
                                    "ENT_MODULE mod ( "
                                    "  .\\mod_in (net_global_in ), "
                                    "  .\\mod_out (net_0 ) "
                                    " ) ; "
                                    "BUF gate_top ( "
                                    "  .I (net_0 ), "
                                    "  .O (net_global_out ) "
                                    " ) ; "
                                    "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gates().size(), 3);      // 1 in top + 2 in mod
            EXPECT_EQ(nl->get_modules().size(), 2);    // top + mod
            Module* top_module = nl->get_top_module();

            ASSERT_EQ(top_module->get_submodules().size(), 1);
            Module* mod = *top_module->get_submodules().begin();

            ASSERT_EQ(mod->get_gates(test_utils::gate_name_filter("gate_a")).size(), 1);
            ASSERT_EQ(mod->get_gates(test_utils::gate_name_filter("gate_b")).size(), 1);
            ASSERT_EQ(nl->get_gates(test_utils::gate_name_filter("gate_top")).size(), 1);
            Gate* gate_a = *mod->get_gates(test_utils::gate_name_filter("gate_a")).begin();
            Gate* gate_b = *mod->get_gates(test_utils::gate_name_filter("gate_b")).begin();
            Gate* gate_top = *nl->get_gates(test_utils::gate_name_filter("gate_top")).begin();

            Net* mod_out = gate_a->get_fan_out_net("O");
            ASSERT_NE(mod_out, nullptr);
            ASSERT_EQ(mod->get_output_nets().size(), 1);
            EXPECT_EQ(*mod->get_output_nets().begin(), mod_out);

            EXPECT_TRUE(test_utils::vectors_have_same_content(mod_out->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_b,
                                                                                                                "I"),
                                                                                        test_utils::get_endpoint(
                                                                                            gate_top,
                                                                                            "I")})));

        }
        {
            // Testing the correct naming of gates and nets that occur in multiple modules by
            // creating the following netlist:

            /*                        MODULE_B
                *                       . -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  .
                *                       '     .-----------------.  shared_net_name  .--------------.     '
                *                       '    |                  |=----------------=|               |     ' net_0
                *      net_global_in ---=---=| shared_gate_name |=----------------=|    gate_b     |=----=-- ...
                *                       '    '------------------'       net_b      '---------------'     '
                *                       ' -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  '
                *
                *                       MODULE_A
                *                       . -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  .
                *                       '     .-----------------.  shared_net_name  .--------------.     '
                *                net_0  '    |                  |=----------------=|               |     ' net_1
                *               ...  ---=---=| shared_gate_name |=----------------=|    gate_a     |=----=-- ...
                *                       '    '------------------'       net_a      '---------------'     '
                *                       ' -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  '
                *
                *                        MODULE_B
                *                net_1  . -  -  . net_2
                *               ... --- =  ...  =-------=| gate_top |=--- net_global_out
                *                       ' -  -  '
                */
            std::string netlist_input("module MODULE_A ( "
                                    "  I_A, "
                                    "  O_A "
                                    " ) ; "
                                    "  input I_A ; "
                                    "  output O_A ; "
                                    "  wire shared_net_name ; "
                                    "  wire net_a ;  "
                                    "COMB12 shared_gate_name ( "
                                    "  .\\I (I_A ), "
                                    "  .\\O0 (shared_net_name ), "
                                    "  .\\O1 (net_a ) "
                                    " ) ; "
                                    "COMB21 gate_a ( "
                                    "  .\\I0 (shared_net_name ), "
                                    "  .\\I1 (net_a ), "
                                    "  .\\O (O_A ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module MODULE_B ( "
                                    "  I_B, "
                                    "  O_B "
                                    " ) ; "
                                    "  input I_B ; "
                                    "  output O_B ; "
                                    "  wire shared_net_name ; "
                                    "  wire net_b ; "
                                    "COMB12 shared_gate_name ( "
                                    "  .\\I (I_B ), "
                                    "  .\\O0 (shared_net_name ), "
                                    "  .\\O1 (net_b ) "
                                    " ) ; "
                                    "COMB21 gate_b ( "
                                    "  .\\I0 (shared_net_name ), "
                                    "  .\\I1 (net_b ), "
                                    "  .\\O (O_B ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module ENT_TOP ( "
                                    "  net_global_in, "
                                    "  net_global_out "
                                    " ) ; "
                                    "  input net_global_in ; "
                                    "  output net_global_out ; "
                                    "  wire net_0 ; "
                                    "  wire net_1; "
                                    "  wire net_2; "
                                    "MODULE_B mod_b_0 ( "
                                    "  .\\I_B (net_global_in ), "
                                    "  .\\O_B (net_0 ) "
                                    " ) ; "
                                    "MODULE_A mod_a_0 ( "
                                    "  .\\I_A (net_0 ), "
                                    "  .\\O_A (net_1 ) "
                                    " ) ; "
                                    "MODULE_B mod_b_1 ( "
                                    "  .\\I_B (net_1 ), "
                                    "  .\\O_B (net_2 ) "
                                    " ) ; "
                                    "BUF gate_top ( "
                                    "  .\\I (net_2 ), "
                                    "  .\\O (net_global_out ) "
                                    " ) ; "
                                    "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);

            // ISSUE: Seems not to be correct. For example net_b occurs two times, but is named net_b__[3]__ and net_b__[4]__
            //  or shared_net_name occurs 3 times and is labeled with 4,5,6

            Net* glob_in = *nl->get_global_input_nets().begin();
            ASSERT_NE(glob_in, nullptr);
            ASSERT_EQ(glob_in->get_destinations().size(), 1);

            Gate* shared_gate_0 = (*glob_in->get_destinations().begin())->get_gate();
            ASSERT_NE(shared_gate_0, nullptr);

            // Get all gates from left to right
            std::vector<Gate*> nl_gates = {shared_gate_0};
            std::vector<std::string> suc_pin = {"O0","O","O0","O","O0","O"};
            for(size_t idx = 0; idx < suc_pin.size(); idx++){
                ASSERT_NE(nl_gates[idx]->get_successor(suc_pin[idx]), nullptr);
                Gate* next_gate = nl_gates[idx]->get_successor(suc_pin[idx])->get_gate();
                ASSERT_NE(next_gate, nullptr);
                nl_gates.push_back(next_gate);
            }

            // Get all nets from left to right (and from top to bottom)
            std::vector<Net*> nl_nets = {glob_in};
            std::vector<std::pair<Gate*, std::string>> net_out_gate_and_pin = {{nl_gates[0], "O0"}, {nl_gates[0], "O1"}, {nl_gates[1], "O"}, {nl_gates[2], "O0"},
                    {nl_gates[2], "O1"}, {nl_gates[3], "O"}, {nl_gates[4], "O0"}, {nl_gates[4], "O1"}, {nl_gates[5], "O"}, {nl_gates[6], "O"}};
            for(size_t idx = 0; idx < net_out_gate_and_pin.size(); idx++){
                Net* next_net = (net_out_gate_and_pin[idx].first)->get_fan_out_net(net_out_gate_and_pin[idx].second);
                ASSERT_NE(next_net, nullptr);
                nl_nets.push_back(next_net);
            }

            // Check that the gate names are correct
            std::vector<std::string> nl_gate_names;
            for(Gate* g : nl_gates) nl_gate_names.push_back(g->get_name());
            std::vector<std::string> expected_gate_names = {"mod_b_0/shared_gate_name", "mod_b_0/gate_b", "mod_a_0/shared_gate_name", "gate_a", "mod_b_1/shared_gate_name", "mod_b_1/gate_b", "gate_top"};
            EXPECT_EQ(nl_gate_names, expected_gate_names);

            // Check that the net names are correct
            std::vector<std::string> nl_net_names;
            for(Net* n : nl_nets) nl_net_names.push_back(n->get_name());
            std::vector<std::string> expected_net_names = {"net_global_in", "mod_b_0/shared_net_name", "mod_b_0/net_b", "net_0", "mod_a_0/shared_net_name", "net_a", "net_1", "mod_b_1/shared_net_name", "mod_b_1/net_b", "net_2", "net_global_out"};
            EXPECT_EQ(nl_net_names, expected_net_names);

        }
        TEST_END
    }

    /**
     * Testing the correct handling of direct assignment (e.g. 'assign net_slave = net_master;'), where two
     * (wire-)identifiers address the same Net. The Net (wire) at the left side of the expression is mapped to the Net
     * at the right side, so only the Net with the right identifier will be created.
     *
     * In Verilog, assignemts of vectors (for example: assign sig_vec_1[0:3] = sig_vec_2[0:3]) and assignments of lists
     * of vectors and/or single nets (for example: "assign {sig_vec_1, signal_1} = {sig_vec_big[0:4]}", if |sig_vec_1|=4)
     * are also supported.
     *
     * However, logic expressions like 'assign A =  B ^ C' are NOT supported.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_direct_assignment) {

        TEST_START
            {
                // Build up a master-slave hierarchy as follows: (NOTE: Whats up with global inputs?)
                /*                                  .--- net_slave_1 (is global input)
                 *   net_master <--- net_slave_0 <--+
                 *                                  '--- net_slave_2
                 */
                std::string netlist_input(
                                        "module top (\n"
                                        "   net_global_in,\n"
                                        "   net_global_out,\n"
                                        "   net_master\n"
                                        ");\n"
                                        "   input net_global_in ;\n"
                                        "   output net_global_out ;\n"
                                        "   wire net_slave_1 ;\n"
                                        "   wire net_slave_0 ;\n"
                                        "   input net_master ;\n"
                                        "   wire net_slave_2 ;\n"
                                        "   assign net_slave_1 = net_slave_0;\n"
                                        "   assign net_slave_2 = net_slave_0;\n"
                                        "   assign net_master = net_slave_0;\n"
                                        ""
                                        "   BUF gate_0 (\n"
                                        "       .I (net_global_in ),\n"
                                        "       .O (net_slave_0 )\n"
                                        "   );\n"
                                        "   AND3 gate_1 (\n"
                                        "       .I0 (net_master ),\n"
                                        "       .I1 (net_slave_1 ),\n"
                                        "       .I2 (net_slave_2 ),\n"
                                        "       .O (net_global_out )\n"
                                        "   );\n"
                                        "endmodule");

                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                EXPECT_EQ(nl->get_nets().size(), 3);    // global_in + global_out + net_master
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_master")).size(), 1);
                Net* net_master = nl->get_nets(test_utils::net_name_filter("net_master")).front();

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).size(), 1);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("AND3", "gate_1")).size(), 1);

                Gate* g_0 = *nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).begin();
                Gate* g_1 = *nl->get_gates(test_utils::gate_filter("AND3", "gate_1")).begin();

                // Check the connections
                EXPECT_EQ(g_0->get_fan_out_net("O"), net_master);
                EXPECT_EQ(g_1->get_fan_in_net("I0"), net_master);
                EXPECT_EQ(g_1->get_fan_in_net("I1"), net_master);
                EXPECT_EQ(g_1->get_fan_in_net("I2"), net_master);

                // Check that net_master becomes also a global input
                EXPECT_TRUE(net_master->is_global_input_net());

            }
            {
                // Create a cyclic master-slave Net hierarchy (first master net should survive)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out "
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "  wire net_0;"
                                        "  wire net_1;"
                                        "  assign net_0 = net_1;"
                                        "  assign net_1 = net_0;"
                                        "BUF gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (net_0 )"
                                        " ) ;"
                                        "BUF gate_1 ("
                                        "  .I (net_1 ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                EXPECT_EQ(nl->get_nets().size(), 3);    // global_in + global_out + net_0
                EXPECT_EQ(nl->get_nets(test_utils::net_name_filter("net_0")).size(), 1);
            }
            {
                std::string netlist_input("module passthrough_test (\n"
                                        "   input net_in,\n"
                                        "   output net_out\n"
                                        ");\n"
                                        "   wire net_a;\n"
                                        "   assign net_out = net_a;\n"
                                        "   BUF gate_1 (\n"
                                        "       .I (net_in ),\n"
                                        "       .O (net_a )\n"
                                        "   );\n"
                                        "endmodule\n"
                                        "\n"
                                        "module top (net_global_in, net_global_out) ;\n"
                                        "   input net_global_in ;\n"
                                        "   output net_global_out ;\n"
                                        "   passthrough_test pt (\n"
                                        "       .net_in (net_global_in ),\n"
                                        "       .net_out ()\n"
                                        "   );\n"
                                        "endmodule");

                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                EXPECT_EQ(nl->get_nets().size(), 3);    // the unused top port net_global_out is kept
                EXPECT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_in")).size(), 1);
                EXPECT_EQ(nl->get_nets(test_utils::net_name_filter("net_out")).size(), 1);
            }
            {
                std::string netlist_input("module passthrough_test (\n"
                                        "   input net_in,\n"
                                        "   output net_out\n"
                                        ");\n"
                                        "   wire net_a;\n"
                                        "   assign net_out = net_a;\n"
                                        "   BUF gate_1 (\n"
                                        "       .I (net_in ),\n"
                                        "       .O (net_a )\n"
                                        "   );\n"
                                        "endmodule\n"
                                        "\n"
                                        "module top (net_global_in, net_global_out) ;\n"
                                        "   input net_global_in ;\n"
                                        "   output net_global_out ;\n"
                                        "   passthrough_test pt (\n"
                                        "       .net_in (),\n"
                                        "       .net_out (net_global_out)\n"
                                        "   );\n"
                                        "endmodule");

                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                
                ASSERT_NE(nl, nullptr);
                EXPECT_EQ(nl->get_nets().size(), 3);    // the unused top port net_global_in is kept
                EXPECT_EQ(nl->get_nets(test_utils::net_name_filter("net_in")).size(), 1);
                EXPECT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_out")).size(), 1);
            }
            // -- Verilog Specific Tests
            {
                // Verilog specific: Testing assignments with logic vectors (assign wires 0 and 1 of each dimension)
                // for example (for dim 2): wire [0:1][0:1] slave_vector; wire [0:3] master_vector; assign slave_vector = master_vector;

                // Will be tested for dimensions up to MAX_DIMENSION (runtime growth exponentially)
                const u8 MAX_DIMENSION = 3;    // Can be turned up, if you are bored ;)

                for (u8 dim = 0; dim <= MAX_DIMENSION; dim++) {
                    std::stringstream global_out_list_module;
                    std::stringstream global_out_list;
                    std::stringstream gate_list;
                    std::string dim_decl = "";

                    dim_decl += "";
                    for (u8 d = 0; d < dim; d++) {
                        dim_decl += "[0:1]";
                    }

                    // 2^(dim) gates (with one pin) must be created to connect all assigned wires
                    for (u64 i = 0; i < (1 << dim); i++) {
                        global_out_list_module << "  global_out_" << i << ",";
                        global_out_list << "  output global_out_" << i << ";";

                        std::bitset<64> i_bs(i);
                        std::stringstream brackets("");
                        for (int j = dim - 1; j >= 0; j--) {
                            brackets << "[" << (i_bs[j] ? "1" : "0") << "]";
                        }

                        gate_list << "BUF in_gate_" << i
                                  << " ("
                                     "  .I (global_in ),"
                                     "  .O ( net_slave_vector"
                                  << brackets.str()
                                  << ")"
                                     " ) ;";
                        gate_list << "BUF out_gate_" << i
                                  << " ("
                                     "  .I (net_slave_vector"
                                  << brackets.str()
                                  << "),"
                                     "  .O ( global_out_"
                                  << i
                                  << ")"
                                     " ) ;";
                    }

                    std::stringstream netlist_input;
                    netlist_input << "module top ("
                             "  global_in,"
                          << global_out_list_module.str()
                          << "  );"
                             "  input global_in ;"
                          << global_out_list.str() << "  wire " << dim_decl << " net_slave_vector;"
                          << "  wire [0:" << ((1 << dim) - 1) << "] net_master_vector;"
                          << "  assign net_master_vector = net_slave_vector;"    // <- !!!
                          << gate_list.str() << "endmodule";

                    const GateLibrary* gate_lib = test_utils::get_gate_library();
                    auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input.str());
                    VerilogParser verilog_parser;
                    auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                    ASSERT_TRUE(nl_res.is_ok());
                    std::unique_ptr<Netlist> nl = nl_res.get();

                    for (u64 i = 0; i < (1 << dim); i++) {
                        ASSERT_NE(nl, nullptr);

                        ASSERT_EQ(nl->get_nets(test_utils::net_name_filter(
                            "net_master_vector(" + std::to_string(i) + ")")).size(),
                                  1);
                        Net* net_i =
                            *nl->get_nets(test_utils::net_name_filter("net_master_vector(" + std::to_string(i) + ")"))
                                .begin();

                        ASSERT_EQ(nl->get_gates(test_utils::gate_filter("BUF", "in_gate_" + std::to_string(i)))
                                      .size(), 1);
                        ASSERT_EQ(nl->get_gates(test_utils::gate_filter("BUF", "out_gate_" + std::to_string(i)))
                                      .size(), 1);
                        Gate* in_gate_i =
                            *nl->get_gates(test_utils::gate_filter("BUF", "in_gate_" + std::to_string(i)))
                                .begin();
                        Gate* out_gate_i =
                            *nl->get_gates(test_utils::gate_filter("BUF", "out_gate_" + std::to_string(i)))
                                .begin();

                        EXPECT_EQ(in_gate_i->get_fan_out_net("O"), net_i);
                        EXPECT_EQ(out_gate_i->get_fan_in_net("I"), net_i);
                    }
                }
            }
            {
                // Verilog specific: Assign constants ('b0 and 'b1)
                std::string netlist_input("module top ("
                                        "  global_out"
                                        " ) ;"
                                        "  output global_out ;"
                                        "  wire [0:3] bit_vector ;"
                                        "  assign bit_vector = 4'hA ;"
                                        ""
                                        "  BUF test_gate ("
                                        "  .I (bit_vector[0] ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_name_filter("test_gate")).size(), 1);
                Gate* test_gate = *nl->get_gates(test_utils::gate_name_filter("test_gate")).begin();
            }
            {
                // Verilog specific: Assign a set of wires to a single vector
                std::string netlist_input("module top ("
                                        "  global_out_0,"
                                        "  global_out_1,"
                                        "  global_out_2"
                                        " ) ;"
                                        "  output global_out_0 ;"
                                        "  output global_out_1 ;"
                                        "  output global_out_2 ;"
                                        ""
                                        "  wire single_net ;"
                                        "  wire [0:2][0:2] _2_d_vector_0;"
                                        "  wire [0:2][0:1] _2_d_vector_1;"
                                        "  wire [0:15] big_vector;"
                                        "  wire [0:11] net_vector_master;"
                                        "  assign net_vector_master = {single_net, big_vector[3], big_vector[0:1], _2_d_vector_0[0:1][0:1], _2_d_vector_1[1:0][0:1]};"
                                        ""
                                        "AND4 test_gate_0 ("
                                        "  .I0 (single_net ),"
                                        "  .I1 (big_vector[3] ),"
                                        "  .I2 (big_vector[0] ),"
                                        "  .I3 (big_vector[1] ),"
                                        "  .O (global_out_0 )"
                                        " ) ;"
                                        ""
                                        "AND4 test_gate_1 ("
                                        "  .I0 (_2_d_vector_0[0][0] ),"
                                        "  .I1 (_2_d_vector_0[0][1] ),"
                                        "  .I2 (_2_d_vector_0[1][0] ),"
                                        "  .I3 (_2_d_vector_0[1][1] ),"
                                        "  .O (global_out_1 )"
                                        " ) ;"
                                        "AND4 test_gate_2 ("
                                        "  .I0 (_2_d_vector_1[1][0] ),"
                                        "  .I1 (_2_d_vector_1[1][1] ),"
                                        "  .I2 (_2_d_vector_1[0][0] ),"
                                        "  .I3 (_2_d_vector_1[0][1] ),"
                                        "  .O (global_out_2 )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                std::vector<Net *> net_master_vector(12);
                for (int i = 0; i < 12; i++) {
                    ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vector_master(" + std::to_string(i) + ")"))
                                  .size(), 1);
                    net_master_vector[i] =
                        *nl->get_nets(test_utils::net_name_filter("net_vector_master(" + std::to_string(i) + ")"))
                            .begin();
                }
                for (int i = 0; i < 12; i++) {
                    ASSERT_EQ(net_master_vector[i]->get_destinations().size(), 1);
                    Endpoint *ep = *net_master_vector[i]->get_destinations().begin();
                    EXPECT_EQ(ep->get_gate()->get_name(), "test_gate_" + std::to_string(i / 4));
                    EXPECT_EQ(ep->get_pin()->get_name(), "I" + std::to_string(i % 4));
                }
            }
            {
                // Verilog specific: Assign a 2 bit vector to a set of 1 bit vectors

                std::string netlist_input("module top ("
                                        "    global_out"
                                        ") ;"
                                        "    output global_out ;"
                                        "    wire net_master_0 ;"
                                        "    wire net_master_1 ;"
                                        "    wire [0:1] net_vector_slave;"
                                        "    assign { net_master_0, net_master_1 } = net_vector_slave;"
                                        "    AND2 test_gate ("
                                        "        .I0 ( net_vector_slave[0] ),"
                                        "        .I1 ( net_vector_slave[1] ),"
                                        "        .O (global_out )"
                                        "    ) ;"
                                        "endmodule");

                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_master_0")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_master_1")).size(), 1);
                Net*
                    net_vector_master_0 = *nl->get_nets(test_utils::net_name_filter("net_master_0")).begin();
                Net*
                    net_vector_master_1 = *nl->get_nets(test_utils::net_name_filter("net_master_1")).begin();
                ASSERT_EQ(net_vector_master_0->get_destinations().size(), 1);
                ASSERT_EQ(net_vector_master_1->get_destinations().size(), 1);
                EXPECT_EQ((*net_vector_master_0->get_destinations().begin())->get_pin()->get_name(), "I0");
                EXPECT_EQ((*net_vector_master_1->get_destinations().begin())->get_pin()->get_name(), "I1");
            }
            {
                // Verilog specific: Testing assignments, where escaped identifiers are used (e.g.\Net[1:3][2:3] stands for a Net, literally named "Net[1:3][2:3]")

                std::string netlist_input("module top ("
                                        "    global_out"
                                        ") ;"
                                        "    output global_out ;"
                                        "    wire \\escaped_net_range[0:3] ;"
                                        "    wire [0:3] escaped_net_range ;"
                                        "    wire \\escaped_net[0] ;"
                                        "    wire [0:1] net_vector_master ;"
                                        "    assign net_vector_master = { \\escaped_net_range[0:3] , \\escaped_net[0] };"
                                        "    AND2 test_gate ("
                                        "        .I0 ( \\escaped_net_range[0:3] ),"
                                        "        .I1 ( \\escaped_net[0] ),"
                                        "        .O (global_out )"
                                        "    ) ;"
                                        "endmodule");

                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
            }
        TEST_END
    }

    /**
     * Testing the port assignment of multiple pins and nets using pin groups and signal vectors
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_pin_group_port_assignment) {

        TEST_START
            {
                // Connect an entire output pin group with global input nets by using a binary string ('b0101)
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire [0:3] l_vec;\n"
                                        "  wire net_1 ;\n"
                                        "RAM gate_0 (\n"
                                        "  .ADDR ('b0101)\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).begin());
                Net* net_0 = gate_0->get_fan_in_net("ADDR(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "'1'");

                Net* net_1 = gate_0->get_fan_in_net("ADDR(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "'0'");

                Net* net_2 = gate_0->get_fan_in_net("ADDR(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "'1'");

                Net* net_3 = gate_0->get_fan_in_net("ADDR(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "'0'");

            }
            {
                // Connect a vector of output pins with a list of nets using '{ net_0, net_1, ... }'
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire net_0;\n"
                                        "  wire net_1;\n"
                                        "  wire[0:1] l_vec;\n"
                                        "RAM gate_0 (\n"
                                        "  .DATA_OUT ({net_0, net_1, l_vec[0], l_vec[1]})\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 4);

                Net* net_0 = gate_0->get_fan_out_net("DATA_OUT(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(1)");

                Net* net_1 = gate_0->get_fan_out_net("DATA_OUT(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(0)");

                Net* net_2 = gate_0->get_fan_out_net("DATA_OUT(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "net_1");

                Net* net_3 = gate_0->get_fan_out_net("DATA_OUT(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "net_0");
            }
            {
                // Connect a vector of output pins with a vector of nets (O(0) with l_vec(0),...,O(4) with l_vec(4))
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire [3:0] l_vec;\n"
                                        "  wire net_1 ;\n"
                                        "RAM gate_0 (\n"
                                        "  .DATA_OUT (l_vec)\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 4);
                Net* net_0 = gate_0->get_fan_out_net("DATA_OUT(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(0)");

                Net* net_1 = gate_0->get_fan_out_net("DATA_OUT(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(1)");

                Net* net_2 = gate_0->get_fan_out_net("DATA_OUT(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(2)");

                Net* net_3 = gate_0->get_fan_out_net("DATA_OUT(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "l_vec(3)");

            }
            {
                // Connect a vector of output pins with a vector of nets (O(0) with l_vec(0),...,O(3) with l_vec(3))
                // but the vector has a smaller size
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire [2:0] l_vec;\n"
                                        "RAM gate_0 (\n"
                                        "  .DATA_OUT (l_vec)\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 3);
                Net* net_0 = gate_0->get_fan_out_net("DATA_OUT(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(0)");

                Net* net_1 = gate_0->get_fan_out_net("DATA_OUT(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(1)");

                Net* net_2 = gate_0->get_fan_out_net("DATA_OUT(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(2)");

                EXPECT_EQ(gate_0->get_fan_out_net("DATA_OUT(3)"), nullptr);

            }
            {
                // Test assigning 2-bit input and 3-bit output wires to 4-bit ports
                std::string netlist_input("module top (in, out) ;\n"
                                        "  input [1:0] in;\n"
                                        "  output [2:0] out;\n"
                                        "RAM gate_0 (\n"
                                        "  .DATA_IN (in),\n"
                                        "  .DATA_OUT (out)\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                Net* net;
                Gate* gate;

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).empty());
                gate = *(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).begin());

                EXPECT_EQ(gate->get_fan_in_nets().size(), 2);
                net = gate->get_fan_in_net("DATA_IN(0)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "in(0)");
                net = gate->get_fan_in_net("DATA_IN(1)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "in(1)");

                EXPECT_EQ(gate->get_fan_out_nets().size(), 3);
                net = gate->get_fan_out_net("DATA_OUT(0)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "out(0)");
                net = gate->get_fan_out_net("DATA_OUT(1)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "out(1)");
                net = gate->get_fan_out_net("DATA_OUT(2)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "out(2)");
            }
            {
                // test using only parts of global in- and outputs (unused nets should be removed)
                std::string netlist_input("module top (in, out) ;\n"
                                        "  input [1:0] in;\n"
                                        "  output [1:0] out;\n"
                                        "BUF gate_0 (\n"
                                        "  .I (in[1]),\n"
                                        "  .O (out[0])\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                Net* net;
                Gate* gate;

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).empty());
                gate = *(nl->get_gates(test_utils::gate_filter("BUF", "gate_0")).begin());

                EXPECT_EQ(nl->get_nets().size(), 4);    // the unused bits of the top ports are kept
                EXPECT_EQ(nl->get_top_module()->get_pins().size(), 2);    // only the used bits reach a gate, so only they get pins

                EXPECT_EQ(gate->get_fan_in_nets().size(), 1);
                net = gate->get_fan_in_net("I");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "in(1)");

                EXPECT_EQ(gate->get_fan_out_nets().size(), 1);
                net = gate->get_fan_out_net("O");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "out(0)");
            }
            {
                // empty pin assignment
                std::string netlist_input("module top (in) ;\n"
                                        "  input [3:0] in;\n"
                                        "RAM gate_0 (\n"
                                        "  .DATA_IN (in),\n"
                                        "  .DATA_OUT ()\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                Net* net;
                Gate* gate;

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).empty());
                gate = *(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).begin());

                EXPECT_EQ(nl->get_nets().size(), 4);
                EXPECT_EQ(nl->get_top_module()->get_pins([](const ModulePin* p) {return p->get_direction() == PinDirection::input; }).size(), 4);
                EXPECT_EQ(nl->get_top_module()->get_pins([](const ModulePin* p) {return p->get_direction() == PinDirection::output; }).size(), 0);

                EXPECT_EQ(gate->get_fan_in_nets().size(), 4);
                EXPECT_EQ(gate->get_fan_out_nets().size(), 0);
            }
            {
                // Connect a vector of output pins with a list of nets using '{ net_0, net_1, ... }' that is wider than
                // the input port size (only the last elements of the list should be assigned)
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire net_0;\n"
                                        "  wire net_1;\n"
                                        "  wire[5:0] l_vec;\n"
                                        "RAM gate_0 (\n"
                                        "  .DATA_OUT (l_vec)\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("RAM", "gate_0")).begin());
                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 4);
                Net* net_0 = gate_0->get_fan_out_net("DATA_OUT(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(0)");
                Net* net_1 = gate_0->get_fan_out_net("DATA_OUT(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(1)");
                Net* net_2 = gate_0->get_fan_out_net("DATA_OUT(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(2)");
                Net* net_3 = gate_0->get_fan_out_net("DATA_OUT(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "l_vec(3)");
            }
        TEST_END
    }

    /*#########################################################################
       Verilog Specific Tests (Tests that can not be directly applied to VHDL)
      #########################################################################*/

    /**
     * Testing the correct detection of single line comments (with '//') and comment blocks(with '/ *' and '* /').
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_comments) {

        TEST_START
            {
                // Testing all comment types with attributes
                std::string netlist_input("/*here comes a module*/ module top (\n"
                                        "  global_in,\n"
                                        "  \\global_out_with//comment \n"
                                        " ) ;\n"
                                        "  input global_in;\n"
                                        "  output \\global_out_with//comment ;\n"
                                        "\n"
                                        "BUF #(\n"
                                        "  .no_comment_0(123), //.comment_0(123),\n"
                                        "  //.comment_1(123),\n"
                                        "  .no_comment_1(123), /*.comment_2(123),*/ .no_comment_2(123),\n"
                                        "  /*.comment_3(123),\n"
                                        "  .comment_4(123),\n"
                                        "  .comment_5(123),*/\n"
                                        "  .no_comment_3(123),\n"
                                        "  .no_comment_4(123), /*.comment_6(123),*/ .no_comment_5(123),\n"
                                        "  /*.comment_7(123),\n"
                                        "  .comment_8(123),\n"
                                        "  .comment_9(123),*/\n"
                                        "  .no_comment_6(123)\n"
                                        ") \n"
                                        "test_gate (\n"
                                        "  .I (global_in ),\n"
                                        "  .O (\\global_out_with//comment )\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("BUF", "test_gate")).size(), 1);
                Gate*
                    test_gate = *nl->get_gates(test_utils::gate_filter("BUF", "test_gate")).begin();

                // Test that the comments did not removed other parts (all no_comment_n generics should be created)
                for (std::string key : std::set<std::string>({"no_comment_0", "no_comment_1", "no_comment_2",
                                                              "no_comment_3", "no_comment_4", "no_comment_5",
                                                              "no_comment_6"})) {
                    EXPECT_NE(parameter_of(test_gate, key), std::make_tuple("", ""));
                    if (parameter_of(test_gate, key) == std::make_tuple("", "")) {
                        std::cout << "comment test failed for: " << key << std::endl;
                    }
                }

                // Test that the comments are not interpreted (all comment_n generics shouldn't be created)
                for (std::string key : std::set<std::string>({"comment_0", "comment_1", "comment_2", "comment_3",
                                                              "comment_4", "comment_5", "comment_6", "comment_7",
                                                              "comment_8", "comment_9"})) {
                    EXPECT_EQ(parameter_of(test_gate, key), std::make_tuple("", ""));
                    if (parameter_of(test_gate, key) != std::make_tuple("", "")) {
                        std::cout << "comment failed for: " << key << std::endl;
                    }
                }

                // comment within escaped identifier
                EXPECT_EQ(test_gate->get_fan_out_net("O")->get_name(), "global_out_with//comment");
            }
        TEST_END
    }

    /**
     * Testing the usage of attributes for gates nets and modules
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_attributes) {
        TEST_START
            {
                // Add attributes for a  module, a gate and a net.
                std::string netlist_input("(* ATTRIBUTE_MODULE=attri_module, FLAG_MODULE *)\n"
                                        "module top (\n"
                                        "  net_global_in,\n"
                                        "  net_global_out\n"
                                        " ) ;\n"
                                        "(* ATTRIBUTE_NET=attri_net, FLAG_NET *)\n"
                                        "  input net_global_in ;\n"
                                        "  output net_global_out ;\n"
                                        "(* ATTRIBUTE_GATE=attri_gate, FLAG_GATE *)\n"
                                        "BUF gate_0 (\n"
                                        "  .\\I (net_global_in ),\n"
                                        "  .\\O (net_global_out )\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);

                // Access the Elements with attributes
                Module* attri_module = nl->get_top_module();

                ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("BUF")).size(), 1);
                Gate* attri_gate = *nl->get_gates(test_utils::gate_type_filter("BUF")).begin();

                ASSERT_EQ(nl->get_global_input_nets().size(), 1);
                Net* attri_net = *nl->get_global_input_nets().begin();

                // Check their attributes
                // -- Module
                EXPECT_EQ(attribute_of(attri_module, "ATTRIBUTE_MODULE"),
                          std::make_tuple("string", "attri_module"));
                EXPECT_EQ(attribute_of(attri_module, "FLAG_MODULE"), std::make_tuple("boolean", "true"));
                // -- Gate
                EXPECT_EQ(attribute_of(attri_gate, "ATTRIBUTE_GATE"),
                          std::make_tuple("string", "attri_gate"));
                EXPECT_EQ(attribute_of(attri_gate, "FLAG_GATE"), std::make_tuple("boolean", "true"));
                // -- Net
                EXPECT_EQ(attribute_of(attri_net, "ATTRIBUTE_NET"),
                          std::make_tuple("string", "attri_net"));
                EXPECT_EQ(attribute_of(attri_net, "FLAG_NET"), std::make_tuple("boolean", "true"));
            }
            {
                // Use atrribute strings with special characters (',','.')
                std::string netlist_input("module top (\n"
                                        "  net_global_in,\n"
                                        "  net_global_out\n"
                                        " ) ;\n"
                                        "  input net_global_in ;\n"
                                        "  output net_global_out ;\n"
                                        "  (* ATTRI_COMMA_STRING=\"test, 1, 2, 3\", ATTRI_FLOAT_STRING=\"1.234\" *)\n"
                                        "BUF gate_0 (\n"
                                        "  .\\I (net_global_in ),\n"
                                        "  .\\O (net_global_out )\n"
                                        " ) ;\n"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("BUF")).size(), 1);
                Gate* attri_gate = *nl->get_gates(test_utils::gate_type_filter("BUF")).begin();
                EXPECT_EQ(attribute_of(attri_gate, "ATTRI_COMMA_STRING"),
                          std::make_tuple("string", "test, 1, 2, 3"));
                EXPECT_EQ(attribute_of(attri_gate, "ATTRI_FLOAT_STRING"),
                          std::make_tuple("string", "1.234"));
            }
        TEST_END
    }

    /**
     * Testing the declaration of multiple wire vectors within one single line.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_one_line_multiple_nets) {

        TEST_START
            {
                // Declare multiple wire vectors in one line
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "  wire [0:1] net_vec_0, net_vec_1 ;"    // <- !!!
                                        "COMB41 gate_0 ("
                                        "  .I0 (net_vec_0[0] ),"
                                        "  .I1 (net_vec_0[1] ),"
                                        "  .I2 (net_vec_1[0] ),"
                                        "  .I3 (net_vec_1[1] ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "BUF gate_1 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_0[0] )"
                                        ") ;"
                                        "BUF gate_2 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_0[1] )"
                                        ") ;"
                                        "BUF gate_3 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_1[0] )"
                                        ") ;"
                                        "BUF gate_4 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_1[1] )"
                                        ") ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                if(nl_res.is_error()) {
                    std::cout << nl_res.get_error().get() << std::endl;
                }
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);
                EXPECT_EQ(nl->get_nets().size(), 6);    // 3 of the net_vector + global_in + global_out
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(0)")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(1)")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(0)")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(1)")).size(), 1);
            }
        TEST_END
    }

    /**
     * Testing the port declaration within the list of ports.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_port_dekl_in_list_of_ports) {
        TEST_START
            {
                // Testing the declaration of single ports within the list of ports.
                std::string netlist_input("module top ( \n"
                                        "  input global_in_0, global_in_1, global_in_2,\n"
                                        "  output global_out_0, global_out_1, global_out_2,\n"
                                        "  input global_in_3,"
                                        "  output global_out_3"
                                        " ) ; \n"
                                        "COMB44 gate_0 ( \n"
                                        "  .I0 (global_in_0 ), \n"
                                        "  .I1 (global_in_1 ), \n"
                                        "  .I2 (global_in_2 ), \n"
                                        "  .I3 (global_in_3 ), \n"
                                        "  .O0 (global_out_0 ), \n"
                                        "  .O1 (global_out_1 ), \n"
                                        "  .O2 (global_out_2 ), \n"
                                        "  .O3 (global_out_3 ) \n"
                                        " ) ; \n"
                                        "endmodule \n");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);

                // Check all input nets
                std::set<std::string> input_net_names;
                for(Net* n : nl->get_global_input_nets())
                    input_net_names.insert(n->get_name());
                EXPECT_EQ(input_net_names, std::set<std::string>({"global_in_0", "global_in_1","global_in_2","global_in_3"}));

                // Check all output nets
                std::set<std::string> output_net_names;
                for(Net* n : nl->get_global_output_nets())
                    output_net_names.insert(n->get_name());
                EXPECT_EQ(output_net_names, std::set<std::string>({"global_out_0", "global_out_1","global_out_2","global_out_3"}));
            }
            {
                // Testing the declaration of port vectors within the list of ports.
                std::string netlist_input("module top ( \n"
                                        "  input [0:1] global_in_asc,\n"
                                        "  output [0:1] global_out_asc,\n"
                                        "  input [3:2] global_in_desc,"
                                        "  output [3:2] global_out_desc"
                                        " ) ; \n"
                                        "COMB44 gate_0 ( \n"
                                        "  .I0 (global_in_asc[0] ), \n"
                                        "  .I1 (global_in_asc[1] ), \n"
                                        "  .I2 (global_in_desc[2] ), \n"
                                        "  .I3 (global_in_desc[3] ), \n"
                                        "  .O0 (global_out_asc[0] ), \n"
                                        "  .O1 (global_out_asc[1] ), \n"
                                        "  .O2 (global_out_desc[2] ), \n"
                                        "  .O3 (global_out_desc[3] ) \n"
                                        " ) ; \n"
                                        "endmodule \n");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);

                // Check all input nets
                std::set<std::string> input_net_names;
                for(Net* n : nl->get_global_input_nets())
                    input_net_names.insert(n->get_name());
                EXPECT_EQ(input_net_names, std::set<std::string>({"global_in_asc(0)", "global_in_asc(1)","global_in_desc(2)","global_in_desc(3)"}));

                // Check all output nets
                std::set<std::string> output_net_names;
                for(Net* n : nl->get_global_output_nets())
                    output_net_names.insert(n->get_name());
                EXPECT_EQ(output_net_names, std::set<std::string>({"global_out_asc(0)", "global_out_asc(1)","global_out_desc(2)","global_out_desc(3)"}));
            }
            {
                // Testing the declaration of 2-dim port vectors within the list of ports.
                std::string netlist_input("module top ( \n"
                                        "  input [0:1][1:0] global_in,\n"
                                        "  output [1:0][0:1] global_out"
                                        " ) ; \n"
                                        "COMB44 gate_0 ( \n"
                                        "  .I0 (global_in[0][0] ), \n"
                                        "  .I1 (global_in[0][1] ), \n"
                                        "  .I2 (global_in[1][0] ), \n"
                                        "  .I3 (global_in[1][1] ), \n"
                                        "  .O0 (global_out[0][0] ), \n"
                                        "  .O1 (global_out[0][1] ), \n"
                                        "  .O2 (global_out[1][0] ), \n"
                                        "  .O3 (global_out[1][1] ) \n"
                                        " ) ; \n"
                                        "endmodule \n");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();

                ASSERT_NE(nl, nullptr);

                // Check all input nets
                std::set<std::string> input_net_names;
                for(Net* n : nl->get_global_input_nets())
                    input_net_names.insert(n->get_name());
                EXPECT_EQ(input_net_names, std::set<std::string>({"global_in(0)(0)", "global_in(0)(1)","global_in(1)(0)","global_in(1)(1)"}));

                // Check all output nets
                std::set<std::string> output_net_names;
                for(Net* n : nl->get_global_output_nets())
                    output_net_names.insert(n->get_name());
                EXPECT_EQ(output_net_names, std::set<std::string>({"global_out(0)(0)", "global_out(0)(1)","global_out(1)(0)","global_out(1)(1)"}));
            }
        TEST_END
    }
    /**
     * Testing the correct handling of invalid input
     *
     * Functions: parse
     */
    /**
     * A module header may give a port as an expression over internal signals, as Vivado does for a port that is partly
     * constant: `.sum({\<const0> ,\^sum [1:0]})`. The body then declares the direction on the signals of the expression.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_port_expressions)
    {
        TEST_START
        {
            std::string netlist_input("module ADDER ("
                                      "  a,"
                                      "  .sum({\\<const0> ,\\^sum [1:0]})"
                                      " ) ;"
                                      "  input [1:0]a ;"
                                      "  output \\<const0> ;"
                                      "  output [1:0]\\^sum ;"
                                      "  wire \\<const0> ;"
                                      "GND gnd_inst ("
                                      "  .O (\\<const0> )"
                                      " ) ;"
                                      "BUF buf_0 ("
                                      "  .I (a[0]),"
                                      "  .O (\\^sum [0])"
                                      " ) ;"
                                      "BUF buf_1 ("
                                      "  .I (a[1]),"
                                      "  .O (\\^sum [1])"
                                      " ) ;"
                                      "endmodule"
                                      "\n"
                                      "module top ("
                                      "  in,"
                                      "  out"
                                      " ) ;"
                                      "  input [1:0]in ;"
                                      "  output [2:0]out ;"
                                      "ADDER adder_inst ("
                                      "  .a (in),"
                                      "  .sum (out)"
                                      " ) ;"
                                      "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file           = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("GND")).size(), 1);
            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("BUF", "buf_0")).size(), 1);
            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("BUF", "buf_1")).size(), 1);
            Gate* gnd   = *nl->get_gates(test_utils::gate_type_filter("GND")).begin();
            Gate* buf_0 = *nl->get_gates(test_utils::gate_filter("BUF", "buf_0")).begin();
            Gate* buf_1 = *nl->get_gates(test_utils::gate_filter("BUF", "buf_1")).begin();

            // the last element of the concatenation is bit 0 of the port
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("out(0)")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("out(1)")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("out(2)")).size(), 1);
            Net* out_0 = *nl->get_nets(test_utils::net_name_filter("out(0)")).begin();
            Net* out_1 = *nl->get_nets(test_utils::net_name_filter("out(1)")).begin();
            Net* out_2 = *nl->get_nets(test_utils::net_name_filter("out(2)")).begin();
            EXPECT_TRUE(out_0->is_global_output_net());
            EXPECT_TRUE(out_1->is_global_output_net());
            EXPECT_TRUE(out_2->is_global_output_net());
            EXPECT_EQ(buf_0->get_fan_out_net("O"), out_0);
            EXPECT_EQ(buf_1->get_fan_out_net("O"), out_1);
            EXPECT_EQ(gnd->get_fan_out_net("O"), out_2);

            // the port of the submodule has one pin per bit of its expression, all outputs
            ASSERT_EQ(nl->get_modules(test_utils::module_name_filter("adder_inst")).size(), 1);
            Module* adder = *nl->get_modules(test_utils::module_name_filter("adder_inst")).begin();
            for (const std::string& pin_name : {"sum(0)", "sum(1)", "sum(2)"})
            {
                ModulePin* pin = adder->get_pin_by_name(pin_name);
                ASSERT_NE(pin, nullptr) << pin_name;
                EXPECT_EQ(pin->get_direction(), PinDirection::output) << pin_name;
            }
            EXPECT_EQ(adder->get_pin_by_name("sum(0)")->get_net(), out_0);
            EXPECT_EQ(adder->get_pin_by_name("sum(2)")->get_net(), out_2);
        }
        TEST_END
    }

    /**
     * A module marked (* top = 1 *), as Yosys does, is the top module even if other modules are not instantiated either;
     * without the mark, several uninstantiated modules are an error that names them.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_top_attribute)
    {
        TEST_START
        {
            std::string netlist_input("module UNUSED ("
                                      "  a,"
                                      "  b"
                                      " ) ;"
                                      "  input a ;"
                                      "  output b ;"
                                      "BUF gate_unused ("
                                      "  .I (a),"
                                      "  .O (b)"
                                      " ) ;"
                                      "endmodule"
                                      "\n"
                                      "(* top =  1  *)"
                                      "module DESIGN ("
                                      "  in,"
                                      "  out"
                                      " ) ;"
                                      "  input in ;"
                                      "  output out ;"
                                      "BUF gate_top ("
                                      "  .I (in),"
                                      "  .O (out)"
                                      " ) ;"
                                      "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file           = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_ok());
            std::unique_ptr<Netlist> nl = nl_res.get();
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_design_name(), "DESIGN");
            EXPECT_EQ(nl->get_gates().size(), 1);
            EXPECT_EQ((*nl->get_gates().begin())->get_name(), "gate_top");
        }
        {
            // the same two modules without the mark: ambiguous, and the error names both candidates
            NO_COUT_TEST_BLOCK;
            std::string netlist_input("module UNUSED ("
                                      "  a,"
                                      "  b"
                                      " ) ;"
                                      "  input a ;"
                                      "  output b ;"
                                      "BUF gate_unused ("
                                      "  .I (a),"
                                      "  .O (b)"
                                      " ) ;"
                                      "endmodule"
                                      "\n"
                                      "module DESIGN ("
                                      "  in,"
                                      "  out"
                                      " ) ;"
                                      "  input in ;"
                                      "  output out ;"
                                      "BUF gate_top ("
                                      "  .I (in),"
                                      "  .O (out)"
                                      " ) ;"
                                      "endmodule");
            const GateLibrary* gate_lib = test_utils::get_gate_library();
            auto verilog_file           = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
            ASSERT_TRUE(nl_res.is_error());
            const std::string message = nl_res.get_error().get();
            EXPECT_NE(message.find("DESIGN"), std::string::npos);
            EXPECT_NE(message.find("UNUSED"), std::string::npos);
        }
        TEST_END
    }

    TEST_F(VerilogParserTest, check_invalid_input) {
        TEST_START
            {
                // Try to connect to a pin, that does not exist
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in "
                                        " ) ;"
                                        "  input global_in ;"

                                        "BUF gate_0 ("
                                        "  .\\NON_EXISTING_PIN (global_in)"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            {
                // The passed Gate library name is unknown
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "UNKNOWN_GATE_TYPE gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            {
                // The input does not contain any Module (is empty)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
        /* non-used entity test commented out (entity erroneously considered as top module)
            {
                // Create a non-used entity (should not create any problems...)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module ignore_me ("
                                        "  min,"
                                        "  mout"
                                        " ) ;"
                                        "  input min ;"
                                        "  output mout ;"
                                        "BUF gate_0 ("
                                        "  .I (min ),"
                                        "  .O (mout )"
                                        " ) ;"
                                        "endmodule"
                                        "\n"
                                        "module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "BUF gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (!nl_res.is_ok())
                    std::cerr << "error <" << nl_res.get_error().get() << ">" << std::endl;
                ASSERT_TRUE(nl_res.is_ok());
                std::unique_ptr<Netlist> nl = nl_res.get();
                EXPECT_NE(nl, nullptr);
            }
            */
            if(test_utils::known_issue_tests_active())
            {
                    // Use non-numeric ranges (invalid) ISSUE: stoi Failure
                    NO_COUT_TEST_BLOCK;
                    std::string netlist_input("module top ("
                             "  global_in,"
                             "  global_out"
                             " ) ;"
                             "  input global_in ;"
                             "  output global_out ;"
                             "  wire [0:4] signal_vec_0 ;"
                             "  wire [0:4] signal_vec_1 ;"
                             "  assign signal_vec_0[p:q] = signal_vec_1[p:q];"
                             "INV gate_0 ("
                             "  .\\I (global_in ),"
                             "  .\\O (global_out )"
                             " ) ;"
                             "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            // ------ Verilog specific tests ------
#if !defined(__APPLE__)
            {
                // The Module has no identifier
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "BUF gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            {
                // implicit net declaration through assign (not yet supported)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input(
                                        "module ();"
                                        "   wire b;"
                                        "   assign a = b;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            {
                // implicit net declaration through port assign (not yet supported)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input(
                                        "module ();"
                                        "   wire b;"
                                        "   BUF gate_0 ("
                                        "       .I(a),"
                                        "       .O(b)"
                                        "   );"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
#endif
            if(test_utils::known_issue_tests_active())
            {
                // one side of the direct assignment is empty
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                         "  global_in,"
                         "  global_out"
                         " ) ;"
                         "  input global_in ;"
                         "  output global_out ;"
                         "  wire signal_0 ;"
                         "  assign signal_0 = ;"
                         "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            {
                // Having a cyclic Module hierarchy
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module ENT_0 ("
                                        "  IE0,"
                                        "  OE0 "
                                        " ) ;"
                                        "  input IE0 ;"
                                        "  output OE0 ;"
                                        "ENT_1 gate_0 ("
                                        "  .\\IE1 (IE0 ),"
                                        "  .\\OE1 (OE0 )"
                                        " ) ;"
                                        "endmodule"
                                        "module ENT_1 ("
                                        "  IE1,"
                                        "  OE1 "
                                        " ) ;"
                                        "  input IE1 ;"
                                        "  output OE1 ;"
                                        "ENT_0 gate_0 ("
                                        "  .\\IE0 (IE1 ),"
                                        "  .\\OE0 (OE1 )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            {
                // Store an unknown data type
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "BUF #("
                                        ".key_unknown(#Unkn0wn!)) "
                                        "gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());    // a value that is no literal is an error; the legacy parser stored it as text
            }
            {
                // Use an undeclared signal
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "BUF gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (net_0 )"    // <- undeclared
                                        " ) ;"
                                        "BUF gate_1 ("
                                        "  .I (net_0 ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());
            }
            {
                // Assign unknown signals
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                         "  global_in,"
                         "  global_out"
                         " ) ;"
                         "  input global_in ;"
                         "  output global_out ;"
                         "  wire [0:4] signal_vec ;"
                         "  assign signal_unknown[0:4] = signal_vec[0:4];"
                         "BUF gate_0 ("
                         "  .I (global_in ),"
                         "  .O (global_out )"
                         " ) ;"
                         "endmodule");
                const GateLibrary* gate_lib = test_utils::get_gate_library();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                auto nl_res = verilog_parser.parse_and_instantiate(verilog_file, gate_lib);
                EXPECT_TRUE(nl_res.is_error());    // assigning to an undeclared signal is an error; the legacy parser ignored it
            }
        TEST_END
    }

    /* ------------------------------------------------------------------------------------------------------------------
     * Edge cases added for the netlist parser rework; the legacy parser skips the ones it cannot pass, this parser
     * passes all of them.
     * ------------------------------------------------------------------------------------------------------------------ */

    /**
     * Escaped identifiers may contain brackets, parentheses, commas and keywords; an escaped `\bus[3]` is a scalar and
     * distinct from bit 3 of a vector `bus`, and the trailing space that ends an escaped identifier never becomes part
     * of its name.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_escaped_identifier_forms)
    {
        TEST_START
            auto nl_res = parse("module top (\\a[3] , \\bus[3] , \\wire , out);\n"
                                "  input \\a[3] ;\n"
                                "  input \\bus[3] ;\n"
                                "  input \\wire ;\n"
                                "  output out;\n"
                                "  wire [3:0] bus;\n"
                                "  wire \\x(y) ;\n"
                                "  wire \\p,q ;\n"
                                "  BUF g0 (.I(\\a[3] ), .O(bus[3]));\n"
                                "  AND2 g1 (.I0(bus[3]), .I1(\\bus[3] ), .O(\\x(y) ));\n"
                                "  AND2 \\g,2 (.I0(\\x(y) ), .I1(\\wire ), .O(out));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* g0 = gate_by_name(nl.get(), "g0");
            Gate* g1 = gate_by_name(nl.get(), "g1");
            Gate* g2 = gate_by_name(nl.get(), "g,2");
            ASSERT_NE(g0, nullptr);
            ASSERT_NE(g1, nullptr);
            ASSERT_NE(g2, nullptr);

            // the escaped name keeps its brackets and is a global input, the vector bit is a different net
            ASSERT_NE(g0->get_fan_in_net("I"), nullptr);
            EXPECT_EQ(g0->get_fan_in_net("I")->get_name(), "a[3]");
            EXPECT_TRUE(nl->is_global_input_net(g0->get_fan_in_net("I")));
            ASSERT_NE(g0->get_fan_out_net("O"), nullptr);
            EXPECT_EQ(g0->get_fan_out_net("O")->get_name(), "bus(3)");
            EXPECT_EQ(g1->get_fan_in_net("I0"), g0->get_fan_out_net("O"));
            ASSERT_NE(g1->get_fan_in_net("I1"), nullptr);
            EXPECT_EQ(g1->get_fan_in_net("I1")->get_name(), "bus[3]");
            EXPECT_NE(g1->get_fan_in_net("I1"), g1->get_fan_in_net("I0"));

            // parentheses, commas and keywords inside escaped names survive
            ASSERT_NE(g1->get_fan_out_net("O"), nullptr);
            EXPECT_EQ(g1->get_fan_out_net("O")->get_name(), "x(y)");
            EXPECT_EQ(g2->get_fan_in_net("I0"), g1->get_fan_out_net("O"));
            ASSERT_NE(g2->get_fan_in_net("I1"), nullptr);
            EXPECT_EQ(g2->get_fan_in_net("I1")->get_name(), "wire");
            EXPECT_TRUE(nl->is_global_input_net(g2->get_fan_in_net("I1")));

            // the unused bits of the vector are dropped, the used one stays
            EXPECT_EQ(net_by_name(nl.get(), "bus(0)"), nullptr);
            EXPECT_EQ(nl->get_nets().size(), 6);
        TEST_END
    }

    /**
     * Verilog identifiers are case-sensitive: `a` and `A` are different nets and `g` and `G` different gates.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_case_sensitive_identifiers)
    {
        TEST_START
            auto nl_res = parse("module top (a, A, out);\n"
                                "  input a;\n"
                                "  input A;\n"
                                "  output out;\n"
                                "  wire Out;\n"
                                "  AND2 g (.I0(a), .I1(A), .O(out));\n"
                                "  BUF G (.I(A), .O(Out));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* g = gate_by_name(nl.get(), "g");
            Gate* G = gate_by_name(nl.get(), "G");
            ASSERT_NE(g, nullptr);
            ASSERT_NE(G, nullptr);
            EXPECT_EQ(g->get_type()->get_name(), "AND2");
            EXPECT_EQ(G->get_type()->get_name(), "BUF");

            Net* a = net_by_name(nl.get(), "a");
            Net* A = net_by_name(nl.get(), "A");
            ASSERT_NE(a, nullptr);
            ASSERT_NE(A, nullptr);
            EXPECT_NE(a, A);
            EXPECT_EQ(g->get_fan_in_net("I0"), a);
            EXPECT_EQ(g->get_fan_in_net("I1"), A);
            EXPECT_EQ(G->get_fan_in_net("I"), A);
            EXPECT_EQ(nl->get_global_input_nets().size(), 2);
            ASSERT_NE(net_by_name(nl.get(), "Out"), nullptr);
            EXPECT_NE(net_by_name(nl.get(), "Out"), net_by_name(nl.get(), "out"));
        TEST_END
    }

    /**
     * Literal forms in connections: underscores are ignored, `x` and `z` bits leave the pin open in any letter case,
     * and a literal narrower than a pin group is zero-extended.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_literal_forms)
    {
        TEST_START
            auto nl_res = parse("module top (out);\n"
                                "  output [3:0] out;\n"
                                "  RAM r (.ADDR(4'b01_10), .DATA_IN(4'b1x0z), .DATA_OUT(out));\n"
                                "  RAM s (.ADDR(2'hZ), .DATA_IN(1'b1), .DATA_OUT());\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* r = gate_by_name(nl.get(), "r");
            ASSERT_NE(r, nullptr);
            ASSERT_NE(r->get_fan_in_net("ADDR(0)"), nullptr);
            EXPECT_TRUE(r->get_fan_in_net("ADDR(0)")->is_gnd_net());
            EXPECT_TRUE(r->get_fan_in_net("ADDR(1)")->is_vcc_net());
            EXPECT_TRUE(r->get_fan_in_net("ADDR(2)")->is_vcc_net());
            EXPECT_TRUE(r->get_fan_in_net("ADDR(3)")->is_gnd_net());

            EXPECT_EQ(r->get_fan_in_net("DATA_IN(0)"), nullptr);    // z
            ASSERT_NE(r->get_fan_in_net("DATA_IN(1)"), nullptr);
            EXPECT_TRUE(r->get_fan_in_net("DATA_IN(1)")->is_gnd_net());
            EXPECT_EQ(r->get_fan_in_net("DATA_IN(2)"), nullptr);    // x
            ASSERT_NE(r->get_fan_in_net("DATA_IN(3)"), nullptr);
            EXPECT_TRUE(r->get_fan_in_net("DATA_IN(3)")->is_vcc_net());

            Gate* s = gate_by_name(nl.get(), "s");
            ASSERT_NE(s, nullptr);
            EXPECT_EQ(s->get_fan_in_net("ADDR(0)"), nullptr);
            EXPECT_EQ(s->get_fan_in_net("ADDR(1)"), nullptr);
            EXPECT_EQ(s->get_fan_in_net("ADDR(2)"), nullptr);
            ASSERT_NE(s->get_fan_in_net("DATA_IN(0)"), nullptr);
            EXPECT_TRUE(s->get_fan_in_net("DATA_IN(0)")->is_vcc_net());
        TEST_END
    }

    /**
     * A literal narrower than the pin group it is connected to is zero-extended, as a port connection of a narrower
     * expression is in Verilog.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_literal_zero_extension)
    {
        TEST_START
            auto nl_res = parse("module top (out);\n"
                                "  output [3:0] out;\n"
                                "  RAM s (.ADDR(2'b10), .DATA_IN(1'b1), .DATA_OUT(out));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* s = gate_by_name(nl.get(), "s");
            ASSERT_NE(s, nullptr);
            for (const auto& [pin, vcc] : std::vector<std::pair<std::string, bool>>{{"ADDR(0)", false}, {"ADDR(1)", true}, {"ADDR(2)", false}, {"ADDR(3)", false}, {"DATA_IN(0)", true}, {"DATA_IN(1)", false}, {"DATA_IN(2)", false}, {"DATA_IN(3)", false}})
            {
                ASSERT_NE(s->get_fan_in_net(pin), nullptr) << pin;
                EXPECT_EQ(s->get_fan_in_net(pin)->is_vcc_net(), vcc) << pin;
                EXPECT_EQ(s->get_fan_in_net(pin)->is_gnd_net(), !vcc) << pin;
            }
        TEST_END
    }

    /**
     * Replication and nested concatenations inside a connection.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_replication_and_nested_concatenation)
    {
        TEST_START
            auto nl_res = parse("module top (a, b, c, out);\n"
                                "  input a, b, c;\n"
                                "  output [3:0] out;\n"
                                "  RAM r (.DATA_IN({{2{a}}, {b, c}}), .DATA_OUT(out));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* r = gate_by_name(nl.get(), "r");
            ASSERT_NE(r, nullptr);
            ASSERT_NE(r->get_fan_in_net("DATA_IN(0)"), nullptr);
            EXPECT_EQ(r->get_fan_in_net("DATA_IN(0)")->get_name(), "c");
            EXPECT_EQ(r->get_fan_in_net("DATA_IN(1)")->get_name(), "b");
            EXPECT_EQ(r->get_fan_in_net("DATA_IN(2)")->get_name(), "a");
            EXPECT_EQ(r->get_fan_in_net("DATA_IN(3)")->get_name(), "a");
        TEST_END
    }

    /**
     * ANSI port kinds: `input wire`, `output reg`, `inout`, and one range shared by two port names.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_ansi_port_kinds)
    {
        TEST_START
            auto nl_res = parse("module top (input wire a, output reg y, inout io, input [1:0] p, q);\n"
                                "  AND2 g (.I0(p[1]), .I1(q[0]), .O(y));\n"
                                "  BUF b (.I(a), .O(io));\n"
                                "  BUF c (.I(io), .O(p[0]));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* g = gate_by_name(nl.get(), "g");
            ASSERT_NE(g, nullptr);
            ASSERT_NE(g->get_fan_in_net("I0"), nullptr);
            EXPECT_EQ(g->get_fan_in_net("I0")->get_name(), "p(1)");
            EXPECT_EQ(g->get_fan_in_net("I1")->get_name(), "q(0)");
            EXPECT_TRUE(nl->is_global_input_net(g->get_fan_in_net("I0")));
            EXPECT_TRUE(nl->is_global_input_net(g->get_fan_in_net("I1")));
            EXPECT_TRUE(nl->is_global_output_net(g->get_fan_out_net("O")));

            // an inout port of the top module is both a global input and a global output
            Net* io = net_by_name(nl.get(), "io");
            ASSERT_NE(io, nullptr);
            EXPECT_TRUE(nl->is_global_input_net(io));
            EXPECT_TRUE(nl->is_global_output_net(io));
            ModulePin* io_pin = nl->get_top_module()->get_pin_by_net(io);
            ASSERT_NE(io_pin, nullptr);
            EXPECT_EQ(io_pin->get_direction(), PinDirection::inout);
        TEST_END
    }

    /**
     * Body declarations: a range shared by several non-ANSI ports, a single-element range, `tri`, and a wire with an
     * initializer, which is an alias like an assign.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_body_declaration_variants)
    {
        TEST_START
            auto nl_res = parse("module top (a, b, y);\n"
                                "  input [1:0] a, b;\n"
                                "  output y;\n"
                                "  wire [3:3] one;\n"
                                "  tri t;\n"
                                "  wire d = a[1];\n"
                                "  AND2 g0 (.I0(a[0]), .I1(b[1]), .O(one[3]));\n"
                                "  AND2 g1 (.I0(one[3]), .I1(d), .O(t));\n"
                                "  BUF g2 (.I(t), .O(y));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* g0 = gate_by_name(nl.get(), "g0");
            Gate* g1 = gate_by_name(nl.get(), "g1");
            Gate* g2 = gate_by_name(nl.get(), "g2");
            ASSERT_NE(g0, nullptr);
            ASSERT_NE(g1, nullptr);
            ASSERT_NE(g2, nullptr);
            EXPECT_EQ(nl->get_global_input_nets().size(), 4);    // b(0) is unused but stays a global input
            ASSERT_NE(g0->get_fan_out_net("O"), nullptr);
            EXPECT_EQ(g0->get_fan_out_net("O")->get_name(), "one(3)");
            EXPECT_EQ(g1->get_fan_in_net("I0"), g0->get_fan_out_net("O"));
            ASSERT_NE(g1->get_fan_in_net("I1"), nullptr);
            EXPECT_TRUE(nl->is_global_input_net(g1->get_fan_in_net("I1")));    // d is an alias of the port bit a[1]
            ASSERT_NE(g1->get_fan_out_net("O"), nullptr);
            EXPECT_EQ(g1->get_fan_out_net("O")->get_name(), "t");
            EXPECT_EQ(g2->get_fan_in_net("I"), g1->get_fan_out_net("O"));
        TEST_END
    }

    /**
     * Positional connections may leave a slot empty, and an instance may have no connections at all.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_positional_connection_gaps)
    {
        TEST_START
            auto nl_res = parse("module top (a, y);\n"
                                "  input a;\n"
                                "  output y;\n"
                                "  AND2 g (a, , y);\n"
                                "  BUF u ();\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* g = gate_by_name(nl.get(), "g");
            ASSERT_NE(g, nullptr);
            ASSERT_NE(g->get_fan_in_net("I0"), nullptr);
            EXPECT_EQ(g->get_fan_in_net("I0")->get_name(), "a");
            EXPECT_EQ(g->get_fan_in_net("I1"), nullptr);
            ASSERT_NE(g->get_fan_out_net("O"), nullptr);
            EXPECT_EQ(g->get_fan_out_net("O")->get_name(), "y");

            Gate* u = gate_by_name(nl.get(), "u");
            ASSERT_NE(u, nullptr);
            EXPECT_TRUE(u->get_fan_in_nets().empty());
            EXPECT_TRUE(u->get_fan_out_nets().empty());
        TEST_END
    }

    /**
     * A module port tied to a constant in the parent connects the module-internal logic to the constant net.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_constant_on_module_port)
    {
        TEST_START
            auto nl_res = parse("module sub (i, j, o);\n"
                                "  input i, j;\n"
                                "  output o;\n"
                                "  AND2 g (.I0(i), .I1(j), .O(o));\n"
                                "endmodule\n"
                                "module top (y);\n"
                                "  output y;\n"
                                "  sub s (.i(1'b0), .j(1'b1), .o(y));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* g = gate_by_name(nl.get(), "g");
            ASSERT_NE(g, nullptr);
            ASSERT_NE(g->get_fan_in_net("I0"), nullptr);
            EXPECT_TRUE(g->get_fan_in_net("I0")->is_gnd_net());
            ASSERT_NE(g->get_fan_in_net("I1"), nullptr);
            EXPECT_TRUE(g->get_fan_in_net("I1")->is_vcc_net());
            EXPECT_TRUE(nl->is_global_output_net(g->get_fan_out_net("O")));
            EXPECT_EQ(nl->get_modules().size(), 2);
        TEST_END
    }

    /**
     * A module may be instantiated before its definition appears in the file.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_forward_module_reference)
    {
        TEST_START
            auto nl_res = parse("module top (a, y);\n"
                                "  input a;\n"
                                "  output y;\n"
                                "  later l (.i(a), .o(y));\n"
                                "endmodule\n"
                                "module later (i, o);\n"
                                "  input i;\n"
                                "  output o;\n"
                                "  BUF g (.I(i), .O(o));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();
            EXPECT_EQ(nl->get_design_name(), "top");
            EXPECT_EQ(nl->get_top_module()->get_type(), "top");
            ASSERT_EQ(nl->get_top_module()->get_submodules().size(), 1);
            EXPECT_EQ(nl->get_top_module()->get_submodules().front()->get_type(), "later");
            Gate* g = gate_by_name(nl.get(), "l/g");
            if (g == nullptr)
            {
                g = gate_by_name(nl.get(), "g");
            }
            ASSERT_NE(g, nullptr);
            EXPECT_TRUE(nl->is_global_input_net(g->get_fan_in_net("I")));
            EXPECT_TRUE(nl->is_global_output_net(g->get_fan_out_net("O")));
        TEST_END
    }

    /**
     * Assign variants: a scalar tied to a constant, concatenations on both sides, a bit-select on the left,
     * a narrower target that takes the low bits, and a high-impedance right side that leaves the target undriven.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_assign_variants)
    {
        TEST_START
            auto nl_res = parse("module top (p, q, wide, out);\n"
                                "  input p, q;\n"
                                "  input [3:0] wide;\n"
                                "  output [3:0] out;\n"
                                "  wire zero, x, y, floating;\n"
                                "  wire [1:0] narrow;\n"
                                "  wire [3:0] sel;\n"
                                "  assign zero = 1'b0;\n"
                                "  assign {x, y} = {p, q};\n"
                                "  assign narrow = wide;\n"
                                "  assign sel[2] = q;\n"
                                "  assign floating = 1'bz;\n"
                                "  RAM r (.ADDR({zero, x, y, floating}), .DATA_IN({narrow, sel[2], sel[3]}), .DATA_OUT(out));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* r = gate_by_name(nl.get(), "r");
            ASSERT_NE(r, nullptr);

            // a high-impedance assignment leaves the target undriven, so the pin sees a net without sources
            Net* floating = r->get_fan_in_net("ADDR(0)");
            ASSERT_NE(floating, nullptr);
            EXPECT_EQ(floating->get_name(), "floating");
            EXPECT_EQ(floating->get_num_of_sources(), 0);

            // {x, y} = {p, q} makes x an alias of p and y an alias of q; both are top-level ports
            Net* y = r->get_fan_in_net("ADDR(1)");
            Net* x = r->get_fan_in_net("ADDR(2)");
            ASSERT_NE(y, nullptr);
            ASSERT_NE(x, nullptr);
            EXPECT_NE(x, y);
            EXPECT_TRUE(nl->is_global_input_net(x));
            EXPECT_TRUE(nl->is_global_input_net(y));

            ASSERT_NE(r->get_fan_in_net("ADDR(3)"), nullptr);
            EXPECT_TRUE(r->get_fan_in_net("ADDR(3)")->is_gnd_net());    // zero

            // sel[3] is declared but never driven, sel[2] is an alias of q
            Net* sel_3 = r->get_fan_in_net("DATA_IN(0)");
            ASSERT_NE(sel_3, nullptr);
            EXPECT_EQ(sel_3->get_name(), "sel(3)");
            EXPECT_EQ(sel_3->get_num_of_sources(), 0);
            EXPECT_EQ(r->get_fan_in_net("DATA_IN(1)"), y);

            // the narrower target takes the low bits of the wider source
            Net* narrow_0 = r->get_fan_in_net("DATA_IN(2)");
            Net* narrow_1 = r->get_fan_in_net("DATA_IN(3)");
            ASSERT_NE(narrow_0, nullptr);
            ASSERT_NE(narrow_1, nullptr);
            EXPECT_NE(narrow_0, narrow_1);
            EXPECT_TRUE(nl->is_global_input_net(narrow_0));
            EXPECT_TRUE(nl->is_global_input_net(narrow_1));
            EXPECT_EQ(nl->get_global_input_nets().size(), 6);    // p, q, wide[3:0]; the unused wide[3:2] stay global inputs
        TEST_END
    }

    /**
     * Logic expressions in continuous assignments are outside the structural subset and must be rejected.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_logic_expression_rejected)
    {
        TEST_START
            NO_COUT_TEST_BLOCK;
            for (const std::string rhs : {"b & c", "~b", "b ? c : 1'b0", "b + c"})
            {
                auto nl_res = parse("module top (b, c, a);\n"
                                    "  input b, c;\n"
                                    "  output a;\n"
                                    "  assign a = " + rhs + ";\n"
                                    "endmodule");
                EXPECT_TRUE(nl_res.is_error()) << "accepted 'assign a = " << rhs << ";'";
            }
        TEST_END
    }

    /**
     * Compiler directives that carry no structural information are ignored.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_ignored_directives)
    {
        TEST_START
            auto nl_res = parse("`timescale 1ns / 1ps\n"
                                "`default_nettype none\n"
                                "`celldefine\n"
                                "module top (a, y);\n"
                                "  input a;\n"
                                "  output y;\n"
                                "  INV g (.I(a), .O(y));\n"
                                "endmodule\n"
                                "`endcelldefine\n"
                                "`resetall\n");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();
            EXPECT_EQ(nl->get_gates().size(), 1);
            EXPECT_NE(gate_by_name(nl.get(), "g"), nullptr);
        TEST_END
    }

    /**
     * Conditional compilation and macros are evaluated: only the active branch of an `ifdef` is parsed and a
     * `define is substituted.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_conditional_compilation)
    {
        TEST_START
            auto nl_res = parse("`define GATE INV\n"
                                "module top (a, y);\n"
                                "  input a;\n"
                                "  output y;\n"
                                "`ifdef UNDEFINED_SYMBOL\n"
                                "  BUF wrong (.I(a), .O(y));\n"
                                "`else\n"
                                "  `GATE g (.I(a), .O(y));\n"
                                "`endif\n"
                                "`ifndef UNDEFINED_SYMBOL\n"
                                "  INV h (.I(y), .O());\n"
                                "`endif\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();
            EXPECT_EQ(nl->get_gates().size(), 2);
            EXPECT_EQ(gate_by_name(nl.get(), "wrong"), nullptr);
            Gate* g = gate_by_name(nl.get(), "g");
            ASSERT_NE(g, nullptr);
            EXPECT_EQ(g->get_type()->get_name(), "INV");
            EXPECT_NE(gate_by_name(nl.get(), "h"), nullptr);
        TEST_END
    }

    /**
     * Behavioral constructs are outside the structural subset and must be rejected with an error instead of being
     * misread as instances.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_unsupported_constructs_rejected)
    {
        TEST_START
            NO_COUT_TEST_BLOCK;
            for (const std::string body : {"initial begin y = 1'b0; end",
                                           "always @(posedge a) y <= a;",
                                           "generate if (1) begin BUF g (.I(a), .O(y)); end endgenerate",
                                           "function f; input x; f = x; endfunction",
                                           "specify (a => y) = 1; endspecify"})
            {
                auto nl_res = parse("module top (a, y);\n"
                                    "  input a;\n"
                                    "  output y;\n"
                                    "  " + body + "\n"
                                    "endmodule");
                EXPECT_TRUE(nl_res.is_error()) << "accepted '" << body << "'";
            }
        TEST_END
    }

    /**
     * Line endings and comment placement: CRLF files, a last line without newline, comments inside a port list and
     * inside a range, and comment markers inside a string.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_line_endings_and_comment_placement)
    {
        TEST_START
            const std::string body = "module top (a, /* between ports */ y);\n"
                                     "  input a; // trailing\n"
                                     "  output y;\n"
                                     "  wire [1 /* inside a range */ :0] v;\n"
                                     "  (* note = \"a//b/*c*/\" *) BUF g (.I(a), .O(v[1]));\n"
                                     "  BUF /* between type and name */ h (.I(v[1]), .O(y));\n"
                                     "endmodule";

            for (const bool crlf : {false, true})
            {
                std::string text = body;
                if (crlf)
                {
                    std::string converted;
                    for (const char c : text)
                    {
                        if (c == '\n')
                        {
                            converted += '\r';
                        }
                        converted += c;
                    }
                    text = converted;
                }

                auto nl_res = parse(text);
                ASSERT_TRUE(nl_res.is_ok()) << (crlf ? "CRLF: " : "LF: ") << nl_res.get_error().get();
                auto nl = nl_res.get();
                EXPECT_EQ(nl->get_gates().size(), 2);
                Gate* g = gate_by_name(nl.get(), "g");
                Gate* h = gate_by_name(nl.get(), "h");
                ASSERT_NE(g, nullptr);
                ASSERT_NE(h, nullptr);
                ASSERT_NE(g->get_fan_out_net("O"), nullptr);
                EXPECT_EQ(g->get_fan_out_net("O")->get_name(), "v(1)");
                EXPECT_EQ(h->get_fan_in_net("I"), g->get_fan_out_net("O"));
                EXPECT_TRUE(nl->is_global_output_net(h->get_fan_out_net("O")));
                EXPECT_EQ(std::get<1>(attribute_of(g, "note")), "a//b/*c*/");
            }
        TEST_END
    }

    /**
     * A module whose output is assigned straight from its input is a pass-through, and an inout module port keeps its
     * direction.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_pass_through_and_inout_modules)
    {
        TEST_START
            auto nl_res = parse("module pass (i, o);\n"
                                "  input i;\n"
                                "  output o;\n"
                                "  assign o = i;\n"
                                "endmodule\n"
                                "module bidi (io, o);\n"
                                "  inout io;\n"
                                "  output o;\n"
                                "  BUF b (.I(io), .O(o));\n"
                                "  BUF c (.I(o), .O(io));\n"
                                "endmodule\n"
                                "module top (a, y, z);\n"
                                "  input a;\n"
                                "  output y, z;\n"
                                "  wire m;\n"
                                "  pass p (.i(a), .o(m));\n"
                                "  bidi q (.io(m), .o(y));\n"
                                "  BUF g (.I(m), .O(z));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();

            Gate* g = gate_by_name(nl.get(), "g");
            ASSERT_NE(g, nullptr);
            ASSERT_NE(g->get_fan_in_net("I"), nullptr);
            EXPECT_TRUE(nl->is_global_input_net(g->get_fan_in_net("I")));

            Gate* b = gate_by_name(nl.get(), "b");
            if (b == nullptr)
            {
                b = gate_by_name(nl.get(), "q/b");
            }
            ASSERT_NE(b, nullptr);
            EXPECT_EQ(b->get_fan_in_net("I"), g->get_fan_in_net("I"));

            // the net is driven inside bidi as well as outside and read on both sides, so the module pin is inout
            Module* bidi = b->get_module();
            ASSERT_NE(bidi, nullptr);
            EXPECT_EQ(bidi->get_type(), "bidi");
            EXPECT_EQ(b->get_fan_in_net("I")->get_num_of_sources(), 1);
            ModulePin* io_pin = bidi->get_pin_by_net(b->get_fan_in_net("I"));
            ASSERT_NE(io_pin, nullptr);
            EXPECT_EQ(io_pin->get_name(), "io");
            EXPECT_EQ(io_pin->get_direction(), PinDirection::inout);
        TEST_END
    }

    /**
     * Two outputs driving one wire give a net with two sources; the parser does not reject that.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_multiple_drivers)
    {
        TEST_START
            auto nl_res = parse("module top (a, b, y);\n"
                                "  input a, b;\n"
                                "  output y;\n"
                                "  BUF g (.I(a), .O(y));\n"
                                "  BUF h (.I(b), .O(y));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();
            Net* y = net_by_name(nl.get(), "y");
            ASSERT_NE(y, nullptr);
            EXPECT_EQ(y->get_num_of_sources(), 2);
            EXPECT_TRUE(nl->is_global_output_net(y));
        TEST_END
    }

    /**
     * A port of the top module that nothing inside the module touches still is part of the interface: its net stays
     * and is a global input or output.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_unconnected_top_port_kept)
    {
        TEST_START
            auto nl_res = parse("module top (a, unused_in, y, unused_out);\n"
                                "  input a, unused_in;\n"
                                "  output y, unused_out;\n"
                                "  BUF g (.I(a), .O(y));\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();
            Net* unused_in  = net_by_name(nl.get(), "unused_in");
            Net* unused_out = net_by_name(nl.get(), "unused_out");
            ASSERT_NE(unused_in, nullptr);
            ASSERT_NE(unused_out, nullptr);
            EXPECT_TRUE(nl->is_global_input_net(unused_in));
            EXPECT_TRUE(nl->is_global_output_net(unused_out));
            EXPECT_EQ(nl->get_global_input_nets().size(), 2);
            EXPECT_EQ(nl->get_global_output_nets().size(), 2);
        TEST_END
    }

    /**
     * Duplicate declarations of a wire, a module, or an instance name are errors.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_duplicate_declarations_rejected)
    {
        TEST_START
            NO_COUT_TEST_BLOCK;
            {
                auto nl_res = parse("module top (a, y);\n"
                                    "  input a;\n"
                                    "  output y;\n"
                                    "  BUF g (.I(a), .O(y));\n"
                                    "endmodule\n"
                                    "module top (a, y);\n"
                                    "  input a;\n"
                                    "  output y;\n"
                                    "  INV g (.I(a), .O(y));\n"
                                    "endmodule");
                EXPECT_TRUE(nl_res.is_error()) << "accepted a module declared twice";
            }
            {
                auto nl_res = parse("module top (a, y);\n"
                                    "  input a;\n"
                                    "  output y;\n"
                                    "  wire w;\n"
                                    "  wire w;\n"
                                    "  BUF g (.I(a), .O(w));\n"
                                    "  BUF h (.I(w), .O(y));\n"
                                    "endmodule");
                EXPECT_TRUE(nl_res.is_error()) << "accepted a wire declared twice";
            }
            {
                auto nl_res = parse("module top (a, y, z);\n"
                                    "  input a;\n"
                                    "  output y, z;\n"
                                    "  BUF g (.I(a), .O(y));\n"
                                    "  BUF g (.I(a), .O(z));\n"
                                    "endmodule");
                EXPECT_TRUE(nl_res.is_error()) << "accepted an instance name used twice";
            }
        TEST_END
    }

    /**
     * Parameter declarations in a module header and body are accepted, positional parameter overrides and a defparam
     * on a module instance end up on the module.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_parameter_declaration_forms)
    {
        TEST_START
            auto nl_res = parse("module sub #(parameter WIDTH = 8, DEPTH = 2) (i, o);\n"
                                "  input i;\n"
                                "  output o;\n"
                                "  localparam HALF = WIDTH / 2;\n"
                                "  BUF g (.I(i), .O(o));\n"
                                "endmodule\n"
                                "module top (a, y, z);\n"
                                "  input a;\n"
                                "  output y, z;\n"
                                "  sub #(.WIDTH(16)) s0 (.i(a), .o(y));\n"
                                "  sub s1 (.i(a), .o(z));\n"
                                "  defparam s1.DEPTH = 4;\n"
                                "endmodule");
            ASSERT_TRUE(nl_res.is_ok()) << nl_res.get_error().get();
            auto nl = nl_res.get();
            ASSERT_EQ(nl->get_top_module()->get_submodules().size(), 2);
            Module* s0 = nullptr;
            Module* s1 = nullptr;
            for (Module* m : nl->get_top_module()->get_submodules())
            {
                if (m->get_name() == "s0")
                {
                    s0 = m;
                }
                else if (m->get_name() == "s1")
                {
                    s1 = m;
                }
            }
            ASSERT_NE(s0, nullptr);
            ASSERT_NE(s1, nullptr);
            EXPECT_EQ(std::get<1>(parameter_of(s0, "WIDTH")), "16");
            EXPECT_EQ(std::get<1>(parameter_of(s1, "DEPTH")), "4");
        TEST_END
    }
} // namespace hal
