#pragma once

#include "graph_algorithm/netlist_graph.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#define FEATURE_TYPE float

namespace hal
{
    /* Forward declaration */
    class Gate;
    class Netlist;

    namespace machine_learning
    {
        /**
         * @struct MultiBitInformation
         * @brief Holds mappings between word labels and gates, and gates and word labels.
         *
         * This struct provides a bi-directional mapping between specific word pairs and their corresponding gates, 
         * as well as between gates and associated word pairs.
         */
        struct MultiBitInformation
        {
        public:
            bool are_gates_considered_a_pair(const PinDirection& direction, const Gate* g_a, const Gate* g_b) const;
            std::optional<bool> is_index_a_smaller_index_b(const PinDirection& direction, const Gate* g_a, const Gate* g_b) const;

            /**
             * @brief Maps word pairs to corresponding gates.
             */
            std::map<std::tuple<std::string, PinDirection, std::string>, std::vector<Gate*>> word_to_gates;

            /**
             * @brief Maps gates to associated word pairs.
             */
            std::map<const Gate*, std::vector<std::tuple<std::string, PinDirection, std::string>>> gate_to_words;

            std::map<std::pair<const Gate*, std::tuple<std::string, PinDirection, std::string>>, u32> gate_word_to_index;
        };

        struct Context
        {
        public:
            Context() = delete;
            Context(const Netlist* netlist, const u32 _num_threads = 1) : nl(netlist), num_threads(_num_threads)
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

            const Result<NetlistAbstraction*> get_sequential_abstraction();
            const Result<NetlistAbstraction*> get_original_abstraction();

            const Result<graph_algorithm::NetlistGraph*> get_sequential_netlist_graph();
            const Result<graph_algorithm::NetlistGraph*> get_original_netlist_graph();

            const std::vector<GateTypeProperty>& get_possible_gate_type_properties();
            const std::vector<PinType>& get_possible_pin_types();
            const std::vector<PinDirection>& get_possible_pin_directions();

            const MultiBitInformation& get_multi_bit_information();

            const std::vector<Gate*>& get_gates() const;
            const std::vector<Gate*>& get_sequential_gates() const;

            u32 get_gate_type_index(const GateType* gt) const;
            u32 get_gate_pin_index(const GateType* gt, const GatePin* gp) const;

            const Netlist* nl;
            const u32 num_threads;

        private:
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

            // Mutexes for thread-safe initialization
            std::mutex m_original_abstraction_mutex;
            std::mutex m_sequential_abstraction_mutex;
            std::mutex m_original_graph_mutex;
            std::mutex m_sequential_graph_mutex;
            std::mutex m_possible_gate_type_properties_mutex;
            std::mutex m_possible_pin_types_mutex;
            std::mutex m_possible_pin_directions_mutex;
            std::mutex m_mbi_mutex;
        };

        enum GraphDirection
        {
            undirected,
            directed,
        };
    }    // namespace machine_learning
}    // namespace hal