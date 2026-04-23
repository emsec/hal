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
            /**
             * Abstract base class for per-pair feature extractors.
             *
             * A GatePairFeature maps an ordered pair of gates (g_a, g_b) — sharing a Context — to
             * a fixed-width vector of FEATURE_TYPE values. The width and semantic meaning of the
             * vector are identical for every pair processed with the same Context and are
             * described by get_legend().
             *
             * Subclasses must override:
             *   - calculate_feature(ctx, g_a, g_b): the per-pair computation;
             *   - to_string(): a stable, parameter-encoded name (used for caching, logging, and
             *                  as the basis of legend labels);
             *   - get_legend(ctx): one label per element of the produced feature vector.
             *
             * The batch variant calculate_features() has a default serial implementation.
             */
            class GatePairFeature
            {
            public:
                /**
                 * Compute the feature vector for an ordered pair of gates.
                 *
                 * @param[in] ctx - Shared ML context.
                 * @param[in] g_a - First gate of the pair.
                 * @param[in] g_b - Second gate of the pair.
                 * @returns The feature vector on success, an error otherwise. The returned vector
                 *          has length equal to get_legend(ctx).size().
                 */
                virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const = 0;

                /**
                 * Compute feature vectors for a batch of gate pairs. The default implementation
                 * calls calculate_feature() serially.
                 */
                virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_features(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const;

                /**
                 * Stable, parameter-encoded name of the feature, suitable as a cache key or log
                 * tag. Two feature instances with the same to_string() produce bit-identical
                 * output for the same context.
                 */
                virtual std::string to_string() const = 0;

                /**
                 * Concise per-index description of the feature vector produced by
                 * calculate_feature(ctx, ...).
                 *
                 * The returned vector's size equals the width of the feature vector. Labels are
                 * purely descriptive (no to_string() prefix); a caller that needs globally unique
                 * names across concatenated features should build that mapping itself.
                 *
                 * @param[in] ctx - Shared ML context.
                 * @returns One concise descriptor per index of the feature vector.
                 */
                virtual std::vector<std::string> get_legend(Context& ctx) const = 0;
            };

            /**
             * Shortest-path distance(s) between two gates on the original netlist abstraction.
             *
             * Width depends on `direction`:
             *   - direction == PinDirection::inout: width 1, a single undirected distance value.
             *   - otherwise:                         width 2, the two directed distances
             *                                       (a -> b, b -> a).
             *
             * Unreachable pairs yield 255 (the saturation value); equal gates yield 0.
             *
             * @param direction - Direction of traversal (input, output, inout).
             * @param directed - If true, respect signal direction; if false, treat as undirected.
             * @param forbidden_pin_types - Endpoints on pins of these types are skipped during
             *                              traversal (e.g., to ignore resets or clocks).
             */
            class LogicalDistance : public GatePairFeature
            {
            public:
                LogicalDistance(const PinDirection direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Same as LogicalDistance but evaluated on the sequential abstraction (combinational
             * gates collapsed).
             */
            class SequentialDistance : public GatePairFeature
            {
            public:
                SequentialDistance(const PinDirection direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Physical (layout) distance between two gates.
             *
             * NOTE: currently a stub — returns an empty vector and logs an error. Reserved for a
             * future implementation that consumes placement coordinates.
             */
            class PhysicalDistance : public GatePairFeature
            {
            public:
                PhysicalDistance(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Number of control-signal nets (clock, enable, reset, set) that are shared between
             * the two gates on the input side.
             *
             * Legend (width 1): the shared control net count.
             */
            class SharedControlSignals : public GatePairFeature
            {
            public:
                SharedControlSignals(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Number of sequential gates in the intersection of the two gates' neighborhoods on
             * the sequential abstraction, under the given traversal parameters. Each gate itself
             * is included in its own neighborhood iff it is sequential.
             *
             * @param depth - Maximum traversal depth.
             * @param direction - Direction of traversal.
             * @param directed - Respect signal direction if true.
             * @param starting_pin_types - If non-empty, only pins of these types on the source
             *                             gate may start the traversal (does not affect later
             *                             levels; mirrors NeighborCount).
             * @param forbidden_pin_types - Pins of these types are never crossed.
             *
             * Legend (width 1): the shared sequential-neighborhood size.
             */
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
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Number of gates in the intersection of the two gates' neighborhoods on the
             * original abstraction, under the given traversal parameters. Each gate itself is
             * always included in its own neighborhood.
             *
             * Parameters mirror SharedSequentialNeighbors.
             *
             * Legend (width 1): the shared neighborhood size.
             */
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
                std::vector<std::string> get_legend(Context& ctx) const override;

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
