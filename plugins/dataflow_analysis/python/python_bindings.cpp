#include "hal_core/python_bindings/python_bindings.h"

#include "dataflow_analysis/plugin_dataflow.h"
#include "hal_core/netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;
namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(dataflow, m)
    {
        m.doc() = "hal DataflowPlugin python bindings";
#else
    PYBIND11_PLUGIN(dataflow)
    {
        py::module m("dataflow", "hal DataflowPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE
        py::class_<plugin_dataflow, RawPtrWrapper<plugin_dataflow>, BasePluginInterface>(m, "DataflowPlugin")
            .def_property_readonly("name", &plugin_dataflow::get_name, R"(
                The name of the plugin.

                :type: str
                )")
            .def("get_name", &plugin_dataflow::get_name, R"(
                Get the name of the plugin.

                :returns: Plugin name.
                :rtype: str
                )")
            .def_property_readonly("version", &plugin_dataflow::get_version, R"(
                The version of the plugin.

                :type: str
                )")
            .def("get_version", &plugin_dataflow::get_version, R"(
                Get the version of the plugin.

                :returns: Plugin version.
                :rtype: str
                )")
            // .def("get_cli_options", &plugin_dataflow::get_cli_options)
            // .def("handle_cli_call", &plugin_dataflow::handle_cli_call)
            .def("execute", &plugin_dataflow::execute, py::arg("netlist"), py::arg("output_path"), py::arg("sizes"), py::arg("draw_graph"), py::arg("known_groups"), R"(
                Executes the dataflow analysis plugin (DANA). Starting from the netlist DANA tries to identify high-level registers.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :param str output_path: Path where the dataflow graph should be written to
                :param list[int] sizes: Prioritized sizes.
                :param bool draw_graph: Switch to turn on/off the generation of the graph.
                :param set[set[int]] known_groups: Previously known groups that stay untouched.
                :returns: Register groups created by DANA
                :rtype: list[list[hal_py.Gate]]
                )");
#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
