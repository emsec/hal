#include "boolean_influence/boolean_influence.h"
#include "graph_algorithm/algorithms/centrality.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "machine_learning/types.h"
#include "netlist_preprocessing/netlist_preprocessing.h"
#include "nlohmann/json.hpp"
#include "z3_utils/subgraph_function_generation.h"

namespace hal
{
    template<>
    std::map<machine_learning::StatisticalMoment, std::string> EnumStrings<machine_learning::StatisticalMoment>::data = {{machine_learning::StatisticalMoment::min, "min"},
                                                                                                                         {machine_learning::StatisticalMoment::max, "max"},
                                                                                                                         {machine_learning::StatisticalMoment::average, "average"},
                                                                                                                         {machine_learning::StatisticalMoment::median, "median"},
                                                                                                                         {machine_learning::StatisticalMoment::stddev, "stddev"}};

    namespace machine_learning
    {
        Result<FEATURE_TYPE> calculate_statistical_moment(StatisticalMoment moment, std::vector<FEATURE_TYPE> data)
        {
            if (data.empty())
            {
                return ERR("calculate: empty data");
            }

            switch (moment)
            {
                case StatisticalMoment::min:
                    return OK(*std::min_element(data.begin(), data.end()));

                case StatisticalMoment::max:
                    return OK(*std::max_element(data.begin(), data.end()));

                case StatisticalMoment::average: {
                    FEATURE_TYPE sum = std::accumulate(data.begin(), data.end(), static_cast<FEATURE_TYPE>(0));
                    return OK(sum / static_cast<FEATURE_TYPE>(data.size()));
                }

                case StatisticalMoment::median: {
                    std::sort(data.begin(), data.end());
                    auto n = data.size();
                    if (n % 2 == 0)
                    {
                        return OK((data[n / 2 - 1] + data[n / 2]) / static_cast<FEATURE_TYPE>(2));
                    }
                    else
                    {
                        return OK(data[n / 2]);
                    }
                }

                case StatisticalMoment::stddev: {
                    FEATURE_TYPE sum  = std::accumulate(data.begin(), data.end(), static_cast<FEATURE_TYPE>(0));
                    FEATURE_TYPE mean = sum / static_cast<FEATURE_TYPE>(data.size());

                    FEATURE_TYPE sq_sum = static_cast<FEATURE_TYPE>(0);
                    for (const auto& v : data)
                    {
                        sq_sum += (v - mean) * (v - mean);
                    }

                    return OK(std::sqrt(sq_sum / static_cast<FEATURE_TYPE>(data.size())));
                }
            }

            return ERR("calculate: unknown StatisticalMoment");
        }

        namespace
        {
            // Composite key identifying a candidate multi-bit word: (word_name, pin_direction, pin_name).
            using WordKey = std::tuple<std::string, PinDirection, std::string>;

            // One gate's membership in a candidate word.
            //
            // `distance` is the number of wire-assignment hops between the gate's pin and the
            // net whose indexed name was adopted as the source of this entry. 0 means the name
            // was taken from a net directly attached to the pin; larger values mean the name was
            // carried across intermediate `wire = wire` assignments in the written netlist
            // (e.g., `reg a = b; wire b = c; wire c = d` yields distances 0/1/2 for a/b/c from
            // the register pin). Smaller distances are preferred because names closer to the
            // gate are more likely to reflect the original signal identity. This struct holds
            // the already-filtered minimum distance per (gate, pin).
            struct GateEntry
            {
                u32 index;       // Bit index within the word.
                Gate* gate;      // Contributing gate.
                u32 distance;    // Wire-assignment hops from the gate pin to the source net (0 = adjacent, smaller is better).
            };

            // A word hypothesis before policy-based deduplication.
            //
            // Multiple candidates can refer to the same group of gates under different WordKeys
            // (e.g., input-side vs. output-side name reconstruction of the same register bus).
            // The scoring step picks a single winner per gate-group.
            struct CandidateWord
            {
                WordKey key;
                std::vector<GateEntry> entries;

                // Sorted-by-gate-id list of the candidate's gates; used as a stable dedupe key
                // across candidates that describe the same physical group.
                std::vector<Gate*> canonical_gates;

                // Populated only for the selected candidate, used when emitting the final mapping.
                std::unordered_map<Gate*, u32> gate_to_index;

                double avg_distance = std::numeric_limits<double>::infinity();

                // Policy signals computed once from `key` + `entries`.
                bool is_output_word        = false;    // true iff direction == PinDirection::output
                bool is_gate_name_encoding = false;    // true iff direction == none AND every entry's distance == 0
                                                       // (i.e., the index was encoded directly in the gate or the net name
                                                       // at the pin, with no intermediate wire hops).
            };

            // Ordered preference of pin types for a given gate-type property. Used by the Yosys
            // policy to break ties among output-pin candidates: pins earlier in the list win.
            using PinPriorityRule = std::pair<GateTypeProperty, std::vector<PinType>>;

            // Ordered list of pin-type priority rules. The first rule whose GateTypeProperty is
            // present on ALL gates of a candidate is applied; subsequent rules are ignored.
            // Extend here (in priority order) to teach new gate families about preferred outputs.
            static const std::vector<PinPriorityRule>& pin_priority_rules()
            {
                static const std::vector<PinPriorityRule> rules = {
                    {GateTypeProperty::ff, {PinType::state, PinType::neg_state}},
                };
                return rules;
            }

            // Return the pin-priority list that applies to every gate in `gv`, or nullptr if none
            // of the configured rules matches all gates uniformly.
            static const std::vector<PinType>* get_applicable_pin_priority(const std::vector<Gate*>& gv)
            {
                for (const auto& [gtp, plist] : pin_priority_rules())
                {
                    const bool applies_to_all = std::all_of(gv.begin(), gv.end(), [&](Gate* g) { return g != nullptr && g->get_type() != nullptr && g->get_type()->has_property(gtp); });

                    if (applies_to_all)
                    {
                        return &plist;
                    }
                }
                return nullptr;
            }

            // Rank a single (gate, pin_name) against the preferred pin-type list.
            // Lower values indicate higher priority. Ranking tiers:
            //   [0 .. plist.size()-1]   index into plist (matched preferred pin types)
            //   plist.size() + 1        known pin type but not in the preferred list
            //   plist.size() + 2        pin could not be resolved on this gate
            static size_t rank_pin_one_gate(Gate* g, const std::string& pin_name, const std::vector<PinType>& plist)
            {
                const size_t worst = plist.size() + 2;

                if (g == nullptr)
                {
                    return worst;
                }

                auto* ep = g->get_fan_out_endpoint(pin_name);
                if (ep == nullptr || ep->get_pin() == nullptr)
                {
                    return worst;
                }

                const PinType pt = ep->get_pin()->get_type();
                const auto it    = std::find(plist.begin(), plist.end(), pt);

                if (it == plist.end())
                {
                    // known pin type, but not in the preferred list -> worse than preferred, better than "unresolved"
                    return plist.size() + 1;
                }

                return static_cast<size_t>(std::distance(plist.begin(), it));
            }

            // Aggregate rank for a pin name across all gates of a candidate. Takes the worst
            // (largest) per-gate rank so a heterogeneous candidate cannot beat a homogeneous one
            // just because a single gate happens to have the preferred pin type.
            static size_t rank_pin_for_vector_worstcase(const std::vector<Gate*>& gv, const std::string& pin_name, const std::vector<PinType>& plist)
            {
                size_t worst_rank = 0;
                for (auto* g : gv)
                {
                    worst_rank = std::max(worst_rank, rank_pin_one_gate(g, pin_name, plist));
                }
                return worst_rank;
            }

            // Totally-ordered score for a candidate word. Smaller values are better.
            //
            // Fields are compared lexicographically in the order declared below:
            //   1. primary        - policy-specific 0/1 preference (e.g., Yosys prefers output words)
            //   2. avg_distance   - fewer wire-assignment hops between gate pins and source nets wins
            //   3. pin_type_rank  - Yosys-only: preferred output pin types win
            //   4. pin_len        - Yosys-only: shorter output pin names win when no rule matched
            //   5..8              - deterministic final tie-breakers (name length/text, direction, pin)
            struct CandidateScore
            {
                // Lower is better.
                int primary         = 0;
                double avg_distance = std::numeric_limits<double>::infinity();

                // Yosys-only tie-breakers (kept neutral for other policies).
                size_t pin_type_rank = 0;    // lower is better (only meaningful for yosys output ties)
                size_t pin_len       = 0;    // shorter pin name wins when no rule applies (yosys output ties)

                // Deterministic final tie-breakers
                size_t name_len = 0;
                std::string name;
                int dir = 0;
                std::string pin;

                friend bool operator<(const CandidateScore& a, const CandidateScore& b)
                {
                    return std::tie(a.primary, a.avg_distance, a.pin_type_rank, a.pin_len, a.name_len, a.name, a.dir, a.pin)
                           < std::tie(b.primary, b.avg_distance, b.pin_type_rank, b.pin_len, b.name_len, b.name, b.dir, b.pin);
                }
            };

            // Build a CandidateScore for `c` under the given multi-bit processing policy.
            // Only the policy-specific fields differ between policies; deterministic tie-breakers
            // are always filled so that scoring is total and reproducible.
            static CandidateScore score_candidate(const CandidateWord& c, MultiBitProcessingPolicy policy)
            {
                const auto& [name, dir, pin] = c.key;

                CandidateScore s;
                s.avg_distance = c.avg_distance;
                s.name_len     = name.size();
                s.name         = name;
                s.dir          = static_cast<int>(dir);
                s.pin          = pin;

                // defaults (neutral)
                s.pin_type_rank = 0;
                s.pin_len       = 0;

                switch (policy)
                {
                    case MultiBitProcessingPolicy::Default:
                        // just avg distance + deterministic tie-breakers
                        s.primary = 0;
                        break;

                    case MultiBitProcessingPolicy::Yosys: {
                        // output wins if ambiguous; then avg distance;
                        // if multiple output candidates tie, prioritize by pin type using the first
                        // GateTypeProperty rule that applies to ALL gates in the vector.
                        s.primary = c.is_output_word ? 0 : 1;

                        // Only meaningful when comparing output candidates against output candidates.
                        if (c.is_output_word)
                        {
                            s.pin_len = pin.size();

                            // apply first matching gate-type-property rule (if any)
                            if (const auto* plist = get_applicable_pin_priority(c.canonical_gates); plist != nullptr)
                            {
                                s.pin_type_rank = rank_pin_for_vector_worstcase(c.canonical_gates, pin, *plist);
                            }
                            else
                            {
                                // No rule applies to all gates -> behave "random like currently"
                                // but deterministic: let shorter pin name win (pin_len), then existing tie-breakers.
                                s.pin_type_rank = std::numeric_limits<size_t>::max();
                            }
                        }
                        else
                        {
                            // keep non-output candidates always worse on pin rank in case of any weird ties
                            s.pin_type_rank = std::numeric_limits<size_t>::max();
                            s.pin_len       = std::numeric_limits<size_t>::max();
                        }

                        break;
                    }

                    case MultiBitProcessingPolicy::Vivado:
                        // gate-name encoding wins if ambiguous; then avg distance
                        s.primary = c.is_gate_name_encoding ? 0 : 1;
                        break;
                }

                return s;
            }

            // Reconstruct multi-bit words from the "multi_bit_indexed_identifiers" gate-data
            // annotations previously written by the netlist preprocessing plugin.
            //
            // Pipeline:
            //   1. For every gate, read its indexed-identifier entries. Per pin, keep only the
            //      entries with the smallest wire-assignment distance — i.e., the names taken
            //      from the nets closest to the pin in the written netlist, before any
            //      intermediate wire-to-wire assignments.
            //   2. Group the surviving entries by WordKey (identifier, direction, pin).
            //   3. Turn each group into a CandidateWord, rejecting groups that have fewer than
            //      two distinct gates, duplicate indices, or duplicate gates.
            //   4. Deduplicate candidates that cover the same set of gates, keeping the one
            //      with the best CandidateScore under `policy`.
            //   5. Materialize the winners into a MultiBitInformation.
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

        // Two gates are considered a pair iff they share at least one word with the requested
        // direction. The match is done on the (identifier, direction) pair only; the pin name is
        // intentionally dropped so that e.g. two flip-flops in the same bus are paired even when
        // their words were derived through different physical pins.
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

        // Compare bit indices of two gates within a common word. If both gates participate in
        // multiple shared words the reference word is the one with the smallest combined gate
        // count (i.e., the most specific word), which yields stable comparisons when wider and
        // narrower buses overlap.
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

        const Result<std::unordered_map<const Net*, u32>*> Context::get_sequential_subgraph_function_inputs()
        {
            std::lock_guard<std::mutex> lock(m_functional_mutex);

            if (m_sequential_subgraph_function_inputs_cache)
            {
                return OK(m_sequential_subgraph_function_inputs_cache.get());
            }

            // Collect data-input nets of all sequential gates; skip clock/ground/power.
            const std::vector<PinType> excluded_types = {PinType::clock, PinType::ground, PinType::power};

            std::vector<Net*> target_nets;
            for (Gate* seq_gate : m_sequential_gates)
            {
                for (const auto* ep : seq_gate->get_fan_in_endpoints())
                {
                    if (ep->get_pin() == nullptr)
                    {
                        continue;
                    }
                    if (std::find(excluded_types.begin(), excluded_types.end(), ep->get_pin()->get_type()) != excluded_types.end())
                    {
                        continue;
                    }
                    Net* net = ep->get_net();
                    if (net != nullptr)
                    {
                        target_nets.push_back(net);
                    }
                }
            }

            for (Net* global_output_net : nl->get_global_output_nets())
            {
                target_nets.push_back(global_output_net);
            }

            // Deduplicate (multiple FFs may share an input net).
            std::sort(target_nets.begin(), target_nets.end());
            target_nets.erase(std::unique(target_nets.begin(), target_nets.end()), target_nets.end());

            SubgraphNetlistDecorator subgraph_dec(*nl);
            auto new_cache = std::make_shared<std::unordered_map<const Net*, u32>>();

            for (Net* net : target_nets)
            {
                const auto inputs_res = subgraph_dec.get_subgraph_function_inputs(GateTypeProperty::combinational, net);
                if (inputs_res.is_error())
                {
                    return ERR_APPEND(inputs_res.get_error(),
                                      "get_sequential_subgraph_function_inputs: failed to get inputs for net '" + net->get_name() + "' (ID " + std::to_string(net->get_id()) + ")");
                }
                new_cache->emplace(net, static_cast<u32>(inputs_res.get().size()));
            }

            m_sequential_subgraph_function_inputs_cache = std::move(new_cache);
            return OK(m_sequential_subgraph_function_inputs_cache.get());
        }

        const Result<std::unordered_map<const Net*, z3::expr>*> Context::get_sequential_subgraph_functions()
        {
            // Ensure input counts are available first (acquires and releases m_functional_mutex).
            auto inputs_result = get_sequential_subgraph_function_inputs();
            if (inputs_result.is_error())
            {
                return ERR_APPEND(inputs_result.get_error(), "get_sequential_subgraph_functions: failed to get subgraph function inputs");
            }
            const auto* inputs_cache = inputs_result.get();

            std::lock_guard<std::mutex> lock(m_functional_mutex);

            if (m_sequential_subgraph_function_cache)
            {
                return OK(m_sequential_subgraph_function_cache.get());
            }

            // Collect data-input nets of all sequential gates; skip clock/ground/power.
            const std::vector<PinType> excluded_types = {PinType::clock, PinType::ground, PinType::power};

            std::vector<Net*> target_nets;
            for (Gate* seq_gate : m_sequential_gates)
            {
                for (const auto* ep : seq_gate->get_fan_in_endpoints())
                {
                    if (ep->get_pin() == nullptr)
                    {
                        continue;
                    }
                    if (std::find(excluded_types.begin(), excluded_types.end(), ep->get_pin()->get_type()) != excluded_types.end())
                    {
                        continue;
                    }
                    Net* net = ep->get_net();
                    if (net != nullptr)
                    {
                        target_nets.push_back(net);
                    }
                }
            }

            for (Net* global_output_net : nl->get_global_output_nets())
            {
                target_nets.push_back(global_output_net);
            }

            // Deduplicate (multiple FFs may share an input net).
            std::sort(target_nets.begin(), target_nets.end());
            target_nets.erase(std::unique(target_nets.begin(), target_nets.end()), target_nets.end());

            // Filter out nets whose combinational subgraph exceeds the maximum input size.
            if (m_max_boolean_function_input_size > 0)
            {
                std::vector<Net*> filtered_nets;
                for (Net* net : target_nets)
                {
                    const auto it = inputs_cache->find(net);
                    if (it == inputs_cache->end())
                    {
                        return ERR("get_sequential_subgraph_functions: missing input count for net '" + net->get_name() + "' (ID " + std::to_string(net->get_id()) + ")");
                    }

                    if (it->second <= m_max_boolean_function_input_size)
                    {
                        filtered_nets.push_back(net);
                    }
                }
                target_nets = std::move(filtered_nets);
            }

            if (!m_z3_ctx)
            {
                m_z3_ctx = std::make_shared<z3::context>();
            }

            auto func_result = z3_utils::get_subgraph_z3_functions(GateTypeProperty::combinational, target_nets, *m_z3_ctx);
            if (func_result.is_error())
            {
                return ERR_APPEND(func_result.get_error(), "get_sequential_subgraph_functions: failed to compute z3 subgraph functions");
            }

            auto new_cache    = std::make_shared<std::unordered_map<const Net*, z3::expr>>();
            const auto& exprs = func_result.get();
            for (u32 i = 0; i < target_nets.size(); i++)
            {
                new_cache->emplace(target_nets.at(i), exprs.at(i));
            }

            m_sequential_subgraph_function_cache = std::move(new_cache);
            return OK(m_sequential_subgraph_function_cache.get());
        }

        const Result<std::unordered_map<const Net*, std::unordered_map<Net*, double>>*> Context::get_sequential_boolean_influences()
        {
            // Ensure subgraph functions are available first (acquires and releases m_functional_mutex).
            auto sf_result = get_sequential_subgraph_functions();
            if (sf_result.is_error())
            {
                return ERR_APPEND(sf_result.get_error(), "get_sequential_boolean_influences: failed to get subgraph functions");
            }
            const auto* subgraph_cache = sf_result.get();

            std::lock_guard<std::mutex> lock(m_functional_mutex);

            if (m_sequential_boolean_influence_cache)
            {
                return OK(m_sequential_boolean_influence_cache.get());
            }

            auto new_cache = std::make_shared<std::unordered_map<const Net*, std::unordered_map<Net*, double>>>();

            for (const auto& [dpin_net, expr] : *subgraph_cache)
            {
                auto influence_result = boolean_influence::get_boolean_influence_bitsliced(expr);
                if (influence_result.is_error())
                {
                    return ERR_APPEND(influence_result.get_error(),
                                      "get_sequential_boolean_influences: failed to compute boolean influence for net '" + dpin_net->get_name() + "' (ID " + std::to_string(dpin_net->get_id()) + ")");
                }
                auto net_influence_map = boolean_influence::translate_boolean_influence_map(this->nl, influence_result.get());

                if (net_influence_map.is_error())
                {
                    return ERR_APPEND(net_influence_map.get_error(),
                                      "get_sequential_boolean_influences: failed to translate boolean influence map for net '" + dpin_net->get_name() + "' (ID " + std::to_string(dpin_net->get_id())
                                          + ")");
                }

                new_cache->emplace(dpin_net, net_influence_map.get());
            }

            m_sequential_boolean_influence_cache = std::move(new_cache);
            return OK(m_sequential_boolean_influence_cache.get());
        }

        const Result<std::unordered_map<const Net*, std::vector<Net*>>*> Context::get_sequential_influenced_nets()
        {
            // Ensure boolean influences are available first (handles its own locking chain).
            auto bi_result = get_sequential_boolean_influences();
            if (bi_result.is_error())
            {
                return ERR_APPEND(bi_result.get_error(), "get_sequential_influenced_net: failed to get boolean influences");
            }
            const auto* influence_cache = bi_result.get();

            std::lock_guard<std::mutex> lock(m_functional_mutex);

            if (m_sequential_influenced_nets_cache)
            {
                return OK(m_sequential_influenced_nets_cache.get());
            }

            auto new_cache = std::make_shared<std::unordered_map<const Net*, std::vector<Net*>>>();

            for (const auto& [dpin_net, influence_map] : *influence_cache)
            {
                for (const auto& [net, _influence] : influence_map)
                {
                    (*new_cache)[net].push_back(const_cast<Net*>(dpin_net));
                }
            }

            m_sequential_influenced_nets_cache = std::move(new_cache);
            return OK(m_sequential_influenced_nets_cache.get());
        }
    }    // namespace machine_learning
}    // namespace hal