/**
 * @file gate_pair_label.h
 * @brief This file contains classes and functions for labeling pairs of gates within a machine learning context.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"
#include "machine_learning/types.h"

#include <map>
#include <vector>

namespace hal
{
    /* Forward declarations */
    class Gate;
    enum class PinDirection : int;

    namespace machine_learning
    {
        namespace gate_pair_label
        {
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
                 * @param[in] ctx - The machine learning context.
                 * @param[in] nl - The netlist to operate on.
                 * @param[in] gates - The gates to be paired.
                 * @returns A vector of gate pairs on success, an error otherwise.
                 */
                virtual Result<std::vector<std::pair<Gate*, Gate*>>> calculate_gate_pairs(Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates) const = 0;

                /**
                 * @brief Calculate labels for a given gate pair.
                 * @param[in] ctx - The machine learning context.
                 * @param[in] g_a - The first gate in the pair.
                 * @param[in] g_b - The second gate in the pair.
                 * @returns A vector of labels on success, an error otherwise.
                 */
                virtual Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g_a, const Gate* g_b) const = 0;

                /**
                 * @brief Calculate labels for multiple gate pairs.
                 * @param[in] ctx - The machine learning context.
                 * @param[in] gate_pairs - The gate pairs to label.
                 * @returns A vector of label vectors for each pair on success, an error otherwise.
                 */
                virtual Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const = 0;

                /**
                 * @brief Calculate both gate pairs and their labels within the labeling context.
                 * @param[in] ctx - The machine learning context.
                 * @returns A pair containing gate pairs and corresponding labels on success, an error otherwise.
                 */
                virtual Result<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> calculate_labels(Context& ctx) const = 0;

                virtual std::string to_string() const = 0;
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
                SharedSignalGroup(const PinDirection& direction, const u32 min_pair_count, const double negative_to_positive_factor)
                    : m_direction(direction), m_min_pair_count(min_pair_count), m_negative_to_positive_factor(negative_to_positive_factor){};

                const std::vector<u32> SHARED_GROUP      = {1, 0};
                const std::vector<u32> INDEPENDENT_GROUP = {0, 1};

                Result<std::vector<std::pair<Gate*, Gate*>>> calculate_gate_pairs(Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates) const override;
                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const override;
                Result<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const u32 m_min_pair_count;
                const double m_negative_to_positive_factor;
            };

            /**
             * @class BitIndexOrdering
             * @brief Labels gate pairs based on their order in a shared control word.
             */
            class BitIndexOrdering : public GatePairLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                BitIndexOrdering(const PinDirection& direction, const u32 min_pair_count, const double negative_to_positive_factor)
                    : m_direction(direction), m_min_pair_count(min_pair_count), m_negative_to_positive_factor(negative_to_positive_factor){};

                const std::vector<u32> LOWER  = {1, 0, 0};
                const std::vector<u32> HIGHER = {0, 1, 0};
                const std::vector<u32> NA     = {0, 0, 1};

                Result<std::vector<std::pair<Gate*, Gate*>>> calculate_gate_pairs(Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates) const override;
                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const override;
                Result<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;

            private:
                const PinDirection m_direction;
                const u32 m_min_pair_count;
                const double m_negative_to_positive_factor;
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

                Result<std::vector<std::pair<Gate*, Gate*>>> calculate_gate_pairs(Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates) const override;
                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g_a, const Gate* g_b) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const override;
                Result<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;
            };
        }    // namespace gate_pair_label
    }        // namespace machine_learning
}    // namespace hal