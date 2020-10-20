#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_utils_init(py::module& m)
    {
        auto py_core_netlist_utils = m.def_submodule("NetlistUtils", R"(
            HAL Core Utility functions.
    )");

        py_core_netlist_utils.def("get_subgraph_function", &netlist_utils::get_subgraph_function, R"(
            Get the boolean function that dependence on multiple gates.

            :param subgraph_gates: Gates that should be concidered for generation.
            :param output_net: Output net for which the generation should take place.
            :returns: BooleanFunction of the specified output net.
            :rtype: hal_py.BooleanFunction
    )");
    }
}    // namespace hal
