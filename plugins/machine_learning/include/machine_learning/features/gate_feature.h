#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"

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

            struct FeatureContext
            {
            public:
                FeatureContext() = delete;
                FeatureContext(const Netlist* netlist) : nl(netlist){};

                const Result<NetlistAbstraction*> get_sequential_abstraction();
                const std::vector<GateTypeProperty>& get_possible_gate_type_properties();

                const Netlist* nl;

            private:
                std::optional<NetlistAbstraction> m_seqential_abstraction;
                std::optional<std::vector<GateTypeProperty>> m_possible_gate_type_properties;
            };

            class GateFeature
            {
            public:
                virtual Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g) const = 0;
                virtual std::string to_string() const                                                       = 0;
            };

            class ConnectedGlobalIOs : public GateFeature
            {
            public:
                ConnectedGlobalIOs(){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g) const override;
                std::string to_string() const override;
            };

            class DistanceGlobalIO : public GateFeature
            {
            public:
                DistanceGlobalIO(const PinDirection& direction) : m_direction(direction){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
            };

            class SequentialDistanceGlobalIO : public GateFeature
            {
            public:
                SequentialDistanceGlobalIO(const PinDirection& direction) : m_direction(direction){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
            };

            class IODegrees : public GateFeature
            {
            public:
                IODegrees(){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g) const override;
                std::string to_string() const override;
            };

            class GateTypeOneHot : public GateFeature
            {
            public:
                GateTypeOneHot(){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g) const override;
                std::string to_string() const override;
            };

            class NeighboringGateTypes : public GateFeature
            {
            public:
                NeighboringGateTypes(const u32 depth, const PinDirection& direction) : m_depth(depth), m_direction(direction){};

                Result<std::vector<u32>> calculate_feature(FeatureContext& fc, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
            };

            // Feature ideas

            // number of sequential predecessors/successors
            // graph metrics (centrality)

            // distance to global io in sequential only netlist

            // distance to nearest type/module (e.g. RAM, DSP)
            // distance to nearest shift register
            // distance to nearest bus register

            Result<std::vector<u32>> build_feature_vec(const std::vector<const GateFeature*>& features, const Gate* g);
            Result<std::vector<u32>> build_feature_vec(FeatureContext& fc, const std::vector<const GateFeature*>& features, const Gate* g);

            Result<std::vector<std::vector<u32>>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
            Result<std::vector<std::vector<u32>>> build_feature_vecs(FeatureContext& fc, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal