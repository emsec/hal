#include "gate_library_test_utils.h"

#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace test_utils
    {
        std::unique_ptr<GateLibrary> create_gate_library(const std::filesystem::path& file_path)
        {
            std::unique_ptr<GateLibrary> lib = std::unique_ptr<GateLibrary>(new GateLibrary(file_path, "TESTING_GATE_LIBRARY"));
            {
                GateType* gnd = lib->create_gate_type("GND", {GateTypeProperty::combinational, GateTypeProperty::ground});
                if (auto res = gnd->create_pin("O", PinDirection::output, PinType::ground); res.is_error())
                {
                    return nullptr;
                }
                gnd->add_boolean_function("O", BooleanFunction::from_string("0").get());
                if (!lib->mark_gnd_gate_type(gnd))
                {
                    return nullptr;
                }
            }
            {
                GateType* vcc = lib->create_gate_type("VCC", {GateTypeProperty::combinational, GateTypeProperty::power});
                if (auto res = vcc->create_pin("O", PinDirection::output, PinType::power); res.is_error())
                {
                    return nullptr;
                }
                vcc->add_boolean_function("O", BooleanFunction::from_string("1").get());
                if (!lib->mark_vcc_gate_type(vcc))
                {
                    return nullptr;
                }
            }
            {
                GateType* buf = lib->create_gate_type("BUF", {GateTypeProperty::combinational, GateTypeProperty::buffer});
                if (auto res = buf->create_pin("I", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = buf->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                buf->add_boolean_function("O", BooleanFunction::from_string("I").get());
            }
            {
                GateType* inv = lib->create_gate_type("INV", {GateTypeProperty::combinational});
                if (auto res = inv->create_pin("I", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = inv->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                inv->add_boolean_function("O", BooleanFunction::from_string("!I").get());
            }
            {
                GateType* and2 = lib->create_gate_type("AND2", {GateTypeProperty::combinational});
                if (auto res = and2->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and2->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and2->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                and2->add_boolean_function("O", BooleanFunction::from_string("I0 & I1").get());
            }
            {
                GateType* and3 = lib->create_gate_type("AND3", {GateTypeProperty::combinational});
                if (auto res = and3->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and3->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and3->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and3->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                and3->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2").get());
            }
            {
                GateType* and4 = lib->create_gate_type("AND4", {GateTypeProperty::combinational});
                if (auto res = and4->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and4->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and4->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and4->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = and4->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                and4->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
            }
            {
                GateType* or2 = lib->create_gate_type("OR2", {GateTypeProperty::combinational});
                if (auto res = or2->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or2->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or2->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                or2->add_boolean_function("O", BooleanFunction::from_string("I0 | I1").get());
            }
            {
                GateType* or3 = lib->create_gate_type("OR3", {GateTypeProperty::combinational});
                if (auto res = or3->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or3->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or3->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or3->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                or3->add_boolean_function("O", BooleanFunction::from_string("I0 | I1 | I2").get());
            }
            {
                GateType* or4 = lib->create_gate_type("OR4", {GateTypeProperty::combinational});
                if (auto res = or4->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or4->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or4->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or4->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = or4->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                or4->add_boolean_function("O", BooleanFunction::from_string("I0 | I1 | I2 | I3").get());
            }
            {
                GateType* xor2 = lib->create_gate_type("XOR2", {GateTypeProperty::combinational});
                if (auto res = xor2->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor2->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor2->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                xor2->add_boolean_function("O", BooleanFunction::from_string("I0 ^ I1").get());
            }
            {
                GateType* xor3 = lib->create_gate_type("XOR3", {GateTypeProperty::combinational});
                if (auto res = xor3->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor3->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor3->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor3->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                xor3->add_boolean_function("O", BooleanFunction::from_string("I0 ^ I1 ^ I2").get());
            }
            {
                GateType* xor4 = lib->create_gate_type("XOR4", {GateTypeProperty::combinational});
                if (auto res = xor4->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor4->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor4->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor4->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = xor4->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                xor4->add_boolean_function("O", BooleanFunction::from_string("I0 ^ I1 ^ I2 ^ I3").get());
            }
            {
                GateType* lut2 = lib->create_gate_type(
                    "LUT2", {GateTypeProperty::combinational, GateTypeProperty::lut}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), true));
                if (auto res = lut2->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut2->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut2->create_pin("O", PinDirection::output, PinType::lut); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* lut3 = lib->create_gate_type(
                    "LUT3", {GateTypeProperty::combinational, GateTypeProperty::lut}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), true));
                if (auto res = lut3->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut3->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut3->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut3->create_pin("O", PinDirection::output, PinType::lut); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* lut4 = lib->create_gate_type(
                    "LUT4", {GateTypeProperty::combinational, GateTypeProperty::lut}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), true));
                if (auto res = lut4->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut4->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut4->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut4->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut4->create_pin("O", PinDirection::output, PinType::lut); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* lut5 = lib->create_gate_type(
                    "LUT5", {GateTypeProperty::combinational, GateTypeProperty::lut}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), true));
                if (auto res = lut5->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut5->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut5->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut5->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut5->create_pin("I4", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut5->create_pin("O", PinDirection::output, PinType::lut); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* lut6 = lib->create_gate_type(
                    "LUT6", {GateTypeProperty::combinational, GateTypeProperty::lut}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), true));
                if (auto res = lut6->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut6->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut6->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut6->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut6->create_pin("I4", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut6->create_pin("I5", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = lut6->create_pin("O", PinDirection::output, PinType::lut); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* mux = lib->create_gate_type("MUX", {GateTypeProperty::combinational, GateTypeProperty::mux});
                if (auto res = mux->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = mux->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = mux->create_pin("S", PinDirection::input, PinType::select); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = mux->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                mux->add_boolean_function("O", BooleanFunction::from_string("(!S & I0) | (S & I1)").get());
            }
            {
                GateType* carry = lib->create_gate_type("CARRY", {GateTypeProperty::combinational, GateTypeProperty::carry});
                if (auto res = carry->create_pin("CI", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = carry->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = carry->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = carry->create_pin("CO", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                carry->add_boolean_function("CO", BooleanFunction::from_string("(I0 & I1) | ((I0 | I1) & CI)").get());
            }
            {
                GateType* comb12 = lib->create_gate_type("COMB12", {GateTypeProperty::combinational});
                if (auto res = comb12->create_pin("I", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb12->create_pin("O0", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb12->create_pin("O1", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb12->add_boolean_function("O0", BooleanFunction::from_string("I").get());
                comb12->add_boolean_function("O1", BooleanFunction::from_string("I").get());
            }
            {
                GateType* comb21 = lib->create_gate_type("COMB21", {GateTypeProperty::combinational});
                if (auto res = comb21->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb21->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb21->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb21->add_boolean_function("O", BooleanFunction::from_string("I0 & I1").get());
            }
            {
                GateType* comb22 = lib->create_gate_type("COMB22", {GateTypeProperty::combinational});
                if (auto res = comb22->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb22->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb22->create_pin("O0", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb22->create_pin("O1", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb22->add_boolean_function("O0", BooleanFunction::from_string("I0 & I1").get());
                comb22->add_boolean_function("O1", BooleanFunction::from_string("I0 & I1").get());
            }
            {
                GateType* comb14 = lib->create_gate_type("COMB14", {GateTypeProperty::combinational});
                if (auto res = comb14->create_pin("I", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb14->create_pin("O0", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb14->create_pin("O1", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb14->create_pin("O2", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb14->create_pin("O3", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb14->add_boolean_function("O0", BooleanFunction::from_string("I").get());
                comb14->add_boolean_function("O1", BooleanFunction::from_string("I").get());
                comb14->add_boolean_function("O2", BooleanFunction::from_string("I").get());
                comb14->add_boolean_function("O3", BooleanFunction::from_string("I").get());
            }
            {
                GateType* comb41 = lib->create_gate_type("COMB41", {GateTypeProperty::combinational});
                if (auto res = comb41->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb41->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb41->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb41->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb41->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb41->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
            }
            {
                GateType* comb44 = lib->create_gate_type("COMB44", {GateTypeProperty::combinational});
                if (auto res = comb44->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("O0", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("O1", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("O2", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("O3", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb44->add_boolean_function("O0", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
                comb44->add_boolean_function("O1", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
                comb44->add_boolean_function("O2", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
                comb44->add_boolean_function("O3", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
            }
            {
                GateType* comb18 = lib->create_gate_type("COMB18", {GateTypeProperty::combinational});
                if (auto res = comb18->create_pin("I", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O0", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O1", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O2", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O3", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O4", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O5", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O6", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb18->create_pin("O7", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb18->add_boolean_function("O0", BooleanFunction::from_string("I").get());
                comb18->add_boolean_function("O1", BooleanFunction::from_string("I").get());
                comb18->add_boolean_function("O2", BooleanFunction::from_string("I").get());
                comb18->add_boolean_function("O3", BooleanFunction::from_string("I").get());
                comb18->add_boolean_function("O4", BooleanFunction::from_string("I").get());
                comb18->add_boolean_function("O5", BooleanFunction::from_string("I").get());
                comb18->add_boolean_function("O6", BooleanFunction::from_string("I").get());
                comb18->add_boolean_function("O7", BooleanFunction::from_string("I").get());
            }
            {
                GateType* comb81 = lib->create_gate_type("COMB81", {GateTypeProperty::combinational});
                if (auto res = comb81->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("I4", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("I5", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("I6", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("I7", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb81->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb81->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
            }
            {
                GateType* comb88 = lib->create_gate_type("COMB88", {GateTypeProperty::combinational});
                if (auto res = comb88->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("I2", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("I3", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("I4", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("I5", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("I6", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("I7", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O0", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O1", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O2", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O3", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O4", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O5", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O6", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb88->create_pin("O7", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb88->add_boolean_function("O0", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
                comb88->add_boolean_function("O1", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
                comb88->add_boolean_function("O2", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
                comb88->add_boolean_function("O3", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
                comb88->add_boolean_function("O4", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
                comb88->add_boolean_function("O5", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
                comb88->add_boolean_function("O6", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
                comb88->add_boolean_function("O7", BooleanFunction::from_string("I0 & I1 & I2 & I3 & I4 & I5 & I6 & I7").get());
            }
            {
                GateType* dff =
                    lib->create_gate_type("DFF",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK").get()));
                if (auto res = dff->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {
                GateType* dff_e =
                    lib->create_gate_type("DFFE",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK & EN").get()));
                if (auto res = dff_e->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_e->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_e->create_pin("EN", PinDirection::input, PinType::enable); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_e->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_e->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff_e->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff_e->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {
                GateType* dff_s =
                    lib->create_gate_type("DFFS",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK").get()));
                FFComponent* ff_component = dff_s->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);
                ff_component->set_async_set_function(BooleanFunction::from_string("S").get());
                if (auto res = dff_s->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_s->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_s->create_pin("S", PinDirection::input, PinType::set); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_s->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_s->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff_s->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff_s->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {
                GateType* dff_r =
                    lib->create_gate_type("DFFR",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK").get()));
                FFComponent* ff_component = dff_r->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);
                ff_component->set_async_reset_function(BooleanFunction::from_string("R").get());
                if (auto res = dff_r->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_r->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_r->create_pin("R", PinDirection::input, PinType::reset); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_r->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_r->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff_r->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff_r->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {
                GateType* dff_rs =
                    lib->create_gate_type("DFFRS",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK").get()));
                FFComponent* ff_component = dff_rs->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);
                ff_component->set_async_set_function(BooleanFunction::from_string("S").get());
                ff_component->set_async_reset_function(BooleanFunction::from_string("R").get());
                ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L);
                if (auto res = dff_rs->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rs->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rs->create_pin("R", PinDirection::input, PinType::reset); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rs->create_pin("S", PinDirection::input, PinType::set); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rs->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rs->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff_rs->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff_rs->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {
                GateType* dff_se =
                    lib->create_gate_type("DFFSE",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK & EN").get()));
                FFComponent* ff_component = dff_se->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);
                ff_component->set_async_set_function(BooleanFunction::from_string("S").get());
                if (auto res = dff_se->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_se->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_se->create_pin("S", PinDirection::input, PinType::set); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_se->create_pin("EN", PinDirection::input, PinType::enable); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_se->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_se->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff_se->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff_se->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {
                GateType* dff_re =
                    lib->create_gate_type("DFFRE",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK & EN").get()));
                FFComponent* ff_component = dff_re->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);
                ff_component->set_async_reset_function(BooleanFunction::from_string("R").get());
                if (auto res = dff_re->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_re->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_re->create_pin("R", PinDirection::input, PinType::reset); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_re->create_pin("EN", PinDirection::input, PinType::enable); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_re->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_re->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff_re->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff_re->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {
                GateType* dff_rse =
                    lib->create_gate_type("DFFRSE",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"),
                                                                                 BooleanFunction::from_string("D").get(),
                                                                                 BooleanFunction::from_string("CLK & EN").get()));
                FFComponent* ff_component = dff_rse->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);
                ff_component->set_async_set_function(BooleanFunction::from_string("S").get());
                ff_component->set_async_reset_function(BooleanFunction::from_string("R").get());
                ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::H, AsyncSetResetBehavior::L);
                if (auto res = dff_rse->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rse->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rse->create_pin("R", PinDirection::input, PinType::reset); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rse->create_pin("S", PinDirection::input, PinType::set); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rse->create_pin("EN", PinDirection::input, PinType::enable); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rse->create_pin("Q", PinDirection::output, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rse->create_pin("QN", PinDirection::output, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
                dff_rse->add_boolean_function("Q", BooleanFunction::from_string("IQ").get());
                dff_rse->add_boolean_function("QN", BooleanFunction::from_string("IQN").get());
            }
            {    // just an empty dummy for now, does not come with relevant components
                GateType* ram = lib->create_gate_type("RAM", {GateTypeProperty::sequential});
                if (auto res = ram->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = ram->create_pin("EN", PinDirection::input, PinType::enable); res.is_error())
                {
                    return nullptr;
                }

                std::vector<GatePin*> addr_pins;
                if (auto res = ram->create_pin("ADDR(0)", PinDirection::input, PinType::address, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("ADDR(1)", PinDirection::input, PinType::address, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("ADDR(2)", PinDirection::input, PinType::address, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("ADDR(3)", PinDirection::input, PinType::address, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }

                std::vector<GatePin*> data_in_pins;
                if (auto res = ram->create_pin("DATA_IN(0)", PinDirection::input, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_IN(1)", PinDirection::input, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_IN(2)", PinDirection::input, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_IN(3)", PinDirection::input, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }

                std::vector<GatePin*> data_out_pins;
                if (auto res = ram->create_pin("DATA_OUT(0)", PinDirection::output, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_out_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_OUT(1)", PinDirection::output, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_out_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_OUT(2)", PinDirection::output, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_out_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_OUT(3)", PinDirection::output, PinType::data, false); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_out_pins.push_back(res.get());
                }

                if (auto res = ram->create_pin_group("ADDR", addr_pins, PinDirection::input, PinType::address, true, 0); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = ram->create_pin_group("DATA_IN", data_in_pins, PinDirection::input, PinType::data, true, 0); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = ram->create_pin_group("DATA_OUT", data_out_pins, PinDirection::output, PinType::data, true, 0); res.is_error())
                {
                    return nullptr;
                }
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

        bool gate_pin_groups_are_equal(const PinGroup<GatePin>* const pg1, const PinGroup<GatePin>* const pg2)
        {
            return *pg1 == *pg2;
        }

        bool gate_types_are_equal(const GateType* const gt1, const GateType* const gt2)
        {
            if (gt1->get_id() != gt2->get_id())
            {
                log_info("test_utils", "unequal ID of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            if (gt1->get_name() != gt2->get_name())
            {
                log_info("test_utils", "unequal name of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            if (gt1->get_properties() != gt2->get_properties())
            {
                log_info("test_utils", "unequal properties of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            if (gt1->get_boolean_functions() != gt2->get_boolean_functions())
            {
                log_info("test_utils", "unequal Boolean functions of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            auto pg1 = gt1->get_pin_groups();
            auto pg2 = gt2->get_pin_groups();
            if (pg1.size() != pg2.size())
            {
                return false;
            }
            for (auto [pg1_it, pg2_it] = std::tuple{pg1.begin(), pg2.begin()}; pg1_it != pg1.end() && pg2_it != pg2.end(); pg1_it++, pg2_it++)
            {
                if (!gate_pin_groups_are_equal(*pg1_it, *pg2_it))
                {
                    log_info("test_utils", "unequal pin groups of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                    return false;
                }
            }

            const LUTComponent* lut_component1 = gt1->get_component_as<LUTComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::lut; });
            const LUTComponent* lut_component2 = gt2->get_component_as<LUTComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::lut; });
            if (lut_component1 != nullptr && lut_component2 != nullptr)
            {
                if (lut_component1->is_init_ascending() != lut_component2->is_init_ascending())
                {
                    log_info("test_utils", "unequal LUT components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                    return false;
                }
            }
            else if (lut_component1 != nullptr || lut_component2 != nullptr)
            {
                log_info("test_utils", "unequal LUT components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            const FFComponent* ff_component1 = gt1->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
            const FFComponent* ff_component2 = gt2->get_component_as<FFComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::ff; });
            if (ff_component1 != nullptr && ff_component2 != nullptr)
            {
                if (ff_component1->get_next_state_function() != ff_component2->get_next_state_function() || ff_component1->get_clock_function() != ff_component2->get_clock_function()
                    || ff_component1->get_async_reset_function() != ff_component2->get_async_reset_function() || ff_component1->get_async_set_function() != ff_component2->get_async_set_function()
                    || ff_component1->get_async_set_reset_behavior() != ff_component2->get_async_set_reset_behavior())
                {
                    log_info("test_utils", "unequal FF components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                    return false;
                }
            }
            else if (ff_component1 != nullptr || ff_component2 != nullptr)
            {
                log_info("test_utils", "unequal FF components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            const LatchComponent* latch_component1 =
                gt1->get_component_as<LatchComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::latch; });
            const LatchComponent* latch_component2 =
                gt2->get_component_as<LatchComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::latch; });
            if (latch_component1 != nullptr && latch_component2 != nullptr)
            {
                if (latch_component1->get_data_in_function() != latch_component2->get_data_in_function() || latch_component1->get_enable_function() != latch_component2->get_enable_function()
                    || latch_component1->get_async_reset_function() != latch_component2->get_async_reset_function()
                    || latch_component1->get_async_set_function() != latch_component2->get_async_set_function()
                    || latch_component1->get_async_set_reset_behavior() != latch_component2->get_async_set_reset_behavior())
                {
                    log_info("test_utils", "unequal Latch components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                    return false;
                }
            }
            else if (latch_component1 != nullptr || latch_component2 != nullptr)
            {
                log_info("test_utils", "unequal Latch components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            const InitComponent* init_component1 =
                gt1->get_component_as<InitComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::init; });
            const InitComponent* init_component2 =
                gt2->get_component_as<InitComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::init; });
            if (init_component1 != nullptr && init_component2 != nullptr)
            {
                if (init_component1->get_init_category() != init_component2->get_init_category() || init_component1->get_init_identifiers() != init_component2->get_init_identifiers())
                {
                    log_info("test_utils", "unequal Init components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                    return false;
                }
            }
            else if (init_component1 != nullptr || init_component2 != nullptr)
            {
                log_info("test_utils", "unequal Init components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            const StateComponent* state_component1 =
                gt1->get_component_as<StateComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::state; });
            const StateComponent* state_component2 =
                gt2->get_component_as<StateComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::state; });
            if (state_component1 != nullptr && state_component2 != nullptr)
            {
                if (state_component1->get_state_identifier() != state_component2->get_state_identifier()
                    || state_component1->get_neg_state_identifier() != state_component2->get_neg_state_identifier())
                {
                    log_info("test_utils", "unequal Init components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                    return false;
                }
            }
            else if (state_component1 != nullptr || state_component2 != nullptr)
            {
                log_info("test_utils", "unequal Init components of gate types with names '{}' and '{}'", gt1->get_name(), gt2->get_name());
                return false;
            }

            return true;
        }

        bool gate_libraries_are_equal(const GateLibrary* const gl1, const GateLibrary* const gl2)
        {
            bool is_equal = true;
            is_equal &= gl1->get_name() == gl2->get_name();

            std::vector<GateType*> gt1;
            for (const auto& [_, type] : gl1->get_gate_types())
            {
                UNUSED(_);
                gt1.push_back(type);
            }
            std::sort(gt1.begin(), gt1.end(), [](GateType* a, GateType* b) { return a->get_name() < b->get_name(); });

            std::vector<GateType*> gt2;
            for (const auto& [_, type] : gl2->get_gate_types())
            {
                UNUSED(_);
                gt2.push_back(type);
            }
            std::sort(gt2.begin(), gt2.end(), [](GateType* a, GateType* b) { return a->get_name() < b->get_name(); });

            if (gt1.size() != gt2.size())
            {
                log_info("test_utils", "unequal number of gate types: '{}' vs. '{}'", gt1.size(), gt2.size());
                return false;
            }
            for (auto [gt1_it, gt2_it] = std::tuple{gt1.begin(), gt2.begin()}; gt1_it != gt1.end() && gt2_it != gt2.end(); gt1_it++, gt2_it++)
            {
                if (!gate_types_are_equal(*gt1_it, *gt2_it))
                {
                    log_info("test_utils", "unequal gate types with names '{}' and '{}'", (*gt1_it)->get_name(), (*gt2_it)->get_name());
                    return false;
                }
            }

            return is_equal;
        }
    }    // namespace test_utils
}    // namespace hal