
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <test_def.h>

class log_test : public ::testing::Test
{
protected:
    log_manager& lm = log_manager::get_instance();
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

/**
 * Testing creation and deletion as well as access to logger channels.
 *
 * Functions: add_channel, remove_channel, get_channel
 */
TEST_F(log_test, check_channel_functions)
{
    TEST_START
    unsigned int start_channel_count = lm.get_channels().size();

    // ########################
    // POSITIVE TESTS
    // ########################

    //Add a channel "test_channel"
    lm.add_channel("test_channel", {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");

    EXPECT_EQ(lm.get_channels().size(), start_channel_count + 1);
    EXPECT_NE(lm.get_channel("test_channel"), lm.get_channel("null"));

    //Remove an existing channel
    lm.remove_channel("test_channel");
    EXPECT_EQ(lm.get_channels().size(), start_channel_count);

    // ########################
    // NEGATIVE TESTS
    // ########################

    NO_COUT_TEST_BLOCK;
    //Add the same channel twice (should not append the channel)
    lm.add_channel("test_channel", {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");
    EXPECT_EQ(lm.get_channels().size(), start_channel_count + 1);
    lm.add_channel("test_channel", {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");
    EXPECT_EQ(lm.get_channels().size(), start_channel_count + 1);

    //Get an non existing channel
    EXPECT_EQ(lm.get_channel("non_exisiting_channel"), lm.get_channel("null"));

    //Remove a non-existing channel
    lm.remove_channel("this_channel_does_not_exist");
    EXPECT_EQ(log_manager::get_instance().get_channels().size(), start_channel_count + 1);

    //Remove the same channel twice
    lm.remove_channel("test_channel");
    lm.remove_channel("test_channel");
    EXPECT_EQ(lm.get_channels().size(), start_channel_count);

    //Try to get a removed channel
    EXPECT_EQ(lm.get_channel("test_channel"), lm.get_channel("null"));
    TEST_END
}

/**
 * Testing the activation and deactivation of logger channels
 *
 * Functions: activate_channel, deactivate_channel, get_level_of_channel
 */
TEST_F(log_test, check_channel_activation)
{
    TEST_START
    lm.add_channel("test_channel", {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");

    // ########################
    // POSITIVE TESTS
    // ########################

    //Deactivate an activated channel
    lm.deactivate_channel("test_channel");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "off");

    //Activate a deactivated channel
    lm.activate_channel("test_channel");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");

    // ########################
    // NEGATIVE TESTS
    // ########################

    //Activate an activated channel
    lm.activate_channel("test_channel");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");

    //Deactivate a deactivated channel
    lm.deactivate_channel("test_channel");
    lm.deactivate_channel("test_channel");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "off");

    lm.remove_channel("test_channel");
    TEST_END
}

/**
 * Testing the level-assignment of logger channels
 *
 * Functions: get_level_of_channel, set_level_of_channel, add_channel
 */
TEST_F(log_test, check_channel_level)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    //Adds a channel of level "info"
    lm.add_channel("test_channel", {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");

    //Set a channel-level
    lm.set_level_of_channel("test_channel", "debug");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "debug");

    // Try all levels out (trace,off?)
    for (auto& lvl : lm.get_available_log_levels())
    {
        if (lvl == "trace")
            continue;
        lm.set_level_of_channel("test_channel", lvl);
        EXPECT_EQ(lm.get_level_of_channel("test_channel"), lvl);
    }

    // ########################
    // NEGATIVE TESTS
    // ########################

    //Sets and gets level of non-existing channel (should not crash)
    lm.set_level_of_channel("unknown_channel", "info");
    EXPECT_EQ(lm.get_level_of_channel("unknown_channel"), "");

    //Set level to an non-existing level
    lm.set_level_of_channel("test_channel", "info");
    lm.set_level_of_channel("test_channel", "non_existing_level");
    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");

    lm.remove_channel("test_channel");
    TEST_END
}

/**
 * Testing if the get_option_descriptions-function generates valid output
 *
 * Functions: get_level_of_channel, set_level_of_channel, add_channel
 */
TEST_F(log_test, check_program_options)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    //Test if there are options available
    program_options& options = lm.get_option_descriptions();
    EXPECT_FALSE(options.get_options().empty());
    TEST_END
}

/**
 * Testing the correct creation of logging-sinks
 *
 * Functions: create_stdout_sink, create_file_sink
 */
TEST_F(log_test, check_sink)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    //Create a stdout-sink
    std::shared_ptr<log_manager::log_sink> test_stdout_sink = lm.create_stdout_sink();
    EXPECT_NE(test_stdout_sink, nullptr);
    EXPECT_FALSE(test_stdout_sink->is_file_sink);

    //Create a stdout-sink (colored = false)
    std::shared_ptr<log_manager::log_sink> test_stdout_sink_2 = lm.create_stdout_sink(false);
    EXPECT_NE(test_stdout_sink_2, nullptr);
    EXPECT_FALSE(test_stdout_sink->is_file_sink);

    //Create a file-sink
    std::shared_ptr<log_manager::log_sink> test_file_sink = lm.create_file_sink("myPath", false);
    EXPECT_NE(test_file_sink, nullptr);
    EXPECT_EQ(test_file_sink->path, "myPath");
    EXPECT_TRUE(test_file_sink->is_file_sink);
    EXPECT_FALSE(test_file_sink->truncate);

    //Create a file-sink (truncate = true)
    std::shared_ptr<log_manager::log_sink> test_file_sink_2 = lm.create_file_sink("myPathTwo", true);
    EXPECT_NE(test_file_sink_2, nullptr);
    EXPECT_EQ(test_file_sink_2->path, "myPathTwo");
    EXPECT_TRUE(test_file_sink_2->is_file_sink);
    EXPECT_TRUE(test_file_sink_2->truncate);

    //Create a file-sink (empty path)
    std::shared_ptr<log_manager::log_sink> test_file_sink_3 = lm.create_file_sink();
    EXPECT_NE(test_file_sink_3, nullptr);
    EXPECT_FALSE(test_file_sink_3->path.empty());
    EXPECT_TRUE(test_file_sink_3->is_file_sink);
    EXPECT_FALSE(test_file_sink_3->truncate);

    // ########################
    // NEGATIVE TESTS
    // ########################

    //Create the same sink
    std::shared_ptr<log_manager::log_sink> test_file_sink_copy = lm.create_file_sink("myPath", false);
    EXPECT_EQ(test_file_sink, test_file_sink_copy);
    TEST_END
}

/**
 * Testing the options listed by get_option_description, as well as invalid options
 *
 * Functions: get_option_descriptions, handle_options
 */
TEST_F(log_test, check_handle_options)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    lm.add_channel("test_channel", {}, "info");

    // Deactivate a channel via program_arguments
    program_arguments p_arg_deactivate;
    p_arg_deactivate.set_option("--log.test_channel.enabled", std::vector<std::string>({"false"}));
    lm.handle_options(p_arg_deactivate);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "off");

    // Activate a channel via program_arguments
    program_arguments p_arg_activate;
    p_arg_activate.set_option("--log.test_channel.enabled", std::vector<std::string>({"true"}));
    lm.handle_options(p_arg_activate);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");

    // Set a level via program_arguments
    program_arguments p_arg_lvl_debug;
    p_arg_lvl_debug.set_option("--log.test_channel.level", std::vector<std::string>({"debug"}));
    lm.handle_options(p_arg_lvl_debug);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "debug");

    // Deactivate all channels
    lm.add_channel("test_channel_2", {}, "info");
    program_arguments p_arg_deactivate_all;
    p_arg_deactivate_all.set_option("--log.enabled", std::vector<std::string>({"false"}));
    lm.handle_options(p_arg_deactivate_all);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "off");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_2"), "off");

    // Activate all channels
    lm.add_channel("test_channel_2", {}, "info");
    program_arguments p_arg_activate_all;
    p_arg_activate_all.set_option("--log.enabled", std::vector<std::string>({"true"}));
    lm.handle_options(p_arg_activate_all);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_2"), "info");

    // Pass more then one option
    lm.add_channel("test_channel_3", {}, "info");
    lm.add_channel("test_channel_4", {}, "off");
    program_arguments p_arg_multi;
    p_arg_multi.set_option("--log.test_channel.level", std::vector<std::string>({"debug"}));
    p_arg_multi.set_option("--log.test_channel_2.level", std::vector<std::string>({"debug"}));
    p_arg_multi.set_option("--log.test_channel_3.enabled", std::vector<std::string>({"false"}));
    p_arg_multi.set_option("--log.test_channel_4.enabled", std::vector<std::string>({"true"}));

    lm.handle_options(p_arg_multi);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "debug");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_2"), "debug");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_3"), "off");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_4"), "info");

    // Set all levels to option
    program_arguments p_arg_level_for_all;
    p_arg_level_for_all.set_option("--log.level", std::vector<std::string>({"info"}));
    lm.handle_options(p_arg_level_for_all);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_2"), "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_3"), "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_4"), "info");

    // ########################
    // NEGATIVE TESTS
    // ########################
    NO_COUT_TEST_BLOCK;
    // Pass an empty argument (nothing should happen)
    program_arguments p_arg_empty;
    lm.handle_options(p_arg_empty);

    // Pass an invalid argument (nothing should happen)
    program_arguments p_arg_invalid;
    p_arg_invalid.set_option("invalid_flag", std::vector<std::string>({"info"}));
    lm.handle_options(p_arg_invalid);

    // Pass an invalid argument followed by a valid one
    program_arguments p_arg_invalid_valid;
    p_arg_invalid_valid.set_option("--log.invalid_flag", std::vector<std::string>({"info"}));
    p_arg_invalid_valid.set_option("--log.test_channel.enabled", std::vector<std::string>({"false"}));
    lm.handle_options(p_arg_invalid_valid);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "off");

    // Pass an argument with unknown channel name (nothing should happen)
    program_arguments p_arg_unknown_channel;
    p_arg_unknown_channel.set_option("--log.unknown_channel.enabled", std::vector<std::string>({"true"}));
    lm.handle_options(p_arg_unknown_channel);

    // Set an unknown level via arguments
    program_arguments p_arg_unknown_level;
    p_arg_unknown_channel.set_option("--log.test_channel.level", std::vector<std::string>({"unknown_level"}));
    lm.handle_options(p_arg_unknown_channel);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "off");

    // Set an unknown level for all channels
    program_arguments p_arg_unknown_level_all;
    p_arg_unknown_channel.set_option("--log.level", std::vector<std::string>({"unknown_level"}));
    lm.handle_options(p_arg_unknown_channel);

    EXPECT_EQ(lm.get_level_of_channel("test_channel"), "off");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_2"), "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_3"), "info");
    EXPECT_EQ(lm.get_level_of_channel("test_channel_4"), "info");

    lm.remove_channel("test_channel");
    lm.remove_channel("test_channel_2");
    lm.remove_channel("test_channel_3");
    lm.remove_channel("test_channel_4");
    TEST_END
}

/**
* Testing the set_file_option
*
* Functions: get_option_descriptions, handle_options
*/
TEST_F(log_test, check_set_file)
{
    TEST_START
    NO_COUT_TEST_BLOCK;
    lm.set_file_name("newFilePath");
    TEST_END
}