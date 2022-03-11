#include "gate_library_test_utils.h"

#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{
    namespace test_utils
    {
        std::unique_ptr<GateLibrary> create_gate_library(const std::filesystem::path& file_path)
        {
            std::unique_ptr<GateLibrary> lib = std::unique_ptr<GateLibrary>(new GateLibrary(file_path, "TESTING_GATE_LIBRARY"));
            {
                GateType* gnd = lib->create_gate_type("GND", {GateTypeProperty::combinational, GateTypeProperty::ground});
                if (auto res = gnd->create_pin("O", PinDirection::output); res.is_error())
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
                if (auto res = vcc->create_pin("O", PinDirection::output); res.is_error())
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
                GateType* buf = lib->create_gate_type("INV", {GateTypeProperty::combinational});
                if (auto res = buf->create_pin("I", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = buf->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                buf->add_boolean_function("O", BooleanFunction::from_string("!I").get());
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
                GateType* comb44 = lib->create_gate_type("COMB12", {GateTypeProperty::combinational});
                if (auto res = comb44->create_pin("I", PinDirection::input); res.is_error())
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
                comb44->add_boolean_function("O0", BooleanFunction::from_string("I").get());
                comb44->add_boolean_function("O1", BooleanFunction::from_string("I").get());
            }
            {
                GateType* comb44 = lib->create_gate_type("COMB21", {GateTypeProperty::combinational});
                if (auto res = comb44->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("I1", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb44->add_boolean_function("O", BooleanFunction::from_string("I0 & I1").get());
            }
            {
                GateType* comb44 = lib->create_gate_type("COMB22", {GateTypeProperty::combinational});
                if (auto res = comb44->create_pin("I0", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = comb44->create_pin("I1", PinDirection::input); res.is_error())
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
                comb44->add_boolean_function("O0", BooleanFunction::from_string("I0 & I1").get());
                comb44->add_boolean_function("O1", BooleanFunction::from_string("I0 & I1").get());
            }
            {
                GateType* comb44 = lib->create_gate_type("COMB14", {GateTypeProperty::combinational});
                if (auto res = comb44->create_pin("I", PinDirection::input); res.is_error())
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
                comb44->add_boolean_function("O0", BooleanFunction::from_string("I").get());
                comb44->add_boolean_function("O1", BooleanFunction::from_string("I").get());
                comb44->add_boolean_function("O2", BooleanFunction::from_string("I").get());
                comb44->add_boolean_function("O3", BooleanFunction::from_string("I").get());
            }
            {
                GateType* comb44 = lib->create_gate_type("COMB41", {GateTypeProperty::combinational});
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
                if (auto res = comb44->create_pin("O", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                comb44->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
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
                GateType* dff =
                    lib->create_gate_type("DFF",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(
                                              GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"), BooleanFunction::from_string("D").get(), BooleanFunction::from_string("CLK").get()));
                if (auto res = dff->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff->create_pin("D", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* dff_e = lib->create_gate_type("DFFE",
                                                        {GateTypeProperty::sequential, GateTypeProperty::ff},
                                                        GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"),
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
                if (auto res = dff_e->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_e->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* dff_s =
                    lib->create_gate_type("DFFS",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(
                                              GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"), BooleanFunction::from_string("D").get(), BooleanFunction::from_string("CLK").get()));
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
                if (auto res = dff_s->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_s->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* dff_r =
                    lib->create_gate_type("DFFR",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(
                                              GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"), BooleanFunction::from_string("D").get(), BooleanFunction::from_string("CLK").get()));
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
                if (auto res = dff_r->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_r->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* dff_rs =
                    lib->create_gate_type("DFFRS",
                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                          GateTypeComponent::create_ff_component(
                                              GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"), BooleanFunction::from_string("D").get(), BooleanFunction::from_string("CLK").get()));
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
                if (auto res = dff_rs->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rs->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* dff_se          = lib->create_gate_type("DFFSE",
                                                         {GateTypeProperty::sequential, GateTypeProperty::ff},
                                                         GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"),
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
                if (auto res = dff_se->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_se->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* dff_re          = lib->create_gate_type("DFFRE",
                                                         {GateTypeProperty::sequential, GateTypeProperty::ff},
                                                         GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"),
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
                if (auto res = dff_re->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_re->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* dff_rse         = lib->create_gate_type("DFFRSE",
                                                          {GateTypeProperty::sequential, GateTypeProperty::ff},
                                                          GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"),
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
                if (auto res = dff_rse->create_pin("Q", PinDirection::input, PinType::state); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = dff_rse->create_pin("QN", PinDirection::input, PinType::neg_state); res.is_error())
                {
                    return nullptr;
                }
            }
            {
                GateType* ram = lib->create_gate_type("RAM", {GateTypeProperty::sequential, GateTypeProperty::ram});
                if (auto res = ram->create_pin("CLK", PinDirection::input, PinType::clock); res.is_error())
                {
                    return nullptr;
                }
                if (auto res = ram->create_pin("EN", PinDirection::input, PinType::enable); res.is_error())
                {
                    return nullptr;
                }

                std::vector<GatePin*> addr_pins;
                if (auto res = ram->create_pin("ADDR(0)", PinDirection::input, PinType::address); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("ADDR(1)", PinDirection::input, PinType::address); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("ADDR(2)", PinDirection::input, PinType::address); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("ADDR(3)", PinDirection::input, PinType::address); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    addr_pins.push_back(res.get());
                }

                std::vector<GatePin*> data_in_pins;
                if (auto res = ram->create_pin("DATA_IN(0)", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_IN(1)", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_IN(2)", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_IN(3)", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_in_pins.push_back(res.get());
                }

                std::vector<GatePin*> data_out_pins;
                if (auto res = ram->create_pin("DATA_OUT(0)", PinDirection::output, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_out_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_OUT(1)", PinDirection::output, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_out_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_OUT(2)", PinDirection::output, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else
                {
                    data_out_pins.push_back(res.get());
                }
                if (auto res = ram->create_pin("DATA_OUT(3)", PinDirection::output, PinType::data); res.is_error())
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
    }    // namespace test_utils
}    // namespace hal