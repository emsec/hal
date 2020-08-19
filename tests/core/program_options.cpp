#include "test_def.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <core/program_options.h>
#include <iostream>
#include "netlist_test_utils.h"

namespace hal {
    class ProgramOptionsTest : public ::testing::Test {
    protected:
        const std::string A_REQUIRED_PARAMETER = ProgramOptions::A_REQUIRED_PARAMETER;
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }

        /*
         * Returns if a string contains a certain substring
         */
        bool string_contains_substring(std::string str, std::string sub_str) {
            return (str.find(sub_str) != std::string::npos);
        }
    };

    /**
     * Tests the add of options to the ProgramOptions by passing the flags with description
     * and default parameters, as well as the add by passing other ProgramOptions objects
     *
     * Functions: add (all 3 overloaded functions), is_registered,
     */
    TEST_F(ProgramOptionsTest, check_add_options) {
        TEST_START
            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Add an option with a single flag
                ProgramOptions p_opts("group_0");
                bool suc = p_opts.add("flag", "flag_desc", {"param_0", "param_1"});
                EXPECT_TRUE(suc);
                EXPECT_TRUE(p_opts.is_registered("flag"));
            }
            {
                // Add an option with multiple flag
                ProgramOptions p_opts("group_0");
                bool suc = p_opts.add({"flag_0", "flag_1", "flag_2"}, "flag_desc", {"param_0", "param_1"});
                EXPECT_TRUE(suc);
                EXPECT_TRUE(p_opts.is_registered("flag_0"));
                EXPECT_TRUE(p_opts.is_registered("flag_1"));
                EXPECT_TRUE(p_opts.is_registered("flag_2"));
            }
            {
                // Add options from another set (option flags are disjoint)
                ProgramOptions p_opts_0("group_0");
                ProgramOptions p_opts_1("group_1");
                p_opts_0.add({"flag_0", "flag_1", "flag_2"}, "flag_desc_0", {"param_0", "param_1"});
                p_opts_1.add("flag_3", "flag_desc_1", {"param_0", "param_1"});
                bool suc = p_opts_0.add(p_opts_1, "category");
                EXPECT_TRUE(suc);
                EXPECT_TRUE(p_opts_0.is_registered("flag_0"));
                EXPECT_TRUE(p_opts_0.is_registered("flag_1"));
                EXPECT_TRUE(p_opts_0.is_registered("flag_2"));
                EXPECT_TRUE(p_opts_0.is_registered("flag_3"));
            }
            {
                // Add options with required parameters (required parameters after non-required)
                ProgramOptions p_opts("group_0");
                bool suc_0 = p_opts.add({"flag_0"}, "flag_desc_0", {A_REQUIRED_PARAMETER});
                bool suc_1 = p_opts.add({"flag_1"}, "flag_desc_1", {A_REQUIRED_PARAMETER, A_REQUIRED_PARAMETER});
                bool suc_2 = p_opts.add({"flag_2"}, "flag_desc_2", {A_REQUIRED_PARAMETER, A_REQUIRED_PARAMETER, "param_3"});
                bool suc_3 = p_opts
                    .add({"flag_3"}, "flag_desc_3", {A_REQUIRED_PARAMETER, A_REQUIRED_PARAMETER, "param_3", "param_4", "param_5"});

                EXPECT_TRUE(suc_0);
                EXPECT_TRUE(suc_1);
                EXPECT_TRUE(suc_2);
                EXPECT_TRUE(suc_3);
                EXPECT_TRUE(p_opts.is_registered("flag_0"));
                EXPECT_TRUE(p_opts.is_registered("flag_1"));
                EXPECT_TRUE(p_opts.is_registered("flag_2"));
                EXPECT_TRUE(p_opts.is_registered("flag_3"));
            }

            // ########################
            // NEGATIVE TESTS
            // ########################
            {
                // Add options from another set (option flags are not disjoint)
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts_0("group_0");
                ProgramOptions p_opts_1("group_1");
                p_opts_0.add({"flag_0", "flag_1", "flag_2"}, "flag_desc_0", {"param_0", "param_1"});
                p_opts_1.add({"flag_2", "flag_3", "flag_4"}, "flag_desc_1", {"param_2", "param_3"});
                bool suc = p_opts_0.add(p_opts_1, "category");
                EXPECT_FALSE(suc);
            }
            {
                // Add options from another set with flags which have the same description
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts_0("group_0");
                ProgramOptions p_opts_1("group_1");
                p_opts_0.add({"flag_0"}, "flag_desc_0", {"param_0", "param_1"});
                p_opts_1.add({"flag_1"}, "flag_desc_0", {"param_2", "param_3"});
                bool suc = p_opts_0.add(p_opts_1, "category");
                EXPECT_FALSE(suc);
            }
            {
                // Add two flag-sets with non-disjoint flag-sets (via init-list)
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts;
                p_opts.add({"flag_0", "flag_1", "flag_2"}, "flag_desc_0", {"param_0"});
                bool suc = p_opts.add({"flag_3", "flag_1", "flag_4"}, "flag_desc_1", {"param_1"});
                EXPECT_FALSE(suc);
                EXPECT_EQ(p_opts.get_options().size(), (size_t) 1);
            }
            {
                // Add option with empty flag
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts;
                bool suc = p_opts.add({}, "flag_desc_0", {"param_0", "param_1"});
                EXPECT_FALSE(suc);
                EXPECT_EQ(p_opts.get_options().size(), (size_t) 0);
            }
            {
                // Add an option with the same description
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts;
                p_opts.add({"flag_0"}, "flag_desc_0", {"param_0"});
                bool suc = p_opts.add({"flag_1"}, "flag_desc_0", {"param_1"});
                EXPECT_FALSE(suc);
                EXPECT_EQ(p_opts.get_options().size(), (size_t) 1);
            }
            {
                // Add an option with an empty description
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts;
                bool suc = p_opts.add({"flag_0"}, "", {"param_0"});
                EXPECT_FALSE(suc);
                EXPECT_EQ(p_opts.get_options().size(), (size_t) 0);
            }
            {
                // Add options with required parameters (required parameters after non-required)
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts;
                bool suc_0 = p_opts.add({"flag_0"}, "flag_desc_0", {"param_0", A_REQUIRED_PARAMETER});
                bool suc_1 = p_opts.add({"flag_1"}, "flag_desc_1", {A_REQUIRED_PARAMETER, "param_1", A_REQUIRED_PARAMETER});
                bool suc_2 = p_opts.add({"flag_2"}, "flag_desc_2", {"param_0", A_REQUIRED_PARAMETER, "param_2"});

                EXPECT_FALSE(suc_0);
                EXPECT_FALSE(suc_1);
                EXPECT_FALSE(suc_2);
                EXPECT_FALSE(p_opts.is_registered("flag_0"));
                EXPECT_FALSE(p_opts.is_registered("flag_1"));
                EXPECT_FALSE(p_opts.is_registered("flag_2"));
            }
        TEST_END
    }

    /**
     * Testing the access on options
     *
     * Functions: get_options
     */
    TEST_F(ProgramOptionsTest, check_get_options) {
        TEST_START
            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Get options with multiple options
                ProgramOptions p_opts("group_0");
                p_opts.add({"flag_0_0", "flag_0_1", "flag_0_2"}, "flag_desc_0");
                p_opts.add({"flag_1_0"}, "flag_desc_1");
                p_opts.add({"flag_2_0"}, "flag_desc_2");
                auto options = p_opts.get_options();

                EXPECT_EQ(std::get<0>(options[0]), std::set<std::string>({"flag_0_0", "flag_0_1", "flag_0_2"}));
                EXPECT_EQ(std::get<1>(options[0]), "flag_desc_0");

                EXPECT_EQ(std::get<0>(options[1]), std::set<std::string>({"flag_1_0"}));
                EXPECT_EQ(std::get<1>(options[1]), "flag_desc_1");

                EXPECT_EQ(std::get<0>(options[2]), std::set<std::string>({"flag_2_0"}));
                EXPECT_EQ(std::get<1>(options[2]), "flag_desc_2");

                EXPECT_EQ(options.size(), (size_t) 3);
            }
            // ########################
            // NEGATIVE TESTS
            // ########################
            {
                // Get options if there are no options
                ProgramOptions p_opts("group_0");
                auto options = p_opts.get_options();
                EXPECT_TRUE(options.empty());
            }
        TEST_END
    }

    /**
     * Testing the creation of a formatted options string and if it contains at least
     * the flag names and their description
     *
     * Functions: get_options_string
     */
    TEST_F(ProgramOptionsTest, check_get_options_string) {
        TEST_START
            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Add an option and create the formatted string
                ProgramOptions p_opts;
                p_opts.add({"flag_0", "flag_1"}, "flag_desc", {"param_0"});
                std::string res = p_opts.get_options_string();
                EXPECT_TRUE(string_contains_substring(res, "flag_0"));
                EXPECT_TRUE(string_contains_substring(res, "flag_1"));
                EXPECT_TRUE(string_contains_substring(res, "flag_desc"));
            }
            {
                // Add an option from another option and create the formatted string
                ProgramOptions p_opts("Options_1");
                ProgramOptions p_opts_other("Option_2");
                p_opts.add({"flag_0", "flag_1"}, "flag_desc_1", {"param_0"});
                p_opts.add({"flag_2", "flag_3"}, "flag_desc_2", {"param_1"});
                std::string res = p_opts.get_options_string();
                EXPECT_TRUE(string_contains_substring(res, "flag_0"));
                EXPECT_TRUE(string_contains_substring(res, "flag_1"));
                EXPECT_TRUE(string_contains_substring(res, "flag_2"));
                EXPECT_TRUE(string_contains_substring(res, "flag_3"));
                EXPECT_TRUE(string_contains_substring(res, "flag_desc_1"));
                EXPECT_TRUE(string_contains_substring(res, "flag_desc_2"));
            }
            {
                // Add options from another set and create the formatted string
                ProgramOptions p_opts_0("group_0");
                ProgramOptions p_opts_1("group_1");
                p_opts_0.add({"flag_0", "flag_1", "flag_2"}, "flag_desc_0", {"param_0", "param_1"});
                p_opts_1.add("flag_3", "flag_desc_1", {"param_0", "param_1"});
                p_opts_0.add(p_opts_1, "category");
                std::string res = p_opts_0.get_options_string();
                EXPECT_TRUE(string_contains_substring(res, "flag_0"));
                EXPECT_TRUE(string_contains_substring(res, "flag_1"));
                EXPECT_TRUE(string_contains_substring(res, "flag_2"));
                EXPECT_TRUE(string_contains_substring(res, "flag_3"));
                EXPECT_TRUE(string_contains_substring(res, "flag_desc_0"));
                EXPECT_TRUE(string_contains_substring(res, "flag_desc_1"));
                EXPECT_TRUE(string_contains_substring(res, "category"));
            }
            // ########################
            // NEGATIVE TESTS
            // ########################
            {
                // Get the formatted string, if no options are added yet (should not crash)
                ProgramOptions p_opts;
                p_opts.get_options_string();
            }
        TEST_END
    }

    /**
     * Testing the parsing of command line arguments and the creation of correct ProgramArguments.
     * The behaviour in case of missing parameters with and without the A_REQUIRED_PARAMETER is tested as well
     *
     * Functions: parse, get_unknown_arguments
     */
    TEST_F(ProgramOptionsTest, check_parse) {
        TEST_START
            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Parse with one valid flag with its parameters
                ProgramOptions p_opts;
                p_opts.add({"flag_0", "flag_1", "flag_2"}, "flag_desc_0", {"default_param_0", "default_param_1"});

                const char *args[] = {"program_name", "flag_0", "param_0", "param_1"};
                ProgramArguments p_args = p_opts.parse(4, args);

                EXPECT_EQ(p_args.get_set_options().size(), (size_t) 1);
                EXPECT_EQ(p_args.get_parameters("flag_0").size(), (size_t) 2);
                EXPECT_EQ(p_args.get_parameters("flag_0")[0], "param_0");
                EXPECT_EQ(p_args.get_parameters("flag_2")[0], "param_0");
                EXPECT_EQ(p_args.get_parameters("flag_0")[1], "param_1");
                EXPECT_TRUE(p_opts.get_unknown_arguments().empty());
            }
            {
                // Parse with multiple flags and their arguments (parameters are complete)
                ProgramOptions p_opts;
                p_opts.add({"flag_0_0", "flag_0_1"}, "flag_desc_0", {"default_param_0_0"});
                p_opts.add({"flag_1_0", "flag_1_1"}, "flag_desc_1", {"default_param_1_0", "default_param_1_1"});

                const char *args[] = {"program_name", "flag_0_0", "param_0_0", "flag_1_1", "param_1_0", "param_1_1"};
                ProgramArguments p_args = p_opts.parse(6, args);

                EXPECT_EQ(p_args.get_set_options().size(), (size_t) 2);
                EXPECT_EQ(p_args.get_parameters("flag_0_0")[0], "param_0_0");
                EXPECT_EQ(p_args.get_parameters("flag_1_0")[0], "param_1_0");
                EXPECT_EQ(p_args.get_parameters("flag_1_1")[1], "param_1_1");
                EXPECT_TRUE(p_opts.get_unknown_arguments().empty());
            }
            {
                // Parse multiple flags with missing parameters
                ProgramOptions p_opts;
                p_opts.add({"flag_0"}, "flag_desc_0", {"default_param_0_0"});
                p_opts.add({"flag_1"}, "flag_desc_1", {"default_param_1_0", "default_param_1_1"});
                p_opts.add({"flag_2"}, "flag_desc_2", {"default_param_2_0", "default_param_2_1"});

                const char *args[] = {"program_name", "flag_0", "flag_1", "param_1_0", "flag_2", "param_2_0"};
                ProgramArguments p_args = p_opts.parse(6, args);

                EXPECT_EQ(p_args.get_set_options().size(), (size_t) 3);
                EXPECT_EQ(p_args.get_parameters("flag_1").size(), (size_t) 2);
                EXPECT_EQ(p_args.get_parameters("flag_1")[0], "param_1_0");
                EXPECT_EQ(p_args.get_parameters("flag_1")[1], "default_param_1_1");
                EXPECT_EQ(p_args.get_parameters("flag_2").size(), (size_t) 2);
                EXPECT_EQ(p_args.get_parameters("flag_2")[0], "param_2_0");
                EXPECT_EQ(p_args.get_parameters("flag_2")[1], "default_param_2_1");
                EXPECT_TRUE(p_opts.get_unknown_arguments().empty());
            }
            {
                // Parse multiple flags with required parameters (intended usage)
                ProgramOptions p_opts;
                p_opts.add({"flag_0"}, "flag_desc_0", {A_REQUIRED_PARAMETER});
                p_opts.add({"flag_1"}, "flag_desc_1", {A_REQUIRED_PARAMETER, "default_param_1_1"});
                p_opts.add({"flag_2"}, "flag_desc_2", {A_REQUIRED_PARAMETER, A_REQUIRED_PARAMETER});

                const char *args[] =
                    {"program_name", "flag_0", "param_0_0", "flag_1", "param_1_0", "flag_2", "param_2_0", "param_2_1"};
                ProgramArguments p_args = p_opts.parse(8, args);

                EXPECT_EQ(p_args.get_set_options().size(), (size_t) 3);
                EXPECT_EQ(p_args.get_parameters("flag_0"), std::vector<std::string>({"param_0_0"}));
                EXPECT_EQ(p_args.get_parameters("flag_1"),
                          std::vector<std::string>({"param_1_0", "default_param_1_1"}));
                EXPECT_EQ(p_args.get_parameters("flag_2"), std::vector<std::string>({"param_2_0", "param_2_1"}));
                EXPECT_TRUE(p_opts.get_unknown_arguments().empty());
            }
            {
                // Parse with unknown arguments
                ProgramOptions p_opts;
                p_opts.add({"flag_0"}, "flag_desc_0", {"default_param_0"});

                const char *args[] = {"program_name", "unknown_flag_0", "flag_0", "param_0", "unknown_flag_1"};
                ProgramArguments p_args = p_opts.parse(5, args);

                EXPECT_EQ(p_args.get_parameters("flag_0")[0], "param_0");
                EXPECT_EQ(p_args.get_set_options().size(), (size_t) 1);

                EXPECT_EQ(p_opts.get_unknown_arguments().size(), (size_t) 2);
                EXPECT_EQ(p_opts.get_unknown_arguments()[0], "unknown_flag_0");
                EXPECT_EQ(p_opts.get_unknown_arguments()[1], "unknown_flag_1");
            }
            {
                // Parse with unknown arguments twice

                ProgramOptions p_opts;
                p_opts.add({"flag_0"}, "flag_desc_0", {"default_param_0"});

                const char *args_0[] = {"program_name", "flag_0", "param_0", "unknown_flag_0"};
                const char *args_1[] = {"program_name", "flag_0", "param_0", "unknown_flag_1"};
                ProgramArguments p_args_0 = p_opts.parse(4, args_0);
                ProgramArguments p_args_1 = p_opts.parse(4, args_1);

                EXPECT_EQ(p_args_0.get_set_options().size(), (size_t) 1);

                EXPECT_EQ(p_opts.get_unknown_arguments().size(), (size_t) 1);
                EXPECT_EQ(p_opts.get_unknown_arguments()[0], "unknown_flag_1");
            }
            // ########################
            // NEGATIVE TESTS
            // ########################
            {
                // Get unknown arguments without parse

                ProgramOptions p_opts;

                EXPECT_TRUE(p_opts.get_unknown_arguments().empty());
            }
            {
                // Parse without any flags

                ProgramOptions p_opts;
                const char *args[] = {"program_name"};
                ProgramArguments p_args = p_opts.parse(1, args);

                EXPECT_TRUE(p_args.get_set_options().empty());
                EXPECT_TRUE(p_opts.get_unknown_arguments().empty());
            }
            {
                // Parse multiple flags with required parameters (wrong usage)
                NO_COUT_TEST_BLOCK;
                ProgramOptions p_opts;
                p_opts.add({"flag_0"}, "flag_desc_0", {A_REQUIRED_PARAMETER});
                p_opts.add({"flag_1"}, "flag_desc_1", {"default_param_0"});

                const char *args[] = {"program_name", "flag_0", "flag_1"};

                EXPECT_DEATH(p_opts.parse(3, args), "");
            }
            {
                //        // Parse multiple flags with required parameters (wrong usage)
                //        ProgramOptions p_opts;
                //        p_opts.add({"flag_0"}, "flag_desc_0", {A_REQUIRED_PARAMETER, A_REQUIRED_PARAMETER});
                //        p_opts.add({"flag_1"}, "flag_desc_1", {"default_param_0"});
                //
                //        const char* args[] = {"program_name",
                //                              "flag_0",
                //                              "param_0"
                //                              "flag_1"};
                //
                //        testing::internal::CaptureStdout();
                //        EXPECT_DEATH(p_opts.parse(4, args), "");
                //        testing::internal::GetCapturedStdout();
            }
        TEST_END
    }
} //namespace hal
