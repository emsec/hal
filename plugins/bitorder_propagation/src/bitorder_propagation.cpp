#include "bitorder_propagation/bitorder_propagation.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/netlist/pins/module_pin.h"

#include <deque>

// #define PRINT_CONFLICT
// #define PRINT_CONNECTIVITY
// #define PRINT_CONNECTIVITY_BUILDING
// #define PRINT_GENERAL

namespace hal
{
    namespace bitorder_propagation
    {
        namespace
        {
            typedef std::pair<Module*, PinGroup<ModulePin>*> MPG;
            typedef std::map<MPG, std::set<u32>> POSSIBLE_BITINDICES;

            /**
             * Build an offset matrix that maps each pair of module and pin group to all the other pairs of module and pin group that overlap by providing an index for the same net.
             * Since that index may be different, we calculate an offset and check whether that offset is the same for all nets where the two origins overlap.
             * The matrix is populated in a way that the offset at `matrix[org_0][org_1]` allows the user to calculate the `index_1 = index_0 + offset`.
             * 
             * @param[in] reduced_indices - The (already reduced) bit indices.
             * @returns The offset matrix.
             */
            Result<std::map<MPG, std::map<MPG, i32>>> build_offset_matrix(const std::map<Net*, POSSIBLE_BITINDICES>& reduced_indices)
            {
                // offset at matrix[org_0][org_1] means index_0 + offset = index_1
                std::map<MPG, std::map<MPG, i32>> origin_offset_matrix;

                for (const auto& [net, possible_bitindices] : reduced_indices)
                {
                    std::map<MPG, u32> all_possible_indices;

                    // fill all possible indices
                    for (const auto& [org_mpg, indices] : possible_bitindices)
                    {
                        all_possible_indices[org_mpg] = *(indices.begin());
                    }

                    // check whether all possible indices are just shifted version of each other with a stable offset
                    for (const auto& [org_mpg, indices] : possible_bitindices)
                    {
                        for (const auto& [already_set_org, already_set_index] : all_possible_indices)
                        {
                            // there does not yet exist an offset between the already set index and the one to be added next
                            if (origin_offset_matrix[org_mpg].find(already_set_org) == origin_offset_matrix[org_mpg].end())
                            {
                                i32 new_index = *indices.begin();
                                i32 offset    = already_set_index - new_index;

                                origin_offset_matrix[org_mpg][already_set_org] = offset;
                                origin_offset_matrix[already_set_org][org_mpg] = -offset;
                            }
                            // check wether the already existing offset leads to the same index
                            else
                            {
                                i32 new_index = *indices.begin();
                                i32 offset    = origin_offset_matrix.at(org_mpg).at(already_set_org);

                                if (new_index + offset != i32(already_set_index))
                                {
                                    return ERR("unable to build offset matrix: failed to find valid offset between " + std::to_string(org_mpg.first->get_id()) + "-" + org_mpg.second->get_name()
                                               + " and " + std::to_string(already_set_org.first->get_id()) + "-" + already_set_org.second->get_name());
                                }
                            }
                        }
                    }
                }

                return OK(origin_offset_matrix);
            }

            /**
             * Attempt to find an offset between two origins (pairs of module and pin group) with the help of a previously generated offset matrix.
             * That matrix stores every known offset between two origins.
             * By building a chain of known origin-offset pairs, we try to find offsets even for origins that do not share an already known offset.
             * During the chain building we populate the matrix along the way whenever we find a valid offset.
             * 
             * @param[in] org1 - First origin as a pair of module and pin group.
             * @param[in] org2 - Second origin as a pair of module and pin group.
             * @param[in] m - The offset matrix.
             * @param[in] v - Set of already visited matrix entries.
             * @returns OK and the index offset between `org1` and `org2` on success, an error otherwise.
             */
            Result<i32> get_offset(const MPG& org1, const MPG& org2, std::map<MPG, std::map<MPG, i32>>& m, std::set<std::set<MPG>>& v)
            {
                if (v.find({org1, org2}) != v.end())
                {
                    return ERR("already tried to follow that offset.");
                }

                v.insert({org1, org2});

                if (org1 == org2)
                {
                    m[org1][org2] = 0;
                    return OK(0);
                }

                if (m.find(org1) == m.end())
                {
                    return ERR("no valid offset to other origins.");
                }

                if (m.at(org1).find(org2) != m.at(org1).end())
                {
                    return OK(m.at(org1).at(org2));
                }

                for (auto& [dst_c, first_proxy_offset] : m.at(org1))
                {
                    auto second_proxy_offset_res = get_offset(dst_c, org2, m, v);
                    if (second_proxy_offset_res.is_error())
                    {
                        continue;
                    }
                    i32 second_proxy_offset = second_proxy_offset_res.get();

                    m[org1][org2] = first_proxy_offset + second_proxy_offset;

                    return OK(first_proxy_offset + second_proxy_offset);
                }

                return ERR("could not find an valid offset");
            }

            /**
             * This function gathers neighboring pingroups for a net by propagating to the neighboring gates and searches for module pin groups.
             * 
             * @param[in] n - TODO
             * @param[in] successors - TODO
             * @param[in] relevant_pin_groups - TODO
             * @param[in] guarantee_propagation - TODO
             * @param[in] inwards_module - TODO
             * @param[in] visited - TODO
             * @param[in] cache - TODO
             * @returns TODO
             */
            Result<std::map<MPG, std::set<Net*>>> gather_connected_neighbors(Net* n,
                                                                             bool successors,
                                                                             const std::set<MPG>& relevant_pin_groups,
                                                                             const bool guarantee_propagation,
                                                                             const Module* inwards_module,
                                                                             std::set<std::tuple<Endpoint*, const bool, const Module*>>& visited,
                                                                             std::map<std::tuple<Endpoint*, const bool, const Module*>, std::map<MPG, std::set<Net*>>>& cache)
            {
                std::map<MPG, std::set<Net*>> connected_neighbors;

#ifdef PRINT_CONNECTIVITY_BUILDING
                std::cout << "Gathering bit index for net " << n->get_id() << " with" << (guarantee_propagation ? "" : "out") << " guaranteed propagation "
                          << " in direction: " << (successors ? "forwards" : "backwards") << std::endl;
#endif

                // check whether the net is a global input or global output net (has no sources or destinations, but might have a bitorder annotated at the top module)
                if ((successors && n->is_global_output_net()) || (!successors && n->is_global_input_net()))
                {
                    auto m             = n->get_netlist()->get_top_module();
                    bool is_border_pin = successors ? m->is_input_net(n) : m->is_output_net(n);
                    if (is_border_pin)
                    {
                        auto border_pin = m->get_pin_by_net(n);
                        if (border_pin == nullptr)
                        {
                            return ERR("cannot get bit index information for net with ID " + std::to_string(n->get_id()) + " from module with ID " + std::to_string(m->get_id())
                                       + ": net is border net but does not have a pin.");
                        }
                        auto pg = border_pin->get_group().first;

#ifdef PRINT_CONNECTIVITY_BUILDING
                        std::cout << "Added global IO net as origin " << m->get_name() << " - " << pg->get_name() << " - " << n->get_id() << std::endl;
#endif

                        connected_neighbors[{m, pg}].insert(n);
                    }
                }

                const auto neighbors = successors ? n->get_destinations() : n->get_sources();
                for (const auto& ep : neighbors)
                {
                    std::tuple<Endpoint*, const bool, const Module*> t_ep = {ep, guarantee_propagation, inwards_module};
                    if (visited.find(t_ep) != visited.end())
                    {
                        continue;
                    }
                    visited.insert(t_ep);

                    Gate* g = ep->get_gate();

                    if (g == nullptr)
                    {
                        continue;
                    }

#ifdef PRINT_CONNECTIVITY_BUILDING
                    std::cout << "Checking gate " << g->get_id() << std::endl;
#endif

                    if ((inwards_module != nullptr) && !inwards_module->contains_gate(g, true))
                    {
                        continue;
                    }

                    const auto modules = g->get_modules();

                    if (!guarantee_propagation)
                    {
                        // check whether the net that leads to the gate is part of a relevant pin_group
                        bool found_relevant_pin_group = false;
                        for (const auto& m : modules)
                        {
                            bool is_border_pin = successors ? m->is_input_net(n) : m->is_output_net(n);
                            if (is_border_pin)
                            {
                                auto border_pin = m->get_pin_by_net(n);
                                if (border_pin == nullptr)
                                {
                                    return ERR("cannot get bit index information for net with ID " + std::to_string(n->get_id()) + " from module with ID " + std::to_string(m->get_id())
                                               + ": net is border net but does not have a pin.");
                                }
                                auto border_pg = border_pin->get_group().first;

                                // only consider relevant pin groups that already have a known bitorder or that are currently unknown but might get one
                                if (relevant_pin_groups.find({m, border_pg}) == relevant_pin_groups.end())
                                {
                                    continue;
                                }

                                connected_neighbors[{m, border_pg}].insert(n);
                                found_relevant_pin_group = true;
                            }
                        }

                        // stop the propagation at the gate when we reached it via at least one relevant pin group
                        if (found_relevant_pin_group)
                        {
                            continue;
                        }
                    }

                    // propagate
                    std::vector<Endpoint*> next_eps;

                    for (const auto& next_ep : successors ? g->get_fan_out_endpoints() : g->get_fan_in_endpoints())
                    {
                        const GatePin* pin = next_ep->get_pin();
                        if (g->get_type()->has_property(GateTypeProperty::sequential) && (g->get_type()->has_property(GateTypeProperty::ff) || g->get_type()->has_property(GateTypeProperty::latch)))
                        {
                            if (PinType t = pin->get_type(); (t == PinType::data) || (t == PinType::state) || (t == PinType::neg_state))
                            {
                                next_eps.push_back(next_ep);
                            }
                        }
                        else
                        {
                            next_eps.push_back(next_ep);
                        }
                    }

                    for (Endpoint* next_ep : next_eps)
                    {
                        // Check whether we leave the gate via a relevant pin group, if that is the case stop
                        bool found_relevant_pin_group = false;
                        for (const auto& m : modules)
                        {
                            bool is_border_pin = successors ? m->is_output_net(next_ep->get_net()) : m->is_input_net(next_ep->get_net());
                            if (is_border_pin)
                            {
                                auto border_pin = m->get_pin_by_net(next_ep->get_net());
                                if (border_pin == nullptr)
                                {
                                    return ERR("cannot get bit index information for net with ID " + std::to_string(next_ep->get_net()->get_id()) + " from module with ID "
                                               + std::to_string(m->get_id()) + ": net is border net but does not have a pin.");
                                }
                                auto border_pg = border_pin->get_group().first;

                                // only consider relevant pin groups that already have a known bitorder or that are currently unknown but might get one
                                if (relevant_pin_groups.find({m, border_pg}) == relevant_pin_groups.end())
                                {
                                    continue;
                                }

                                connected_neighbors[{m, border_pg}].insert(next_ep->get_net());
                                found_relevant_pin_group = true;
                            }
                        }

                        // stop the propagation at the gate when we would leave it via at least one relevant pin group
                        if (found_relevant_pin_group)
                        {
                            continue;
                        }

                        std::map<MPG, std::set<hal::Net*>> connected;
                        std::tuple<Endpoint*, const bool, const Module*> t = {next_ep, false, nullptr};
                        if (auto it = cache.find(t); it != cache.end())
                        {
                            connected = it->second;
                        }
                        else
                        {
                            auto res = gather_connected_neighbors(next_ep->get_net(), successors, relevant_pin_groups, false, nullptr, visited, cache);
                            if (res.is_error())
                            {
                                return res;
                            }
                            connected = res.get();
                        }

                        cache[t] = connected;

                        for (auto& [org_mpg, nets] : connected)
                        {
                            connected_neighbors[org_mpg].insert(nets.begin(), nets.end());
                        }
                    }
                }

                return OK(connected_neighbors);
            }

            /**
             * Reduce a collection of bit indices by deleting invalid indices.
             * Indices are considered invalid if:
             *  - a pair of module and pin group annotates different indices for the same pin
             *  - a pair of module and pin group annotates the same index for different pins
             * 
             * @param[in] collected_bitindices - All bit indices collected after propagation.
             * @returns A reduced set of possible bit indices for every net.
             */
            const std::map<Net*, POSSIBLE_BITINDICES> reduce_indices(const std::map<Net*, POSSIBLE_BITINDICES>& collected_bitindices)
            {
#ifdef PRINT_CONFLICT
                std::cout << "\tVanilla indices: " << std::endl;
                for (const auto& [net, possible_bitindices] : collected_bitindices)
                {
                    std::cout << "\t\tNet " << net->get_id() << " - " << net->get_name() << ": " << std::endl;
                    u32 origins = 0;
                    for (const auto& [org_mpg, indices] : possible_bitindices)
                    {
                        auto org_m  = org_mpg.first;
                        auto org_pg = org_mpg.second;

                        std::cout << "\t\t\t" << org_m->get_id() << "-" << org_pg->get_name() << ": [";
                        for (const auto& index : indices)
                        {
                            std::cout << index << ", ";
                        }
                        std::cout << "]" << std::endl;
                        origins += 1;
                    }

                    std::cout << "\t\tORIGINS: [" << origins << "]" << std::endl;
                }
#endif

                auto reduced_collected_indices = collected_bitindices;

                // 1) Checks whether the mpg has annotated the same index to different nets
                std::set<std::pair<MPG, u32>> origin_indices;
                std::set<std::pair<MPG, u32>> origin_indices_to_remove;

                for (const auto& [net, possible_bitindices] : reduced_collected_indices)
                {
                    for (const auto& [org_mpg, indices] : possible_bitindices)
                    {
                        for (const auto& index : indices)
                        {
                            if (origin_indices.find({org_mpg, index}) != origin_indices.end())
                            {
                                origin_indices_to_remove.insert({org_mpg, index});
                            }
                            else
                            {
                                origin_indices.insert({org_mpg, index});
                            }
                        }
                    }
                }

#ifdef PRINT_CONFLICT
                for (const auto& [org_mpg, index] : origin_indices_to_remove)
                {
                    std::cout << "Found org " << org_mpg.first->get_id() << "-" << org_mpg.second->get_name() << " index " << index << " pair to remove!" << std::endl;
                }
#endif

                for (auto& [net, possible_bitindices] : collected_bitindices)
                {
                    for (auto& [org_mpg, indices] : possible_bitindices)
                    {
                        for (const auto& index : indices)
                        {
                            if (origin_indices_to_remove.find({org_mpg, index}) != origin_indices_to_remove.end())
                            {
                                reduced_collected_indices.at(net).at(org_mpg).erase(index);
                            }
                        }

                        if (reduced_collected_indices.at(net).at(org_mpg).empty())
                        {
                            reduced_collected_indices.at(net).erase(org_mpg);
                        }
                    }

                    if (reduced_collected_indices.at(net).empty())
                    {
                        reduced_collected_indices.erase(net);
                    }
                }

                if (reduced_collected_indices.empty())
                {
                    return {};
                }

                // 2)  Checks whether a net has multiple indices annotated from the same origin mpg
                auto further_reduced_collected_indices = reduced_collected_indices;
                for (auto& [net, possible_bitindices] : reduced_collected_indices)
                {
                    for (auto& [org_mpg, indices] : possible_bitindices)
                    {
                        if (indices.size() != 1)
                        {
                            further_reduced_collected_indices.at(net).erase(org_mpg);
                        }
                    }

                    if (further_reduced_collected_indices.at(net).empty())
                    {
                        further_reduced_collected_indices.erase(net);
                    }
                }

                if (further_reduced_collected_indices.empty())
                {
                    return {};
                }

#ifdef PRINT_CONFLICT
                std::cout << "\tReduced Possible Indices: " << std::endl;
                for (const auto& [net, possible_bitindices] : further_reduced_collected_indices)
                {
                    std::cout << "\t\tNet " << net->get_id() << ": " << std::endl;
                    u32 origins = 0;
                    for (const auto& [org_mpg, indices] : possible_bitindices)
                    {
                        auto org_m  = org_mpg.first;
                        auto org_pg = org_mpg.second;

                        std::cout << "\t\t\t" << org_m->get_id() << "-" << org_pg->get_name() << ": [";
                        for (const auto& index : indices)
                        {
                            std::cout << index << ", ";
                        }
                        std::cout << "]" << std::endl;
                    }
                }
#endif

                return further_reduced_collected_indices;
            }

            /**
             * Check whether every net of a module pin group has been assigned a valid index.
             * 
             * @param[in] mpg - The module pin group top check.
             * @param[in] consensus_bitindices - The bit indices after attempting consensus finding.
             * @returns `true` if the module pin group is complete, `false` otherwise.
             */
            const bool check_completeness(const MPG& mpg, const std::map<Net*, i32>& consensus_bitindices)
            {
                bool is_complete_pin_group_bitorder = true;

                for (auto& pin : mpg.second->get_pins())
                {
                    Net* net = pin->get_net();
                    if (consensus_bitindices.find(net) == consensus_bitindices.end())
                    {
                        is_complete_pin_group_bitorder = false;

#ifdef PRINT_CONFLICT
                        std::cout << "Missing net " << net->get_id() << " - " << net->get_name() << " for complete bitorder." << std::endl;
#endif
                        break;
                    }
                }

#ifdef PRINT_CONFLICT
                if (is_complete_pin_group_bitorder)
                {
                    std::cout << "Found complete bitorder for pingroup " << mpg.second->get_name() << std::endl;
                    for (const auto& [net, index] : consensus_bitindices)
                    {
                        std::cout << net->get_id() << ": " << index << std::endl;
                    }
                }
#endif

                return is_complete_pin_group_bitorder;
            }

            /**
             * Align the given bit indices from range `[m:m+n]` to `[0:n]`.
             * If the `enforce_continuous_bitorders` is `true`, also check whether the indices actually form a consecutive range.
             * A checks whether all indices are unique, otherwise we can not determine an order.
             * 
             * @param[in] consensus_bitindices - The bit indices after consensus finding.
             * @param[in] enforce_continuous_bitorders - Set `true` to only allow consecutive bit orders, `false` otherwise. Defaults to `true`.
             * @returns The aligned bit indices or an empty map if any condition has been violated.
             */
            const std::map<Net*, u32> align_indices(const std::map<Net*, i32>& consensus_bitindices, const bool enforce_continuous_bitorders)
            {
                std::map<Net*, u32> aligned_consensus;

                std::set<i32> unique_indices;
                for (const auto& [_n, index] : consensus_bitindices)
                {
                    unique_indices.insert(index);
                }

                if (unique_indices.empty())
                {
                    return {};
                }

                const i32 min_index = *(unique_indices.begin());
                const i32 max_index = *(unique_indices.rbegin());

                // when the range is larger than pin group size there are holes in the bitorder
                if (enforce_continuous_bitorders && ((max_index - min_index) > (i32(consensus_bitindices.size()) - 1)))
                {
                    return {};
                }

                // when there are less unique indices in the range than nets, there are duplicates
                if (unique_indices.size() < consensus_bitindices.size())
                {
                    return {};
                }

                std::map<i32, Net*> index_to_net;
                for (const auto& [net, index] : consensus_bitindices)
                {
                    index_to_net[index] = net;
                }

                u32 index_counter = 0;
                for (const auto& [_unaligned_index, net] : index_to_net)
                {
                    aligned_consensus[net] = index_counter++;
                }

                return aligned_consensus;
            }

            /**
             * Attempt consensus finding by searching for a pair-wise offset between all pairs of modules and pin groups.
             * This offset must hold for all nets that have been assigned indices from the same origins.
             * The offset propagates, so when A and B share an offset p and B and C share an offset q then the offset between A and C should be p + q.
             * 
             * @param[in] mpg - The pin group for which to attempt consensus finding.
             * @param[in] indices - The (already reduced) bit indices on which to search for a pair-wise offset.
             * @param[in] enforce_continuous_bitorders - Set `true` to only allow consecutive bit orders, `false` otherwise. Defaults to `true`.
             * @returns A map from nets to their index for the nets for which there is a common pair-wise offset.
             */
            std::map<Net*, u32> find_consensus_via_offset(const MPG& mpg, const std::map<hal::Net*, POSSIBLE_BITINDICES>& indices, const bool enforce_continuous_bitorders)
            {
                std::map<Net*, i32> consensus_bitindices;

                auto offset_matrix_res = build_offset_matrix(indices);
                if (offset_matrix_res.is_error())
                {
#ifdef PRINT_CONFLICT
                    std::cout << "Failed to build offset matrix : " << offset_matrix_res.get_error().get() << std::endl;
#endif
                    return {};
                }
                auto offset_matrix = offset_matrix_res.get();

                // select a pseudo random base line and gather the offsets between the base line and all other possible module/pin group origins
                auto base_line = offset_matrix.begin()->first;

#ifdef PRINT_CONFLICT
                std::cout << "Found valid offsets pingroup " << mpg.second->get_name() << ": " << std::endl;
                std::cout << "Baseline: " << base_line.first->get_id() << "-" << base_line.second->get_name() << std::endl;
                for (const auto& [org1, col] : offset_matrix)
                {
                    std::cout << org1.first->get_id() << "-" << org1.second->get_name() << ": ";
                    for (const auto& [org2, offset] : col)
                    {
                        std::cout << org2.first->get_id() << "-" << org2.second->get_name() << "[" << offset << "] ";
                    }
                    std::cout << std::endl;
                }
#endif

                for (const auto& [net, possible_bitindices] : indices)
                {
                    // pair of first possible org_mod and org_pin_group
                    MPG org = possible_bitindices.begin()->first;
                    // index at first possible origin
                    i32 org_index = *(possible_bitindices.begin()->second.begin());
                    std::set<std::set<MPG>> v;
                    auto offset_res = get_offset(org, base_line, offset_matrix, v);
                    if (offset_res.is_error())
                    {
                        if (possible_bitindices.size() == 1)
                        {
                            // if there cannot be found any valid offset to the baseline, but there is just one possible index annotated, we still allow it
                            // -> this wont break anything, since this only allows for bitorders that we otherwise would have discarded because of a missing net
                            consensus_bitindices[net] = org_index;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        i32 offset                = offset_res.get();
                        consensus_bitindices[net] = org_index + offset;
                    }
                }

#ifdef PRINT_CONFLICT
                std::cout << "Found offset bitorder: " << std::endl;
                for (const auto& [net, index] : consensus_bitindices)
                {
                    std::cout << net->get_id() << ": " << index << std::endl;
                }
#endif

                // check completeness, i.e., whether each pin of the pin group was annotated an index
                const auto is_complete_pin_group_bitorder = check_completeness(mpg, consensus_bitindices);

                if (!is_complete_pin_group_bitorder)
                {
                    return {};
                }

                // check if consecutive and shift so that indices start at 0
                const auto aligned_indices = align_indices(consensus_bitindices, enforce_continuous_bitorders);

                return aligned_indices;
            }

            /**
             * Find the most common bit index from a set of possible bit indices for all nets.
             * 
             * @param[in] indices - The bit indices on which to perform the majority vote.
             * @returns A map from nets to the most common index of the respective net.
             */
            const std::map<Net*, i32> conduct_majority_vote(const std::map<hal::Net*, POSSIBLE_BITINDICES>& indices)
            {
                std::map<Net*, i32> majority_indices;

                for (const auto& [net, possible_indices] : indices)
                {
                    std::map<u32, u32> index_to_count;
                    for (const auto& [_org, org_indices] : possible_indices)
                    {
                        for (const auto& index : org_indices)
                        {
                            index_to_count[index]++;
                        }
                    }

                    // if there is only one index use this one
                    if (index_to_count.size() == 1)
                    {
                        majority_indices.insert({net, index_to_count.begin()->first});
                        continue;
                    }

                    // sort possible indices by how often they occur and afterwards check whether there is a clear majority
                    std::vector<std::pair<u32, u32>> index_counts = {index_to_count.begin(), index_to_count.end()};
                    std::sort(index_counts.begin(), index_counts.end(), [](const auto& p1, const auto& p2) { return p1.second > p2.second; });

                    // check if unambiguous majority exists
                    if (index_counts.at(0).second > index_counts.at(1).second)
                    {
                        majority_indices.insert({net, index_counts.at(0).first});
                    }
                }

                return majority_indices;
            }

            /**
             * Attempt consensus finding by conducting a majority vote on the possible indices of each net of the target pin group.
             * 
             * @param[in] mpg - The pin group for which to attempt consensus finding.
             * @param[in] indices - The (already reduced) bit indices on which to perform the majority vote.
             * @param[in] enforce_continuous_bitorders - Set `true` to only allow consecutive bit orders, `false` otherwise. Defaults to `true`.
             * @returns A map from nets to the most common index of the respective net.
             */
            std::map<Net*, u32> find_consensus_via_majority(const MPG& mpg, const std::map<hal::Net*, POSSIBLE_BITINDICES>& indices, const bool enforce_continuous_bitorders)
            {
                const auto majority_indices = conduct_majority_vote(indices);

#ifdef PRINT_CONFLICT
                std::cout << "Found majority bitorder: " << std::endl;
                for (const auto& [net, index] : majority_indices)
                {
                    std::cout << net->get_id() << ": " << index << std::endl;
                }
#endif

                // check completeness, i.e., whether each pin of the pin group was annotated an index
                const auto is_complete_pin_group_bitorder = check_completeness(mpg, majority_indices);
                if (!is_complete_pin_group_bitorder)
                {
                    return {};
                }

                // check if consecutive and shift so that indices start at 0
                const auto aligned_indices = align_indices(majority_indices, enforce_continuous_bitorders);

                return aligned_indices;
            }

            /**
             * Attempt consensus finding by applying two majority vote iterations.
             * First, a simple majority vote is performed on the already reduced bit indices.
             * In the second iteration, all nets that already had an index annotated in the first iteration are disregarded from the set of ALL (unreduced) bit indices.
             * Afterwards, the smaller set is then reduced again and a second majority vote is performed.
             * The general idea is that conflicts that were previously present when considering all nets might disappear when disregarding nets that have already been annotated with an index.
             * 
             * @param[in] mpg - The pin group for which to attempt consensus finding.
             * @param[in] all_indices - All possible bit indices, i.e., the set of possible bit indices that have not yet been reduced.
             * @param[in] reduced_indices - The (already reduced) bit indices on which to perform the majority vote.
             * @param[in] enforce_continuous_bitorders - Set `true` to only allow consecutive bit orders, `false` otherwise. Defaults to `true`.
             * @returns A map from nets to the most common index of the respective net after applying the relaxed majority voting.
             */
            std::map<Net*, u32> find_consensus_via_majority_relaxed(const MPG& mpg,
                                                                    const std::map<hal::Net*, POSSIBLE_BITINDICES>& all_indices,
                                                                    const std::map<hal::Net*, POSSIBLE_BITINDICES>& reduced_indices,
                                                                    const bool enforce_continuous_bitorders)
            {
                // 1st iteration
                const auto first_majority_indices = conduct_majority_vote(reduced_indices);

                // take ALL collected net indices and delete the ones already annotated in the first iteration
                auto unfound_indices = all_indices;
                for (const auto& [net, _] : first_majority_indices)
                {
                    unfound_indices.erase(net);
                }

                // reduce indices again, but this time only consider nets that do not yet have an index found via majority
                auto relaxed_reduced_indices = reduce_indices(unfound_indices);

                // 2nd iteration
                const auto second_majority_indices = conduct_majority_vote(relaxed_reduced_indices);

#ifdef PRINT_CONFLICT
                std::cout << "Found majority bitorder: " << std::endl;
                for (const auto& [net, index] : second_majority_indices)
                {
                    std::cout << net->get_id() << ": " << index << std::endl;
                }
#endif

                std::map<Net*, i32> combined_indices = first_majority_indices;
                for (const auto& p : second_majority_indices)
                {
                    combined_indices.insert(p);
                }

                // check completeness, i.e., whether each pin of the pin group was annotated an index
                const auto is_complete_pin_group_bitorder = check_completeness(mpg, combined_indices);
                if (!is_complete_pin_group_bitorder)
                {
                    return {};
                }

                // check if consecutive and shift so that indices start at 0
                const auto aligned_indices = align_indices(combined_indices, enforce_continuous_bitorders);

                return aligned_indices;
            }

            /**
             * Try to extract valid bit orders from the bit-index information that was gathered during the propagation step.
             * The bit indices are given as a map from the net they are associated with to all possible bit indices, which are given as a map from the source module pin group to a set of the indices extracted from that pin group.
             * First, conflicting information is deleted.
             * Next, different strategies for consensus finding are applied. 
             * The resulting bit-order consensus is continuously validated in terms of its continuity and completeness.
             * The validation strictness can be tweaked with the parameter `enforce_continuous_bitorders`.
             * 
             * @param[in] mpg - The pin group for which to attempt consensus finding.
             * @param[in] collected_bitindices - The bit indices collected during propagation.
             * @param[in] enforce_continuous_bitorders - Set `true` to only allow consecutive bit orders, `false` otherwise. Defaults to `true`.
             * @returns The resulting bit orders as a map from nets to their indices.
             */
            std::map<Net*, u32> extract_well_formed_bitorder(const MPG& mpg, const std::map<Net*, POSSIBLE_BITINDICES>& collected_bitindices, bool enforce_continuous_bitorders = true)
            {
                auto reduced_collected_indices = reduce_indices(collected_bitindices);

                if (reduced_collected_indices.empty())
                {
                    return {};
                }

                auto aligned_consensus = find_consensus_via_offset(mpg, reduced_collected_indices, enforce_continuous_bitorders);

                if (aligned_consensus.empty())
                {
                    aligned_consensus = find_consensus_via_majority(mpg, reduced_collected_indices, enforce_continuous_bitorders);
                }

                if (aligned_consensus.empty())
                {
                    aligned_consensus = find_consensus_via_majority_relaxed(mpg, collected_bitindices, reduced_collected_indices, enforce_continuous_bitorders);
                }

                if (aligned_consensus.empty())
                {
                    return {};
                }

#ifdef PRINT_CONFLICT
                std::cout << "Found valid input bitorder for pingroup " << mpg.second->get_name() << std::endl;
                for (const auto& [net, index] : aligned_consensus)
                {
                    std::cout << net->get_id() << ": " << index << std::endl;
                }
#endif

                return aligned_consensus;
            }

        }    // namespace

        Result<std::map<MPG, std::map<Net*, u32>>>
            propagate_module_pingroup_bitorder(const std::map<MPG, std::map<Net*, u32>>& known_bitorders, const std::set<MPG>& unknown_bitorders, const bool enforce_continuous_bitorders)
        {
            // std::unordered_map<std::pair<MPG, Net*>, std::vector<std::pair<MPG, std::set<Net*>>>, boost::hash<std::pair<MPG, std::set<Net*>>>> connectivity_inwards;
            // std::unordered_map<std::pair<MPG, Net*>, std::vector<std::pair<MPG, std::set<Net*>>>, boost::hash<std::pair<MPG, std::set<Net*>>>> connectivity_outwards;

            std::map<std::pair<MPG, Net*>, std::vector<std::pair<MPG, std::set<Net*>>>> connectivity_inwards;
            std::map<std::pair<MPG, Net*>, std::vector<std::pair<MPG, std::set<Net*>>>> connectivity_outwards;

#ifdef PRINT_GENERAL
            std::cout << "Known bitorders [" << known_bitorders.size() << "]:" << std::endl;
            for (const auto& [mpg, _] : known_bitorders)
            {
                std::cout << "\t" << mpg.first->get_name() << " - " << mpg.second->get_name() << std::endl;
            }

            std::cout << "Unknown bitorders [" << known_bitorders.size() << "]:" << std::endl;
            for (const auto& [m, pg] : unknown_bitorders)
            {
                std::cout << "\t" << m->get_name() << " - " << pg->get_name() << std::endl;
            }

#endif

            std::set<MPG> relevant_pin_groups = unknown_bitorders;
            for (const auto& [kb, _] : known_bitorders)
            {
                relevant_pin_groups.insert(kb);
            }

            std::map<std::tuple<Endpoint*, const bool, const Module*>, std::map<MPG, std::set<Net*>>> cache_outwards;
            std::map<std::tuple<Endpoint*, const bool, const Module*>, std::map<MPG, std::set<Net*>>> cache_inwards;

            // Build connectivity
            for (const auto& [m, pg] : unknown_bitorders)
            {
                bool successors = pg->get_direction() == PinDirection::output;

                for (const auto& p : pg->get_pins())
                {
                    const auto starting_net = p->get_net();

                    std::set<std::tuple<Endpoint*, const bool, const Module*>> visited_outwards;
                    const auto res_outwards = gather_connected_neighbors(starting_net, successors, relevant_pin_groups, false, nullptr, visited_outwards, cache_outwards);
                    if (res_outwards.is_error())
                    {
                        return ERR_APPEND(res_outwards.get_error(),
                                          "cannot porpagate bitorder: failed to gather bit indices outwards starting from the module with ID " + std::to_string(m->get_id()) + " and pin group "
                                              + pg->get_name());
                    }
                    const auto connected_outwards = res_outwards.get();

                    std::set<std::tuple<Endpoint*, const bool, const Module*>> visited_inwards;
                    // NOTE when propagating inwards we guarantee the first propagation since otherwise we would stop at our starting pingroup
                    const auto res_inwards = gather_connected_neighbors(starting_net, !successors, relevant_pin_groups, true, m, visited_inwards, cache_inwards);
                    if (res_inwards.is_error())
                    {
                        return ERR_APPEND(res_inwards.get_error(),
                                          "cannot porpagate bitorder: failed to gather bit indices inwwards starting from the module with ID " + std::to_string(m->get_id()) + " and pin group "
                                              + pg->get_name());
                    }
                    const auto connected_inwards = res_inwards.get();

                    for (const auto& [org_mpg, nets] : connected_outwards)
                    {
                        connectivity_outwards[{{m, pg}, starting_net}].push_back({org_mpg, nets});
                    }

                    for (const auto& [org_mpg, nets] : connected_inwards)
                    {
                        connectivity_inwards[{{m, pg}, starting_net}].push_back({org_mpg, nets});
                    }
                }
            }

#ifdef PRINT_CONNECTIVITY
            for (const auto& [start, connected] : connectivity_outwards)
            {
                std::cout << start.first.first->get_id() << " / " << start.first.first->get_name() << " - " << start.first.second->get_name() << " (OUTWARDS)@ " << start.second->get_id() << " / "
                          << start.second->get_name() << std::endl;
                for (const auto& [mpg, nets] : connected)
                {
                    for (const auto& net : nets)
                    {
                        std::cout << "\t" << mpg.first->get_id() << " / " << mpg.first->get_name() << " - " << mpg.second->get_name() << ": " << net->get_id() << " / " << net->get_name() << std::endl;
                    }
                }
            }
            for (const auto& [start, connected] : connectivity_inwards)
            {
                std::cout << start.first.first->get_id() << " / " << start.first.first->get_name() << " - " << start.first.second->get_name() << " (INWARDS)@ " << start.second->get_id() << " / "
                          << start.second->get_name() << std::endl;
                for (const auto& [mpg, nets] : connected)
                {
                    for (const auto& net : nets)
                    {
                        std::cout << "\t" << mpg.first->get_id() << " / " << mpg.first->get_name() << " - " << mpg.second->get_name() << ": " << net->get_id() << " / " << net->get_name() << std::endl;
                    }
                }
            }
#endif

            log_info("bitorder_propagation", "Finished conncetivity analysis for bitorder propagation");

            std::map<MPG, std::map<Net*, u32>> wellformed_module_pin_groups = known_bitorders;

            u32 iteration_ctr = 0;

            while (true)
            {
                // find modules that are neither blocked nor are they already wellformed
                std::vector<MPG> modules_and_pingroup;
                for (const auto& mpg : unknown_bitorders)
                {
                    if (mpg.first->is_top_module())
                    {
                        log_error("bitorder_propagation", "Top module is part of the unknown bitorders!");
                        continue;
                    }

                    // NOTE We can skip module/pin group pairs that are already wellformed
                    if (wellformed_module_pin_groups.find(mpg) == wellformed_module_pin_groups.end())
                    {
                        modules_and_pingroup.push_back(mpg);
                    }
                };

                std::deque<MPG> q = {modules_and_pingroup.begin(), modules_and_pingroup.end()};

                if (q.empty())
                {
                    break;
                }

                log_info("bitorder_propagation", "Starting {}bitorder propagation iteration {}.", (enforce_continuous_bitorders ? "strict " : ""), iteration_ctr);

                std::map<MPG, std::map<Net*, u32>> new_wellformed_module_pin_groups = {};

                while (!q.empty())
                {
                    auto [m, pg] = q.front();
                    q.pop_front();

                    // check wether m has submodules that are in the q
                    bool no_submodules_in_q = true;
                    for (const auto& sub_m : m->get_submodules(nullptr, true))
                    {
                        for (const auto& [sm, sp] : q)
                        {
                            if (sm == sub_m)
                            {
                                no_submodules_in_q = false;
                                break;
                            }
                        }
                    }

                    if (!no_submodules_in_q)
                    {
                        q.push_back({m, pg});
                        continue;
                    }

                    bool successors = pg->get_direction() == PinDirection::output;

                    std::map<Net*, POSSIBLE_BITINDICES> collected_inwards;
                    std::map<Net*, POSSIBLE_BITINDICES> collected_outwards;
                    std::map<Net*, POSSIBLE_BITINDICES> collected_combined;

                    for (const auto& pin : pg->get_pins())
                    {
                        Net* starting_net = pin->get_net();

                        // ############################################### //
                        // ################### INWARDS ################### //
                        // ############################################### //

                        if (auto con_it = connectivity_inwards.find({{m, pg}, starting_net}); con_it == connectivity_inwards.end())
                        {
                            // log_warning("bitorder_propagation",
                            //             "There are no valid origins connected to modue {} / {} with pin group {} and net {} / {}.",
                            //             m->get_id(),
                            //             m->get_name(),
                            //             pg->get_name(),
                            //             starting_net->get_id(),
                            //             starting_net->get_name());
                            continue;
                        }

                        const auto& connected_inwards = connectivity_inwards.at({{m, pg}, starting_net});

                        for (const auto& [org_mpg, org_nets] : connected_inwards)
                        {
                            if (auto mpg_it = wellformed_module_pin_groups.find(org_mpg); mpg_it != wellformed_module_pin_groups.end())
                            {
                                const auto& nets = mpg_it->second;
                                for (const auto& org_net : org_nets)
                                {
                                    if (auto net_it = nets.find(org_net); net_it != nets.end())
                                    {
                                        collected_inwards[starting_net][org_mpg].insert(net_it->second);
                                        collected_combined[starting_net][org_mpg].insert(net_it->second);
                                    }
                                    else
                                    {
                                        log_warning("bitorder_propagation",
                                                    "Module {} / {} and pin group {} are wellformed but are missing an index for net {} / {}!",
                                                    org_mpg.first->get_id(),
                                                    org_mpg.first->get_name(),
                                                    org_mpg.second->get_name(),
                                                    org_net->get_id(),
                                                    org_net->get_name());
                                    }
                                }
                            }
                        }

                        // ############################################### //
                        // ################### OUTWARDS ################## //
                        // ############################################### //

                        if (auto con_it = connectivity_outwards.find({{m, pg}, starting_net}); con_it == connectivity_outwards.end())
                        {
                            // log_warning("bitorder_propagation",
                            //             "There are no valid origins connected to modue {} / {} with pin group {} and net {} / {}.",
                            //             m->get_id(),
                            //             m->get_name(),
                            //             pg->get_name(),
                            //             starting_net->get_id(),
                            //             starting_net->get_name());
                            continue;
                        }

                        const auto& connected_outwards = connectivity_outwards.at({{m, pg}, starting_net});

                        for (const auto& [org_mpg, org_nets] : connected_outwards)
                        {
                            if (auto mpg_it = wellformed_module_pin_groups.find(org_mpg); mpg_it != wellformed_module_pin_groups.end())
                            {
                                const auto& nets = mpg_it->second;
                                for (const auto& org_net : org_nets)
                                {
                                    if (auto net_it = nets.find(org_net); net_it != nets.end())
                                    {
                                        collected_outwards[starting_net][org_mpg].insert(net_it->second);
                                        collected_combined[starting_net][org_mpg].insert(net_it->second);
                                    }
                                    else
                                    {
                                        log_warning("bitorder_propagation",
                                                    "Module {} / {} and pin group {} are wellformed but are missing an index for net {} / {}!",
                                                    org_mpg.first->get_id(),
                                                    org_mpg.first->get_name(),
                                                    org_mpg.second->get_name(),
                                                    org_net->get_id(),
                                                    org_net->get_name());
                                    }
                                }
                            }
                        }
                    }

#ifdef PRINT_CONFLICT
                    std::cout << "Extract for " << m->get_id() << " / " << m->get_name() << " - " << pg->get_name() << ": (INWARDS) " << std::endl;
#endif

                    const auto newly_wellformed_inwards = extract_well_formed_bitorder({m, pg}, collected_inwards, enforce_continuous_bitorders);
                    if (!newly_wellformed_inwards.empty())
                    {
                        new_wellformed_module_pin_groups[{m, pg}] = newly_wellformed_inwards;
                        continue;
                    }

#ifdef PRINT_CONFLICT
                    std::cout << "Extract for " << m->get_id() << " / " << m->get_name() << " - " << pg->get_name() << ": (OUTWARDS) " << std::endl;
#endif
                    const auto newly_wellformed_outwards = extract_well_formed_bitorder({m, pg}, collected_outwards, enforce_continuous_bitorders);
                    if (!newly_wellformed_outwards.empty())
                    {
                        new_wellformed_module_pin_groups[{m, pg}] = newly_wellformed_outwards;
                        continue;
                    }

#ifdef PRINT_CONFLICT
                    std::cout << "Extract for " << m->get_id() << " / " << m->get_name() << " - " << pg->get_name() << ": (COMBINED) " << std::endl;
#endif
                    const auto newly_wellformed_combined = extract_well_formed_bitorder({m, pg}, collected_combined, enforce_continuous_bitorders);
                    if (!newly_wellformed_combined.empty())
                    {
                        new_wellformed_module_pin_groups[{m, pg}] = newly_wellformed_combined;
                    }
                }

                if (new_wellformed_module_pin_groups.empty())
                {
                    break;
                }

                log_info("bitorder_propagation", "Found {} new bitorders in iteration: {}", new_wellformed_module_pin_groups.size(), iteration_ctr);

                // NOTE could think about merging if we find that information is lost between iterations
                wellformed_module_pin_groups.insert(new_wellformed_module_pin_groups.begin(), new_wellformed_module_pin_groups.end());

                iteration_ctr++;

                if (iteration_ctr > 100)
                {
                    log_error("bitorder_propagation", "Endless loop protection, something went wrong!");
                    break;
                }
            }

            log_info("bitorder_propagation", "Found a valid bitorder for {} pingroups.", wellformed_module_pin_groups.size());

            return OK(wellformed_module_pin_groups);
        }

        Result<std::monostate> reorder_module_pin_groups(const std::map<MPG, std::map<Net*, u32>>& ordered_module_pin_groups)
        {
            // reorder pin groups to match found bit orders
            for (const auto& [mpg, bitorder] : ordered_module_pin_groups)
            {
                auto m  = mpg.first;
                auto pg = mpg.second;

                std::map<u32, ModulePin*> index_to_pin;

                // collect pins by the nets that run through them and store new index of each pin
                for (const auto& [net, index] : bitorder)
                {
                    ModulePin* pin = m->get_pin_by_net(net);
                    if (pin != nullptr)
                    {
                        auto [current_pin_group, _old_index] = pin->get_group();
                        if (pg == current_pin_group)
                        {
                            index_to_pin[index] = pin;
                        }
                        else
                        {
                            return ERR("cannot reorder module pin groups: pin '" + pin->get_name() + "' appears in bit order of pin group '" + pg->get_name() + "' for module with ID "
                                       + std::to_string(m->get_id()) + " but belongs to pin group '" + current_pin_group->get_name() + "'");
                        }
                    }
                }

                // apply new indices to pins
                for (const auto& [index, pin] : index_to_pin)
                {
                    if (!m->move_pin_within_group(pg, pin, index))
                    {
                        return ERR("cannot reorder module pin groups: failed to move pin '" + pin->get_name() + "' in pin group '" + pg->get_name() + "' of module with ID "
                                   + std::to_string(m->get_id()) + " to new index " + std::to_string(index));
                    }

                    const auto pin_name = pg->get_name() + "(" + std::to_string(index) + ")";
                    if (auto collision_pins = m->get_pins([pin_name](const ModulePin* pin) { return pin->get_name() == pin_name; }); !collision_pins.empty())
                    {
                        m->set_pin_name(collision_pins.front(), pin_name + "_OLD");
                    }

                    m->set_pin_name(pin, pin_name);
                }
            }

            return OK({});
        }

        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(Netlist* nl, const std::pair<u32, std::string>& src, const std::pair<u32, std::string>& dst)
        {
            const std::vector<std::pair<u32, std::string>> src_vec = {src};
            const std::vector<std::pair<u32, std::string>> dst_vec = {dst};
            return propagate_bitorder(nl, src_vec, dst_vec);
        }

        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(const std::pair<Module*, PinGroup<ModulePin>*>& src,
                                                                                                           const std::pair<Module*, PinGroup<ModulePin>*>& dst)
        {
            const std::vector<std::pair<Module*, PinGroup<ModulePin>*>> src_vec = {src};
            const std::vector<std::pair<Module*, PinGroup<ModulePin>*>> dst_vec = {dst};
            return propagate_bitorder(src_vec, dst_vec);
        }

        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>>
            propagate_bitorder(Netlist* nl, const std::vector<std::pair<u32, std::string>>& src, const std::vector<std::pair<u32, std::string>>& dst)
        {
            std::vector<std::pair<Module*, PinGroup<ModulePin>*>> internal_src;
            std::vector<std::pair<Module*, PinGroup<ModulePin>*>> internal_dst;

            // collect known bit orders
            for (const auto& [mod_id, pg_name] : src)
            {
                auto src_mod = nl->get_module_by_id(mod_id);
                if (src_mod == nullptr)
                {
                    return ERR("Cannot propagate bit order: failed to find a module with ID " + std::to_string(mod_id));
                }

                PinGroup<ModulePin>* src_pin_group = nullptr;
                for (const auto& pin_group : src_mod->get_pin_groups())
                {
                    if (pin_group->get_name() == pg_name)
                    {
                        // check whether there are multiple pin groups with the same name
                        if (src_pin_group != nullptr)
                        {
                            return ERR("Cannot propagate bit order: found multiple pin groups with name " + pg_name + " at module with ID " + std::to_string(mod_id));
                        }

                        src_pin_group = pin_group;
                    }
                }

                if (src_pin_group == nullptr)
                {
                    return ERR("Cannot propagate bit order: failed to find a pin group with the name '" + pg_name + "' at module with ID " + std::to_string(mod_id));
                }

                internal_src.push_back({src_mod, src_pin_group});
            }

            // collect unknown bit orders
            for (const auto& [mod_id, pg_name] : dst)
            {
                auto src_mod = nl->get_module_by_id(mod_id);
                if (src_mod == nullptr)
                {
                    return ERR("Cannot propagate bit order: failed to find a module with ID " + std::to_string(mod_id));
                }

                PinGroup<ModulePin>* src_pin_group = nullptr;
                for (const auto& pin_group : src_mod->get_pin_groups())
                {
                    if (pin_group->get_name() == pg_name)
                    {
                        // check whether there are multiple pin groups with the same name
                        if (src_pin_group != nullptr)
                        {
                            return ERR("Cannot propagate bitorder: found multiple pin groups with name '" + pg_name + "' at module with ID " + std::to_string(mod_id));
                        }

                        src_pin_group = pin_group;
                    }
                }

                if (src_pin_group == nullptr)
                {
                    return ERR("Cannot propagate bitorder: failed to find a pin group with the name '" + pg_name + "' at module with ID " + std::to_string(mod_id));
                }

                internal_dst.push_back({src_mod, src_pin_group});
            }

            // actually propagate the bit order
            return propagate_bitorder(internal_src, internal_dst);
        }

        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& src,
                                                                                                           const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& dst)
        {
            std::map<MPG, std::map<Net*, u32>> known_bitorders;
            std::set<MPG> unknown_bitorders = {dst.begin(), dst.end()};

            // collect known bit orders
            for (auto& [m, pg] : src)
            {
                std::map<Net*, u32> src_bitorder;

                // TODO this is gonna crash if pin does not start at index 0
                for (u32 index = 0; index < pg->get_pins().size(); index++)
                {
                    auto pin_res = pg->get_pin_at_index(index);
                    if (pin_res.is_error())
                    {
                        return ERR_APPEND(pin_res.get_error(), "cannot propagate bit order: failed to get pin at index " + std::to_string(index) + " inside of pin group '" + pg->get_name() + "'");
                    }
                    const ModulePin* pin = pin_res.get();

                    src_bitorder.insert({pin->get_net(), index});
                }

                known_bitorders.insert({{m, pg}, src_bitorder});
            }

            // actually propagate the bit order
            const auto res = propagate_module_pingroup_bitorder(known_bitorders, unknown_bitorders);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "cannot propagate bit order: failed propagation");
            }

            const auto all_wellformed_module_pin_groups = res.get();

            // apply bit orders to module pin groups (rename and reorder pins)
            reorder_module_pin_groups(all_wellformed_module_pin_groups);

#ifdef PRINT_GENERAL
            for (const auto& [mpg, bitorder] : all_wellformed_module_pin_groups)
            {
                auto m  = mpg.first;
                auto pg = mpg.second;

                std::cout << "Module: " << m->get_id() << " / " << m->get_name() << ": " << std::endl;
                std::cout << "Pingroup: " << pg->get_name() << ": " << std::endl;

                for (const auto& [net, index] : bitorder)
                {
                    std::cout << net->get_id() << ": " << index << std::endl;
                }
            }
#endif

            // print stats
            const u32 all_wellformed_bitorders_count = all_wellformed_module_pin_groups.size();
            const u32 new_bit_order_count            = all_wellformed_bitorders_count - src.size();

            log_info("bitorder_propagation", "reconstructed {} unknown bit orders from {} known bit orders", new_bit_order_count, src.size());
            log_info("bitorder_propagation", "{} / {} = {} of all unknown bit orders", new_bit_order_count, dst.size(), double(new_bit_order_count) / double(dst.size()));
            log_info("bitorder_propagation",
                     "{} / {} = {} of all pin group bit orders",
                     all_wellformed_bitorders_count,
                     dst.size() + src.size(),
                     double(all_wellformed_bitorders_count) / double(dst.size() + src.size()));

            return OK(all_wellformed_module_pin_groups);
        }
    }    // namespace bitorder_propagation
}    // namespace hal