#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/utilities/log.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include <iostream>

namespace hal
{
    /**
     * Tests for class GateType.
     */
    class GateTypeTest : public ::testing::Test
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
     * Testing construction and basic functionality.
     *
     * Functions: GateType, get_id, get_name, get_base_type
     */
    TEST_F(GateTypeTest, check_construction)
    {
        TEST_START

        // combinational gate type
        GateType gt1("combinational", GateType::BaseType::combinational);
        EXPECT_EQ(gt1.get_id(), 1);
        EXPECT_EQ(gt1.get_name(), "combinational");
        EXPECT_EQ(gt1.get_base_type(), GateType::BaseType::combinational);

        // FF gate type
        GateType gt2("ff", GateType::BaseType::ff);
        EXPECT_EQ(gt2.get_id(), 2);
        EXPECT_EQ(gt2.get_name(), "ff");
        EXPECT_EQ(gt2.get_base_type(), GateType::BaseType::ff);

        // latch gate type
        GateType gt3("latch", GateType::BaseType::latch);
        EXPECT_EQ(gt3.get_id(), 3);
        EXPECT_EQ(gt3.get_name(), "latch");
        EXPECT_EQ(gt3.get_base_type(), GateType::BaseType::latch);

        // LUT gate type
        GateType gt4("lut", GateType::BaseType::lut);
        EXPECT_EQ(gt4.get_id(), 4);
        EXPECT_EQ(gt4.get_name(), "lut");
        EXPECT_EQ(gt4.get_base_type(), GateType::BaseType::lut);

        // RAM gate type
        GateType gt5("ram", GateType::BaseType::ram);
        EXPECT_EQ(gt5.get_id(), 5);
        EXPECT_EQ(gt5.get_name(), "ram");
        EXPECT_EQ(gt5.get_base_type(), GateType::BaseType::ram);

        TEST_END
    }

    /**
     * Testing operators.
     *
     * Functions: to_string, operator<<, operator==, operator!=
     */
    TEST_F(GateTypeTest, check_operators)
    {
        TEST_START

        std::stringstream ss;
        GateType gt1("dummy1", GateType::BaseType::combinational);
        GateType gt2("dummy2", GateType::BaseType::combinational);

        // gate type to_string
        EXPECT_EQ(gt1.to_string(), "dummy1");

        // gate type to ostream
        ss << gt1;
        EXPECT_EQ(ss.str(), "dummy1");
        ss.str(std::string());

        // base type to ostream
        ss << GateType::BaseType::combinational;
        EXPECT_EQ(ss.str(), "combinational");
        ss.str(std::string());
        ss << GateType::BaseType::ff;
        EXPECT_EQ(ss.str(), "ff");
        ss.str(std::string());
        ss << GateType::BaseType::latch;
        EXPECT_EQ(ss.str(), "latch");
        ss.str(std::string());
        ss << GateType::BaseType::lut;
        EXPECT_EQ(ss.str(), "lut");
        ss.str(std::string());

        // pin direction to ostream
        ss << GateType::PinDirection::input;
        EXPECT_EQ(ss.str(), "input");
        ss.str(std::string());
        ss << GateType::PinDirection::output;
        EXPECT_EQ(ss.str(), "output");
        ss.str(std::string());
        ss << GateType::PinDirection::inout;
        EXPECT_EQ(ss.str(), "inout");
        ss.str(std::string());
        ss << GateType::PinDirection::internal;
        EXPECT_EQ(ss.str(), "internal");
        ss.str(std::string());

        // pin type to ostream
        ss << GateType::PinType::none;
        EXPECT_EQ(ss.str(), "none");
        ss.str(std::string());
        ss << GateType::PinType::power;
        EXPECT_EQ(ss.str(), "power");
        ss.str(std::string());
        ss << GateType::PinType::ground;
        EXPECT_EQ(ss.str(), "ground");
        ss.str(std::string());
        ss << GateType::PinType::lut;
        EXPECT_EQ(ss.str(), "lut");
        ss.str(std::string());
        ss << GateType::PinType::state;
        EXPECT_EQ(ss.str(), "state");
        ss.str(std::string());
        ss << GateType::PinType::neg_state;
        EXPECT_EQ(ss.str(), "neg_state");
        ss.str(std::string());
        ss << GateType::PinType::clock;
        EXPECT_EQ(ss.str(), "clock");
        ss.str(std::string());
        ss << GateType::PinType::enable;
        EXPECT_EQ(ss.str(), "enable");
        ss.str(std::string());
        ss << GateType::PinType::set;
        EXPECT_EQ(ss.str(), "set");
        ss.str(std::string());
        ss << GateType::PinType::reset;
        EXPECT_EQ(ss.str(), "reset");
        ss.str(std::string());
        ss << GateType::PinType::data;
        EXPECT_EQ(ss.str(), "data");
        ss.str(std::string());
        ss << GateType::PinType::address;
        EXPECT_EQ(ss.str(), "address");
        ss.str(std::string());

        // comparison
        EXPECT_TRUE(gt1 == gt1);
        EXPECT_FALSE(gt1 == gt2);
        
        EXPECT_TRUE(gt1 != gt2);
        EXPECT_FALSE(gt1 != gt1);

        TEST_END
    }

    /**
     * Testing handling of input and output pins.
     *
     * Functions: add_input_pin, add_input_pins, get_input_pins, add_output_pin, add_output_pins, get_output_pins
     */
    TEST_F(GateTypeTest, check_pin_handling)
    {
        TEST_START

        GateType gt("dummy", GateType::BaseType::combinational);
        gt.add_input_pin("I0");
        gt.add_input_pins(std::vector<std::string>({"I1", "I2"}));
        EXPECT_EQ(gt.get_input_pins(), std::vector<std::string>({"I0", "I1", "I2"}));

        gt.add_output_pin("O0");
        gt.add_output_pins(std::vector<std::string>({"O1", "O2"}));
        EXPECT_EQ(gt.get_output_pins(), std::vector<std::string>({"O0", "O1", "O2"}));

        TEST_END
    }

    /**
     * Testing handling of input and output pin groups.
     *
     * Functions: assign_input_pin_group, assign_input_pin_groups, get_input_pin_groups, assign_output_pin_group, assign_output_pin_groups, get_output_pin_groups
     */
    TEST_F(GateTypeTest, check_pin_group_handling)
    {
        TEST_START

        std::map<u32, std::string> index_to_pin_a = {{0, "A(0)"}, {1, "A(1)"}};
        std::map<u32, std::string> index_to_pin_b = {{0, "B(0)"}, {1, "B(1)"}};
        std::map<u32, std::string> index_to_pin_c = {{0, "C(0)"}, {1, "C(1)"}};
        std::unordered_map<std::string, std::map<u32, std::string>> groups_a = {{"A", index_to_pin_a}};
        std::unordered_map<std::string, std::map<u32, std::string>> groups_ab = {{"A", index_to_pin_a}, {"B", index_to_pin_b}};
        std::unordered_map<std::string, std::map<u32, std::string>> groups_bc = {{"B", index_to_pin_b}, {"C", index_to_pin_c}};
        std::unordered_map<std::string, std::map<u32, std::string>> groups_abc = {{"A", index_to_pin_a}, {"B", index_to_pin_b}, {"C", index_to_pin_c}};
        
        // input groups
        {   
            GateType gt("dummy", GateType::BaseType::combinational);
            gt.add_input_pins({"A(0)", "A(1)", "B(0)", "B(1)", "C(0)", "C(1)"});
        
            // multiple groups
            gt.assign_input_pin_groups(groups_ab);
            EXPECT_EQ(gt.get_input_pin_groups(), groups_ab);

            // single group
            gt.assign_input_pin_group("C", index_to_pin_c);
            EXPECT_EQ(gt.get_input_pin_groups(), groups_abc);
        }

        // output groups
        {
            GateType gt("dummy", GateType::BaseType::combinational);
            gt.add_output_pins({"A(0)", "A(1)", "B(0)", "B(1)", "C(0)", "C(1)"});
        
            // multiple groups
            gt.assign_output_pin_groups(groups_ab);
            EXPECT_EQ(gt.get_output_pin_groups(), groups_ab);

            // single group
            gt.assign_output_pin_group("C", index_to_pin_c);
            EXPECT_EQ(gt.get_output_pin_groups(), groups_abc);
        }

        // add already existing input pin group
        {
            GateType gt("dummy", GateType::BaseType::combinational);

            gt.add_input_pins({"A(0)", "A(1)"});
            gt.assign_input_pin_group("A", index_to_pin_a);
            EXPECT_EQ(gt.get_input_pin_groups(), groups_a);
            gt.assign_input_pin_group("A", index_to_pin_a);
            EXPECT_EQ(gt.get_input_pin_groups(), groups_a);
            gt.assign_input_pin_groups(groups_a);
            EXPECT_EQ(gt.get_input_pin_groups(), groups_a);
        }

        // add already existing output pin group
        {
            GateType gt("dummy", GateType::BaseType::combinational);

            gt.add_output_pins({"A(0)", "A(1)"});
            gt.assign_output_pin_group("A", index_to_pin_a);
            EXPECT_EQ(gt.get_output_pin_groups(), groups_a);
            gt.assign_output_pin_group("A", index_to_pin_a);
            EXPECT_EQ(gt.get_output_pin_groups(), groups_a);
            gt.assign_output_pin_groups(groups_a);
            EXPECT_EQ(gt.get_output_pin_groups(), groups_a);
        }

        TEST_END
    }

    /**
     * Testing pin type handling.
     *
     * Functions: assign_pin_type, get_pin_type, get_pin_types, get_pins_of_type
     */
    TEST_F(GateTypeTest, check_pin_types)
    {
        TEST_START

        // combinational pin types
        {
            GateType gt("dummy", GateType::BaseType::combinational);

            std::vector<std::string> in_pins = {"I0", "I1"};
            std::vector<std::string> out_pins = {"O"};
            std::unordered_map<std::string, GateType::PinType> pin_to_type;

            gt.add_input_pins(in_pins);
            gt.add_output_pins(out_pins);

            EXPECT_EQ(gt.get_pin_type("I0"), GateType::PinType::none);
            EXPECT_EQ(gt.get_pin_type("I1"), GateType::PinType::none);
            EXPECT_EQ(gt.get_pin_type("O"), GateType::PinType::none);

            ASSERT_TRUE(gt.assign_pin_type("I0", GateType::PinType::power));
            ASSERT_TRUE(gt.assign_pin_type("I1", GateType::PinType::power));
            ASSERT_FALSE(gt.assign_pin_type("O", GateType::PinType::power));

            pin_to_type = {
                {"I0", GateType::PinType::power}, 
                {"I1", GateType::PinType::power}, 
                {"O", GateType::PinType::none}};

            EXPECT_EQ(gt.get_pin_types(), pin_to_type);
            EXPECT_EQ(gt.get_pins_of_type(GateType::PinType::power), in_pins);
            EXPECT_EQ(gt.get_pins_of_type(GateType::PinType::none), out_pins);
            EXPECT_EQ(gt.get_pin_type("I0"), GateType::PinType::power);
            EXPECT_EQ(gt.get_pin_type("I1"), GateType::PinType::power);
            EXPECT_EQ(gt.get_pin_type("O"), GateType::PinType::none);

            ASSERT_TRUE(gt.assign_pin_type("I0", GateType::PinType::ground));
            ASSERT_TRUE(gt.assign_pin_type("I1", GateType::PinType::ground));
            ASSERT_FALSE(gt.assign_pin_type("O", GateType::PinType::ground));

            pin_to_type = {
                {"I0", GateType::PinType::ground}, 
                {"I1", GateType::PinType::ground}, 
                {"O", GateType::PinType::none}};

            EXPECT_EQ(gt.get_pin_types(), pin_to_type);
            EXPECT_EQ(gt.get_pins_of_type(GateType::PinType::ground), in_pins);
            EXPECT_EQ(gt.get_pins_of_type(GateType::PinType::none), out_pins);
            EXPECT_EQ(gt.get_pin_type("I0"), GateType::PinType::ground);
            EXPECT_EQ(gt.get_pin_type("I1"), GateType::PinType::ground);
            EXPECT_EQ(gt.get_pin_type("O"), GateType::PinType::none);
        }

        // LUT pin types
        {
            GateType gt("dummy", GateType::BaseType::lut);

            std::unordered_map<std::string, GateType::PinType> pin_to_type;

            std::vector<std::string> in_pins = {"I"};
            std::vector<std::string> out_pins = {"O"};

            gt.add_input_pins(in_pins);
            gt.add_output_pins(out_pins);

            EXPECT_EQ(gt.get_pin_type("I"), GateType::PinType::none);
            EXPECT_EQ(gt.get_pin_type("O"), GateType::PinType::none);

            ASSERT_FALSE(gt.assign_pin_type("I", GateType::PinType::lut));
            ASSERT_TRUE(gt.assign_pin_type("O", GateType::PinType::lut));

            pin_to_type = {
                {"I", GateType::PinType::none},
                {"O", GateType::PinType::lut}};

            EXPECT_EQ(gt.get_pin_types(), pin_to_type);
            EXPECT_EQ(gt.get_pins_of_type(GateType::PinType::none), in_pins);
            EXPECT_EQ(gt.get_pins_of_type(GateType::PinType::lut), out_pins);
            EXPECT_EQ(gt.get_pin_type("I"), GateType::PinType::none);
            EXPECT_EQ(gt.get_pin_type("O"), GateType::PinType::lut);
        }

        // FF pin types
        {
            GateType gt("dummy", GateType::BaseType::ff);

            std::unordered_map<std::string, GateType::PinType> pin_to_type;

            std::vector<std::string> in_pins = {"I0", "I1", "I2", "I3", "I4", "I5", "I6"};
            std::vector<std::string> out_pins = {"O0", "O1", "O2", "O3", "O4"};

            gt.add_input_pins(in_pins);
            gt.add_output_pins(out_pins);

            for (const auto& pin : in_pins) 
            {
                EXPECT_EQ(gt.get_pin_type(pin), GateType::PinType::none);
            }
            for (const auto& pin : out_pins) 
            {
                EXPECT_EQ(gt.get_pin_type(pin), GateType::PinType::none);
            }

            ASSERT_TRUE(gt.assign_pin_type("I0", GateType::PinType::clock));
            ASSERT_TRUE(gt.assign_pin_type("I1", GateType::PinType::enable));
            ASSERT_TRUE(gt.assign_pin_type("I2", GateType::PinType::set));
            ASSERT_TRUE(gt.assign_pin_type("I3", GateType::PinType::reset));
            ASSERT_TRUE(gt.assign_pin_type("I4", GateType::PinType::data));
            ASSERT_TRUE(gt.assign_pin_type("I5", GateType::PinType::address));
            ASSERT_FALSE(gt.assign_pin_type("I6", GateType::PinType::state));
            ASSERT_FALSE(gt.assign_pin_type("I6", GateType::PinType::neg_state));
            ASSERT_TRUE(gt.assign_pin_type("O0", GateType::PinType::state));
            ASSERT_TRUE(gt.assign_pin_type("O1", GateType::PinType::neg_state));
            ASSERT_FALSE(gt.assign_pin_type("O4", GateType::PinType::clock));
            ASSERT_FALSE(gt.assign_pin_type("O4", GateType::PinType::enable));
            ASSERT_FALSE(gt.assign_pin_type("O4", GateType::PinType::set));
            ASSERT_FALSE(gt.assign_pin_type("O4", GateType::PinType::reset));
            ASSERT_TRUE(gt.assign_pin_type("O2", GateType::PinType::data));
            ASSERT_TRUE(gt.assign_pin_type("O3", GateType::PinType::address));

            pin_to_type = {
                {"I0", GateType::PinType::clock},
                {"I1", GateType::PinType::enable},
                {"I2", GateType::PinType::set},
                {"I3", GateType::PinType::reset},
                {"I4", GateType::PinType::data},
                {"I5", GateType::PinType::address},
                {"I6", GateType::PinType::none},
                {"O0", GateType::PinType::state},
                {"O1", GateType::PinType::neg_state},
                {"O2", GateType::PinType::data},
                {"O3", GateType::PinType::address},
                {"O4", GateType::PinType::none}};

            EXPECT_EQ(gt.get_pin_types(), pin_to_type);
        }
    
        TEST_END
    }



    /**
     * Testing Boolean function handling.
     *
     * Functions: add_boolean_function, add_boolean_functions, get_boolean_functions
     */
    TEST_F(GateTypeTest, check_boolean_functions)
    {
        TEST_START
        
        // single function
        {
            GateType gt("dummy", GateType::BaseType::combinational);

            BooleanFunction bf1 = BooleanFunction::ONE;
            gt.add_boolean_function("bf1", bf1);

            std::unordered_map<std::string, BooleanFunction> ret_map = gt.get_boolean_functions();
            EXPECT_EQ(ret_map.size(), 1);
            ASSERT_FALSE(ret_map.find("bf1") == ret_map.end());
            EXPECT_EQ(ret_map.at("bf1"), bf1);
        }

        // multiple functions
        {
            GateType gt("dummy", GateType::BaseType::combinational);
        
            std::unordered_map<std::string, BooleanFunction> func_map = {
                {"bf1", BooleanFunction::ONE},
                {"bf2", BooleanFunction::ONE}};
            gt.add_boolean_functions(func_map);

            std::unordered_map<std::string, BooleanFunction> ret_map = gt.get_boolean_functions();
            EXPECT_EQ(func_map, ret_map);
        }
        
        TEST_END
    }

    /**
     * Testing config data category and identifier.
     *
     * Functions: set_config_data_category, get_config_data_category, set_config_data_identifier, get_config_data_identifier
     */
    TEST_F(GateTypeTest, check_config_data)
    {
        TEST_START
        
        GateType gt("dummy", GateType::BaseType::lut);

        // category
        gt.set_config_data_category("category1");
        EXPECT_EQ(gt.get_config_data_category(), "category1");
        gt.set_config_data_category("category2");
        EXPECT_EQ(gt.get_config_data_category(), "category2");

        // identifier
        gt.set_config_data_identifier("identifier1");
        EXPECT_EQ(gt.get_config_data_identifier(), "identifier1");
        gt.set_config_data_identifier("identifier2");
        EXPECT_EQ(gt.get_config_data_identifier(), "identifier2");

        TEST_END
    }

    /**
     * Testing LUT initialization string order.
     *
     * Functions: set_lut_init_ascending, is_lut_init_ascending
     */
    TEST_F(GateTypeTest, check_init_order)
    {
        TEST_START
        
        GateType gt("dummy", GateType::BaseType::lut);

        gt.set_lut_init_ascending(true);
        EXPECT_TRUE(gt.is_lut_init_ascending());

        gt.set_lut_init_ascending(false);
        EXPECT_FALSE(gt.is_lut_init_ascending());

        TEST_END
    }

    /**
     * Testing clear-preset behavior handling.
     *
     * Functions: set_clear_preset_behavior, get_clear_preset_behavior
     */
    TEST_F(GateTypeTest, check_clear_preset)
    {
        TEST_START

        GateType gt("dummy", GateType::BaseType::ff);

        EXPECT_EQ(gt.get_clear_preset_behavior(), std::make_pair(GateType::ClearPresetBehavior::U, GateType::ClearPresetBehavior::U));
        gt.set_clear_preset_behavior(GateType::ClearPresetBehavior::L, GateType::ClearPresetBehavior::H);
        EXPECT_EQ(gt.get_clear_preset_behavior(), std::make_pair(GateType::ClearPresetBehavior::L, GateType::ClearPresetBehavior::H));
        
        TEST_END
    }
}    //namespace hal
