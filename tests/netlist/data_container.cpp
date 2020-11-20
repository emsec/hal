#include "hal_core/netlist/data_container.h"

#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include <iostream>

namespace hal
{
    /*
     * A child of DataContainer (used for testing)
     */
    class TestDataContainer : public DataContainer
    {
    public:
        TestDataContainer()  = default;
        ~TestDataContainer() = default;

        /**
         * Returns true whenever the notify_updated() was called since the last
         * call of data_update_notified
         */
        bool data_update_notified()
        {
            if (m_notified)
            {
                m_notified = false;
                return true;
            }
            return false;
        }

    private:
        bool m_notified = false;
        void notify_updated()
        {
            m_notified = true;
        }
    };

    class DataContainerTest : public ::testing::Test
    {
    protected:
        const std::tuple<std::string, std::string> m_empty_pair = std::make_tuple("", "");

        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }
    };

    /**
     * Testing the set_data function. To verify success, the get_data function
     * is used.
     *
     * Functions: set_data, get_data
     */
    TEST_F(DataContainerTest, check_set_data){TEST_START{// Set multiple data with different keys and categories
                                                         TestDataContainer d_cont;
    EXPECT_TRUE(d_cont.set_data("category_0", "key_0", "data_type_0", "value_0", false));
    EXPECT_TRUE(d_cont.set_data("category_0", "key_1", "data_type_1", "value_1", false));
    EXPECT_TRUE(d_cont.set_data("category_1", "key_2", "data_type_2", "value_2", false));
    EXPECT_TRUE(d_cont.set_data("category_1", "key_0", "data_type_3", "value_3", false));

    //EXPECT_TRUE(d_cont.data_update_notified());
    EXPECT_EQ(d_cont.get_data("category_0", "key_0"), std::make_tuple("data_type_0", "value_0"));
    EXPECT_EQ(d_cont.get_data("category_0", "key_1"), std::make_tuple("data_type_1", "value_1"));
    EXPECT_EQ(d_cont.get_data("category_1", "key_2"), std::make_tuple("data_type_2", "value_2"));
    EXPECT_EQ(d_cont.get_data("category_1", "key_0"), std::make_tuple("data_type_3", "value_3"));
}    // namespace hal
{
    // Overwrites data with the same key and category
    TestDataContainer d_cont;
    EXPECT_TRUE(d_cont.set_data("category", "key", "data_type", "value", false));
    //EXPECT_TRUE(d_cont.data_update_notified());
    EXPECT_EQ(d_cont.get_data("category", "key"), std::make_tuple("data_type", "value"));

    EXPECT_TRUE(d_cont.set_data("category", "key", "new_data_type", "new_value", false));
    //EXPECT_TRUE(d_cont.data_update_notified());
    EXPECT_EQ(d_cont.get_data("category", "key"), std::make_tuple("new_data_type", "new_value"));
}
{
    // Log with info level = true
    ::testing::internal::CaptureStdout();
    TestDataContainer d_cont;
    EXPECT_TRUE(d_cont.set_data("category", "key", "data_type", "value", true));
    EXPECT_NE(::testing::internal::GetCapturedStdout(), "");
}

// Negative
{
    // Leave category empty
    NO_COUT_TEST_BLOCK;
    TestDataContainer d_cont;
    EXPECT_FALSE(d_cont.set_data("", "key", "data_type", "value"));
    //EXPECT_FALSE(d_cont.data_update_notified());
    EXPECT_EQ(d_cont.get_data("", "key"), m_empty_pair);
}
{
    // Leave key empty
    NO_COUT_TEST_BLOCK;
    TestDataContainer d_cont;
    EXPECT_FALSE(d_cont.set_data("category", "", "data_type", "value"));
    //EXPECT_FALSE(d_cont.data_update_notified());
    EXPECT_EQ(d_cont.get_data("category", ""), m_empty_pair);
}

TEST_END
}

/**
     * Testing the delete_data function. To verify success, the get_data function
     * is used.
     *
     * Functions: delete_data, get_data
     */
TEST_F(DataContainerTest, check_delete_data){TEST_START{// Delete an existing entry
                                                        TestDataContainer d_cont;
d_cont.set_data("category", "key", "data_type", "value", false);    // create an entry
//EXPECT_TRUE(d_cont.data_update_notified());

// delete the created entry
EXPECT_TRUE(d_cont.delete_data("category", "key"));
//EXPECT_TRUE(d_cont.data_update_notified());
EXPECT_EQ(d_cont.get_data("category", "key"), m_empty_pair);
}
{
    // Delete a non-existing entry
    TestDataContainer d_cont;

    // delete the created entry
    EXPECT_TRUE(d_cont.delete_data("category", "key"));
    EXPECT_EQ(d_cont.get_data("category", "key"), m_empty_pair);
    //EXPECT_FALSE(d_cont.data_update_notified());
}
{
    // Log with info level = true
    ::testing::internal::CaptureStdout();
    TestDataContainer d_cont;
    d_cont.set_data("category", "key", "data_type", "value", false);
    EXPECT_TRUE(d_cont.delete_data("category", "key", true));
    EXPECT_NE(::testing::internal::GetCapturedStdout(), "");
}

// Negative

{
    // Leave category empty
    NO_COUT_TEST_BLOCK;
    TestDataContainer d_cont;
    d_cont.set_data("category", "key", "data_type", "value", false);
    d_cont.data_update_notified();
    EXPECT_FALSE(d_cont.delete_data("", "key", false));
    //EXPECT_FALSE(d_cont.data_update_notified());
}
{
    // Leave key empty
    NO_COUT_TEST_BLOCK;
    TestDataContainer d_cont;
    d_cont.set_data("category", "key", "data_type", "value", false);
    d_cont.data_update_notified();
    EXPECT_FALSE(d_cont.delete_data("category", "", false));
    //EXPECT_FALSE(d_cont.data_update_notified());
}

TEST_END
}

/**
     * Testing the get_data_map function
     *
     * Functions: get_data_map
     */
TEST_F(DataContainerTest, check_get_data_map)
{
    TEST_START
    TestDataContainer d_cont;
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

    EXPECT_EQ(d_cont.get_data_map(), data);

    TEST_END
}

/**
     * Testing the get_data function
     *
     * Functions: get_data
     */
TEST_F(DataContainerTest, check_get_data)
{
    TEST_START
    // Create a data container, filled with some entries
    TestDataContainer d_cont;
    d_cont.set_data("category_0", "key_0", "data_type_0", "value_0", false);
    d_cont.set_data("category_0", "key_1", "data_type_1", "value_1", false);
    d_cont.set_data("category_1", "key_2", "data_type_2", "value_2", false);
    d_cont.set_data("category_1", "key_0", "data_type_3", "value_3", false);

    {
        // Get an existing entry
        EXPECT_EQ(d_cont.get_data("category_0", "key_0"), std::make_tuple("data_type_0", "value_0"));
    }
    {
        // Get an entry with a non-existing (category,key) pair
        EXPECT_EQ(d_cont.get_data("category_0", "key_2"), m_empty_pair);
    }
    // NEGATIVE
    {
        // Leave category empty
        NO_COUT_TEST_BLOCK;
        EXPECT_EQ(d_cont.get_data("", "key"), m_empty_pair);
    }
    {
        // Leave key empty
        NO_COUT_TEST_BLOCK;
        EXPECT_EQ(d_cont.get_data("category", ""), m_empty_pair);
    }

    TEST_END
}
}
