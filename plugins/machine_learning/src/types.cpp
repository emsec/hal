#include "graph_algorithm/algorithms/centrality.h"
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
                MultiBitInformation m_mbi;

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
                    std::vector<Gate*> gates_vec;

                    // TODO remove
                    // std::cout << "Order Word: " << std::endl;
                    for (auto& [index, gate] : word)
                    {
                        // TODO remove
                        // std::cout << index << std::endl;

                        indices.insert(index);
                        unique_gates.insert(gate);

                        gates_vec.push_back(gate);
                        m_mbi.gate_word_to_index.insert({{gate, name_direction}, index});
                    }

                    // sanity check
                    if (indices.size() != word.size())
                    {
                        // TODO return result
                        log_error("machine_learning", "Found index double in word {}-{} - {} !", std::get<0>(name_direction), enum_to_string(std::get<1>(name_direction)), std::get<2>(name_direction));

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
                        gates_to_word.insert({gates_vec, name_direction});
                    }
                    // NOTE could think about a priorization of shorter names or something similar
                }

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

        std::optional<bool> MultiBitInformation::is_index_a_smaller_index_b(const PinDirection& direction, const Gate* g_a, const Gate* g_b) const
        {
            const auto it_a = gate_to_words.find(g_a);
            if (it_a == gate_to_words.end())
            {
                return std::nullopt;
            }

            const auto it_b = gate_to_words.find(g_b);
            if (it_b == gate_to_words.end())
            {
                return std::nullopt;
            }

            const auto& words_a = it_a->second;
            const auto& words_b = it_b->second;

            std::set<std::tuple<std::string, PinDirection, std::string>> filtered_words_a;
            std::set<std::tuple<std::string, PinDirection, std::string>> filtered_words_b;

            for (const auto& w_a : words_a)
            {
                if (direction == PinDirection::inout || std::get<1>(w_a) == direction)
                {
                    filtered_words_a.insert(w_a);
                }
            }

            for (const auto& w_b : words_b)
            {
                if (direction == PinDirection::inout || std::get<1>(w_b) == direction)
                {
                    filtered_words_b.insert(w_b);
                }
            }

            std::vector<std::pair<std::tuple<std::string, PinDirection, std::string>, std::tuple<std::string, PinDirection, std::string>>> matches;
            for (const auto& wa : filtered_words_a)
            {
                const auto& [wa_org, wa_dir, _wa_pin] = wa;
                for (const auto& wb : filtered_words_b)
                {
                    const auto& [wb_org, wb_dir, _wb_pin] = wb;
                    if ((wa_org == wb_org) && (wa_dir == wb_dir))
                    {
                        matches.push_back({wa, wb});
                    }
                }
            }

            if (matches.empty())
            {
                return std::nullopt;
            }

            std::sort(matches.begin(), matches.end(), [&](const auto& m1, const auto& m2) {
                const auto& [w1a, w1b] = m1;
                const auto& [w2a, w2b] = m2;

                const auto size1 = word_to_gates.at(w1a).size() + word_to_gates.at(w1b).size();
                const auto size2 = word_to_gates.at(w2a).size() + word_to_gates.at(w2b).size();

                return size1 < size2;
            });

            const auto& [wa, wb] = matches.front();
            const auto index_a   = gate_word_to_index.at({g_a, wa});
            const auto index_b   = gate_word_to_index.at({g_b, wb});

            if (index_a < index_b)
            {
                return true;
            }

            return false;
        }

        const MultiBitInformation& Context::get_multi_bit_information()
        {
            auto mbi = std::atomic_load_explicit(&m_mbi, std::memory_order_acquire);
            if (mbi)
            {
                return *mbi;
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_mbi_mutex);
                mbi = std::atomic_load_explicit(&m_mbi, std::memory_order_acquire);
                if (mbi)
                {
                    return *mbi;
                }

                auto new_mbi         = std::make_shared<MultiBitInformation>();
                const auto seq_gates = nl->get_gates([](const auto* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });
                *new_mbi             = calculate_multi_bit_information(seq_gates);

                std::atomic_store_explicit(&m_mbi, new_mbi, std::memory_order_release);

                return *new_mbi;
            }
        }

        const Result<NetlistAbstraction*> Context::get_sequential_abstraction()
        {
            auto abstraction = std::atomic_load_explicit(&m_sequential_abstraction, std::memory_order_acquire);
            if (abstraction)
            {
                return OK(abstraction.get());
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_sequential_abstraction_mutex);
                // Double-check after acquiring the lock
                abstraction = std::atomic_load_explicit(&m_sequential_abstraction, std::memory_order_acquire);
                if (abstraction)
                {
                    return OK(abstraction.get());
                }

                const auto seq_gates = nl->get_gates([](const auto* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });

                const std::vector<PinType> forbidden_pins = {PinType::clock, PinType::ground, PinType::power};

                const auto endpoint_filter = [forbidden_pins](const auto* ep, const auto&) {
                    return std::find(forbidden_pins.begin(), forbidden_pins.end(), ep->get_pin()->get_type()) == forbidden_pins.end();
                };

                auto new_abstraction = NetlistAbstraction::create(nl, seq_gates, true, endpoint_filter, endpoint_filter);
                if (new_abstraction.is_error())
                {
                    return ERR_APPEND(new_abstraction.get_error(), "Cannot get sequential netlist abstraction: failed to build abstraction.");
                }

                std::shared_ptr<NetlistAbstraction> shared_abstracttion(std::move(new_abstraction).get());

                std::atomic_store_explicit(&m_sequential_abstraction, shared_abstracttion, std::memory_order_release);

                return OK(m_sequential_abstraction.get());
            }
        }

        const Result<NetlistAbstraction*> Context::get_original_abstraction()
        {
            auto abstraction = std::atomic_load_explicit(&m_original_abstraction, std::memory_order_acquire);
            if (abstraction)
            {
                return OK(abstraction.get());
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_original_abstraction_mutex);
                // Double-check after acquiring the lock
                abstraction = std::atomic_load_explicit(&m_original_abstraction, std::memory_order_acquire);
                if (abstraction)
                {
                    return OK(abstraction.get());
                }

                auto new_abstraction = NetlistAbstraction::create(nl, nl->get_gates(), true, nullptr, nullptr);
                if (new_abstraction.is_error())
                {
                    return ERR_APPEND(new_abstraction.get_error(), "Cannot get original netlist abstraction: failed to build abstraction.");
                }

                std::shared_ptr<NetlistAbstraction> shared_abstracttion(std::move(new_abstraction).get());

                std::atomic_store_explicit(&m_original_abstraction, shared_abstracttion, std::memory_order_release);

                return OK(m_original_abstraction.get());
            }
        }

        const Result<graph_algorithm::NetlistGraph*> Context::get_sequential_netlist_graph()
        {
            auto graph = std::atomic_load_explicit(&m_sequential_netlist_graph, std::memory_order_acquire);
            if (graph)
            {
                return OK(graph.get());
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_sequential_graph_mutex);
                // Double-check after acquiring the lock
                graph = std::atomic_load_explicit(&m_sequential_netlist_graph, std::memory_order_acquire);
                if (graph)
                {
                    return OK(graph.get());
                }

                const auto sequential_abstraction = this->get_sequential_abstraction();
                if (sequential_abstraction.is_error())
                {
                    return ERR_APPEND(sequential_abstraction.get_error(), "cannot get origianl netlist abstraction");
                }

                auto new_graph = graph_algorithm::NetlistGraph::from_netlist_abstraction(sequential_abstraction.get(), true);
                if (new_graph.is_error())
                {
                    return ERR_APPEND(new_graph.get_error(), "cannot get sequential netlist graph: failed to build new graph");
                }

                std::shared_ptr<graph_algorithm::NetlistGraph> shared_graph(std::move(new_graph).get());

                std::atomic_store_explicit(&m_sequential_netlist_graph, shared_graph, std::memory_order_release);

                return OK(m_sequential_netlist_graph.get());
            }
        }

        const Result<graph_algorithm::NetlistGraph*> Context::get_original_netlist_graph()
        {
            auto graph = std::atomic_load_explicit(&m_original_netlist_graph, std::memory_order_acquire);
            if (graph)
            {
                return OK(graph.get());
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_original_graph_mutex);
                // Double-check after acquiring the lock
                graph = std::atomic_load_explicit(&m_original_netlist_graph, std::memory_order_acquire);
                if (graph)
                {
                    return OK(graph.get());
                }

                const auto original_abstraction = this->get_original_abstraction();
                if (original_abstraction.is_error())
                {
                    return ERR_APPEND(original_abstraction.get_error(), "cannot get origianl netlist abstraction");
                }

                auto new_graph = graph_algorithm::NetlistGraph::from_netlist_abstraction(original_abstraction.get(), true);
                if (new_graph.is_error())
                {
                    return ERR_APPEND(new_graph.get_error(), "cannot get original netlist graph: failed to build new graph");
                }
                std::shared_ptr<graph_algorithm::NetlistGraph> shared_graph(std::move(new_graph).get());

                std::atomic_store_explicit(&m_original_netlist_graph, shared_graph, std::memory_order_release);

                return OK(m_original_netlist_graph.get());
            }
        }

        const std::vector<GateTypeProperty>& Context::get_possible_gate_type_properties()
        {
            auto properties = std::atomic_load_explicit(&m_possible_gate_type_properties, std::memory_order_acquire);
            if (properties)
            {
                return *properties;
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_possible_gate_type_properties_mutex);
                // Double-check after acquiring the lock
                properties = std::atomic_load_explicit(&m_possible_gate_type_properties, std::memory_order_acquire);
                if (properties)
                {
                    return *properties;
                }

                std::set<GateTypeProperty> property_set;

                for (const auto& [_name, gt] : nl->get_gate_library()->get_gate_types())
                {
                    const auto& gt_properties = gt->get_properties();
                    property_set.insert(gt_properties.begin(), gt_properties.end());
                }

                auto properties_vec = std::make_shared<std::vector<GateTypeProperty>>(property_set.begin(), property_set.end());

                // Sort alphabetically
                std::sort(properties_vec->begin(), properties_vec->end(), [](const auto& a, const auto& b) { return enum_to_string(a) < enum_to_string(b); });

                std::atomic_store_explicit(&m_possible_gate_type_properties, properties_vec, std::memory_order_release);

                return *properties_vec;
            }
        }

    }    // namespace machine_learning
}    // namespace hal