#include "hal_core/utilities/result.h"
#include "netlist_test_utils.h"

#include "test_def.h"

#include "gtest/gtest.h"

namespace hal
{
    class ResultTest : public ::testing::Test
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

    TEST_F(ResultTest, check_ok)
    {
        TEST_START
        {
            Result<u32> ok_123 = OK(123);
            ASSERT_TRUE(ok_123.is_ok());
            EXPECT_FALSE(ok_123.is_error());
            EXPECT_EQ(ok_123.get(), 123);

            Result<std::string> ok_string = OK("works on my machine");
            ASSERT_TRUE(ok_string.is_ok());
            EXPECT_FALSE(ok_string.is_error());
            EXPECT_EQ(ok_string.get(), "works on my machine");

            Result<std::vector<u32>> ok_vector = OK({1, 2, 3});
            ASSERT_TRUE(ok_vector.is_ok());
            EXPECT_FALSE(ok_vector.is_error());
            EXPECT_EQ(ok_vector.get(), std::vector<u32>({1, 2, 3}));

            auto shared_ptr = std::make_shared<u32>(123);
            Result<std::shared_ptr<u32>> ok_shared = OK(shared_ptr);
            ASSERT_TRUE(ok_shared.is_ok());
            EXPECT_FALSE(ok_shared.is_error());
            EXPECT_EQ(ok_shared.get(), shared_ptr);

            auto unique_ptr = std::make_unique<u32>(123);
            Result<std::unique_ptr<u32>> ok_unique = OK(std::move(unique_ptr));
            ASSERT_TRUE(ok_unique.is_ok());
            EXPECT_FALSE(ok_unique.is_error());
            EXPECT_EQ(*ok_unique.get(), 123);
        }
        TEST_END
    }

    TEST_F(ResultTest, check_error)
    {
        TEST_START
        {
            Result<u32> error_int = ERR("failed");
            EXPECT_FALSE(error_int.is_ok());
            ASSERT_TRUE(error_int.is_error());
            EXPECT_TRUE(utils::ends_with<std::string>(error_int.get_error().get(), "failed"));

            Result<std::string> error_string = ERR("works on my machine");
            EXPECT_FALSE(error_string.is_ok());
            ASSERT_TRUE(error_string.is_error());
            EXPECT_TRUE(utils::ends_with<std::string>(error_string.get_error().get(), "works on my machine"));

            Result<std::vector<u32>> error_vector = ERR("works on my machine");
            EXPECT_FALSE(error_vector.is_ok());
            ASSERT_TRUE(error_vector.is_error());
            EXPECT_TRUE(utils::ends_with<std::string>(error_vector.get_error().get(), "works on my machine"));

            Result<std::shared_ptr<u32>> error_shared = ERR("works on my machine");
            EXPECT_FALSE(error_shared.is_ok());
            ASSERT_TRUE(error_shared.is_error());
            EXPECT_TRUE(utils::ends_with<std::string>(error_shared.get_error().get(), "works on my machine"));

            Result<std::unique_ptr<u32>> error_unique = ERR("works on my machine");
            EXPECT_FALSE(error_unique.is_ok());
            ASSERT_TRUE(error_unique.is_error());
            EXPECT_TRUE(utils::ends_with<std::string>(error_unique.get_error().get(), "works on my machine"));
        }
        TEST_END
    }

    TEST_F(ResultTest, check_type_deduction)
    {
        TEST_START
        { 
            auto f = []() -> Result<u32> { return OK(123); };
            auto g = []() -> Result<u32> { return ERR("works on my machine"); };

            ASSERT_TRUE(f().is_ok());
            EXPECT_FALSE(f().is_error());
            EXPECT_EQ(f().get(), 123);

            EXPECT_FALSE(g().is_ok());
            ASSERT_TRUE(g().is_error());
            EXPECT_TRUE(utils::ends_with<std::string>(g().get_error().get(), "works on my machine"));
        }
        TEST_END
    }

    TEST_F(ResultTest, check_map)
    {
        TEST_START
        { 
            auto f = []() -> Result<u32> { return OK(123); };
            auto g = []() -> Result<u32> { return ERR("works on my machine"); };

            auto f_map = f().map<std::string>([](auto value) -> Result<std::string> { return OK(std::to_string(value)); });
            ASSERT_TRUE(f_map.is_ok());
            EXPECT_EQ(f_map.get(), "123");

            auto g_map = g().map<std::string>([](auto value) -> Result<std::string> { return OK(std::to_string(value)); });
            ASSERT_TRUE(g_map.is_error());
            EXPECT_TRUE(utils::ends_with<std::string>(g_map.get_error().get(), "works on my machine"));
        }
        TEST_END
    }
}    // namespace hal