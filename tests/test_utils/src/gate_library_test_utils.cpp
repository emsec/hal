#include "gate_library_test_utils.h"

namespace hal
{
    namespace test_utils
    {
        std::unique_ptr<GateLibrary> create_gate_library(const std::filesystem::path& file_path)
        {
            std::unique_ptr<GateLibrary> lib = std::unique_ptr<GateLibrary>(new GateLibrary(file_path, "TESTING_GATE_LIBRARY"));
            {
                GateType* gnd = lib->create_gate_type("GND", {GateTypeProperty::combinational, GateTypeProperty::ground});
                gnd->add_pin("O", PinDirection::output);
                gnd->add_boolean_function("O", BooleanFunction::from_string("0"));
                lib->mark_gnd_gate_type(gnd);
            }
            {
                GateType* vcc = lib->create_gate_type("VCC", {GateTypeProperty::combinational, GateTypeProperty::power});
                vcc->add_pin("O", PinDirection::output);
                vcc->add_boolean_function("O", BooleanFunction::from_string("1"));
                lib->mark_vcc_gate_type(vcc);
            }
            {
                GateType* buf = lib->create_gate_type("BUF", {GateTypeProperty::combinational, GateTypeProperty::buffer});
                buf->add_pin("I", PinDirection::input);
                buf->add_pin("O", PinDirection::output);
                buf->add_boolean_function("O", BooleanFunction::from_string("I"));
            }
            {
                GateType* buf = lib->create_gate_type("INV", {GateTypeProperty::combinational});
                buf->add_pin("I", PinDirection::input);
                buf->add_pin("O", PinDirection::output);
                buf->add_boolean_function("O", BooleanFunction::from_string("!I"));
            }
            {
                GateType* and2 = lib->create_gate_type("AND2", {GateTypeProperty::combinational});
                and2->add_pins({"I0", "I1"}, PinDirection::input);
                and2->add_pin("O", PinDirection::output);
                and2->add_boolean_function("O", BooleanFunction::from_string("I0 & I1"));
            }
            {
                GateType* and3 = lib->create_gate_type("AND3", {GateTypeProperty::combinational});
                and3->add_pins({"I0", "I1", "I2"}, PinDirection::input);
                and3->add_pin("O", PinDirection::output);
                and3->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2"));
            }
            {
                GateType* and4 = lib->create_gate_type("AND4", {GateTypeProperty::combinational});
                and4->add_pins({"I0", "I1", "I2", "I3"}, PinDirection::input);
                and4->add_pin("O", PinDirection::output);
                and4->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2 & I3"));
            }
            {
                GateType* or2 = lib->create_gate_type("OR2", {GateTypeProperty::combinational});
                or2->add_pins({"I0", "I1"}, PinDirection::input);
                or2->add_pin("O", PinDirection::output);
                or2->add_boolean_function("O", BooleanFunction::from_string("I0 | I1"));
            }
            {
                GateType* or3 = lib->create_gate_type("OR3", {GateTypeProperty::combinational});
                or3->add_pins({"I0", "I1", "I2"}, PinDirection::input);
                or3->add_pin("O", PinDirection::output);
                or3->add_boolean_function("O", BooleanFunction::from_string("I0 | I1 | I2"));
            }
            {
                GateType* or4 = lib->create_gate_type("OR4", {GateTypeProperty::combinational});
                or4->add_pins({"I0", "I1", "I2", "I3"}, PinDirection::input);
                or4->add_pin("O", PinDirection::output);
                or4->add_boolean_function("O", BooleanFunction::from_string("I0 | I1 | I2 | I3"));
            }
            {
                GateType* or2 = lib->create_gate_type("XOR2", {GateTypeProperty::combinational});
                or2->add_pins({"I0", "I1"}, PinDirection::input);
                or2->add_pin("O", PinDirection::output);
                or2->add_boolean_function("O", BooleanFunction::from_string("I0 ^ I1"));
            }
            {
                GateType* or3 = lib->create_gate_type("XOR3", {GateTypeProperty::combinational});
                or3->add_pins({"I0", "I1", "I2"}, PinDirection::input);
                or3->add_pin("O", PinDirection::output);
                or3->add_boolean_function("O", BooleanFunction::from_string("I0 ^ I1 ^ I2"));
            }
            {
                GateType* or4 = lib->create_gate_type("XOR4", {GateTypeProperty::combinational});
                or4->add_pins({"I0", "I1", "I2", "I3"}, PinDirection::input);
                or4->add_pin("O", PinDirection::output);
                or4->add_boolean_function("O", BooleanFunction::from_string("I0 ^ I1 ^ I2 ^ I3"));
            }
            {
                GateType* lut2 = lib->create_gate_type("LUT2", {GateTypeProperty::combinational, GateTypeProperty::lut});
                lut2->add_pins({"I0", "I1"}, PinDirection::input);
                lut2->add_pin("O", PinDirection::output, PinType::lut);
                lut2->set_config_data_category("generic");
                lut2->set_config_data_identifier("INIT");
                lut2->set_lut_init_ascending(true);
            }
            {
                GateType* lut3 = lib->create_gate_type("LUT3", {GateTypeProperty::combinational, GateTypeProperty::lut});
                lut3->add_pins({"I0", "I1", "I2"}, PinDirection::input);
                lut3->add_pin("O", PinDirection::output, PinType::lut);
                lut3->set_config_data_category("generic");
                lut3->set_config_data_identifier("INIT");
                lut3->set_lut_init_ascending(true);
            }
            {
                GateType* lut4 = lib->create_gate_type("LUT4", {GateTypeProperty::combinational, GateTypeProperty::lut});
                lut4->add_pins({"I0", "I1", "I2", "I3"}, PinDirection::input);
                lut4->add_pin("O", PinDirection::output, PinType::lut);
                lut4->set_config_data_category("generic");
                lut4->set_config_data_identifier("INIT");
                lut4->set_lut_init_ascending(true);
            }
            {
                GateType* lut5 = lib->create_gate_type("LUT5", {GateTypeProperty::combinational, GateTypeProperty::lut});
                lut5->add_pins({"I0", "I1", "I2", "I3", "I4"}, PinDirection::input);
                lut5->add_pin("O", PinDirection::output, PinType::lut);
                lut5->set_config_data_category("generic");
                lut5->set_config_data_identifier("INIT");
                lut5->set_lut_init_ascending(true);
            }
            {
                GateType* lut6 = lib->create_gate_type("LUT6", {GateTypeProperty::combinational, GateTypeProperty::lut});
                lut6->add_pins({"I0", "I1", "I2", "I3", "I4", "I5"}, PinDirection::input);
                lut6->add_pin("O", PinDirection::output, PinType::lut);
                lut6->set_config_data_category("generic");
                lut6->set_config_data_identifier("INIT");
                lut6->set_lut_init_ascending(true);
            }
            {
                GateType* carry = lib->create_gate_type("CARRY", {GateTypeProperty::combinational, GateTypeProperty::carry});
                carry->add_pins({"CI", "I0", "I1"}, PinDirection::input);
                carry->add_pin("CO", PinDirection::output);
                carry->add_boolean_function("CO", BooleanFunction::from_string("(I0 & I1) | ((I0 | I1) & CI)"));
            }
            {
                GateType* dff = lib->create_gate_type("DFF", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff->add_pin("CLK", PinDirection::input, PinType::clock);
                dff->add_pin("D", PinDirection::input, PinType::data);
                dff->add_pin("Q", PinDirection::output, PinType::state);
                dff->add_pin("QN", PinDirection::output, PinType::neg_state);
            }
            {
                GateType* dff_e = lib->create_gate_type("DFFE", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff_e->add_pin("CLK", PinDirection::input, PinType::clock);
                dff_e->add_pin("D", PinDirection::input, PinType::data);
                dff_e->add_pin("EN", PinDirection::input, PinType::enable);
                dff_e->add_pin("Q", PinDirection::output, PinType::state);
                dff_e->add_pin("QN", PinDirection::output, PinType::neg_state);
            }
            {
                GateType* dff_s = lib->create_gate_type("DFFS", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff_s->add_pin("CLK", PinDirection::input, PinType::clock);
                dff_s->add_pin("D", PinDirection::input, PinType::data);
                dff_s->add_pin("S", PinDirection::input, PinType::set);
                dff_s->add_pin("Q", PinDirection::output, PinType::state);
                dff_s->add_pin("QN", PinDirection::output, PinType::neg_state);
            }
            {
                GateType* dff_r = lib->create_gate_type("DFFR", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff_r->add_pin("CLK", PinDirection::input, PinType::clock);
                dff_r->add_pin("D", PinDirection::input, PinType::data);
                dff_r->add_pin("R", PinDirection::input, PinType::reset);
                dff_r->add_pin("Q", PinDirection::output, PinType::state);
                dff_r->add_pin("QN", PinDirection::output, PinType::neg_state);
            }
            {
                GateType* dff_rs = lib->create_gate_type("DFFRS", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff_rs->add_pin("CLK", PinDirection::input, PinType::clock);
                dff_rs->add_pin("D", PinDirection::input, PinType::data);
                dff_rs->add_pin("S", PinDirection::input, PinType::set);
                dff_rs->add_pin("R", PinDirection::input, PinType::reset);
                dff_rs->add_pin("Q", PinDirection::output, PinType::state);
                dff_rs->add_pin("QN", PinDirection::output, PinType::neg_state);
                dff_rs->set_clear_preset_behavior(GateType::ClearPresetBehavior::H, GateType::ClearPresetBehavior::L);
            }
            {
                GateType* dff_se = lib->create_gate_type("DFFSE", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff_se->add_pin("CLK", PinDirection::input, PinType::clock);
                dff_se->add_pin("D", PinDirection::input, PinType::data);
                dff_se->add_pin("EN", PinDirection::input, PinType::enable);
                dff_se->add_pin("S", PinDirection::input, PinType::set);
                dff_se->add_pin("Q", PinDirection::output, PinType::state);
                dff_se->add_pin("QN", PinDirection::output, PinType::neg_state);
            }
            {
                GateType* dff_re = lib->create_gate_type("DFFRE", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff_re->add_pin("CLK", PinDirection::input, PinType::clock);
                dff_re->add_pin("D", PinDirection::input, PinType::data);
                dff_re->add_pin("EN", PinDirection::input, PinType::enable);
                dff_re->add_pin("R", PinDirection::input, PinType::reset);
                dff_re->add_pin("Q", PinDirection::output, PinType::state);
                dff_re->add_pin("QN", PinDirection::output, PinType::neg_state);
            }
            {
                GateType* dff_rse = lib->create_gate_type("DFFRSE", {GateTypeProperty::sequential, GateTypeProperty::ff});
                dff_rse->add_pin("CLK", PinDirection::input, PinType::clock);
                dff_rse->add_pin("D", PinDirection::input, PinType::data);
                dff_rse->add_pin("EN", PinDirection::input, PinType::enable);
                dff_rse->add_pin("S", PinDirection::input, PinType::set);
                dff_rse->add_pin("R", PinDirection::input, PinType::reset);
                dff_rse->add_pin("Q", PinDirection::output, PinType::state);
                dff_rse->add_pin("QN", PinDirection::output, PinType::neg_state);
                dff_rse->set_clear_preset_behavior(GateType::ClearPresetBehavior::H, GateType::ClearPresetBehavior::L);
            }
            {
                GateType* ram = lib->create_gate_type("RAM", {GateTypeProperty::sequential, GateTypeProperty::ram});
                ram->add_pin("CLK", PinDirection::input, PinType::clock);
                ram->add_pin("EN", PinDirection::input, PinType::enable);
                ram->add_pins({"ADDR(0)", "ADDR(1)", "ADDR(2)", "ADDR(3)"}, PinDirection::input, PinType::address);
                ram->add_pins({"DATA_IN(0)", "DATA_IN(1)", "DATA_IN(2)", "DATA_IN(3)"}, PinDirection::input, PinType::data);
                ram->add_pins({"DATA_OUT(0)", "DATA_OUT(1)", "DATA_OUT(2)", "DATA_OUT(3)"}, PinDirection::output, PinType::data);
                ram->assign_pin_group("ADDR", {{0, "ADDR(0)"}, {1, "ADDR(1)"}, {2, "ADDR(2)"}, {3, "ADDR(3)"}});
                ram->assign_pin_group("DATA_IN", {{0, "DATA_IN(0)"}, {1, "DATA_IN(1)"}, {2, "DATA_IN(2)"}, {3, "DATA_IN(3)"}});
                ram->assign_pin_group("DATA_OUT", {{0, "DATA_OUT(0)"}, {1, "DATA_OUT(1)"}, {2, "DATA_OUT(2)"}, {3, "DATA_OUT(3)"}});
            }

            return std::move(lib);
        }

        const GateLibrary* get_gate_library()
        {
            static std::unique_ptr<GateLibrary> gl = nullptr;
            if (gl == nullptr)
            {
                gl = create_gate_library();
            }
            return gl.get();
        }
    }    // namespace test_utils
}    // namespace hal