#pragma once

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
            Context(const Netlist* netlist, const u32 num_threads = 1) : nl(netlist), num_threads(num_threads){};

            const Result<NetlistAbstraction*> get_sequential_abstraction();
            const Result<NetlistAbstraction*> get_original_abstraction();
            const std::vector<GateTypeProperty>& get_possible_gate_type_properties();
            const MultiBitInformation& get_multi_bit_information();

            const Netlist* nl;
            const u32 num_threads;

        private:
            std::optional<NetlistAbstraction> m_sequential_abstraction;
            std::optional<NetlistAbstraction> m_original_abstraction;
            std::optional<std::vector<GateTypeProperty>> m_possible_gate_type_properties;
            std::optional<MultiBitInformation> m_mbi;
        };

        enum GraphDirection
        {
            undirected,
            directed,
        };
    }    // namespace machine_learning
}    // namespace hal