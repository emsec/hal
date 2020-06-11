#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/data_container.h>

using namespace test_utils;

/*
 * A child of data_container (used for testing)
 */
class test_data_container : public data_container
{
public:
    test_data_container()  = default;
    ~test_data_container() = default;

    /**
     * Returns true whenever the notify_updated() was called since the last
     * call of data_update_notified
     */
    bool data_update_notified()
    {
        if (notified)
        {
            notified = false;
            return true;
        }
        return false;
    }

private:
    bool notified = false;
    void notify_updated()
    {
        notified = true;
    }
};

class data_container_test : public ::testing::Test
{
protected:
    const std::tuple<std::string, std::string> empty_pair = std::make_tuple("", "");

    virtual void SetUp()
    {
        test_utils::init_log_channels();
    }

    virtual void TearDown()
    {
    }
};

/**
 * Testing the set_data function. To verify success, the get_data_by_key function
 * is used.
 *
 * Functions: set_data, get_data_by_key
 */
TEST_F(data_container_test, check_set_data){
    TEST_START
        {
            // Set multiple data with different keys and categories
            test_data_container d_cont;
            EXPECT_TRUE(d_cont.set_data("category_0", "key_0", "data_type_0", "value_0", false));
            EXPECT_TRUE(d_cont.set_data("category_0", "key_1", "data_type_1", "value_1", false));
            EXPECT_TRUE(d_cont.set_data("category_1", "key_2", "data_type_2", "value_2", false));
            EXPECT_TRUE(d_cont.set_data("category_1", "key_0", "data_type_3", "value_3", false));

            //EXPECT_TRUE(d_cont.data_update_notified());
            EXPECT_EQ(d_cont.get_data_by_key("category_0", "key_0"), std::make_tuple("data_type_0", "value_0"));
            EXPECT_EQ(d_cont.get_data_by_key("category_0", "key_1"), std::make_tuple("data_type_1", "value_1"));
            EXPECT_EQ(d_cont.get_data_by_key("category_1", "key_2"), std::make_tuple("data_type_2", "value_2"));
            EXPECT_EQ(d_cont.get_data_by_key("category_1", "key_0"), std::make_tuple("data_type_3", "value_3"));
        }
        {
            // Overwrites data with the same key and category
            test_data_container d_cont;
            EXPECT_TRUE(d_cont.set_data("category", "key", "data_type", "value", false));
            //EXPECT_TRUE(d_cont.data_update_notified());
            EXPECT_EQ(d_cont.get_data_by_key("category", "key"), std::make_tuple("data_type", "value"));

            EXPECT_TRUE(d_cont.set_data("category", "key", "new_data_type", "new_value", false));
            //EXPECT_TRUE(d_cont.data_update_notified());
            EXPECT_EQ(d_cont.get_data_by_key("category", "key"), std::make_tuple("new_data_type", "new_value"));
        }
        {
            // Log with info level = true
            ::testing::internal::CaptureStdout();
            test_data_container d_cont;
            EXPECT_TRUE(d_cont.set_data("category", "key", "data_type", "value", true));
            EXPECT_NE(::testing::internal::GetCapturedStdout(), "");
        }

        // Negative
        {
            // Leave category empty
            NO_COUT_TEST_BLOCK;
            test_data_container d_cont;
            EXPECT_FALSE(d_cont.set_data("", "key", "data_type", "value"));
            //EXPECT_FALSE(d_cont.data_update_notified());
            EXPECT_EQ(d_cont.get_data_by_key("", "key"), empty_pair);
        }
        {
            // Leave key empty
            NO_COUT_TEST_BLOCK;
            test_data_container d_cont;
            EXPECT_FALSE(d_cont.set_data("category", "", "data_type", "value"));
            //EXPECT_FALSE(d_cont.data_update_notified());
            EXPECT_EQ(d_cont.get_data_by_key("category", ""), empty_pair);
        }


    TEST_END
}

/**
 * Testing the delete_data function. To verify success, the get_data_by_key function
 * is used.
 *
 * Functions: delete_data, get_data_by_key
 */
TEST_F(data_container_test, check_delete_data){
    TEST_START
        {
            // Delete an existing entry
            test_data_container d_cont;
            d_cont.set_data("category", "key", "data_type", "value", false);    // create an entry
            //EXPECT_TRUE(d_cont.data_update_notified());

            // delete the created entry
            EXPECT_TRUE(d_cont.delete_data("category", "key"));
            //EXPECT_TRUE(d_cont.data_update_notified());
            EXPECT_EQ(d_cont.get_data_by_key("category", "key"), empty_pair);
        }
        {
            // Delete a non-existing entry
            test_data_container d_cont;

            // delete the created entry
            EXPECT_TRUE(d_cont.delete_data("category", "key"));
            EXPECT_EQ(d_cont.get_data_by_key("category", "key"), empty_pair);
            //EXPECT_FALSE(d_cont.data_update_notified());
        }
        {
            // Log with info level = true
            ::testing::internal::CaptureStdout();
            test_data_container d_cont;
            d_cont.set_data("category", "key", "data_type", "value", false);
            EXPECT_TRUE(d_cont.delete_data("category", "key", true));
            EXPECT_NE(::testing::internal::GetCapturedStdout(), "");
        }

        // Negative

        {
            // Leave category empty
            NO_COUT_TEST_BLOCK;
            test_data_container d_cont;
            d_cont.set_data("category", "key", "data_type", "value", false);
            d_cont.data_update_notified();
            EXPECT_FALSE(d_cont.delete_data("", "key", false));
            //EXPECT_FALSE(d_cont.data_update_notified());
        }
        {
            // Leave key empty
            NO_COUT_TEST_BLOCK;
            test_data_container d_cont;
            d_cont.set_data("category", "key", "data_type", "value", false);
            d_cont.data_update_notified();
            EXPECT_FALSE(d_cont.delete_data("category", "", false));
            //EXPECT_FALSE(d_cont.data_update_notified());
        }

TEST_END
}

/**
 * Testing the get_data function
 *
 * Functions: get_data
 */
TEST_F(data_container_test, check_get_data)
{
    TEST_START
        test_data_container d_cont;
        d_cont.set_data("category_0", "key_0", "data_type_0", "value_0", false);
        d_cont.set_data("category_0", "key_1", "data_type_1", "value_1", false);
        d_cont.set_data("category_1", "key_2", "data_type_2", "value_2", false);
        d_cont.set_data("category_1", "key_0", "data_type_3", "value_3", false);

        // The expected result of get_data
        std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> data;
        data[std::make_tuple("category_0", "key_0")] = std::make_tuple("data_type_0", "value_0");
        data[std::make_tuple("category_0", "key_1")] = std::make_tuple("data_type_1", "value_1");
        data[std::make_tuple("category_1", "key_2")] = std::make_tuple("data_type_2", "value_2");
        data[std::make_tuple("category_1", "key_0")] = std::make_tuple("data_type_3", "value_3");

        EXPECT_EQ(d_cont.get_data(), data);

    TEST_END
}

/**
 * Testing the get_data_by_key function
 *
 * Functions: get_data_by_key
 */
TEST_F(data_container_test, check_get_data_by_key)
{
    TEST_START
    // Create a data container, filled with some entries
    test_data_container d_cont;
    d_cont.set_data("category_0", "key_0", "data_type_0", "value_0", false);
    d_cont.set_data("category_0", "key_1", "data_type_1", "value_1", false);
    d_cont.set_data("category_1", "key_2", "data_type_2", "value_2", false);
    d_cont.set_data("category_1", "key_0", "data_type_3", "value_3", false);

    {
        // Get an existing entry
        EXPECT_EQ(d_cont.get_data_by_key("category_0", "key_0"), std::make_tuple("data_type_0", "value_0"));
    }
    {
        // Get an entry with a non-existing (category,key) pair
        EXPECT_EQ(d_cont.get_data_by_key("category_0", "key_2"), empty_pair);
    }
    // NEGATIVE
    {
        // Leave category empty
        NO_COUT_TEST_BLOCK;
        EXPECT_EQ(d_cont.get_data_by_key("", "key"), empty_pair);
    }
    {
        // Leave key empty
        NO_COUT_TEST_BLOCK;
        EXPECT_EQ(d_cont.get_data_by_key("category", ""), empty_pair);
    }

    TEST_END
}

/**
 * Testing the get_data_keys function
 *
 * Functions: get_data_keys
 */
TEST_F(data_container_test, check_get_data_keys)
{
    TEST_START
    // Create a data container, filled with some entries
    test_data_container d_cont;
    d_cont.set_data("category_0", "key_0", "data_type_0", "value_0", false);
    d_cont.set_data("category_0", "key_1", "data_type_1", "value_1", false);
    d_cont.set_data("category_1", "key_2", "data_type_2", "value_2", false);
    d_cont.set_data("category_1", "key_0", "data_type_3", "value_3", false);

    // The expected result of get_data_keys
    std::vector<std::tuple<std::string, std::string>> keys = {
        std::make_tuple("category_0", "key_0"), std::make_tuple("category_0", "key_1"), std::make_tuple("category_1", "key_2"), std::make_tuple("category_1", "key_0")};

    EXPECT_TRUE(vectors_have_same_content(d_cont.get_data_keys(), keys));

    TEST_END
}