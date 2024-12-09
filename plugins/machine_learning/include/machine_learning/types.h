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

            /**
             * @brief Maps word pairs to corresponding gates.
             */
            std::map<std::tuple<std::string, PinDirection, std::string>, std::vector<Gate*>> word_to_gates;

            /**
             * @brief Maps gates to associated word pairs.
             */
            std::map<const Gate*, std::vector<std::tuple<std::string, PinDirection, std::string>>> gate_to_words;
        };

        struct Context
        {
        public:
            Context() = delete;
            Context(const Netlist* netlist, const u32 _num_threads = 1) : nl(netlist), num_threads(_num_threads){};

            const Result<NetlistAbstraction*> get_sequential_abstraction();
            const Result<NetlistAbstraction*> get_original_abstraction();
            const Result<graph_algorithm::NetlistGraph*> get_sequential_netlist_graph();
            const Result<graph_algorithm::NetlistGraph*> get_original_netlist_graph();
            const std::vector<GateTypeProperty>& get_possible_gate_type_properties();
            const MultiBitInformation& get_multi_bit_information();

            const Netlist* nl;
            const u32 num_threads;

        private:
            std::shared_ptr<NetlistAbstraction> m_original_abstraction{nullptr};
            std::shared_ptr<NetlistAbstraction> m_sequential_abstraction{nullptr};
            std::shared_ptr<graph_algorithm::NetlistGraph> m_original_netlist_graph{nullptr};
            std::shared_ptr<graph_algorithm::NetlistGraph> m_sequential_netlist_graph{nullptr};
            std::shared_ptr<std::vector<GateTypeProperty>> m_possible_gate_type_properties{nullptr};
            std::shared_ptr<MultiBitInformation> m_mbi{nullptr};

            // Mutexes for thread-safe initialization
            std::mutex m_original_abstraction_mutex;
            std::mutex m_sequential_abstraction_mutex;
            std::mutex m_original_graph_mutex;
            std::mutex m_sequential_graph_mutex;
            std::mutex m_possible_gate_type_properties_mutex;
            std::mutex m_mbi_mutex;
        };

        enum GraphDirection
        {
            undirected,
            directed,
        };
    }    // namespace machine_learning
}    // namespace hal