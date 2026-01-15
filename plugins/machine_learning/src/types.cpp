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
            using WordKey = std::tuple<std::string, PinDirection, std::string>;    // {word_name, pin_direction, pin_name}

            struct GateEntry
            {
                u32 index;
                Gate* gate;
                u32 distance;    // minimal per-pin distance, as you already filter
            };

            struct CandidateWord
            {
                WordKey key;
                std::vector<GateEntry> entries;

                // Canonical gate-vector used as dedupe key (sorted by gate id for stability).
                std::vector<Gate*> canonical_gates;

                // For final output mapping (only for the selected candidate).
                std::unordered_map<Gate*, u32> gate_to_index;

                double avg_distance = std::numeric_limits<double>::infinity();

                // Derived “signals” used by policies
                bool is_output_word        = false;    // PinDirection::output
                bool is_gate_name_encoding = false;    // (dir==none && dist==0) per your encoding
            };

            struct CandidateScore
            {
                // Lower is better.
                int primary         = 0;
                double avg_distance = std::numeric_limits<double>::infinity();

                // Deterministic tie-breakers
                size_t name_len = 0;
                std::string name;
                int dir = 0;
                std::string pin;

                friend bool operator<(const CandidateScore& a, const CandidateScore& b)
                {
                    return std::tie(a.primary, a.avg_distance, a.name_len, a.name, a.dir, a.pin) < std::tie(b.primary, b.avg_distance, b.name_len, b.name, b.dir, b.pin);
                }
            };

            static CandidateScore score_candidate(const CandidateWord& c, MultiBitProcessingPolicy policy)
            {
                const auto& [name, dir, pin] = c.key;

                CandidateScore s;
                s.avg_distance = c.avg_distance;
                s.name_len     = name.size();
                s.name         = name;
                s.dir          = static_cast<int>(dir);
                s.pin          = pin;

                switch (policy)
                {
                    case MultiBitProcessingPolicy::Default:
                        s.primary = 0;    // just avg distance + tie-breakers
                        break;

                    case MultiBitProcessingPolicy::Yosys:
                        // output wins if ambiguous; then avg distance
                        s.primary = c.is_output_word ? 0 : 1;
                        break;

                    case MultiBitProcessingPolicy::Vivado:
                        // gate-name encoding wins if ambiguous; then avg distance
                        s.primary = c.is_gate_name_encoding ? 0 : 1;
                        break;
                }

                return s;
            }

            MultiBitInformation calculate_multi_bit_information(const std::vector<Gate*>& gates, const MultiBitProcessingPolicy policy = MultiBitProcessingPolicy::Default)
            {
                MultiBitInformation m_mbi;

                // 1) Collect entries per WordKey (after per-pin minimal distance filtering).
                std::map<WordKey, std::vector<GateEntry>> word_to_entries;

                for (Gate* g : gates)
                {
                    if (!g->has_data("preprocessing_information", "multi_bit_indexed_identifiers"))
                    {
                        log_error("machine_learning", "unable to find indexed identifiers for gate with {} ID {}", g->get_name(), g->get_id());
                        continue;
                    }

                    const std::string json_string = std::get<1>(g->get_data("preprocessing_information", "multi_bit_indexed_identifiers"));

                    // TODO catch exceptions and return result
                    nlohmann::json j                                                         = nlohmann::json::parse(json_string);
                    std::vector<netlist_preprocessing::indexed_identifier> index_information = j.get<std::vector<netlist_preprocessing::indexed_identifier>>();

                    // --- DEBUG PRINT (same as in your current function)
                    std::cout << "Gate: " << g->get_name() << " / " << g->get_id() << std::endl;

                    // For each pin, only consider the index information with the least distance
                    std::map<std::string, u32> pin_to_min_distance;
                    for (const auto& [_name, _index, _origin, pin, _direction, distance] : index_information)
                    {
                        // --- DEBUG PRINT (same as in your current function)
                        std::cout << _name << " - " << _index << " - " << _origin << " - " << pin << " - " << _direction << " - " << distance << std::endl;

                        auto [it, inserted] = pin_to_min_distance.emplace(pin, distance);
                        if (!inserted)
                        {
                            it->second = std::min(it->second, distance);
                        }
                    }

                    for (const auto& [name, index, _origin, pin, direction, distance] : index_information)
                    {
                        if (pin_to_min_distance.at(pin) != distance)
                        {
                            continue;
                        }

                        const WordKey wk{name, direction, pin};
                        word_to_entries[wk].push_back(GateEntry{index, g, distance});
                    }
                }

                // --- DEBUG PRINT
                for (const auto& [word, entries] : word_to_entries)
                {
                    const auto& [name, direction, pin] = word;
                    std::cout << name << " - " << direction << " - " << pin << std::endl;

                    for (const auto& e : entries)
                    {
                        std::cout << "\t" << e.gate->get_name() << " / " << e.gate->get_id() << std::endl;
                    }
                }

                // 2) Build validated candidates (unique indices, unique gates, size>1), compute avg distance + policy features.
                std::vector<CandidateWord> candidates;
                candidates.reserve(word_to_entries.size());

                for (auto& [wk, entries] : word_to_entries)
                {
                    if (entries.size() <= 1)
                    {
                        continue;
                    }

                    std::set<u32> indices;
                    std::set<Gate*> unique_gates;

                    bool bad = false;
                    for (const auto& e : entries)
                    {
                        if (!indices.insert(e.index).second)
                        {
                            log_error("machine_learning", "Found index double in word {}-{} - {} !", std::get<0>(wk), enum_to_string(std::get<1>(wk)), std::get<2>(wk));

                            // --- DEBUG PRINT (same as in your current function)
                            std::cout << "Insane Word: " << std::endl;
                            for (const auto& ee : entries)
                            {
                                std::cout << ee.index << ": " << ee.gate->get_id() << std::endl;
                            }

                            bad = true;
                            break;
                        }

                        if (!unique_gates.insert(e.gate).second)
                        {
                            bad = true;
                            break;
                        }
                    }

                    if (bad || unique_gates.size() <= 1)
                    {
                        continue;
                    }

                    CandidateWord c;
                    c.key     = wk;
                    c.entries = std::move(entries);

                    // canonical gate group key
                    c.canonical_gates.assign(unique_gates.begin(), unique_gates.end());
                    std::sort(c.canonical_gates.begin(), c.canonical_gates.end(), [](Gate* a, Gate* b) { return a->get_id() < b->get_id(); });

                    // avg distance
                    u64 sum = 0;
                    for (const auto& e : c.entries)
                    {
                        sum += e.distance;
                    }
                    c.avg_distance = static_cast<double>(sum) / static_cast<double>(c.entries.size());

                    // gate->index map (used when emitting chosen candidate)
                    c.gate_to_index.reserve(c.entries.size());
                    for (const auto& e : c.entries)
                    {
                        c.gate_to_index.emplace(e.gate, e.index);
                    }

                    // policy features
                    c.is_output_word = (std::get<1>(c.key) == PinDirection::output);

                    const bool dir_none     = (std::get<1>(c.key) == PinDirection::none);
                    const bool all_zero     = std::all_of(c.entries.begin(), c.entries.end(), [](const GateEntry& e) { return e.distance == 0; });
                    c.is_gate_name_encoding = dir_none && all_zero;

                    candidates.push_back(std::move(c));
                }

                // 3) Policy-based dedupe: for each gate-group, keep best candidate.
                std::map<std::vector<Gate*>, CandidateWord> best_by_gates;

                for (auto& c : candidates)
                {
                    const auto key = c.canonical_gates;    // copy for map key

                    auto it = best_by_gates.find(key);
                    if (it == best_by_gates.end())
                    {
                        best_by_gates.emplace(key, std::move(c));
                        continue;
                    }

                    const CandidateScore new_s  = score_candidate(c, policy);
                    const CandidateScore prev_s = score_candidate(it->second, policy);

                    if (new_s < prev_s)
                    {
                        it->second = std::move(c);
                    }
                }

                // 4) Emit result
                for (auto& [gate_vec, chosen] : best_by_gates)
                {
                    const WordKey& wk = chosen.key;

                    m_mbi.word_to_gates[wk] = gate_vec;
                    for (Gate* g : gate_vec)
                    {
                        m_mbi.gate_to_words[g].push_back(wk);

                        if (auto it = chosen.gate_to_index.find(g); it != chosen.gate_to_index.end())
                        {
                            m_mbi.gate_word_to_index.insert({{g, wk}, it->second});
                        }
                    }
                }

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
                *new_mbi             = calculate_multi_bit_information(seq_gates, m_mbi_policy);

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

        const std::vector<PinType>& Context::get_possible_pin_types()
        {
            auto pin_types = std::atomic_load_explicit(&m_possible_pin_types, std::memory_order_acquire);
            if (pin_types)
            {
                return *pin_types;
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_possible_pin_types_mutex);
                // Double-check after acquiring the lock
                pin_types = std::atomic_load_explicit(&m_possible_pin_types, std::memory_order_acquire);
                if (pin_types)
                {
                    return *pin_types;
                }

                std::set<PinType> type_set;

                for (const auto& [_name, gt] : nl->get_gate_library()->get_gate_types())
                {
                    const auto& gt_pins = gt->get_pins();
                    for (const auto& pin : gt_pins)
                    {
                        type_set.insert(pin->get_type());
                    }
                }

                auto pin_types_vec = std::make_shared<std::vector<PinType>>(type_set.begin(), type_set.end());

                // Sort alphabetically
                std::sort(pin_types_vec->begin(), pin_types_vec->end(), [](const auto& a, const auto& b) { return enum_to_string(a) < enum_to_string(b); });

                std::atomic_store_explicit(&m_possible_pin_types, pin_types_vec, std::memory_order_release);

                return *pin_types_vec;
            }
        }

        const std::vector<PinDirection>& Context::get_possible_pin_directions()
        {
            auto pin_directions = std::atomic_load_explicit(&m_possible_pin_directions, std::memory_order_acquire);
            if (pin_directions)
            {
                return *pin_directions;
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_possible_pin_directions_mutex);
                // Double-check after acquiring the lock
                pin_directions = std::atomic_load_explicit(&m_possible_pin_directions, std::memory_order_acquire);
                if (pin_directions)
                {
                    return *pin_directions;
                }

                std::set<PinDirection> pin_directions_set;

                for (const auto& [_name, gt] : nl->get_gate_library()->get_gate_types())
                {
                    const auto& gt_pins = gt->get_pins();
                    for (const auto& pin : gt_pins)
                    {
                        pin_directions_set.insert(pin->get_direction());
                    }
                }

                auto pin_directions_vec = std::make_shared<std::vector<PinDirection>>(pin_directions_set.begin(), pin_directions_set.end());

                // Sort alphabetically
                std::sort(pin_directions_vec->begin(), pin_directions_vec->end(), [](const auto& a, const auto& b) { return enum_to_string(a) < enum_to_string(b); });

                std::atomic_store_explicit(&m_possible_pin_directions, pin_directions_vec, std::memory_order_release);

                return *pin_directions_vec;
            }
        }

        const std::vector<Gate*>& Context::get_gates() const
        {
            return m_gates;
        }

        const std::vector<Gate*>& Context::get_sequential_gates() const
        {
            return m_sequential_gates;
        }

        u32 Context::get_gate_type_index(const GateType* gt) const
        {
            return m_gate_type_indices.at(gt);
        }

        u32 Context::get_gate_pin_index(const GateType* gt, const GatePin* gp) const
        {
            return m_gate_pin_indices.at(gt).at(gp);
        }

    }    // namespace machine_learning
}    // namespace hal