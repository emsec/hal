#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_utils_init(py::module& m)
    {
        auto py_netlist_utils = m.def_submodule("NetlistUtils", R"(
            HAL Netlist Utility functions.
        )");

        py_netlist_utils.def("get_subgraph_function", &netlist_utils::get_subgraph_function, py::arg("subgraph_gates"), py::arg("output_net"), R"(
            Get the combined Boolean function of a subgraph of the netlist that is defined by a set of gates and an output net.
            The gates within the net should be connected and the given output net should represent the targeted output of the subgraph.
            The resulting Boolean function uses the net IDs of the automatically identified input nets as variables.

            :param list[hal_py.Gate] subgraph_gates: The gates making up the subgraph.
            :param hal_py.Net output_net: The output net for which to generate the Boolean function.
            :returns: The combined Boolean function of the subgraph.
            :rtype: hal_py.BooleanFunction
        )");

        py_netlist_utils.def("copy_netlist", &netlist_utils::copy_netlist, py::arg("nl"), R"(
            Get a deep copy of an entire netlist including all of its gates, nets, modules, and groupings.

            :param hal_py.Netlist nl: The netlist to copy.
            :returns: The deep copy of the netlist.
            :rtype: hal_py.Netlist
        )");
    }
}    // namespace hal
