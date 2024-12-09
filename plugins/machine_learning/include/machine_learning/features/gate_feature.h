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
        namespace gate_feature
        {
            class GateFeature
            {
            public:
                virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const = 0;
                virtual std::string to_string() const                                                                                         = 0;
            };

            class GateFeatureSingle : public GateFeature
            {
            public:
                virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const = 0;
                virtual std::string to_string() const                                                          = 0;

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
            };

            class GateFeatureBulk : public GateFeature
            {
            public:
                virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const = 0;
                virtual std::string to_string() const                                                                                         = 0;
            };

            // Feature ideas

            // number of sequential predecessors/successors (this is somewhat encoded in the neighboring gate types)

            // distance to nearest type/module (e.g. RAM, DSP)
            //  - distance to nearest shift register
            //  - distance to nearest bus register

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(Context& ctx, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal