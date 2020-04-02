
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

//using namespace test_utils;

// NOTE, FIXME: The gate library tests must be updated, due to the changes of the gate library system


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
    NO_COUT_TEST_BLOCK;
    create_test_lib();
    // Load the gate library twice by its filename
    // ISSUE: get_gate_library can take both filename or library name, but if file isn't loaded, the library name can't...
    // ISSUE: ... be used (for reasons). Should this one function be split in two (load_gate_library(file_name) + get_gate_library(lib_name))?
    std::shared_ptr<gate_library> test_lib_0 = gate_library_manager::get_gate_library(lib_file_name);
    //std::shared_ptr<gate_library> test_lib_1 = gate_library_manager::get_gate_library(lib_file_name);
    EXPECT_NE(test_lib_0, nullptr);
    // EXPECT_NE(test_lib_1, nullptr); // <- ISSUE: doesn't work

    // Check that the library can be accessed by get_gate_libraries
    auto g_libs = gate_library_manager::get_gate_libraries();
    ASSERT_TRUE(g_libs.find(test_lib_name) != g_libs.end());
    EXPECT_EQ(g_libs.at(test_lib_name)->get_name(), test_lib_name);
}

/**
 * Testing the load_all function that loads all gate_librariesfound in the directories got by
 * core_utils::get_gate_library_directories().
 *
 * Functions: get_gate_library, get_gate_libraries
 */
TEST_F(gate_library_manager_test, check_load_all)
{
    // Check that load_all also loads the test gate library
    NO_COUT_TEST_BLOCK;
    create_test_lib();
    gate_library_manager::load_all();

    // Check that the test library can be accessed by get_gate_libraries
    auto g_libs = gate_library_manager::get_gate_libraries();
    ASSERT_TRUE(g_libs.find(test_lib_name) != g_libs.end());
    EXPECT_EQ(g_libs.at(test_lib_name)->get_name(), test_lib_name);

}

// IN_PROGRESS: invalid input/negative tests


// OLD TESTS (will be removed later)
#ifdef DONT_BUILD
/**
 * Testing the loading of gate libraries. Since a test library is added at the start of the test
 * to the searching path of the load_all function, this library should be found always. Also
 * try to access the test_library via get_gate_library and get_gate_libraries. Moreover the
 * access on the getters of the created gate library object is tested to check that the library
 * was loaded properly.
 *
 * Functions: load_all, get_gate_libraries, get_gate_library,
 */
TEST_F(gate_library_manager_test, check_load_all)
{
    create_test_lib();
    test_def::capture_stdout();
    gate_library_manager::load_all();

    std::shared_ptr<gate_library> test_lib                    = gate_library_manager::get_gate_library("test_lib");
    std::map<std::string, std::shared_ptr<gate_library>> libs = gate_library_manager::get_gate_libraries();
    test_def::get_captured_stdout();
    EXPECT_NE(test_lib, nullptr);
    EXPECT_FALSE(libs.empty());
    EXPECT_EQ(test_lib, libs["test_lib"]);

    EXPECT_EQ(test_lib->get_name(), "test_lib");
    EXPECT_EQ(*test_lib->get_gate_types(), std::set<std::string>({"TEST_GATE"}));
    EXPECT_EQ(*test_lib->get_vcc_gate_types(), std::set<std::string>({"TEST_GATE"}));
    EXPECT_EQ(*test_lib->get_gnd_gate_types(), std::set<std::string>({"TEST_GATE"}));
    EXPECT_EQ(*test_lib->get_input_pin_types(), std::set<std::string>({"I"}));
    EXPECT_EQ(*test_lib->get_output_pins(), std::set<std::string>({"O"}));
    EXPECT_EQ(*test_lib->get_inout_pin_types(), std::set<std::string>({"IO"}));
    EXPECT_EQ(*test_lib->get_vhdl_includes(), std::vector<std::string>({"test_vhdl_include;"}));

    std::map<std::string, std::vector<std::string>> exp_gate_to_input_types = {{"TEST_GATE", std::vector<std::string>({"I"})}};
    EXPECT_EQ(*test_lib->get_gate_type_map_to_input_pin_types(), exp_gate_to_input_types);

    std::map<std::string, std::vector<std::string>> exp_gate_to_output_types = {{"TEST_GATE", std::vector<std::string>({"O"})}};
    EXPECT_EQ(*test_lib->get_gate_type_map_to_output_pin_types(), exp_gate_to_output_types);

    std::map<std::string, std::vector<std::string>> exp_gate_to_inout_types = {{"TEST_GATE", std::vector<std::string>({"IO"})}};
    EXPECT_EQ(*test_lib->get_gate_type_map_to_inout_pin_types(), exp_gate_to_inout_types);
}

/**
 * Testing the loading of gate_libraries via get_gate_library.
 *
 * Functions: get_gate_library
 */
TEST_F(gate_library_manager_test, check_get_gate_library)
{
    {
        // Load a gate library via get_gate_library
        create_test_lib();
        test_def::capture_stdout();
        std::shared_ptr<gate_library> test_lib = gate_library_manager::get_gate_library("test_lib");
        test_def::get_captured_stdout();

        EXPECT_NE(test_lib, nullptr);
        EXPECT_EQ(test_lib->get_name(), "test_lib");
    }
    {
        // Try to load a non-existing gate library
        NO_COUT_TEST_BLOCK;
        std::shared_ptr<gate_library> lib = gate_library_manager::get_gate_library("non_existing_lib");

        EXPECT_EQ(lib, nullptr);
    }
}

#endif //DONT_BUILD
