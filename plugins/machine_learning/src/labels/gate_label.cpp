#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/utils.h"
#include "machine_learning/labels/gate_label.h"
#include "netlist_preprocessing/netlist_preprocessing.h"
#include "nlohmann/json.hpp"

namespace hal
{
    namespace machine_learning
    {
        namespace gate_label
        {
            Result<std::vector<u32>> GateNameKeyWord::calculate_label(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                bool is_applicable = m_applicable_to.empty();
                if (!m_applicable_to.empty())
                {
                    for (const auto& gtp : m_applicable_to)
                    {
                        if (g->get_type()->has_property(gtp))
                        {
                            is_applicable = true;
                            break;
                        }
                    }
                }

                if (is_applicable)
                {
                    const auto& gate_name = g->get_name();
                    if (gate_name.find(m_key_word) != std::string::npos)
                    {
                        return OK(MATCH);
                    }

                    return OK(MISMATCH);
                }

                return OK(NA);
            }

            Result<std::vector<std::vector<u32>>> GateNameKeyWord::calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& g : gates)
                {
                    const auto new_label = calculate_label(ctx, g);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(), "Cannot caluclate label for gate  " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::vector<std::vector<u32>>> GateNameKeyWord::calculate_labels(Context& ctx) const
            {
                const auto labels = calculate_labels(ctx, ctx.get_gates());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK(labels.get());
            }

            std::string GateNameKeyWord::to_string() const
            {
                std::string applicable_to_str = utils::join("_", m_applicable_to.begin(), m_applicable_to.end(), [](const GateTypeProperty& gtp) { return enum_to_string(gtp); });

                return "GateNameKeyWord_" + m_key_word + "_" + (m_applicable_to.empty() ? "ALL" : applicable_to_str);
            }

            Result<std::vector<u32>> NetNameKeyWord::calculate_label(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                for (const auto& gtp : m_applicable_to)
                {
                    if (g->get_type()->has_property(gtp))
                    {
                        if (!g->has_data("preprocessing_information", "multi_bit_indexed_identifiers"))
                        {
                            log_error("machine_learning", "unable to find indexed identifiers for gate with ID {}", g->get_id());
                            continue;
                        }

                        const std::string json_string = std::get<1>(g->get_data("preprocessing_information", "multi_bit_indexed_identifiers"));

                        nlohmann::json j                                                         = nlohmann::json::parse(json_string);
                        std::vector<netlist_preprocessing::indexed_identifier> index_information = j.get<std::vector<netlist_preprocessing::indexed_identifier>>();

                        // for each pin, only consider the index information with the least distance
                        std::map<std::string, u32> pin_to_min_distance;
                        for (const auto& [_name, _index, _origin, pin, _direction, distance] : index_information)
                        {
                            if (const auto it = pin_to_min_distance.find(pin); it == pin_to_min_distance.end())
                            {
                                pin_to_min_distance.insert({pin, distance});
                            }
                            else
                            {
                                pin_to_min_distance.at(pin) = std::min(it->second, distance);
                            }
                        }

                        std::map<std::string, std::string> pin_to_net_name;
                        for (const auto& [name, _index, _origin, pin, _direction, distance] : index_information)
                        {
                            if (pin_to_min_distance.at(pin) == distance)
                            {
                                pin_to_net_name.insert({pin, name});
                            }
                        }

                        for (const auto& pt : m_pin_types)
                        {
                            const auto& pins = g->get_type()->get_pins([&pt](const auto& gt_p) { return gt_p->get_type() == pt; });
                            for (const auto* p : pins)
                            {
                                std::string net_name;
                                if (const auto it = pin_to_net_name.find(p->get_name()); it != pin_to_net_name.end())
                                {
                                    net_name = pin_to_net_name.at(p->get_name());
                                }
                                else
                                {
                                    const auto* ep = (p->get_direction() == PinDirection::input) ? g->get_fan_in_endpoint(p) : g->get_fan_out_endpoint(p);
                                    net_name       = ep->get_net()->get_name();
                                }

                                if (net_name.find(m_key_word) != std::string::npos)
                                {
                                    return OK(MATCH);
                                }
                            }
                        }

                        return OK(MISMATCH);
                    }
                }

                return OK(NA);
            }

            Result<std::vector<std::vector<u32>>> NetNameKeyWord::calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& g : gates)
                {
                    const auto new_label = calculate_label(ctx, g);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(), "Cannot caluclate label for gate  " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::vector<std::vector<u32>>> NetNameKeyWord::calculate_labels(Context& ctx) const
            {
                const auto labels = calculate_labels(ctx, ctx.get_gates());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK(labels.get());
            }

            std::string NetNameKeyWord::to_string() const
            {
                std::string pin_types_str     = utils::join("_", m_pin_types.begin(), m_pin_types.end(), [](const PinType& pt) { return enum_to_string(pt); });
                std::string applicable_to_str = utils::join("_", m_applicable_to.begin(), m_applicable_to.end(), [](const GateTypeProperty& gtp) { return enum_to_string(gtp); });

                return "NetNameKeyWord_" + m_key_word + "_" + pin_types_str + "_" + (m_applicable_to.empty() ? "ALL" : applicable_to_str);
            }

            Result<std::vector<u32>> GateNameKeyWords::calculate_label(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                bool is_applicable = m_applicable_to.empty();
                if (!m_applicable_to.empty())
                {
                    for (const auto& gtp : m_applicable_to)
                    {
                        if (g->get_type()->has_property(gtp))
                        {
                            is_applicable = true;
                            break;
                        }
                    }
                }

                if (is_applicable)
                {
                    const auto& gate_name = g->get_name();
                    std::vector<u32> matches;
                    for (u32 i = 0; i < m_key_words.size(); i++)
                    {
                        if (gate_name.find(m_key_words.at(i)) != std::string::npos)
                        {
                            matches.push_back(i);
                        }
                    }

                    if (!matches.empty())
                    {
                        const auto match_res = make_match(matches);
                        if (match_res.is_error())
                        {
                            return ERR_APPEND(match_res.get_error(), "failed to build label for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                        }
                        return OK(match_res.get());
                    }

                    return OK(NO_MATCH);
                }

                return OK(NA);
            }

            Result<std::vector<std::vector<u32>>> GateNameKeyWords::calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& g : gates)
                {
                    const auto new_label = calculate_label(ctx, g);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(), "Cannot caluclate label for gate  " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::vector<std::vector<u32>>> GateNameKeyWords::calculate_labels(Context& ctx) const
            {
                const auto labels = calculate_labels(ctx, ctx.get_gates());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK(labels.get());
            }

            std::string GateNameKeyWords::to_string() const
            {
                std::string applicable_to_str = utils::join("_", m_applicable_to.begin(), m_applicable_to.end(), [](const GateTypeProperty& gtp) { return enum_to_string(gtp); });
                std::string key_words_to_str  = utils::join("_", m_key_words.begin(), m_key_words.end(), [](const std::string& s) { return s; });

                return "GateNameKeyWords_" + key_words_to_str + "_" + (m_applicable_to.empty() ? "ALL" : applicable_to_str);
            }

            Result<std::vector<u32>> NetNameKeyWords::calculate_label(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                bool is_applicable = m_applicable_to.empty();
                if (!m_applicable_to.empty())
                {
                    for (const auto& gtp : m_applicable_to)
                    {
                        if (g->get_type()->has_property(gtp))
                        {
                            is_applicable = true;
                            break;
                        }
                    }
                }

                if (is_applicable)
                {
                    std::vector<u32> matches;
                    for (u32 i = 0; i < m_key_words.size(); i++)
                    {
                        std::set<Net*> nets_to_check;
                        for (const auto& in_ep : g->get_fan_in_endpoints())
                        {
                            const auto pin_type = in_ep->get_pin()->get_type();
                            if (std::find(m_pin_types.begin(), m_pin_types.end(), pin_type) != m_pin_types.end())
                            {
                                if (in_ep->get_net() != nullptr)
                                {
                                    nets_to_check.insert(in_ep->get_net());
                                }
                            }
                        }

                        for (const auto& out_ep : g->get_fan_out_endpoints())
                        {
                            const auto pin_type = out_ep->get_pin()->get_type();
                            if (std::find(m_pin_types.begin(), m_pin_types.end(), pin_type) != m_pin_types.end())
                            {
                                if (out_ep->get_net() != nullptr)
                                {
                                    nets_to_check.insert(out_ep->get_net());
                                }
                            }
                        }

                        for (const auto& net : nets_to_check)
                        {
                            const std::string net_name = net->get_name();
                            if (net_name.find(m_key_words.at(i)) != std::string::npos)
                            {
                                matches.push_back(i);
                            }
                        }
                    }

                    if (!matches.empty())
                    {
                        const auto match_res = make_match(matches);
                        if (match_res.is_error())
                        {
                            return ERR_APPEND(match_res.get_error(), "failed to build label for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                        }
                        return OK(match_res.get());
                    }

                    return OK(NO_MATCH);
                }

                return OK(NA);
            }

            Result<std::vector<std::vector<u32>>> NetNameKeyWords::calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& g : gates)
                {
                    const auto new_label = calculate_label(ctx, g);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(), "Cannot caluclate label for gate  " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::vector<std::vector<u32>>> NetNameKeyWords::calculate_labels(Context& ctx) const
            {
                const auto labels = calculate_labels(ctx, ctx.get_gates());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK(labels.get());
            }

            std::string NetNameKeyWords::to_string() const
            {
                std::string key_words_to_str  = utils::join("_", m_key_words.begin(), m_key_words.end(), [](const std::string& s) { return s; });
                std::string pin_types_str     = utils::join("_", m_pin_types.begin(), m_pin_types.end(), [](const PinType& pt) { return enum_to_string(pt); });
                std::string applicable_to_str = utils::join("_", m_applicable_to.begin(), m_applicable_to.end(), [](const GateTypeProperty& gtp) { return enum_to_string(gtp); });

                return "NetNameKeyWords_" + key_words_to_str + "_" + pin_types_str + "_" + (m_applicable_to.empty() ? "ALL" : applicable_to_str);
            }

            namespace
            {
                /**
                 * @brief Determines which keyword groups match a given name string.
                 * 
                 * This function checks if the provided name contains any keyword from one or more
                 * groups of keywords. Each keyword group is represented as a vector of strings.
                 * 
                 * - When `allow_multiple` is true, it returns the indices of all groups where at least
                 *   one keyword appears in the name.
                 * - When `allow_multiple` is false, it returns only the index of the group containing
                 *   the longest matching keyword (with ties broken by lowest group index).
                 * - Case sensitivity can be controlled via the `case_sensitive` parameter.
                 * 
                 * @param name              The input name to check.
                 * @param key_words         A vector of groups of keywords.
                 * @param allow_multiple    If true, returns all matching group indices; 
                 *                          if false, returns only the group with the longest match.
                 * @param case_sensitive    If false, the comparison ignores case.
                 * @return std::vector<u32> List of matching keyword group indices.
                 */
                std::vector<u32> find_matching_keywords(const std::string& name, const std::vector<std::vector<std::string>>& key_words, const bool allow_multiple, const bool case_sensitive)
                {
                    std::vector<u32> result;
                    if (name.empty() || key_words.empty())
                    {
                        return result;
                    }

                    const std::string name_cmp = case_sensitive ? name : utils::to_lower(name);

                    size_t best_len = 0;
                    int best_group  = -1;

                    for (u32 gi = 0; gi < static_cast<u32>(key_words.size()); ++gi)
                    {
                        const auto& group  = key_words[gi];
                        bool group_matched = false;

                        for (const auto& kw : group)
                        {
                            if (kw.empty())
                            {
                                continue;
                            }

                            const std::string kw_cmp = case_sensitive ? kw : utils::to_lower(kw);
                            if (name_cmp.find(kw_cmp) != std::string::npos)
                            {
                                group_matched = true;

                                if (!allow_multiple)
                                {
                                    const size_t len = kw_cmp.size();
                                    if (len > best_len || (len == best_len && (best_group < 0 || gi < static_cast<u32>(best_group))))
                                    {
                                        best_len   = len;
                                        best_group = static_cast<int>(gi);
                                    }
                                }
                                else
                                {
                                    // For multiple matches, we can stop after the first keyword found in this group
                                    break;
                                }
                            }
                        }

                        if (allow_multiple && group_matched)
                        {
                            result.push_back(gi);
                        }
                    }

                    if (!allow_multiple && best_group >= 0)
                    {
                        result.push_back(static_cast<u32>(best_group));
                    }

                    return result;
                }
            }    // namespace

            Result<std::vector<u32>> ModuleNameKeyWords::calculate_label(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                bool is_applicable = m_applicable_to.empty();
                if (!m_applicable_to.empty())
                {
                    for (const auto& gtp : m_applicable_to)
                    {
                        if (g->get_type()->has_property(gtp))
                        {
                            is_applicable = true;
                            break;
                        }
                    }
                }

                if (is_applicable)
                {
                    const auto modules = g->get_modules(nullptr, m_recursive);
                    std::set<u32> matches;
                    for (const auto& m : modules)
                    {
                        const auto module_name = m->get_name();
                        const auto m_matches   = find_matching_keywords(module_name, m_key_words, m_allow_multiple, false);
                        matches.insert(m_matches.begin(), m_matches.end());
                    }

                    if (!matches.empty())
                    {
                        const auto match_res = make_match(utils::to_vector(matches));
                        if (match_res.is_error())
                        {
                            return ERR_APPEND(match_res.get_error(), "failed to build label for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                        }
                        return OK(match_res.get());
                    }

                    return OK(NO_MATCH);
                }

                return OK(NA);
            }

            Result<std::vector<std::vector<u32>>> ModuleNameKeyWords::calculate_labels(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& g : gates)
                {
                    const auto new_label = calculate_label(ctx, g);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(), "Cannot caluclate label for gate  " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::vector<std::vector<u32>>> ModuleNameKeyWords::calculate_labels(Context& ctx) const
            {
                const auto labels = calculate_labels(ctx, ctx.get_gates());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK(labels.get());
            }

            std::string ModuleNameKeyWords::to_string() const
            {
                std::string applicable_to_str = utils::join("_", m_applicable_to.begin(), m_applicable_to.end(), [](const GateTypeProperty& gtp) { return enum_to_string(gtp); });
                std::string key_words_to_str  = utils::join("__",    // separator between groups (double underscore for clarity)
                                                           m_key_words.begin(),
                                                           m_key_words.end(),
                                                           [](const std::vector<std::string>& group) { return utils::join("_", group.begin(), group.end(), [](const std::string& s) { return s; }); });

                return "ModuleNameKeyWords_" + key_words_to_str + "_" + (m_applicable_to.empty() ? "ALL" : applicable_to_str + "_" + (m_recursive ? "RECURSIVE" : ""));
            }

        }    // namespace gate_label
    }    // namespace machine_learning
}    // namespace hal