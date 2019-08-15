#include "gtest/gtest.h"
#include "test_def.h"
#include <core/log.h>
#include <core/utils.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist_factory.h"
#include "netlist/netlist.h"
#include "hdl_parser/hdl_parser_vhdl_old.h"
#include "hdl_writer/hdl_writer_vhdl.h"


class hdl_writer_vhdl_test : public ::testing::Test
{
protected:

    const std::string g_lib_name = "EXAMPLE_GATE_LIBRARY";

    // Minimum id for netlists, gates, nets and modules
    const u32 INVALID_GATE_ID = 0;
    const u32 INVALID_NET_ID = 0;
    const u32 INVALID_MODULE_ID = 0;
    const u32 MIN_MODULE_ID = 2;
    const u32 MIN_GATE_ID = 1;
    const u32 MIN_NET_ID = 1;
    const u32 MIN_NETLIST_ID = 1;
    const u32 TOP_MODULE_ID = 1;

    const std::string GATE_SUFFIX = "";


    virtual void SetUp() {
        NO_COUT_BLOCK;
        gate_library_manager::load_all();
    }

    virtual void TearDown() {

    }

    // Creates an empty netlist with a certain id if passed
    std::shared_ptr<netlist> create_empty_netlist(const int id = -1) {
        NO_COUT_BLOCK;
        std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
        std::shared_ptr<netlist> nl(new netlist(gl));

        if (id >= 0) {
            nl->set_id(id);
        }
        return nl;
    }

    // Creates an endpoint from a gate and a pin_type
    endpoint get_endpoint(std::shared_ptr<gate> g, std::string pin_type){
        endpoint ep;
        ep.gate = g;
        ep.pin_type = pin_type;
        return ep;
    }

    endpoint get_dst_by_pin_type(std::vector<endpoint> dsts ,std::string pin_type){
        for(auto dst : dsts){
            if(dst.get_pin_type() == pin_type){
                return dst;
            }
        }
        return {nullptr, ""};
    }

    // Returns true if two nets have the same content (id and name doesn't matter)
    bool nets_are_equal(std::shared_ptr<net> n0, std::shared_ptr<net> n1){
        if(n0 == nullptr || n1 == nullptr){
            if(n0 == n1)
                return true;
            else
                return false;
        }
        if(n0->get_src().get_pin_type() != n1->get_src().get_pin_type())
            return false;
        if(!gates_are_equal(n0->get_src().get_gate(), n1->get_src().get_gate()))
            return false;
        for(auto n0_dst : n0->get_dsts()){
            if (!gates_are_equal(n0_dst.get_gate(), get_dst_by_pin_type(n1->get_dsts(), n0_dst.get_pin_type()).get_gate())){
                return false;
            }
        }
        return true;


    }

    // Returns true if the two gates have the same content (id and name doesn't matter)
    bool gates_are_equal(std::shared_ptr<gate> g0, std::shared_ptr<gate> g1){
        if(g0 == nullptr || g1 == nullptr){
            if(g0 == g1)
                return true;
            else
                return false;
        }
        if(g0->get_type() != g1->get_type())
            return false;
        if(g0->get_data() != g1->get_data())
            return false;
        return true;
    }



    /*
      *      Example netlist circuit diagram (Id in brackets). Used for get fan in and
      *      out nets.
      *
      *
      *      GND (1) =-= INV (3) =--=             .------=  INV (4)  =---
      *                                 AND2 (0) =-
      *      VCC (2) =--------------=             '------=
      *                                                     AND2 (5) =---
      *                                                  =
      *
      *                           =                .-----=
      *                              OR2 (6)  =----'        OR2 (7)  =---
      *                           =                      =
      */

    // Creates a simple netlist shown in the diagram above. The nets that have a GND/VCC gate as a source are named '0'/'1'
    std::shared_ptr<netlist> create_example_parse_netlist(int id = -1)
    {
        //NO_COUT_BLOCK;
        std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
        std::shared_ptr<netlist> nl      = std::make_shared<netlist>(gl);
        if (id >= 0)
        {
            nl->set_id(id);
        }

        // Create the gates
        std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, "AND2", "gate_0");
        std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, "GND", "gate_1");
        std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, "VCC", "gate_2");
        std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, "INV", "gate_3");
        std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_GATE_ID+4, "INV", "gate_4");
        std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_GATE_ID+5, "AND2", "gate_5");
        //std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID+6, "BUF", "gate_6");
        std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID+6, "OR2", "gate_6");
        std::shared_ptr<gate> gate_7 = nl->create_gate(MIN_GATE_ID+7, "OR2", "gate_7");

        // Add the nets (net_x_y1_y2... := net between the gate with id x and the gates y1,y2,...)
        std::shared_ptr<net> net_1_3 = nl->create_net(MIN_NET_ID+13, "0_net");
        net_1_3->set_src(gate_1, "O");
        net_1_3->add_dst(gate_3, "I");

        std::shared_ptr<net> net_3_0 = nl->create_net(MIN_NET_ID+30, "net_3_0");
        net_3_0->set_src(gate_3, "O");
        net_3_0->add_dst(gate_0, "I0");

        std::shared_ptr<net> net_2_0 = nl->create_net(MIN_NET_ID+20, "1_net");
        net_2_0->set_src(gate_2, "O");
        net_2_0->add_dst(gate_0, "I1");

        std::shared_ptr<net> net_0_4_5 = nl->create_net(MIN_NET_ID+045, "net_0_4_5");
        net_0_4_5->set_src(gate_0, "O");
        net_0_4_5->add_dst(gate_4, "I");
        net_0_4_5->add_dst(gate_5, "I0");

        std::shared_ptr<net> net_6_7 = nl->create_net(MIN_NET_ID+67, "net_6_7");
        net_6_7->set_src(gate_6, "O");
        net_6_7->add_dst(gate_7, "I0");

        std::shared_ptr<net> net_4_out = nl->create_net(MIN_NET_ID+400, "net_4_out");
        net_4_out->set_src(gate_4, "O");

        std::shared_ptr<net> net_5_out = nl->create_net(MIN_NET_ID+500, "net_5_out");
        net_5_out->set_src(gate_5, "O");

        std::shared_ptr<net> net_7_out = nl->create_net(MIN_NET_ID+700, "net_7_out");
        net_7_out->set_src(gate_7, "O");

        return nl;
    }

    // Get a net of netlist nl by its name (name must be unique)
    std::shared_ptr<net> get_net_by_subname(std::shared_ptr<netlist> nl, const std::string subname){
        /*std::set<std::shared_ptr<net>> res = nl->get_nets(name);
        if(res.size() == 0){
            return nullptr;
        }
        if(res.size() >= 2){
            std::cerr << "Multiple nets with the same name! That should not happen..." << std::endl;
            return nullptr;
        }
        return *res.begin();*/
        if(nl == nullptr)
            return nullptr;
        std::set<std::shared_ptr<net>> nets = nl->get_nets();
        std::shared_ptr<net> res = nullptr;
        for (auto n : nets){
            std::string n_name = n->get_name();
            if (n_name.find(subname) != n_name.npos){
                if (res != nullptr){
                    std::cerr << "Multiple gates contains the subtring '" << subname << "'! This should not happen..." << std::endl;
                    return nullptr;
                }
                res = n;
            }
        }
        return res;

    }

    // Get a gate of netlist nl, which name contains the passed subname (name must be unique). Since the writer adds a
    // certain suffix we can ignore additional characters and only focus on the substring
    std::shared_ptr<gate> get_gate_by_subname(std::shared_ptr<netlist> nl, const std::string subname){
        if(nl == nullptr)
            return nullptr;
        std::set<std::shared_ptr<gate>> gates = nl->get_gates();
        std::shared_ptr<gate> res = nullptr;
        for (auto g : gates){
            std::string g_name = g->get_name();
            if (g_name.find(subname) != g_name.npos){
                if (res != nullptr){
                    std::cerr << "Multiple gates contains the subtring '" << subname << "'! This should not happen..." << std::endl;
                    return nullptr;
                }
                res = g;
            }
        }
        return res;
    }
};


/**
 * Testing to write a given netlist in a sstream and parses it after, with
 * the hdl_parser_vhdl.
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
TEST_F(hdl_writer_vhdl_test, check_write_and_parse_main_example) {
    TEST_START
        { //NOTE: Fails because of an issue in the parser (empty 'port map' can't be parsed)
            // Write and parse the example netlist (with some additions) and compare the result with the original netlist
            std::shared_ptr<netlist> nl = create_example_parse_netlist(0);


            // Mark the global gates as such
            nl->mark_global_gnd_gate(nl->get_gate_by_id(MIN_GATE_ID+1));
            nl->mark_global_vcc_gate(nl->get_gate_by_id(MIN_GATE_ID+2));


            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);


            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);

            if(!writer_suc){
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if(parsed_nl == nullptr){
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the original netlist and the parsed one are equal

            // -- Check if gates and nets are the same
            EXPECT_EQ(nl->get_gates().size(), parsed_nl->get_gates().size());
            for(auto g_0 : nl->get_gates()){
                EXPECT_TRUE(gates_are_equal(g_0, get_gate_by_subname(parsed_nl, g_0->get_name())));
            }

            EXPECT_EQ(nl->get_nets().size(), parsed_nl->get_nets().size());

            for(auto n_0 : nl->get_nets()){
                EXPECT_TRUE(nets_are_equal(n_0, get_net_by_subname(parsed_nl, n_0->get_name())));
            }

            // -- Check if global gates are the same
            EXPECT_EQ(nl->get_global_gnd_gates().size(), parsed_nl->get_global_gnd_gates().size());
            for(auto gl_gnd_0 : nl->get_global_gnd_gates()){
                EXPECT_TRUE(parsed_nl->is_global_gnd_gate(get_gate_by_subname(parsed_nl, gl_gnd_0->get_name())));
            }

            EXPECT_EQ(nl->get_global_vcc_gates().size(), parsed_nl->get_global_vcc_gates().size());
            for(auto gl_vcc_0 : nl->get_global_vcc_gates()){
                EXPECT_TRUE(parsed_nl->is_global_vcc_gate(get_gate_by_subname(parsed_nl, gl_vcc_0->get_name())));
            }
        }
    TEST_END
}


/**
 * Testing the writing of global input/output/inout nets
 *
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
TEST_F(hdl_writer_vhdl_test, check_global_nets) {
    TEST_START
        {
            // Add 2 global input nets to an empty netlist
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<net> global_in_0 = nl->create_net( MIN_NET_ID+0, "0_global_in");
            std::shared_ptr<net> global_in_1 = nl->create_net( MIN_NET_ID+1, "1_global_in");

            nl->mark_global_input_net(global_in_0);
            nl->mark_global_input_net(global_in_1);

            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();


            // Check if the nets are written/parsed correctly
            std::shared_ptr<net> p_global_in_0 = get_net_by_subname(parsed_nl, "0_global_in");
            ASSERT_NE(p_global_in_0, nullptr);
            EXPECT_TRUE(parsed_nl->is_global_input_net(p_global_in_0));

            std::shared_ptr<net> p_global_in_1 = get_net_by_subname(parsed_nl, "1_global_in");
            ASSERT_NE(p_global_in_1, nullptr);
            EXPECT_TRUE(parsed_nl->is_global_input_net(p_global_in_1));

        }
        {
            // Add 2 global output nets to an empty netlist
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<net> global_out_0 = nl->create_net( MIN_NET_ID+0, "0_global_out");
            std::shared_ptr<net> global_out_1 = nl->create_net( MIN_NET_ID+1, "1_global_out");

            nl->mark_global_output_net(global_out_0);
            nl->mark_global_output_net(global_out_1);

            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the nets are written/parsed correctly
            std::shared_ptr<net> p_global_out_0 = get_net_by_subname(parsed_nl, "0_global_out");
            ASSERT_NE(p_global_out_0, nullptr);
            EXPECT_TRUE(parsed_nl->is_global_output_net(p_global_out_0));

            std::shared_ptr<net> p_global_out_1 = get_net_by_subname(parsed_nl, "1_global_out");
            ASSERT_NE(p_global_out_1, nullptr);
            EXPECT_TRUE(parsed_nl->is_global_output_net(p_global_out_1));

        }
        {
            // Add 2 global inout nets to an empty netlist
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<net> global_inout_0 = nl->create_net( MIN_NET_ID+0, "0_global_inout");
            std::shared_ptr<net> global_inout_1 = nl->create_net( MIN_NET_ID+1, "1_global_inout");

            nl->mark_global_inout_net(global_inout_0);
            nl->mark_global_inout_net(global_inout_1);

            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the nets are written/parsed correctly
            std::shared_ptr<net> p_global_inout_0 = get_net_by_subname(parsed_nl, "0_global_inout");
            ASSERT_NE(p_global_inout_0, nullptr);
            EXPECT_TRUE(parsed_nl->is_global_inout_net(p_global_inout_0));

            std::shared_ptr<net> p_global_inout_1 = get_net_by_subname(parsed_nl, "1_global_inout");
            ASSERT_NE(p_global_inout_1, nullptr);
            EXPECT_TRUE(parsed_nl->is_global_inout_net(p_global_inout_1));

        }
    TEST_END
}

/**
 * Testing the storage of generic data within gates
 *
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
TEST_F(hdl_writer_vhdl_test, check_generic_data_storage) {
    TEST_START
        { //NOTE: Need to assure that the nl is valid. by passing output nets
            // Add a gate to the netlist and store some data
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<net> global_in = nl->create_net( MIN_NET_ID+0, "global_in");
            nl->mark_global_input_net(global_in);

            std::shared_ptr<gate> test_gate_0 = nl->create_gate( MIN_GATE_ID+0, "INV", "test_gate_0");
            std::shared_ptr<gate> test_gate_1 = nl->create_gate( MIN_GATE_ID+1, "INV", "test_gate_1");
            std::shared_ptr<gate> test_gate_2 = nl->create_gate( MIN_GATE_ID+2, "INV", "test_gate_2");
            std::shared_ptr<gate> test_gate_3 = nl->create_gate( MIN_GATE_ID+3, "INV", "test_gate_3");
            std::shared_ptr<gate> test_gate_4 = nl->create_gate( MIN_GATE_ID+4, "INV", "test_gate_4");
            std::shared_ptr<gate> test_gate_5 = nl->create_gate( MIN_GATE_ID+5, "INV", "test_gate_5");
            std::shared_ptr<gate> test_gate_6 = nl->create_gate( MIN_GATE_ID+6, "INV", "test_gate_6");

            // Create output nets for all gates to create a valid netlist
            unsigned int idx = 0;
            for (auto g : nl->get_gates()){
                std::shared_ptr<net> out_net = nl->create_net("net_" + std::to_string(idx));
                out_net->set_src(g,"O");
                idx++;
            }

            global_in->add_dst(test_gate_0, "I");
            global_in->add_dst(test_gate_1, "I");
            global_in->add_dst(test_gate_2, "I");
            global_in->add_dst(test_gate_3, "I");
            global_in->add_dst(test_gate_4, "I");
            global_in->add_dst(test_gate_5, "I");
            global_in->add_dst(test_gate_6, "I");

            // Store some data in the test_gate
            test_gate_0->set_data("generic", "0_key_time", "time", "123s");
            test_gate_0->set_data("generic", "1_key_bit_vector", "bit_vector", "123abc");
            test_gate_0->set_data("generic", "2_key_bit_vector", "bit_vector", "456def");

            test_gate_1->set_data("generic", "0_key_bit_vector", "bit_vector", "123abc");
            test_gate_1->set_data("generic", "1_key_string", "string", "one_two_three");

            test_gate_2->set_data("generic", "0_key_string", "string", "one_two_three");
            test_gate_2->set_data("generic", "1_key_boolean", "boolean", "true");

            test_gate_3->set_data("generic", "0_key_boolean", "boolean", "true");
            test_gate_3->set_data("generic", "1_key_integer", "integer", "123");

            test_gate_4->set_data("generic", "0_key_integer", "integer", "123");
            test_gate_4->set_data("generic", "1_key_time", "time", "123s");

            test_gate_5->set_data("generic", "0_key_invalid", "invalid", "ignore_me" ); // Should be ignored
            test_gate_5->set_data("generic", "1_key_bit_vector", "bit_vector", "1");

            test_gate_6->set_data("generic", "0_key_bit_value", "bit_value", "10101100");
            test_gate_6->set_data("generic", "1_key_bit_value", "bit_value", "01010011");



            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            std::cout << parser_input.str() << std::endl;
            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the data is written/parsed correctly
            std::shared_ptr<gate> p_test_gate_0 = get_gate_by_subname(parsed_nl, "test_gate_0");
            ASSERT_NE(p_test_gate_0, nullptr);
            //EXPECT_EQ(p_test_gate_0->get_data(), test_gate_0->get_data()); //NOTE: generic, time can't be used

            std::shared_ptr<gate> p_test_gate_1 = get_gate_by_subname(parsed_nl, "test_gate_1");
            ASSERT_NE(p_test_gate_1, nullptr);
            EXPECT_EQ(p_test_gate_1->get_data(), test_gate_1->get_data());

            std::shared_ptr<gate> p_test_gate_2 = get_gate_by_subname(parsed_nl, "test_gate_2");
            ASSERT_NE(p_test_gate_2, nullptr);
            EXPECT_EQ(p_test_gate_2->get_data(), test_gate_2->get_data());

            std::shared_ptr<gate> p_test_gate_3 = get_gate_by_subname(parsed_nl, "test_gate_3");
            ASSERT_NE(p_test_gate_3, nullptr);
            EXPECT_EQ(p_test_gate_3->get_data(), test_gate_3->get_data());

            std::shared_ptr<gate> p_test_gate_4 = get_gate_by_subname(parsed_nl, "test_gate_4");
            ASSERT_NE(p_test_gate_4, nullptr);
            //EXPECT_EQ(p_test_gate_4->get_data(), test_gate_4->get_data()); //NOTE: generic, time can't be used

            std::shared_ptr<gate> p_test_gate_5 = get_gate_by_subname(parsed_nl, "test_gate_5");
            ASSERT_NE(p_test_gate_5, nullptr);
            auto test_gate_5_data_without_invalid = test_gate_5->get_data();
            test_gate_5_data_without_invalid.erase(std::make_tuple("generic", "0_key_invalid"));
            EXPECT_EQ(p_test_gate_5->get_data(), test_gate_5_data_without_invalid);

            std::shared_ptr<gate> p_test_gate_6 = get_gate_by_subname(parsed_nl, "test_gate_6");
            ASSERT_NE(p_test_gate_6, nullptr);
            EXPECT_EQ(p_test_gate_6->get_data(), test_gate_6->get_data());
        }
    TEST_END
}

/**
 * Testing the handling of net names which contains only digits (i.e. 123 should become NET_123)
 *
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
TEST_F(hdl_writer_vhdl_test, check_digit_net_name) {
    TEST_START
        {
            // Add a gate to the netlist and store some data
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<net> global_in = nl->create_net( MIN_NET_ID+0, "123");
            nl->mark_global_input_net(global_in);


            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the net_name is written/parsed correctly
            ASSERT_EQ(parsed_nl->get_nets().size(), (size_t)1);
            std::shared_ptr<net> p_global_in = *parsed_nl->get_nets().begin();
            EXPECT_EQ(p_global_in->get_name(), "NET_123");

        }
    TEST_END
}


/*
 *    ONE =---=
 *               AND2 =---
 *   ZERO =---=
 */
/**
 * Testing the handling of vcc and gnd gates (ONE and ZERO)
 *
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
TEST_F(hdl_writer_vhdl_test, check_vcc_and_gnd_gates) {
    TEST_START
        /*{ // NOTE: X_ONE, X_ZERO are no gate types in the example gate library...
            // Two nets with individual names connect a test_gate with a gnd/vcc gate
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            // Add the gates
            std::shared_ptr<gate> gnd_gate = nl->create_gate( MIN_GATE_ID+0, "ZERO", "gnd_gate");
            std::shared_ptr<gate> vcc_gate = nl->create_gate( MIN_GATE_ID+1, "ONE", "vcc_gate");
            std::shared_ptr<gate> test_gate = nl->create_gate( MIN_GATE_ID+2, "AND2", "test_gate");


            // Add and connect the nets
            std::shared_ptr<net> zero_net = nl->create_net(MIN_GATE_ID+0,"zero_net");
            std::shared_ptr<net> one_net = nl->create_net(MIN_GATE_ID+1,"one_net");


            zero_net->set_src(gnd_gate, "O");
            zero_net->add_dst(test_gate, "I0");

            one_net->set_src(vcc_gate, "O");
            one_net->add_dst(test_gate, "I1");


            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the GND/VCC gates and nets are written/parsed correctly
            std::shared_ptr<gate> p_vcc_gate = get_gate_by_name(parsed_nl,  "vcc_gate");
            std::shared_ptr<gate> p_gnd_gate = get_gate_by_name(parsed_nl,  "gnd_gate");
            std::shared_ptr<gate> p_test_gate = get_gate_by_subname(parsed_nl,  "test_gate");

            ASSERT_NE(vcc_gate, nullptr);
            ASSERT_NE(gnd_gate, nullptr);
            ASSERT_NE(test_gate, nullptr);

            ASSERT_EQ(parsed_nl->get_nets().size(), (size_t)2);
            std::shared_ptr<net> p_zero_net = get_net_by_name(parsed_nl,  "zero_net");
            std::shared_ptr<net> p_one_net = get_net_by_subname(parsed_nl,  "one_net");
            ASSERT_NE(zero_net, nullptr);
            ASSERT_NE(one_net, nullptr);

            EXPECT_EQ(p_zero_net->get_src(), get_endpoint(p_gnd_gate, "O"));
            EXPECT_EQ(p_one_net->get_src(), get_endpoint(p_vcc_gate, "O"));

            EXPECT_TRUE(p_zero_net->is_a_dst(get_endpoint(p_test_gate, "I0")));
            EXPECT_TRUE(p_one_net->is_a_dst(get_endpoint(p_test_gate, "I1")));


        }
        {
            // Two nets ('0' and '1') connect a test_gate with a gnd/vcc gate. Here the gates are substituted
            // by GLOBAL_GND/VCC gates
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            // Add the gates
            std::shared_ptr<gate> gnd_gate = nl->create_gate( MIN_GATE_ID+0, "ZERO", "gnd_gate");
            std::shared_ptr<gate> vcc_gate = nl->create_gate( MIN_GATE_ID+1, "ONE", "vcc_gate");
            std::shared_ptr<gate> test_gate = nl->create_gate( MIN_GATE_ID+2, "AND2", "test_gate");


            // Add and connect the nets
            std::shared_ptr<net> zero_net = nl->create_net(MIN_NET_ID+0,"'0'");
            std::shared_ptr<net> one_net = nl->create_net(MIN_NET_ID+1,"'1'");

            zero_net->set_src(gnd_gate, "O");
            zero_net->add_dst(test_gate, "I0");

            one_net->set_src(vcc_gate, "O");
            one_net->add_dst(test_gate, "I1");


            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // NOTE: Requirements?

        }*/
    TEST_END
}


/**
 * Testing the handling of net/gate names with special characters and their translation.
 * Special characters: '(', ')', ',', ', ', '/', '\', '[', ']', '<', '>', '__', '_'
 * Other special cases: only digits, '_' at the beginning or at the end
 *
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
TEST_F(hdl_writer_vhdl_test, check_special_net_names) {
    TEST_START
        {
            // Testing the handling of special net names
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<net> bracket_net = nl->create_net( MIN_NET_ID+0, "net(0)");
            std::shared_ptr<net> comma_net = nl->create_net( MIN_NET_ID+1, "net,1");
            std::shared_ptr<net> comma_space_net = nl->create_net( MIN_NET_ID+2, "net, 2");
            std::shared_ptr<net> slash_net = nl->create_net( MIN_NET_ID+3, "net/_3");
            std::shared_ptr<net> backslash_net = nl->create_net( MIN_NET_ID+4, "net\\_4");
            std::shared_ptr<net> curly_bracket_net = nl->create_net( MIN_NET_ID+5, "net[5]");
            std::shared_ptr<net> angle_bracket_net = nl->create_net( MIN_NET_ID+6, "net<6>");
            std::shared_ptr<net> double_underscore_net = nl->create_net( MIN_NET_ID+7, "net__7");
            std::shared_ptr<net> edges_underscore_net = nl->create_net( MIN_NET_ID+8, "_net_8_");
            std::shared_ptr<net> digit_only_net = nl->create_net( MIN_NET_ID+9, "9"); // should be converted to NET_9

            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the net_name is translated correctly
            EXPECT_FALSE(parsed_nl->get_nets("net_0").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_1").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_2").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_3").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_4").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_5").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_6").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_7").empty());
            EXPECT_FALSE(parsed_nl->get_nets("net_8").empty());
            EXPECT_FALSE(parsed_nl->get_nets("NET_9").empty());
        }
        {
            // Testing the handling of special gate names
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            // Create various gates with special gate name characters
            std::shared_ptr<gate> bracket_gate = nl->create_gate( MIN_GATE_ID+0, "INV", "gate(0)");
            std::shared_ptr<gate> comma_gate = nl->create_gate( MIN_GATE_ID+1, "INV", "gate,1");
            std::shared_ptr<gate> comma_space_gate = nl->create_gate( MIN_GATE_ID+2, "INV", "gate, 2");
            std::shared_ptr<gate> slash_gate = nl->create_gate( MIN_GATE_ID+3, "INV", "gate/_3");
            std::shared_ptr<gate> backslash_gate = nl->create_gate( MIN_GATE_ID+4, "INV", "gate\\_4");
            std::shared_ptr<gate> curly_bracket_gate = nl->create_gate( MIN_GATE_ID+5, "INV", "gate[5]");
            std::shared_ptr<gate> angle_bracket_gate = nl->create_gate( MIN_GATE_ID+6, "INV", "gate<6>");
            std::shared_ptr<gate> double_underscore_gate = nl->create_gate( MIN_GATE_ID+7, "INV", "gate__7");
            std::shared_ptr<gate> edges_underscore_gate = nl->create_gate( MIN_GATE_ID+8, "INV", "_gate_8_");
            std::shared_ptr<gate> digit_only_gate = nl->create_gate( MIN_GATE_ID+9, "INV", "9"); // should be converted to GATE_9

            // Create output nets for all gates to create a valid netlist
            unsigned int idx = 0;
            for (auto g : nl->get_gates()){
                std::shared_ptr<net> out_net = nl->create_net("net_" + std::to_string(idx));
                out_net->set_src(g,"O");
                idx++;
            }

            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the gate_name is translated correctly
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_0" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_1" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_2" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_3" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_4" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_5" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_6" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_7" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "gate_8" + GATE_SUFFIX).empty());
            EXPECT_FALSE(parsed_nl->get_gates(DONT_CARE, "GATE_9" + GATE_SUFFIX).empty());
        }
    TEST_END
}

/**
 * Testing the handling of collisions with gate and net names
 *
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
TEST_F(hdl_writer_vhdl_test, check_gate_net_name_collision) {
    TEST_START
        {
            // Add a gate to the netlist and store some data
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<net> test_net = nl->create_net( MIN_NET_ID+0, "gate_net_name");
            std::shared_ptr<gate> test_gate = nl->create_gate( MIN_GATE_ID+0, "INV", "gate_net_name");

            test_net->add_dst(test_gate, "I");

            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the gate name was added a "_inst"
            EXPECT_NE(get_net_by_subname(parsed_nl, "gate_net_name"), nullptr);
            //EXPECT_NE(get_gate_by_subname(parsed_nl, "gate_net_name_inst"), nullptr);

        }
    TEST_END
}

/**
 * Testing the translation of net names, that contain only digits
 *
 * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
 * the issue isn't within the parser, but in the writer...
 *
 * Functions: write, parse
 */
 /*
TEST_F(hdl_writer_vhdl_test, check_digit_net_name) {
    TEST_START
        {
            // Add a gate to the netlist and store some data
            std::shared_ptr<netlist> nl = create_empty_netlist(0);

            std::shared_ptr<gate> test_gate = nl->create_gate( MIN_GATE_ID+0, "INV", "gate_net_name");
            std::shared_ptr<net> test_net_0 = nl->create_net( MIN_NET_ID+0, "0");
            std::shared_ptr<net> test_net_1 = nl->create_net( MIN_NET_ID+1, "1");

            test_net_0->add_dst(test_gate, "I");
            test_net_1->set_src(test_gate, "O");

            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_vhdl vhdl_writer(parser_input);

            // Writes the netlist in the sstream
            bool writer_suc = vhdl_writer.write(nl);
            if (!writer_suc) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_vhdl_old vhdl_parser(parser_input);
            // Parse the .vhdl file
            std::shared_ptr<netlist> parsed_nl = vhdl_parser.parse(g_lib_name);

            if (parsed_nl == nullptr) {
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            // Check if the gate name was added a "_inst"
            EXPECT_NE(get_net_by_subname(parsed_nl, "gate_net_name"), nullptr);
            EXPECT_NE(get_gate_by_subname(parsed_nl, "gate_net_name_inst"), nullptr);

        }
    TEST_END
}*/

