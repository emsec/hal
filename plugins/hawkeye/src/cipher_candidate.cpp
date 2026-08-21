#include "hawkeye/cipher_candidate.h"

#include "graph_algorithm/algorithms/components.h"
#include "graph_algorithm/algorithms/subgraph.h"
#include "graph_algorithm/netlist_graph.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/user_feedback.h"

#include <algorithm>
#include <bitset>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    namespace hawkeye
    {
        namespace
        {
            /**
             * The given gates ordered by ID, so that neither the contents of a candidate nor the order in which it is
             * traversed depend on where the gates of the netlist happen to be allocated.
             */
            template<typename T>
            std::vector<Gate*> sorted_by_id(const T& gates)
            {
                std::vector<Gate*> res(gates.begin(), gates.end());
                std::sort(res.begin(), res.end(), [](const Gate* lhs, const Gate* rhs) { return lhs->get_id() < rhs->get_id(); });
                return res;
            }

            std::vector<u32> ids_of(const std::vector<Gate*>& gates)
            {
                std::vector<u32> res;
                res.reserve(gates.size());
                for (const auto* g : gates)
                {
                    res.push_back(g->get_id());
                }
                return res;
            }

            /** An S-box is looked up as a vector of `u8`, so it reads and writes at most 8 bits. */
            constexpr u32 MAX_SBOX_BITS = 8;

            /**
             * The most control inputs an S-box may read for identification to try every assignment of them. The
             * assignment loop is exponential in this, so every additional bit doubles what an S-box that matches
             * nothing in the database costs before that is known.
             */
            constexpr u32 MAX_CONTROL_BITS = 8;

            // Identification tabulates an S-box over its state inputs and its control inputs together, so the combined
            // table has to stay within what compute_truth_table will produce. Without this, raising one of the limits
            // would not fail to compile but would make identify_sbox return an error for every S-box that reads many
            // control inputs.
            static_assert(MAX_SBOX_BITS + MAX_CONTROL_BITS <= BooleanFunction::MAX_TRUTH_TABLE_VARIABLES,
                          "an S-box tabulated over its state and control inputs must stay within the truth table limit");

            /** The number of rows of the truth table of an S-box of `MAX_SBOX_BITS` input bits. */
            constexpr u32 MAX_SBOX_ROWS = 1 << MAX_SBOX_BITS;

            /**
             * Turn a truth table given as one row per input value, each holding the value of every output bit, into
             * the S-box that it describes and look that up in the database.
             *
             * Yields an empty name if the rows do not describe a usable S-box at all, that is, if there are more
             * outputs than inputs and they cannot be reduced to as many linearly independent ones as there are
             * inputs, or if the S-box is not bijective. Neither is an error: it only means that this guess at the
             * outputs of an S-box was a wrong one.
             */
            Result<std::string> lookup_sbox(const std::vector<std::vector<BooleanFunction::Value>>& rows, const u32 num_inputs, const SBoxDatabase& db)
            {
                if (num_inputs == 0 || num_inputs > MAX_SBOX_BITS)
                {
                    return ERR("an S-box of " + std::to_string(num_inputs) + " input bits is not supported, expected between 1 and " + std::to_string(MAX_SBOX_BITS));
                }

                const u32 num_rows = 1 << num_inputs;
                if (rows.size() != num_rows)
                {
                    return ERR("expected " + std::to_string(num_rows) + " rows for " + std::to_string(num_inputs) + " input bits, got " + std::to_string(rows.size()));
                }
                const u32 num_outputs = rows.front().size();

                std::vector<u64> values;
                values.reserve(num_rows);
                for (const auto& row : rows)
                {
                    const auto u64_res = BooleanFunction::to_u64(row);
                    if (u64_res.is_error())
                    {
                        return ERR(u64_res.get_error());
                    }
                    values.push_back(u64_res.get());
                }

                // More outputs than inputs means that some of them are linear combinations of the others, so reduce
                // to a linearly independent set by Gaussian elimination over GF(2) and keep only those outputs. Every
                // output is one vector over the rows of the table, which is why the elimination runs over the rows.
                if (num_outputs != num_inputs)
                {
                    std::vector<std::bitset<MAX_SBOX_ROWS>> mat(num_outputs);
                    for (u32 out = 0; out < num_outputs; out++)
                    {
                        for (u32 row = 0; row < num_rows; row++)
                        {
                            mat[out][row] = (values[row] >> out) & 1;
                        }
                    }

                    for (u32 row = 0; row < num_rows; row++)
                    {
                        u32 pivot = 0;
                        for (u32 out = 0; out < num_outputs; out++)
                        {
                            if (mat[out][row])
                            {
                                pivot = out;
                                break;
                            }
                        }
                        for (u32 out = pivot + 1; out < num_outputs; out++)
                        {
                            if (mat[out][row])
                            {
                                mat[out] ^= mat[pivot];
                            }
                        }
                    }

                    std::vector<u32> independent;
                    for (u32 out = 0; out < num_outputs; out++)
                    {
                        if (mat[out].any())
                        {
                            independent.push_back(out);
                        }
                    }

                    if (independent.size() != num_inputs)
                    {
                        return OK(std::string());
                    }

                    for (auto& value : values)
                    {
                        u64 reduced = 0;
                        for (u32 i = 0; i < independent.size(); i++)
                        {
                            reduced |= ((value >> independent.at(i)) & 1) << i;
                        }
                        value = reduced;
                    }
                }

                std::vector<u8> sbox;
                sbox.reserve(values.size());
                for (const auto value : values)
                {
                    sbox.push_back((u8)value);
                }

                if (std::set<u8>(sbox.begin(), sbox.end()).size() != sbox.size())
                {
                    // an S-box is a permutation, so a value occurring twice rules this one out
                    return OK(std::string());
                }

                if (const auto lookup_res = db.lookup(sbox); lookup_res.is_ok())
                {
                    return OK(lookup_res.get());
                }
                return OK(std::string());
            }
        }    // namespace

        std::vector<Gate*> SBox::get_combinational_gates() const
        {
            // Walk back from the outputs to the flip-flops, staying within the component. A component split into
            // several S-boxes is shared by all of them, so taking the component itself would hand out the logic of the
            // other S-boxes as well.
            const std::unordered_set<Gate*> in_component(component.begin(), component.end());

            std::unordered_set<Gate*> visited;
            std::vector<Gate*> stack(output_gates.begin(), output_gates.end());
            while (!stack.empty())
            {
                auto* current_gate = stack.back();
                stack.pop_back();

                if (!visited.insert(current_gate).second)
                {
                    continue;
                }

                for (auto* pred_gate : current_gate->get_unique_predecessors())
                {
                    if (in_component.count(pred_gate) && !pred_gate->get_type()->has_property(GateTypeProperty::ff))
                    {
                        stack.push_back(pred_gate);
                    }
                }
            }

            return sorted_by_id(visited);
        }

        CipherCandidate::CipherCandidate(const std::set<Gate*>& round_reg)
        {
            m_in_reg         = sorted_by_id(round_reg);
            m_out_reg        = m_in_reg;
            m_in_reg_ids     = ids_of(m_in_reg);
            m_out_reg_ids    = m_in_reg_ids;
            m_size           = m_in_reg.size();
            m_netlist        = m_in_reg.front()->get_netlist();
            m_is_round_based = true;
        }

        CipherCandidate::CipherCandidate(const std::set<Gate*>& in_reg, const std::set<Gate*>& out_reg)
        {
            m_in_reg         = sorted_by_id(in_reg);
            m_out_reg        = sorted_by_id(out_reg);
            m_in_reg_ids     = ids_of(m_in_reg);
            m_out_reg_ids    = ids_of(m_out_reg);
            m_size           = m_out_reg.size();
            m_netlist        = m_out_reg.front()->get_netlist();
            m_is_round_based = m_in_reg == m_out_reg;
        }

        bool CipherCandidate::operator<(const CipherCandidate& rhs) const
        {
            // larger candidates come first, as reducing a set of candidates relies on a candidate being visited before
            // the smaller candidates that it may contain. The registers only break ties among candidates of equal size
            // and are compared by gate ID, so that the order does not depend on where the gates are allocated.
            if (this->m_size != rhs.m_size)
            {
                return this->m_size > rhs.m_size;
            }
            if (this->m_in_reg_ids != rhs.m_in_reg_ids)
            {
                return this->m_in_reg_ids < rhs.m_in_reg_ids;
            }
            return this->m_out_reg_ids < rhs.m_out_reg_ids;
        }

        bool CipherCandidate::operator==(const CipherCandidate& rhs) const
        {
            return this->m_size == rhs.m_size && this->m_in_reg_ids == rhs.m_in_reg_ids && this->m_out_reg_ids == rhs.m_out_reg_ids;
        }

        Netlist* CipherCandidate::get_netlist() const
        {
            return m_netlist;
        }

        u32 CipherCandidate::get_size() const
        {
            return m_size;
        }

        bool CipherCandidate::is_round_based() const
        {
            return m_is_round_based;
        }

        bool CipherCandidate::has_round_function() const
        {
            return m_has_round_function;
        }

        const std::vector<Gate*>& CipherCandidate::get_input_reg() const
        {
            return m_in_reg;
        }

        const std::vector<Gate*>& CipherCandidate::get_output_reg() const
        {
            return m_out_reg;
        }

        const std::vector<Gate*>& CipherCandidate::get_round_logic() const
        {
            return m_round_logic;
        }

        std::vector<Gate*> CipherCandidate::get_gates() const
        {
            std::set<Gate*> all(m_in_reg.begin(), m_in_reg.end());
            all.insert(m_out_reg.begin(), m_out_reg.end());
            all.insert(m_round_logic.begin(), m_round_logic.end());
            return sorted_by_id(all);
        }

        std::vector<SBox*> CipherCandidate::get_sboxes() const
        {
            std::vector<SBox*> res;
            res.reserve(m_sboxes.size());
            for (const auto& sbox : m_sboxes)
            {
                res.push_back(sbox.get());
            }
            return res;
        }

        graph_algorithm::NetlistGraph* CipherCandidate::get_graph() const
        {
            return m_graph.get();
        }

        const std::set<Net*>& CipherCandidate::get_state_inputs() const
        {
            return m_state_inputs;
        }

        const std::set<Net*>& CipherCandidate::get_control_inputs() const
        {
            return m_control_inputs;
        }

        const std::set<Net*>& CipherCandidate::get_other_inputs() const
        {
            return m_other_inputs;
        }

        const std::set<Net*>& CipherCandidate::get_state_outputs() const
        {
            return m_state_outputs;
        }

        const std::map<Gate*, std::set<Gate*>>& CipherCandidate::get_input_ffs_of_gate() const
        {
            compute_gate_dependencies();
            return m_input_ffs_of_gate;
        }

        const std::map<u32, std::set<Gate*>>& CipherCandidate::get_longest_distance_to_gate() const
        {
            compute_gate_dependencies();
            return m_longest_distance_to_gate;
        }

        void CipherCandidate::clear_sboxes()
        {
            m_sboxes.clear();
        }

        Result<std::vector<SBox*>> CipherCandidate::locate_sboxes()
        {
            if (!m_has_round_function)
            {
                return ERR("round function has not been computed, call build_round_function first");
            }

            // locating the S-boxes again would produce the same result, so hand out the ones located before rather
            // than invalidating the pointers to them. Use clear_sboxes to locate them anew.
            if (!m_sboxes.empty())
            {
                return OK(get_sboxes());
            }

            const user_feedback::ProgressScope progress("hawkeye: locating S-boxes …");

            const std::unordered_set<Gate*> in_reg_lookup(m_in_reg.begin(), m_in_reg.end());
            const std::unordered_set<Gate*> out_reg_lookup(m_out_reg.begin(), m_out_reg.end());

            // The candidate used to be copied into a netlist of its own, so that looking at the neighbors of a gate
            // could not reach anything but the candidate. Working on the netlist itself, every such lookup has to be
            // restricted to the candidate explicitly.
            const auto candidate_gates = get_gates();
            const std::unordered_set<Gate*> candidate_lookup(candidate_gates.begin(), candidate_gates.end());

            auto comp_res = graph_algorithm::get_connected_components(m_graph.get(), false);
            if (comp_res.is_error())
            {
                return ERR(comp_res.get_error());
            }

            for (const auto& component_vertices : comp_res.get())
            {
                // A state flip-flop of a round-based candidate is present twice in the graph, as the primary vertex
                // feeding the round function and as the shadow vertex it writes back to. Both resolve to the same
                // gate, so the role has to be read off the vertex: only the primary vertex of an input register gate
                // is an input of this component. Deciding this by gate identity instead would count both as inputs
                // for a round-based candidate, where the input and the output register are the very same gates.
                std::set<Gate*> component_input_ffs;
                std::set<Gate*> component_gates;
                for (const u32 vertex : component_vertices)
                {
                    const auto gate_res = m_graph->get_gate_from_vertex(vertex);
                    if (gate_res.is_error())
                    {
                        return ERR(gate_res.get_error());
                    }
                    auto* g = gate_res.get();
                    if (g == nullptr)
                    {
                        continue;
                    }

                    component_gates.insert(g);
                    if (!m_graph->is_shadow_vertex(vertex) && in_reg_lookup.count(g))
                    {
                        component_input_ffs.insert(g);
                    }
                }

                const std::vector<Gate*> component = sorted_by_id(component_gates);
                const u32 number_input_ffs         = component_input_ffs.size();

                if (number_input_ffs < 3)
                {
                    // too small for an S-box
                    continue;
                }

                if (number_input_ffs <= MAX_SBOX_BITS)
                {
                    // assume a single S-box was found
                    std::set<Gate*> sbox_output_gates;
                    for (auto* cand_gate : component)
                    {
                        if (cand_gate->get_type()->has_property(GateTypeProperty::ff))
                        {
                            continue;
                        }

                        // output gates are the combinational gates whose successors within this candidate all belong
                        // to the output register
                        const auto suc_gates = cand_gate->get_unique_successors();
                        if (std::none_of(suc_gates.begin(), suc_gates.end(), [&](Gate* g) { return candidate_lookup.count(g) && !out_reg_lookup.count(g); }))
                        {
                            sbox_output_gates.insert(cand_gate);
                        }
                    }

                    if (sbox_output_gates.size() == number_input_ffs)
                    {
                        auto sbox          = std::make_unique<SBox>();
                        sbox->component    = component;
                        sbox->input_gates  = sorted_by_id(component_input_ffs);
                        sbox->output_gates = sorted_by_id(sbox_output_gates);
                        m_sboxes.push_back(std::move(sbox));
                    }
                    continue;
                }

                // A component reading more than 8 flip-flops is too wide to be a single S-box, so assume it holds
                // several of them followed by the linear layer that mixes their outputs. Grow a subset of the round
                // function outwards from the register one distance level at a time and watch how it falls apart into
                // sub-components: as long as the linear layer has not mixed the S-boxes yet, every sub-component
                // covers one S-box, and the input flip-flops of a sub-component form the input group of that S-box.
                // the only place that needs to know what each gate depends on, so only pay for it here
                compute_gate_dependencies();

                std::set<Gate*> current_subset = component_input_ffs;
                std::vector<std::vector<std::set<Gate*>>> input_groupings;

                const u32 max_distance = m_longest_distance_to_gate.empty() ? 0 : m_longest_distance_to_gate.rbegin()->first;
                for (u32 step = 1; step <= max_distance; step++)
                {
                    const auto dist_it = m_longest_distance_to_gate.find(step);
                    if (dist_it == m_longest_distance_to_gate.end())
                    {
                        // no gate sits at this distance, so nothing further out is connected to the register either
                        break;
                    }
                    current_subset.insert(dist_it->second.begin(), dist_it->second.end());

                    auto subgraph_res = graph_algorithm::get_subgraph(m_graph.get(), sorted_by_id(current_subset));
                    if (subgraph_res.is_error())
                    {
                        return ERR(subgraph_res.get_error());
                    }
                    const auto subgraph = std::move(subgraph_res.get());

                    auto sub_comp_res = graph_algorithm::get_connected_components(subgraph.get(), false);
                    if (sub_comp_res.is_error())
                    {
                        return ERR(sub_comp_res.get_error());
                    }

                    // determine the input groups feeding into distinct sub-circuits
                    std::set<u32> group_sizes;
                    std::vector<std::set<Gate*>> input_groups;
                    for (const auto& sub_component : sub_comp_res.get())
                    {
                        auto gates_res = subgraph->get_gates_from_vertices(sub_component);
                        if (gates_res.is_error())
                        {
                            return ERR(gates_res.get_error());
                        }

                        // Only sub-components of the component at hand are of interest. Since the subset spans the
                        // whole round function, the other components are present as well, but they are disconnected
                        // from this one and hence never share an input flip-flop with it.
                        std::set<Gate*> input_group;
                        for (auto* sub_gate : gates_res.get())
                        {
                            if (component_input_ffs.count(sub_gate))
                            {
                                input_group.insert(sub_gate);
                            }
                        }

                        if (input_group.empty())
                        {
                            continue;
                        }
                        group_sizes.insert(input_group.size());
                        input_groups.push_back(std::move(input_group));
                    }

                    // the S-boxes of a round function are alike, so only accept a split into more than one group of
                    // equal size, each reading more than one flip-flop
                    if (group_sizes.size() == 1 && input_groups.size() > 1 && input_groups.front().size() > 1)
                    {
                        input_groupings.push_back(std::move(input_groups));
                    }
                }

                for (const auto& input_groups : input_groupings)
                {
                    for (const auto& input_group : input_groups)
                    {
                        // the output gates of the S-box reading this input group are the gates that depend on the
                        // whole group, on nothing outside it, and that feed something which does mix in other inputs
                        std::set<Gate*> output_group;
                        for (auto* comp_gate : component)
                        {
                            if (in_reg_lookup.count(comp_gate) || out_reg_lookup.count(comp_gate))
                            {
                                continue;
                            }

                            const auto ffs_it = m_input_ffs_of_gate.find(comp_gate);
                            if (ffs_it == m_input_ffs_of_gate.end() || ffs_it->second.size() <= 1)
                            {
                                // disregard gates that depend on at most one input flip-flop
                                continue;
                            }

                            // disregard gates that depend on input flip-flops outside the sub-component
                            if (!std::includes(input_group.begin(), input_group.end(), ffs_it->second.begin(), ffs_it->second.end()))
                            {
                                continue;
                            }

                            // disregard gates whose successors all stay within the sub-component, as an S-box output
                            // has to reach the linear layer mixing it with the outputs of the other S-boxes
                            bool feeds_other_inputs = false;
                            for (auto* suc_gate : comp_gate->get_unique_successors())
                            {
                                const auto suc_it = m_input_ffs_of_gate.find(suc_gate);
                                if (!candidate_lookup.count(suc_gate) || suc_it == m_input_ffs_of_gate.end())
                                {
                                    continue;
                                }
                                if (!std::includes(input_group.begin(), input_group.end(), suc_it->second.begin(), suc_it->second.end()))
                                {
                                    feeds_other_inputs = true;
                                    break;
                                }
                            }
                            if (!feeds_other_inputs)
                            {
                                continue;
                            }

                            // disregard inverters behind output gates, as inverting an output does not make for
                            // another one. The removal below covers this as well, but only once the group is complete.
                            if (comp_gate->get_type()->has_property(GateTypeProperty::c_inverter))
                            {
                                auto preds = comp_gate->get_unique_predecessors();
                                preds.erase(std::remove_if(preds.begin(), preds.end(), [&](Gate* g) { return !candidate_lookup.count(g); }), preds.end());
                                std::sort(preds.begin(), preds.end());
                                if (std::includes(output_group.begin(), output_group.end(), preds.begin(), preds.end()))
                                {
                                    continue;
                                }
                            }

                            output_group.insert(comp_gate);
                        }

                        // disregard output gates that only depend on other output gates
                        std::vector<Gate*> to_delete;
                        for (auto* out_gate : output_group)
                        {
                            auto pred_gates = out_gate->get_unique_predecessors();
                            pred_gates.erase(std::remove_if(pred_gates.begin(), pred_gates.end(), [&](Gate* g) { return !candidate_lookup.count(g); }), pred_gates.end());
                            if (std::all_of(pred_gates.begin(), pred_gates.end(), [&output_group](Gate* g) { return output_group.count(g); }))
                            {
                                to_delete.push_back(out_gate);
                            }
                        }
                        for (auto* del_gate : to_delete)
                        {
                            output_group.erase(del_gate);
                        }

                        if (input_group.size() > MAX_SBOX_BITS || output_group.empty() || output_group.size() > 20)
                        {
                            continue;
                        }

                        const std::vector<Gate*> inputs  = sorted_by_id(input_group);
                        const std::vector<Gate*> outputs = sorted_by_id(output_group);

                        auto add_sbox = [&](std::vector<Gate*> output_gates) {
                            auto sbox          = std::make_unique<SBox>();
                            sbox->component    = component;
                            sbox->input_gates  = inputs;
                            sbox->output_gates = std::move(output_gates);
                            m_sboxes.push_back(std::move(sbox));
                        };

                        // An S-box is square, but which of the gates found are the outputs is not certain, so guess:
                        // for one or two gates too many, every way of dropping the surplus becomes an S-box of its own.
                        // Anything else is taken as it is, and identification sorts out which guess was right.
                        if (outputs.size() == inputs.size() + 1)
                        {
                            for (u32 drop = 0; drop < outputs.size(); drop++)
                            {
                                std::vector<Gate*> reduced = outputs;
                                reduced.erase(reduced.begin() + drop);
                                add_sbox(std::move(reduced));
                            }
                        }
                        else if (outputs.size() == inputs.size() + 2)
                        {
                            for (u32 drop_1 = 0; drop_1 < outputs.size(); drop_1++)
                            {
                                for (u32 drop_2 = drop_1 + 1; drop_2 < outputs.size(); drop_2++)
                                {
                                    std::vector<Gate*> reduced = outputs;
                                    reduced.erase(reduced.begin() + drop_2);
                                    reduced.erase(reduced.begin() + drop_1);
                                    add_sbox(std::move(reduced));
                                }
                            }
                        }
                        else
                        {
                            add_sbox(outputs);
                        }
                    }
                }
            }

            log_info("hawkeye", "located {} S-boxes within the round function of the candidate.", m_sboxes.size());
            return OK(get_sboxes());
        }

        Result<std::monostate> CipherCandidate::build_round_function()
        {
            if (m_netlist == nullptr)
            {
                return ERR("candidate is empty");
            }

            const user_feedback::ProgressScope progress("hawkeye: computing round function …");

            // the S-boxes are derived from the round function, so they do not survive it being computed anew
            clear_sboxes();
            m_round_logic.clear();
            m_state_inputs.clear();
            m_state_outputs.clear();
            m_control_inputs.clear();
            m_other_inputs.clear();
            m_input_ffs_of_gate.clear();
            m_longest_distance_to_gate.clear();
            m_has_gate_dependencies = false;
            m_graph.reset();
            m_has_round_function = false;

            const std::unordered_set<Gate*> in_reg_lookup(m_in_reg.begin(), m_in_reg.end());
            std::set<Gate*> state_logic;

            // walk backwards from every output flip-flop to the input register, collecting the combinational gates on
            // the way. The registers are iterated in order of their gate IDs so that the traversal, and hence the
            // shortcut below, does not depend on where the gates are allocated.
            for (const auto* out_ff : m_out_reg)
            {
                auto ff_data_predecessors = out_ff->get_predecessors([](const GatePin* p, const Endpoint*) { return p->get_type() == PinType::data; });
                if (ff_data_predecessors.size() != 1)
                {
                    // a flip-flop can only have one predecessor at its data input
                    continue;
                }

                const auto* pred_ep   = ff_data_predecessors.at(0);
                auto* first_comb_gate = pred_ep->get_gate();
                if (!first_comb_gate->get_type()->has_property(GateTypeProperty::combinational))
                {
                    continue;
                }
                m_state_outputs.insert(pred_ep->get_net());

                std::unordered_set<Gate*> visited;
                std::vector<Gate*> stack = {first_comb_gate};
                std::vector<Gate*> previous;
                while (!stack.empty())
                {
                    auto* current_gate = stack.back();

                    // pop the stack if the gate on top has been dealt with completely
                    if (!previous.empty() && previous.back() == current_gate)
                    {
                        stack.pop_back();
                        previous.pop_back();
                        continue;
                    }

                    visited.insert(current_gate);

                    bool added = false;
                    for (auto* next_predecessor : current_gate->get_predecessors())
                    {
                        auto* predecessor_gate = next_predecessor->get_gate();
                        if (predecessor_gate->get_type()->has_property(GateTypeProperty::ff))
                        {
                            // reaching the input register means the current path computes part of the next state
                            if (in_reg_lookup.find(predecessor_gate) != in_reg_lookup.end())
                            {
                                m_state_inputs.insert(next_predecessor->get_net());
                                state_logic.insert(current_gate);
                                state_logic.insert(previous.begin(), previous.end());
                            }
                        }
                        else if (predecessor_gate->get_type()->has_property(GateTypeProperty::combinational))
                        {
                            if (visited.find(predecessor_gate) == visited.end())
                            {
                                stack.push_back(predecessor_gate);
                                added = true;
                            }
                            else if (state_logic.find(predecessor_gate) != state_logic.end())
                            {
                                state_logic.insert(current_gate);
                                state_logic.insert(previous.begin(), previous.end());
                            }
                        }
                    }

                    if (added)
                    {
                        previous.push_back(current_gate);
                    }
                    else
                    {
                        stack.pop_back();
                    }
                }
            }

            m_round_logic = sorted_by_id(state_logic);

            // split the remaining inputs of the round function into control inputs, which reach most of it, and others
            std::set<Net*> visited_nets;
            for (auto* gate : m_round_logic)
            {
                for (auto* in_net : gate->get_fan_in_nets())
                {
                    if (!visited_nets.insert(in_net).second)
                    {
                        continue;
                    }

                    if (in_net->get_num_of_sources() != 1)
                    {
                        continue;
                    }

                    if (m_state_inputs.find(in_net) != m_state_inputs.end())
                    {
                        continue;
                    }

                    auto* src_gate = in_net->get_sources().at(0)->get_gate();
                    if (state_logic.find(src_gate) != state_logic.end())
                    {
                        continue;
                    }

                    const u32 num_state_destinations = in_net->get_num_of_destinations([&state_logic](const Endpoint* ep) { return state_logic.find(ep->get_gate()) != state_logic.end(); });
                    if (num_state_destinations > m_size / 2)
                    {
                        m_control_inputs.insert(in_net);
                    }
                    else
                    {
                        m_other_inputs.insert(in_net);
                    }
                }
            }

            // Build the graph of the round function on the netlist itself. Only the gates of the candidate become
            // vertices, so the graph is the closed world that the copied partial netlist used to provide. A round-based
            // candidate reads and writes the same register, which would close a cycle through every state flip-flop and
            // merge all S-boxes into a single connected component, so its register is split into a source and a sink
            // vertex. A pipelined candidate needs no splitting, as neither register has both of its sides inside the
            // graph to begin with.
            std::set<Gate*> split_gates;
            if (m_is_round_based)
            {
                split_gates.insert(m_out_reg.begin(), m_out_reg.end());
            }

            auto graph_res = graph_algorithm::NetlistGraph::from_gates(get_gates(), split_gates);
            if (graph_res.is_error())
            {
                return ERR(graph_res.get_error());
            }
            m_graph = std::move(graph_res.get());

            m_has_round_function = true;
            return OK({});
        }

        void CipherCandidate::compute_gate_dependencies() const
        {
            if (m_has_gate_dependencies)
            {
                return;
            }
            m_has_gate_dependencies = true;

            const user_feedback::ProgressScope progress("hawkeye: analyzing the round function …");

            const std::unordered_set<Gate*> out_reg_lookup(m_out_reg.begin(), m_out_reg.end());
            const std::unordered_set<Gate*> round_logic_lookup(m_round_logic.begin(), m_round_logic.end());

            // Walk forwards from every input flip-flop to record which of them each gate depends on. Visiting a gate
            // once per flip-flop is what makes this the expensive part of analyzing a candidate; visiting it once per
            // *path* would be exponential in a cone that reconverges, which is what a wide false positive looks like.
            for (auto* in_ff : m_in_reg)
            {
                std::unordered_set<Gate*> visited = {in_ff};
                std::vector<Gate*> stack        = {in_ff};
                while (!stack.empty())
                {
                    auto* current_gate = stack.back();
                    stack.pop_back();

                    m_input_ffs_of_gate[current_gate].insert(in_ff);

                    for (auto* next_successor : current_gate->get_successors())
                    {
                        auto* successor_gate = next_successor->get_gate();
                        if (successor_gate->get_type()->has_property(GateTypeProperty::ff))
                        {
                            // the register is where the round function ends, so record the dependency but stop here
                            if (out_reg_lookup.find(successor_gate) != out_reg_lookup.end())
                            {
                                m_input_ffs_of_gate[successor_gate].insert(in_ff);
                            }
                        }
                        else if (round_logic_lookup.find(successor_gate) != round_logic_lookup.end())
                        {
                            if (visited.insert(successor_gate).second)
                            {
                                stack.push_back(successor_gate);
                            }
                        }
                    }
                }
            }

            // The distance of a gate is the length of the longest path reaching it from any input flip-flop. Enumerating
            // the paths to find the longest one is what the reachability walk above must not do, so relax the distances
            // along a topological order of the round function instead, which visits every edge exactly once. The round
            // function is acyclic: it runs from the register to the register, and the register is not part of it.
            std::unordered_map<Gate*, u32> in_degree;
            in_degree.reserve(m_round_logic.size());
            for (auto* gate : m_round_logic)
            {
                in_degree[gate];
            }
            for (auto* gate : m_round_logic)
            {
                for (auto* next_successor : gate->get_successors())
                {
                    auto* successor_gate = next_successor->get_gate();
                    if (round_logic_lookup.find(successor_gate) != round_logic_lookup.end())
                    {
                        in_degree[successor_gate]++;
                    }
                }
            }

            // a gate driven by the register directly sits at distance one
            std::unordered_map<Gate*, u32> distance;
            distance.reserve(m_round_logic.size());
            for (auto* in_ff : m_in_reg)
            {
                for (auto* next_successor : in_ff->get_successors())
                {
                    auto* successor_gate = next_successor->get_gate();
                    if (round_logic_lookup.find(successor_gate) != round_logic_lookup.end())
                    {
                        distance[successor_gate] = std::max(distance[successor_gate], u32(1));
                    }
                }
            }

            std::vector<Gate*> ordered;
            ordered.reserve(m_round_logic.size());
            for (auto* gate : m_round_logic)
            {
                if (in_degree[gate] == 0)
                {
                    ordered.push_back(gate);
                }
            }
            for (u32 i = 0; i < ordered.size(); i++)
            {
                auto* gate = ordered.at(i);
                for (auto* next_successor : gate->get_successors())
                {
                    auto* successor_gate = next_successor->get_gate();
                    if (round_logic_lookup.find(successor_gate) == round_logic_lookup.end())
                    {
                        continue;
                    }

                    distance[successor_gate] = std::max(distance[successor_gate], distance[gate] + 1);
                    if (--in_degree[successor_gate] == 0)
                    {
                        ordered.push_back(successor_gate);
                    }
                }
            }

            if (ordered.size() != m_round_logic.size())
            {
                // a combinational loop would leave gates unordered, and their distance short rather than wrong
                log_warning("hawkeye", "the round function of the candidate is not acyclic, {} of its {} gates were not reached in topological order.", m_round_logic.size() - ordered.size(), m_round_logic.size());
            }

            for (const auto& [gate, gate_distance] : distance)
            {
                if (gate_distance != 0)
                {
                    m_longest_distance_to_gate[gate_distance].insert(gate);
                }
            }
        }

        Result<u32> CipherCandidate::identify_sboxes(const SBoxDatabase& db)
        {
            // The search produces one S-box per guess at which of the surplus gates are the outputs, so the S-boxes
            // reading the same input flip-flops are variants of one and the same S-box. Identifying one of them
            // answers the question for all of them, which is what keeps this affordable.
            std::map<std::vector<u32>, std::vector<SBox*>> variants_by_input;
            for (const auto& sbox : m_sboxes)
            {
                sbox->status = SBoxStatus::unidentified;
                sbox->identified_as.clear();
                variants_by_input[ids_of(sbox->input_gates)].push_back(sbox.get());
            }

            // The narrower S-boxes first, and by gate ID within one width. The wide groups tend to be several real
            // S-boxes merged through the surrounding logic: they rarely match anything, and looking their variants up
            // is the most expensive part of identification, as the canonical form search behind the lookup degenerates
            // on such glued-together tables. Trying the narrow groups first therefore lands the real matches before
            // any time is spent on the merged ones.
            std::vector<const std::vector<SBox*>*> groups;
            groups.reserve(variants_by_input.size());
            for (const auto& [_, variants] : variants_by_input)
            {
                groups.push_back(&variants);
            }
            std::stable_sort(groups.begin(), groups.end(), [](const auto* lhs, const auto* rhs) { return lhs->front()->input_gates.size() < rhs->front()->input_gates.size(); });

            const user_feedback::ProgressScope progress("hawkeye: identifying S-boxes …");

            u32 num_identified = 0;
            for (const auto* group : groups)
            {
                for (u32 i = 0; i < group->size(); i++)
                {
                    auto* variant       = group->at(i);
                    const auto name_res = identify_sbox(variant, db);
                    if (name_res.is_error())
                    {
                        return ERR(name_res.get_error());
                    }

                    if (name_res.get().empty())
                    {
                        // this guess at the outputs was a wrong one, so try the next variant
                        continue;
                    }

                    variant->identified_as = name_res.get();
                    variant->status        = SBoxStatus::identified;
                    num_identified++;

                    for (u32 j = i + 1; j < group->size(); j++)
                    {
                        group->at(j)->status = SBoxStatus::superseded;
                    }
                    break;
                }
            }

            log_info("hawkeye", "identified {} of the {} S-boxes of the candidate.", num_identified, m_sboxes.size());
            return OK(num_identified);
        }

        Result<std::string> CipherCandidate::identify_sbox(const SBox* sbox, const SBoxDatabase& db) const
        {
            if (sbox == nullptr)
            {
                return ERR("S-box is a nullptr");
            }

            if (!m_has_round_function)
            {
                return ERR("round function has not been computed, call build_round_function first");
            }

            if (sbox->input_gates.empty() || sbox->output_gates.empty())
            {
                return ERR("S-box has no input or no output gates");
            }

            // The S-box computes its outputs from its input flip-flops, so its combinational gates are the subgraph
            // that the output functions are taken over. Leaving the flip-flops out of it makes them the inputs of
            // those functions instead of being traversed through.
            const std::vector<Gate*> subgraph_gates = sbox->get_combinational_gates();

            const auto snd = SubgraphNetlistDecorator(*m_netlist);
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> cache;

            std::vector<BooleanFunction> bfs;
            std::set<Net*> all_inputs;
            for (const auto* out_gate : sbox->output_gates)
            {
                const auto& fan_out_nets = out_gate->get_fan_out_nets();
                if (fan_out_nets.size() != 1)
                {
                    return ERR("gate '" + out_gate->get_name() + "' with ID " + std::to_string(out_gate->get_id())
                               + " has none or multiple fan-out nets, which is currently not supported");
                }

                auto bf_res = snd.get_subgraph_function(subgraph_gates, fan_out_nets.front(), cache);
                if (bf_res.is_error())
                {
                    return ERR(bf_res.get_error());
                }
                bfs.push_back(bf_res.get());

                // gather the nets that the component actually reads, which can be fewer than it is connected to
                for (const auto& var : bfs.back().get_variable_names())
                {
                    const auto net_res = BooleanFunctionNetDecorator::get_net_from(m_netlist, var);
                    if (net_res.is_error())
                    {
                        return ERR(net_res.get_error());
                    }
                    all_inputs.insert(net_res.get());
                }
            }

            // split the inputs read by this S-box the same way the round function as a whole was split
            std::set<Net*> state_inputs, control_inputs, other_inputs;
            std::set_intersection(all_inputs.begin(), all_inputs.end(), m_state_inputs.begin(), m_state_inputs.end(), std::inserter(state_inputs, state_inputs.begin()));
            std::set_intersection(all_inputs.begin(), all_inputs.end(), m_control_inputs.begin(), m_control_inputs.end(), std::inserter(control_inputs, control_inputs.begin()));
            std::set_intersection(all_inputs.begin(), all_inputs.end(), m_other_inputs.begin(), m_other_inputs.end(), std::inserter(other_inputs, other_inputs.begin()));

            if (state_inputs.empty() || state_inputs.size() > MAX_SBOX_BITS)
            {
                log_info("hawkeye", "skipping an S-box that reads {} state inputs, which is not a supported S-box width.", state_inputs.size());
                return OK(std::string());
            }

            if (control_inputs.size() > MAX_CONTROL_BITS)
            {
                log_info("hawkeye", "skipping an S-box that reads {} control inputs, which is too many to try every assignment of.", control_inputs.size());
                return OK(std::string());
            }

            // hold everything that is neither state nor control at '0'
            const auto bf_zero = BooleanFunction::Const(0, 1);
            for (auto& bf : bfs)
            {
                for (const auto* other_in : other_inputs)
                {
                    auto sub_res = bf.substitute(BooleanFunctionNetDecorator(*other_in).get_boolean_variable_name(), bf_zero);
                    if (sub_res.is_error())
                    {
                        return ERR(sub_res.get_error());
                    }
                    bf = sub_res.get();
                }
            }

            // Tabulate every output over the state and the control inputs together, so that an assignment of the
            // control inputs becomes a slice of that one table instead of a table of its own. Ordering the state
            // inputs first makes the rows of one assignment contiguous: the state inputs occupy the low bits of a row
            // index and the control inputs the high ones, so assignment i covers the rows i << |state| to
            // (i + 1) << |state|. The static_assert above keeps the combined table within what `compute_truth_table`
            // handles.
            std::vector<std::string> variable_names;
            for (const auto* n : state_inputs)
            {
                variable_names.push_back(BooleanFunctionNetDecorator(*n).get_boolean_variable_name());
            }
            for (const auto* n : control_inputs)
            {
                variable_names.push_back(BooleanFunctionNetDecorator(*n).get_boolean_variable_name());
            }

            std::vector<std::vector<BooleanFunction::Value>> tables;
            tables.reserve(bfs.size());
            for (const auto& bf : bfs)
            {
                auto tt_res = bf.compute_truth_table(variable_names);
                if (tt_res.is_error())
                {
                    return ERR(tt_res.get_error());
                }
                tables.push_back(std::move(tt_res.get().front()));
            }

            // The round function computes the S-box for one assignment of the control inputs and something else for
            // the others, and which one that is is not known in advance, so try them all.
            const u32 num_state_rows = 1 << state_inputs.size();
            for (u32 assignment = 0; assignment < (1u << control_inputs.size()); assignment++)
            {
                std::vector<std::vector<BooleanFunction::Value>> rows(num_state_rows, std::vector<BooleanFunction::Value>(tables.size()));
                for (u32 out = 0; out < tables.size(); out++)
                {
                    const auto& table = tables.at(out);
                    for (u32 row = 0; row < num_state_rows; row++)
                    {
                        rows[row][out] = table.at((u64(assignment) << state_inputs.size()) + row);
                    }
                }

                auto name_res = lookup_sbox(rows, state_inputs.size(), db);
                if (name_res.is_error())
                {
                    return ERR(name_res.get_error());
                }
                if (!name_res.get().empty())
                {
                    return OK(name_res.get());
                }
            }

            return OK(std::string());
        }

        Result<std::string> CipherCandidate::identify_sbox(const std::vector<BooleanFunction>& output_functions, const SBoxDatabase& db)
        {
            if (output_functions.empty())
            {
                return ERR("no output functions provided");
            }

            std::set<std::string> variables;
            for (const auto& bf : output_functions)
            {
                const auto bf_variables = bf.get_variable_names();
                variables.insert(bf_variables.begin(), bf_variables.end());
            }

            if (variables.empty() || variables.size() > MAX_SBOX_BITS)
            {
                return ERR("the output functions read " + std::to_string(variables.size()) + " variables, but an S-box reads between 1 and " + std::to_string(MAX_SBOX_BITS));
            }

            const std::vector<std::string> variable_names(variables.begin(), variables.end());

            std::vector<std::vector<BooleanFunction::Value>> tables;
            tables.reserve(output_functions.size());
            for (const auto& bf : output_functions)
            {
                auto tt_res = bf.compute_truth_table(variable_names);
                if (tt_res.is_error())
                {
                    return ERR(tt_res.get_error());
                }
                tables.push_back(std::move(tt_res.get().front()));
            }

            std::vector<std::vector<BooleanFunction::Value>> rows(1 << variable_names.size(), std::vector<BooleanFunction::Value>(tables.size()));
            for (u32 out = 0; out < tables.size(); out++)
            {
                for (u32 row = 0; row < rows.size(); row++)
                {
                    rows[row][out] = tables.at(out).at(row);
                }
            }

            return lookup_sbox(rows, variable_names.size(), db);
        }

        Result<Module*> CipherCandidate::create_modules()
        {
            if (m_netlist == nullptr)
            {
                return ERR("candidate is empty");
            }

            auto* candidate_module = m_netlist->create_module("cipher_candidate", m_netlist->get_top_module(), get_gates());
            if (candidate_module == nullptr)
            {
                return ERR("could not create a module for the candidate");
            }

            if (m_is_round_based)
            {
                if (m_netlist->create_module("state_register", candidate_module, m_in_reg) == nullptr)
                {
                    return ERR("could not create a module for the state register of the candidate");
                }
            }
            else
            {
                if (m_netlist->create_module("input_register", candidate_module, m_in_reg) == nullptr)
                {
                    return ERR("could not create a module for the input register of the candidate");
                }
                if (m_netlist->create_module("output_register", candidate_module, m_out_reg) == nullptr)
                {
                    return ERR("could not create a module for the output register of the candidate");
                }
            }

            // A gate belongs to exactly one module, so two S-boxes sharing a gate cannot both become one. That the
            // S-boxes located overlap is expected, as the search guesses at their outputs, but by the time they have
            // been identified an overlap means that two guesses were both taken for real.
            std::unordered_set<Gate*> already_in_a_module;
            u32 num_sboxes = 0;
            for (const auto& sbox : m_sboxes)
            {
                if (sbox->status != SBoxStatus::identified)
                {
                    continue;
                }

                const auto sbox_gates = sbox->get_combinational_gates();
                if (std::any_of(sbox_gates.begin(), sbox_gates.end(), [&already_in_a_module](Gate* g) { return already_in_a_module.count(g); }))
                {
                    log_info("hawkeye", "skipping S-box '{}' reading flip-flop {}, as it overlaps an S-box that was already turned into a module.", sbox->identified_as, sbox->input_gates.front()->get_id());
                    continue;
                }

                if (m_netlist->create_module(sbox->identified_as + "_" + std::to_string(num_sboxes), candidate_module, sbox_gates) == nullptr)
                {
                    return ERR("could not create a module for S-box '" + sbox->identified_as + "' of the candidate");
                }

                already_in_a_module.insert(sbox_gates.begin(), sbox_gates.end());
                num_sboxes++;
            }

            log_info("hawkeye", "created a module for the candidate holding {} S-box modules.", num_sboxes);
            return OK(candidate_module);
        }
    }    // namespace hawkeye
}    // namespace hal
