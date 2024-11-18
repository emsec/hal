/**
 * @file gate_pair_label.h
 * @brief This file contains classes and functions for labeling pairs of gates within a machine learning context.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"
#include "machine_learning/types.h"

#include <map>
#include <optional>
#include <vector>

namespace hal
{
    /* Forward declarations */
    class Gate;
    class Netlist;
    enum class PinDirection : int;

    namespace machine_learning
    {
        namespace gate_pair_label
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
                /**
                 * @brief Maps word pairs to corresponding gates.
                 */
                std::map<std::tuple<std::string, PinDirection, std::string>, std::vector<Gate*>> word_to_gates;

                /**
                 * @brief Maps gates to associated word pairs.
                 */
                std::map<const Gate*, std::vector<std::tuple<std::string, PinDirection, std::string>>> gate_to_words;
            };

            /**
             * @struct LabelContext
             * @brief Provides context for gate-pair labeling within a netlist.
             *
             * This struct is initialized with a reference to the netlist and the gates involved in the labeling. 
             * It also provides access to multi-bit information for use in labeling calculations.
             */
            struct LabelContext
            {
                /**
                 * @brief Deleted default constructor to enforce initialization with parameters.
                 */
                LabelContext() = delete;

                /**
                 * @brief Constructs a `LabelContext` with the specified netlist and gates.
                 * @param[in] netlist - The netlist to which the gates belong.
                 * @param[in] gates - The gates to be labeled.
                 */
                LabelContext(const Netlist* netlist, const std::vector<Gate*>& gates) : nl(netlist), gates{gates} {};

                /**
                 * @brief Retrieves the multi-bit information, initializing it if not already done.
                 * @returns A constant reference to the `MultiBitInformation` object.
                 */
                const MultiBitInformation& get_multi_bit_information();

                /**
                 * @brief The netlist to which the gates belong.
                 */
                const Netlist* nl;

                /**
                 * @brief The gates that are part of this labeling context.
                 */
                const std::vector<Gate*> gates;

                /**
                 * @brief Optional storage for multi-bit information, initialized on demand.
                 */
                std::optional<MultiBitInformation> mbi;
            };

            /**
             * @class GatePairLabel
             * @brief Base class for calculating gate pairs and labels for machine learning models.
             *
             * This abstract class provides methods for calculating gate pairs and labels based on various criteria.
             */
            class GatePairLabel
            {
            public:
                /**
                 * @brief Calculate gate pairs based on the provided labeling context and netlist.
                 * @param[in] lc - The labeling context.
                 * @param[in] nl - The netlist to operate on.
                 * @param[in] gates - The gates to be paired.
                 * @returns A vector of gate pairs on success, an error otherwise.
                 */
                virtual Result<std::vector<std::pair<const Gate*, const Gate*>>> calculate_gate_pairs(LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates) const = 0;

                /**
                 * @brief Calculate labels for a given gate pair.
                 * @param[in] lc - The labeling context.
                 * @param[in] g_a - The first gate in the pair.
                 * @param[in] g_b - The second gate in the pair.
                 * @returns A vector of labels on success, an error otherwise.
                 */
                virtual Result<std::vector<u32>> calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const = 0;

                /**
                 * @brief Calculate labels for multiple gate pairs.
                 * @param[in] lc - The labeling context.
                 * @param[in] gate_pairs - The gate pairs to label.
                 * @returns A vector of label vectors for each pair on success, an error otherwise.
                 */
                virtual Result<std::vector<std::vector<u32>>> calculate_labels(LabelContext& lc, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const = 0;

                /**
                 * @brief Calculate both gate pairs and their labels within the labeling context.
                 * @param[in] lc - The labeling context.
                 * @returns A pair containing gate pairs and corresponding labels on success, an error otherwise.
                 */
                virtual Result<std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>>> calculate_labels(LabelContext& lc) const = 0;
            };

            /**
             * @class SharedSignalGroup
             * @brief Labels gate pairs based on shared signal groups.
             */
            class SharedSignalGroup : public GatePairLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                SharedSignalGroup(){};

                Result<std::vector<std::pair<const Gate*, const Gate*>>> calculate_gate_pairs(LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates) const override;
                Result<std::vector<u32>> calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(LabelContext& lc, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const override;
                Result<std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>>> calculate_labels(LabelContext& lc) const override;
            };

            /**
             * @class SharedConnection
             * @brief Labels gate pairs based on shared connections.
             */
            class SharedConnection : public GatePairLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                SharedConnection(){};

                Result<std::vector<std::pair<const Gate*, const Gate*>>> calculate_gate_pairs(LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates) const override;
                Result<std::vector<u32>> calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(LabelContext& lc, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const override;
                Result<std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>>> calculate_labels(LabelContext& lc) const override;
            };
        }    // namespace gate_pair_label
    }    // namespace machine_learning
}    // namespace hal