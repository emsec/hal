#include "hal_core/netlist/data_container.h"

#include "hal_core/netlist/parameter.h"
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

/**
     * Testing the typed-parameter API: set_parameter / get_parameter_value /
     * get_parameter_declaration / has_parameter / delete_parameter /
     * get_parameters. Values are validated against the supplied Parameter
     * declaration via Parameter::validate.
     *
     * Functions: set_parameter, get_parameter_value, get_parameter_declaration,
     *            has_parameter, delete_parameter, get_parameters
     */
TEST_F(DataContainerTest, check_parameters)
{
    TEST_START
    const auto width_decl  = Parameter::BitVector("WIDTH", 32, "").get();
    const auto mask_decl   = Parameter::BitVector("mask", 16, "").get();
    const auto flavor_decl = Parameter::Enum("flavor", {"normal", "fast", "slow"}, "normal").get();

    {
        // Positive: store typed values, read them back, and inspect the map.
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.set_parameter(width_decl, "0x20").is_ok());
        EXPECT_TRUE(d_cont.set_parameter(mask_decl, "0xCAFE").is_ok());
        EXPECT_TRUE(d_cont.set_parameter(flavor_decl, "fast").is_ok());

        EXPECT_EQ(d_cont.get_parameter_value("WIDTH").get(), "0x20");
        EXPECT_EQ(d_cont.get_parameter_value("mask").get(), "0xCAFE");
        EXPECT_EQ(d_cont.get_parameter_value("flavor").get(), "fast");

        EXPECT_EQ(d_cont.get_parameter_declaration("WIDTH").get(), width_decl);
        EXPECT_EQ(d_cont.get_parameter_declaration("mask").get(), mask_decl);
        EXPECT_EQ(d_cont.get_parameter_declaration("flavor").get(), flavor_decl);

        ASSERT_EQ(d_cont.get_parameters().size(), 3u);
        EXPECT_EQ(d_cont.get_parameters().at({Parameter::Source::Generic, "WIDTH"}).first, width_decl);
        EXPECT_EQ(d_cont.get_parameters(Parameter::Source::Generic).at("WIDTH").second, "0x20");
    }
    {
        // The (Parameter) lookup returns the stored value when the declaration matches.
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.set_parameter(width_decl, "0x20").is_ok());
        EXPECT_EQ(d_cont.get_parameter_value(width_decl).get(), "0x20");

        // A declaration that disagrees with the stored one is rejected.
        const auto width_8 = Parameter::BitVector("WIDTH", 8, "").get();
        EXPECT_TRUE(d_cont.get_parameter_value(width_8).is_error());
    }
    {
        // has_parameter reports both name- and Parameter-keyed presence.
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.set_parameter(width_decl, "0x20").is_ok());
        EXPECT_TRUE(d_cont.has_parameter("WIDTH"));
        EXPECT_TRUE(d_cont.has_parameter(width_decl));
        EXPECT_FALSE(d_cont.has_parameter("unknown"));
        EXPECT_FALSE(d_cont.has_parameter(Parameter::BitVector("WIDTH", 8, "").get()));
    }
    {
        // delete_parameter removes the stored entry and reports whether anything was deleted.
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.set_parameter(width_decl, "0x20").is_ok());
        EXPECT_TRUE(d_cont.delete_parameter("WIDTH"));
        EXPECT_FALSE(d_cont.has_parameter("WIDTH"));
        EXPECT_TRUE(d_cont.get_parameter_value("WIDTH").is_error());
        EXPECT_FALSE(d_cont.delete_parameter("WIDTH"));    // already gone
    }

    // Negative
    {
        // Values that fail Parameter::validate are rejected; nothing is stored.
        NO_COUT_TEST_BLOCK;
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.set_parameter(mask_decl, "0x10000").is_error());          // overflow
        EXPECT_FALSE(d_cont.has_parameter("mask"));
        EXPECT_TRUE(d_cont.set_parameter(flavor_decl, "unknown_value").is_error());  // not in enum
        EXPECT_FALSE(d_cont.has_parameter("flavor"));
    }
    {
        // Looking up a missing parameter returns an error rather than a default.
        NO_COUT_TEST_BLOCK;
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.get_parameter_value("missing").is_error());
        EXPECT_TRUE(d_cont.get_parameter_declaration("missing").is_error());
        EXPECT_FALSE(d_cont.has_parameter("missing"));
    }
    TEST_END
}

/**
     * Testing attributes in the typed store: a declaration with the attribute source lives next to the generics, so
     * that an attribute and a generic may share a name, and every accessor selects by source.
     *
     * Functions: set_parameter, get_parameter_value, get_parameter_declaration, has_parameter, delete_parameter,
     *            get_parameters
     */
TEST_F(DataContainerTest, check_attributes)
{
    TEST_START
    const auto attr      = Parameter::Source::Attribute;
    const auto keep_decl = Parameter::Boolean("keep", "false", attr).get();
    const auto loc_decl  = Parameter::String("LOC", "", attr).get();
    const auto init_decl = Parameter::BitVector("INIT", 8, "0x0", attr).get();
    EXPECT_EQ(keep_decl.get_source(), attr);
    EXPECT_EQ(Parameter::Boolean("keep", "false").get().get_source(), Parameter::Source::Generic);
    EXPECT_EQ(Parameter::Boolean("keep", "false").get().with_source(attr), keep_decl);
    EXPECT_NE(Parameter::Boolean("keep", "false").get(), keep_decl);    // the source is part of the declaration

    {
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.set_parameter(keep_decl, "true").is_ok());
        EXPECT_TRUE(d_cont.set_parameter(loc_decl, "SLICE_X0Y0").is_ok());
        EXPECT_TRUE(d_cont.set_parameter(init_decl, "0xab").is_ok());

        EXPECT_EQ(d_cont.get_parameter_value("keep", attr).get(), "true");
        EXPECT_EQ(d_cont.get_parameter_value("LOC", attr).get(), "SLICE_X0Y0");
        EXPECT_EQ(d_cont.get_parameter_value("INIT", attr).get(), "0xAB");    // bit vectors are normalized like generics
        EXPECT_EQ(d_cont.get_parameter_value(keep_decl).get(), "true");
        EXPECT_EQ(d_cont.get_parameter_declaration("keep", attr).get(), keep_decl);
        EXPECT_TRUE(d_cont.has_parameter("LOC", attr));
        EXPECT_TRUE(d_cont.has_parameter(loc_decl));
        EXPECT_FALSE(d_cont.has_parameter("LOC"));    // no generic of that name
        EXPECT_FALSE(d_cont.has_parameter("unknown", attr));
        ASSERT_EQ(d_cont.get_parameters().size(), 3u);
        ASSERT_EQ(d_cont.get_parameters(attr).size(), 3u);
        EXPECT_TRUE(d_cont.get_parameters(Parameter::Source::Generic).empty());
        EXPECT_EQ(d_cont.get_parameters(attr).at("INIT").first, init_decl);
        EXPECT_EQ(d_cont.get_parameters().at({attr, "INIT"}).first, init_decl);

        // an attribute and a generic may share a name
        EXPECT_TRUE(d_cont.set_parameter(Parameter::String("keep", "").get(), "no").is_ok());
        EXPECT_EQ(d_cont.get_parameter_value("keep", attr).get(), "true");
        EXPECT_EQ(d_cont.get_parameter_value("keep").get(), "no");
        EXPECT_EQ(d_cont.get_parameters().size(), 4u);
        EXPECT_EQ(d_cont.get_parameters(Parameter::Source::Generic).size(), 1u);

        EXPECT_TRUE(d_cont.delete_parameter("keep", attr));
        EXPECT_FALSE(d_cont.has_parameter("keep", attr));
        EXPECT_TRUE(d_cont.has_parameter("keep"));
        EXPECT_FALSE(d_cont.delete_parameter("keep", attr));
    }

    // Negative
    {
        NO_COUT_TEST_BLOCK;
        TestDataContainer d_cont;
        EXPECT_TRUE(d_cont.set_parameter(init_decl, "0x100").is_error());    // overflow
        EXPECT_FALSE(d_cont.has_parameter("INIT", attr));
        EXPECT_TRUE(d_cont.get_parameter_value("missing", attr).is_error());
        EXPECT_TRUE(d_cont.get_parameter_declaration("missing", attr).is_error());
        EXPECT_TRUE(d_cont.set_parameter(keep_decl, "true").is_ok());
        EXPECT_TRUE(d_cont.get_parameter_value(keep_decl.with_source(Parameter::Source::Generic)).is_error());
    }
    {
        // equality takes the attributes into account
        TestDataContainer a;
        TestDataContainer b;
        EXPECT_TRUE(a.set_parameter(keep_decl, "true").is_ok());
        EXPECT_NE(a, b);
        EXPECT_TRUE(b.set_parameter(keep_decl, "true").is_ok());
        EXPECT_EQ(a, b);
    }
    TEST_END
}

/**
     * Equality of DataContainers takes both the data map and the parameter map
     * into account.
     *
     * Functions: operator==, operator!=
     */
TEST_F(DataContainerTest, check_parameter_equality)
{
    TEST_START
    const auto width_decl = Parameter::BitVector("WIDTH", 32, "").get();

    TestDataContainer a;
    TestDataContainer b;
    EXPECT_TRUE(a == b);

    EXPECT_TRUE(a.set_parameter(width_decl, "0x20").is_ok());
    EXPECT_TRUE(a != b);

    EXPECT_TRUE(b.set_parameter(width_decl, "0x20").is_ok());
    EXPECT_TRUE(a == b);

    EXPECT_TRUE(b.delete_parameter("WIDTH"));
    EXPECT_TRUE(b.set_parameter(width_decl, "0x10").is_ok());
    EXPECT_TRUE(a != b);    // same declaration, different value
    TEST_END
}
}
