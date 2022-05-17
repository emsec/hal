#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
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
            .value("state", GateTypeComponent::ComponentType::state, R"(State component type.)")
            .value("ram_port", GateTypeComponent::ComponentType::ram_port, R"(RAM port component type.)")
            .export_values();

        py_gate_type_component.def_property_readonly("type", &GateTypeComponent::get_type, R"(
            The type of the gate type component.

            :type: hal_py.GateTypeComponent.ComponentType
        )");

        py_gate_type_component.def("get_type", &GateTypeComponent::get_type, R"(
            Get the type of the gate type component.

            :returns: The type of the gate type component.
            :rtype: hal_py.GateTypeComponent.ComponentType
        )");

        py_gate_type_component.def_property_readonly("components", &GateTypeComponent::get_components, R"(
            All components of the gate type component as a list.

            :type: list[hal_py.GateTypeComponent]
        )");

        py_gate_type_component.def("get_components", &GateTypeComponent::get_components, py::arg("filter") = nullptr, R"(
            Get all components matching the filter condition (if provided) as a list. 
            Returns an empty list if (i) the gate type does not contain any components or (ii) no component matches the filter condition.

            :param lambda filter: The filter applied to all candidate components, disabled by default.
            :returns: The components.
            :rtype: list[hal_py.GateTypeComponent]
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

        py_ff_component.def_static("is_class_of", &FFComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is a FFComponent.

            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a FFComponent, False otherwise.
            :rtype: bool
        )");

        py_ff_component.def_property("next_state_function", &FFComponent::get_next_state_function, &FFComponent::set_next_state_function, R"(
            The Boolean function describing the next internal state of the flip-flop.
        )");

        py_ff_component.def("get_next_state_function", &FFComponent::get_next_state_function, R"(
            Get the Boolean function describing the next internal state of the flip-flop.

            :returns: The function describing the internal state.
            :rtype: hal_py.BooleanFunction
        )");

        py_ff_component.def("set_next_state_function", &FFComponent::set_next_state_function, py::arg("next_state_bf"), R"(
            Set the Boolean function describing the next internal state of the flip-flop.

            :param hal_py.BooleanFunction next_state_bf: The function describing the internal state.
        )");

        py_ff_component.def_property("clock_function", &FFComponent::get_clock_function, &FFComponent::set_clock_function, R"(
            The Boolean function describing the clock input of the flip-flop.
        )");

        py_ff_component.def("get_clock_function", &FFComponent::get_clock_function, R"(
            Get the Boolean function describing the clock input of the flip-flop.

            :returns: The function describing the clock input.
            :rtype: hal_py.BooleanFunction
        )");

        py_ff_component.def("set_clock_function", &FFComponent::set_clock_function, py::arg("clock_bf"), R"(
            Set the Boolean function describing the clock input of the flip-flop.

            :param hal_py.BooleanFunction clock_bf: The function describing the clock input.
        )");

        py_ff_component.def_property("async_reset_function", &FFComponent::get_async_reset_function, &FFComponent::set_async_reset_function, R"(
            The Boolean function describing the asynchronous reset behavior of the flip-flop.
            Is an empty function if asynchronous reset is not supported by the flip-flop.
        )");

        py_ff_component.def("get_async_reset_function", &FFComponent::get_async_reset_function, R"(
            Get the Boolean function describing the asynchronous reset behavior of the flip-flop.
            Returns an empty function if asynchronous reset is not supported by the flip-flop.

            :returns: The function describing the asynchronous reset behavior.
            :rtype: hal_py.BooleanFunction
        )");

        py_ff_component.def("set_async_reset_function", &FFComponent::set_async_reset_function, py::arg("async_reset_bf"), R"(
            Set the Boolean function describing the asynchronous reset behavior of the flip-flop.

            :param hal_py.BooleanFunction async_reset_bf: The function describing the asynchronous reset behavior.
        )");

        py_ff_component.def_property("async_set_function", &FFComponent::get_async_set_function, &FFComponent::set_async_set_function, R"(
            The Boolean function describing the asynchronous set behavior of the flip-flop.
            Is an empty function if asynchronous set is not supported by the flip-flop.
        )");

        py_ff_component.def("get_async_set_function", &FFComponent::get_async_set_function, R"(
            Get the Boolean function describing the asynchronous set behavior of the flip-flop.
            Returns an empty function if asynchronous set is not supported by the flip-flop.

            :returns: The function describing the asynchronous set behavior.
            :rtype: hal_py.BooleanFunction
        )");

        py_ff_component.def("set_async_set_function", &FFComponent::set_async_set_function, py::arg("async_set_bf"), R"(
            Set the Boolean function describing the asynchronous set behavior of the flip-flop.

            :param hal_py.BooleanFunction async_set_bf: The function describing the asynchronous set behavior.
        )");

        py_ff_component.def("get_async_set_reset_behavior", &FFComponent::get_async_set_reset_behavior, R"(
            Get the behavior of the internal state and the negated internal state when both asynchronous set and reset are active at the same time.

            :returns: The values specifying the behavior for the internal and negated internal state.
            :rytpe: tuple(hal_py.AsyncSetResetBehavior, hal_py.AsyncSetResetBehavior)
        )");

        py_ff_component.def("set_async_set_reset_behavior", &FFComponent::set_async_set_reset_behavior, py::arg("behav_state"), py::arg("behav_neg_state"), R"(
            Set the behavior of the internal state and the negated internal state when both asynchronous set and reset are active at the same time.

            :param hal_py.AsyncSetResetBehavior behav_state: The behavior of the internal state.
            :param hal_py.AsyncSetResetBehavior behav_neg_state: The behavior of the negated internal state.
        )");

        py::class_<LatchComponent, GateTypeComponent, RawPtrWrapper<LatchComponent>> py_latch_component(m, "LatchComponent", R"(
            A latch component specifying the latch gate type's functionality.
        )");

        py_latch_component.def_static("is_class_of", &LatchComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is a LatchComponent.

            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a LatchComponent, False otherwise.
            :rtype: bool
        )");

        py_latch_component.def_property("data_in_function", &LatchComponent::get_data_in_function, &LatchComponent::set_data_in_function, R"(
            The Boolean function describing the data input of the latch.
        )");

        py_latch_component.def("get_data_in_function", &LatchComponent::get_data_in_function, R"(
            Get the Boolean function describing the data input of the latch.

            :returns: The function describing the data input.
            :rtype: hal_py.BooleanFunction
        )");

        py_latch_component.def("set_data_in_function", &LatchComponent::set_data_in_function, py::arg("data_in_bf"), R"(
            Set the Boolean function describing the data input of the latch.

            :param hal_py.BooleanFunction data_in_bf: The function describing the data input.
        )");

        py_latch_component.def_property("enable_function", &LatchComponent::get_enable_function, &LatchComponent::set_enable_function, R"(
            The Boolean function describing the enable behavior of the latch.
        )");

        py_latch_component.def("get_enable_function", &LatchComponent::get_enable_function, R"(
            Get the Boolean function describing the enable behavior of the latch.

            :returns: The function describing the enable behavior.
            :rtype: hal_py.BooleanFunction
        )");

        py_latch_component.def("set_enable_function", &LatchComponent::set_enable_function, py::arg("enable_bf"), R"(
            Set the Boolean function describing the enable behavior of the latch.

            :param hal_py.BooleanFunction enable_bf: The function describing the enable behavior.
        )");

        py_latch_component.def_property("async_reset_function", &LatchComponent::get_async_reset_function, &LatchComponent::set_async_reset_function, R"(
            The Boolean function describing the asynchronous reset behavior of the latch.
            Is an empty function if asynchronous reset is not supported by the latch.
        )");

        py_latch_component.def("get_async_reset_function", &LatchComponent::get_async_reset_function, R"(
            Get the Boolean function describing the asynchronous reset behavior of the latch.
            Returns an empty function if asynchronous reset is not supported by the latch.

            :returns: The function describing the asynchronous reset behavior.
            :rtype: hal_py.BooleanFunction
        )");

        py_latch_component.def("set_async_reset_function", &LatchComponent::set_async_reset_function, py::arg("async_reset_bf"), R"(
            Set the Boolean function describing the asynchronous reset behavior of the latch.

            :param hal_py.BooleanFunction async_reset_bf: The function describing the asynchronous reset behavior.
        )");

        py_latch_component.def_property("async_set_function", &LatchComponent::get_async_set_function, &LatchComponent::set_async_set_function, R"(
            The Boolean function describing the asynchronous set behavior of the latch.
            Is an empty function if asynchronous set is not supported by the latch.
        )");

        py_latch_component.def("get_async_set_function", &LatchComponent::get_async_set_function, R"(
            Get the Boolean function describing the asynchronous set behavior of the latch.
            Returns an empty function if asynchronous set is not supported by the latch.

            :returns: The function describing the asynchronous set behavior.
            :rtype: hal_py.BooleanFunction
        )");

        py_latch_component.def("set_async_set_function", &LatchComponent::set_async_set_function, py::arg("async_set_bf"), R"(
            Set the Boolean function describing the asynchronous set behavior of the latch.

            :param hal_py.BooleanFunction async_set_bf: The function describing the asynchronous set behavior.
        )");

        py_latch_component.def("get_async_set_reset_behavior", &LatchComponent::get_async_set_reset_behavior, R"(
            Get the behavior of the internal state and the negated internal state when both asynchronous set and reset are active at the same time.

            :returns: The values specifying the behavior for the internal and negated internal state.
            :rytpe: tuple(hal_py.AsyncSetResetBehavior, hal_py.AsyncSetResetBehavior)
        )");

        py_latch_component.def("set_async_set_reset_behavior", &LatchComponent::set_async_set_reset_behavior, py::arg("behav_state"), py::arg("behav_neg_state"), R"(
            Set the behavior of the internal state and the negated internal state when both asynchronous set and reset are active at the same time.

            :param hal_py.AsyncSetResetBehavior behav_state: The behavior of the internal state.
            :param hal_py.AsyncSetResetBehavior behav_neg_state: The behavior of the negated internal state.
        )");

        py::class_<RAMComponent, GateTypeComponent, RawPtrWrapper<RAMComponent>> py_ram_component(m, "RAMComponent", R"(
            A RAM component specifying the RAM gate type's functionality.
        )");

        py_ram_component.def_static("is_class_of", &RAMComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is a RAMComponent.

            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a RAMComponent, False otherwise.
            :rtype: bool
        )");

        py_ram_component.def_property("bit_size", &RAMComponent::get_bit_size, &RAMComponent::set_bit_size, R"(
            The size of the RAM in bits.

            :type: int
        )");

        py_ram_component.def("get_bit_size", &RAMComponent::get_bit_size, R"(
            Get the size of the RAM in bits.

            :returns: The size of the RAM in bits.
        )");

        py_ram_component.def("set_bit_size", &RAMComponent::set_bit_size, py::arg("bit_size"), R"(
            Set the size of the RAM in bits.

            :param int bit_size: The size of the RAM in bits.
        )");

        py::class_<MACComponent, GateTypeComponent, RawPtrWrapper<MACComponent>> py_mac_component(m, "MACComponent", R"(
            A MAC component specifying the MAC gate type's functionality.
        )");

        py_mac_component.def_static("is_class_of", &MACComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is a MACComponent.

            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a MACComponent, False otherwise.
            :rtype: bool
        )");

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

        py_init_component.def_property("init_identifiers", &InitComponent::get_init_identifiers, &InitComponent::set_init_identifiers, R"(
            The list of identifiers at which to find initialization data.
            
            :type: str
        )");

        py_init_component.def("get_init_identifier", &InitComponent::get_init_identifiers, R"(
            Get the list of identifiers at which to find the initialization data.

            :returns: The data identifiers.
            :rtype: list[str]
        )");

        py_init_component.def("set_init_identifier", &InitComponent::set_init_identifiers, py::arg("init_identifiers"), R"(
            Set the list of identifiers at which to find the initialization data.

            :param list[str] init_identifiers: The data identifiers.
        )");

        py::class_<StateComponent, GateTypeComponent, RawPtrWrapper<StateComponent>> py_state_component(m, "StateComponent", R"(
            A StateComponent with given child component and the internal state identifiers.
        )");

        py_state_component.def_static("is_class_of", &StateComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is an StateComponent.

            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a StateComponent, False otherwise.
            :rtype: bool
        )");

        py_state_component.def_property("state_identifier", &StateComponent::get_state_identifier, &StateComponent::set_state_identifier, R"(
            The identifier of the internal state.
            
            :type: str
        )");

        py_state_component.def("get_state_identifier", &StateComponent::get_state_identifier, R"(
            Get the identifier of the internal state.

            :returns: The identifier of the internal state.
            :rtype: str
        )");

        py_state_component.def("set_state_identifier", &StateComponent::set_state_identifier, py::arg("state_identifier"), R"(
            Set the identifier of the internal state.

            :param str state_identifier: The identifier of the internal state.
        )");

        py_state_component.def_property("neg_state_identifier", &StateComponent::get_neg_state_identifier, &StateComponent::set_neg_state_identifier, R"(
            The identifier of the negated internal state.
            
            :type: str
        )");

        py_state_component.def("get_neg_state_identifier", &StateComponent::get_neg_state_identifier, R"(
            Get the identifier of the negated internal state.

            :returns: The identifier of the negated internal state.
            :rtype: str
        )");

        py_state_component.def("set_neg_state_identifier", &StateComponent::set_neg_state_identifier, py::arg("neg_state_identifier"), R"(
            Set the identifier of the negated internal state.

            :param str neg_state_identifier: The identifier of the negated internal state.
        )");

        py::class_<RAMPortComponent, GateTypeComponent, RawPtrWrapper<RAMPortComponent>> py_ram_port_component(m, "RAMPortComponent", R"(
            A RAM port component specifying the behavior of a port belonging to a RAM gate type.
        )");

        py_ram_port_component.def_static("is_class_of", &RAMPortComponent::is_class_of, py::arg("component"), R"(
            Check whether a component is a RAMPortComponent.

            :param hal_py.GateTypeComponent component: The component to check.
            :returns: True if component is a RAMPortComponent, False otherwise.
            :rtype: bool
        )");

        py_ram_port_component.def_property("data_group", &RAMPortComponent::get_data_group, &RAMPortComponent::set_data_group, R"(
            The name of the read or write data pin group.

            :type: str
        )");

        py_ram_port_component.def("get_data_group", &RAMPortComponent::get_data_group, R"(
            Get the name of the read or write data pin group.

            :returns: The name of the pin group.
            :rtype: str
        )");

        py_ram_port_component.def("set_data_group", &RAMPortComponent::set_data_group, py::arg("data_group"), R"(
            Set the name of the read or write data pin group.

            :param str data_group: The name of the pin group.
        )");

        py_ram_port_component.def_property("address_group", &RAMPortComponent::get_address_group, &RAMPortComponent::set_address_group, R"(
            The name of the read or write address pin group.

            :type: str
        )");

        py_ram_port_component.def("get_address_group", &RAMPortComponent::get_address_group, R"(
            Get the name of the read or write address pin group.

            :returns: The name of the pin group.
            :rtype: str
        )");

        py_ram_port_component.def("set_address_group", &RAMPortComponent::set_address_group, py::arg("addr_group"), R"(
            Set the name of the read or write address pin group.

            :param str addr_group: The name of the pin group.
        )");

        py_ram_port_component.def_property("clock_function", &RAMPortComponent::get_clock_function, &RAMPortComponent::set_clock_function, R"(
            The Boolean function determining the read or write clock.

            :type: hal_py.BooleanFunction
        )");

        py_ram_port_component.def("get_clock_function", &RAMPortComponent::get_clock_function, R"(
            Get the Boolean function determining the read or write clock.

            :returns: The Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_ram_port_component.def("set_clock_function", &RAMPortComponent::set_clock_function, py::arg("clock_bf"), R"(
            Set the Boolean function determining the read or write clock.

            :param hal_py.BooleanFunction clock_bf: The Boolean function.
        )");

        py_ram_port_component.def_property("enable_function", &RAMPortComponent::get_enable_function, &RAMPortComponent::set_enable_function, R"(
            The Boolean function determining the read or write enable.

            :type: hal_py.BooleanFunction
        )");

        py_ram_port_component.def("get_enable_function", &RAMPortComponent::get_enable_function, R"(
            Get the Boolean function determining the read or write enable.

            :returns: The Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_ram_port_component.def("set_enable_function", &RAMPortComponent::set_enable_function, py::arg("enable_bf"), R"(
            Set the Boolean function determining the read or write enable.

            :param hal_py.BooleanFunction enable_bf: The Boolean function.
        )");

        py_ram_port_component.def_property("write_port", &RAMPortComponent::is_write_port, &RAMPortComponent::set_write_port, R"(
            True if the port is a write port, false if it is a read port.

            :type: bool
        )");

        py_ram_port_component.def("is_write_port", &RAMPortComponent::is_write_port, R"(
            Check whether the port is a write or a read port.

            :returns: True if the port is a write port, false if it is a read port.
            :rtype: bool
        )");

        py_ram_port_component.def("set_write_port", &RAMPortComponent::set_write_port, py::arg("is_write"), R"(
            Set the port to be a write or a read port.

            :param bool is_write: True if the port is a write port, false if it is a read port.
        )");
    }
}    // namespace hal