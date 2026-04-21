#pragma once

#include "graph_algorithm/netlist_graph.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/enums.h"
#include "z3_utils/subgraph_function_generation.h"

#include <algorithm>
#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

/**
 * Scalar type used for every numeric feature produced by the machine_learning plugin.
 * Kept as a macro so downstream code (including Python bindings and tensor conversions)
 * can depend on a single, centrally-defined width.
 */
#define FEATURE_TYPE float

namespace hal
{
    class Gate;
    class Netlist;

    namespace machine_learning
    {
        /**
         * Aggregation modes for reducing a feature vector to a single scalar.
         * Used by feature extractors that need to summarize per-pin or per-neighbor values.
         */
        enum class StatisticalMoment
        {
            min,        /**< Minimum value. */
            max,        /**< Maximum value. */
            average,    /**< Arithmetic mean. */
            median,     /**< Median (average of the two middle values for even-sized inputs). */
            stddev      /**< Population standard deviation. */
        };

        /**
         * Reduce a feature vector to a single scalar using the selected statistical moment.
         *
         * @param[in] moment - The aggregation mode to apply.
         * @param[in] data - The samples to aggregate. Must be non-empty; passed by value because
         *                   `median` sorts in place.
         * @returns The aggregated value on success, an error if `data` is empty.
         */
        Result<FEATURE_TYPE> calculate_statistical_moment(StatisticalMoment moment, std::vector<FEATURE_TYPE> data);

    }    // namespace machine_learning

    template<>
    std::map<machine_learning::StatisticalMoment, std::string> EnumStrings<machine_learning::StatisticalMoment>::data;

    namespace machine_learning
    {
        /**
         * Identifies the originating toolchain of a netlist (vendor, synthesizer, backend).
         * Different flavors encode multi-bit signals in different ways, so downstream
         * heuristics need to know which conventions to expect.
         */
        enum class NetlistFlavor
        {
            Default,    /**< Unknown or generic netlist with no vendor-specific assumptions. */
            Yosys,      /**< Open-source Yosys synthesizer output. */
            Vivado,     /**< Xilinx Vivado synthesizer output. */
        };

        /**
         * Tie-breaking policy used when multiple candidate words claim the same set of gates
         * during multi-bit word reconstruction (see MultiBitInformation).
         */
        enum class MultiBitProcessingPolicy
        {
            Default,    /**< Lowest average wire-assignment distance wins (the closest-named source nets). */
            Yosys,      /**< Prefer output-pin-derived words, then pin-type priority, then Default. */
            Vivado      /**< Prefer gate-name-encoded words (direction=none, all distances 0), then Default. */
        };

        /**
         * Map a NetlistFlavor to its corresponding MultiBitProcessingPolicy.
         * This is the single source of truth for that mapping; callers should prefer this
         * over hard-coding a policy.
         */
        constexpr MultiBitProcessingPolicy policy_from_flavor(const NetlistFlavor f) noexcept
        {
            switch (f)
            {
                case NetlistFlavor::Yosys:
                    return MultiBitProcessingPolicy::Yosys;
                case NetlistFlavor::Vivado:
                    return MultiBitProcessingPolicy::Vivado;
                default:
                    return MultiBitProcessingPolicy::Default;
            }
        }

        /**
         * Result of multi-bit word reconstruction for a netlist.
         *
         * A "word" is identified by the triple (identifier_name, pin_direction, pin_name) and groups
         * gates that, together, were likely synthesized from a single multi-bit signal in the source
         * description. This struct exposes the bidirectional mapping between gates and words, plus
         * the per-gate bit index within each word.
         *
         * Populated by Context::get_multi_bit_information().
         */
        struct MultiBitInformation
        {
        public:
            /**
             * Test whether two gates share at least one reconstructed word with the given direction.
             * Useful for grouping gates that are likely bits of the same bus.
             *
             * @param[in] direction - Direction to filter on; PinDirection::inout disables the filter.
             * @param[in] g_a - First gate.
             * @param[in] g_b - Second gate.
             * @returns True if both gates are members of a common word, false otherwise (also false
             *          if either gate has no reconstructed words).
             */
            bool are_gates_considered_a_pair(const PinDirection& direction, const Gate* g_a, const Gate* g_b) const;

            /**
             * Compare the bit indices of two gates within a shared word.
             * If several shared words exist, the smallest one (by summed gate count) is used as the
             * reference, which tends to pick the tightest/most specific word.
             *
             * @param[in] direction - Direction to filter on; PinDirection::inout disables the filter.
             * @param[in] g_a - First gate.
             * @param[in] g_b - Second gate.
             * @returns True if `g_a`'s index is strictly smaller, false otherwise, or std::nullopt
             *          if the gates have no shared word in the requested direction.
             */
            std::optional<bool> is_index_a_smaller_index_b(const PinDirection& direction, const Gate* g_a, const Gate* g_b) const;

            /** Word (identifier, direction, pin) -> gates forming the word, ordered by gate id. */
            std::map<std::tuple<std::string, PinDirection, std::string>, std::vector<Gate*>> word_to_gates;

            /** Gate -> all words the gate is a member of. */
            std::map<const Gate*, std::vector<std::tuple<std::string, PinDirection, std::string>>> gate_to_words;

            /** (gate, word) -> bit index of the gate within that word. */
            std::map<std::pair<const Gate*, std::tuple<std::string, PinDirection, std::string>>, u32> gate_word_to_index;
        };

        /**
         * Shared state and lazy cache bundle for machine-learning feature and label extraction.
         *
         * A single Context holds a netlist pointer together with cached derived structures that are
         * expensive to compute (netlist abstractions, graph projections, Boolean subgraph functions,
         * Boolean influence maps, multi-bit word reconstruction, gate-type / pin-type indexing).
         * All cached getters are thread-safe via double-checked locking and compute their result on
         * first access; subsequent calls return the cached pointer.
         *
         * The NetlistFlavor fixes both the MultiBitProcessingPolicy and any future vendor-specific
         * decisions in one place.
         */
        struct Context
        {
        public:
            Context() = delete;

            /**
             * Build a Context over an existing netlist. The netlist must outlive the Context.
             *
             * The constructor eagerly collects all gates and all sequential gates (both sorted by id
             * for determinism) and builds index maps for gate types and their pins. All heavier
             * derived state is computed lazily on demand.
             *
             * @param[in] netlist - Netlist this context operates on; pointer is stored, no ownership.
             * @param[in] flavor - Originating toolchain. Determines the multi-bit processing policy via
             *                     policy_from_flavor(). If the policy ever needs to be overridden
             *                     independently, add an optional parameter at this site.
             * @param[in] _num_threads - Hint for parallelizable feature/label computations. Exposed
             *                           publicly as `num_threads` for call sites to honor.
             * @param[in] max_boolean_function_input_size - If > 0, any combinational subgraph whose
             *                                              input net count exceeds this threshold is
             *                                              skipped when building the Boolean function
             *                                              and Boolean influence caches. 0 means no
             *                                              limit.
             */
            Context(const Netlist* netlist, const NetlistFlavor flavor, const u32 _num_threads = 1, const u32 max_boolean_function_input_size = 0)
                : nl(netlist), num_threads(_num_threads), m_netlist_flavor(flavor), m_mbi_policy(policy_from_flavor(flavor)), m_max_boolean_function_input_size(max_boolean_function_input_size)
            {
                m_gates = netlist->get_gates();
                std::sort(m_gates.begin(), m_gates.end(), [](const auto* g_a, const auto* g_b) { return g_a->get_id() < g_b->get_id(); });

                m_sequential_gates = netlist->get_gates([](const auto* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
                std::sort(m_sequential_gates.begin(), m_sequential_gates.end(), [](const auto* g_a, const auto* g_b) { return g_a->get_id() < g_b->get_id(); });

                const auto gate_types_unordered                           = netlist->get_gate_library()->get_gate_types();
                const std::map<std::string, GateType*> gate_types_ordered = {gate_types_unordered.begin(), gate_types_unordered.end()};

                u32 gt_index = 0;
                for (const auto& [_gt_name, gt] : gate_types_ordered)
                {
                    m_gate_type_indices.insert({gt, gt_index});
                    gt_index++;

                    const auto pins                          = gt->get_pins();
                    std::vector<const GatePin*> pins_ordered = {pins.begin(), pins.end()};
                    std::sort(pins_ordered.begin(), pins_ordered.end(), [](const auto& p_a, const auto& p_b) { return p_a->get_name() < p_b->get_name(); });

                    u32 gt_pin_index = 0;
                    for (const auto& pin : pins_ordered)
                    {
                        m_gate_pin_indices[gt].insert({pin, gt_pin_index});
                        gt_pin_index++;
                    }
                }
            };

            /**
             * Get a netlist abstraction over the sequential gates only, with clock/ground/power
             * endpoints filtered out on both sides. Lazily built and cached.
             */
            const Result<NetlistAbstraction*> get_sequential_abstraction();

            /**
             * Get a netlist abstraction over all gates with no endpoint filtering. Lazily built
             * and cached.
             */
            const Result<NetlistAbstraction*> get_original_abstraction();

            /** Graph projection of the sequential abstraction. Lazily built and cached. */
            const Result<graph_algorithm::NetlistGraph*> get_sequential_netlist_graph();

            /** Graph projection of the original abstraction. Lazily built and cached. */
            const Result<graph_algorithm::NetlistGraph*> get_original_netlist_graph();

            /** Sorted union of every GateTypeProperty present in the netlist's gate library. */
            const std::vector<GateTypeProperty>& get_possible_gate_type_properties();

            /** Sorted union of every PinType present in the netlist's gate library. */
            const std::vector<PinType>& get_possible_pin_types();

            /** Sorted union of every PinDirection present in the netlist's gate library. */
            const std::vector<PinDirection>& get_possible_pin_directions();

            /**
             * Reconstructed multi-bit word information for the sequential gates, tie-broken using
             * the policy derived from the NetlistFlavor. Lazily built and cached.
             */
            const MultiBitInformation& get_multi_bit_information();

            /**
             * @brief Compute (once) the combinational subgraph function inputs for every sequential gate in the netlist as well as all global output nets.  Clock, ground, and power pins are excluded.
             * The result is cached; repeated calls return the cached pointer instantly.
             * 
             * @returns Pointer to the cache (sequential gate input net → input nets) on success, an error otherwise.
             */
            const Result<std::unordered_map<const Net*, u32>*> get_sequential_subgraph_function_inputs();

            /**
             * Compute (once) the z3 combinational subgraph function for every input net of every
             * sequential gate in the netlist as well as all global output nets.  Clock, ground, and power pins are excluded.
             * The result is cached; repeated calls return the cached pointer instantly.
             *
             * @returns Pointer to the cache on success, an error otherwise.
             */
            const Result<std::unordered_map<const Net*, z3::expr>*> get_sequential_subgraph_functions();

            /**
             * Compute (once) the Boolean influence for every data-input net of every sequential gate.
             * Calls get_sequential_subgraph_functions() internally and then runs
             * get_boolean_influence_bitsliced on each resulting z3 expression.
             * The result is cached; repeated calls return the cached pointer instantly.
             *
             * @returns Pointer to the cache (input net → variable-name → influence) on success, an error otherwise.
             */
            const Result<std::unordered_map<const Net*, std::unordered_map<Net*, double>>*> get_sequential_boolean_influences();

            /**
             * Compute (once) the reverse influence index for all sequential gates.
             * Calls get_sequential_boolean_influences() internally, converts variable names back to Net*
             * via BooleanFunctionNetDecorator, and builds a map:
             *   source_net  →  list of input nets whose Boolean function the source net appears in.
             * The result is cached; repeated calls return the cached pointer instantly.
             *
             * @returns Pointer to the cache (source net → influenced D-pin nets) on success, an error otherwise.
             */
            const Result<std::unordered_map<const Net*, std::vector<Net*>>*> get_sequential_influenced_nets();

            /** All gates in the netlist, sorted ascending by gate id. */
            const std::vector<Gate*>& get_gates() const;

            /** All sequential gates in the netlist, sorted ascending by gate id. */
            const std::vector<Gate*>& get_sequential_gates() const;

            /**
             * Stable index of a gate type within the netlist's gate library.
             * Indices are assigned alphabetically by gate-type name and are suitable as
             * one-hot or embedding indices.
             */
            u32 get_gate_type_index(const GateType* gt) const;

            /**
             * Stable index of a pin within its owning gate type.
             * Indices are assigned alphabetically by pin name within each gate type.
             */
            u32 get_gate_pin_index(const GateType* gt, const GatePin* gp) const;

            /** Flavor the context was constructed with. Exposed for introspection. */
            NetlistFlavor get_netlist_flavor() const
            {
                return m_netlist_flavor;
            }

            /** Multi-bit processing policy derived from the flavor. Exposed for introspection. */
            MultiBitProcessingPolicy get_multi_bit_processing_policy() const
            {
                return m_mbi_policy;
            }

            const Netlist* nl;       /**< Non-owning pointer to the netlist this context wraps. */
            const u32 num_threads;   /**< Parallelism hint for feature/label computations. */

        private:
            NetlistFlavor m_netlist_flavor        = NetlistFlavor::Default;
            MultiBitProcessingPolicy m_mbi_policy = MultiBitProcessingPolicy::Default;

            std::vector<Gate*> m_gates;
            std::vector<Gate*> m_sequential_gates;

            std::map<const GateType*, u32> m_gate_type_indices;
            std::map<const GateType*, std::map<const GatePin*, u32>> m_gate_pin_indices;

            std::shared_ptr<NetlistAbstraction> m_original_abstraction{nullptr};
            std::shared_ptr<NetlistAbstraction> m_sequential_abstraction{nullptr};
            std::shared_ptr<graph_algorithm::NetlistGraph> m_original_netlist_graph{nullptr};
            std::shared_ptr<graph_algorithm::NetlistGraph> m_sequential_netlist_graph{nullptr};
            std::shared_ptr<std::vector<GateTypeProperty>> m_possible_gate_type_properties{nullptr};
            std::shared_ptr<std::vector<PinType>> m_possible_pin_types{nullptr};
            std::shared_ptr<std::vector<PinDirection>> m_possible_pin_directions{nullptr};
            std::shared_ptr<MultiBitInformation> m_mbi{nullptr};

            // Boolean / z3 caches. z3::context is not thread-safe, so every read and write of
            // any of the four caches below is serialized via m_functional_mutex.
            std::shared_ptr<z3::context> m_z3_ctx{nullptr};
            std::shared_ptr<std::unordered_map<const Net*, u32>> m_sequential_subgraph_function_inputs_cache{nullptr};
            std::shared_ptr<std::unordered_map<const Net*, z3::expr>> m_sequential_subgraph_function_cache{nullptr};
            std::shared_ptr<std::unordered_map<const Net*, std::unordered_map<Net*, double>>> m_sequential_boolean_influence_cache{nullptr};
            std::shared_ptr<std::unordered_map<const Net*, std::vector<Net*>>> m_sequential_influenced_nets_cache{nullptr};

            u32 m_max_boolean_function_input_size = 0;

            // Mutexes for thread-safe initialization
            std::mutex m_original_abstraction_mutex;
            std::mutex m_sequential_abstraction_mutex;
            std::mutex m_original_graph_mutex;
            std::mutex m_sequential_graph_mutex;
            std::mutex m_possible_gate_type_properties_mutex;
            std::mutex m_possible_pin_types_mutex;
            std::mutex m_possible_pin_directions_mutex;
            std::mutex m_mbi_mutex;
            // Single mutex for all z3/functional cache operations (z3::context is not thread-safe)
            std::mutex m_functional_mutex;
        };

        /**
         * Direction flag passed to graph construction routines to select whether the resulting
         * netlist graph preserves signal direction or treats edges as symmetric.
         */
        enum GraphDirection
        {
            undirected,
            directed,
        };
    }    // namespace machine_learning
}    // namespace hal
