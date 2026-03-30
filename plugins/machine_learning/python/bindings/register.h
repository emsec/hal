#pragma once

#include "shared.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_plugin(py::module& m);
            void bind_gate_features(py::module& m, py::module& py_gate_feature);
            void bind_gate_pair_features(py::module& m, py::module& py_gate_pair_feature);
            void bind_gate_pair_labels(py::module& m, py::module& py_gate_pair_label);
            void bind_gate_labels(py::module& m, py::module& py_gate_label);
            void bind_subgraph_labels(py::module& m, py::module& py_subgraph_label);
            void bind_edge_features(py::module& m, py::module& py_edge_feature);
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
