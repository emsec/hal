#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "machine_learning/features/gate_feature.h"

#include <set>
#include <vector>

#define MAX_CYCLE_LENGTH FEATURE_TYPE(255)

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
            namespace
            {
                bool pin_type_allowed(const Endpoint* ep, const std::vector<PinType>& forbidden_pin_types)
                {
                    return std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) == forbidden_pin_types.end();
                }

                /**
                 * Forward BFS from g's output endpoints; returns the shortest distance at which
                 * an input endpoint of g is reached. Output endpoints of g are seeded into the
                 * BFS visited set, so the BFS does not match at distance 0; the smallest match
                 * therefore corresponds to the shortest directed cycle length through g.
                 */
                Result<std::optional<u32>>
                    find_shortest_cycle_length(const NetlistAbstraction& abstr, const Gate* g, const i32 cutoff, const std::vector<PinType>& forbidden_pin_types)
                {
                    const auto target_filter = [g](const Endpoint* ep, const NetlistAbstraction& _a) {
                        UNUSED(_a);
                        return ep->get_pin()->get_direction() == PinDirection::input && ep->get_gate() == g;
                    };

                    const auto endpoint_filter = [forbidden_pin_types, cutoff](const Endpoint* ep, const u32 d) {
                        if (cutoff >= 0 && d > static_cast<u32>(cutoff))
                        {
                            return false;
                        }
                        return pin_type_allowed(ep, forbidden_pin_types);
                    };

                    return NetlistAbstractionDecorator(abstr).get_shortest_path_distance(g, target_filter, PinDirection::output, true, endpoint_filter, endpoint_filter);
                }

                /**
                 * Returns true iff there exists a closed directed walk of length exactly `length`
                 * from g back to g on the abstraction. Computed by iteratively expanding the set
                 * of gates reachable in exactly k steps for k = 1..length.
                 */
                Result<bool> in_cycle_of_length(const NetlistAbstraction& abstr, const Gate* g, const u32 length, const std::vector<PinType>& forbidden_pin_types)
                {
                    if (length == 0)
                    {
                        return OK(false);
                    }

                    std::set<const Gate*> current = {g};
                    for (u32 step = 0; step < length; step++)
                    {
                        std::set<const Gate*> next;
                        for (const auto* curr_g : current)
                        {
                            for (auto* exit_ep : curr_g->get_fan_out_endpoints())
                            {
                                if (!pin_type_allowed(exit_ep, forbidden_pin_types))
                                {
                                    continue;
                                }
                                const auto succs = abstr.get_successors(exit_ep);
                                if (succs.is_error())
                                {
                                    return ERR_APPEND(succs.get_error(), "failed to get successors during cycle expansion");
                                }
                                for (auto* succ_ep : succs.get())
                                {
                                    if (!pin_type_allowed(succ_ep, forbidden_pin_types))
                                    {
                                        continue;
                                    }
                                    next.insert(succ_ep->get_gate());
                                }
                            }
                        }
                        current = std::move(next);
                        if (current.empty())
                        {
                            return OK(false);
                        }
                    }
                    return OK(current.count(g) > 0);
                }

                std::string forbidden_pin_types_suffix(const std::vector<PinType>& forbidden_pin_types)
                {
                    const std::string s = utils::join("_", forbidden_pin_types.begin(), forbidden_pin_types.end(), [](const PinType& pt) { return enum_to_string(pt); });
                    return s.empty() ? "None" : s;
                }
            }    // namespace

            Result<std::vector<FEATURE_TYPE>> InCycle::calculate_feature(Context& ctx, const Gate* g) const
            {
                const auto nl_abstr = ctx.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                const auto distance = find_shortest_cycle_length(*nl_abstr.get(), g, m_cutoff, m_forbidden_pin_types);
                if (distance.is_error())
                {
                    return ERR_APPEND(distance.get_error(), "cannot calculate feature " + to_string());
                }

                return OK({distance.get().has_value() ? FEATURE_TYPE(1) : FEATURE_TYPE(0)});
            }

            std::string InCycle::to_string() const
            {
                return "InCycle_" + std::to_string(m_cutoff) + "_" + forbidden_pin_types_suffix(m_forbidden_pin_types);
            }

            std::vector<std::string> InCycle::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"in_cycle"};
            }

            Result<std::vector<FEATURE_TYPE>> ShortestCycleLength::calculate_feature(Context& ctx, const Gate* g) const
            {
                const auto nl_abstr = ctx.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                const auto distance = find_shortest_cycle_length(*nl_abstr.get(), g, m_cutoff, m_forbidden_pin_types);
                if (distance.is_error())
                {
                    return ERR_APPEND(distance.get_error(), "cannot calculate feature " + to_string());
                }

                if (!distance.get().has_value())
                {
                    return OK({MAX_CYCLE_LENGTH});
                }
                return OK({FEATURE_TYPE(std::min(distance.get().value(), u32(MAX_CYCLE_LENGTH)))});
            }

            std::string ShortestCycleLength::to_string() const
            {
                return "ShortestCycleLength_" + std::to_string(m_cutoff) + "_" + forbidden_pin_types_suffix(m_forbidden_pin_types);
            }

            std::vector<std::string> ShortestCycleLength::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"shortest_cycle_length"};
            }

            Result<std::vector<FEATURE_TYPE>> InCycleOfLength::calculate_feature(Context& ctx, const Gate* g) const
            {
                const auto nl_abstr = ctx.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                const auto in_cycle = in_cycle_of_length(*nl_abstr.get(), g, m_length, m_forbidden_pin_types);
                if (in_cycle.is_error())
                {
                    return ERR_APPEND(in_cycle.get_error(), "cannot calculate feature " + to_string());
                }

                return OK({in_cycle.get() ? FEATURE_TYPE(1) : FEATURE_TYPE(0)});
            }

            std::string InCycleOfLength::to_string() const
            {
                return "InCycleOfLength_" + std::to_string(m_length) + "_" + forbidden_pin_types_suffix(m_forbidden_pin_types);
            }

            std::vector<std::string> InCycleOfLength::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"in_cycle_of_length_" + std::to_string(m_length)};
            }

            Result<std::vector<FEATURE_TYPE>> SequentialInCycle::calculate_feature(Context& ctx, const Gate* g) const
            {
                if (!g->get_type()->has_property(GateTypeProperty::sequential))
                {
                    return OK({FEATURE_TYPE(0)});
                }

                const auto nl_abstr = ctx.get_sequential_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get sequential netlist abstraction");
                }

                const auto distance = find_shortest_cycle_length(*nl_abstr.get(), g, m_cutoff, m_forbidden_pin_types);
                if (distance.is_error())
                {
                    return ERR_APPEND(distance.get_error(), "cannot calculate feature " + to_string());
                }

                return OK({distance.get().has_value() ? FEATURE_TYPE(1) : FEATURE_TYPE(0)});
            }

            std::string SequentialInCycle::to_string() const
            {
                return "SequentialInCycle_" + std::to_string(m_cutoff) + "_" + forbidden_pin_types_suffix(m_forbidden_pin_types);
            }

            std::vector<std::string> SequentialInCycle::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"sequential_in_cycle"};
            }

            Result<std::vector<FEATURE_TYPE>> SequentialShortestCycleLength::calculate_feature(Context& ctx, const Gate* g) const
            {
                if (!g->get_type()->has_property(GateTypeProperty::sequential))
                {
                    return OK({MAX_CYCLE_LENGTH});
                }

                const auto nl_abstr = ctx.get_sequential_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get sequential netlist abstraction");
                }

                const auto distance = find_shortest_cycle_length(*nl_abstr.get(), g, m_cutoff, m_forbidden_pin_types);
                if (distance.is_error())
                {
                    return ERR_APPEND(distance.get_error(), "cannot calculate feature " + to_string());
                }

                if (!distance.get().has_value())
                {
                    return OK({MAX_CYCLE_LENGTH});
                }
                return OK({FEATURE_TYPE(std::min(distance.get().value(), u32(MAX_CYCLE_LENGTH)))});
            }

            std::string SequentialShortestCycleLength::to_string() const
            {
                return "SequentialShortestCycleLength_" + std::to_string(m_cutoff) + "_" + forbidden_pin_types_suffix(m_forbidden_pin_types);
            }

            std::vector<std::string> SequentialShortestCycleLength::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"sequential_shortest_cycle_length"};
            }

            Result<std::vector<FEATURE_TYPE>> SequentialInCycleOfLength::calculate_feature(Context& ctx, const Gate* g) const
            {
                if (!g->get_type()->has_property(GateTypeProperty::sequential))
                {
                    return OK({FEATURE_TYPE(0)});
                }

                const auto nl_abstr = ctx.get_sequential_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get sequential netlist abstraction");
                }

                const auto in_cycle = in_cycle_of_length(*nl_abstr.get(), g, m_length, m_forbidden_pin_types);
                if (in_cycle.is_error())
                {
                    return ERR_APPEND(in_cycle.get_error(), "cannot calculate feature " + to_string());
                }

                return OK({in_cycle.get() ? FEATURE_TYPE(1) : FEATURE_TYPE(0)});
            }

            std::string SequentialInCycleOfLength::to_string() const
            {
                return "SequentialInCycleOfLength_" + std::to_string(m_length) + "_" + forbidden_pin_types_suffix(m_forbidden_pin_types);
            }

            std::vector<std::string> SequentialInCycleOfLength::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"sequential_in_cycle_of_length_" + std::to_string(m_length)};
            }
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal
