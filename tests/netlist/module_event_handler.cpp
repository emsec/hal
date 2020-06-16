
// Checks if two vectors have the same content regardless of their order
template<typename T>
bool vectors_have_same_content(std::vector<T> vec_1, std::vector<T> vec_2)
{
    if (vec_1.size() != vec_2.size())
        return false;

    // Each element of vec_1 must be found in vec_2
    while (vec_1.size() > 0)
    {
        auto it_1       = vec_1.begin();
        bool found_elem = false;
        for (auto it_2 = vec_2.begin(); it_2 != vec_2.end(); it_2++)
        {
            if (*it_1 == *it_2)
            {
                found_elem = true;
                vec_2.erase(it_2);
                break;
            }
        }
        if (!found_elem)
        {
            return false;
        }
        vec_1.erase(it_1);
    }

    return true;
}
}
;

/**
 * Testing the registration of update_callbacks (called whenever data of a Module is changed).
 * The notify_data_updated is called by the modules.
 *
 * Functions: register_update_callback
 */
TEST_F(module_handler_test, check_data_updated){TEST_START{// Create a netlist with 3 modules
                                                              std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Module> module_0(new Module(nl, 0, "module_0"));
std::shared_ptr<Module> module_1(new Module(nl, 1, "module_1"));
std::shared_ptr<Module> module_2(new Module(nl, 2, "module_2"));

// Register the update callback
module_handler::enable(true);
module_handler::register_update_callback("test_update_callback", add_updated_module);

// Change the note of module_0 and module_1 (should call the callback)
{
    NO_COUT_TEST_BLOCK;
    module_0->set_note("new_note");
    module_1->set_note("new_note");
}

std::vector<std::shared_ptr<Module>> exp_updated = {module_0, module_1};

EXPECT_TRUE(vectors_have_same_content(get_updated_modules(), exp_updated));

module_handler::unregister_update_callback("test_update_callback");
}
TEST_END
}

/**
 * Testing the registration of remove_callbacks (called whenever a Module is removed).
 * The notify_data_removed is called by the modules.
 *
 * Functions: register_remove_callback
 */
TEST_F(module_handler_test, check_data_removed){TEST_START{// Create a netlist with 3 modules
                                                              std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Module> module_0(new Module(nl, 0, "module_0"));
std::shared_ptr<Module> module_1(new Module(nl, 1, "module_1"));
std::shared_ptr<Module> module_2(new Module(nl, 2, "module_2"));

// Register the remove callback
module_handler::enable(true);
module_handler::register_remove_callback("test_remove_callback", add_removed_module);

// Call the notify_data_removed function to call the callbacks
{
    NO_COUT_TEST_BLOCK;
    module_handler::notify_data_removed(module_0);
    module_handler::notify_data_removed(module_1);
}

std::vector<std::shared_ptr<Module>> exp_removed = {module_0, module_1};

EXPECT_TRUE(vectors_have_same_content(get_removed_modules(), exp_removed));

module_handler::unregister_remove_callback("test_remove_callback");
}
TEST_END
}

/**
 * Testing the unregister functions
 *
 * Functions: unregister_update_callback, unregister_remove_callback
 */
TEST_F(module_handler_test, check_unregister){TEST_START{// Create a netlist with one Module
                                                            std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Module> module_0(new Module(nl, 0, "module_0"));

// Register the two callbacks and unregister them immediately
module_handler::enable(true);
module_handler::register_update_callback("test_update_callback", add_updated_module);
module_handler::register_remove_callback("test_remove_callback", add_removed_module);

module_handler::unregister_update_callback("test_update_callback");
module_handler::unregister_remove_callback("test_remove_callback");

// Update the Module and call the notify_data_removed function
{
    NO_COUT_TEST_BLOCK;
    module_0->set_note("new_note");
    module_handler::notify_data_removed(module_0);
}

// The callback shouldn't be called
EXPECT_TRUE(get_updated_modules().empty());
EXPECT_TRUE(get_removed_modules().empty());
}
TEST_END
}

/**
 * Testing the callbacks with the enable flag set to false
 *
 * Functions: enable
 */
TEST_F(module_handler_test, check_enable)
{
    TEST_START
    {
        // Create a netlist with one Module
        std::shared_ptr<Netlist> nl = create_empty_netlist(0);
        std::shared_ptr<Module> module_0(new Module(nl, 0, "module_0"));

        // Register the two callbacks but set enable to false
        module_handler::enable(false);
        module_handler::register_update_callback("test_update_callback", add_updated_module);
        module_handler::register_remove_callback("test_remove_callback", add_removed_module);

        // Update the Module and call the notify_data_removed function
        {
            NO_COUT_TEST_BLOCK;
            module_0->set_note("new_note");
            module_handler::notify_data_removed(module_0);
        }

        // The callback shouldn't be called
        EXPECT_TRUE(get_updated_modules().empty());
        EXPECT_TRUE(get_removed_modules().empty());
    }
    TEST_END
}
