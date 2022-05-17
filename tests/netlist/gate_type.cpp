#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
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

        GateLibrary gl("no_path", "example_gl");

        // combinational gate type
        {
            GateType* gt = gl.create_gate_type("combinational", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
            EXPECT_EQ(gt->get_id(), 1);
            EXPECT_EQ(gt->get_name(), "combinational");
            EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));
            EXPECT_TRUE(gt->has_property(GateTypeProperty::combinational));
            EXPECT_FALSE(gt->has_property(GateTypeProperty::ff));
            EXPECT_EQ(gt->get_gate_library(), &gl);
        }

        // FF gate type
        {
            GateType* gt = gl.create_gate_type("ff", {GateTypeProperty::ff});
            ASSERT_NE(gt, nullptr);
            EXPECT_EQ(gt->get_id(), 2);
            EXPECT_EQ(gt->get_name(), "ff");
            EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::ff}));
            EXPECT_TRUE(gt->has_property(GateTypeProperty::ff));
            EXPECT_FALSE(gt->has_property(GateTypeProperty::combinational));
            EXPECT_EQ(gt->get_gate_library(), &gl);
        }

        // latch gate type
        {
            GateType* gt = gl.create_gate_type("latch", {GateTypeProperty::latch});
            ASSERT_NE(gt, nullptr);
            EXPECT_EQ(gt->get_id(), 3);
            EXPECT_EQ(gt->get_name(), "latch");
            EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::latch}));
            EXPECT_TRUE(gt->has_property(GateTypeProperty::latch));
            EXPECT_FALSE(gt->has_property(GateTypeProperty::ff));
            EXPECT_EQ(gt->get_gate_library(), &gl);
        }

        // LUT gate type
        {
            GateType* gt = gl.create_gate_type("lut", {GateTypeProperty::lut});
            ASSERT_NE(gt, nullptr);
            EXPECT_EQ(gt->get_id(), 4);
            EXPECT_EQ(gt->get_name(), "lut");
            EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::lut}));
            EXPECT_TRUE(gt->has_property(GateTypeProperty::lut));
            EXPECT_FALSE(gt->has_property(GateTypeProperty::ff));
            EXPECT_EQ(gt->get_gate_library(), &gl);
        }

        // RAM gate type
        {
            GateType* gt = gl.create_gate_type("ram", {GateTypeProperty::ram});
            ASSERT_NE(gt, nullptr);
            EXPECT_EQ(gt->get_id(), 5);
            EXPECT_EQ(gt->get_name(), "ram");
            EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::ram}));
            EXPECT_TRUE(gt->has_property(GateTypeProperty::ram));
            EXPECT_FALSE(gt->has_property(GateTypeProperty::combinational));
            EXPECT_EQ(gt->get_gate_library(), &gl);
        }

        // IO gate type
        {
            GateType* gt = gl.create_gate_type("io", {GateTypeProperty::io});
            ASSERT_NE(gt, nullptr);
            EXPECT_EQ(gt->get_id(), 6);
            EXPECT_EQ(gt->get_name(), "io");
            EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::io}));
            EXPECT_TRUE(gt->has_property(GateTypeProperty::io));
            EXPECT_FALSE(gt->has_property(GateTypeProperty::combinational));
            EXPECT_EQ(gt->get_gate_library(), &gl);
        }

        // DSP gate type
        {
            GateType* gt = gl.create_gate_type("dsp", {GateTypeProperty::dsp});
            ASSERT_NE(gt, nullptr);
            EXPECT_EQ(gt->get_id(), 7);
            EXPECT_EQ(gt->get_name(), "dsp");
            EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::dsp}));
            EXPECT_TRUE(gt->has_property(GateTypeProperty::dsp));
            EXPECT_FALSE(gt->has_property(GateTypeProperty::combinational));
            EXPECT_EQ(gt->get_gate_library(), &gl);
        }

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
        GateLibrary gl("no_path", "example_gl");
        GateType* gt1 = gl.create_gate_type("dummy1", {GateTypeProperty::combinational});
        GateType* gt2 = gl.create_gate_type("dummy2", {GateTypeProperty::combinational});

        ASSERT_NE(gt1, nullptr);
        ASSERT_NE(gt2, nullptr);

        // gate type to_string
        EXPECT_EQ(gt1->to_string(), "dummy1");

        // gate type to ostream
        ss << *gt1;
        EXPECT_EQ(ss.str(), "dummy1");
        ss.str(std::string());

        // base type to ostream
        ss << GateTypeProperty::combinational;
        EXPECT_EQ(ss.str(), "combinational");
        ss.str(std::string());
        ss << GateTypeProperty::ff;
        EXPECT_EQ(ss.str(), "ff");
        ss.str(std::string());
        ss << GateTypeProperty::latch;
        EXPECT_EQ(ss.str(), "latch");
        ss.str(std::string());
        ss << GateTypeProperty::lut;
        EXPECT_EQ(ss.str(), "lut");
        ss.str(std::string());

        // pin direction to ostream
        ss << PinDirection::input;
        EXPECT_EQ(ss.str(), "input");
        ss.str(std::string());
        ss << PinDirection::output;
        EXPECT_EQ(ss.str(), "output");
        ss.str(std::string());
        ss << PinDirection::inout;
        EXPECT_EQ(ss.str(), "inout");
        ss.str(std::string());
        ss << PinDirection::internal;
        EXPECT_EQ(ss.str(), "internal");
        ss.str(std::string());

        // pin type to ostream
        ss << PinType::none;
        EXPECT_EQ(ss.str(), "none");
        ss.str(std::string());
        ss << PinType::power;
        EXPECT_EQ(ss.str(), "power");
        ss.str(std::string());
        ss << PinType::ground;
        EXPECT_EQ(ss.str(), "ground");
        ss.str(std::string());
        ss << PinType::lut;
        EXPECT_EQ(ss.str(), "lut");
        ss.str(std::string());
        ss << PinType::state;
        EXPECT_EQ(ss.str(), "state");
        ss.str(std::string());
        ss << PinType::neg_state;
        EXPECT_EQ(ss.str(), "neg_state");
        ss.str(std::string());
        ss << PinType::clock;
        EXPECT_EQ(ss.str(), "clock");
        ss.str(std::string());
        ss << PinType::enable;
        EXPECT_EQ(ss.str(), "enable");
        ss.str(std::string());
        ss << PinType::set;
        EXPECT_EQ(ss.str(), "set");
        ss.str(std::string());
        ss << PinType::reset;
        EXPECT_EQ(ss.str(), "reset");
        ss.str(std::string());
        ss << PinType::data;
        EXPECT_EQ(ss.str(), "data");
        ss.str(std::string());
        ss << PinType::address;
        EXPECT_EQ(ss.str(), "address");
        ss.str(std::string());

        // comparison
        EXPECT_TRUE(*gt1 == *gt1);
        EXPECT_FALSE(*gt1 == *gt2);
        
        EXPECT_TRUE(*gt1 != *gt2);
        EXPECT_FALSE(*gt1 != *gt1);

        TEST_END
    }

    /**
     * Testing handling of input and output pins.
     *
     * Functions: add_pin, add_pins, get_input_pins, get_output_pins
     */
    TEST_F(GateTypeTest, check_pin_handling)
    {
        TEST_START

        GateLibrary gl("no_path", "example_gl");

        // input
        {
            GateType* gt = gl.create_gate_type("dummy1", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
            EXPECT_TRUE(gt->add_pin("I0", PinDirection::input));
            EXPECT_TRUE(gt->add_pins({"I1", "I2"}, PinDirection::input));
            EXPECT_EQ(gt->get_pins(), std::vector<std::string>({"I0", "I1", "I2"}));
            EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"I0", "I1", "I2"}));
            EXPECT_TRUE(gt->get_output_pins().empty());
            EXPECT_EQ(gt->get_pins_of_direction(PinDirection::input), std::unordered_set<std::string>({"I0", "I1", "I2"}));
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::output).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::inout).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::internal).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::none).empty());
            EXPECT_EQ(gt->get_pin_direction("I0"), PinDirection::input);
            EXPECT_EQ(gt->get_pin_direction("I1"), PinDirection::input);
            EXPECT_EQ(gt->get_pin_direction("I2"), PinDirection::input);
            std::unordered_map<std::string, PinDirection> direction_map = {
                {"I0", PinDirection::input},
                {"I1", PinDirection::input},
                {"I2", PinDirection::input}};
            EXPECT_EQ(gt->get_pin_directions(), direction_map);
        }

        // output
        {
            GateType* gt = gl.create_gate_type("dummy2", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
            EXPECT_TRUE(gt->add_pin("O0", PinDirection::output));
            EXPECT_TRUE(gt->add_pins({"O1", "O2"}, PinDirection::output));
            EXPECT_EQ(gt->get_pins(), std::vector<std::string>({"O0", "O1", "O2"}));
            EXPECT_TRUE(gt->get_input_pins().empty());
            EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"O0", "O1", "O2"}));
            EXPECT_EQ(gt->get_pins_of_direction(PinDirection::output), std::unordered_set<std::string>({"O0", "O1", "O2"}));
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::input).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::inout).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::internal).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::none).empty());
            EXPECT_EQ(gt->get_pin_direction("O0"), PinDirection::output);
            EXPECT_EQ(gt->get_pin_direction("O1"), PinDirection::output);
            EXPECT_EQ(gt->get_pin_direction("O2"), PinDirection::output);
            std::unordered_map<std::string, PinDirection> direction_map = {
                {"O0", PinDirection::output},
                {"O1", PinDirection::output},
                {"O2", PinDirection::output}};
            EXPECT_EQ(gt->get_pin_directions(), direction_map);
        }

        // inout
        {
            GateType* gt = gl.create_gate_type("dummy3", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
            EXPECT_TRUE(gt->add_pin("IO0", PinDirection::inout));
            EXPECT_TRUE(gt->add_pins({"IO1", "IO2"}, PinDirection::inout));
            EXPECT_EQ(gt->get_pins(), std::vector<std::string>({"IO0", "IO1", "IO2"}));
            EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"IO0", "IO1", "IO2"}));
            EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"IO0", "IO1", "IO2"}));
            EXPECT_EQ(gt->get_pins_of_direction(PinDirection::inout), std::unordered_set<std::string>({"IO0", "IO1", "IO2"}));
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::input).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::output).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::internal).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::none).empty());
            EXPECT_EQ(gt->get_pin_direction("IO0"), PinDirection::inout);
            EXPECT_EQ(gt->get_pin_direction("IO1"), PinDirection::inout);
            EXPECT_EQ(gt->get_pin_direction("IO2"), PinDirection::inout);
            std::unordered_map<std::string, PinDirection> direction_map = {
                {"IO0", PinDirection::inout},
                {"IO1", PinDirection::inout},
                {"IO2", PinDirection::inout}};
            EXPECT_EQ(gt->get_pin_directions(), direction_map);
        }

        // internal
        {
            GateType* gt = gl.create_gate_type("dummy4", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
            EXPECT_TRUE(gt->add_pin("INT0", PinDirection::internal));
            EXPECT_TRUE(gt->add_pins({"INT1", "INT2"}, PinDirection::internal));
            EXPECT_EQ(gt->get_pins(), std::vector<std::string>({"INT0", "INT1", "INT2"}));
            EXPECT_TRUE(gt->get_input_pins().empty());
            EXPECT_TRUE(gt->get_output_pins().empty());
            EXPECT_EQ(gt->get_pins_of_direction(PinDirection::internal), std::unordered_set<std::string>({"INT0", "INT1", "INT2"}));
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::input).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::output).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::inout).empty());
            EXPECT_TRUE(gt->get_pins_of_direction(PinDirection::none).empty());
            EXPECT_EQ(gt->get_pin_direction("INT0"), PinDirection::internal);
            EXPECT_EQ(gt->get_pin_direction("INT1"), PinDirection::internal);
            EXPECT_EQ(gt->get_pin_direction("INT2"), PinDirection::internal);
            std::unordered_map<std::string, PinDirection> direction_map = {
                {"INT0", PinDirection::internal},
                {"INT1", PinDirection::internal},
                {"INT2", PinDirection::internal}};
            EXPECT_EQ(gt->get_pin_directions(), direction_map);
        }

        // existing pin
        {
            GateType* gt = gl.create_gate_type("dummy5", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
            EXPECT_TRUE(gt->add_pin("A", PinDirection::input));
            EXPECT_EQ(gt->get_pins(), std::vector<std::string>({"A"}));
            EXPECT_FALSE(gt->add_pin("A", PinDirection::input));
            EXPECT_EQ(gt->get_pins(), std::vector<std::string>({"A"}));
            EXPECT_FALSE(gt->add_pin("A", PinDirection::output));
            EXPECT_EQ(gt->get_pins(), std::vector<std::string>({"A"}));
            EXPECT_EQ(gt->get_pin_direction("A"), PinDirection::input);
        }

        TEST_END
    }

    /**
     * Testing handling of input and output pin groups.
     *
     * Functions: assign_pin_group, get_pin_groups
     */
    TEST_F(GateTypeTest, check_pin_group_handling)
    {
        TEST_START

        GateLibrary gl("no_path", "example_gl");

        std::vector<std::pair<u32, std::string>> index_to_pin_a = {{0, "A(0)"}, {1, "A(1)"}};
        std::vector<std::pair<u32, std::string>> index_to_pin_b = {{0, "B(0)"}, {1, "B(1)"}};
        std::vector<std::pair<u32, std::string>> index_to_pin_c = {{0, "C(0)"}, {1, "C(1)"}};
        std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> groups_a = {{"A", index_to_pin_a}};
        std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> groups_ab = {{"A", index_to_pin_a}, {"B", index_to_pin_b}};
        std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> groups_bc = {{"B", index_to_pin_b}, {"C", index_to_pin_c}};
        std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> groups_abc = {{"A", index_to_pin_a}, {"B", index_to_pin_b}, {"C", index_to_pin_c}};
        
        // pin groups
        {   
            GateType* gt = gl.create_gate_type("dummy1", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
            EXPECT_TRUE(gt->add_pins({"A(0)", "A(1)", "B(0)", "B(1)", "C(0)", "C(1)"}, PinDirection::input));
        
            EXPECT_TRUE(gt->assign_pin_group("A", index_to_pin_a));
            EXPECT_TRUE(gt->assign_pin_group("B", index_to_pin_b));
            EXPECT_EQ(gt->get_pin_groups(), groups_ab);
            EXPECT_EQ(gt->get_pins_of_group("A"), index_to_pin_a);
            EXPECT_EQ(gt->get_pin_of_group_at_index("A", 0), "A(0)");
            EXPECT_EQ(gt->get_index_in_group_of_pin("A", "A(0)"), 0);
            EXPECT_EQ(gt->get_pin_of_group_at_index("B", 1), "B(1)");
            EXPECT_EQ(gt->get_index_in_group_of_pin("B", "B(1)"), 1);
        }

        // add already existing pin group
        {
            GateType* gt = gl.create_gate_type("dummy2", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);

            EXPECT_TRUE(gt->add_pins({"A(0)", "A(1)"}, PinDirection::input));
            EXPECT_TRUE(gt->assign_pin_group("A", index_to_pin_a));
            EXPECT_EQ(gt->get_pin_groups(), groups_a);
            EXPECT_FALSE(gt->assign_pin_group("A", index_to_pin_a));
            EXPECT_EQ(gt->get_pin_groups(), groups_a);
        }

        // add invalid pin to pin group
        {
            GateType* gt = gl.create_gate_type("dummy3", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);

            EXPECT_FALSE(gt->assign_pin_group("A", index_to_pin_a));
            EXPECT_TRUE(gt->get_pin_groups().empty());
        }

        // querry invalid groups or indices
        {
            GateType* gt = gl.create_gate_type("dummy4", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);

            EXPECT_TRUE(gt->add_pins({"A(0)", "A(1)"}, PinDirection::input));
            EXPECT_TRUE(gt->assign_pin_group("A", index_to_pin_a));
            EXPECT_TRUE(gt->get_pins_of_group("B").empty());
            EXPECT_EQ(gt->get_pin_of_group_at_index("B", 0), "");
            EXPECT_EQ(gt->get_index_in_group_of_pin("B", "B(0)"), -1);
            EXPECT_EQ(gt->get_pin_of_group_at_index("A", 2), "");
            EXPECT_EQ(gt->get_index_in_group_of_pin("A", "A(2)"), -1);
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

        GateLibrary gl("no_path", "example_gl");

        // combinational pin types
        {
            GateType* gt = gl.create_gate_type("dummy1", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);

            std::unordered_set<std::string> pins = {"I0", "I1", "O"};
            std::unordered_map<std::string, PinType> pin_to_type;

            EXPECT_TRUE(gt->add_pins({"I0", "I1"}, PinDirection::input, PinType::ground));
            EXPECT_TRUE(gt->add_pins({"O"}, PinDirection::output));

            EXPECT_EQ(gt->get_pin_type("I0"), PinType::ground);
            EXPECT_EQ(gt->get_pin_type("I1"), PinType::ground);
            EXPECT_EQ(gt->get_pin_type("O"), PinType::none);

            ASSERT_TRUE(gt->assign_pin_type("I0", PinType::power));
            ASSERT_TRUE(gt->assign_pin_type("I1", PinType::power));
            ASSERT_TRUE(gt->assign_pin_type("O", PinType::power));

            pin_to_type = {
                {"I0", PinType::power}, 
                {"I1", PinType::power}, 
                {"O", PinType::power}};

            EXPECT_EQ(gt->get_pin_types(), pin_to_type);
            EXPECT_EQ(gt->get_pins_of_type(PinType::power), pins);
            EXPECT_EQ(gt->get_pin_type("I0"), PinType::power);
            EXPECT_EQ(gt->get_pin_type("I1"), PinType::power);
            EXPECT_EQ(gt->get_pin_type("O"), PinType::power);

            ASSERT_TRUE(gt->assign_pin_type("I0", PinType::ground));
            ASSERT_TRUE(gt->assign_pin_type("I1", PinType::ground));
            ASSERT_TRUE(gt->assign_pin_type("O", PinType::ground));

            pin_to_type = {
                {"I0", PinType::ground}, 
                {"I1", PinType::ground}, 
                {"O", PinType::ground}};

            EXPECT_EQ(gt->get_pin_types(), pin_to_type);
            EXPECT_EQ(gt->get_pins_of_type(PinType::ground), pins);
            EXPECT_EQ(gt->get_pin_type("I0"), PinType::ground);
            EXPECT_EQ(gt->get_pin_type("I1"), PinType::ground);
            EXPECT_EQ(gt->get_pin_type("O"), PinType::ground);
        }

        // LUT pin types
        {
            GateType* gt = gl.create_gate_type("dummy2", {GateTypeProperty::lut});
            ASSERT_NE(gt, nullptr);

            std::unordered_map<std::string, PinType> pin_to_type;

            std::unordered_set<std::string> pins = {"I", "O"};

            EXPECT_TRUE(gt->add_pins({"I"}, PinDirection::input));
            EXPECT_TRUE(gt->add_pins({"O"}, PinDirection::output));

            EXPECT_EQ(gt->get_pin_type("I"), PinType::none);
            EXPECT_EQ(gt->get_pin_type("O"), PinType::none);

            ASSERT_TRUE(gt->assign_pin_type("I", PinType::lut));
            ASSERT_TRUE(gt->assign_pin_type("O", PinType::lut));

            pin_to_type = {
                {"I", PinType::lut},
                {"O", PinType::lut}};

            EXPECT_EQ(gt->get_pin_types(), pin_to_type);
            EXPECT_EQ(gt->get_pins_of_type(PinType::lut), pins);
            EXPECT_EQ(gt->get_pin_type("I"), PinType::lut);
            EXPECT_EQ(gt->get_pin_type("O"), PinType::lut);
        }

        // FF pin types
        {
            GateType* gt = gl.create_gate_type("dummy3", {GateTypeProperty::ff});
            ASSERT_NE(gt, nullptr);

            std::unordered_map<std::string, PinType> pin_to_type;

            std::unordered_set<std::string> in_pins = {"I0", "I1", "I2", "I3", "I4", "I5", "I6"};
            std::unordered_set<std::string> out_pins = {"O0", "O1", "O2", "O3", "O4"};

            EXPECT_TRUE(gt->add_pins({"I0", "I1", "I2", "I3", "I4", "I5", "I6"}, PinDirection::input));
            EXPECT_TRUE(gt->add_pins({"O0", "O1", "O2", "O3", "O4", "O5"}, PinDirection::output));

            for (const auto& pin : in_pins) 
            {
                EXPECT_EQ(gt->get_pin_type(pin), PinType::none);
            }
            for (const auto& pin : out_pins) 
            {
                EXPECT_EQ(gt->get_pin_type(pin), PinType::none);
            }

            ASSERT_TRUE(gt->assign_pin_type("I0", PinType::clock));
            ASSERT_TRUE(gt->assign_pin_type("I1", PinType::enable));
            ASSERT_TRUE(gt->assign_pin_type("I2", PinType::set));
            ASSERT_TRUE(gt->assign_pin_type("I3", PinType::reset));
            ASSERT_TRUE(gt->assign_pin_type("I4", PinType::data));
            ASSERT_TRUE(gt->assign_pin_type("I5", PinType::address));
            ASSERT_TRUE(gt->assign_pin_type("I6", PinType::state));
            ASSERT_TRUE(gt->assign_pin_type("I6", PinType::neg_state));
            ASSERT_TRUE(gt->assign_pin_type("O0", PinType::state));
            ASSERT_TRUE(gt->assign_pin_type("O1", PinType::neg_state));
            ASSERT_TRUE(gt->assign_pin_type("O2", PinType::data));
            ASSERT_TRUE(gt->assign_pin_type("O3", PinType::address));
            ASSERT_TRUE(gt->assign_pin_type("O4", PinType::clock));
            ASSERT_TRUE(gt->assign_pin_type("O5", PinType::enable));
            ASSERT_TRUE(gt->assign_pin_type("O5", PinType::set));
            ASSERT_TRUE(gt->assign_pin_type("O5", PinType::reset));
            

            pin_to_type = {
                {"I0", PinType::clock},
                {"I1", PinType::enable},
                {"I2", PinType::set},
                {"I3", PinType::reset},
                {"I4", PinType::data},
                {"I5", PinType::address},
                {"I6", PinType::neg_state},
                {"O0", PinType::state},
                {"O1", PinType::neg_state},
                {"O2", PinType::data},
                {"O3", PinType::address},
                {"O4", PinType::clock},
                {"O5", PinType::reset}};

            EXPECT_EQ(gt->get_pin_types(), pin_to_type);
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

        GateLibrary gl("no_path", "example_gl");
        
        // single function
        {
            GateType* gt = gl.create_gate_type("dummy1", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);

            BooleanFunction bf1 = BooleanFunction::Const(BooleanFunction::Value::ONE);
            gt->add_boolean_function("bf1", bf1);

            std::unordered_map<std::string, BooleanFunction> ret_map = gt->get_boolean_functions();
            EXPECT_EQ(ret_map.size(), 1);
            ASSERT_FALSE(ret_map.find("bf1") == ret_map.end());
            EXPECT_EQ(ret_map.at("bf1"), bf1);
        }

        // multiple functions
        {
            GateType* gt = gl.create_gate_type("dummy2", {GateTypeProperty::combinational});
            ASSERT_NE(gt, nullptr);
        
            std::unordered_map<std::string, BooleanFunction> func_map = {
                {"bf1",  BooleanFunction::Const(BooleanFunction::Value::ONE)},
                {"bf2",  BooleanFunction::Const(BooleanFunction::Value::ONE)}};
            gt->add_boolean_functions(func_map);

            std::unordered_map<std::string, BooleanFunction> ret_map = gt->get_boolean_functions();
            EXPECT_EQ(func_map, ret_map);
        }
        
        TEST_END
    }

    /**
     * Testing LUT gate type component.
     */
    TEST_F(GateTypeTest, check_lut_gt)
    {
        TEST_START

        GateLibrary gl("no_path", "example_gl");
        
        GateType* gt = gl.create_gate_type("dummy", {GateTypeProperty::lut}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("category1", {"identifier1"}), true));
        ASSERT_NE(gt, nullptr);

        LUTComponent* lut_component = gt->get_component_as<LUTComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::lut; });
        ASSERT_NE(lut_component, nullptr);
        InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::init; });
        ASSERT_NE(init_component, nullptr);
        EXPECT_EQ(lut_component->is_init_ascending(), true);
        EXPECT_EQ(init_component->get_init_category(), "category1");
        EXPECT_EQ(init_component->get_init_identifiers(), std::vector<std::string>({"identifier1"}));
        lut_component->set_init_ascending(false);
        init_component->set_init_category("category2");
        init_component->set_init_identifiers({"identifier2"});
        EXPECT_EQ(lut_component->is_init_ascending(), false);
        EXPECT_EQ(init_component->get_init_category(), "category2");
        EXPECT_EQ(init_component->get_init_identifiers(), std::vector<std::string>({"identifier2"}));
        
        TEST_END
    }

    /**
     * Testing FF gate type component.
     */
    TEST_F(GateTypeTest, check_ff_gt)
    {
        TEST_START
        {
            GateLibrary gl("no_path", "example_gl");

            GateType* gt = gl.create_gate_type("dummy", {GateTypeProperty::ff}, GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"), BooleanFunction::Var("D"), BooleanFunction::Var("C")));
            ASSERT_NE(gt, nullptr);
            FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* component){ return component->get_type() ==  GateTypeComponent::ComponentType::ff; });
            ASSERT_NE(ff_component, nullptr);
            StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c){ return StateComponent::is_class_of(c); });
            ASSERT_NE(state_component, nullptr);
            InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::init; });
            EXPECT_EQ(init_component, nullptr);

            EXPECT_EQ(ff_component->get_clock_function(), BooleanFunction::Var("C"));
            EXPECT_EQ(ff_component->get_next_state_function(), BooleanFunction::Var("D"));
            EXPECT_TRUE(ff_component->get_async_reset_function().is_empty());
            EXPECT_TRUE(ff_component->get_async_set_function().is_empty());
            EXPECT_EQ(ff_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::undef, AsyncSetResetBehavior::undef));
            EXPECT_EQ(state_component->get_state_identifier(), "IQ");
            EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");

            ff_component->set_clock_function(BooleanFunction::Var("CLK"));
            ff_component->set_next_state_function(BooleanFunction::Var("DIN"));
            ff_component->set_async_reset_function(BooleanFunction::Var("RST"));
            ff_component->set_async_set_function(BooleanFunction::Var("SET"));
            ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L);
            state_component->set_state_identifier("IIQ");
            state_component->set_neg_state_identifier("IIQN");

            EXPECT_EQ(ff_component->get_clock_function(), BooleanFunction::Var("CLK"));
            EXPECT_EQ(ff_component->get_next_state_function(), BooleanFunction::Var("DIN"));
            EXPECT_EQ(ff_component->get_async_reset_function(), BooleanFunction::Var("RST"));
            EXPECT_EQ(ff_component->get_async_set_function(), BooleanFunction::Var("SET"));
            EXPECT_EQ(ff_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L));
            EXPECT_EQ(state_component->get_state_identifier(), "IIQ");
            EXPECT_EQ(state_component->get_neg_state_identifier(), "IIQN");
        }
        {
            GateLibrary gl("no_path", "example_gl");

            GateType* gt = gl.create_gate_type("dummy", {GateTypeProperty::ff}, GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("category1", {"identifier1"}), "IQ", "IQN"), BooleanFunction::Var("D"), BooleanFunction::Var("C")));
            ASSERT_NE(gt, nullptr);
            FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c){ return FFComponent::is_class_of(c); });
            ASSERT_NE(ff_component, nullptr);
            StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c){ return StateComponent::is_class_of(c); });
            ASSERT_NE(state_component, nullptr);
            InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* c){ return InitComponent::is_class_of(c); });
            ASSERT_NE(init_component, nullptr);

            EXPECT_EQ(ff_component->get_clock_function(), BooleanFunction::Var("C"));
            EXPECT_EQ(ff_component->get_next_state_function(), BooleanFunction::Var("D"));
            EXPECT_TRUE(ff_component->get_async_reset_function().is_empty());
            EXPECT_TRUE(ff_component->get_async_set_function().is_empty());
            EXPECT_EQ(ff_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::undef, AsyncSetResetBehavior::undef));
            EXPECT_EQ(state_component->get_state_identifier(), "IQ");
            EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");
            EXPECT_EQ(init_component->get_init_category(), "category1");
            EXPECT_EQ(init_component->get_init_identifiers(), std::vector<std::string>({"identifier1"}));

            ff_component->set_clock_function(BooleanFunction::Var("CLK"));
            ff_component->set_next_state_function(BooleanFunction::Var("DIN"));
            ff_component->set_async_reset_function(BooleanFunction::Var("RST"));
            ff_component->set_async_set_function(BooleanFunction::Var("SET"));
            ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L);
            state_component->set_state_identifier("IIQ");
            state_component->set_neg_state_identifier("IIQN");
            init_component->set_init_category("category2");
            init_component->set_init_identifiers({"identifier2"});

            EXPECT_EQ(ff_component->get_clock_function(), BooleanFunction::Var("CLK"));
            EXPECT_EQ(ff_component->get_next_state_function(), BooleanFunction::Var("DIN"));
            EXPECT_EQ(ff_component->get_async_reset_function(), BooleanFunction::Var("RST"));
            EXPECT_EQ(ff_component->get_async_set_function(), BooleanFunction::Var("SET"));
            EXPECT_EQ(ff_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L));
            EXPECT_EQ(state_component->get_state_identifier(), "IIQ");
            EXPECT_EQ(state_component->get_neg_state_identifier(), "IIQN");
            EXPECT_EQ(init_component->get_init_category(), "category2");
            EXPECT_EQ(init_component->get_init_identifiers(), std::vector<std::string>({"identifier2"}));
        }
        TEST_END
    }

    /**
     * Testing Latch gate type component.
     */
    TEST_F(GateTypeTest, check_latch_gt)
    {
        TEST_START
    
        GateLibrary gl("no_path", "example_gl");

        GateType* gt = gl.create_gate_type("dummy", {GateTypeProperty::latch}, GateTypeComponent::create_latch_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN")));
        ASSERT_NE(gt, nullptr);
        LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* component){ return component->get_type() ==  GateTypeComponent::ComponentType::latch; });
        ASSERT_NE(latch_component, nullptr);
        StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c){ return StateComponent::is_class_of(c); });
        ASSERT_NE(state_component, nullptr);

        EXPECT_TRUE(latch_component->get_enable_function().is_empty());
        EXPECT_TRUE(latch_component->get_data_in_function().is_empty());
        EXPECT_TRUE(latch_component->get_async_reset_function().is_empty());
        EXPECT_TRUE(latch_component->get_async_set_function().is_empty());
        EXPECT_EQ(latch_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::undef, AsyncSetResetBehavior::undef));
        EXPECT_EQ(state_component->get_state_identifier(), "IQ");
        EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");

        latch_component->set_enable_function(BooleanFunction::Var("EN"));
        latch_component->set_data_in_function(BooleanFunction::Var("DIN"));
        latch_component->set_async_reset_function(BooleanFunction::Var("RST"));
        latch_component->set_async_set_function(BooleanFunction::Var("SET"));
        latch_component->set_async_set_reset_behavior(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L);
        state_component->set_state_identifier("IIQ");
        state_component->set_neg_state_identifier("IIQN");

        EXPECT_EQ(latch_component->get_enable_function(), BooleanFunction::Var("EN"));
        EXPECT_EQ(latch_component->get_data_in_function(), BooleanFunction::Var("DIN"));
        EXPECT_EQ(latch_component->get_async_reset_function(), BooleanFunction::Var("RST"));
        EXPECT_EQ(latch_component->get_async_set_function(), BooleanFunction::Var("SET"));
        EXPECT_EQ(latch_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L));
        EXPECT_EQ(state_component->get_state_identifier(), "IIQ");
        EXPECT_EQ(state_component->get_neg_state_identifier(), "IIQN");
    
        TEST_END
    }
}    //namespace hal
