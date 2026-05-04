#pragma once

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

        template<typename T>
        Result<std::monostate> normalize_vector_min_max(std::vector<T>& values)
        {
            // Ensure T is a numeric type
            static_assert(std::is_arithmetic<T>::value, "Vector elements must be numeric.");

            if (!values.empty())
            {
                const auto min_val = *std::min_element(values.begin(), values.end());
                const auto max_val = *std::max_element(values.begin(), values.end());

                // Avoid division by zero if all elements are the same
                if (min_val == max_val)
                {
                    values.assign(values.size(), static_cast<T>(0.5));
                    return OK({});
                }

                // Apply min-max normalization
                for (auto& value : values)
                {
                    value = (value - min_val) / (max_val - min_val);
                }
            }

            return OK({});
        }

        template<typename T>
        Result<std::monostate> normalize_vector_gate_count(std::vector<T>& values, const u32 gate_count)
        {
            static_assert(std::is_arithmetic<T>::value, "Vector elements must be numeric.");

            if (gate_count == 0)
            {
                return ERR("cannot normalize by gate count: gate count is 0");
            }

            const auto divisor = static_cast<T>(gate_count);
            for (auto& value : values)
            {
                value = value / divisor;
            }

            return OK({});
        }

        /**
         * Dispatches to the requested normalization mode.
         *
         * @param[in] type - Normalization mode to apply.
         * @param[in,out] values - Values to normalize in place.
         * @param[in] gate_count - Number of gates in the netlist; only consulted when
         *                         `type == NormalizationType::GateCount`.
         */
        template<typename T>
        Result<std::monostate> normalize_vector(const NormalizationType type, std::vector<T>& values, const u32 gate_count = 0)
        {
            switch (type)
            {
                case NormalizationType::None:
                    return OK({});
                case NormalizationType::MinMax:
                    return normalize_vector_min_max(values);
                case NormalizationType::GateCount:
                    return normalize_vector_gate_count(values, gate_count);
            }
            return ERR("unknown normalization type");
        }

    }    // namespace machine_learning
}    // namespace hal
