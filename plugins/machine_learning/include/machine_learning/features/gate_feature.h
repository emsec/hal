#pragma once

#include "hal_core/defines.h"
#include "machine_learning/types.h"
#include "machine_learning/utilities/normalization.h"

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
            /**
             * Abstract base class for per-gate feature extractors.
             *
             * A GateFeature maps a gate (or a batch of gates, sharing a Context) to a fixed-width
             * vector of FEATURE_TYPE values. The width and semantic meaning of the vector is
             * identical for every gate processed with the same Context and is described by
             * get_legend().
             *
             * Subclasses must either:
             *   - override the single-gate variant calculate_feature(ctx, gate), in which case the
             *     batch variant calculate_feature(ctx, gates) automatically parallelizes over
             *     ctx.num_threads; OR
             *   - override the batch variant directly (e.g., for graph-level computations such as
             *     centrality, where per-gate extraction is not well-defined).
             *
             * Every concrete subclass must override:
             *   - to_string(): a stable, unique, parameter-encoded name of the feature (used for
             *                  caching, logging, and as the basis of legend labels);
             *   - get_legend(ctx): one label per element of the produced feature vector.
             */
            class GateFeature
            {
            public:
                /**
                 * Compute the feature vector for a single gate.
                 * Default implementation returns an error; subclasses that compute per-gate must
                 * override this. Subclasses that only make sense at the batch level (e.g.,
                 * centrality) may leave this unimplemented and override the batch overload below
                 * instead.
                 *
                 * @param[in] ctx - Shared ML context; used for cached abstractions, gate-type
                 *                  indexing, and Boolean-influence caches.
                 * @param[in] g - The gate to extract the feature for.
                 * @returns The feature vector on success, an error otherwise. The returned vector
                 *          has length equal to get_legend(ctx).size().
                 */
                virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const;

                /**
                 * Compute feature vectors for a batch of gates.
                 * The default implementation dispatches to the single-gate variant in parallel
                 * across ctx.num_threads. Override this directly for batch-only features (e.g.,
                 * whole-graph centralities) where the per-gate form is not applicable.
                 *
                 * @param[in] ctx - Shared ML context.
                 * @param[in] gates - Gates to extract features for; output order matches input order.
                 * @returns One feature vector per input gate on success, an error otherwise. Every
                 *          returned inner vector has length equal to get_legend(ctx).size().
                 */
                virtual Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const;

                /**
                 * Stable, parameter-encoded name of the feature, suitable as a cache key or log tag.
                 * The format is deterministic across runs and encodes every constructor parameter
                 * that influences the output, so two feature instances with the same to_string()
                 * produce bit-identical output for the same context.
                 */
                virtual std::string to_string() const = 0;

                /**
                 * Concise per-index description of the feature vector produced by
                 * calculate_feature(ctx, ...).
                 *
                 * The returned vector's size equals the width of the feature vector, and entry
                 * i is a short label describing what the value at index i represents. Labels
                 * are purely descriptive and are NOT prefixed with to_string(); a caller that
                 * needs globally unique names across concatenated features should build that
                 * mapping itself (e.g., by combining to_string() with the legend entries).
                 *
                 * The main purpose is to let downstream code (logging, serialized tensors, ML
                 * experiments run without the netlist in hand) recover the meaning of each
                 * index — e.g., for GateTypeOneHot, to know which gate-type property each
                 * column corresponds to without needing to reload the gate library.
                 *
                 * Some features have context-dependent widths (e.g., GateTypeOneHot scales
                 * with the set of gate-type properties present in the netlist's gate library),
                 * which is why the Context is a parameter.
                 *
                 * @param[in] ctx - Shared ML context. Not modified in practice, but taken by
                 *                  non-const ref to match calculate_feature and to allow
                 *                  queries such as ctx.get_possible_gate_type_properties()
                 *                  which lazily cache.
                 * @returns One concise descriptor per index of the feature vector.
                 */
                virtual std::vector<std::string> get_legend(Context& ctx) const = 0;
            };

            /**
             * Counts global input and output nets directly connected to a gate.
             *
             * Legend (width 2): connected_global_inputs, connected_global_outputs.
             */
            class ConnectedGlobalIOs : public GateFeature
            {
            public:
                ConnectedGlobalIOs(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Shortest-path distance from a gate to the nearest global input or output, measured
             * on the original netlist abstraction. Distances are clamped to 255 and default to 255
             * when no path exists.
             *
             * @param direction - Search toward global inputs (PinDirection::input) or global
             *                    outputs (PinDirection::output).
             * @param directed - If true, respect signal direction; if false, treat the netlist
             *                   as undirected.
             * @param forbidden_pin_types - Endpoints on pins of these types are skipped during
             *                              traversal (e.g., to ignore resets or clocks).
             *
             * Legend (width 1): the distance itself.
             */
            class DistanceGlobalIO : public GateFeature
            {
            public:
                DistanceGlobalIO(const PinDirection& direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Same as DistanceGlobalIO but evaluated on the sequential abstraction of the netlist
             * (combinational gates collapsed). Useful for reasoning in terms of pipeline depth
             * rather than gate count.
             *
             * Legend (width 1): the distance itself.
             */
            class SequentialDistanceGlobalIO : public GateFeature
            {
            public:
                SequentialDistanceGlobalIO(const PinDirection& direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
                    : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Number of pins on a gate's type that match the configured direction and pin-type
             * filters. Purely a gate-type property — does NOT depend on which nets are actually
             * connected to the instance.
             *
             * Use multiple instances to build richer feature vectors, e.g.
             *   PinCount({PinDirection::input})                          -> number of input pins
             *   PinCount({PinDirection::output})                         -> number of output pins
             *   PinCount({}, {PinType::clock})                           -> number of clock pins
             *   PinCount({PinDirection::input}, {PinType::data})         -> number of data inputs
             *   PinCount()                                               -> total pin count
             *
             * @param directions - Accepted PinDirection values. Empty means "accept any direction".
             * @param pin_types - Accepted PinType values. Empty means "accept any type".
             *
             * A pin is counted iff it passes BOTH filters (AND semantics; empty filter is the
             * identity).
             *
             * Legend (width 1): a single descriptor encoding the active filter set.
             */
            class PinCount : public GateFeature
            {
            public:
                PinCount(const std::vector<PinDirection>& directions = {}, const std::vector<PinType>& pin_types = {}) : m_directions(directions), m_pin_types(pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const std::vector<PinDirection> m_directions;
                const std::vector<PinType> m_pin_types;
            };

            /**
             * One-hot encoding of a gate's GateTypeProperty set.
             *
             * A gate type can expose multiple properties (e.g., `combinational` + `lut`), so the
             * result is technically multi-hot: each index in the vector corresponds to a property
             * present in the gate library, and its value is the number of times that property
             * appears on the gate's type (typically 0 or 1).
             *
             * Width and index order are determined by Context::get_possible_gate_type_properties(),
             * which is sorted alphabetically and deterministic across runs for a given gate library.
             *
             * Legend (width = number of distinct GateTypeProperty values in the gate library):
             * one label per property, in the same order as get_possible_gate_type_properties().
             */
            class GateTypeOneHot : public GateFeature
            {
            public:
                GateTypeOneHot(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Integer index of a gate's type within the gate library, suitable as an embedding
             * lookup key (see Context::get_gate_type_index).
             *
             * NOTE: currently declared but not implemented; reserved for future use.
             */
            class GateTypeIndex : public GateFeature
            {
            public:
                GateTypeIndex(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Count of gates reachable from the source gate within a bounded traversal on the
             * original netlist abstraction.
             *
             * @param depth - Maximum traversal depth (in gate hops). Endpoints whose depth exceeds
             *                this are not visited.
             * @param direction - Direction of traversal (input-side, output-side, or both).
             * @param directed - If true, follow signal direction; if false, treat edges as symmetric.
             * @param starting_pin_types - If non-empty, only pins of these types on the source gate
             *                             are used to start the traversal. Later levels are not
             *                             restricted by this filter (this lets you, e.g., look at
             *                             the upstream cone of just the reset pin while still
             *                             letting traversal cross reset pins of downstream gates —
             *                             unless those are forbidden via `forbidden_pin_types`).
             * @param forbidden_pin_types - Pins of these types are never crossed.
             *
             * Legend (width 1): the neighborhood size.
             */
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
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Same as NeighborCount but evaluated on the sequential abstraction (combinational
             * logic collapsed), counting the reachable sequential gates.
             *
             * Legend (width 1): the neighborhood size.
             */
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
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Distribution of GateTypeProperty values over the neighborhood defined by the same
             * traversal parameters as NeighborCount. For each property present in the gate
             * library, counts how many times it appears across the types of gates in the
             * neighborhood.
             *
             * Width and index order mirror GateTypeOneHot (determined by
             * Context::get_possible_gate_type_properties()).
             *
             * Legend (width = number of distinct GateTypeProperty values in the gate library):
             * one label per property, in the same order as get_possible_gate_type_properties().
             */
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
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const u32 m_depth;
                const PinDirection m_direction;
                const bool m_directed;
                const std::vector<PinType> m_starting_pin_types;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Betweenness centrality of each gate on the original netlist graph.
             *
             * @param directed - If true, use directed shortest paths; otherwise undirected.
             * @param cutoff - Maximum path length to consider (-1 means unbounded).
             * @param normalization - Normalization applied to the centrality values across the
             *                        batch before writing them into the feature vectors. See
             *                        NormalizationType.
             *
             * This is a batch-level feature (only the batch overload of calculate_feature is
             * implemented).
             *
             * Legend (width 1): the per-gate centrality.
             */
            class BetweennessCentrality : public GateFeature
            {
            public:
                BetweennessCentrality(const bool directed = true, const i32 cutoff = -1, const NormalizationType normalization = NormalizationType::None)
                    : m_directed(directed), m_cutoff(cutoff), m_normalization(normalization){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const bool m_directed;
                const i32 m_cutoff;
                const NormalizationType m_normalization;
            };

            /**
             * Harmonic centrality of each gate on the original netlist graph.
             *
             * @param direction - PinDirection::input / output selects the traversal direction;
             *                    PinDirection::inout uses both (undirected traversal).
             * @param cutoff - Maximum distance to consider (-1 means unbounded).
             * @param normalization - Normalization applied to the centrality values across the
             *                        batch before writing them into the feature vectors. See
             *                        NormalizationType.
             *
             * This is a batch-level feature (only the batch overload of calculate_feature is
             * implemented).
             *
             * Legend (width 1): the per-gate centrality.
             */
            class HarmonicCentrality : public GateFeature
            {
            public:
                HarmonicCentrality(const PinDirection& direction, const i32 cutoff = -1, const NormalizationType normalization = NormalizationType::None)
                    : m_direction(direction), m_cutoff(cutoff), m_normalization(normalization){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const PinDirection m_direction;
                const i32 m_cutoff;
                const NormalizationType m_normalization;
            };

            /**
             * Betweenness centrality evaluated on the sequential netlist graph (combinational
             * gates collapsed). Non-sequential gates in the input batch receive a centrality of 0.
             *
             * Legend (width 1): the per-gate centrality.
             */
            class SequentialBetweennessCentrality : public GateFeature
            {
            public:
                SequentialBetweennessCentrality(const bool directed = true, const i32 cutoff = -1, const NormalizationType normalization = NormalizationType::None)
                    : m_directed(directed), m_cutoff(cutoff), m_normalization(normalization){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const bool m_directed;
                const i32 m_cutoff;
                const NormalizationType m_normalization;
            };

            /**
             * Harmonic centrality evaluated on the sequential netlist graph (combinational gates
             * collapsed). Non-sequential gates in the input batch receive a centrality of 0.
             *
             * Legend (width 1): the per-gate centrality.
             */
            class SequentialHarmonicCentrality : public GateFeature
            {
            public:
                SequentialHarmonicCentrality(const PinDirection& direction, const i32 cutoff = -1, const NormalizationType normalization = NormalizationType::None)
                    : m_direction(direction), m_cutoff(cutoff), m_normalization(normalization){};

                Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const PinDirection m_direction;
                const i32 m_cutoff;
                const NormalizationType m_normalization;
            };

            /**
             * Binary indicator: is the gate part of a directed cycle on the original netlist
             * graph? Equivalent to "can the gate reach itself by following directed edges".
             *
             * Implementation: forward BFS from `g`'s output endpoints, returning 1 as soon as
             * an input endpoint of `g` is reached, 0 otherwise. The BFS is bounded by `cutoff`
             * (unbounded if negative), so a cycle longer than the cutoff is reported as 0.
             *
             * @param cutoff - Maximum cycle length to consider (-1 means unbounded).
             * @param forbidden_pin_types - Endpoints on pins of these types are not crossed
             *                              during traversal (e.g., to ignore reset / clock cones).
             *
             * Legend (width 1): the binary indicator.
             */
            class InCycle : public GateFeature
            {
            public:
                InCycle(const i32 cutoff = -1, const std::vector<PinType>& forbidden_pin_types = {}) : m_cutoff(cutoff), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const i32 m_cutoff;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Length of the shortest directed cycle through the gate on the original netlist
             * graph, clamped to 255. Gates that are not part of any cycle (within `cutoff`)
             * receive the clamp value.
             *
             * @param cutoff - Maximum cycle length to consider (-1 means unbounded). Cycles
             *                 longer than the cutoff are reported as the clamp value.
             * @param forbidden_pin_types - Endpoints on pins of these types are not crossed
             *                              during traversal.
             *
             * Legend (width 1): the cycle length.
             */
            class ShortestCycleLength : public GateFeature
            {
            public:
                ShortestCycleLength(const i32 cutoff = -1, const std::vector<PinType>& forbidden_pin_types = {}) : m_cutoff(cutoff), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const i32 m_cutoff;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Binary indicator: is the gate part of a closed directed walk of length exactly
             * `length` on the original netlist graph?
             *
             * Implementation: layer-by-layer expansion of "set of gates reachable in exactly k
             * steps from `g`" for k = 0..length, then test `g ∈ R_length`. Walks may revisit
             * intermediate gates; this is equivalent to A^length[g][g] > 0 on the directed
             * adjacency matrix.
             *
             * @param length - The walk length to test for (number of edges).
             * @param forbidden_pin_types - Endpoints on pins of these types are not crossed
             *                              during traversal.
             *
             * Legend (width 1): the binary indicator.
             */
            class InCycleOfLength : public GateFeature
            {
            public:
                InCycleOfLength(const u32 length, const std::vector<PinType>& forbidden_pin_types = {}) : m_length(length), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const u32 m_length;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Same as InCycle but evaluated on the sequential abstraction (combinational gates
             * collapsed). Non-sequential gates always receive 0.
             *
             * Legend (width 1): the binary indicator.
             */
            class SequentialInCycle : public GateFeature
            {
            public:
                SequentialInCycle(const i32 cutoff = -1, const std::vector<PinType>& forbidden_pin_types = {}) : m_cutoff(cutoff), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const i32 m_cutoff;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Same as ShortestCycleLength but evaluated on the sequential abstraction
             * (combinational gates collapsed). Non-sequential gates receive the clamp value.
             *
             * Legend (width 1): the cycle length.
             */
            class SequentialShortestCycleLength : public GateFeature
            {
            public:
                SequentialShortestCycleLength(const i32 cutoff = -1, const std::vector<PinType>& forbidden_pin_types = {}) : m_cutoff(cutoff), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const i32 m_cutoff;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Same as InCycleOfLength but evaluated on the sequential abstraction (combinational
             * gates collapsed). Non-sequential gates always receive 0.
             *
             * Legend (width 1): the binary indicator.
             */
            class SequentialInCycleOfLength : public GateFeature
            {
            public:
                SequentialInCycleOfLength(const u32 length, const std::vector<PinType>& forbidden_pin_types = {}) : m_length(length), m_forbidden_pin_types(forbidden_pin_types){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const u32 m_length;
                const std::vector<PinType> m_forbidden_pin_types;
            };

            /**
             * Boolean influence of a sequential gate's output nets on the downstream sequential
             * cone.
             *
             * For a sequential gate, collects, for every output-net / downstream-D-pin-net pair
             * the gate influences, the Boolean influence value from
             * Context::get_sequential_boolean_influences(), and reduces the resulting sample set
             * to one scalar per requested StatisticalMoment (min / max / average / median / stddev).
             *
             * Non-sequential gates, gates with no output nets, and gates whose influence cache
             * is empty return a zero vector.
             *
             * @param moments - Ordered list of statistical moments to compute; defaults to
             *                  {average}. The resulting feature vector has one element per
             *                  requested moment, in the given order.
             *
             * Legend (width = number of requested moments): one label per moment.
             */
            class BooleanInfluence : public GateFeature
            {
            public:
                BooleanInfluence(const std::vector<StatisticalMoment>& moments = {}) : m_moments(moments.empty() ? std::vector<StatisticalMoment>{StatisticalMoment::average} : moments){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;

            private:
                const std::vector<StatisticalMoment> m_moments;
            };

            // Feature ideas:
            // - number of sequential predecessors/successors (this is somewhat encoded in the neighboring gate types)
            // - number of predecessors/successors

            //  - distance to nearest type/module (e.g. RAM, DSP)
            //  - distance to nearest shift register
            //  - distance to nearest bus register

            //  - Graphlet Degree Vector (GDV) (possibly calculated using ORCA or gtrieScanner)

            // - number of self-loops of length n
            // - number of self-loops in a sequential abstraction of length n
            // - length of shortest self-loop
            // - length of shortest self-loop in sequential circuit

            // The following features are mostly applicable to endpoints of gates which would require an attapted version of netlist graph
            // An excpetion would be the case where a gate only has one output pin
            // - boolean influence (for flip flops) in all boolean functions it appears in as input (min, max, average, mean)
            // - signal probability (simulate n input vectors and approximate possibility of output signal to be 1)
            // - value when all inputs are 1 (highly dependent on control values)
            // - value when all inputs are 0 (highly dependent on control values)

            /**
             * Build a concatenated feature matrix for a list of gates by evaluating every
             * GateFeature in `features` and horizontally stacking the resulting vectors.
             *
             * The i-th row of the returned matrix is the concatenation of
             * features[0].calculate_feature(gates[i]) ++ features[1].calculate_feature(gates[i])
             * ++ ... The column layout can be reconstructed by concatenating each feature's
             * get_legend(ctx) in the same order.
             *
             * This overload creates a fresh Context(NetlistFlavor::Default) on the gates' netlist;
             * use the overload below if you want to share a Context across multiple calls.
             */
            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);

            /**
             * Build a concatenated feature matrix for a list of gates using a caller-provided
             * Context. Prefer this overload when running many feature queries on the same netlist
             * so expensive lazy caches (abstractions, Boolean-influence maps, etc.) are reused.
             */
            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(Context& ctx, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates);
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal
