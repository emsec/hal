#include "hal_core/python_bindings/python_bindings.h"

#include "dataflow_analysis/dataflow.h"
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
            .def("execute",
                 &plugin_dataflow::execute,
                 py::arg("netlist"),
                 py::arg("output_path"),
                 py::arg("sizes"),
                 py::arg("draw_graph")                    = false,
                 py::arg("create_modules")                = false,
                 py::arg("register_stage_identification") = false,
                 py::arg("known_groups")                  = std::vector<std::vector<u32>>(),
                 py::arg("bad_group_sizes")               = 7,
                 R"(
                Executes the dataflow analysis plugin (DANA). Starting from the netlist DANA tries to identify high-level registers.

                :param hal_py.Netlist netlist: The netlist to operate on.
                :param str output_path: Path where the dataflow graph should be written to
                :param list[int] sizes: Prioritized sizes.
                :param bool draw_graph: Switch to turn on/off the generation of the graph.
                :param bool create_modules: Switch to turn on/off the creation of HAL modules for the registers.
                :param bool register_stage_identification: Switch to turn on/off the register stage rule. Note that this rule can be too restrictive and is turned off by default.
                :param list[list[int]] known_groups: Previously known groups that stay untouched.
                :returns: Register groups created by DANA
                :rtype: list[list[hal_py.Gate]]
                )");

        auto py_dataflow = m.def_submodule("Dataflow");
        py_dataflow.def("analyze",
                        &dataflow::analyze,
                        py::arg("nl"),
                        py::arg("out_path"),
                        py::arg("sizes")                         = std::vector<u32>(),
                        py::arg("register_stage_identification") = false,
                        py::arg("known_groups")                  = std::vector<std::vector<u32>>(),
                        py::arg("bad_group_size")                = 7,
                        R"(
                            Analyze the datapath to identify word-level registers in the given netlist.
                            The registers are returned as a grouping and the results are written to a JSON file.

                            :param hal_py.Netlist nl: The netlist.
                            :param pathlib.Path out_path: The output path to which the results are written.
                            :param list[int] sizes: Register sizes that are expected to be found in the netlist. These sizes will be prioritized over others during analysis. Defaults to an empty list.
                            :param bool register_stage_identification: Set ``True``to enable register stage identification during analysis, ``False`` otherwise. Defaults to ``False``.
                            :param list[list[int]] known_groups: Registers that have been identified prior to dataflow analysis. Must be provided as a list of registers with each register being represented as a list of gate IDs. Defaults to an empty list.
                            :param int bad_group_size: Minimum expected register size. Smaller registers will not be considered during analysis. Defults to ``7``.
                            :returns: The grouping containing the registers.
                            :rtype: hal_py.Dataflow.Grouping
                        )");

        py_dataflow.def("write_dot_graph", &dataflow::write_dot_graph, py::arg("grouping"), py::arg("out_path"), py::arg("ids") = std::unordered_set<u32>(), R"(
                Write the dataflow graph as a DOT graph to the specified location.

                :param hal_py.Dataflow.Grouping grouping: The grouping containing the registers.
                :param pathlib.Path out_path: The output path.
                :param set[int] ids: The grouping IDs to include in the DOT graph. If no IDs are provided, the entire graph is written. Defaults to an empty set.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        py_dataflow.def("create_grouping_modules", &dataflow::create_grouping_modules, py::arg("grouping"), py::arg("nl"), py::arg("ids") = std::unordered_set<u32>(), R"(
                Create modules for the groupings output by dataflow analysis.

                :param hal_py.Dataflow.Grouping grouping: The grouping containing the registers.
                :param hal_py.Netlist nl: The netlist.
                :param set[int] ids: The grouping IDs to create modules for. If no IDs are provided, all modules will be created. Defaults to an empty set.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        py::class_<dataflow::Grouping, std::shared_ptr<dataflow::Grouping>> py_dataflow_grouping(py_dataflow, "Grouping");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
