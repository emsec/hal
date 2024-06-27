#include "hal_core/netlist/gate_library/gate_library.h"

#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/utilities/log.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include <iostream>

namespace hal
{
    /**
     * Tests for class GateLibrary.
     */

    class GateLibraryTest : public ::testing::Test
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
     * Testing the creation of a new GateLibrary and the addition of Gate types and includes to it
     *
     * Functions: constructor, create_gate_type, get_name, get_gate_types, get_vcc_gate_types, get_gnd_gate_types,
     *            add_include, get_includes
     */
    TEST_F(GateLibraryTest, check_library)
    {
        TEST_START
        {
            auto gl = std::make_unique<GateLibrary>("imaginary_path", "gl");
            auto other_gl = std::make_unique<GateLibrary>("imaginary_path", "other_gl");

            // check name and path
            EXPECT_EQ(gl->get_name(), "gl");
            EXPECT_EQ(other_gl->get_name(), "other_gl");
            EXPECT_EQ(gl->get_path(), "imaginary_path");
            EXPECT_EQ(other_gl->get_path(), "imaginary_path");

            // create gate types
            // AND gate type
            auto gt_and = gl->create_gate_type("gt_and");
            ASSERT_TRUE(gt_and != nullptr);
            ASSERT_TRUE(gt_and->create_pin("I0", PinDirection::input).is_ok());
            ASSERT_TRUE(gt_and->create_pin("I1", PinDirection::input).is_ok());
            ASSERT_TRUE(gt_and->create_pin("O", PinDirection::output).is_ok());
            gt_and->add_boolean_function("O", BooleanFunction::from_string("I0 & I1").get());

            // OR gate type
            auto gt_or = gl->create_gate_type("gt_or", {GateTypeProperty::combinational});
            ASSERT_TRUE(gt_or != nullptr);
            ASSERT_TRUE(gt_or->create_pin("I0", PinDirection::input).is_ok());
            ASSERT_TRUE(gt_or->create_pin("I1", PinDirection::input).is_ok());
            ASSERT_TRUE(gt_or->create_pin("O", PinDirection::output).is_ok());
            gt_or->add_boolean_function("O", BooleanFunction::from_string("I0 | I1").get());

            // GND gate type
            auto gt_gnd = gl->create_gate_type("gt_gnd");
            ASSERT_TRUE(gt_gnd != nullptr);
            ASSERT_TRUE(gt_gnd->create_pin("O", PinDirection::output).is_ok());
            gt_gnd->add_boolean_function("O", BooleanFunction::Const(BooleanFunction::Value::ZERO));
            gl->mark_gnd_gate_type(gt_gnd);

            // VCC gate type
            auto gt_vcc = gl->create_gate_type("gt_vcc");
            ASSERT_TRUE(gt_vcc != nullptr);
            ASSERT_TRUE(gt_vcc->create_pin("O", PinDirection::output).is_ok());
            gt_vcc->add_boolean_function("O",  BooleanFunction::Const(BooleanFunction::Value::ONE));
            gl->mark_vcc_gate_type(gt_vcc);

            // FF gate type
            auto gt_ff = gl->create_gate_type("gt_ff", {GateTypeProperty::ff});
            ASSERT_TRUE(gt_ff != nullptr);
            

            // Latch gate type
            auto gt_latch = gl->create_gate_type("gt_latch", {GateTypeProperty::latch});
            ASSERT_TRUE(gt_latch != nullptr);

            // LUT gate type
            auto gt_lut = gl->create_gate_type("gt_lut", {GateTypeProperty::c_lut});
            ASSERT_TRUE(gt_lut != nullptr);

            // check if all gate types contained in library
            EXPECT_EQ(gl->get_gate_types(),(std::unordered_map<std::string, GateType*>({{"gt_and", gt_and}, {"gt_gnd", gt_gnd}, {"gt_vcc", gt_vcc}, {"gt_or", gt_or}, {"gt_ff", gt_ff}, {"gt_latch", gt_latch}, {"gt_lut", gt_lut}})));
            EXPECT_EQ(gl->get_vcc_gate_types(), (std::unordered_map<std::string, GateType*>({{"gt_vcc", gt_vcc}})));
            EXPECT_EQ(gl->get_gnd_gate_types(), (std::unordered_map<std::string, GateType*>({{"gt_gnd", gt_gnd}})));

            // check base types
            EXPECT_EQ(gt_and->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));
            EXPECT_EQ(gt_or->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));
            EXPECT_EQ(gt_ff->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::ff}));
            EXPECT_EQ(gt_latch->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::latch}));
            EXPECT_EQ(gt_lut->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::c_lut}));

            // check contains_gate_type and contains_gate_type_by_name
            EXPECT_TRUE(gl->contains_gate_type(gt_and));
            EXPECT_FALSE(gl->contains_gate_type(nullptr));
            GateType* gt_nil = other_gl->create_gate_type("not_in_library", {GateTypeProperty::combinational});
            EXPECT_FALSE(gl->contains_gate_type(gt_nil));

            EXPECT_TRUE(gl->contains_gate_type_by_name(gt_and->get_name()));
            EXPECT_FALSE(gl->contains_gate_type_by_name(""));
            EXPECT_FALSE(gl->contains_gate_type_by_name("not_in_library"));

            // check get_gate_type_by_name
            EXPECT_EQ(gl->get_gate_type_by_name("gt_and"), gt_and);
            EXPECT_EQ(gl->get_gate_type_by_name(""), nullptr);
            EXPECT_EQ(gl->get_gate_type_by_name("not_in_library"), nullptr);

            // Check the addition of includes
            gl->add_include("in.clu.de");
            gl->add_include("another.include");
            gl->add_include("last.include");
            EXPECT_EQ(gl->get_includes(), std::vector<std::string>({"in.clu.de", "another.include", "last.include"}));

            // Replace gate type
            u32 repl_id = gt_and->get_id();
            EXPECT_EQ(gl->replace_gate_type(repl_id, "gt_or"), nullptr); // must avoid name collision
            gt_and = gl->replace_gate_type(repl_id, "gt_and");           // same name as before is ok
            ASSERT_TRUE(gt_and != nullptr);
            gt_and = gl->replace_gate_type(repl_id, "gt_and_repl");      // new name also ok
            ASSERT_TRUE(gt_and != nullptr);
            EXPECT_EQ(gl->get_gate_type_by_name("gt_and"), nullptr);     // no longer in gate library
            EXPECT_EQ(gl->get_gate_type_by_name("gt_and_repl"), gt_and); // replaced by type with new name
        }
        TEST_END
    }
}    //namespace hal
