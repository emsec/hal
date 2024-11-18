#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
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

            struct FeatureContext
            {
            public:
                FeatureContext() = delete;
                FeatureContext(const Netlist* netlist) : nl(netlist){};

                const Result<NetlistAbstraction*> get_sequential_abstraction();

                const Netlist* nl;

            private:
                std::optional<NetlistAbstraction> m_seqential_abstraction;
            };

            class GatePairFeature
            {
            public:
                virtual Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const = 0;
                virtual std::string to_string() const                                                                          = 0;
            };

            class LogicalDistance : public GatePairFeature
            {
            public:
                LogicalDistance(const PinDirection direction) : m_direction(direction){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
            };

            class SequentialDistance : public GatePairFeature
            {
            public:
                SequentialDistance(const PinDirection direction) : m_direction(direction){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
            };

            class PhysicalDistance : public GatePairFeature
            {
            public:
                PhysicalDistance(){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
            };

            class SharedControlSignals : public GatePairFeature
            {
            public:
                SharedControlSignals(){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
            };

            class SharedSequentialNeighbors : public GatePairFeature
            {
            public:
                SharedSequentialNeighbors(const u32 depth, const PinDirection direction) : m_depth(depth), m_direction(direction){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
            };

            class SharedNeighbors : public GatePairFeature
            {
            public:
                SharedNeighbors(const u32 depth, const PinDirection direction) : m_depth(depth), m_direction(direction){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
            };

            // feature ideas:

            // distance to each other in a sequential only netlist
            // shared neighbors in a sequential only netlist

            Result<std::vector<u32>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b);
            Result<std::vector<u32>> build_feature_vec(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b);

            Result<std::vector<u32>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const std::pair<const Gate*, const Gate*>& gate_pair);
            Result<std::vector<u32>> build_feature_vec(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const std::pair<const Gate*, const Gate*>& gate_pair);

            Result<std::vector<std::vector<u32>>> build_feature_vecs(const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<const Gate*, const Gate*>>& gate_pairs);
            Result<std::vector<std::vector<u32>>>
                build_feature_vecs(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<const Gate*, const Gate*>>& gate_pairs);
        }    // namespace gate_pair_feature
    }    // namespace machine_learning
}    // namespace hal