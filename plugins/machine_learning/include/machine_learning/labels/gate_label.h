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

            /**
             * @class NetNameKeyWord
             * @brief Labels gate based on whether one of the net names at specified pins includes a keyword or not.
             */
            class NetNameKeyWord : public GateLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                NetNameKeyWord(const std::string& key_word, const std::vector<PinType>& pin_types, const std::vector<GateTypeProperty>& applicable_to = {})
                    : m_key_word(key_word), m_pin_types(pin_types), m_applicable_to(applicable_to){};

                const std::vector<u32> MATCH    = {1, 0, 0};
                const std::vector<u32> MISMATCH = {0, 1, 0};
                const std::vector<u32> NA       = {0, 0, 1};

                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;

            private:
                const std::string m_key_word;
                const std::vector<PinType> m_pin_types;
                const std::vector<GateTypeProperty> m_applicable_to;
            };

            /**
             * @class GateNameKeyWords
             * @brief Labels gate based on which keyword of a selection it matches
             */
            class GateNameKeyWords : public GateLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                GateNameKeyWords(const std::vector<std::string>& key_words, const std::vector<GateTypeProperty>& applicable_to = {}, const bool allow_multiple = false)
                    : m_key_words(key_words), m_applicable_to(applicable_to), m_allow_multiple(allow_multiple), NO_MATCH(make_no_match()), NA(make_na()){};

                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;

            private:
                const std::vector<std::string> m_key_words;
                const std::vector<GateTypeProperty> m_applicable_to;
                const bool m_allow_multiple;

            public:
                const std::vector<u32> NO_MATCH;
                const std::vector<u32> NA;

            private:
                std::vector<u32> make_no_match() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);

                    // If the label is appblicable to all gates than we do not need the NA label
                    if (m_applicable_to.empty())
                    {
                        v.insert(v.end(), {1});
                    }
                    else
                    {
                        v.insert(v.end(), {1, 0});
                    }

                    return v;
                }

                std::vector<u32> make_na() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);
                    if (m_applicable_to.empty())
                    {
                        log_error("machine_learning", "generating a Not Applicable label for a label that is applicable to all gate types");
                        v.insert(v.end(), {0});
                    }
                    else
                    {
                        v.insert(v.end(), {0, 1});
                    }

                    return v;
                }

                Result<std::vector<u32>> make_match(const u32 i) const
                {
                    if (i >= m_key_words.size())
                    {
                        return ERR("cannot construct a match for keyword at index " + std::to_string(i) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                    }

                    return make_match(std::vector<u32>{i});
                }

                Result<std::vector<u32>> make_match(const std::vector<u32>& i) const
                {
                    if (!m_allow_multiple && i.size() > 1)
                    {
                        return ERR("cannot construct a match label for " + std::to_string(i.size()) + " keywords, when the label does not allow for multiple matches!");
                    }

                    if (i.empty())
                    {
                        return ERR("cannot construct a match label without any matching keywords. Construct a no match instead.");
                    }

                    u32 label_size = m_key_words.size() + 1;
                    if (!m_applicable_to.empty())
                    {
                        // if the label is not applicable to all gate types we need to account for any possible NA labels
                        label_size += 1;
                    }

                    std::vector<u32> v(label_size, 0);
                    for (const auto ii : i)
                    {
                        if (ii >= m_key_words.size())
                        {
                            return ERR("cannot construct a match for keyword at index " + std::to_string(ii) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                        }

                        v[ii] = 1;
                    }
                    return OK(v);
                }
            };

            /**
             * @class NetNameKeyWords
             * @brief Labels gate based on whether one of the net names at specified pins matches keywords of a selection or no
             */
            class NetNameKeyWords : public GateLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                NetNameKeyWords(const std::vector<std::string>& key_words,
                                const std::vector<PinType>& pin_types,
                                const std::vector<GateTypeProperty>& applicable_to = {},
                                const bool allow_multiple                          = false)
                    : m_key_words(key_words), m_pin_types(pin_types), m_applicable_to(applicable_to), m_allow_multiple(allow_multiple), NO_MATCH(make_no_match()), NA(make_na()){};

                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;

            private:
                const std::vector<std::string> m_key_words;
                const std::vector<PinType> m_pin_types;
                const std::vector<GateTypeProperty> m_applicable_to;
                const bool m_allow_multiple;

            public:
                const std::vector<u32> NO_MATCH;
                const std::vector<u32> NA;

            private:
                std::vector<u32> make_no_match() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);

                    // If the label is appblicable to all gates than we do not need the NA label
                    if (m_applicable_to.empty())
                    {
                        v.insert(v.end(), {1});
                    }
                    else
                    {
                        v.insert(v.end(), {1, 0});
                    }

                    return v;
                }

                std::vector<u32> make_na() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);
                    if (m_applicable_to.empty())
                    {
                        log_error("machine_learning", "generating a Not Applicable label for a label that is applicable to all gate types");
                        v.insert(v.end(), {0});
                    }
                    else
                    {
                        v.insert(v.end(), {0, 1});
                    }

                    return v;
                }

                Result<std::vector<u32>> make_match(const u32 i) const
                {
                    if (i >= m_key_words.size())
                    {
                        return ERR("cannot construct a match for keyword at index " + std::to_string(i) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                    }

                    return make_match(std::vector<u32>{i});
                }

                Result<std::vector<u32>> make_match(const std::vector<u32>& i) const
                {
                    if (!m_allow_multiple && i.size() > 1)
                    {
                        return ERR("cannot construct a match label for " + std::to_string(i.size()) + " keywords, when the label does not allow for multiple matches!");
                    }

                    if (i.empty())
                    {
                        return ERR("cannot construct a match label without any matching keywords. Construct a no match instead.");
                    }

                    u32 label_size = m_key_words.size() + 1;
                    if (!m_applicable_to.empty())
                    {
                        // if the label is not applicable to all gate types we need to account for any possible NA labels
                        label_size += 1;
                    }

                    std::vector<u32> v(label_size, 0);
                    for (const auto ii : i)
                    {
                        if (ii >= m_key_words.size())
                        {
                            return ERR("cannot construct a match for keyword at index " + std::to_string(ii) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                        }

                        v[ii] = 1;
                    }
                    return OK(v);
                }
            };

            /**
             * @class ModuleNameKeyWords
             * @brief Labels gate based on which keyword of its parent module(s) name is matching
             */
            class ModuleNameKeyWords : public GateLabel
            {
            public:
                /**
                 * @brief Default constructor.
                 */
                ModuleNameKeyWords(const std::vector<std::string>& key_words, const std::vector<GateTypeProperty>& applicable_to = {}, const bool recursive = false, const bool allow_multiple = false)
                    : m_key_words(key_words), m_applicable_to(applicable_to), m_recursive(recursive), m_allow_multiple(allow_multiple), NO_MATCH(make_no_match()), NA(make_na()){};

                Result<std::vector<u32>> calculate_label(Context& ctx, const Gate* g) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const override;
                Result<std::vector<std::vector<u32>>> calculate_labels(Context& ctx) const override;
                std::string to_string() const override;

            private:
                const std::vector<std::string> m_key_words;
                const std::vector<GateTypeProperty> m_applicable_to;
                const bool m_recursive;
                const bool m_allow_multiple;

            public:
                const std::vector<u32> NO_MATCH;
                const std::vector<u32> NA;

            private:
                std::vector<u32> make_no_match() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);

                    // If the label is appblicable to all gates than we do not need the NA label
                    if (m_applicable_to.empty())
                    {
                        v.insert(v.end(), {1});
                    }
                    else
                    {
                        v.insert(v.end(), {1, 0});
                    }

                    return v;
                }

                std::vector<u32> make_na() const
                {
                    std::vector<u32> v(m_key_words.size(), 0);
                    if (m_applicable_to.empty())
                    {
                        log_error("machine_learning", "generating a Not Applicable label for a label that is applicable to all gate types");
                        v.insert(v.end(), {0});
                    }
                    else
                    {
                        v.insert(v.end(), {0, 1});
                    }

                    return v;
                }

                Result<std::vector<u32>> make_match(const u32 i) const
                {
                    if (i >= m_key_words.size())
                    {
                        return ERR("cannot construct a match for keyword at index " + std::to_string(i) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                    }

                    return make_match(std::vector<u32>{i});
                }

                Result<std::vector<u32>> make_match(const std::vector<u32>& i) const
                {
                    if (!m_allow_multiple && i.size() > 1)
                    {
                        return ERR("cannot construct a match label for " + std::to_string(i.size()) + " keywords, when the label does not allow for multiple matches!");
                    }

                    if (i.empty())
                    {
                        return ERR("cannot construct a match label without any matching keywords. Construct a no match instead.");
                    }

                    u32 label_size = m_key_words.size() + 1;
                    if (!m_applicable_to.empty())
                    {
                        // if the label is not applicable to all gate types we need to account for any possible NA labels
                        label_size += 1;
                    }

                    std::vector<u32> v(label_size, 0);
                    for (const auto ii : i)
                    {
                        if (ii >= m_key_words.size())
                        {
                            return ERR("cannot construct a match for keyword at index " + std::to_string(ii) + " as there are only " + std::to_string(m_key_words.size()) + " many keywords!");
                        }

                        v[ii] = 1;
                    }
                    return OK(v);
                }
            };

        }    // namespace gate_label
    }    // namespace machine_learning
}    // namespace hal