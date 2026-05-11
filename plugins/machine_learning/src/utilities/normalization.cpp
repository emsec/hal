#include "machine_learning/utilities/normalization.h"

#include <algorithm>

namespace hal
{
    namespace machine_learning
    {
        Result<std::monostate> normalize_vector_min_max(std::vector<double>& values)
        {
            if (values.empty())
            {
                return OK({});
            }

            const auto min_val = *std::min_element(values.begin(), values.end());
            const auto max_val = *std::max_element(values.begin(), values.end());

            if (min_val == max_val)
            {
                values.assign(values.size(), 0.5);
                return OK({});
            }

            for (auto& value : values)
            {
                value = (value - min_val) / (max_val - min_val);
            }

            return OK({});
        }

        Result<std::monostate> normalize_vector_gate_count(std::vector<double>& values, const u32 gate_count)
        {
            if (gate_count == 0)
            {
                return ERR("cannot normalize by gate count: gate count is 0");
            }

            const auto divisor = static_cast<double>(gate_count);
            for (auto& value : values)
            {
                value = value / divisor;
            }

            return OK({});
        }

        Result<std::monostate> normalize_vector(const NormalizationType type, std::vector<double>& values, const u32 gate_count)
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

    template<>
    std::map<machine_learning::NormalizationType, std::string> EnumStrings<machine_learning::NormalizationType>::data = {
        {machine_learning::NormalizationType::None, "None"},
        {machine_learning::NormalizationType::MinMax, "MinMax"},
        {machine_learning::NormalizationType::GateCount, "GateCount"},
    };
}    // namespace hal
