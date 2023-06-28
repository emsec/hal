#include "machine_learning/features/gate_feature.h"

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
            std::vector<u32> ConnectedGlobalIOs::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                UNUSED(fc);

                u32 connected_global_inputs  = 0;
                u32 connected_global_outputs = 0;

                for (const auto& in : g->get_fan_in_nets())
                {
                    if (in->is_global_input_net())
                    {
                        connected_global_inputs += 1;
                    }
                }

                for (const auto& out : g->get_fan_out_nets())
                {
                    if (out->is_global_output_net())
                    {
                        connected_global_outputs += 1;
                    }
                }

                return {connected_global_inputs, connected_global_outputs};
            }

            std::vector<u32> DistanceGlobalIO::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                UNUSED(fc);

                return {};
            }

            std::vector<u32> IODegrees::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                u32 input_io_degree  = g->get_fan_in_nets().size();
                u32 output_io_degree = g->get_fan_out_nets().size();

                return {input_io_degree, output_io_degree};
            }

            std::vector<u32> GateTypeOneHot::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                return {};
            }

            std::vector<u32> NeighboringGateTypes::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                return {};
            }

            std::vector<u32> build_feature_vec(FeatureContext& fc, const std::vector<const GateFeature*>& features, const Gate* g)
            {
                std::vector<u32> feature_vec;

                for (const auto& gf : features)
                {
                    const auto new_features = gf->calculate_feature(fc, g);
                    feature_vec.insert(feature_vec.end(), new_features.begin(), new_features.end());
                }

                return feature_vec;
            }

            std::vector<std::vector<u32>> test_build_feature_vec(const Netlist* nl)
            {
                const auto t0 = gate_feature::ConnectedGlobalIOs();
                const auto t1 = gate_feature::IODegrees();

                const std::vector<const gate_feature::GateFeature*> features = {dynamic_cast<const gate_feature::GateFeature*>(&t0), dynamic_cast<const gate_feature::GateFeature*>(&t1)};

                gate_feature::FeatureContext fc = gate_feature::FeatureContext{nl};
                std::vector<std::vector<u32>> gate_features;

                for (const auto& g : nl->get_gates())
                {
                    gate_features.push_back(build_feature_vec(fc, features, g));
                }
            }

        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal