#include "machine_learning/labels/gate_pair_label.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "nlohmann/json.hpp"
#include "netlist_preprocessing/netlist_preprocessing.h"

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
                    std::map<std::tuple<std::string, PinDirection, std::string>, std::set<std::tuple<u32, Gate*>>> word_to_gates_unsorted;

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
                        nlohmann::json j                                                         = nlohmann::json::parse(json_string);
                        std::vector<netlist_preprocessing::indexed_identifier> index_information = j.get<std::vector<netlist_preprocessing::indexed_identifier>>();

                        // TODO remove
                        if (!index_information.empty())
                        {
                            std::cout << "For gate " << g->get_id() << " found " <<index_information.front().identifier << " - " << index_information.front().index << " - " << index_information.front().distance << std::endl;
                        }

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


                        for (const auto& [name, index, _origin, pin, direction, distance] : index_information)
                        {
                            if (pin_to_min_distance.at(pin) == distance)
                            {
                                word_to_gates_unsorted[{name, direction, pin}].insert({index, g});
                            }
                        }
                    }

                    // 1. Sort out words with the same name by checking whether they contain duplicate indices
                    // 2. Dedupe all words by only keeping one word/name_direction for each multi_bit_signal/vector of gates.
                    std::map<std::vector<Gate*>, std::tuple<std::string, PinDirection, std::string>> gates_to_word;

                    for (const auto& [name_direction, word] : word_to_gates_unsorted)
                    {
                        std::set<u32> indices;
                        std::set<Gate*> unique_gates;
                        std::vector<Gate*> gates;

                        // TODO remove
                        // std::cout << "Order Word: " << std::endl;
                        for (auto& [index, gate] : word)
                        {
                            // TODO remove
                            // std::cout << index << std::endl;

                            indices.insert(index);
                            unique_gates.insert(gate);

                            gates.push_back(gate);
                        }

                        // sanity check
                        if (indices.size() != word.size())
                        {
                            // TODO return result
                            log_error("machine_learning", "Found index double in word {}-{} - !", std::get<0>(name_direction), enum_to_string(std::get<1>(name_direction)), std::get<2>(name_direction));

                            // TODO remove
                            std::cout << "Insane Word: " << std::endl;
                            for (const auto& [index, gate] : word)
                            {
                                std::cout << index << ": " << gate->get_id() << std::endl;
                            }

                            continue;
                        }

                        if (unique_gates.size() != word.size())
                        {
                            continue;
                        }

                        if (unique_gates.size() <= 1)
                        {
                            continue;
                        }

                        std::cout << "Word [" << word.size() << "] " << std::get<0>(name_direction) << " - " << std::get<1>(name_direction) <<  " - " << std::get<2>(name_direction) << " : " << std::endl;
                        for (const auto& [index, gate] : word)
                        {
                            std::cout << index << ": " << gate->get_id() << std::endl;
                        }

                        if (const auto it = gates_to_word.find(gates); it == gates_to_word.end())
                        {
                            gates_to_word.insert({gates, name_direction});
                        }
                        // NOTE could think about a priorization of shorter names or something similar
                    }

                    MultiBitInformation mbi;

                    for (auto& [word_gates, name_direction] : gates_to_word)
                    {
                        mbi.word_to_gates[name_direction] = word_gates;
                        for (const auto g : word_gates)
                        {
                            mbi.gate_to_words[g].push_back(name_direction);
                        }
                    }

                    // filter words for each gate:
                    // 1) For each direction only take the biggest word
                    // 2) From all remaining only take the smallest word
                    // std::map<const Gate*, std::vector<const std::tuple<const std::string, const PinDirection, const std::string>>> filtered_gate_to_words;
                    // for (const auto g : gates)
                    // {
                    //     const auto it = mbi.gate_to_words.find(g);
                    //     if (it == mbi.gate_to_words.end())
                    //     {
                    //         continue;
                    //     }

                    //     std::set<u32> sizes;
                    //     for (const auto& w : it->second)
                    //     {
                    //         sizes.insert(mbi.word_to_gates.at(w).size());
                    //     }

                    //     std::vector<const std::tuple<const std::string, const PinDirection, const std::string>> filtered_words;
                    //     for (const auto& w : it->second)
                    //     {
                    //         if (mbi.word_to_gates.at(w).size() == *(sizes.begin()))
                    //         {
                    //             filtered_words.push_back(w);
                    //         }
                    //     }

                    //     filtered_gate_to_words.insert({g, filtered_words});
                    // }

                    // std::map<const std::tuple<const std::string, const PinDirection, const std::string>, std::vector<const Gate*>> filtered_word_to_gates;
                    // for (const auto& [g, words] : filtered_gate_to_words)
                    // {
                    //     for (const auto& w : words)
                    //     {
                    //         filtered_word_to_gates[w].push_back(g);
                    //     }
                    // }

                    // mbi.gate_to_words = filtered_gate_to_words;
                    // mbi.word_to_gates = filtered_word_to_gates;

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
                            for (const auto* g_i : word_gates)
                            {
                                if (g == g_i)
                                {
                                    continue;
                                }

                                pairs.push_back({g, g_i});
                                pos_gates.insert(g_i);
                            }
                        }
                    }

                    // negative labels (equal amount to the positive labels)
                    const u64 pos_count = pos_gates.size();
                    const u64 neg_count = std::min(gates.size() - pos_count, pos_count);

                    std::cout << "Gate ID: " << g->get_id() << "   " <<  pos_count << " vs. " << neg_count << std::endl;

                    std::set<Gate*> chosen_gates;
                    for (u32 i = 0; i < neg_count; i++)
                    {
                        const u32 start = std::rand() % gates.size();
                        for (u32 idx = start; idx < start + gates.size(); idx = (idx + 1) % gates.size())
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

                return pairs;
            };

            std::vector<u32> SharedSignalGroup::calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const
            {
                const auto& mbi     = lc.get_multi_bit_information();

                const auto it_a = mbi.gate_to_words.find(g_a);
                if (it_a == mbi.gate_to_words.end())
                {
                    return {0};
                }

                const auto it_b = mbi.gate_to_words.find(g_b);
                if (it_b == mbi.gate_to_words.end())
                {
                    return {0};
                } 

                const auto& words_a = it_a->second;
                const auto& words_b = it_b->second;

                // // only consider the smallest words a gate is part of
                // std::set<u32> sizes_a;
                // std::set<u32> sizes_b;

                // for (const auto& w_a : words_a)
                // {
                //     sizes_a.insert(mbi.word_to_gates.at(w_a).size());
                // }

                // for (const auto& w_b : words_b)
                // {
                //     sizes_b.insert(mbi.word_to_gates.at(w_b).size());
                // }

                // std::vector<std::pair<std::string, PinDirection>> filtered_words_a;
                // std::vector<std::pair<std::string, PinDirection>> filtered_words_b;

                // for (const auto& w_a : words_a)
                // {
                //     if (mbi.word_to_gates.at(w_a).size() == *(sizes_a.begin()))
                //     {
                //         filtered_words_a.push_back(w_a);
                //     }
                // }

                // for (const auto& w_b : words_b)
                // {
                //     if (mbi.word_to_gates.at(w_b).size() == *(sizes_b.begin()))
                //     {
                //         filtered_words_b.push_back(w_b);
                //     }
                // }

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