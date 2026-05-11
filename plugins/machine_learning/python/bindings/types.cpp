#include "register.h"

namespace hal
{
    namespace machine_learning
    {
        namespace python
        {
            void bind_types(py::module& m)
            {
                py::enum_<machine_learning::StatisticalMoment>(m, "StatisticalMoment", R"(
            Aggregation modes for reducing a feature vector to a single scalar.
        )")
                    .value("min", machine_learning::StatisticalMoment::min, R"(
                Minimum value.
            )")
                    .value("max", machine_learning::StatisticalMoment::max, R"(
                Maximum value.
            )")
                    .value("average", machine_learning::StatisticalMoment::average, R"(
                Arithmetic mean.
            )")
                    .value("median", machine_learning::StatisticalMoment::median, R"(
                Median (average of the two middle values for even-sized inputs).
            )")
                    .value("stddev", machine_learning::StatisticalMoment::stddev, R"(
                Population standard deviation.
            )")
                    .export_values();

                m.def(
                    "calculate_statistical_moment",
                    [](machine_learning::StatisticalMoment moment, std::vector<FEATURE_TYPE> data) -> std::optional<FEATURE_TYPE> {
                        auto res = machine_learning::calculate_statistical_moment(moment, std::move(data));
                        if (res.is_ok())
                        {
                            return res.get();
                        }
                        else
                        {
                            log_error("python_context", "error encountered while calculating statistical moment:\n{}", res.get_error().get());
                            return std::nullopt;
                        }
                    },
                    py::arg("moment"),
                    py::arg("data"),
                    R"(
            Reduce a feature vector to a single scalar using the selected statistical moment.

            :param hal_py.machine_learning.StatisticalMoment moment: The aggregation mode to apply.
            :param list[float] data: The samples to aggregate. Must be non-empty.
            :returns: The aggregated value on success, None otherwise.
            :rtype: float or None
        )");

                py::enum_<machine_learning::NormalizationType>(m, "NormalizationType", R"(
            Selects how a vector of feature values is normalized.
        )")
                    .value("None_", machine_learning::NormalizationType::None, R"(
                Leave values untouched.
            )")
                    .value("MinMax", machine_learning::NormalizationType::MinMax, R"(
                Min-max normalize across the batch into [0, 1].
            )")
                    .value("GateCount", machine_learning::NormalizationType::GateCount, R"(
                Divide every value by the number of gates in the netlist.
            )")
                    .export_values();

                m.def(
                    "normalize_vector",
                    [](const machine_learning::NormalizationType type, std::vector<double> values, const u32 gate_count) -> std::optional<std::vector<double>> {
                        auto res = machine_learning::normalize_vector(type, values, gate_count);
                        if (res.is_ok())
                        {
                            return values;
                        }
                        else
                        {
                            log_error("python_context", "error encountered while normalizing vector:\n{}", res.get_error().get());
                            return std::nullopt;
                        }
                    },
                    py::arg("type"),
                    py::arg("values"),
                    py::arg("gate_count") = 0,
                    R"(
            Normalize a vector of values using the selected normalization mode.

            :param hal_py.machine_learning.NormalizationType type: Normalization mode to apply.
            :param list[float] values: Values to normalize.
            :param int gate_count: Number of gates in the netlist; only consulted when type == NormalizationType.GateCount. Defaults to 0.
            :returns: The normalized values on success, None otherwise.
            :rtype: list[float] or None
        )");
            }
        }    // namespace python
    }    // namespace machine_learning
}    // namespace hal
