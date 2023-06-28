#include "hal_core/defines.h"

#include <vector>

namespace hal
{
    /* Forward declaration */
    class Gate;
    class Netlist;

    namespace machine_learning
    {
        namespace gate_feature
        {

            struct FeatureContext
            {
                FeatureContext() = delete;
                FeatureContext(const Netlist* netlist) : nl(netlist){};

                const Netlist* nl;
            };

            class GateFeature
            {
            public:
                virtual std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g) const = 0;
            };

            class ConnectedGlobalIOs : public GateFeature
            {
            public:
                ConnectedGlobalIOs(){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g) const override;
            };

            class DistanceGlobalIO : public GateFeature
            {
            public:
                DistanceGlobalIO(){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g) const override;
            };

            class IODegrees : public GateFeature
            {
            public:
                IODegrees(){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g) const override;
            };

            class GateTypeOneHot : public GateFeature
            {
            public:
                GateTypeOneHot(){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g) const override;
            };

            class NeighboringGateTypes : public GateFeature
            {
            public:
                NeighboringGateTypes(const u32 depth) : m_depth(depth){};

                std::vector<u32> calculate_feature(FeatureContext& fc, const Gate* g) const override;

            private:
                u32 m_depth;
            };

            std::vector<u32> build_feature_vec(FeatureContext& fc, const std::vector<const GateFeature*>& features, const Gate* g);

            std::vector<std::vector<u32>> test_build_feature_vec(const Netlist* nl);

        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal