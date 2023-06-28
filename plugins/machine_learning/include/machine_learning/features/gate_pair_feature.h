#pragma once

#include "hal_core/defines.h"
#include "machine_learning/types.h"

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
                FeatureContext() = delete;
                FeatureContext(const Netlist* netlist) : nl(netlist){};

                const Netlist* nl;
            };

            class GatePairFeature
            {
            public:
                virtual std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const = 0;
                virtual std::string get_name() const                                                                   = 0;
            };

            class LogicalDistance : public GatePairFeature
            {
            public:
                LogicalDistance(){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string get_name() const override;
            };

            class PhysicalDistance : public GatePairFeature
            {
            public:
                PhysicalDistance(){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string get_name() const override;
            };

            class SharedControlSignals : public GatePairFeature
            {
            public:
                SharedControlSignals(){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string get_name() const override;
            };

            class SharedSequentialNeighbors : public GatePairFeature
            {
            public:
                SharedSequentialNeighbors(const u32 depth, const GraphDirection direction) : m_depth(depth), m_direction(direction){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string get_name() const override;

            private:
                u32 m_depth;
                GraphDirection m_direction;
            };

            class SharedNeighbors : public GatePairFeature
            {
            public:
                SharedNeighbors(const u32 depth, const GraphDirection direction) : m_depth(depth), m_direction(direction){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const override;
                std::string get_name() const override;

            private:
                u32 m_depth;
                GraphDirection m_direction;
            };

            std::vector<u32> build_feature_vec(FeatureContext& fc, const std::vector<const gate_pair_feature::GatePairFeature*>& features, const Gate* g_a, const Gate* g_b);

            std::vector<std::vector<u32>> test_build_feature_vec(const Netlist* nl, const std::vector<std::pair<Gate*, Gate*>>& pairs);
        }    // namespace gate_pair_feature
    }        // namespace machine_learning
}    // namespace hal