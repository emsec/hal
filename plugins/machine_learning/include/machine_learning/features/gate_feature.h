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
                virtual Result<std::vector<u32>> calculate_feature(Context& ctx, const Gate* g) const = 0;
                virtual std::string to_string() const                                                 = 0;
            };

            class ConnectedGlobalIOs : public GateFeature
            {
            public:
                ConnectedGlobalIOs(){};

                Result<std::vector<u32>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
            };

            class DistanceGlobalIO : public GateFeature
            {
            public:
                DistanceGlobalIO(const PinDirection& direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<u32>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class SequentialDistanceGlobalIO : public GateFeature
            {
            public:
                SequentialDistanceGlobalIO(const PinDirection& direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<u32>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class IODegrees : public GateFeature
            {
            public:
                IODegrees(){};

                Result<std::vector<u32>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
            };

            class GateTypeOneHot : public GateFeature
            {
            public:
                GateTypeOneHot(){};

                Result<std::vector<u32>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
            };

            class NeighboringGateTypes : public GateFeature
            {
            public:
                NeighboringGateTypes(const u32 depth, const PinDirection& direction, const bool directed = true) : m_depth(depth), m_direction(direction), m_directed(directed){};

                Result<std::vector<u32>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
            };

            // Feature ideas

            // number of sequential predecessors/successors (this is somewhat encoded in the neighboring gate types)
            // graph metrics (centrality)

            // distance to nearest type/module (e.g. RAM, DSP)
            //  - distance to nearest shift register
            //  - distance to nearest bus register

            Result<std::vector<u32>> build_feature_vec(const std::vector<const GateFeature*>& features, const Gate* g);
            Result<std::vector<u32>> build_feature_vec(Context& ctx, const std::vector<const GateFeature*>& features, const Gate* g);

            Result<std::vector<std::vector<u32>>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
            Result<std::vector<std::vector<u32>>> build_feature_vecs(Context& ctx, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal