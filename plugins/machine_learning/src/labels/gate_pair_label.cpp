#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "machine_learning/labels/gate_pair_label.h"
#include "machine_learning/types.h"

#include <stdlib.h>

namespace hal
{
    namespace machine_learning
    {
        namespace gate_pair_label
        {
            Result<std::vector<std::pair<Gate*, Gate*>>> SharedSignalGroup::calculate_gate_pairs(Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates) const
            {
                UNUSED(nl);

                const auto& mbi = ctx.get_multi_bit_information();

                std::vector<std::pair<Gate*, Gate*>> pairs;

                for (auto& g : gates)
                {
                    // positive labels
                    std::unordered_set<const Gate*> pos_gates;
                    if (mbi.gate_to_words.find(g) == mbi.gate_to_words.end())
                    {
                        // gate is only in a group with itself
                        // pairs.push_back({g, g});
                        pos_gates.insert(g);
                    }
                    else
                    {
                        // add all gates that are part of at least one other signal group as positive pair
                        for (const auto& name_direction : mbi.gate_to_words.at(g))
                        {
                            const auto& word_gates = mbi.word_to_gates.at(name_direction);
                            for (auto* g_i : word_gates)
                            {
                                if (g == g_i)
                                {
                                    continue;
                                }

                                if (mbi.are_gates_considered_a_pair(m_direction, g, g_i))
                                {
                                    pairs.push_back({g, g_i});
                                    pos_gates.insert(g_i);
                                }
                            }
                        }
                    }

                    // negative labels (equal amount to the positive labels)
                    const u64 pos_count         = pos_gates.size();
                    const u64 desired_neg_count = std::max(m_min_pair_count, u32(m_negative_to_positive_factor * pos_count));
                    const u64 real_neg_count    = std::min(gates.size() - pos_count, desired_neg_count);

                    // TODO remove debug print
                    // std::cout << "Gate ID: " << g->get_id() << "   " <<  pos_count << " vs. " << real_neg_count << std::endl;

                    std::set<Gate*> chosen_gates;
                    for (u32 i = 0; i < real_neg_count; i++)
                    {
                        const u32 start = std::rand() % gates.size();
                        for (u32 idx = start; idx < start + gates.size(); idx++)
                        {
                            const auto g_i = gates.at(idx % gates.size());
                            if (pos_gates.find(g_i) == pos_gates.end() && chosen_gates.find(g_i) == chosen_gates.end())
                            {
                                pairs.push_back({g, g_i});
                                chosen_gates.insert(g_i);
                                break;
                            }
                        }
                    }
                }

                return OK(pairs);
            };

            Result<std::vector<u32>> SharedSignalGroup::calculate_label(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                const auto& mbi = ctx.get_multi_bit_information();

                if (mbi.are_gates_considered_a_pair(m_direction, g_a, g_b))
                {
                    return OK(SHARED_GROUP);
                }

                return OK(INDEPENDENT_GROUP);
            };

            Result<std::vector<std::vector<u32>>> SharedSignalGroup::calculate_labels(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& gp : gate_pairs)
                {
                    const auto new_label = calculate_label(ctx, gp.first, gp.second);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(),
                                          "Cannot caluclate label for gate pair " + gp.first->get_name() + " with ID " + std::to_string(gp.first->get_id()) + " and " + gp.second->get_name()
                                              + " with ID " + std::to_string(gp.second->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> SharedSignalGroup::calculate_labels(Context& ctx) const
            {
                const auto ff_gates = ctx.nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::ff); });

                const auto pairs = calculate_gate_pairs(ctx, ctx.nl, ff_gates);
                if (pairs.is_error())
                {
                    return ERR_APPEND(pairs.get_error(), "Failed to calculate labels");
                }
                const auto labels = calculate_labels(ctx, pairs.get());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK({pairs.get(), labels.get()});
            };

            std::string SharedSignalGroup::to_string() const
            {
                return "SharedSignalGroup_" + enum_to_string(m_direction) + "_" + std::to_string(m_min_pair_count) + "_"
                       + std::to_string(static_cast<int>(m_negative_to_positive_factor * 100) / 100.0)
                             .substr(0, std::to_string(static_cast<int>(m_negative_to_positive_factor * 100) / 100.0).find('.') + 3);
            }

            Result<std::vector<std::pair<Gate*, Gate*>>> BitIndexOrdering::calculate_gate_pairs(Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates) const
            {
                UNUSED(nl);

                const auto& mbi = ctx.get_multi_bit_information();

                std::vector<std::pair<Gate*, Gate*>> pairs;

                for (auto& g : gates)
                {
                    // positive labels
                    std::unordered_set<const Gate*> pos_gates;
                    if (mbi.gate_to_words.find(g) == mbi.gate_to_words.end())
                    {
                        // gate is only in a group with itself
                        // pairs.push_back({g, g});
                        pos_gates.insert(g);
                    }
                    else
                    {
                        // add all gates that are part of at least one other signal group as positive pair
                        for (const auto& name_direction : mbi.gate_to_words.at(g))
                        {
                            const auto& word_gates = mbi.word_to_gates.at(name_direction);
                            for (auto* g_i : word_gates)
                            {
                                if (g == g_i)
                                {
                                    continue;
                                }

                                if (mbi.are_gates_considered_a_pair(m_direction, g, g_i))
                                {
                                    pairs.push_back({g, g_i});
                                    pos_gates.insert(g_i);
                                }
                            }
                        }
                    }

                    // negative labels (equal amount to the positive labels)
                    const u64 pos_count         = pos_gates.size();
                    const u64 desired_neg_count = std::max(m_min_pair_count, u32(m_negative_to_positive_factor * pos_count));
                    const u64 real_neg_count    = std::min(gates.size() - pos_count, desired_neg_count);

                    // TODO remove debug print
                    // std::cout << "Gate ID: " << g->get_id() << "   " <<  pos_count << " vs. " << real_neg_count << std::endl;

                    std::set<Gate*> chosen_gates;
                    for (u32 i = 0; i < real_neg_count; i++)
                    {
                        const u32 start = std::rand() % gates.size();
                        for (u32 idx = start; idx < start + gates.size(); idx++)
                        {
                            const auto g_i = gates.at(idx % gates.size());
                            if (pos_gates.find(g_i) == pos_gates.end() && chosen_gates.find(g_i) == chosen_gates.end())
                            {
                                pairs.push_back({g, g_i});
                                chosen_gates.insert(g_i);
                                break;
                            }
                        }
                    }
                }

                return OK(pairs);
            };

            Result<std::vector<u32>> BitIndexOrdering::calculate_label(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                const auto& mbi = ctx.get_multi_bit_information();

                const auto comp = mbi.is_index_a_smaller_index_b(m_direction, g_a, g_b);

                if (!comp.has_value())
                {
                    return OK(NA);
                }

                if (comp.value())
                {
                    return OK(LOWER);
                }

                return OK(HIGHER);
            };

            Result<std::vector<std::vector<u32>>> BitIndexOrdering::calculate_labels(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& gp : gate_pairs)
                {
                    const auto new_label = calculate_label(ctx, gp.first, gp.second);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(),
                                          "Cannot caluclate label for gate pair " + gp.first->get_name() + " with ID " + std::to_string(gp.first->get_id()) + " and " + gp.second->get_name()
                                              + " with ID " + std::to_string(gp.second->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> BitIndexOrdering::calculate_labels(Context& ctx) const
            {
                const auto ff_gates = ctx.nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::ff); });

                const auto pairs = calculate_gate_pairs(ctx, ctx.nl, ff_gates);
                if (pairs.is_error())
                {
                    return ERR_APPEND(pairs.get_error(), "Failed to calculate labels");
                }
                const auto labels = calculate_labels(ctx, pairs.get());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK({pairs.get(), labels.get()});
            };

            std::string BitIndexOrdering::to_string() const
            {
                return "BitIndexOrdering_" + enum_to_string(m_direction) + "_" + std::to_string(m_min_pair_count) + "_"
                       + std::to_string(static_cast<int>(m_negative_to_positive_factor * 100) / 100.0)
                             .substr(0, std::to_string(static_cast<int>(m_negative_to_positive_factor * 100) / 100.0).find('.') + 3);
            }

            namespace
            {
                std::unordered_set<Gate*> get_all_connected_gates(const Gate* g)
                {
                    std::unordered_set<Gate*> connected;
                    for (auto pre : g->get_unique_predecessors())
                    {
                        connected.insert(pre);
                    }

                    for (auto suc : g->get_unique_successors())
                    {
                        connected.insert(suc);
                    }

                    return connected;
                }

                std::unordered_set<const Gate*> get_all_connected_gates_const(const Gate* g)
                {
                    std::unordered_set<const Gate*> connected;
                    for (auto pre : g->get_unique_predecessors())
                    {
                        connected.insert(pre);
                    }

                    for (auto suc : g->get_unique_successors())
                    {
                        connected.insert(suc);
                    }

                    return connected;
                }
            }    // namespace

            Result<std::vector<std::pair<Gate*, Gate*>>> SharedConnection::calculate_gate_pairs(Context& ctx, const Netlist* nl, const std::vector<Gate*>& gates) const
            {
                UNUSED(ctx);
                UNUSED(nl);

                std::vector<std::pair<Gate*, Gate*>> pairs;

                for (const auto& g : gates)
                {
                    // positive labels
                    const auto all_connected = get_all_connected_gates(g);
                    for (const auto g_c : all_connected)
                    {
                        pairs.push_back({g, g_c});
                    }

                    // negative labels (equal amount to the positive labels)
                    const u32 neg_count = std::min(nl->get_gates().size() - all_connected.size(), all_connected.size());

                    // TODO remove
                    // std::cout << "Found " << all_connected.size() << " connections for gate " << g->get_id() << ". Trying to find " << neg_count << " opposites!" << std::endl;

                    std::set<Gate*> chosen_gates;
                    for (u32 i = 0; i < neg_count; i++)
                    {
                        const u32 start = std::rand() % gates.size();
                        for (u32 idx = start; idx < start + gates.size(); idx++)
                        {
                            const auto g_i = gates.at(idx % gates.size());
                            if (all_connected.find(g_i) == all_connected.end() && chosen_gates.find(g_i) == chosen_gates.end())
                            {
                                pairs.push_back({g, g_i});
                                chosen_gates.insert(g_i);
                                break;
                            }
                        }
                    }
                }

                return OK(pairs);
            };

            Result<std::vector<u32>> SharedConnection::calculate_label(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                UNUSED(ctx);

                const auto all_connected = get_all_connected_gates_const(g_a);

                if (all_connected.find(g_b) == all_connected.end())
                {
                    return OK({0});
                }

                return OK({1});
            };

            Result<std::vector<std::vector<u32>>> SharedConnection::calculate_labels(Context& ctx, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& gp : gate_pairs)
                {
                    const auto new_label = calculate_label(ctx, gp.first, gp.second);
                    if (new_label.is_error())
                    {
                        return ERR_APPEND(new_label.get_error(),
                                          "Cannot caluclate label for gate pair " + gp.first->get_name() + " with ID " + std::to_string(gp.first->get_id()) + " and " + gp.second->get_name()
                                              + " with ID " + std::to_string(gp.second->get_id()));
                    }

                    labels.push_back(new_label.get());
                }

                return OK(labels);
            }

            Result<std::pair<std::vector<std::pair<Gate*, Gate*>>, std::vector<std::vector<u32>>>> SharedConnection::calculate_labels(Context& ctx) const
            {
                const auto pairs = calculate_gate_pairs(ctx, ctx.nl, ctx.nl->get_gates());
                if (pairs.is_error())
                {
                    return ERR_APPEND(pairs.get_error(), "Failed to calculate labels");
                }
                const auto labels = calculate_labels(ctx, pairs.get());
                if (labels.is_error())
                {
                    return ERR_APPEND(labels.get_error(), "Failed to calculate labels");
                }

                return OK({pairs.get(), labels.get()});
            };

            std::string SharedConnection::to_string() const
            {
                return "SharedConnection";
            }

        }    // namespace gate_pair_label
    }        // namespace machine_learning
}    // namespace hal