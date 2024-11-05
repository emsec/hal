#include "machine_learning/labels/gate_pair_label.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "nlohmann_json/json.hpp"

#include <stdlib.h>

namespace hal
{
    namespace machine_learning
    {
        namespace gate_pair_label
        {
            namespace
            {
                MultiBitInformation calculate_multi_bit_information(const std::vector<Gate*>& gates)
                {
                    std::map<std::pair<std::string, std::string>, std::set<std::tuple<u32, Gate*>>> word_to_gates_unsorted;

                    for (const auto g : gates)
                    {
                        if (!g->has_data("preprocessing_information", "multi_bit_indexed_identifiers"))
                        {
                            log_error("machine_learning", "unable to find indexed identifiers for gate with ID {}", g->get_id());
                            continue;
                        }

                        const std::string json_string = std::get<1>(g->get_data("preprocessing_information", "multi_bit_indexed_identifiers"));

                        // TODO remove
                        // std::cout << "Trying to parse string: " << json_string << std::endl;

                        // TODO catch exceptions and return result
                        const nlohmann::json j                                                                      = nlohmann::json::parse(json_string);
                        const std::vector<std::tuple<std::string, u32, std::string, std::string>> index_information = j;

                        // TODO remove
                        // if (!index_information.empty())
                        // {
                        //     std::cout << "For gate " << g->get_id() << " found " << std::get<0>(index_information.front()) << " - " << std::get<1>(index_information.front()) << std::endl;
                        // }

                        for (const auto& [name, index, _origin, direction] : index_information)
                        {
                            word_to_gates_unsorted[{name, direction}].insert({index, g});
                        }
                    }

                    // 1. Sort out words with the same name by checking whether they contain duplicate indices
                    // 2. Dedupe all words by only keeping one word/name_direction for each multi_bit_signal/vector of gates.
                    std::map<std::vector<const Gate*>, std::pair<std::string, std::string>> gates_to_word;

                    for (const auto& [name_direction, word] : word_to_gates_unsorted)
                    {
                        std::set<u32> indices;
                        std::vector<const Gate*> gates;

                        // TODO remove
                        // std::cout << "Order Word: " << std::endl;
                        for (const auto& [index, gate] : word)
                        {
                            // TODO remove
                            // std::cout << index << std::endl;

                            indices.insert(index);
                            gates.push_back(gate);
                        }

                        // sanity check
                        if (indices.size() != word.size())
                        {
                            // TODO return result
                            log_error("machine_learning", "Found index double in word {}-{}!", name_direction.first, name_direction.second);

                            // TODO remove
                            std::cout << "Insane Word: " << std::endl;
                            for (const auto& [index, gate] : word)
                            {
                                std::cout << index << ": " << gate->get_id() << std::endl;
                            }

                            continue;
                        }

                        if (const auto it = gates_to_word.find(gates); it == gates_to_word.end())
                        {
                            gates_to_word.insert({gates, name_direction});
                        }
                        // NOTE could think about a priorization of shorter names or something similar
                        // else
                    }

                    MultiBitInformation mbi;

                    for (const auto& [gates, name_direction] : gates_to_word)
                    {
                        mbi.word_to_gates[name_direction] = gates;
                        for (const auto g : gates)
                        {
                            mbi.gate_to_words[g].push_back(name_direction);
                        }
                    }

                    return mbi;
                }
            }    // namespace

            const MultiBitInformation& LabelContext::get_multi_bit_information()
            {
                if (!mbi.has_value())
                {
                    mbi = calculate_multi_bit_information(gates);
                }

                return mbi.value();
            }

            std::vector<std::pair<const Gate*, const Gate*>> SharedSignalGroup::calculate_gate_pairs(LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates) const
            {
                const auto& mbi = lc.get_multi_bit_information();

                std::vector<std::pair<const Gate*, const Gate*>> pairs;

                for (const auto& g : gates)
                {
                    // positive labels
                    std::unordered_set<Gate*> pos_gates;
                    if (mbi.gate_to_words.find(g) == mbi.gate_to_words.end())
                    {
                        // gate is only in a group with itself
                        pairs.push_back({g, g});
                        pos_gates.insert(g);
                    }
                    else
                    {
                        // add all gates that are part of at least one other signal group as positive pair
                        for (const auto& name_direction : mbi.gate_to_words.at(g))
                        {
                            const auto& gates = mbi.word_to_gates.at(name_direction);
                            for (const auto g_i : gates)
                            {
                                pairs.push_back({g, g_i});
                                pos_gates.insert(g);
                            }
                        }
                    }

                    // negative labels (equal amount to the positive labels)
                    const u64 pos_count = pos_gates.size();
                    const u64 neg_count = std::min(gates.size() - pos_count, pos_count);

                    std::set<Gate*> chosen_gates;
                    for (u32 i = 0; i < neg_count; i++)
                    {
                        const u32 start = std::rand() % lc.nl->get_gates().size();
                        for (u32 idx = start; idx < start + lc.nl->get_gates().size(); idx++)
                        {
                            const auto g_i = lc.nl->get_gates().at(idx % lc.nl->get_gates().size());
                            if (pos_gates.find(g_i) == pos_gates.end() && chosen_gates.find(g_i) == chosen_gates.end())
                            {
                                pairs.push_back({g, g_i});
                                chosen_gates.insert(g_i);
                                break;
                            }
                        }
                    }
                }

                return pairs;
            };

            std::vector<u32> SharedSignalGroup::calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const
            {
                const auto& mbi     = lc.get_multi_bit_information();
                const auto& words_a = mbi.gate_to_words.at(g_a);
                const auto& words_b = mbi.gate_to_words.at(g_b);

                for (const auto& wa : words_a)
                {
                    for (const auto& wb : words_b)
                    {
                        if (wa == wb)
                        {
                            return {1};
                        }
                    }
                }

                return {0};
            };

            std::vector<std::vector<u32>> SharedSignalGroup::calculate_labels(LabelContext& lc, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& p : gate_pairs)
                {
                    labels.push_back(calculate_label(lc, p.first, p.second));
                }

                return labels;
            }

            std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>> SharedSignalGroup::calculate_labels(LabelContext& lc) const
            {
                const auto& mbi = lc.get_multi_bit_information();

                std::vector<std::pair<const Gate*, const Gate*>> pairs;
                std::vector<std::vector<u32>> labels;

                for (const auto& g : lc.nl->get_gates([](const Gate* g_i) { return g_i->get_type()->has_property(GateTypeProperty::ff); }))
                {
                    // positive labels
                    std::unordered_set<Gate*> pos_gates;
                    if (mbi.gate_to_words.find(g) == mbi.gate_to_words.end())
                    {
                        // gate is only in a group with itself
                        pairs.push_back({g, g});
                        labels.push_back({{1}});
                        pos_gates.insert(g);
                    }
                    else
                    {
                        // add all gates that are part of at least one other signal group as positive pair
                        for (const auto& name_direction : mbi.gate_to_words.at(g))
                        {
                            const auto& gates = mbi.word_to_gates.at(name_direction);
                            for (const auto g_i : gates)
                            {
                                pairs.push_back({g, g_i});
                                labels.push_back({{1}});
                                pos_gates.insert(g);
                            }
                        }
                    }

                    // negative labels (equal amount to the positive labels)
                    const u64 pos_count = pos_gates.size();
                    const u64 neg_count = std::min(lc.nl->get_gates().size() - pos_count, pos_count);

                    // TODO remove
                    // std::cout << "Found " << all_connected.size() << " connections for gate " << g->get_id() << ". Trying to find " << neg_count << " opposites!" << std::endl;

                    std::set<Gate*> chosen_gates;
                    for (u32 i = 0; i < neg_count; i++)
                    {
                        const u32 start = std::rand() % lc.nl->get_gates().size();
                        for (u32 idx = start; idx < start + lc.nl->get_gates().size(); idx++)
                        {
                            const auto g_i = lc.nl->get_gates().at(idx % lc.nl->get_gates().size());
                            if (pos_gates.find(g_i) == pos_gates.end() && chosen_gates.find(g_i) == chosen_gates.end())
                            {
                                pairs.push_back({g, g_i});
                                labels.push_back({{0}});

                                chosen_gates.insert(g_i);
                                break;
                            }
                        }
                    }
                }

                return {pairs, labels};
            };

            namespace
            {
                std::unordered_set<const Gate*> get_all_connected_gates(const Gate* g)
                {
                    std::unordered_set<const Gate*> connected;
                    for (const auto pre : g->get_unique_predecessors())
                    {
                        connected.insert(pre);
                    }

                    for (const auto suc : g->get_unique_successors())
                    {
                        connected.insert(suc);
                    }

                    return connected;
                }
            }    // namespace

            std::vector<std::pair<const Gate*, const Gate*>> SharedConnection::calculate_gate_pairs(LabelContext& lc, const Netlist* nl, const std::vector<Gate*>& gates) const
            {
                std::vector<std::pair<const Gate*, const Gate*>> pairs;

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

                return pairs;
            };

            std::vector<u32> SharedConnection::calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const
            {
                const auto all_connected = get_all_connected_gates(g_a);

                if (all_connected.find(g_b) == all_connected.end())
                {
                    return {0};
                }

                return {1};
            };

            std::vector<std::vector<u32>> SharedConnection::calculate_labels(LabelContext& lc, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs) const
            {
                std::vector<std::vector<u32>> labels;

                for (const auto& p : gate_pairs)
                {
                    labels.push_back(calculate_label(lc, p.first, p.second));
                }

                return labels;
            }

            std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>> SharedConnection::calculate_labels(LabelContext& lc) const
            {
                std::vector<std::pair<const Gate*, const Gate*>> pairs;
                std::vector<std::vector<u32>> labels;

                for (const auto& g : lc.nl->get_gates())
                {
                    // positive labels
                    u64 pos_count            = 0;
                    const auto all_connected = get_all_connected_gates(g);
                    for (const auto g_c : all_connected)
                    {
                        pairs.push_back({g, g_c});
                        labels.push_back({{1}});

                        pos_count += 1;
                    }

                    // negative labels (equal amount to the positive labels)
                    const u64 neg_count = std::min(lc.nl->get_gates().size() - pos_count, pos_count);

                    // TODO remove
                    // std::cout << "Found " << all_connected.size() << " connections for gate " << g->get_id() << ". Trying to find " << neg_count << " opposites!" << std::endl;

                    for (u32 i = 0; i < neg_count; i++)
                    {
                        const u32 start = std::rand() % lc.nl->get_gates().size();
                        for (u32 idx = start; idx < start + lc.nl->get_gates().size(); idx++)
                        {
                            const auto g_i = lc.nl->get_gates().at(idx % lc.nl->get_gates().size());
                            if (all_connected.find(g_i) == all_connected.end())
                            {
                                pairs.push_back({g, g_i});
                                labels.push_back({{0}});
                                break;
                            }
                        }
                    }
                }

                return {pairs, labels};
            };
        }    // namespace gate_pair_label
    }    // namespace machine_learning
}    // namespace hal