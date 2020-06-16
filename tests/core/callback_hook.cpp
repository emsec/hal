#include "test_def.h"
#include "gtest/gtest.h"
#include <core/callback_hook.h>
#include <core/log.h>
#include <iostream>
#include "netlist_test_utils.h"

typedef CallbackHook<std::string(std::string)> test_hook;
typedef std::function<std::string(std::string)> test_function;

typedef CallbackHook<void(int&)> sum_up_hook;
typedef std::function<void(int&)> add_function;

#define CALLBACK_HOOK_MIN_IDX 0x1

class callback_hook_test : public ::testing::Test
{
protected:
    // std::functions for the corresponding member functions
    test_function test_func_0;
    test_function test_func_1;
    add_function add_2_func;
    add_function add_5_func;
    add_function add_7_func;

    virtual void SetUp()
    {
        test_utils::init_log_channels();
        test_func_0 = test_callback_0;
        test_func_1 = test_callback_1;
        add_2_func  = add_2;
        add_5_func  = add_5;
        add_7_func  = add_7;
    }

    virtual void TearDown()
    {
    }

    // Functions used to test adding and removing callbacks
    static std::string test_callback_0(std::string str)
    {
        return "0_" + str;
    }

    static std::string test_callback_1(std::string str)
    {
        return "1_" + str;
    }

    // Functions that sums up an integer (used in last TEST_F)
    static void add_2(int& i)
    {
        i = i + 2;
    }

    static void add_5(int& i)
    {
        i = i + 5;
    }

    static void add_7(int& i)
    {
        i = i + 7;
    }
};

/**
 * Tests adding callbacks with an id. Access it with is_callback_registered
 * and call it after. Also try to overwrite a callback with the same id and
 * call callbacks with unknown id.
 *
 * Functions: add_callback, is_callback_registered, call, size
 */
TEST_F(callback_hook_test, check_add_callbacks_id){TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################

        {// Adds two hooks and calls them
            {test_hook c_hook;
                c_hook.add_callback(test_func_0, (u64) CALLBACK_HOOK_MIN_IDX);
                c_hook.add_callback(test_func_1, (u64) CALLBACK_HOOK_MIN_IDX+1);

                EXPECT_EQ(c_hook.call(CALLBACK_HOOK_MIN_IDX, "test"), "0_test");
                EXPECT_EQ(c_hook.call(CALLBACK_HOOK_MIN_IDX+1, "test"), "1_test");
                EXPECT_EQ(c_hook.size(), (size_t)2);
                EXPECT_TRUE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX));
                EXPECT_TRUE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX+1));
                EXPECT_FALSE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX+2));
            }
            {
                // Adds two hooks, but the second one without id
                test_hook c_hook;
                c_hook.add_callback(test_func_0, (u64) CALLBACK_HOOK_MIN_IDX);
                c_hook.add_callback(test_func_1);

                EXPECT_EQ(c_hook.call(CALLBACK_HOOK_MIN_IDX, "test"), "0_test");
                EXPECT_EQ(c_hook.call(CALLBACK_HOOK_MIN_IDX+1, "test"), "1_test");
                EXPECT_EQ(c_hook.size(), (size_t)2);
                EXPECT_TRUE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX));
                EXPECT_TRUE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX+1));
                EXPECT_FALSE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX+2));
            }
        }
        {
            // Overwrites the first hook with the second one
            test_hook c_hook;
            c_hook.add_callback(test_func_0, (u64)CALLBACK_HOOK_MIN_IDX);
            c_hook.add_callback(test_func_1, (u64)CALLBACK_HOOK_MIN_IDX);

            EXPECT_EQ(c_hook.size(), (size_t)1);
            EXPECT_EQ(c_hook.call(CALLBACK_HOOK_MIN_IDX, "test"), "1_test");
            EXPECT_TRUE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX));
        }

// ########################
// NEGATIVE TESTS
// ########################

        {
            // Try to call a callback with an unknown id
            test_hook c_hook;
            c_hook.add_callback(test_func_0, (u64)CALLBACK_HOOK_MIN_IDX);

            EXPECT_EQ(c_hook.call(123, "test"), "");
        }
    TEST_END
}

/**
 * Tests adding callbacks with a id-string. Access it with is_callback_registered
 * and call it after. Also try to overwrite a callback with the same id-string and
 * call callbacks with unknown id.
 *
 * Functions: add_callback, is_callback_registered, call, size
 */
TEST_F(callback_hook_test, check_add_callbacks_string){TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################

        {// Adds two hooks and calls them
            test_hook c_hook;
            c_hook.add_callback("id_0", test_func_0);
            c_hook.add_callback("id_1", test_func_1);

            EXPECT_EQ(c_hook.call("id_0", "test"), "0_test");
            EXPECT_EQ(c_hook.call("id_1", "test"), "1_test");
            EXPECT_TRUE(c_hook.is_callback_registered("id_0"));
            EXPECT_TRUE(c_hook.is_callback_registered("id_1"));
            EXPECT_EQ(c_hook.size(), (size_t)2);
            EXPECT_FALSE(c_hook.is_callback_registered("unknown_id"));
        }
        {
            // Overwrites the first hook with the second one
            test_hook c_hook;
            c_hook.add_callback("id_0", test_func_0);
            c_hook.add_callback("id_0", test_func_1);

            EXPECT_EQ(c_hook.call("id_0", "test"), "1_test");
            EXPECT_EQ(c_hook.size(), (size_t)1);
            EXPECT_TRUE(c_hook.is_callback_registered("id_0"));
        }

// ########################
// NEGATIVE TESTS
// ########################

        {
            // Try to call a callback with an unknown id-string
            test_hook c_hook;
            c_hook.add_callback("id_0", test_func_0);

            EXPECT_EQ(c_hook.call("unknown_id", "test"), "");
        }
        {
            // Try to add a callback with an empty id-string
            test_hook c_hook;
            c_hook.add_callback("", test_func_0);

            EXPECT_EQ(c_hook.call("", "test"), "0_test");
            EXPECT_EQ(c_hook.size(), (size_t)1);
        }
    TEST_END
}

/**
 * Tests removing an existing callback as well as a non-existing one
 *
 * Functions: remove_callback, size, is_callback_registered
 */
TEST_F(callback_hook_test, check_remove_callback){TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################
        {// Remove an existing callback (id)
            test_hook c_hook;
            c_hook.add_callback(test_func_0, CALLBACK_HOOK_MIN_IDX);
            c_hook.remove_callback(CALLBACK_HOOK_MIN_IDX);

            EXPECT_FALSE(c_hook.is_callback_registered(CALLBACK_HOOK_MIN_IDX));
            EXPECT_EQ(c_hook.size(), (size_t)0);
        }
        {
            // Remove an existing callback (id-string)
            test_hook c_hook;
            c_hook.add_callback("id_0", test_func_0);
            c_hook.remove_callback("id_0");

            EXPECT_FALSE(c_hook.is_callback_registered("id_0"));
            EXPECT_EQ(c_hook.size(), (size_t)0);
        }

// ########################
// NEGATIVE TESTS
// ########################

        {
            // Remove an non-existing callback (id)
            test_hook c_hook;
            c_hook.remove_callback(42);

            EXPECT_FALSE(c_hook.is_callback_registered(42));
            EXPECT_EQ(c_hook.size(), (size_t)0);
        }
        {
            // Remove an non-existing callback (id-string)
            test_hook c_hook;
            c_hook.remove_callback("unknown_id");

            EXPECT_FALSE(c_hook.is_callback_registered("unknown_id"));
            EXPECT_EQ(c_hook.size(), (size_t)0);
        }
    TEST_END
}

/**
 * Tests the operator() which calls all registered callbacks, by using
 * callbacks which sums up an integer. Add the callbacks with a mixture of
 * id and id-string representations
 *
 * Functions: operator()
 */
TEST_F(callback_hook_test, check_bracket_operator)
{
    TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################

        {
            // Sums up 2,5 and 7 via callback_hooks
            sum_up_hook sum_hook;
            sum_hook.add_callback(add_2_func, CALLBACK_HOOK_MIN_IDX);
            sum_hook.add_callback(add_5_func, CALLBACK_HOOK_MIN_IDX+1);
            sum_hook.add_callback("id_2", add_7_func);

            int res = 0;
            sum_hook(res);
            EXPECT_EQ(res, 14);
        }

        // ########################
        // NEGATIVE TESTS
        // ########################

        {
            // Calls operator() without any callbacks registered
            sum_up_hook sum_hook;
            int res = 0;
            sum_hook(res);
            EXPECT_EQ(res, 0);
        }
    TEST_END
}
