#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_utils_init(py::module& m)
    {
        auto py_netlist_utils = m.def_submodule("NetlistUtils", R"(
            HAL Netlist Utility functions.
        )");

        py_netlist_utils.def(
            "get_subgraph_function", py::overload_cast<const Net*, const std::vector<const Gate*>&>(&netlist_utils::get_subgraph_function), py::arg("net"), py::arg("subgraph_gates"), R"(
            Get the combined Boolean function of a specific net, considering an entire subgraph.<br>
            In other words, the Boolean functions of the subgraph gates that influence the target net are combined to one function.<br>
            The variables of the resulting Boolean function are the net IDs of the nets that influence the output.
            If this function is used extensively, consider using the above variant with a cache.

            :param hal_py.Net net: The output net for which to generate the Boolean function.
            :param list[hal_py.Gate] subgraph_gates: The gates making up the subgraph.
            :returns: The combined Boolean function of the subgraph.
            :rtype: hal_py.BooleanFunction
        )");

        py_netlist_utils.def("copy_netlist", &netlist_utils::copy_netlist, py::arg("nl"), R"(
            Get a deep copy of an entire netlist including all of its gates, nets, modules, and groupings.

            :param hal_py.Netlist nl: The netlist to copy.
            :returns: The deep copy of the netlist.
            :rtype: hal_py.Netlist
        )");

        py_netlist_utils.def("get_next_sequential_gates", py::overload_cast<const Gate*, bool>(&netlist_utils::get_next_sequential_gates), py::arg("gate"), py::arg("get_successors"), R"(
            Find all sequential predecessors or successors of a specific gate.
            Traverses combinational logic of all input or output nets until sequential gates are found.
            This may include the gate itself.

            :param hal_py.Gate gate: The gate to start from
            :param bool: If true, sequential successors are returned, else sequential predecessors are returned
            :returns: All sequential successors or predecessors of the gate.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_next_sequential_gates", py::overload_cast<const Net*, bool>(&netlist_utils::get_next_sequential_gates), py::arg("net"), py::arg("get_successors"), R"(
            Find all sequential predecessors or successors of a specific net.
            Traverses combinational logic logic until sequential gates are found.

            :param hal_py.Net net: The net to start from
            :param bool: If true, sequential successors are returned, else sequential predecessors are returned
            :returns: All sequential successors or predecessors of the net.
            :rtype: list[hal_py.Net]
        )");
    }
}    // namespace hal
