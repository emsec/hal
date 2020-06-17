#include "netlist_test_utils.h"
#include "test_def.h"

#include "gtest/gtest.h"
#include <core/log.h>
#include <core/utils.h>
#include <cstdio>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>

namespace hal
{
    using namespace core_utils;

    class utils_test : public ::testing::Test
    {
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
 * Testing get_bit definition of utils
 *
 * Functions: get_bit
 */
    TEST_F(utils_test, check_get_bit){TEST_START
                                      // ########################
                                      // POSITIVE TESTS
                                      // ########################
                                      {// Get some bits of the given int
                                       int i = 0b1101;
    EXPECT_EQ(get_bit(i, 0), 1);
    EXPECT_EQ(get_bit(i, 1), 0);
    EXPECT_EQ(get_bit(i, 2), 1);
    EXPECT_EQ(get_bit(i, 3), 1);
    EXPECT_EQ(get_bit(i, 4), 0);
}    // namespace hal

TEST_END
}

/**
 * Testing set_bit definition of utils
 *
 * Functions: set_bit
 */
TEST_F(utils_test, check_set_bit){TEST_START
// ########################
// POSITIVE TESTS
// ########################
{
    // Set the bit if the bit is 0
    EXPECT_EQ(set_bit(0b1001, 1), 0b1011);
}
{
    // Set the bit if the bit is 1
    EXPECT_EQ(set_bit(0b1001, 0), 0b1001);
}
TEST_END
}

/**
 * Testing clear_bit definition of utils
 *
 * Functions: clear_bit
 */
TEST_F(utils_test, check_clear_bit){TEST_START
// ########################
// POSITIVE TESTS
// ########################
{
    // Clear the bit if the bit is 1
    EXPECT_EQ(clear_bit(0b1001, 0), 0b1000);
}
{
    // Clear the bit if the bit is 0
    EXPECT_EQ(clear_bit(0b1001, 1), 0b1001);
}
TEST_END
}

/**
 * Testing toggle_bit definition of utils
 *
 * Functions: toggle_bit
 */
TEST_F(utils_test, check_toggle_bit){TEST_START
// ########################
// POSITIVE TESTS
// ########################
{
    // Clear the bit if the bit is 1
    EXPECT_EQ(toggle_bit(0b1001, 0), 0b1000);
}
{
    // Clear the bit if the bit is 0
    EXPECT_EQ(toggle_bit(0b1001, 1), 0b1011);
}
TEST_END
}

/**
 * Testing the ends_with function with different strings
 *
 * Functions: ends_with
 */
TEST_F(utils_test, check_ends_with)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some combinations which should result in true
    EXPECT_TRUE(ends_with<std::string>("string", "ing"));
    EXPECT_TRUE(ends_with<std::string>("string", "g"));
    EXPECT_TRUE(ends_with<std::string>("string", "string"));

    // Some combinations which should result in false
    EXPECT_FALSE(ends_with<std::string>("string", "in"));
    EXPECT_FALSE(ends_with<std::string>("string", "xing"));
    EXPECT_FALSE(ends_with<std::string>("string", "x"));
    EXPECT_FALSE(ends_with<std::string>("string", "ingx"));
    EXPECT_FALSE(ends_with<std::string>("string", "xstring"));

    // Some special cases
    EXPECT_TRUE(ends_with<std::string>("string", ""));
    EXPECT_TRUE(ends_with<std::string>("", ""));
    EXPECT_FALSE(ends_with<std::string>("", "x"));

    TEST_END
}

/**
 * Testing the starts_with function with different strings
 *
 * Functions: starts_with
 */
TEST_F(utils_test, check_starts_with)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some combinations which should result in true
    EXPECT_TRUE(starts_with<std::string>("string", "str"));
    EXPECT_TRUE(starts_with<std::string>("string", "s"));
    EXPECT_TRUE(starts_with<std::string>("string", "string"));

    // Some combinations which should result in false
    EXPECT_FALSE(starts_with<std::string>("string", "tr"));
    EXPECT_FALSE(starts_with<std::string>("string", "strx"));
    EXPECT_FALSE(starts_with<std::string>("string", "x"));
    EXPECT_FALSE(starts_with<std::string>("string", "xstr"));
    EXPECT_FALSE(starts_with<std::string>("string", "stringx"));

    // Some special cases
    EXPECT_TRUE(starts_with<std::string>("string", ""));
    EXPECT_TRUE(starts_with<std::string>("", ""));
    EXPECT_FALSE(starts_with<std::string>("", "x"));
    TEST_END
}

/**
 * Testing the is_integer function with different strings
 *
 * Functions: is_floating_point
 */
TEST_F(utils_test, check_is_integer)
{
    TEST_START

    // ########################
    // POSITIVE TESTS
    // ########################

    // Some strings which should result in true
    EXPECT_TRUE(is_integer<std::string>("12"));
    EXPECT_TRUE(is_integer<std::string>("-42"));
    EXPECT_TRUE(is_integer<std::string>("0"));
    EXPECT_TRUE(is_integer<std::string>("+27"));

    // Some strings which should result in false
    EXPECT_FALSE(is_integer<std::string>("not_a_number"));
    EXPECT_FALSE(is_integer<std::string>("12.3"));
    EXPECT_FALSE(is_integer<std::string>("18e1234"));
    EXPECT_FALSE(is_integer<std::string>(".0"));

    // Some special cases
    EXPECT_FALSE(is_integer<std::string>(""));
    EXPECT_FALSE(is_integer<std::string>("+"));
    EXPECT_FALSE(is_integer<std::string>("-"));

    TEST_END
}

/**
 * Testing the is_floating_point function with different strings
 *
 * Functions: is_floating_point
 */
TEST_F(utils_test, check_is_floating_point)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some strings which should result in true
    EXPECT_TRUE(is_floating_point<std::string>("1.2345"));
    EXPECT_TRUE(is_floating_point<std::string>("-0.005"));
    EXPECT_TRUE(is_floating_point<std::string>(".05"));
    EXPECT_TRUE(is_floating_point<std::string>("123"));
    EXPECT_TRUE(is_floating_point<std::string>("+123.3"));
    EXPECT_TRUE(is_floating_point<std::string>("+3e33"));
    EXPECT_TRUE(is_floating_point<std::string>("+3e-33"));

    // Some strings which should result in false
    EXPECT_FALSE(is_floating_point<std::string>("not_a_number"));
    EXPECT_FALSE(is_floating_point<std::string>("1.234.5"));
    EXPECT_FALSE(is_floating_point<std::string>("?.345"));
    EXPECT_FALSE(is_floating_point<std::string>("3e"));

    // Some special cases
    EXPECT_FALSE(is_floating_point<std::string>(""));
    EXPECT_FALSE(is_floating_point<std::string>("+"));
    TEST_END
}

/**
 * Testing the split function
 *
 * Functions: split
 */
TEST_F(utils_test, check_split)
{
    TEST_START

    // ########################
    // POSITIVE TESTS
    // ########################

    // Some calls without the obey_brackets-flag
    EXPECT_EQ(split<std::string>("string", ',', false), std::vector<std::string>({"string"}));
    EXPECT_EQ(split<std::string>("str,ing", ',', false), std::vector<std::string>({"str", "ing"}));
    EXPECT_EQ(split<std::string>(",str,ing,", ',', false), std::vector<std::string>({"", "str", "ing", ""}));
    EXPECT_EQ(split<std::string>("(st,r,ing)", ',', false), std::vector<std::string>({"(st", "r", "ing)"}));

    // Some calls with the obey_brackets-flag
    EXPECT_EQ(split<std::string>("str,(ing)", ',', true), std::vector<std::string>({"str", "(ing)"}));
    EXPECT_EQ(split<std::string>("s(tr,i)ng", ',', true), std::vector<std::string>({"s(tr,i)ng"}));
    EXPECT_EQ(split<std::string>("s(tr,i),ng", ',', true), std::vector<std::string>({"s(tr,i)", "ng"}));
    EXPECT_EQ(split<std::string>("s(tr(i),(ng))", ',', true), std::vector<std::string>({"s(tr(i),(ng))"}));
    EXPECT_EQ(split<std::string>("(s))tr,(in,g)", ',', true), std::vector<std::string>({"(s))tr", "(in,g)"}));

    // ########################
    // NEGATIVE TESTS
    // ########################

    EXPECT_EQ(split<std::string>("", ',', true), std::vector<std::string>({}));
    EXPECT_EQ(split<std::string>("", ',', false), std::vector<std::string>({}));
    TEST_END
}

/**
 * Testing the trim function
 *
 * Functions: trim
 */
TEST_F(utils_test, check_trim)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some 'normal' cases
    EXPECT_EQ(trim<std::string>("  string  "), "string");
    EXPECT_EQ(trim<std::string>("string  "), "string");
    EXPECT_EQ(trim<std::string>("   string"), "string");
    EXPECT_EQ(trim<std::string>("  stri ng  "), "stri ng");
    EXPECT_EQ(trim<std::string>("\tstring\t"), "string");
    EXPECT_EQ(trim<std::string>("\t  string\t \n"), "string");

    // Test with other char's to remove
    EXPECT_EQ(trim<std::string>("$#$str#ing#$#", "#$"), "str#ing");

    // Some special cases
    EXPECT_EQ(trim<std::string>("      "), "");
    EXPECT_EQ(trim<std::string>(""), "");
    EXPECT_EQ(trim<std::string>("string", ""), "string");

    TEST_END
}

/**
 * Testing the ltrim function
 *
 * Functions: ltrim
 */
TEST_F(utils_test, check_ltrim)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some 'normal' cases
    EXPECT_EQ(ltrim<std::string>("  string  "), "string  ");
    EXPECT_EQ(ltrim<std::string>("string  "), "string  ");
    EXPECT_EQ(ltrim<std::string>("  stri ng  "), "stri ng  ");
    EXPECT_EQ(ltrim<std::string>("\tstring\t"), "string\t");
    EXPECT_EQ(ltrim<std::string>("\t\n\r string"), "string");

    // Test with other char's to remove
    EXPECT_EQ(ltrim<std::string>("$#$string#", "#$"), "string#");

    // Some special cases
    EXPECT_EQ(ltrim<std::string>("      "), "");
    EXPECT_EQ(ltrim<std::string>(""), "");
    EXPECT_EQ(ltrim<std::string>("string", ""), "string");

    TEST_END
}

/**
 * Testing the rtrim function
 *
 * Functions: rtrim
 */
TEST_F(utils_test, check_rtrim)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some 'normal' cases
    EXPECT_EQ(rtrim<std::string>("  string  "), "  string");
    EXPECT_EQ(rtrim<std::string>("string  "), "string");
    EXPECT_EQ(rtrim<std::string>("  stri ng  "), "  stri ng");
    EXPECT_EQ(rtrim<std::string>("\tstring\t"), "\tstring");
    EXPECT_EQ(rtrim<std::string>("string\t\n\r "), "string");

    // Test with other char's to remove
    EXPECT_EQ(rtrim<std::string>("#string$#$", "#$"), "#string");

    // Some special cases
    EXPECT_EQ(rtrim<std::string>("      "), "");
    EXPECT_EQ(rtrim<std::string>(""), "");
    EXPECT_EQ(ltrim<std::string>("string", ""), "string");

    TEST_END
}

/**
 * Testing the replace function
 *
 * Functions: replace
 */
TEST_F(utils_test, check_replace)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    EXPECT_EQ(replace<std::string>("This is a string", "is", "XX"), "ThXX XX a string");
    EXPECT_EQ(replace<std::string>("This is a string", "is", ""), "Th  a string");
    EXPECT_EQ(replace<std::string>("This is a string", "unknown_string", ""), "This is a string");

    // ########################
    // NEGATIVE TESTS
    // ########################

    EXPECT_EQ(replace<std::string>("This is a string", "", "XX"), "This is a string");
    EXPECT_EQ(replace<std::string>("", "XX", "YY"), "");
    TEST_END
}

/**
 * Testing the join function for vectors and sets
 *
 * Functions: join (with vector and set)
 */
TEST_F(utils_test, check_join)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################
    // Join of vectors
    EXPECT_EQ(join(",", std::vector<std::string>({"A", "B", "C"})), "A,B,C");
    EXPECT_EQ(join(",", std::vector<std::string>({"A,", "B,", "C"})), "A,,B,,C");
    EXPECT_EQ(join("", std::vector<std::string>({"A", "B", "C"})), "ABC");
    EXPECT_EQ(join(",", std::vector<std::string>({"A"})), "A");

    // Join of sets
    EXPECT_EQ(join(",", std::set<std::string>({"A", "B", "C"})), "A,B,C");

    // ########################
    // NEGATIVE TESTS
    // ########################

    EXPECT_EQ(join(",", std::vector<std::string>({})), "");
    EXPECT_EQ(join(",", std::set<std::string>({})), "");
    TEST_END
}

/**
 * Testing the to_upper function
 *
 * Functions: to_upper
 */
TEST_F(utils_test, check_to_upper)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some 'normal' cases
    EXPECT_EQ(to_upper<std::string>("string"), "STRING");
    EXPECT_EQ(to_upper<std::string>("StRinG"), "STRING");
    EXPECT_EQ(to_upper<std::string>("StRinG123"), "STRING123");

    // A special case
    EXPECT_EQ(to_upper<std::string>(""), "");
    TEST_END
}

/**
 * Testing the to_lower function
 *
 * Functions: to_lower
 */
TEST_F(utils_test, check_to_lower)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Some 'normal' cases
    EXPECT_EQ(to_lower<std::string>("STRING"), "string");
    EXPECT_EQ(to_lower<std::string>("StRinG"), "string");
    EXPECT_EQ(to_lower<std::string>("StRinG123"), "string123");

    // A special case
    EXPECT_EQ(to_lower<std::string>(""), "");
    TEST_END
}

/**
 * Testing the folder_exists_and_is_accessible function. For the test a new folder is temporary created.
 *
 * Functions: folder_exists_and_is_accessible
 */
TEST_F(utils_test, check_folder_exists_and_is_accessible)
{
    TEST_START
    // Create a temporary directory for testing
    std::filesystem::path tmp_dir_path = get_binary_directory().string() + "/tmp_test";

    fs::create_directory(tmp_dir_path);

    // ########################
    // POSITIVE TESTS
    // ########################

    // The directory exists and is accessible
    EXPECT_TRUE(folder_exists_and_is_accessible(tmp_dir_path));

    // The directory does not exists
    EXPECT_FALSE(folder_exists_and_is_accessible(std::filesystem::path("/this/dir/does/not/exist")));

    TEST_END
    // removes the temporary directories and files
    if (fs::is_directory(get_binary_directory().string() + "/tmp_test"))
    {
        fs::remove_all(get_binary_directory().string() + "/tmp_test");
    }
}

/**
 * Testing functions which return certain project directories. Since the
 * functions should return different paths at different systems, only the
 * availability of the functions is tested. Moreover it is tested, if the
 * directory exists.
 *
 *
 * Functions: get_binary_directory,get_base_directory,get_library_directory,get_share_directory,
 *            get_user_share_directory, get_user_config_directory, get_default_log_directory,
 *            get_gate_library_directories, get_plugin_directories
 */
TEST_F(utils_test, check_to_directory_functions)
{
    TEST_START

    // ########################
    // POSITIVE TESTS
    // ########################

    EXPECT_TRUE(fs::is_directory(get_binary_directory()));
    EXPECT_TRUE(fs::is_directory(get_base_directory()));
    EXPECT_TRUE(fs::is_directory(get_library_directory()));
    EXPECT_TRUE(fs::is_directory(get_share_directory()));
    EXPECT_TRUE(fs::is_directory(get_user_share_directory()));
    EXPECT_TRUE(fs::is_directory(get_user_config_directory()));
    EXPECT_TRUE(fs::is_directory(get_default_log_directory()));

    get_gate_library_directories();
    get_plugin_directories();

    TEST_END
}

/**
 * Testing the functions which returns the first directory which exists
 *
 * Functions: get_first_directory_exists
 */
TEST_F(utils_test, check_get_first_directory_exists)
{
    TEST_START

    std::filesystem::path base_path = get_base_directory();    // This directory should exist
    std::filesystem::path lib_path  = get_base_directory();    // This directory should exist as well
    std::filesystem::path non_existing_path_1(base_path.string() + "/bin/bam/bum");
    std::filesystem::path non_existing_path_2("not_existing_path");

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // One existing dir
        std::vector<std::filesystem::path> path_set({base_path, non_existing_path_1, non_existing_path_2});
        EXPECT_EQ(get_first_directory_exists(path_set), base_path);
    }
    {
        std::vector<std::filesystem::path> path_set({non_existing_path_1, non_existing_path_2, base_path});
        EXPECT_EQ(get_first_directory_exists(path_set), base_path);
    }
    {
        // Two existing dirs (should return the first one)
        std::vector<std::filesystem::path> path_set({lib_path, non_existing_path_2, base_path});
        EXPECT_EQ(get_first_directory_exists(path_set), lib_path);
    }
    {
        // No existing dirs (should return "")
        std::vector<std::filesystem::path> path_set({non_existing_path_1, non_existing_path_2});
        get_first_directory_exists(path_set);
        EXPECT_EQ(get_first_directory_exists(path_set).string(), std::filesystem::path("").string());
    }

    // ########################
    // NEGATIVE TESTS
    // ########################
    {
        // Empty list of paths
        std::vector<std::filesystem::path> path_set_empty({});
        EXPECT_EQ(get_first_directory_exists(path_set_empty).string(), "");
    }

    TEST_END
}

/**
 * Testing the function get_file
 *
 * Functions: get_file
 */
TEST_F(utils_test, check_get_file)
{
    TEST_START

    // Create a temporary directory for testing
    std::filesystem::path tmp_dir_path = get_binary_directory().string() + "/tmp_test";

    fs::create_directory(tmp_dir_path);
    fs::create_directory(tmp_dir_path.string() + "/dir_1");
    fs::create_directory(tmp_dir_path.string() + "/dir_2");
    fs::create_directory(tmp_dir_path.string() + "/dir_empty_1");
    fs::create_directory(tmp_dir_path.string() + "/dir_empty_2");

    std::ofstream outfile_1(get_binary_directory().string() + "/tmp_test/dir_1/tmp.txt");
    std::ofstream outfile_2(get_binary_directory().string() + "/tmp_test/dir_2/tmp.txt");
    outfile_1.close();
    outfile_2.close();

    std::filesystem::path dir_1_path(tmp_dir_path.string() + "/dir_1");
    std::filesystem::path dir_2_path(tmp_dir_path.string() + "/dir_2");
    std::filesystem::path dir_empty_1_path(tmp_dir_path.string() + "/dir_empty_1");
    std::filesystem::path dir_empty_2_path(tmp_dir_path.string() + "/dir_empty_2");

    // ########################
    // POSITIVE TESTS
    // ########################

    {
        // Only one directory contains the file with the given name
        std::vector<std::filesystem::path> dirs_to_search({dir_1_path, dir_empty_1_path, dir_empty_2_path});
        EXPECT_EQ(get_file("tmp.txt", dirs_to_search).string(), dir_1_path.string() + "/tmp.txt");
    }
    {
        // Only one directory contains the file with the given name
        std::vector<std::filesystem::path> dirs_to_search({dir_empty_1_path, dir_empty_2_path, dir_1_path});
        EXPECT_EQ(get_file("tmp.txt", dirs_to_search).string(), dir_1_path.string() + "/tmp.txt");
    }
    {
        // Two directories contain a file with the given name
        std::vector<std::filesystem::path> dirs_to_search({dir_1_path, dir_empty_2_path, dir_2_path});
        EXPECT_EQ(get_file("tmp.txt", dirs_to_search).string(), dir_1_path.string() + "/tmp.txt");
    }
    {
        // No directory contains the searched file -> wrong documentation?
        std::vector<std::filesystem::path> dirs_to_search({dir_empty_1_path, dir_empty_2_path});
        EXPECT_EQ(get_file("tmp.txt", dirs_to_search).string(), std::filesystem::path(""));
    }

    // ########################
    // NEGATIVE TESTS
    // ########################
    {
        // path hints are empty -> wrong documentation?
        std::vector<std::filesystem::path> dirs_to_search({});
        EXPECT_EQ(get_file("tmp.txt", dirs_to_search).string(), std::filesystem::path("").string());
    }
    {
        // file_name is an empty string -> intended behaviour?
        std::vector<std::filesystem::path> dirs_to_search({dir_empty_1_path});
        EXPECT_EQ(get_file("", dirs_to_search).string(), std::filesystem::path("").string());
    }

    TEST_END

    // removes the temporary directories and files
    if (fs::is_directory(get_binary_directory().string() + "/tmp_test"))
    {
        fs::remove_all(get_binary_directory().string() + "/tmp_test");
    }
}

/**
 * Testing the which function. The test expects, that the directory given by
 * get_binary_directory() has at least one executable with the name 'hal'.
 * Moreover a temporary directory
 * with no executables is created.
 *
 * Functions: which
 */
TEST_F(utils_test, check_which)
{
    TEST_START

    // Create a temporary directory for testing
    std::filesystem::path tmp_dir_path = get_binary_directory().string() + "/tmp_test";

    fs::create_directory(tmp_dir_path);
    fs::create_directory(tmp_dir_path.string() + "/dir_txt");
    fs::create_directory(tmp_dir_path.string() + "/dir_empty");

    // Creates a file within a directory, which is not executable
    std::ofstream outfile_1(get_binary_directory().string() + "/tmp_test/dir_txt/tmp.txt");
    outfile_1.close();

    std::filesystem::path dir_txt_path(tmp_dir_path.string() + "/dir_txt");
    std::filesystem::path dir_empty_path(tmp_dir_path.string() + "/dir_empty");

    // ########################
    // POSITIVE TESTS
    // ########################

    // This test are only valid if an executable hal exists within the bin folder
    if (fs::exists(get_binary_directory().string() + "/hal"))
    {
        {
            // Directory exists and contains the given executable
            std::filesystem::path res = which("hal", get_binary_directory().string());
            EXPECT_EQ(res.string(), get_binary_directory().string() + "/hal");
        }
    }
    {
        // Directory exists but contains no files
        std::filesystem::path res = which("hal", tmp_dir_path.string() + "/dir_empty");
        EXPECT_EQ(res.string(), "");
    }
    {
        // Directory exists and contains the given file which is NOT an executable
        std::filesystem::path res = which("tmp.txt", tmp_dir_path.string() + "/dir_txt");
        EXPECT_EQ(res.string(), "");
    }
    {
        // Directory does not exist
        std::filesystem::path res = which("tmp.txt", "/this/directory/does/not/exist");
        EXPECT_EQ(res.string(), "");
    }

    // ########################
    // NEGATIVE TESTS
    // ########################

    {
        // File name is the empty string "" -> FAILS
        std::filesystem::path res = which("", tmp_dir_path.string() + "/dir_empty");
        EXPECT_EQ(res.string(), "");
    }
    {
        // Directory is the empty string ""
        std::filesystem::path res = which("tmp.txt", "");
        EXPECT_EQ(res.string(), "");
    }

    TEST_END

    // removes the temporary directories and files
    if (fs::is_directory(get_binary_directory().string() + "/tmp_test"))
    {
        fs::remove_all(get_binary_directory().string() + "/tmp_test");
    }
}

/**
 * Testing the function get_open_source_licenses. Only testing the access, not the content.
 *
 * Functions: get_open_source_licenses
 */
TEST_F(utils_test, check_get_open_source_license)
{
    TEST_START

    // ########################
    // POSITIVE TESTS
    // ########################

    std::string lic_str = get_open_source_licenses();
    EXPECT_TRUE(lic_str.length() > 0);

    TEST_END
}
}
