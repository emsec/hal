/**
 * @file gate_label.h
 * @brief This file contains classes and functions for labeling gates within a machine learning context.
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
    enum class GateTypeProperty : int;

    namespace machine_learning
    {
        namespace gate_label
        {
            /**
             * @class GateLabel
             * @brief Base class for calculating labels for machine learning models.
             *
             * This abstract class provides methods for calculating labels based on various criteria.
             */
            class GateLabel
            {
            public:
                /**
                 * @brief Calculate labels for a given gate pair.
                 * @param[in] ctx - The machine learning context.
                 * @param[in] g - The gate
                 * @returns A vector of labels on success, an error otherwise.
                 */
                virtual Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g) const = 0;

                /**
                 * @brief Calculate labels for multiple gate.
                 * @param[in] ctx - The machine learning context.
                 * @param[in] gate_pairs - The gate to label.
                 * @returns A vector of label vectors for each pair on success, an error otherwise.
                 */
                virtual Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const = 0;

                /**
                 * @brief Calculate labels within the labeling context.
                 * @param[in] ctx - The machine learning context.
                 * @returns A labels on success, an error otherwise.
                 */
                virtual Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx) const = 0;

                virtual std::string to_string() const = 0;
            };

            /**
             * @class GateNameKeyWord
             * @brief Labels gate based on whether their name includes a keyword or not.
             */
            class GateNameKeyWord : public GateLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                GateNameKeyWord(const std::string& key_word, const std::vector<GateTypeProperty>& applicable_to = {}) : m_key_word(key_word), m_applicable_to(applicable_to){};

                const std::vector<u32> MATCH    = {1, 0, 0};
                const std::vector<u32> MISMATCH = {0, 1, 0};
                const std::vector<u32> NA       = {0, 0, 1};

                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;

            private:
                const std::string m_key_word;
                const std::vector<GateTypeProperty> m_applicable_to;
            };
        }    // namespace gate_label
    }        // namespace machine_learning
}    // namespace hal