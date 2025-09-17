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
                virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const;
                virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const;
                virtual std::string to_string() const = 0;
            };

            // class GateFeatureSingle : public GateFeature
            // {
            // public:
            //     virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const = 0;
            //     virtual std::string to_string() const                                                          = 0;

            //     virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
            // };

            // class GateFeature : public GateFeature
            // {
            // public:
            //     virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const = 0;
            //     virtual std::string to_string() const                                                                                         = 0;
            // };

            class ConnectedGlobalIOs : public GateFeature
            {
            public:
                ConnectedGlobalIOs(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
            };

            class DistanceGlobalIO : public GateFeature
            {
            public:
                DistanceGlobalIO(const PinDirection& direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
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

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
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

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
            };

            class GateTypeOneHot : public GateFeature
            {
            public:
                GateTypeOneHot(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
            };

            class GateTypeIndex : public GateFeature
            {
            public:
                GateTypeIndex(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
            };

            class NeighborCount : public GateFeature
            {
            public:
                NeighborCount(const u32 depth,
                              const PinDirection direction,
                              const bool directed                             = true,
                              const std::vector<PinType>& starting_pin_types  = {},
                              const std::vector<PinType>& forbidden_pin_types = {})
                    : m_depth(depth), m_direction(direction), m_directed(directed), m_starting_pin_types(starting_pin_types), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class SequentialNeighborCount : public GateFeature
            {
            public:
                SequentialNeighborCount(const u32 depth,
                                        const PinDirection direction,
                                        const bool directed                             = true,
                                        const std::vector<PinType>& starting_pin_types  = {},
                                        const std::vector<PinType>& forbidden_pin_types = {})
                    : m_depth(depth), m_direction(direction), m_directed(directed), m_starting_pin_types(starting_pin_types), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class NeighboringGateTypes : public GateFeature
            {
            public:
                NeighboringGateTypes(const u32 depth,
                                     const PinDirection direction,
                                     const bool directed                             = true,
                                     const std::vector<PinType>& starting_pin_types  = {},
                                     const std::vector<PinType>& forbidden_pin_types = {})
                    : m_depth(depth), m_direction(direction), m_directed(directed), m_starting_pin_types(starting_pin_types), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            class BetweennessCentrality : public GateFeature
            {
            public:
                BetweennessCentrality(const bool directed = true, const i32 cutoff = -1, const bool normalize = true) : m_directed(directed), m_cutoff(cutoff), m_normalize(normalize){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;

            private:
                const bool m_directed;
                const i32 m_cutoff;
                const bool m_normalize;
            };

            class HarmonicCentrality : public GateFeature
            {
            public:
                HarmonicCentrality(const PinDirection& direction, const i32 cutoff = -1, const bool normalize = true) : m_direction(direction), m_cutoff(cutoff), m_normalize(normalize){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const i32 m_cutoff;
                const bool m_normalize;
            };

            class SequentialBetweennessCentrality : public GateFeature
            {
            public:
                SequentialBetweennessCentrality(const bool directed = true, const i32 cutoff = -1, const bool normalize = true) : m_directed(directed), m_cutoff(cutoff), m_normalize(normalize){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;

            private:
                const bool m_directed;
                const i32 m_cutoff;
                const bool m_normalize;
            };

            class SequentialHarmonicCentrality : public GateFeature
            {
            public:
                SequentialHarmonicCentrality(const PinDirection& direction, const i32 cutoff = -1, const bool normalize = true) : m_direction(direction), m_cutoff(cutoff), m_normalize(normalize){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const i32 m_cutoff;
                const bool m_normalize;
            };

            // Feature ideas:
            // - number of sequential predecessors/successors (this is somewhat encoded in the neighboring gate types)

            //  - distance to nearest type/module (e.g. RAM, DSP)
            //  - distance to nearest shift register
            //  - distance to nearest bus register

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(Context& ctx, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal