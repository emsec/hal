#pragma once

#include "hal_core/defines.h"
#include "machine_learning/types.h"

#include <optional>
#include <vector>

namespace hal
{
    /* Forward declaration */
    class Gate;
    class Netlist;

    namespace machine_learning
    {
        namespace gate_pair_feature
        {
            class GatePairFeature
            {
            public:
                virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const = 0;
                virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_features(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const;
                virtual std::string to_string() const = 0;
            };

            class LogicalDistance : public GatePairFeature
            {
            public:
                LogicalDistance(const PinDirection direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class SequentialDistance : public GatePairFeature
            {
            public:
                SequentialDistance(const PinDirection direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class PhysicalDistance : public GatePairFeature
            {
            public:
                PhysicalDistance(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
            };

            class SharedControlSignals : public GatePairFeature
            {
            public:
                SharedControlSignals(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
            };

            class SharedSequentialNeighbors : public GatePairFeature
            {
            public:
                SharedSequentialNeighbors(const u32 depth,
                                          const PinDirection direction,
                                          const bool directed                             = true,
                                          const std::vector<PinType>& starting_pin_types  = {},
                                          const std::vector<PinType>& forbidden_pin_types = {})
                    : m_depth(depth), m_direction(direction), m_directed(directed), m_starting_pin_types(starting_pin_types), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class SharedNeighbors : public GatePairFeature
            {
            public:
                SharedNeighbors(const u32 depth,
                                const PinDirection direction,
                                const bool directed                             = true,
                                const std::vector<PinType>& starting_pin_types  = {},
                                const std::vector<PinType>& forbidden_pin_types = {})
                    : m_depth(depth), m_direction(direction), m_directed(directed), m_starting_pin_types(starting_pin_types), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b);
            Result<std::vector<FEATURE_TYPE>> build_feature_vec(Context& ctx, const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b);

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const std::pair<Gate*, Gate*>& gate_pair);
            Result<std::vector<FEATURE_TYPE>> build_feature_vec(Context& ctx, const std::vector<const GatePairFeature*>& features, const std::pair<Gate*, Gate*>& gate_pair);

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs);
            Result<std::vector<std::vector<FEATURE_TYPE>>>
                build_feature_vecs(Context& ctx, const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs);
        }    // namespace gate_pair_feature
    }        // namespace machine_learning
}    // namespace hal