#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_edge_features(py::module& m, py::module& py_edge_feature)
            {
        UNUSED(m);

        py::class_<machine_learning::edge_feature::EdgeFeature, RawPtrWrapper<machine_learning::edge_feature::EdgeFeature>> py_edge_feature_class(py_edge_feature,
                                                                                                                                                  "EdgeFeature",
                                                                                                                                                  R"(
Abstract base class representing a feature for an edge between two endpoints.
)");

        py_edge_feature_class.def(
            "calculate_feature",
            [](machine_learning::edge_feature::EdgeFeature& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in EdgeFeature::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
Calculate the feature vector for the given edge.

:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_edge_feature_class.def("to_string",
                                  &machine_learning::edge_feature::EdgeFeature::to_string,
                                  R"(
Get a string representation of this feature.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::PinTypesOnehot, RawPtrWrapper<machine_learning::edge_feature::PinTypesOnehot>, machine_learning::edge_feature::EdgeFeature> py_pin_types_onehot(
            py_edge_feature,
            "PinTypesOnehot",
            R"(
One-hot encoding feature for pin types on an edge.
)");

        py_pin_types_onehot.def(py::init<>(), R"(
Construct a PinTypesOnehot feature.
)");

        py_pin_types_onehot.def(
            "calculate_feature",
            [](machine_learning::edge_feature::PinTypesOnehot& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in PinTypesOnehot::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
One-hot encoding feature vector for pin types.

:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_pin_types_onehot.def("to_string",
                                &machine_learning::edge_feature::PinTypesOnehot::to_string,
                                R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::PinDirectionOnehot, RawPtrWrapper<machine_learning::edge_feature::PinDirectionOnehot>, machine_learning::edge_feature::EdgeFeature>
            py_pin_direction_onehot(py_edge_feature,
                                    "PinDirectionOnehot",
                                    R"(
One-hot encoding feature for pin directions on an edge.
)");

        py_pin_direction_onehot.def(py::init<>(), R"(
Construct a PinDirectionOnehot feature.
)");

        py_pin_direction_onehot.def(
            "calculate_feature",
            [](machine_learning::edge_feature::PinDirectionOnehot& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination)
                -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in PinDirectionOnehot::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
One-hot encoding feature vector for pin directions.

:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_pin_direction_onehot.def("to_string",
                                    &machine_learning::edge_feature::PinDirectionOnehot::to_string,
                                    R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::PinIndices, RawPtrWrapper<machine_learning::edge_feature::PinIndices>, machine_learning::edge_feature::EdgeFeature> py_pin_indices(py_edge_feature,
                                                                                                                                                                                      "PinIndices",
                                                                                                                                                                                      R"(
)");

        py_pin_indices.def(py::init<>(), R"(
Construct a PinIndices feature.
)");

        py_pin_indices.def(
            "calculate_feature",
            [](machine_learning::edge_feature::PinIndices& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in PinIndices::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_pin_indices.def("to_string",
                           &machine_learning::edge_feature::PinIndices::to_string,
                           R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py::class_<machine_learning::edge_feature::GateTypeIndices, RawPtrWrapper<machine_learning::edge_feature::GateTypeIndices>, machine_learning::edge_feature::EdgeFeature> py_gate_type_indices(
            py_edge_feature,
            "GateTypeIndices",
            R"(
)");

        py_gate_type_indices.def(py::init<>(), R"(
Construct a GateTypeIndices feature.
)");

        py_gate_type_indices.def(
            "calculate_feature",
            [](machine_learning::edge_feature::GateTypeIndices& self, machine_learning::Context& ctx, const Endpoint* source, const Endpoint* destination) -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = self.calculate_feature(ctx, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in GateTypeIndices::calculate_feature:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("source"),
            py::arg("destination"),
            R"(
:param hal_py.Context ctx: The context.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");

        py_gate_type_indices.def("to_string",
                                 &machine_learning::edge_feature::GateTypeIndices::to_string,
                                 R"(
Get a string representation.

:returns: A string.
:rtype: str
)");

        py_edge_feature.def(
            "build_feature_vec",
            [](machine_learning::Context& ctx, const std::vector<const machine_learning::edge_feature::EdgeFeature*>& features, const Endpoint* source, const Endpoint* destination)
                -> std::optional<std::vector<FEATURE_TYPE>> {
                auto res = machine_learning::edge_feature::build_feature_vec(ctx, features, source, destination);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "Error in build_feature_vec:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("ctx"),
            py::arg("features"),
            py::arg("source"),
            py::arg("destination"),
            R"(
Build a combined feature vector from multiple features.

:param hal_py.Context ctx: The context.
:param list[hal_py.edge_feature.EdgeFeature] features: A list of edge features.
:param hal_py.Endpoint source: The source endpoint.
:param hal_py.Endpoint destination: The destination endpoint.
:returns: The combined feature vector or None.
:rtype: list[hal_py.FEATURE_TYPE] or None
)");
            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
