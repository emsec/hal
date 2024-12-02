#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "machine_learning/types.h"
#include "netlist_preprocessing/netlist_preprocessing.h"
#include "nlohmann/json.hpp"

namespace hal
{
    namespace machine_learning
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
                    // if (!index_information.empty())
                    // {
                    //     std::cout << "For gate " << g->get_id() << " found " <<index_information.front().identifier << " - " << index_information.front().index << " - " << index_information.front().distance << std::endl;
                    // }

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

                    // TODO remove debug print
                    // std::cout << "Word [" << word.size() << "] " << std::get<0>(name_direction) << " - " << std::get<1>(name_direction) << " - " << std::get<2>(name_direction) << " : "
                    //           << std::endl;
                    // for (const auto& [index, gate] : word)
                    // {
                    //     std::cout << index << ": " << gate->get_id() << std::endl;
                    // }

                    if (const auto it = gates_to_word.find(gates); it == gates_to_word.end())
                    {
                        gates_to_word.insert({gates, name_direction});
                    }
                    // NOTE could think about a priorization of shorter names or something similar
                }

                MultiBitInformation m_mbi;

                for (auto& [word_gates, name_direction] : gates_to_word)
                {
                    m_mbi.word_to_gates[name_direction] = word_gates;
                    for (const auto g : word_gates)
                    {
                        m_mbi.gate_to_words[g].push_back(name_direction);
                    }
                }

                // filter words for each gate:
                // 1) For each direction only take the biggest word
                // 2) From all remaining only take the smallest word
                // std::map<const Gate*, std::vector<const std::tuple<const std::string, const PinDirection, const std::string>>> filtered_gate_to_words;
                // for (const auto g : gates)
                // {
                //     const auto it = m_mbi.gate_to_words.find(g);
                //     if (it == m_mbi.gate_to_words.end())
                //     {
                //         continue;
                //     }

                //     std::set<u32> sizes;
                //     for (const auto& w : it->second)
                //     {
                //         sizes.insert(m_mbi.word_to_gates.at(w).size());
                //     }

                //     std::vector<const std::tuple<const std::string, const PinDirection, const std::string>> filtered_words;
                //     for (const auto& w : it->second)
                //     {
                //         if (m_mbi.word_to_gates.at(w).size() == *(sizes.begin()))
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

                // m_mbi.gate_to_words = filtered_gate_to_words;
                // m_mbi.word_to_gates = filtered_word_to_gates;

                return m_mbi;
            }
        }    // namespace

        bool MultiBitInformation::are_gates_considered_a_pair(const PinDirection& direction, const Gate* g_a, const Gate* g_b) const
        {
            const auto it_a = gate_to_words.find(g_a);
            if (it_a == gate_to_words.end())
            {
                return false;
            }

            const auto it_b = gate_to_words.find(g_b);
            if (it_b == gate_to_words.end())
            {
                return false;
            }

            const auto& words_a = it_a->second;
            const auto& words_b = it_b->second;

            // // only consider the smallest words a gate is part of
            // std::set<u32> sizes_a;
            // std::set<u32> sizes_b;

            // for (const auto& w_a : words_a)
            // {
            //     sizes_a.insert(word_to_gates.at(w_a).size());
            // }

            // for (const auto& w_b : words_b)
            // {
            //     sizes_b.insert(word_to_gates.at(w_b).size());
            // }

            // std::vector<std::pair<std::string, PinDirection>> filtered_words_a;
            // std::vector<std::pair<std::string, PinDirection>> filtered_words_b;

            // for (const auto& w_a : words_a)
            // {
            //     if (word_to_gates.at(w_a).size() == *(sizes_a.begin()))
            //     {
            //         filtered_words_a.push_back(w_a);
            //     }
            // }

            // for (const auto& w_b : words_b)
            // {
            //     if (word_to_gates.at(w_b).size() == *(sizes_b.begin()))
            //     {
            //         filtered_words_b.push_back(w_b);
            //     }
            // }

            std::set<std::pair<std::string, PinDirection>> filtered_words_a;
            std::set<std::pair<std::string, PinDirection>> filtered_words_b;

            for (const auto& w_a : words_a)
            {
                if (direction == PinDirection::inout || std::get<1>(w_a) == direction)
                {
                    filtered_words_a.insert({std::get<0>(w_a), std::get<1>(w_a)});
                }
            }

            for (const auto& w_b : words_b)
            {
                if (direction == PinDirection::inout || std::get<1>(w_b) == direction)
                {
                    filtered_words_b.insert({std::get<0>(w_b), std::get<1>(w_b)});
                }
            }

            // Alternative: Only consider two gates as a pair if they share all annotated index words
            // if (filtered_words_a == filtered_words_b)
            // {
            //     return true;
            // }

            for (const auto& wa : filtered_words_a)
            {
                for (const auto& wb : filtered_words_b)
                {
                    if (wa == wb)
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        const MultiBitInformation& Context::get_multi_bit_information()
        {
            if (!m_mbi.has_value())
            {
                const auto seq_gates = nl->get_gates([](const auto* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
                m_mbi                = calculate_multi_bit_information(seq_gates);
            }

            return m_mbi.value();
        }

        const Result<NetlistAbstraction*> Context::get_sequential_abstraction()
        {
            if (!m_sequential_abstraction.has_value())
            {
                const auto seq_gates = nl->get_gates([](const auto* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });

                const std::vector<PinType> forbidden_pins = {
                    PinType::clock, /*PinType::done, PinType::error, PinType::error_detection,*/ /*PinType::none,*/ PinType::ground, PinType::power /*, PinType::status*/};

                const auto endpoint_filter = [forbidden_pins](const auto* ep, const auto& _d) {
                    UNUSED(_d);
                    return std::find(forbidden_pins.begin(), forbidden_pins.end(), ep->get_pin()->get_type()) == forbidden_pins.end();
                };

                const auto sequential_abstraction_res = NetlistAbstraction::create(nl, seq_gates, true, endpoint_filter, endpoint_filter);
                if (sequential_abstraction_res.is_error())
                {
                    return ERR_APPEND(sequential_abstraction_res.get_error(), "cannot get sequential netlist abstraction for gate feature context: failed to build abstraction.");
                }

                m_sequential_abstraction = sequential_abstraction_res.get();

                // TODO remove debug print
                // std::cout << "Built abstraction" << std::endl;
            }

            return OK(&m_sequential_abstraction.value());
        }

        const Result<NetlistAbstraction*> Context::get_original_abstraction()
        {
            if (!m_original_abstraction.has_value())
            {
                // const std::vector<PinType> forbidden_pins = {
                //     PinType::clock, /*PinType::done, PinType::error, PinType::error_detection,*/ /*PinType::none,*/ PinType::ground, PinType::power /*, PinType::status*/};

                // const auto endpoint_filter = [forbidden_pins](const auto* ep, const auto& _d) {
                //     UNUSED(_d);
                //     return std::find(forbidden_pins.begin(), forbidden_pins.end(), ep->get_pin()->get_type()) == forbidden_pins.end();
                // };

                const auto original_abstraction_res = NetlistAbstraction::create(nl, nl->get_gates(), true, nullptr, nullptr);
                if (original_abstraction_res.is_error())
                {
                    return ERR_APPEND(original_abstraction_res.get_error(), "cannot get original netlist abstraction for gate feature context: failed to build abstraction.");
                }

                m_original_abstraction = original_abstraction_res.get();

                // TODO remove debug print
                // std::cout << "Built abstraction" << std::endl;
            }

            return OK(&m_original_abstraction.value());
        }

        const std::vector<GateTypeProperty>& Context::get_possible_gate_type_properties()
        {
            if (!m_possible_gate_type_properties.has_value())
            {
                std::set<GateTypeProperty> properties;

                for (const auto& [_name, gt] : nl->get_gate_library()->get_gate_types())
                {
                    const auto gt_properties = gt->get_properties();
                    properties.insert(gt_properties.begin(), gt_properties.end());
                }

                // for (auto& [gtp, _name] : EnumStrings<GateTypeProperty>::data)
                // {
                //     UNUSED(_name);
                //     properties.insert(gtp);
                // }

                auto properties_vec = utils::to_vector(properties);
                // sort alphabetically
                std::sort(properties_vec.begin(), properties_vec.end(), [](const auto& a, const auto& b) { return enum_to_string(a) < enum_to_string(b); });
                m_possible_gate_type_properties = properties_vec;
            }

            return m_possible_gate_type_properties.value();
        }
    }    // namespace machine_learning
}    // namespace hal