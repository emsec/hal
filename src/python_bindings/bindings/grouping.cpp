#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void grouping_init(py::module& m)
    {
        py::class_<Grouping, RawPtrWrapper<Grouping>> py_grouping(m, "Grouping", R"(
            A grouping is an unstructured collection of gates, nets, and modules that do not need to be connected in any way.
            It is designed to act as a container to temporarily store related entities during netlist exploration.
            In contrast to a module, it does not allow for hierarchization.
            Each gate, net, or module within the netlist may only be assigned to a single grouping.
        )");

        py_grouping.def_property_readonly("id", &Grouping::get_id, R"(
            The unique ID of the grouping.

            :type: int
        )");

        py_grouping.def("get_id", &Grouping::get_id, R"(
            Get the unique ID of the grouping.

            :returns: The unique id.
            :rtype: int
        )");

        py_grouping.def_property("name", &Grouping::get_name, &Grouping::set_name, R"(
            The name of the grouping.

            :type: str
        )");

        py_grouping.def("get_name", &Grouping::get_name, R"(
            Get the name of the grouping.

            :returns: The name.
            :rtype: str
        )");

        py_grouping.def("set_name", &Grouping::set_name, py::arg("name"), R"(
            Set the name of the grouping.

            :param str name: The new name.
        )");

        py_grouping.def_property_readonly(
            "netlist", [](Grouping* grouping) { return RawPtrWrapper<Netlist>(grouping->get_netlist()); }, R"(
            The netlist this grouping is associated with.

            :type: hal_py.Netlist
        )");

        py_grouping.def(
            "get_netlist", [](Grouping* grouping) { return RawPtrWrapper<Netlist>(grouping->get_netlist()); }, R"(
            Get the netlist this grouping is associated with.

            :returns: The netlist.
            :rtype: hal_py.Netlist
        )");

        py_grouping.def("assign_gate", &Grouping::assign_gate, py::arg("gate"), py::arg("force") = false, R"(
            Assign a gate to the grouping.
            Fails if the gate is already contained within another grouping.
            If force is set and the gate is contained in another grouping, it is removed from the previous grouping to be assigned to this one.

            :param hal_py.Gate gate: The gate to assign.
            :param bool force: Overwrite previous assignment.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("assign_gate_by_id", &Grouping::assign_gate_by_id, py::arg("gate_id"), py::arg("force") = false, R"(
            Assign a gate to the grouping by ID.
            Fails if the gate is already contained within another grouping.
            If force is set and the gate is contained in another grouping, it is removed from the previous grouping to be assigned to this one.

            :param int gate_id: The ID of the gate to assign.
            :param bool force: Overwrite previous assignment.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def_property_readonly(
            "gates", [](Grouping* grouping) { return grouping->get_gates(); }, R"(
            All gates contained within the grouping.

            :type: list[hal_py.Gate]
        )");

        py_grouping.def("get_gates", &Grouping::get_gates, py::arg("filter") = nullptr, R"(
            Get all gates contained within the grouping.
            A filter can be applied to the result to only get gates matching the specified condition.

            :param lambda filter: Filter to be applied to the gates.
            :returns: A list of gates.
            :rtype: list[hal_py.Gate]
        )");

        py_grouping.def_property_readonly(
            "gate_ids", [](Grouping* grouping) { return grouping->get_gate_ids(); }, R"(
            The IDs of all gates contained within the grouping.

            :type: list[int]
        )");

        py_grouping.def("get_gate_ids", &Grouping::get_gate_ids, py::arg("filter") = nullptr, R"(
            Get the IDs of all gates contained within the grouping.
            A filter can be applied to the result to only get gate IDs for gates matching the specified condition.

            :param lambda filter: Filter to be applied to the gates.
            :returns: A list of gate IDs.
            :rtype: list[int]
        )");

        py_grouping.def("remove_gate", &Grouping::remove_gate, py::arg("gate"), R"(
            Remove a gate from the grouping.
            Fails if the gate is not contained within the grouping.

            :param hal_py.Gate gate: The gate to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("remove_gate_by_id", &Grouping::remove_gate_by_id, py::arg("gate_id"), R"(
            Remove a gate from the grouping by ID.
            Fails if the gate is not contained within the grouping.

            :param int gate_id: The ID of the gate to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("contains_gate", &Grouping::contains_gate, py::arg("gate"), R"(
            Check whether a gate is in the grouping.

            :param hal_py.Gate gate: The gate to check for.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("contains_gate_by_id", &Grouping::contains_gate_by_id, py::arg("gate_id"), R"(
            Check whether a gate is in the grouping by ID.

            :param int gate_id: The ID of the gate to check for.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("assign_net", &Grouping::assign_net, py::arg("net"), py::arg("force") = false, R"(
            Assign a net to the grouping.
            Fails if the net is already contained within another grouping.
            If force is set and the net is contained in another grouping, it is removed from the previous grouping to be assigned to this one.

            :param hal_py.Net net: The net to assign.
            :param bool force: Overwrite previous assignment.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("assign_net_by_id", &Grouping::assign_net_by_id, py::arg("net_id"), py::arg("force") = false, R"(
            Assign a net to the grouping by ID.
            Fails if the net is already contained within another grouping.
            If force is set and the net is contained in another grouping, it is removed from the previous grouping to be assigned to this one.

            :param int net_id: The ID of the net to assign.
            :param bool force: Overwrite previous assignment.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def_property_readonly(
            "nets", [](Grouping* grouping) { return grouping->get_nets(); }, R"(
            All nets contained within the grouping.

            :type: list[hal_py.Net]
        )");

        py_grouping.def("get_nets", &Grouping::get_nets, py::arg("filter") = nullptr, R"(
            Get all nets contained within the grouping.
            A filter can be applied to the result to only get nets matching the specified condition.

            :param lambda filter: Filter to be applied to the nets.
            :returns: A list of nets.
            :rtype: list[hal_py.Net]
        )");

        py_grouping.def_property_readonly(
            "net_ids", [](Grouping* grouping) { return grouping->get_net_ids(); }, R"(
            The IDs of all nets contained within the grouping.

            :type: list[int]
        )");

        py_grouping.def("get_net_ids", &Grouping::get_net_ids, py::arg("filter") = nullptr, R"(
            Get the IDs of all nets contained within the grouping.
            A filter can be applied to the result to only get net IDs for nets matching the specified condition.

            :param lambda filter: Filter to be applied to the nets.
            :returns: A list of net IDs.
            :rtype: list[int]
        )");

        py_grouping.def("remove_net", &Grouping::remove_net, py::arg("net"), R"(
            Remove a net from the grouping.
            Fails if the net is not contained within the grouping.

            :param hal_py.Net net: The net to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("remove_net_by_id", &Grouping::remove_net_by_id, py::arg("net_id"), R"(
            Remove a net from the grouping by ID.
            Fails if the net is not contained within the grouping.

            :param int net_id: The ID of the net to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("contains_net", &Grouping::contains_net, py::arg("net"), R"(
            Check whether a net is in the grouping.

            :param hal_py.Net net: The net to check for.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("contains_net_by_id", &Grouping::contains_net_by_id, py::arg("net_id"), R"(
            Check whether a net is in the grouping by ID.

            :param int net_id: The ID of the net to check for.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("assign_module", &Grouping::assign_module, py::arg("module"), py::arg("force") = false, R"(
            Assign a module to the grouping.
            Fails if the module is already contained within another grouping.
            If force is set and the module is contained in another grouping, it is removed from the previous grouping to be assigned to this one.

            :param hal_py.Gate module: The module to assign.
            :param bool force: Overwrite previous assignment.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("assign_module_by_id", &Grouping::assign_module_by_id, py::arg("module_id"), py::arg("force") = false, R"(
            Assign a module to the grouping by ID.
            Fails if the module is already contained within another grouping.
            If force is set and the module is contained in another grouping, it is removed from the previous grouping to be assigned to this one.

            :param int module_id: The ID of the module to assign.
            :param bool force: Overwrite previous assignment.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def_property_readonly(
            "modules", [](Grouping* grouping) { return grouping->get_modules(); }, R"(
            All modules contained within the grouping.

            :type: list[hal_py.Module]
        )");

        py_grouping.def("get_modules", &Grouping::get_modules, py::arg("filter") = nullptr, R"(
            Get all modules contained within the grouping.
            A filter can be applied to the result to only get modules matching the specified condition.

            :param lambda filter: Filter to be applied to the modules.
            :returns: A list of modules.
            :rtype: list[hal_py.Module]
        )");

        py_grouping.def_property_readonly(
            "module_ids", [](Grouping* grouping) { return grouping->get_module_ids(); }, R"(
            The IDs of all modules contained within the grouping.
            :type: list[int]
        )");

        py_grouping.def("get_module_ids", &Grouping::get_module_ids, py::arg("filter") = nullptr, R"(
            Get the IDs of all modules contained within the grouping.
            A filter can be applied to the result to only get module IDs for modules matching the specified condition.

            :param lambda filter: Filter to be applied to the modules.
            :returns: A list of module IDs.
            :rtype: list[int]
        )");

        py_grouping.def("remove_module", &Grouping::remove_module, py::arg("module"), R"(
            Remove a module from the grouping.
            Fails if the module is not contained within the grouping.

            :param hal_py.Gate module: The module to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("remove_module_by_id", &Grouping::remove_gate_by_id, py::arg("module_id"), R"(
            Remove a module from the grouping by ID.
            Fails if the module is not contained within the grouping.

            :param int module_id: The ID of the module to remove.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("contains_module", &Grouping::contains_module, py::arg("module"), R"(
            Check whether a module is in the grouping.

            :param hal_py.Gate module: The module to check for.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_grouping.def("contains_module_by_id", &Grouping::contains_module_by_id, py::arg("module_id"), R"(
            Check whether a module is in the grouping by ID.

            :param int module_id: The ID of the module to check for.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");
    }
}    // namespace hal
