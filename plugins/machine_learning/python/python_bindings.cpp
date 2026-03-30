#include "bindings/register.h"

namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(machine_learning, m)
    {
        m.doc() = "Machine learning plugin for HAL.";
#else
    PYBIND11_PLUGIN(machine_learning)
    {
        py::module m("machine_learning", "Machine learning plugin for HAL.");
#endif    // ifdef PYBIND11_MODULE

        py::module py_edge_feature      = m.def_submodule("edge_feature");
        py::module py_gate_feature      = m.def_submodule("gate_feature");
        py::module py_gate_pair_feature = m.def_submodule("gate_pair_feature");
        py::module py_gate_pair_label   = m.def_submodule("gate_pair_label");
        py::module py_gate_label        = m.def_submodule("gate_label");
        py::module py_subgraph_label    = m.def_submodule("subgraph_label");

        machine_learning::python::bind_plugin(m);
        machine_learning::python::bind_gate_features(m, py_gate_feature);
        machine_learning::python::bind_gate_pair_features(m, py_gate_pair_feature);
        machine_learning::python::bind_gate_pair_labels(m, py_gate_pair_label);
        machine_learning::python::bind_gate_labels(m, py_gate_label);
        machine_learning::python::bind_subgraph_labels(m, py_subgraph_label);
        machine_learning::python::bind_edge_features(m, py_edge_feature);

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
