#include "hal_core/python_bindings/python_bindings.h"

#include "machine_learning/features/gate_pair_feature.h"
#include "machine_learning/graph_neural_network.h"
#include "machine_learning/labels/gate_pair_label.h"
#include "machine_learning/plugin_machine_learning.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(machine_learning, m)
    {
        m.doc() = "hal MachineLearningPlugin python bindings";
#else
    PYBIND11_PLUGIN(machine_learning)
    {
        py::module m("machine_learning", "hal MachineLearningPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<MachineLearningPlugin, RawPtrWrapper<MachineLearningPlugin>, BasePluginInterface>(m, "MachineLearningPlugin")
            .def_property_readonly("name", &MachineLearningPlugin::get_name)
            .def("get_name", &MachineLearningPlugin::get_name)
            .def_property_readonly("version", &MachineLearningPlugin::get_version)
            .def("get_version", &MachineLearningPlugin::get_version);

        auto py_ml    = m.def_submodule("MachineLearning");
        auto py_graph = py_ml.def_submodule("Graph");
        auto py_gpf   = py_ml.def_submodule("GatePairFeature");
        auto py_gpl   = py_ml.def_submodule("GatePairLabel");

        py::enum_<machine_learning::GraphDirection>(py_graph, "GraphDirection")
            .value("directed_forward", machine_learning::GraphDirection::directed_forward)
            .value("directed_backward", machine_learning::GraphDirection::directed_backward)
            .value("bidirectional", machine_learning::GraphDirection::bidirectional)
            .export_values();

        py::class_<machine_learning::gnn::NetlistGraph>(py_graph, "NetlistGraph")
            .def(py::init<>())
            .def_readwrite("edge_list", &machine_learning::gnn::NetlistGraph::edge_list)
            .def_readwrite("node_features", &machine_learning::gnn::NetlistGraph::node_features);

        py_graph.def("annotate_netlist_graph",
                     &machine_learning::gnn::annotate_netlist_graph,
                     py::arg("nl"),
                     py::arg("nlg"),
                     R"(
          Tests constructing a netlist graph.

          :param Netlist nl: The netlist.
          :param Netlist nlg: The netlist graph.
          :returns: nothing
          :rtype: none
          )");

        py_graph.def("test_construct_netlist_graph",
                     &machine_learning::gnn::test_construct_netlist_graph,
                     py::arg("nl"),
                     R"(
          Tests constructing a netlist graph.

          :param Netlist nl: The netlist.
          :returns: The test result.
          :rtype: tuple[tuple[list[uint32], list[uint32]], list[uint32]]
          )");

        py_graph.def("test_construct_node_labels",
                     &machine_learning::gnn::test_construct_node_labels,
                     py::arg("nl"),
                     R"(
          Tests constructing node labels.

          :param Netlist nl: The netlist.
          :returns: The test result.
          :rtype: list[list[uint32]]
          )");

        py_gpf.def("test_build_feature_vec", &machine_learning::gate_pair_feature::test_build_feature_vec, py::arg("nl"), py::arg("pairs"), R"()");
        py_gpl.def("test_build_labels", &machine_learning::gate_pair_label::test_build_labels, py::arg("nl"), R"()");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
