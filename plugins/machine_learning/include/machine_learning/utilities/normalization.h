#pragma once

#include "hal_core/utilities/result.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
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

    }    // namespace machine_learning
}    // namespace hal