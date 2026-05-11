#pragma once

#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/result.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        /**
         * Selects how a vector of feature values should be normalized.
         */
        enum class NormalizationType
        {
            None,         /**< Leave the values untouched. */
            MinMax,       /**< Min-max normalize across the batch into [0, 1]. */
            GateCount,    /**< Divide every value by the number of gates in the netlist. */
        };

        /**
         * Min-max normalize the values in place into [0, 1].
         *
         * @param[in,out] values - Values to normalize in place.
         */
        Result<std::monostate> normalize_vector_min_max(std::vector<double>& values);

        /**
         * Divide every value in place by the given gate count.
         *
         * @param[in,out] values - Values to normalize in place.
         * @param[in] gate_count - Number of gates in the netlist.
         */
        Result<std::monostate> normalize_vector_gate_count(std::vector<double>& values, const u32 gate_count);

        /**
         * Dispatches to the requested normalization mode.
         *
         * @param[in] type - Normalization mode to apply.
         * @param[in,out] values - Values to normalize in place.
         * @param[in] gate_count - Number of gates in the netlist; only consulted when
         *                         `type == NormalizationType::GateCount`.
         */
        Result<std::monostate> normalize_vector(const NormalizationType type, std::vector<double>& values, const u32 gate_count = 0);

    }    // namespace machine_learning

    template<>
    std::map<machine_learning::NormalizationType, std::string> EnumStrings<machine_learning::NormalizationType>::data;
}    // namespace hal
