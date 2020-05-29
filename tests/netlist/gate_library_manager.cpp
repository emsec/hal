
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <core/utils.h>
#include <fstream>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>


class gate_library_manager_test : public ::testing::Test
{
protected:
    // The path, where the library is temporary stored
    hal::path test_lib_path;
    std::string lib_file_name;
    std::string test_lib_name;

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        lib_file_name = "test_lib";
        test_lib_name = "TEST_GATE_LIBRARY";
        test_lib_path = (core_utils::get_gate_library_directories()[0]) / (lib_file_name + ".lib");
    }

    virtual void TearDown()
    {
        fs::remove(test_lib_path);
    }

    /**
     * Creates a minimal custom gate library used for testing the gate library manager
     */
    void create_test_lib()
    {
        std::ofstream test_lib(test_lib_path.string());
        test_lib << "/* This file only exists for testing purposes and should be already destroyed*/\n"
                    "library (" << test_lib_name <<") {\n"
                    "    define(cell);\n"
                    "    cell(GATE_A) {\n"
                    "        pin(I) {\n"
                    "            direction: input;\n"
                    "        }\n"
                    "        pin(O) {\n"
                    "            direction: output;\n"
                    "            function: \"I\";\n"
                    "        }\n"
                    "    }\n"
                    "    cell(GATE_B) {\n"
                    "        pin(I) {\n"
                    "            direction: input;\n"
                    "        }\n"
                    "        pin(O) {\n"
                    "            direction: output;\n"
                    "            function: \"!I\";\n"
                    "        }\n"
                    "    }\n"
                    "    cell(GND) {\n"
                    "        pin(O) {\n"
                    "            direction: output;\n"
                    "            function: \"0\";\n"
                    "        }\n"
                    "    }\n"
                    "    cell(VCC) {\n"
                    "        pin(O) {\n"
                    "            direction: output;\n"
                    "            function: \"1\";\n"
                    "        }\n"
                    "    }\n"
                    "}";

        test_lib.close();
    }


};

/**
 * Testing the access on a single gate library via the get_gate_library function.
 *
 * Functions: get_gate_library, get_gate_libraries
 */
TEST_F(gate_library_manager_test, check_get_gate_library)
{
    TEST_START
        NO_COUT_TEST_BLOCK;
        create_test_lib();
        // Load the gate library twice by its filename
        std::shared_ptr<gate_library> test_lib_0 = gate_library_manager::get_gate_library(lib_file_name);
        std::shared_ptr<gate_library> test_lib_1 = gate_library_manager::get_gate_library(lib_file_name);
        EXPECT_NE(test_lib_0, nullptr);
        EXPECT_NE(test_lib_1, nullptr);

        // Check that the test library can be found in the get_gate_libraries vector
        bool found_test_lib = false;
        for (std::shared_ptr<gate_library> gl :  gate_library_manager::get_gate_libraries()){
            if(gl->get_name() == test_lib_name){
                found_test_lib = true;
                break;
            }
        }
        EXPECT_TRUE(found_test_lib);
    TEST_END
}

/**
 * Testing the load_all function that loads all gate_libraries found in the directories got by
 * core_utils::get_gate_library_directories().
 *
 * Functions: get_gate_library, get_gate_libraries
 * TODO: Embed test gate library in library manager
 */
TEST_F(gate_library_manager_test, DISABLED_check_load_all)
{
    TEST_START
        // Check that load_all also loads the test gate library
        NO_COUT_TEST_BLOCK;
        create_test_lib();
        gate_library_manager::load_all();

        // Check that the test library can be found in the get_gate_libraries vector
        bool found_test_lib = false;
        for (std::shared_ptr<gate_library> gl :  gate_library_manager::get_gate_libraries()){
            if(gl->get_name() == test_lib_name){
                found_test_lib = true;
                break;
            }
        }
        EXPECT_TRUE(found_test_lib);
    TEST_END

}

/**
 * Testing that a GND/VCC gate type is added to the gate library, if the file does not contain any.
 *
 * Functions: get_gate_library, get_gate_libraries
 */
TEST_F(gate_library_manager_test, check_prepare_library)
{
    TEST_START
        {
            // Parse a file that does not contain a GND or VCC gate type (constant 0 / constant 1)
            NO_COUT_TEST_BLOCK;
            std::ofstream test_lib(test_lib_path.string());
            test_lib << "/* This file only exists for testing purposes and should be already destroyed*/\n"
                        "library (check_prepare_library_1) {\n"
                                                       "    define(cell);\n"
                                                       "}";

            test_lib.close();
            std::shared_ptr<gate_library> empty_lib = gate_library_manager::get_gate_library(lib_file_name);
            ASSERT_NE(empty_lib, nullptr);
            auto g_types = empty_lib->get_gate_types();
            // Check the creation of a gnd gate type
            ASSERT_TRUE(g_types.find("GND") != g_types.end());
            auto gnd_bf = g_types.at("GND")->get_boolean_functions();
            ASSERT_TRUE(gnd_bf.find("O") != gnd_bf.end());
            EXPECT_TRUE(gnd_bf.at("O").is_constant_zero());
            // Check the creation of a vcc gate type
            ASSERT_TRUE(g_types.find("VCC") != g_types.end());
            auto vcc_bf = g_types.at("VCC")->get_boolean_functions();
            ASSERT_TRUE(vcc_bf.find("O") != vcc_bf.end());
            EXPECT_TRUE(vcc_bf.at("O").is_constant_one());

        }
        fs::remove(test_lib_path);
        /*{ // NOTE: Currently not handled like this
            // (very special)
            // Parse a file that does contain gate types with the name "GND"/"VCC", but they are not constant.
            // In this special case, a new GND/VCC gate type is created with
            // the name "GND (auto generated)"/"VCC (auto generated)"
            NO_COUT_TEST_BLOCK;
            std::ofstream test_lib(test_lib_path.string());
            test_lib << "/" << "* This file only exists for testing purposes and should be already destroyed*"<<"/\n"
                        "library (check_prepare_library_2) {\n"
                                                       "    define(cell);\n"
                                                       "    cell(GND) {\n"
                                                       "        pin(I) {\n"
                                                       "            direction: input;\n"
                                                       "        }\n"
                                                       "        pin(O) {\n"
                                                       "            direction: output;\n"
                                                       "            function: \"I\";\n"
                                                       "        }\n"
                                                       "    }\n"
                                                       "    cell(VCC) {\n"
                                                       "        pin(I) {\n"
                                                       "            direction: input;\n"
                                                       "        }\n"
                                                       "        pin(O) {\n"
                                                       "            direction: output;\n"
                                                       "            function: \"!I\";\n"
                                                       "        }\n"
                                                       "    }\n"
                                                       "}";

            test_lib.close();
            std::shared_ptr<gate_library> strange_lib = gate_library_manager::get_gate_library(lib_file_name);
            ASSERT_NE(strange_lib, nullptr);
            auto g_types = strange_lib->get_gate_types();
            // Check the creation of a gnd gate type
            ASSERT_TRUE(g_types.find("GND (auto generated)") != g_types.end());
            auto gnd_bf = g_types.at("GND (auto generated)")->get_boolean_functions();
            ASSERT_TRUE(gnd_bf.find("O") != gnd_bf.end());
            EXPECT_TRUE(gnd_bf.at("O").is_constant_zero());
            // Check the creation of a vcc gate type
            ASSERT_TRUE(g_types.find("VCC (auto generated)") != g_types.end());
            auto vcc_bf = g_types.at("VCC (auto generated)")->get_boolean_functions();
            ASSERT_TRUE(vcc_bf.find("O") != vcc_bf.end());
            EXPECT_TRUE(vcc_bf.at("O").is_constant_one());

        }*/
    TEST_END

}

/**
* Testing the handling of various invalid inputs.
*
* Functions: get_gate_library, get_gate_libraries
*/
TEST_F(gate_library_manager_test, check_invalid)
{
    TEST_START
        {
            // The file path does not exist
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<gate_library> test_lib = gate_library_manager::get_gate_library("/non/existing/path.lib");
            EXPECT_EQ(test_lib, nullptr);
        }
    TEST_END

}
