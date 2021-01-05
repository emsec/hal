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

        py_netlist_utils.def("get_next_sequential_gates",
                             py::overload_cast<const Gate*, bool, std::unordered_map<u32, std::vector<Gate*>>&>(&netlist_utils::get_next_sequential_gates),
                             py::arg("gate"),
                             py::arg("get_successors"),
                             py::arg("cache"),
                             R"(
            Find all sequential predecessors or successors of a gate.
            Traverses combinational logic of all input or output nets until sequential gates are found.
            The result may include the provided gate itself.
            The use of the this cached version is recommended in case of extensive usage to improve performance. 
            The cache will be filled by this function and should initially be provided empty.
            Different caches for different values of get_successors shall be used.
        
            :param hal_py.Gate gate: The initial gate.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :param dict[int, list[hal_py.Gate]] cache - The cache. 
            :returns: All sequential successors or predecessors of the gate.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_next_sequential_gates", py::overload_cast<const Gate*, bool>(&netlist_utils::get_next_sequential_gates), py::arg("gate"), py::arg("get_successors"), R"(
            Find all sequential predecessors or successors of a gate.
            Traverses combinational logic of all input or output nets until sequential gates are found.
            The result may include the provided gate itself.

            :param hal_py.Gate gate: The initial gate.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :returns: All sequential successors or predecessors of the gate.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_next_sequential_gates",
                             py::overload_cast<const Net*, bool, std::unordered_map<u32, std::vector<Gate*>>&>(&netlist_utils::get_next_sequential_gates),
                             py::arg("net"),
                             py::arg("get_successors"),
                             py::arg("cache"),
                             R"(
            Find all sequential predecessors or successors of a net.
            Traverses combinational logic of all input or output nets until sequential gates are found.
            The use of the cache is recommended in case of extensive usage of this function. 
            The cache will be filled by this function and should initially be provided empty.
            Different caches for different values of get_successors shall be used.

            :param hal_py.Net net: The initial net.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :param dict[int, list[hal_py.Gate]] cache - The cache. 
            :returns: All sequential successors or predecessors of the net.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def("get_next_sequential_gates", py::overload_cast<const Net*, bool>(&netlist_utils::get_next_sequential_gates), py::arg("net"), py::arg("get_successors"), R"(
            Find all sequential predecessors or successors of a net.
            Traverses combinational logic of all input or output nets until sequential gates are found.

            :param hal_py.Net net: The initial net.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :returns: All sequential successors or predecessors of the net.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def("get_nets_at_pins", netlist_utils::get_nets_at_pins, py::arg("gate"), py::arg("pins"), py::arg("is_inputs"), R"(
            Get the nets that are connected to a subset of pins of the specified gate.
        
            :param hal_py.Gate gate: The gate.
            :param set[str] pins: The targeted pins.
            :param bool is_input: True to look for fan-in nets, false for fan-out.
            :returns: The set of nets connected to the pins.
            :rtype: set[hal_py.Net]
        )");

        py_netlist_utils.def("remove_buffers", netlist_utils::remove_buffers, py::arg("netlist"), R"(
            Remove all buffer gates from the netlist and connect their fan-in to their fan-out nets.

            :param hal_py.Netlist netlist: The target netlist.
        )");

        py_netlist_utils.def("remove_unused_lut_endpoints", netlist_utils::remove_unused_lut_endpoints, py::arg("netlist"), R"(
            Remove all LUT fan-in endpoints that are not present within the Boolean function of the output of a gate.
        
            :param hal_py.Netlist netlist: The target netlist.
        )");

        py_netlist_utils.def("rename_luts_according_to_function", netlist_utils::rename_luts_according_to_function, py::arg("netlist"), R"(
            Rename LUTs that implement simple functions to better reflect their functionality.
        
            :param hal_py.Netlist netlist: The target netlist.
        )");
    }
}    // namespace hal
