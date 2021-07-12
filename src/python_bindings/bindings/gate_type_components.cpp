#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"
#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_type_components_init(py::module& m)
    {
        py::class_<GateTypeComponent, RawPtrWrapper<GateTypeComponent>> py_gate_type_component(m, "GateTypeComponent", R"(
            A component defining additional functionality of a gate type.
        )");

        py::enum_<GateTypeComponent::ComponentType>(py_gate_type_component, "ComponentType", R"(
            The type of a gate type component.
        )")
            .value("lut", GateTypeComponent::ComponentType::lut, R"(LUT component type.)")
            .value("ff", GateTypeComponent::ComponentType::ff, R"(Flip-flop component type.)")
            .value("latch", GateTypeComponent::ComponentType::latch, R"(Latch component type.)")
            .value("ram", GateTypeComponent::ComponentType::ram, R"(RAM component type.)")
            .value("mac", GateTypeComponent::ComponentType::mac, R"(MAC component type.)")
            .value("init", GateTypeComponent::ComponentType::init, R"(Initializiation component type.)")
            .value("ram_port", GateTypeComponent::ComponentType::ram_port, R"(RAM port component type.)")
            .export_values();

        py_gate_type_component.def("get_type", &GateTypeComponent::get_type, R"(
            Get the type of the gate type component.

            :returns: The type of the gate type component.
            :rtype: hal_py.GateTypeComponent.ComponentType
        )");

        py_gate_type_component.def("convert_to_lut_component", &GateTypeComponent::convert_to<LUTComponent>, R"(
            Convert the gate type component to a LUT component.
            A check is performed to determine whether the conversion is legal and None is returned in case it is not.
            
            :returns: The LUT component or None.
            :rtype: hal_py.LUTComponent
        )");

        py_gate_type_component.def("convert_to_ff_component", &GateTypeComponent::convert_to<FFComponent>, R"(
            Convert the gate type component to a FF component.
            A check is performed to determine whether the conversion is legal and None is returned in case it is not.
            
            :returns: The FF component or None.
            :rtype: hal_py.FFComponent
        )");

        py_gate_type_component.def("convert_to_latch_component", &GateTypeComponent::convert_to<LatchComponent>, R"(
            Convert the gate type component to a latch component.
            A check is performed to determine whether the conversion is legal and None is returned in case it is not.
            
            :returns: The latch component or None.
            :rtype: hal_py.LatchComponent
        )");

        py_gate_type_component.def("convert_to_ram_component", &GateTypeComponent::convert_to<RAMComponent>, R"(
            Convert the gate type component to a RAM component.
            A check is performed to determine whether the conversion is legal and None is returned in case it is not.
            
            :returns: The RAM component or None.
            :rtype: hal_py.RAMComponent
        )");

        py_gate_type_component.def("convert_to_mac_component", &GateTypeComponent::convert_to<MACComponent>, R"(
            Convert the gate type component to a MAC component.
            A check is performed to determine whether the conversion is legal and None is returned in case it is not.
            
            :returns: The MAC component or None.
            :rtype: hal_py.MACComponent
        )");

        py_gate_type_component.def("convert_to_init_component", &GateTypeComponent::convert_to<InitComponent>, R"(
            Convert the gate type component to a initialization component.
            A check is performed to determine whether the conversion is legal and None is returned in case it is not.
            
            :returns: The initializiation component or None.
            :rtype: hal_py.InitComponent
        )");

        py_gate_type_component.def("convert_to_ram_port_component", &GateTypeComponent::convert_to<RAMPortComponent>, R"(
            Convert the gate type component to a RAM port component.
            A check is performed to determine whether the conversion is legal and None is returned in case it is not.
            
            :returns: The RAM port component or None.
            :rtype: hal_py.RAMPortComponent
        )");

        py_gate_type_component.def("get_components", &GateTypeComponent::get_components, py::arg("filter") = nullptr, R"(
            Get the sub-components of the gate type component.
            A user-defined filter may be applied to the result set, but is disabled by default.

            :param lambda filter: The user-defined filter function applied to all candidate components.
            :returns: The sub-components of the gate type component.
            :rtype: set[hal_py.GateTypeComponent]
        )");

        py_gate_type_component.def("get_component", &GateTypeComponent::get_component, py::arg("filter") = nullptr, R"(
            Get a single sub-component of the gate type component.
            A user-defined filter may be applied to the result set, but is disabled by default.
            If more no or than one components match the filter condition, a nullptr is returned.

            :param lambda filter: The user-defined filter function applied to all candidate components.
            :returns: The sub-component of the gate type component or None.
            :rtype: hal_py.GateTypeComponent or None
        )");

        py::class_<LUTComponent, GateTypeComponent, RawPtrWrapper<LUTComponent>> py_lut_component(m, "LUTComponent", R"(
            A LUT component specifying the LUT gate type's functionality.
        )");

        py_lut_component.def_static("is_class_of", &LUTComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is a LUTComponent.
            
            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a LUTComponent, False otherwise.
            :rtype: bool
        )");

        py_lut_component.def("is_init_ascending", &LUTComponent::is_init_ascending, R"(
            Get the bit-order of the initialization string.
         
            :returns: True if ascending bit-order, False otherwise.
            :rtype: bool
        )");

        py_lut_component.def("set_init_ascending", &LUTComponent::set_init_ascending, py::arg("init_ascending") = true, R"(
            Set the bit-order of the initialization string.
         
            :param bool init_ascending: True if ascending bit-order, False otherwise.
        )");

        py::class_<FFComponent, GateTypeComponent, RawPtrWrapper<FFComponent>> py_ff_component(m, "FFComponent", R"(
            A FF component specifying the FF gate type's functionality.
        )");

        py::class_<LatchComponent, GateTypeComponent, RawPtrWrapper<LatchComponent>> py_latch_component(m, "LatchComponent", R"(
            A latch component specifying the latch gate type's functionality.
        )");

        py::class_<RAMComponent, GateTypeComponent, RawPtrWrapper<RAMComponent>> py_ram_component(m, "RAMComponent", R"(
            A RAM component specifying the RAM gate type's functionality.
        )");

        py::class_<MACComponent, GateTypeComponent, RawPtrWrapper<MACComponent>> py_mac_component(m, "MACComponent", R"(
            A MAC component specifying the MAC gate type's functionality.
        )");

        py::class_<InitComponent, GateTypeComponent, RawPtrWrapper<InitComponent>> py_init_component(m, "InitComponent", R"(
            An initialization component specifying the initialization behavior of a gate type.
        )");

        py::class_<RAMPortComponent, GateTypeComponent, RawPtrWrapper<RAMPortComponent>> py_ram_port_component(m, "RAMPortComponent", R"(
            A RAM port component specifying the behavior of a port belonging to a RAM gate type.
        )");
    }
}    // namespace hal