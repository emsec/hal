#include "hal_core/netlist/persistent/netlist_serializer.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace hal {
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;
    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_GROUPING_ID;

    class NetlistSerializerTest : public ::testing::Test {
    protected:
        const GateLibrary* m_gl;

        virtual void SetUp() 
        {
            test_utils::init_log_channels();
            plugin_manager::load_all_plugins();
            test_utils::create_sandbox_directory();

            // gate library needs to be registered through gate_library_manager for serialization
            std::unique_ptr<GateLibrary> gl_tmp = test_utils::create_gate_library(test_utils::create_sandbox_path("testing_gate_library.hgl"));
            gate_library_manager::save(gl_tmp->get_path(), gl_tmp.get(), true);
            m_gl = gate_library_manager::load(gl_tmp->get_path());
        }

        virtual void TearDown() 
        {
            plugin_manager::unload_all_plugins();
            test_utils::remove_sandbox_directory();
        }

        // ===== Example Serializer Netlists =====

        /*
        *
        *                 __________________________________________
        *                | test_mod_0           ____________________|
        *                |                     | test_mod_1         |
        *      gnd (1) =-=-= gate_1_to_1 (3) =-=-=                  | .------= gate_1_to_1 (4) =
        *                |                     |  gate_2_to_1 (0) =-=-+
        *      vcc (2) =-=---------------------=-=                  | '------=
        *                |_____________________|____________________|          gate_2_to_1 (5) =
        *                                                                    =
        *
        *     =                       =                    .-------=
        *       gate_2_to_0 (6)         gate_2_to_1 (7) =--'       ...  gate_2_to_1 (8) =
        *     =                       =                            =
        */
        std::unique_ptr<Netlist> create_example_serializer_netlist() 
        {
            std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);
            nl->set_id(123);
            nl->set_input_filename("esnl_input_filename");
            nl->set_device_name("esnl_device_name");
            nl->set_design_name("design_name");
            nl->get_top_module()->set_type("top_mod_type");

            // Create the gates
            Gate* gate_0 = nl->create_gate(1, m_gl->get_gate_type_by_name("AND2"), "gate_0");
            Gate* gate_1 = nl->create_gate(2, m_gl->get_gate_type_by_name("GND"), "gate_1");
            Gate* gate_2 = nl->create_gate(3, m_gl->get_gate_type_by_name("VCC"), "gate_2");
            Gate* gate_3 = nl->create_gate(4, m_gl->get_gate_type_by_name("BUF"), "gate_3");
            Gate* gate_4 = nl->create_gate(5, m_gl->get_gate_type_by_name("INV"), "gate_4");
            Gate* gate_5 = nl->create_gate(6, m_gl->get_gate_type_by_name("OR2"), "gate_5");
            Gate* gate_6 = nl->create_gate(7, m_gl->get_gate_type_by_name("AND2"), "gate_6");
            Gate* gate_7 = nl->create_gate(8, m_gl->get_gate_type_by_name("OR2"), "gate_7");
            Gate* gate_8 = nl->create_gate(9, m_gl->get_gate_type_by_name("XOR2"), "gate_8");

            // Add the nets (net_x_y1_y2... := Net between the Gate with id x and the gates y1,y2,...)
            Net* net_1_3 = nl->create_net(13, "net_1_3");
            net_1_3->add_source(gate_1, "O");
            net_1_3->add_destination(gate_3, "I");

            Net* net_3_0 = nl->create_net(30, "net_3_0");
            net_3_0->add_source(gate_3, "O");
            net_3_0->add_destination(gate_0, "I0");

            Net* net_2_0 = nl->create_net(20, "net_2_0");
            net_2_0->add_source(gate_2, "O");
            net_2_0->add_destination(gate_0, "I1");

            Net* net_0_4_5 = nl->create_net(45, "net_0_4_5");
            net_0_4_5->add_source(gate_0, "O");
            net_0_4_5->add_destination(gate_4, "I");
            net_0_4_5->add_destination(gate_5, "I0");

            Net* net_7_8 = nl->create_net(78, "net_7_8");
            net_7_8->add_source(gate_7, "O");
            net_7_8->add_destination(gate_8, "I0");

            // Mark some gates as gnd/vcc gates
            gate_1->mark_gnd_gate();
            gate_2->mark_vcc_gate();

            // Mark some nets as global nets
            net_1_3->mark_global_input_net();
            net_3_0->mark_global_output_net();

            // Create the modules
            Module* test_m_0 = nl->create_module(2, "test_mod_0", nl->get_top_module());
            test_m_0->set_type("test_mod_type_0");
            test_m_0->assign_gate(gate_0);
            test_m_0->assign_gate(gate_3);

            Module* test_m_1 = nl->create_module(3, "test_mod_1", test_m_0);
            test_m_1->set_type("test_mod_type_1");
            test_m_1->assign_gate(gate_1);

            // Store some data in a Gate, Net and module
            gate_1->set_data("category_0", "key_0", "data_type", "test_value");
            gate_1->set_data("category_1", "key_1", "data_type", "test_value_1");
            gate_1->set_data("category_1", "key_0", "data_type", "test_value_2");
            net_1_3->set_data("category", "key_2", "data_type", "test_value");
            test_m_0->set_data("category", "key_3", "data_type", "test_value");

            // Set some input/output port names of module 0
            
            ModulePin* pin_1 = test_m_0->get_pin_by_net(net_1_3);
            assert(pin_1 != nullptr);
            assert(test_m_0->set_pin_name(pin_1, "test_m_0_net_1_3_in"));
            ModulePin* pin_2 = test_m_0->get_pin_by_net(net_2_0);
            assert(pin_2 != nullptr);
            assert(test_m_0->set_pin_name(pin_2, "test_m_0_net_2_0_in"));
            assert(test_m_0->create_pin_group("great_group", {pin_1, pin_2}).is_ok());
            ModulePin* pin_3 = test_m_0->get_pin_by_net(net_0_4_5);
            assert(pin_3 != nullptr);
            assert(test_m_0->set_pin_name(pin_3, "test_m_0_net_0_4_5_out"));
            ModulePin* pin_4 = test_m_1->get_pin_by_net(net_1_3);
            assert(pin_4 != nullptr);
            assert(test_m_1->set_pin_name(pin_4, "test_m_1_net_1_3_out"));

            // Store some boolean functions in some gates
            gate_0->add_boolean_function("O_and", BooleanFunction::from_string("I0 & I1").get());
            gate_0->add_boolean_function("O_or", BooleanFunction::from_string("I0 & I1").get());
            gate_4->add_boolean_function("O_not", BooleanFunction::from_string("!I").get());

            // create the groupings
            Grouping* grouping_0 = nl->create_grouping(1, "grouping_0");
            grouping_0->assign_gate(gate_0);
            grouping_0->assign_gate(gate_1);
            grouping_0->assign_gate(gate_2);
            grouping_0->assign_net(net_1_3);
            grouping_0->assign_net(net_2_0);
            grouping_0->assign_module(test_m_0);
            grouping_0->assign_module(test_m_1);

            Grouping* grouping_1 = nl->create_grouping(2, "grouping_1");
            grouping_1->assign_gate(gate_3);
            grouping_1->assign_gate(gate_4);
            grouping_1->assign_gate(gate_5);
            grouping_1->assign_net(net_7_8);

            return nl;
        }

        // Write a minimal old-format .hal JSON to path.  gate_data_entries and
        // mod_data_entries are raw JSON arrays for the "data" key (pass "" to omit).
        // gate_param_entries is a raw JSON object for the "parameters" key (pass "" to omit).
        void write_old_format_hal(const std::filesystem::path& path,
                                   const std::string& gate_data_entries,
                                   const std::string& gate_param_entries,
                                   const std::string& mod_data_entries) const
        {
            std::string gate_data_section  = gate_data_entries.empty()  ? "" : ",\"data\":"       + gate_data_entries;
            std::string gate_param_section = gate_param_entries.empty() ? "" : ",\"parameters\":" + gate_param_entries;
            std::string mod_data_section   = mod_data_entries.empty()   ? "" : ",\"data\":"       + mod_data_entries;

            std::ostringstream ss;
            ss << R"({"serialization_format_version":14,"netlist":{"gate_library":")"
               << m_gl->get_path().string()
               << R"(","id":1,"input_file":"","design_name":"","device_name":"",)"
               << R"("gates":[{"id":1,"name":"test_gate","type":"PARAM_TEST")"
               << gate_data_section << gate_param_section
               << R"(}],"global_vcc":[],"global_gnd":[],)"
               << R"("nets":[],"global_in":[],"global_out":[],)"
               << R"("modules":[{"id":1,"name":"top","parent":0,"type":"","gates":[1])"
               << mod_data_section
               << R"(}]}})";

            std::ofstream f(path.string());
            f << ss.str();
        }
    };

    // /**
    //  * Testing the serialization and a followed deserialization of the example
    //  * netlist.
    //  *
    //  * Functions: serialize_netlist, deserialize_netlist
    //  */
     TEST_F(NetlistSerializerTest, check_serialize_and_deserialize) {
         TEST_START
             {// Serialize and deserialize the example netlist (with some additions) and compare the result with the original netlist
                 auto nl = create_example_serializer_netlist();



                 // Serialize and deserialize the netlist now
                 std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                 ASSERT_TRUE(netlist_serializer::serialize_to_file(nl.get(), test_hal_file_path));
                 auto des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);
                 ASSERT_NE(des_nl, nullptr);
                 EXPECT_TRUE(*nl == *des_nl);
             }
             {
                 // Test the example netlist against its deserialized version, but flip the module ids
                 auto nl = create_example_serializer_netlist();
                 // -- Remove the modules
                 nl->delete_module(nl->get_module_by_id(2));
                 nl->delete_module(nl->get_module_by_id(3));
                 // -- Add them again with flipped ids
                 Module *test_m_0_flipped = nl->create_module(3, "test_mod_0_flipped", nl->get_top_module());
                 test_m_0_flipped->set_type("test_mod_type_0_flipped");
                 test_m_0_flipped->assign_gate(nl->get_gate_by_id(1));
                 test_m_0_flipped->assign_gate(nl->get_gate_by_id(4));

                 Module *test_m_1_flipped = nl->create_module(2, "test_mod_1_flipped", test_m_0_flipped);
                 test_m_1_flipped->set_type("test_mod_type_1_flipped");
                 test_m_1_flipped->assign_gate(nl->get_gate_by_id(2));

                 test_m_0_flipped->set_data("category", "key_3", "data_type", "test_value");

                 // Set some input/output port names of module 0
                 ModulePin* pin_1 = test_m_0_flipped->get_pin_by_net((nl->get_net_by_id(13)));
                 ASSERT_NE(pin_1, nullptr);
                 ASSERT_TRUE(test_m_0_flipped->set_pin_name(pin_1, "test_m_0_flipped_net_1_3_in"));
                 ModulePin* pin_2 = test_m_0_flipped->get_pin_by_net((nl->get_net_by_id(20)));
                 ASSERT_NE(pin_2, nullptr);
                 ASSERT_TRUE(test_m_0_flipped->set_pin_name(pin_2, "test_m_0_flipped_net_2_0_in"));
                 ModulePin* pin_3 = test_m_0_flipped->get_pin_by_net((nl->get_net_by_id(45)));
                 ASSERT_NE(pin_3, nullptr);
                 ASSERT_TRUE(test_m_0_flipped->set_pin_name(pin_3, "test_m_0_flipped_net_0_4_5_out"));
                 ModulePin* pin_4 = test_m_1_flipped->get_pin_by_net((nl->get_net_by_id(13)));
                 ASSERT_NE(pin_4, nullptr);
                 ASSERT_TRUE(test_m_1_flipped->set_pin_name(pin_4, "test_m_1_flipped_net_1_3_out"));

                 // Serialize and deserialize the netlist now
                 std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                 ASSERT_TRUE(netlist_serializer::serialize_to_file(nl.get(), test_hal_file_path));
                 auto des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);
                 ASSERT_NE(des_nl, nullptr);
                 EXPECT_TRUE(*nl == *des_nl);
             }
             {
                 // Serialize and deserialize an empty netlist and compare the result with the original netlist
                 auto nl = std::make_unique<Netlist>(m_gl);

                 std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                 ASSERT_TRUE(netlist_serializer::serialize_to_file(nl.get(), test_hal_file_path));
                 auto des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);
                 ASSERT_NE(des_nl, nullptr);
                 EXPECT_TRUE(*nl == *des_nl);
             }
             {
                 // Round-trip typed parameter values on Gate, Module, and Net through the .hal serializer.
                 auto nl = std::make_unique<Netlist>(m_gl);
                 GateType* gt = m_gl->get_gate_type_by_name("PARAM_TEST");
                 ASSERT_NE(gt, nullptr);

                 // Gate: store values against the gate-type's declarations.
                 Gate* g = nl->create_gate(gt, "instance_with_params");
                 ASSERT_NE(g, nullptr);
                 const Parameter gate_width = gt->get_parameter("width").get();
                 const Parameter gate_mode  = gt->get_parameter("mode").get();
                 ASSERT_TRUE(g->set_parameter(gate_width, "0xBEEF").is_ok());
                 ASSERT_TRUE(g->set_parameter(gate_mode, "inverted").is_ok());

                 // Module: arbitrary declarations are accepted; one parameter of each type
                 // is stored so the full Parameter::Type matrix exercises the JSON round-trip.
                 Module* mod = nl->create_module("parametric_mod", nl->get_top_module());
                 ASSERT_NE(mod, nullptr);
                 const Parameter mod_bool   = Parameter::Boolean("ENABLE", "false").get();
                 const Parameter mod_width  = Parameter::BitVector("WIDTH", 32, "").get();
                 const Parameter mod_logic  = Parameter::LogicVector("STATE", 4, "0b0").get();
                 const Parameter mod_count  = Parameter::Integer("COUNT", "0").get();
                 const Parameter mod_label  = Parameter::String("LABEL", "").get();
                 const Parameter mod_pi     = Parameter::Float("PI", "0").get();
                 const Parameter mod_delay  = Parameter::Time("DELAY", "0ns").get();
                 const Parameter mod_flavor = Parameter::Enum("FLAVOR", {"normal", "fast"}, "normal").get();
                 ASSERT_TRUE(mod->set_parameter(mod_bool, "true").is_ok());
                 ASSERT_TRUE(mod->set_parameter(mod_width, "0x20").is_ok());
                 ASSERT_TRUE(mod->set_parameter(mod_logic, "0b10XZ").is_ok());
                 ASSERT_TRUE(mod->set_parameter(mod_count, "-7").is_ok());
                 ASSERT_TRUE(mod->set_parameter(mod_label, "hello world").is_ok());
                 ASSERT_TRUE(mod->set_parameter(mod_pi, "3.14").is_ok());
                 ASSERT_TRUE(mod->set_parameter(mod_delay, "250ns").is_ok());
                 ASSERT_TRUE(mod->set_parameter(mod_flavor, "fast").is_ok());

                 // Net: parameters work identically on nets.
                 Net* net = nl->create_net("metadata_net");
                 ASSERT_NE(net, nullptr);
                 const Parameter net_delay = Parameter::BitVector("delay_ps", 32, "").get();
                 ASSERT_TRUE(net->set_parameter(net_delay, "0xFA").is_ok());

                 std::filesystem::path path = test_utils::create_sandbox_path("test_param_roundtrip.hal");
                 ASSERT_TRUE(netlist_serializer::serialize_to_file(nl.get(), path));
                 auto des_nl = netlist_serializer::deserialize_from_file(path);
                 ASSERT_NE(des_nl, nullptr);

                 // Gate values and declarations come back intact, and encode_as_int still works.
                 Gate* des_g = nullptr;
                 for (Gate* candidate : des_nl->get_gates())
                 {
                     if (candidate->get_name() == "instance_with_params")
                     {
                         des_g = candidate;
                         break;
                     }
                 }
                 ASSERT_NE(des_g, nullptr);
                 EXPECT_EQ(des_g->get_parameter_value("width").get(), "0xBEEF");
                 EXPECT_EQ(des_g->get_parameter_value("mode").get(), "inverted");
                 EXPECT_EQ(des_g->get_parameter_declaration("width").get(), gate_width);
                 EXPECT_EQ(des_g->get_parameter_declaration("mode").get(), gate_mode);
                 EXPECT_EQ(des_g->get_parameter_declaration("width").get().encode_as_int("0xBEEF").get(), 0xBEEFu);
                 EXPECT_EQ(des_g->get_parameter_declaration("mode").get().encode_as_int("inverted").get(), 1u);

                 // Module declarations and values are preserved end-to-end.
                 Module* des_mod = nullptr;
                 for (Module* candidate : des_nl->get_modules())
                 {
                     if (candidate->get_name() == "parametric_mod")
                     {
                         des_mod = candidate;
                         break;
                     }
                 }
                 ASSERT_NE(des_mod, nullptr);
                 EXPECT_EQ(des_mod->get_parameter_value("ENABLE").get(), "true");
                 EXPECT_EQ(des_mod->get_parameter_declaration("ENABLE").get(), mod_bool);
                 EXPECT_EQ(des_mod->get_parameter_value("WIDTH").get(), "0x20");
                 EXPECT_EQ(des_mod->get_parameter_declaration("WIDTH").get(), mod_width);
                 EXPECT_EQ(des_mod->get_parameter_value("STATE").get(), "0b10XZ");
                 EXPECT_EQ(des_mod->get_parameter_declaration("STATE").get(), mod_logic);
                 EXPECT_EQ(des_mod->get_parameter_value("COUNT").get(), "-7");
                 EXPECT_EQ(des_mod->get_parameter_declaration("COUNT").get(), mod_count);
                 EXPECT_EQ(des_mod->get_parameter_value("LABEL").get(), "hello world");
                 EXPECT_EQ(des_mod->get_parameter_declaration("LABEL").get(), mod_label);
                 EXPECT_EQ(des_mod->get_parameter_value("PI").get(), "3.14");
                 EXPECT_EQ(des_mod->get_parameter_declaration("PI").get(), mod_pi);
                 EXPECT_EQ(des_mod->get_parameter_value("DELAY").get(), "250ns");
                 EXPECT_EQ(des_mod->get_parameter_declaration("DELAY").get(), mod_delay);
                 EXPECT_EQ(des_mod->get_parameter_value("FLAVOR").get(), "fast");
                 EXPECT_EQ(des_mod->get_parameter_declaration("FLAVOR").get(), mod_flavor);

                 // Net declarations and values are preserved end-to-end.
                 Net* des_net = nullptr;
                 for (Net* candidate : des_nl->get_nets())
                 {
                     if (candidate->get_name() == "metadata_net")
                     {
                         des_net = candidate;
                         break;
                     }
                 }
                 ASSERT_NE(des_net, nullptr);
                 EXPECT_EQ(des_net->get_parameter_value("delay_ps").get(), "0xFA");
                 EXPECT_EQ(des_net->get_parameter_declaration("delay_ps").get(), net_delay);
             }


         TEST_END
     }

     TEST_F(NetlistSerializerTest, check_generic_data_migration)
     {
         TEST_START
         {
             // Gate migration: gate type declarations take precedence.
             // Old-format "bit_vector" value "CAFE" (no prefix) → "0xCAFE", BitVector(16) from gate type.
             // Old-format "string" value for an Enum field → Enum declaration from gate type.
             // Non-"generic" data entries are left untouched.
             auto path = test_utils::create_sandbox_path("migration_gate.hal");
             write_old_format_hal(path,
                 R"([["generic","width","bit_vector","CAFE"],)"
                 R"(["generic","mode","string","inverted"],)"
                 R"(["attribute","info","string","keep_me"]])",
                 "", "");

             NO_COUT_TEST_BLOCK;
             auto nl = netlist_serializer::deserialize_from_file(path);
             ASSERT_NE(nl, nullptr);
             Gate* g = nl->get_gate_by_id(1);
             ASSERT_NE(g, nullptr);

             // "width": gate-type's BitVector(16) declaration used, unprefixed hex prepended
             EXPECT_TRUE(g->has_parameter("width"));
             EXPECT_EQ(g->get_parameter_value("width").get(), "0xCAFE");
             EXPECT_EQ(g->get_parameter_declaration("width").get(),
                       m_gl->get_gate_type_by_name("PARAM_TEST")->get_parameter("width").get());

             // "mode": gate-type's Enum declaration used, value validated as enum member
             EXPECT_TRUE(g->has_parameter("mode"));
             EXPECT_EQ(g->get_parameter_value("mode").get(), "inverted");
             EXPECT_EQ(g->get_parameter_declaration("mode").get(),
                       m_gl->get_gate_type_by_name("PARAM_TEST")->get_parameter("mode").get());

             // "generic" data entries are deleted after successful migration
             EXPECT_FALSE(g->has_data("generic", "width"));
             EXPECT_FALSE(g->has_data("generic", "mode"));

             // non-"generic" category data is not touched
             EXPECT_FALSE(g->has_parameter("info"));
             EXPECT_TRUE(g->has_data("attribute", "info"));
         }
         {
             // Module migration: inference from data-type string, covering all supported types.
             auto path = test_utils::create_sandbox_path("migration_module_types.hal");
             write_old_format_hal(path, "", "",
                 R"([["generic","B",  "boolean",          "true" ],)"
                 R"( ["generic","I",  "integer",          "-3"   ],)"
                 R"( ["generic","F",  "floating_point",   "2.5"  ],)"
                 R"( ["generic","T",  "time",             "10ns" ],)"
                 R"( ["generic","S",  "string",           "hello"],)"
                 R"( ["generic","V0", "bit_value",        "0"    ],)"
                 R"( ["generic","V1", "bit_value",        "1"    ],)"
                 R"( ["generic","VX", "bit_value",        "X"    ],)"
                 R"( ["generic","VZ", "std_logic",        "Z"    ],)"
                 R"( ["generic","BV", "bit_vector",       "ABCD" ],)"
                 R"( ["generic","BP", "bit_vector",       "0x12" ],)"
                 R"( ["generic","LV", "std_logic_vector", "0bXX01"],)"
                 R"( ["other",  "O",  "string",           "skip" ]])");

             NO_COUT_TEST_BLOCK;
             auto nl = netlist_serializer::deserialize_from_file(path);
             ASSERT_NE(nl, nullptr);
             Module* top = nl->get_top_module();
             ASSERT_NE(top, nullptr);

             EXPECT_EQ(top->get_parameter_declaration("B").get().get_type(),  Parameter::Type::Boolean);
             EXPECT_EQ(top->get_parameter_value("B").get(), "true");

             EXPECT_EQ(top->get_parameter_declaration("I").get().get_type(),  Parameter::Type::Integer);
             EXPECT_EQ(top->get_parameter_value("I").get(), "-3");

             EXPECT_EQ(top->get_parameter_declaration("F").get().get_type(),  Parameter::Type::Float);
             EXPECT_EQ(top->get_parameter_value("F").get(), "2.5");

             EXPECT_EQ(top->get_parameter_declaration("T").get().get_type(),  Parameter::Type::Time);
             EXPECT_EQ(top->get_parameter_value("T").get(), "10ns");

             EXPECT_EQ(top->get_parameter_declaration("S").get().get_type(),  Parameter::Type::String);
             EXPECT_EQ(top->get_parameter_value("S").get(), "hello");

             // "bit_value" '0'/'1' → BitVector(1) with "0b" prefix
             EXPECT_EQ(top->get_parameter_declaration("V0").get().get_type(), Parameter::Type::BitVector);
             EXPECT_EQ(top->get_parameter_declaration("V0").get().get_size(), 1u);
             EXPECT_EQ(top->get_parameter_value("V0").get(), "0b0");

             EXPECT_EQ(top->get_parameter_declaration("V1").get().get_type(), Parameter::Type::BitVector);
             EXPECT_EQ(top->get_parameter_value("V1").get(), "0b1");

             // "bit_value" state char → LogicVector(1)
             EXPECT_EQ(top->get_parameter_declaration("VX").get().get_type(), Parameter::Type::LogicVector);
             EXPECT_EQ(top->get_parameter_declaration("VX").get().get_size(), 1u);
             EXPECT_EQ(top->get_parameter_value("VX").get(), "0bX");

             // "std_logic" → LogicVector(1)
             EXPECT_EQ(top->get_parameter_declaration("VZ").get().get_type(), Parameter::Type::LogicVector);
             EXPECT_EQ(top->get_parameter_value("VZ").get(), "0bZ");

             // "bit_vector" unprefixed hex "ABCD" → BitVector(16), value "0xABCD"
             EXPECT_EQ(top->get_parameter_declaration("BV").get().get_type(), Parameter::Type::BitVector);
             EXPECT_EQ(top->get_parameter_declaration("BV").get().get_size(), 16u);
             EXPECT_EQ(top->get_parameter_value("BV").get(), "0xABCD");

             // "bit_vector" already-prefixed "0x12" → BitVector(8), value "0x12"
             EXPECT_EQ(top->get_parameter_declaration("BP").get().get_type(), Parameter::Type::BitVector);
             EXPECT_EQ(top->get_parameter_declaration("BP").get().get_size(), 8u);
             EXPECT_EQ(top->get_parameter_value("BP").get(), "0x12");

             // "std_logic_vector" prefixed "0bXX01" → LogicVector(4), value "0bXX01"
             EXPECT_EQ(top->get_parameter_declaration("LV").get().get_type(), Parameter::Type::LogicVector);
             EXPECT_EQ(top->get_parameter_declaration("LV").get().get_size(), 4u);
             EXPECT_EQ(top->get_parameter_value("LV").get(), "0bXX01");

             // non-"generic" category entry is untouched
             EXPECT_FALSE(top->has_parameter("O"));
             EXPECT_TRUE(top->has_data("other", "O"));

             // migrated entries removed from data map
             for (const auto& key : {"B", "I", "F", "T", "S", "V0", "V1", "VX", "VZ", "BV", "BP", "LV"})
                 EXPECT_FALSE(top->has_data("generic", key));
         }
         {
             // When a "parameters" section is present, migration is skipped.
             // The "generic" data entry stays in the data map.
             auto path = test_utils::create_sandbox_path("migration_skip.hal");
             write_old_format_hal(path,
                 R"([["generic","width","bit_vector","DEAD"]])",
                 R"({"width":{"type":"bit_vector","size":16,"default":"0xCAFE","value":"0xBEEF"}})",
                 "");

             NO_COUT_TEST_BLOCK;
             auto nl = netlist_serializer::deserialize_from_file(path);
             ASSERT_NE(nl, nullptr);
             Gate* g = nl->get_gate_by_id(1);
             ASSERT_NE(g, nullptr);

             // Explicit "parameters" value is loaded
             EXPECT_EQ(g->get_parameter_value("width").get(), "0xBEEF");
             // Old "generic" data entry is left in the data map (migration was skipped)
             EXPECT_TRUE(g->has_data("generic", "width"));
         }
         TEST_END
     }

     /**
      * Testing the serialization and deserialization of a netlist with invalid input
      *
      * Functions: serialize_netlist, deserialize_netlist
      */
     TEST_F(NetlistSerializerTest, check_serialize_and_deserialize_negative) {
         TEST_START
             {
                 // Serialize a netlist which is a nullptr
                 std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                 EXPECT_FALSE(netlist_serializer::serialize_to_file(nullptr, test_hal_file_path));
             }
             {
                 // Serialize a netlist to an invalid path
                 NO_COUT_TEST_BLOCK;
                 auto nl = create_example_serializer_netlist();
                 EXPECT_FALSE(netlist_serializer::serialize_to_file(nl.get(), std::filesystem::path("")));
             }
             {
                 // Deserialize a netlist from a non existing path
                 NO_COUT_TEST_BLOCK;
                 EXPECT_EQ(netlist_serializer::deserialize_from_file(std::filesystem::path("/using/this/file/is/let.hal")), nullptr);
             }
             {
                 // Deserialize invalid input
                 NO_COUT_TEST_BLOCK;
                 std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                 std::ofstream myfile;
                 myfile.open(test_hal_file_path.string());
                 myfile << "I h4ve no JSON f0rmat!!!\n(Temporary file for testing. Should be already deleted...)";
                 myfile.close();
                 EXPECT_EQ(netlist_serializer::deserialize_from_file(test_hal_file_path), nullptr);
             }
         TEST_END
     }
}    //namespace hal
