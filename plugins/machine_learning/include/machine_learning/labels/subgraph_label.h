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
                 * @brief Default constructor.
                 */
                ContainedComponents(const std::vector<std::string>& key_words, const bool binary_flags) : m_key_words(wrap(key_words)), m_binary_flags(binary_flags){};
                ContainedComponents(const std::vector<std::vector<std::string>>& key_words, const bool binary_flags) : m_key_words(key_words), m_binary_flags(binary_flags){};

                /**
                 * @brief Calculate register subgraphs based on the provided labeling context and netlist.
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

            private:
                const std::vector<std::vector<std::string>> m_key_words;
                const bool m_binary_flags;

            private:
                static std::vector<std::vector<std::string>> wrap(const std::vector<std::string>& v)
                {
                    std::vector<std::vector<std::string>> result;
                    result.reserve(v.size());
                    for (const auto& s : v)
                    {
                        result.push_back({s});
                    }
                    return result;
                }

                std::vector<u32> make_no_match() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);

                    v.insert(v.end(), {1});

                    return v;
                }

                Result<std::vector<u32>> make_match(const std::pair<u32, u32> match) const
                {
                    if (match.first >= m_key_words.size())
                    {
                        return ERR("cannot construct a match for keyword at index " + std::to_string(match.first) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                    }

                    return make_match(std::vector<std::pair<u32, u32>>{match});
                }

                Result<std::vector<u32>> make_match(const std::vector<std::pair<u32, u32>>& matches) const
                {
                    if (matches.empty())
                    {
                        return ERR("cannot construct a match label without any matching keywords. Construct a no match instead.");
                    }

                    u32 label_size = m_key_words.size() + 1;

                    std::vector<u32> v(label_size, 0);
                    for (const auto [m_idx, m_count] : matches)
                    {
                        if (m_idx >= m_key_words.size())
                        {
                            return ERR("cannot construct a match for keyword at index " + std::to_string(m_idx) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                        }

                        v[m_idx] = m_binary_flags ? 1 : m_count;
                    }
                    return OK(v);
                };
            };

            /**
             * @class ContainedComponentsNetlist
             * @brief Class for generating labels representing all included operations/components in a subgraph, in this case the netlist.
             */
            class ContainedComponentsNetlist : public SubgraphLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                ContainedComponentsNetlist(const std::vector<std::string>& key_words, const bool binary_flags) : m_key_words(wrap(key_words)), m_binary_flags(binary_flags){};
                ContainedComponentsNetlist(const std::vector<std::vector<std::string>>& key_words, const bool binary_flags) : m_key_words(key_words), m_binary_flags(binary_flags){};

                /**
                 * @brief Calculate register subgraphs based on the provided labeling context and netlist.
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
                 * @brief Helper function to annotate contained components to top module of netlöist by reading from netlist metadata at a path 
                 * 
                 * @return 
                 */
                Result<u32> annotate_from_netlist_metadata(Context& ctx, Netlist* nl, const std::string& metadata_path) const;

                std::string to_string() const override;

            private:
                const std::vector<std::vector<std::string>> m_key_words;
                const bool m_binary_flags;

            private:
                static std::vector<std::vector<std::string>> wrap(const std::vector<std::string>& v)
                {
                    std::vector<std::vector<std::string>> result;
                    result.reserve(v.size());
                    for (const auto& s : v)
                    {
                        result.push_back({s});
                    }
                    return result;
                }

                std::vector<u32> make_no_match() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);

                    v.insert(v.end(), {1});

                    return v;
                }

                Result<std::vector<u32>> make_match(const std::pair<u32, u32> match) const
                {
                    if (match.first >= m_key_words.size())
                    {
                        return ERR("cannot construct a match for keyword at index " + std::to_string(match.first) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                    }

                    return make_match(std::vector<std::pair<u32, u32>>{match});
                }

                Result<std::vector<u32>> make_match(const std::vector<std::pair<u32, u32>>& matches) const
                {
                    if (matches.empty())
                    {
                        return ERR("cannot construct a match label without any matching keywords. Construct a no match instead.");
                    }

                    u32 label_size = m_key_words.size() + 1;

                    std::vector<u32> v(label_size, 0);
                    for (const auto [m_idx, m_count] : matches)
                    {
                        if (m_idx >= m_key_words.size())
                        {
                            return ERR("cannot construct a match for keyword at index " + std::to_string(m_idx) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                        }

                        v[m_idx] = m_binary_flags ? 1 : m_count;
                    }
                    return OK(v);
                };
            };
        }    // namespace subgraph_label
    }    // namespace machine_learning
}    // namespace hal