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
            Get all components matching the filter condition (if provided) as a set. 
            Returns an empty set if (i) the gate type does not contain any components or (ii) no component matches the filter condition.

            :param lambda filter: The filter applied to all candidate components, disabled by default.
            :returns: The components.
            :rtype: set[hal_py.GateTypeComponent]
        )");

        py_gate_type_component.def("get_component", &GateTypeComponent::get_component, py::arg("filter") = nullptr, R"(
            Get a single component matching the filter condition (if provided).
            Returns None if (i) the gate type does not contain any components, (ii) multiple components match the filter condition, or (iii) no component matches the filter condition.

            :param lambda filter: The filter applied to all candidate components.
            :returns: The component or None.
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

        py_lut_component.def_property("init_ascending", &LUTComponent::is_init_ascending, &LUTComponent::set_init_ascending, R"(
            The bit-order of the initialization string. 
            True if ascending, False otherwise.

            :type: bool
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

        // TODO implement pybinds

        py::class_<LatchComponent, GateTypeComponent, RawPtrWrapper<LatchComponent>> py_latch_component(m, "LatchComponent", R"(
            A latch component specifying the latch gate type's functionality.
        )");

        // TODO implement pybinds

        py::class_<RAMComponent, GateTypeComponent, RawPtrWrapper<RAMComponent>> py_ram_component(m, "RAMComponent", R"(
            A RAM component specifying the RAM gate type's functionality.
        )");

        // TODO implement pybinds

        py::class_<MACComponent, GateTypeComponent, RawPtrWrapper<MACComponent>> py_mac_component(m, "MACComponent", R"(
            A MAC component specifying the MAC gate type's functionality.
        )");

        // TODO implement pybinds

        py::class_<InitComponent, GateTypeComponent, RawPtrWrapper<InitComponent>> py_init_component(m, "InitComponent", R"(
            An initialization component specifying the initialization behavior of a gate type.
        )");

        py_init_component.def_static("is_class_of", &InitComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is an InitComponent.

            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a InitComponent, False otherwise.
            :rtype: bool
        )");

        py_init_component.def_property("init_category", &InitComponent::get_init_category, &InitComponent::set_init_category, R"(
            The category in which to find the initialization data.
            
            :type: str
        )");

        py_init_component.def("get_init_category", &InitComponent::get_init_category, R"(
            Get the category in which to find the initialization data.

            :returns: The data category.
            :rtype: str
        )");

        py_init_component.def("set_init_category", &InitComponent::set_init_category, py::arg("init_category"), R"(
            Set the category in which to find the initialization data.

            :param str init_category: The data category.
        )");

        py_init_component.def_property("init_identifier", &InitComponent::get_init_identifier, &InitComponent::set_init_identifier, R"(
            The identifier at which to find the initialization data.
            
            :type: str
        )");

        py_init_component.def("get_init_identifier", &InitComponent::get_init_identifier, R"(
            Get the identifier at which to find the initialization data.

            :returns: The data identifier.
            :rtype: str
        )");

        py_init_component.def("set_init_identifier", &InitComponent::set_init_identifier, py::arg("init_identifier"), R"(
            Set the identifier at which to find the initialization data.

            :param str init_identifier: The data identifier.
        )");

        py::class_<RAMPortComponent, GateTypeComponent, RawPtrWrapper<RAMPortComponent>> py_ram_port_component(m, "RAMPortComponent", R"(
            A RAM port component specifying the behavior of a port belonging to a RAM gate type.
        )");

        // TODO implement pybinds
    }
}    // namespace hal