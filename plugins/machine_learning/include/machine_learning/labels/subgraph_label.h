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
    enum class PinType : int;

    namespace machine_learning
    {
        namespace subgraph_label
        {
            /**
             * @class SubgraphLabel
             * @brief Base class for calculating labels for machine learning models.
             *
             * This abstract class provides methods for calculating labels based on various criteria.
             */
            class SubgraphLabel
            {
            public:
                /**
                 * @brief Calculate subgraphs based on the provided labeling context.
                 * @param[in] ctx - The machine learning context.
                 * @returns A vector of subgrahs on success, an error otherwise.
                 */
                virtual Result<std::vector<std::vector<Gate*>>> calculate_subgraphs(Context& ctx) const = 0;

                /**
                 * @brief Calculate labels for multiple gate.
                 * @param[in] ctx - The machine learning context.
                 * @returns A vector of label vectors for each pair on success, an error otherwise.
                 */
                virtual Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& subgraphs) const = 0;

                virtual std::string to_string() const = 0;
            };

            /**
             * @class ContainedComponents
             * @brief Class for generating labels representing all included operations/components in a subgraph.
             */
            class ContainedComponents : public SubgraphLabel
            {
            public:
                /**
                 * @brief Calculate gate pairs based on the provided labeling context and netlist.
                 * @param[in] ctx - The machine learning context.
                 * @returns A vector of subgrahs on success, an error otherwise.
                 */
                Result<std::vector<std::vector<Gate*>>> calculate_subgraphs(Context& ctx) const override;

                /**
                 * @brief Calculate labels for multiple gate.
                 * @param[in] ctx - The machine learning context.
                 * @returns A vector of label vectors for each pair on success, an error otherwise.
                 */
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& subgraphs) const override;

                /**
                 * @brief Helper function to annotate contained components to a falttened subgraph of gates in front of a register by analyzing a unflattened twin netlist 
                 * 
                 * @return 
                 */
                Result<u32> annotate_from_twin_netlist(Context& ctx, Netlist* nl, const Netlist* twin_nl) const;

                std::string to_string() const override;
            };

        }    // namespace subgraph_label
    }    // namespace machine_learning
}    // namespace hal