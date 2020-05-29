#include "test_def.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <core/program_arguments.h>
#include <iostream>

class program_arguments_test : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

/**
 * Testing the access on the original arguments, which were handed over he constructor
 *
 * Functions: constructor, get_original_arguments
 */
TEST_F(program_arguments_test, check_get_original_arguments){
    TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // Call the constructor with some arguments and get them after
            const char * args[] = {"arg_0", "arg_1"};
            program_arguments p_args(2, args);
            int ret_argc;
            const char** ret_argv;
            p_args.get_original_arguments(&ret_argc, &ret_argv);

            EXPECT_EQ(ret_argc, 2);
            EXPECT_EQ(ret_argv[0], args[0]);
            EXPECT_EQ(ret_argv[1], args[1]);
        }
    TEST_END
}

/**
 * Testing the creation of options with stand-alone flags as well as flags with
 * equivalent descriptions. Moreover the access of the passed parameters is being tested.
 *
 * Functions: set_option, get_parameter, get_parameters, is_option_set
 */
TEST_F(program_arguments_test, check_flags_and_parameters)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // Set a single flag with a set of parameters
        program_arguments p_args;
        std::vector<std::string> flag_params = {"param_0", "param_1"};
        p_args.set_option("flag_0", flag_params);

        EXPECT_TRUE(p_args.is_option_set("flag_0"));
        EXPECT_EQ(p_args.get_parameters("flag_0")[0], "param_0");
        EXPECT_EQ(p_args.get_parameters("flag_0")[1], "param_1");
    }
    {
        // Set multiple flags with a set of parameters (found_flag part of flag-list)
        program_arguments p_args;
        std::set<std::string> flags_with_flag_0 = {"flag_0", "flag_1", "flag_2"};
        std::vector<std::string> flag_params_1  = {"param_0", "param_1"};
        p_args.set_option("flag_0", flags_with_flag_0, flag_params_1);

        EXPECT_TRUE(p_args.is_option_set("flag_0"));
        EXPECT_TRUE(p_args.is_option_set("flag_1"));
        EXPECT_TRUE(p_args.is_option_set("flag_2"));
        EXPECT_EQ(p_args.get_parameters("flag_0")[1], "param_1");
        EXPECT_EQ(p_args.get_parameters("flag_1")[1], "param_1");
        EXPECT_EQ(p_args.get_parameters("flag_2")[1], "param_1");
        EXPECT_EQ(p_args.get_set_options()[0], "flag_0");
    }
    {
        // Set multiple flags with a set of parameters (found_flag not part of flag-list)
        program_arguments p_args;
        std::set<std::string> flags_without_flag_0 = {"flag_1", "flag_2"};
        std::vector<std::string> flag_params       = {"param_0", "param_1"};
        p_args.set_option("flag_0", flags_without_flag_0, flag_params);

        EXPECT_TRUE(p_args.is_option_set("flag_0"));
        EXPECT_TRUE(p_args.is_option_set("flag_1"));
        EXPECT_TRUE(p_args.is_option_set("flag_2"));
        EXPECT_EQ(p_args.get_parameters("flag_0")[1], "param_1");
        EXPECT_EQ(p_args.get_parameters("flag_1")[1], "param_1");
        EXPECT_EQ(p_args.get_parameters("flag_2")[1], "param_1");
        EXPECT_EQ(p_args.get_set_options()[0], "flag_0");
    }
    {
        // Set multiple valid options
        program_arguments p_args;
        std::vector<std::string> flag_params_0 = {"param_0_0"};
        p_args.set_option("flag_0", flag_params_0);
        std::vector<std::string> flag_params_1 = {"param_1_0", "param_1_1"};
        p_args.set_option("flag_1", flag_params_1);
        std::vector<std::string> flag_params_2 = {"param_2_0"};
        p_args.set_option("flag_2", {"flag_2_1", "flag_2_2"}, flag_params_2);

        EXPECT_TRUE(p_args.is_option_set("flag_0"));
        EXPECT_TRUE(p_args.is_option_set("flag_1"));
        EXPECT_TRUE(p_args.is_option_set("flag_2"));
        EXPECT_EQ(p_args.get_parameters("flag_0"), flag_params_0);
        EXPECT_EQ(p_args.get_parameters("flag_1"), flag_params_1);
        EXPECT_EQ(p_args.get_parameters("flag_2"), flag_params_2);
        EXPECT_EQ(p_args.get_parameters("flag_2_1"), flag_params_2);
    }

    // ########################
    // NEGATIVE TESTS
    // ########################

    {
        // Get parameter/-s with unknown flag
        NO_COUT_TEST_BLOCK;
        program_arguments p_args;
        std::vector<std::string> unknown_flag_params = p_args.get_parameters("unknown_flag");
        std::string unknown_flag_param               = p_args.get_parameter("unknown_flag");

        EXPECT_TRUE(unknown_flag_params.empty());
        EXPECT_EQ(unknown_flag_param, "");
    }
    {
        // Overwrite a given flag
        program_arguments p_args;
        std::vector<std::string> flag_params_0 = {"param_0"};
        std::vector<std::string> flag_params_1 = {"param_1"};
        p_args.set_option("flag", flag_params_0);
        p_args.set_option("flag", flag_params_1);

        EXPECT_EQ(p_args.get_parameters("flag"), flag_params_1);
    }
    {
        // Overwrite a given flag via alias
        program_arguments p_args;
        std::vector<std::string> flag_params_0 = {"param_0"};
        std::vector<std::string> flag_params_1 = {"param_1"};
        p_args.set_option("flag", {"alternate_flag"}, flag_params_0);
        p_args.set_option("alternate_flag", flag_params_1);

        EXPECT_TRUE(p_args.is_option_set("flag"));
        EXPECT_TRUE(p_args.is_option_set("alternate_flag"));
        EXPECT_EQ(p_args.get_parameters("flag"), flag_params_1);
    }
    {
        // Set two options with partially overlapping flags
        NO_COUT_TEST_BLOCK;
        program_arguments p_args;
        p_args.set_option("flag_0", {"alternative_flag_0"}, {"param_0"});
        bool suc = p_args.set_option("flag_1", {"alternative_flag_0"}, {"param_1"});

        EXPECT_FALSE(suc);
        EXPECT_TRUE(p_args.is_option_set("flag_0"));
        EXPECT_EQ(p_args.get_parameter("flag_0"), "param_0");
        EXPECT_FALSE(p_args.is_option_set("flag_1"));
    }
    {
        // Set an options with flags of two different options
        NO_COUT_TEST_BLOCK;
        program_arguments p_args;
        p_args.set_option("flag_0", {"alternative_flag_0"}, {"param_0"});
        p_args.set_option("flag_1", {"alternative_flag_1"}, {"param_1"});
        bool suc = p_args.set_option("flag_1", {"alternative_flag_0"}, {"param_2"});

        EXPECT_FALSE(suc);
        EXPECT_TRUE(p_args.is_option_set("flag_0"));
        EXPECT_TRUE(p_args.is_option_set("flag_1"));
        EXPECT_EQ(p_args.get_parameter("flag_0"), "param_0");
        EXPECT_EQ(p_args.get_parameter("flag_1"), "param_1");
    }
    {
        // set an existing option but specify more flags than "last time"
        NO_COUT_TEST_BLOCK;
        program_arguments p_args;
        p_args.set_option("flag", {"alternative_flag_0"}, {});
        bool suc = p_args.set_option("flag", {"alternative_flag_0", "alternative_flag_1"}, {});

        EXPECT_FALSE(suc);
        EXPECT_TRUE(p_args.is_option_set("flag"));
        EXPECT_TRUE(p_args.is_option_set("alternative_flag_0"));
        EXPECT_FALSE(p_args.is_option_set("alternative_flag_1"));
    }
    {
        // Set a flag with empty parameters
        NO_COUT_TEST_BLOCK;
        program_arguments p_args;
        std::vector<std::string> flag_params = {};
        p_args.set_option("flag", flag_params);
        std::string param               = p_args.get_parameter("flag");
        std::vector<std::string> params = p_args.get_parameters("flag");

        EXPECT_EQ(param, "");
        EXPECT_TRUE(params.empty());
    }
    TEST_END
}
