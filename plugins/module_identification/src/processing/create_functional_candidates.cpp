#include "hal_core/utilities/log.h"
#include "module_identification/candidates/base_candidate.h"
#include "module_identification/candidates/functional_candidate.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/utils/utils.h"

namespace hal
{
    namespace module_identification
    {
        //comparisons

        namespace
        {
            // creates a sorted list of Nets pairing 2 nets with close to same influence together for leq and eq comparisons
            Result<std::vector<std::pair<Net*, Net*>>> create_net_pair_sorting_by_influence(CandidateContext& ctx, const Net* output_net, const std::map<Net*, BooleanFunction::Value>& initial_mapping)
            {
                const Netlist* nl = output_net->get_netlist();

                auto to_use_control_mapping = initial_mapping;

                std::vector<std::pair<Net*, Net*>> result;
                while (true)
                {
                    const auto res = ctx.get_boolean_influence(output_net, to_use_control_mapping);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "failed to create boolean influence");
                    }
                    const auto influence_mapping = res.get();

                    // get 5 pairs with the highest influence as they are ordered enough
                    std::vector<std::pair<Net*, double>> influence_vector;
                    for (std::pair<std::string, double> cur_pair : influence_mapping)
                    {
                        Net* cur_net = BooleanFunctionNetDecorator::get_net_from(nl, cur_pair.first).get();
                        influence_vector.push_back(std::make_pair(cur_net, cur_pair.second));
                    }

                    // if at any point the influence vector does not contain an even amount of nets we can stop
                    if ((influence_vector.size() % 2) != 0)
                    {
                        return OK({});
                    }

                    // sort the values to take first 10
                    std::sort(influence_vector.begin(), influence_vector.end(), [](const std::pair<Net*, double>& a, const std::pair<Net*, double>& b) { return a.second > b.second; });

                    const u32 upper_bound = std::min((u32)influence_vector.size(), (u32)10);
                    for (u32 i = 0; i < upper_bound; i += 2)
                    {
                        to_use_control_mapping[influence_vector[i].first]     = BooleanFunction::Value::ZERO;
                        to_use_control_mapping[influence_vector[i + 1].first] = BooleanFunction::Value::ZERO;

                        result.push_back(std::make_pair(influence_vector[i].first, influence_vector[i + 1].first));
                    }

                    if (influence_vector.size() <= 10)
                    {
                        break;
                    }
                }

                std::reverse(result.begin(), result.end());

                return OK(result);
            }
        }    // namespace

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::trim_to_single_output_net(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> result;
            if (candidate.m_output_nets.size() > 2 || candidate.m_gates.empty())
            {
                return OK(std::vector<FunctionalCandidate>());
            }
            for (Net* cur_out_net : candidate.m_output_nets)
            {
                auto new_candidate          = FunctionalCandidate(candidate);
                new_candidate.m_output_nets = {cur_out_net};

                result.push_back(new_candidate);
            }
            return OK(result);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::discard_equal_candidate(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            std::vector<FunctionalCandidate> candidates;
            const auto var_names_res = ctx.get_variable_names(candidate.m_output_nets.front(), candidate.m_control_mapping);
            if (var_names_res.is_error())
            {
                return ERR_APPEND(var_names_res.get_error(), "cannot check whether to discard equal candidate: failed to retrieve variable names from context");
            }
            const auto& var_names = var_names_res.get();

            if (var_names.size() % 2 == 0)
            {
                candidates.push_back(FunctionalCandidate(candidate));
            }
            return OK(candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::find_control_signals(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            std::vector<FunctionalCandidate> candidates = {FunctionalCandidate(candidate)};
            const Netlist* nl                           = candidate.m_gates.front()->get_netlist();

            auto res = ctx.get_boolean_influence(candidate.m_output_nets[0], candidate.m_control_mapping);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "cannot find control signals via Boolean influence: failed Boolean influence calculation");
            }
            std::unordered_map<std::string, double> influence_mapping = res.get();

            std::vector<std::pair<Net*, double>> influence_vector;
            for (std::pair<std::string, double> cur_pair : influence_mapping)
            {
                Net* cur_net = BooleanFunctionNetDecorator::get_net_from(nl, cur_pair.first).get();
                influence_vector.push_back(std::make_pair(cur_net, cur_pair.second));
            }

            // sort the values to categorize the signal of the highest influence as control signal
            std::sort(influence_vector.begin(), influence_vector.end(), [](const std::pair<Net*, double>& a, const std::pair<Net*, double>& b) { return a.second > b.second; });
            // const u32 max_control_signals = std::min(candidate.m_max_control_signals, u32(influence_vector.size() - 1));
            const u32 max_control_signals = std::min(candidate.m_max_control_signals, u32(1));
            for (u32 num_control_signals = 0; num_control_signals < max_control_signals; num_control_signals++)
            {
                FunctionalCandidate new_candidate = FunctionalCandidate(candidate);
                std::vector<Net*> control_signals;
                for (u32 net_idx = 0; net_idx < num_control_signals + 1; net_idx++)
                {
                    control_signals.push_back(influence_vector[net_idx].first);
                }

                new_candidate.m_control_signals = control_signals;
                candidates.push_back(new_candidate);
            }

            return OK(candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::create_sign_extension_variants(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            std::vector<FunctionalCandidate> new_candidates = {FunctionalCandidate(candidate)};

            const Netlist* nl = candidate.m_gates[0]->get_netlist();
            auto res          = ctx.get_boolean_influence(candidate.m_output_nets[0], candidate.m_control_mapping);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), ("failed to get boolean influence for net " + candidate.m_output_nets[0]->get_name()));
            }
            auto influences = res.get();

            // convert influence to vector of nets
            std::vector<std::pair<Net*, double>> influence_vector;
            for (const auto& cur_pair : influences)
            {
                Net* cur_net = BooleanFunctionNetDecorator::get_net_from(nl, cur_pair.first).get();
                influence_vector.push_back(std::make_pair(cur_net, cur_pair.second));
            }

            if (influence_vector.size() < 3)
            {
                return OK(new_candidates);
            }

            // sort the values to find the nets with the biggest influence
            std::sort(influence_vector.begin(), influence_vector.end(), [](const std::pair<Net*, double>& a, const std::pair<Net*, double>& b) { return a.second > b.second; });

            // sign extended variables are expected to be about 2 times the influence of the next bit
            if ((influence_vector[0].second * 0.75 < influence_vector[1].second))
            {
                return OK(new_candidates);
            }

            auto new_candidate = FunctionalCandidate(candidate);
            new_candidate.m_sign_nets.push_back(influence_vector[0].first);

            // all extension bits basicly have about the same influence after the first
            for (u32 net_idx = 1; net_idx < (influence_vector.size() - 1); net_idx++)
            {
                new_candidate.m_sign_nets.push_back(influence_vector[net_idx].first);

                if ((influence_vector[net_idx].second * 0.75) > influence_vector[net_idx + 1].second)
                {
                    break;
                }
            }

            // we dont allow more than 6 sign extension bits for complexity
            if (new_candidate.m_sign_nets.size() > 6)
            {
                return OK(new_candidates);
            }

            // sign extend in a way that the remaining operands are the same size
            if (((influence_vector.size() - new_candidate.m_sign_nets.size()) % 2) != 0)
            {
                return OK(new_candidates);
            }

            new_candidates.push_back(new_candidate);

            return OK(new_candidates);
        }

        bool has_constant_value(const z3::expr& e, const u64& val)
        {
            if (!e.is_numeral())
            {
                return false;
            }

            if (e.get_sort().bv_size() > 64)
            {
                return false;
            }

            return e.get_numeral_uint64() == val;
        }

        bool has_constant_value(const BooleanFunction& bf, const u64& val)
        {
            return bf.has_constant_value(val);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::order_input_operands(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            auto input_mapping = candidate.m_control_mapping;

            for (const auto& net : candidate.m_sign_nets)
            {
                input_mapping.insert({net, BooleanFunction::Value::ZERO});
            }

            const auto input_nets_res = ctx.get_variable_nets(candidate.m_output_nets[0], input_mapping);
            if (input_nets_res.is_error())
            {
                return ERR_APPEND(input_nets_res.get_error(), "cannot create net pair sorting: failed to retrieve variable nets");
            }
            const auto input_nets = input_nets_res.get();

            if ((input_nets.size() % 2) != 0)
            {
                return OK({});
            }

            FunctionalCandidate new_candidate = FunctionalCandidate(candidate);

            auto input_pair_res = create_net_pair_sorting_by_influence(ctx, new_candidate.m_output_nets[0], input_mapping);
            if (input_pair_res.is_error())
            {
                return ERR_APPEND(input_pair_res.get_error(), "failed to create sorting of net pairs for LEQ");
            }
            const auto input_pairs = input_pair_res.get();

            if (input_pairs.empty())
            {
                return OK({});
            }

            // now evaluate the function to check which is supposed bigger value
            // set all inputs to zero
            for (const auto& n : input_nets)
            {
                input_mapping.insert({n, BooleanFunction::Value::ZERO});
            }

            //save operands to append later
            auto to_append_operands = new_candidate.m_operands;
            new_candidate.m_operands.clear();
            new_candidate.m_operands.push_back({});
            new_candidate.m_operands.push_back({});

            for (const std::pair<Net*, Net*>& cur_bit : input_pairs)
            {
                input_mapping[cur_bit.first] = BooleanFunction::Value::ONE;
                bool found_first             = false;
                const auto first_res         = ctx.get_boolean_function(new_candidate.m_output_nets[0], input_mapping);
                if (first_res.is_error())
                {
                    return ERR_APPEND(first_res.get_error(), "failed to evaluate boolean functions");
                }

                if (has_constant_value(first_res.get(), 1))
                {
                    found_first = true;
                }
                // swap and check
                input_mapping[cur_bit.first]  = BooleanFunction::Value::ZERO;
                input_mapping[cur_bit.second] = BooleanFunction::Value::ONE;

                const auto res = ctx.get_boolean_function(new_candidate.m_output_nets[0], input_mapping);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "failed to evaluate boolean functions");
                }

                input_mapping[cur_bit.second] = BooleanFunction::Value::ZERO;
                if ((has_constant_value(res.get(), 1) && found_first) || (has_constant_value(res.get(), 0) && !found_first))
                {
                    return OK({});
                }

                //append newly found operands
                if (found_first)
                {
                    // keep order
                    new_candidate.m_operands.at(0).push_back(cur_bit.second);
                    new_candidate.m_operands.at(1).push_back(cur_bit.first);
                }
                else
                {
                    // switch order if equation is true for second
                    new_candidate.m_operands.at(0).push_back(cur_bit.first);
                    new_candidate.m_operands.at(1).push_back(cur_bit.second);
                }
            }

            std::vector<FunctionalCandidate> result;
            result.push_back(new_candidate);

            // create variantions for the different possible orderings of sign nets
            for (u32 msb_idx = 1; msb_idx < candidate.m_sign_nets.size(); msb_idx++)
            {
                std::vector<Net*> extension_side;
                std::vector<Net*> not_extended_side;

                for (u32 idx = 1; idx < candidate.m_sign_nets.size(); idx++)
                {
                    if (idx == msb_idx)
                    {
                        continue;
                    }

                    extension_side.push_back(candidate.m_sign_nets.at(msb_idx));
                    not_extended_side.push_back(candidate.m_sign_nets.at(idx));
                }

                extension_side.push_back(candidate.m_sign_nets.at(msb_idx));
                not_extended_side.push_back(candidate.m_sign_nets.at(0));

                //clone current new candidate and create two versions
                auto new_candidate_left  = FunctionalCandidate(new_candidate);
                auto new_candidate_right = FunctionalCandidate(new_candidate);

                //do sign extension for both sides
                new_candidate_left.m_operands[0].insert(new_candidate_left.m_operands[0].end(), extension_side.begin(), extension_side.end());
                new_candidate_left.m_operands[1].insert(new_candidate_left.m_operands[1].end(), not_extended_side.begin(), not_extended_side.end());
                new_candidate_right.m_operands[0].insert(new_candidate_right.m_operands[0].end(), not_extended_side.begin(), not_extended_side.end());
                new_candidate_right.m_operands[1].insert(new_candidate_right.m_operands[1].end(), extension_side.begin(), extension_side.end());

                result.push_back(new_candidate_left);
                result.push_back(new_candidate_right);
            }

            return OK(result);
        }

        // adders subtractor, counter and const mul stuff

        //helper functions
        namespace
        {
            std::vector<std::vector<Net*>> combinations(const std::vector<Net*>& candidates, const u32 k)
            {
                std::vector<std::vector<Net*>> res;
                std::vector<Net*> tmp;
                std::string bitmask(k, 1);               // K leading 1's
                bitmask.resize(candidates.size(), 0);    // N-K trailing 0's

                do
                {
                    for (int i = 0; i < candidates.size(); ++i)    // [0..N-1] integers
                    {
                        if (bitmask[i])
                        {
                            tmp.push_back(candidates[i]);
                        }
                    }

                    res.push_back(tmp);
                    tmp.clear();
                } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
                return res;
            }

            std::map<Net*, BooleanFunction::Value> generate_control_mapping(const std::vector<Net*>& ctrl_signlas, u32 val)
            {
                std::map<Net*, BooleanFunction::Value> substitution_map;

                for (u32 idx = 0; idx < ctrl_signlas.size(); idx++)
                {
                    const u32 val_i                        = (val >> idx) & 0x1;
                    substitution_map[ctrl_signlas.at(idx)] = val_i ? BooleanFunction::Value::ONE : BooleanFunction::ZERO;
                }

                return substitution_map;
            }

            std::vector<std::pair<std::map<u32, std::vector<Net*>>, std::vector<Net*>>>
                generate_control_input_splits(const std::pair<std::map<u32, std::vector<Net*>>, std::vector<Net*>>& initial, const u32 threshold, const u32 max_control_signals, const u32 max_depth)
            {
                const auto& [count_to_vars, ctrl_vars] = initial;

                if (ctrl_vars.size() > max_control_signals)
                {
                    return {};
                }

                for (const auto& [count, vars] : count_to_vars)
                {
                    if (vars.size() > threshold)
                    {
                        // check how many control signals have to be removed from the bin in order to fit the threshold
                        const u32 excess = vars.size() - threshold;
                        if (excess > max_control_signals)
                        {
                            return {};
                        }

                        // if we already checked more bins for control signals than allowed, return an empty list
                        if (max_depth == 0)
                        {
                            return {};
                        }

                        // find all possible combinations that can be removed from the bin
                        const auto combs = combinations(vars, excess);

                        // for each combination build the split with the chosen control signals removed from the bin
                        std::vector<std::pair<std::map<u32, std::vector<Net*>>, std::vector<Net*>>> tmp;
                        for (const auto& c : combs)
                        {
                            auto new_result = initial;

                            for (const auto& ctrl_sig : c)
                            {
                                auto& c_vars = new_result.first.at(count);
                                c_vars.erase(std::remove(c_vars.begin(), c_vars.end(), ctrl_sig), c_vars.end());
                                new_result.second.push_back(ctrl_sig);
                            }

                            // TODO clean up
                            // const auto& [variable_count, ctrl_vars] = new_result;
                            tmp.push_back(new_result);
                        }

                        // for each new split now check all the other bins recursively
                        std::vector<std::pair<std::map<u32, std::vector<Net*>>, std::vector<Net*>>> result;
                        for (const auto& t : tmp)
                        {
                            for (const auto& split : generate_control_input_splits(t, threshold, max_control_signals, max_depth - 1))
                            {
                                result.push_back(split);
                            }
                        }

                        return result;
                    }
                }

                return {initial};
            }

        }    // namespace

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::identify_control_signals(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            if (candidate.m_max_control_signals < 5)
            {
                // determine the number of operands by finding the most common size of variable set (for a N input adder we expect most var sets to contain N variables)
                std::map<u32, u32> size_to_occurence;
                std::set<u32> sizes;
                std::vector<u32> size_list;
                for (const auto& [count, vars] : candidate.m_influence_count_to_input_nets)
                {
                    size_list.push_back(vars.size());
                    sizes.insert(vars.size());
                    size_to_occurence[vars.size()] += 1;
                }

                std::sort(size_list.begin(), size_list.end());

                // TODO this happens for the lattice canny edge detector benchmark, need to investigate
                if (sizes.empty())
                {
                    log_warning("module_identification", "unable to identify control signals: found empty size list");
                    return OK({});
                }

                const u32 upper_bound = (sizes.size() == 1) ? *(size_list.begin()) : std::min({*(size_list.rbegin()), *std::next(size_list.rbegin(), 1)});
                u32 threshold         = candidate.m_max_operands ? candidate.m_max_operands : upper_bound;

                const u32 upper_bound_alt = (sizes.size() == 2) ? *(size_list.begin()) : std::min({*std::next(size_list.rbegin(), 1), *std::next(size_list.rbegin(), 2)});
                u32 threshold_alt         = candidate.m_max_operands ? candidate.m_max_operands : upper_bound_alt;

                const u32 max_bins_including_control = 2;

                threshold     = std::min({u32(6), threshold});
                threshold_alt = std::min({u32(6), threshold_alt});

                // generate all possible combinations of nets such that in any bin are only two input vars left at max while the rest of the vars is considered control variables
                auto control_input_splits = generate_control_input_splits({candidate.m_influence_count_to_input_nets, {}}, threshold, candidate.m_max_control_signals, max_bins_including_control);

                u32 ctrl_signals          = control_input_splits.empty() ? 0 : control_input_splits.front().second.size();
                u32 future_candidate_size = control_input_splits.size() * (1 << ctrl_signals);

                // std::cout << "Found " << ctrl_signals << " control signals and " << control_input_splits.size() << " split leading to a future size of " << future_candidate_size << " candidates."
                //           << std::endl;
                if (future_candidate_size > 32000)
                {
                    control_input_splits.clear();
                }

                // TODO remove debug pru32ing
                // std::cout << "Found " << control_input_splits.size() << " control splits for " << threshold << " max operands and " << candidate.m_max_control_signals << " max control signals ." << std::endl;

                if (threshold_alt != threshold)
                {
                    auto control_input_splits_alt =
                        generate_control_input_splits({candidate.m_influence_count_to_input_nets, {}}, threshold_alt, candidate.m_max_control_signals, max_bins_including_control);

                    // TODO remove debug printing
                    // std::cout << "Found " << control_splits_alt.size() << " alternative control splits for " << threshold_alt << " max operands and " << candidate.m_max_control_signals
                    //           << " max control signals ." << std::endl;

                    u32 ctrl_signals_alt          = control_input_splits_alt.empty() ? 0 : control_input_splits_alt.front().second.size();
                    u32 future_candidate_size_alt = control_input_splits_alt.size() * (1 << ctrl_signals_alt);

                    // std::cout << "Found " << ctrl_signals_alt << " control signals and " << control_input_splits_alt.size() << " split leading to a future size of " << future_candidate_size_alt
                    //           << " candidates." << std::endl;
                    if (future_candidate_size_alt > 32000)
                    {
                        control_input_splits_alt.clear();
                    }

                    for (const auto& split : control_input_splits_alt)
                    {
                        // TODO maybe check for uniqueness
                        control_input_splits.push_back(split);
                    }
                }

                // for each split generate a new candidate
                bool added_vanilla_candidate = false;
                for (const auto& [_variable_count, ctrl_vars] : control_input_splits)
                {
                    auto new_candidate = FunctionalCandidate(candidate);

                    // new_candidate.m_variable_count = variable_count;
                    for (const auto& cv : ctrl_vars)
                    {
                        new_candidate.m_control_signals.push_back(cv);
                    }

                    new_candidates.push_back(new_candidate);

                    if (ctrl_vars.empty())
                    {
                        added_vanilla_candidate = true;
                    }
                }

                if ((candidate.m_candidate_type == module_identification::CandidateType::counter || candidate.m_candidate_type == module_identification::CandidateType::absolute)
                    && !added_vanilla_candidate)
                {
                    auto pass_through_candidate = FunctionalCandidate(candidate);
                    pass_through_candidate.m_control_signals.clear();
                    for (const auto& [ctrl_val, _] : pass_through_candidate.m_control_mapping)
                    {
                        pass_through_candidate.m_control_signals.push_back(ctrl_val);
                    }
                    new_candidates.push_back(pass_through_candidate);
                }
            }
            else
            {
                log_error("module_identification", "More than 5 control signals are currently not implemented!");
                new_candidates = {candidate};
            }

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::create_operand_control_variations(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            for (const auto& ctrl_signal : candidate.m_control_signals)
            {
                auto new_candidate = FunctionalCandidate(candidate);
                new_candidate.m_control_signals.erase(std::remove(new_candidate.m_control_signals.begin(), new_candidate.m_control_signals.end(), ctrl_signal), new_candidate.m_control_signals.end());
                new_candidate.m_ctrl_to_operand_net = ctrl_signal;

                new_candidates.push_back(new_candidate);
            }

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::realize_control_signals(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            const u32 max_val = 1 << candidate.m_control_signals.size();
            for (u32 val = 0; val < max_val; val++)
            {
                auto new_candidate                  = FunctionalCandidate(candidate);
                new_candidate.m_max_control_signals = 0;

                if (module_identification::candidate_type_max_operands.find(new_candidate.m_candidate_type) != module_identification::candidate_type_max_operands.end())
                {
                    new_candidate.m_max_operands = module_identification::candidate_type_max_operands.at(new_candidate.m_candidate_type);
                }

                new_candidate.m_control_mapping = generate_control_mapping(candidate.m_control_signals, val);

                new_candidates.push_back(new_candidate);
            }

            return OK(new_candidates);
        }

        namespace
        {
            std::vector<Net*> order_by_influence(const std::vector<Net*>& nets, CandidateContext& ctx, const std::map<hal::Net*, hal::BooleanFunction::Value>& ctrl_mapping)
            {
                std::vector<Net*> reordered_nets = nets;
                std::map<Net*, double> output_net_to_influence_score;
                for (const auto& n : reordered_nets)
                {
                    const auto influences_res = ctx.get_boolean_influence(n, ctrl_mapping);
                    if (influences_res.is_error())
                    {
                        log_error(influences_res.get_error().get());
                        return {};
                    }
                    const auto& influences = influences_res.get();

                    if (influences.empty())
                    {
                        // log_error("module_identification", "got empty Boolean influence for net {} with function: {}", n->get_name(), ctx.get_boolean_function(n, ctrl_mapping).to_string());
                        return {};
                    }

                    double influence_score = 0;
                    for (const auto& [_, inf] : influences)
                    {
                        influence_score += inf;
                    }

                    output_net_to_influence_score.insert({n, influence_score});
                }

                std::sort(reordered_nets.begin(), reordered_nets.end(), [&output_net_to_influence_score](const auto& n1, const auto& n2) {
                    return output_net_to_influence_score.at(n1) < output_net_to_influence_score.at(n2);
                });

                return reordered_nets;
            }

            std::vector<std::vector<Net*>> permutations(const std::vector<Net*>& nets)
            {
                std::vector<Net*> initial = nets;
                std::vector<std::vector<Net*>> result;

                std::sort(initial.begin(), initial.end());

                do
                {
                    result.push_back({initial.begin(), initial.end()});
                } while (std::next_permutation(initial.begin(), initial.end()));

                return result;
            }

            std::vector<std::vector<Net*>> reorder_overfull_input_bins(const std::map<u32, std::vector<Net*>>& initial, const u32 max_overfull_bins)
            {
                std::vector<std::vector<Net*>> results;
                std::vector<Net*> singleton_bins;
                std::vector<Net*> to_permute;
                std::vector<std::vector<Net*>> overfull_bins;

                bool found_singleton_bin = false;
                for (auto it = initial.rbegin(); it != initial.rend(); it++)
                {
                    const auto nets = it->second;
                    if (nets.size() != 1)
                    {
                        // we only reorder overfull bins that starting from the start.
                        // after encountering the first bin with only one element we abort when we find another overfull bin further down.
                        if (found_singleton_bin)
                        {
                            return {};
                        }

                        to_permute.insert(to_permute.end(), nets.begin(), nets.end());
                        overfull_bins.push_back(nets);
                    }
                    else
                    {
                        found_singleton_bin = true;

                        singleton_bins.push_back(nets.front());
                    }
                }

                // Due to computational complexity we limit the number of overfull bins to permute to 5
                if (overfull_bins.size() > max_overfull_bins)
                {
                    return {};
                }

                // Due to computational complexity we limit the number of variable to permute to 5
                if (to_permute.size() <= 5)
                {
                    std::sort(to_permute.begin(), to_permute.end());

                    do
                    {
                        std::vector<Net*> tmp = {to_permute.begin(), to_permute.end()};
                        tmp.insert(tmp.end(), singleton_bins.begin(), singleton_bins.end());

                        results.push_back(tmp);
                    } while (std::next_permutation(to_permute.begin(), to_permute.end()));
                }
                else
                {
                    // check whether all overfull bins have the same size
                    std::set<u32> overfull_sizes;
                    for (const auto& bin : overfull_bins)
                    {
                        overfull_sizes.insert(bin.size());
                    }

                    if (overfull_sizes.size() != 1)
                    {
                        return {};
                    }

                    // create all possible combinations of individually permuted bins
                    std::vector<std::vector<std::vector<Net*>>> isolated_permuted_bins = {{}};    // initialize with one empyt bin set
                    std::vector<std::vector<std::vector<Net*>>> new_isolated_permuted_bins;
                    for (auto bin : overfull_bins)
                    {
                        if (bin.size() > 5)
                        {
                            return {};
                        }

                        std::sort(bin.begin(), bin.end());
                        do
                        {
                            for (const auto& bin_set : isolated_permuted_bins)
                            {
                                auto new_bin_set = bin_set;
                                new_bin_set.push_back(bin);
                                new_isolated_permuted_bins.push_back(new_bin_set);
                            }
                        } while (std::next_permutation(bin.begin(), bin.end()));

                        isolated_permuted_bins = new_isolated_permuted_bins;
                        new_isolated_permuted_bins.clear();
                    }

                    for (const auto& bin_set : isolated_permuted_bins)
                    {
                        std::vector<Net*> input_operand;
                        for (u32 idx = 0; idx < bin_set.front().size(); idx++)
                        {
                            for (const auto& bin : bin_set)
                            {
                                input_operand.push_back(bin.at(idx));
                            }
                        }
                        input_operand.insert(input_operand.end(), singleton_bins.begin(), singleton_bins.end());
                        results.push_back(input_operand);
                    }
                }

                return results;
            }

            std::vector<std::vector<Net*>>
                reorder_overfull_output_bins(CandidateContext& ctx, const std::map<hal::Net*, hal::BooleanFunction::Value>& ctrl_mapping, const std::map<u32, std::vector<Net*>>& initial)
            {
                std::vector<std::vector<Net*>> prev_results;
                std::vector<std::vector<Net*>> next_results;

                std::vector<std::vector<Net*>> bins;

                for (auto it = initial.rbegin(); it != initial.rend(); it++)
                {
                    bins.push_back(it->second);
                }

                for (const auto& bin : bins)
                {
                    // Due to computational complexity we limit the content of overfull bins to 6
                    const auto perm = (bin.size() < 4) ? permutations(bin) : std::vector<std::vector<Net*>>{order_by_influence(bin, ctx, ctrl_mapping)};

                    // if the permutation is empty, we cannot create a good canidate. This can be the case for constant output functions with no influences
                    if (perm.empty())
                    {
                        return {};
                    }

                    if (prev_results.empty())
                    {
                        next_results = perm;
                    }
                    else
                    {
                        for (const auto& pr : prev_results)
                        {
                            for (const auto& p : perm)
                            {
                                std::vector<Net*> tmp = pr;
                                tmp.insert(tmp.end(), p.begin(), p.end());
                                next_results.push_back(tmp);
                            }
                        }
                    }

                    prev_results = next_results;
                    next_results.clear();
                }

                return prev_results;
            }
        }    // namespace

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::update_input_output_stats(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            auto new_candidate = FunctionalCandidate(candidate);

            if (candidate.m_output_nets.empty())
            {
                return OK({});
            }

            if (candidate.m_output_nets.size() < 2)
            {
                return OK({});
            }

            std::vector<std::set<hal::Net*>> variable_nets;
            for (u32 o_idx = 0; o_idx < candidate.m_output_nets.size(); o_idx++)
            {
                const auto& n                = candidate.m_output_nets.at(o_idx);
                const auto variable_nets_res = ctx.get_variable_nets(n, candidate.m_control_mapping);
                if (variable_nets_res.is_error())
                {
                    return ERR_APPEND(variable_nets_res.get_error(),
                                      "cannot update input output stats: failed to retrieve input variable nets for net " + n->get_name() + " with ID " + std::to_string(n->get_id()));
                }

                variable_nets.push_back(variable_nets_res.get());
            }

            // SINGLE INPUT OUTPUTS
            std::map<Net*, Net*> single_input_to_output;
            for (u32 o_idx = 0; o_idx < candidate.m_output_nets.size(); o_idx++)
            {
                const auto& n = candidate.m_output_nets.at(o_idx);
                if (variable_nets.at(o_idx).size() == 1)
                {
                    const auto single_net = *(variable_nets.at(o_idx).begin());

                    // NOTE: currently a input net that drives two single input output nets can not create any meaningful caniddates
                    if (single_input_to_output.find(single_net) != single_input_to_output.end())
                    {
                        return OK({});
                    }

                    single_input_to_output.insert({single_net, n});
                }
            }

            // INPUTS
            std::map<Net*, u32> input_nets_to_influenced_outputs;
            for (u32 o_idx = 0; o_idx < candidate.m_output_nets.size(); o_idx++)
            {
                for (const auto& input_net : variable_nets.at(o_idx))
                {
                    // TODO check why we dont do this for constant multiplications
                    // ignore single inputs from the input operand count for adders and counters
                    if (candidate.m_candidate_type == module_identification::CandidateType::adder || candidate.m_candidate_type == module_identification::CandidateType::counter
                        || candidate.m_candidate_type == module_identification::CandidateType::absolute)
                    {
                        if (single_input_to_output.find(input_net) != single_input_to_output.end())
                        {
                            continue;
                        }
                    }

                    // ignore the ctrl msb for absoulte
                    if (candidate.m_candidate_type == module_identification::CandidateType::absolute)
                    {
                        if (input_net == candidate.m_ctrl_to_operand_net)
                        {
                            continue;
                        }
                    }

                    input_nets_to_influenced_outputs[input_net] += 1;
                }
            }

            // sort input variables into bins
            std::map<u32, std::vector<Net*>> influence_count_to_input_nets;
            for (const auto& [var, count] : input_nets_to_influenced_outputs)
            {
                influence_count_to_input_nets[count].push_back(var);
            }

            u32 max_bin_size = 0;
            for (const auto& [_, nets] : influence_count_to_input_nets)
            {
                if (nets.size() > max_bin_size)
                {
                    max_bin_size = nets.size();
                }
            }

            // NOTE make this a config parameter
            if (max_bin_size > 12)
            {
                // log_warning("module_identification", "abandoned canidate creation for candidate {} due to a max bin size of {}", candidate.m_gates.front()->get_name(), max_bin_size);
                return OK({});
            }

            // OUTPUTS

            // count the input variables to each output function
            std::map<Net*, u32> output_net_to_input_count;
            for (u32 o_idx = 0; o_idx < candidate.m_output_nets.size(); o_idx++)
            {
                const auto& n = candidate.m_output_nets.at(o_idx);
                output_net_to_input_count.insert({n, u32(variable_nets.at(o_idx).size())});
            }

            std::map<u32, std::vector<Net*>> input_count_to_output_nets;
            for (const auto& [n, c] : output_net_to_input_count)
            {
                if ((candidate.m_candidate_type == module_identification::CandidateType::adder || candidate.m_candidate_type == module_identification::CandidateType::counter
                     || candidate.m_candidate_type == module_identification::CandidateType::absolute || candidate.m_candidate_type == module_identification::CandidateType::constant_multiplication)
                    && c == 1)
                {
                    continue;
                }

                input_count_to_output_nets[c].push_back(n);
            }

            new_candidate.m_influence_count_to_input_nets = influence_count_to_input_nets;
            new_candidate.m_input_count_to_output_nets    = input_count_to_output_nets;
            new_candidate.m_single_input_to_output        = single_input_to_output;

            return OK({new_candidate});
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::permute_single_input_signals(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            if (candidate.m_single_input_to_output.size() > 3)
            {
                return OK(std::vector<FunctionalCandidate>());
            }

            std::vector<std::pair<Net*, Net*>> single_input_outputs = {candidate.m_single_input_to_output.begin(), candidate.m_single_input_to_output.end()};
            std::sort(single_input_outputs.begin(), single_input_outputs.end());

            do
            {
                auto new_candidate                    = FunctionalCandidate(candidate);
                new_candidate.m_permuted_single_pairs = single_input_outputs;

                new_candidates.push_back(new_candidate);
            } while (std::next_permutation(single_input_outputs.begin(), single_input_outputs.end()));

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::build_input_operands(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            // check whether all inputs are covered in the output net influenced by the most inputs
            u32 input_count = 0;
            for (const auto& [_, nets] : candidate.m_influence_count_to_input_nets)
            {
                input_count += nets.size();
            }

            if (candidate.m_input_count_to_output_nets.empty())
            {
                return OK({});
            }

            if (candidate.m_input_count_to_output_nets.rbegin()->first < input_count)
            {
                return OK({});
            }

            if (candidate.m_candidate_type == module_identification::CandidateType::adder)
            {
                // 1) Edge Case - Shifted Operand
                // Check whether there is a "hump" in the distribution of var count to input signals and pad the higher var counts with logical zeros to get a continously decreasing amount of variables
                u32 max_variables = 0;
                for (const auto& [_count, nets] : candidate.m_influence_count_to_input_nets)
                {
                    if (nets.size() > max_variables)
                    {
                        max_variables = nets.size();
                    }
                }

                // check whether the first vector of nets is smaller than the max number of nets
                if ((!candidate.m_influence_count_to_input_nets.empty()) && candidate.m_influence_count_to_input_nets.rbegin()->second.size() < max_variables)
                {
                    bool reached_maximum_variable_count = false;
                    std::vector<std::vector<Net*>> operands;
                    for (auto it = candidate.m_influence_count_to_input_nets.rbegin(); it != candidate.m_influence_count_to_input_nets.rend(); it++)
                    {
                        const auto& [count, nets] = *it;
                        if (nets.size() == max_variables)
                        {
                            reached_maximum_variable_count = true;
                        }

                        const u32 max_idx = reached_maximum_variable_count ? nets.size() : max_variables;
                        for (u32 idx = 0; idx < max_idx; idx++)
                        {
                            if (operands.size() <= idx)
                            {
                                operands.push_back({});
                            }

                            if (idx >= nets.size())
                            {
                                operands.at(idx).push_back(candidate.m_gates.front()->get_netlist()->get_gnd_nets().front());
                            }
                            else
                            {
                                operands.at(idx).push_back(nets.at(idx));
                            }
                        }
                    }

                    // TODO this duplicates further below
                    // check validity of constructed operands

                    // The max operands is mainly here to prevent massive explosion of candidate later down the line during operand expansion
                    bool is_valid = (operands.size() >= 2) && (operands.size() <= candidate.m_max_operands);

                    // if one of the operands is only one bit long then this should be a counter with enable signal
                    // NOTE there was a bug regarding the amount of non constant vaiables that i fixed. This would also be adapted in the generic candidate when merging
                    for (const auto& op : operands)
                    {
                        u32 non_const_nets = 0;
                        for (const auto& net : op)
                        {
                            if (!(net->is_gnd_net() || net->is_vcc_net()))
                            {
                                non_const_nets++;
                            }
                        }

                        if (non_const_nets < 2)
                        {
                            is_valid = false;
                        }
                    }

                    if (is_valid)
                    {
                        // TODO remove debug printing
                        // std::cout << "Found the following Candidate after building inputs: " << std::endl;
                        // for (const auto& op : operands)
                        // {
                        //     std::cout << "OP [" << op.size() << "]: " << std::endl;
                        //     for (const auto& var : op)
                        //     {
                        //         std::cout << "\t" << var.to_string() << std::endl;
                        //     }
                        // }

                        auto new_candidate       = FunctionalCandidate(candidate);
                        new_candidate.m_operands = operands;

                        new_candidates.push_back(new_candidate);
                    }
                }

                // 2) Edge Case - Buffered Operand
                // find buffered signals
                // find variables that are just buffered signals and append them to the front of the input operand

                // TODO this can be moved out of the loop
                std::vector<std::vector<Net*>> operands;

                for (auto it = candidate.m_influence_count_to_input_nets.rbegin(); it != candidate.m_influence_count_to_input_nets.rend(); it++)
                {
                    const auto& [count, nets] = *it;
                    for (u32 idx = 0; idx < nets.size(); idx++)
                    {
                        if (candidate.m_max_operands != 0 && idx >= candidate.m_max_operands)
                        {
                            // the candidate would result in more than the maximum number of allowed operands
                            return OK(std::vector<FunctionalCandidate>());
                        }

                        if (operands.size() <= idx)
                        {
                            operands.push_back({});
                        }

                        operands.at(idx).push_back(nets.at(idx));
                    }
                }

                bool is_valid = (operands.size() >= 2) && (operands.size() <= candidate.m_max_operands);
                if (!is_valid)
                {
                    return OK(std::vector<FunctionalCandidate>());
                }

                // add buffered signals to the start of one operand and pad the rest with zeros
                for (const auto& [in_net, out_net] : candidate.m_permuted_single_pairs)
                {
                    for (u32 op_idx = 0; op_idx < operands.size(); op_idx++)
                    {
                        auto val = (op_idx == 0) ? in_net : candidate.m_gates.front()->get_netlist()->get_gnd_nets().front();
                        operands.at(op_idx).insert(operands.at(op_idx).begin(), val);
                    }
                }

                // if one of the operands is only one bit long then this should be a counter with enable signal
                // NOTE there was a bug regarding the amount of non constant vaiables that i fixed. This would also be adapted in the generic candidate when merging
                for (const auto& op : operands)
                {
                    u32 non_const_nets = 0;
                    for (const auto& net : op)
                    {
                        if (!(net->is_gnd_net() || net->is_vcc_net()))
                        {
                            non_const_nets++;
                        }
                    }

                    if (non_const_nets < 2)
                    {
                        return OK(std::vector<FunctionalCandidate>());
                    }
                }

                auto new_candidate       = FunctionalCandidate(candidate);
                new_candidate.m_operands = operands;

                new_candidates.push_back(new_candidate);
            }
            else if (candidate.m_candidate_type == module_identification::CandidateType::counter)
            {
                const auto reorderings = reorder_overfull_input_bins(candidate.m_influence_count_to_input_nets, 2);
                // for each split generate a new candidate
                for (const auto& nets : reorderings)
                {
                    // ignore empty operands or operands with only one bit
                    if (nets.empty() || nets.size() == 1)
                    {
                        continue;
                    }

                    auto new_candidate = FunctionalCandidate(candidate);
                    new_candidate.m_operands.push_back({});
                    for (const auto& [in_net, _] : candidate.m_permuted_single_pairs)
                    {
                        new_candidate.m_operands.front().push_back(in_net);
                    }

                    for (const auto& n : nets)
                    {
                        new_candidate.m_operands.front().push_back(n);
                    }

                    new_candidates.push_back(new_candidate);
                }
            }
            else if (candidate.m_candidate_type == module_identification::CandidateType::constant_multiplication)
            {
                // generate all possible combinations of nets such that in any bin are only two input vars left at max while the rest of the vars is considered control variables
                const auto reorderings = reorder_overfull_input_bins(candidate.m_influence_count_to_input_nets, 5);
                // for each split generate a new candidate
                for (const auto& nets : reorderings)
                {
                    if (nets.empty())
                    {
                        continue;
                    }

                    auto new_candidate = FunctionalCandidate(candidate);
                    new_candidate.m_operands.push_back({});
                    for (const auto& n : nets)
                    {
                        new_candidate.m_operands.front().push_back(n);
                    }

                    new_candidates.push_back(new_candidate);
                }
            }
            else if (candidate.m_candidate_type == module_identification::CandidateType::absolute)
            {
                const auto reorderings = reorder_overfull_input_bins(candidate.m_influence_count_to_input_nets, 2);

                // for each split generate a new candidate
                for (const auto& nets : reorderings)
                {
                    // ignore empty operands or operands with only one bit
                    if (nets.empty() || nets.size() == 1)
                    {
                        continue;
                    }

                    auto new_candidate = FunctionalCandidate(candidate);
                    new_candidate.m_operands.push_back({});
                    for (const auto& [in_net, _] : candidate.m_permuted_single_pairs)
                    {
                        new_candidate.m_operands.front().push_back(in_net);
                    }

                    for (const auto& n : nets)
                    {
                        new_candidate.m_operands.front().push_back(n);
                    }

                    if (new_candidate.m_operands.front().size() >= new_candidate.m_output_nets.size())
                    {
                        return OK({});
                    }

                    if (new_candidate.m_ctrl_to_operand_net != nullptr)
                    {
                        new_candidate.m_operands.front().push_back(new_candidate.m_ctrl_to_operand_net);
                    }

                    new_candidates.push_back(new_candidate);
                }
            }

            // TODO remove sanity check
            for (const auto& nc : new_candidates)
            {
                for (const auto& net_set : nc.m_operands)
                {
                    for (const auto& net : net_set)
                    {
                        if (net->is_gnd_net() || net->is_vcc_net())
                        {
                            continue;
                        }

                        if (const auto it = std::find(nc.m_input_nets.begin(), nc.m_input_nets.end(), net); it == nc.m_input_nets.end())
                        {
                            std::cout << "ERROR! Found net " << net->get_name() << " - " << net->get_id() << " in variable set that is not part of the input nets!" << std::endl;
                        }
                    }
                }
            }

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::order_output_signals(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;
            auto new_candidate = FunctionalCandidate(candidate);

            const std::vector<std::vector<Net*>> reorderings = reorder_overfull_output_bins(ctx, candidate.m_control_mapping, new_candidate.m_input_count_to_output_nets);

            // for each split generate a new candidate
            for (const auto& nets : reorderings)
            {
                if (nets.empty())
                {
                    continue;
                }

                auto reordered_candidate          = FunctionalCandidate(new_candidate);
                reordered_candidate.m_output_nets = {nets.rbegin(), nets.rend()};

                new_candidates.push_back(reordered_candidate);
            }

            return OK(std::move(new_candidates));
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::add_single_input_signals(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;
            auto new_candidate = candidate;

            for (auto it = candidate.m_permuted_single_pairs.rbegin(); it != candidate.m_permuted_single_pairs.rend(); it++)
            {
                const auto& [input_net, output_net] = *it;
                new_candidate.m_output_nets.insert(new_candidate.m_output_nets.begin(), output_net);
            }

            if ((new_candidate.m_candidate_type == module_identification::CandidateType::absolute) && (new_candidate.m_operands.at(0).size() > new_candidate.m_output_nets.size()))
            {
                return OK(std::vector<FunctionalCandidate>());
            }

            new_candidates.push_back(new_candidate);

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::early_abort(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            if (candidate.m_candidate_type == CandidateType::adder)
            {
                std::map<std::string, BooleanFunction::Value> zero_eval_mapping;
                std::map<std::string, BooleanFunction::Value> one_eval_mapping;
                for (const auto& n : candidate.m_input_nets)
                {
                    const std::string var_name = BooleanFunctionNetDecorator(*n).get_boolean_variable_name();
                    if (const auto it = std::find(candidate.m_control_signals.begin(), candidate.m_control_signals.end(), n); it != candidate.m_control_signals.end())
                    {
                        const BooleanFunction::Value val = candidate.m_control_mapping.at(n);
                        zero_eval_mapping.insert({var_name, val});
                        one_eval_mapping.insert({var_name, val});
                    }
                    else
                    {
                        zero_eval_mapping.insert({var_name, BooleanFunction::Value::ZERO});
                        one_eval_mapping.insert({var_name, BooleanFunction::Value::ONE});
                    }
                }

                // set all non control inputs to zero, expect all outputs to be zero or expect all outputs to be zero except one (incase of a offset with a constant one)
                u32 non_zero_count = 0;
                for (const auto& o_net : candidate.m_output_nets)
                {
                    const auto eval_res = ctx.evaluate(o_net, {}, zero_eval_mapping);
                    if (eval_res.is_error())
                    {
                        return ERR_APPEND(eval_res.get_error(), "cannot check for early abort: failed to evaluate Boolean function");
                    }

                    if (eval_res.get().front() != BooleanFunction::Value::ZERO)
                    {
                        non_zero_count++;
                    }

                    if (non_zero_count > 1)
                    {
                        // early abort
                        return OK({});
                    }
                }
            }
            else if (candidate.m_candidate_type == CandidateType::constant_multiplication)
            {
                // set all inputs to zero, expect all outputs to be zero
                std::map<std::string, BooleanFunction::Value> zero_eval_mapping;
                for (const auto& n : candidate.m_input_nets)
                {
                    const std::string var_name = BooleanFunctionNetDecorator(*n).get_boolean_variable_name();
                    if (const auto it = std::find(candidate.m_control_signals.begin(), candidate.m_control_signals.end(), n); it != candidate.m_control_signals.end())
                    {
                        const BooleanFunction::Value val = candidate.m_control_mapping.at(n);
                        zero_eval_mapping.insert({var_name, val});
                    }
                    else
                    {
                        zero_eval_mapping.insert({var_name, BooleanFunction::Value::ZERO});
                    }
                }

                u32 non_zero_count = 0;
                for (const auto& o_net : candidate.m_output_nets)
                {
                    const auto eval_res = ctx.evaluate(o_net, {}, zero_eval_mapping);
                    if (eval_res.is_error())
                    {
                        return ERR_APPEND(eval_res.get_error(), "cannot check for early abort: failed to evaluate Boolean function");
                    }

                    if (eval_res.get().front() != BooleanFunction::Value::ZERO)
                    {
                        non_zero_count++;
                    }

                    if (non_zero_count != 0)
                    {
                        // early abort
                        return OK({});
                    }
                }
            }
            else if (candidate.m_candidate_type == CandidateType::counter || candidate.m_candidate_type == CandidateType::absolute)
            {
                if (candidate.m_output_nets.size() > candidate.m_input_nets.size())
                {
                    return OK({candidate});
                }

                const u32 diff = candidate.m_input_nets.size() - candidate.m_output_nets.size();
                if (diff >= candidate.m_output_nets.size())
                {
                    return OK({});
                }
            }

            return OK({candidate});
        }

        namespace
        {
            std::vector<std::vector<std::vector<Net*>>> generate_operand_permutations(const std::vector<std::vector<Net*>>& operands, const u32 bit_position)
            {
                std::vector<Net*> sign_bit_signals;
                std::vector<u32> op_indices;

                for (u32 op_idx = 0; op_idx < operands.size(); op_idx++)
                {
                    const auto& op = operands.at(op_idx);
                    if (bit_position < op.size())
                    {
                        sign_bit_signals.push_back(op.at(bit_position));
                        op_indices.push_back(op_idx);
                    }
                }

                if (op_indices.size() == 1)
                {
                    return {operands};
                }

                std::vector<std::vector<std::vector<Net*>>> permuted_operands;

                do
                {
                    std::vector<std::vector<Net*>> ops = operands;

                    for (u32 i = 0; i < op_indices.size(); i++)
                    {
                        const auto op_idx = op_indices.at(i);

                        ops.at(op_idx).at(bit_position) = sign_bit_signals.at(i);
                    }

                    permuted_operands.push_back(ops);

                } while (std::next_permutation(op_indices.begin(), op_indices.end()));

                // std::cout << "Found " << op_indices.size() << " operands that contain bit position " << bit_position << std::endl;
                // std::cout << "Generated " << permuted_operands.size() << " permuted varianst." << std::endl;

                return permuted_operands;
            }
        }    // namespace

        namespace
        {
            std::vector<Net*> sign_extend_operand(const std::vector<Net*>& operand, const u32 new_size, Net* sign_net = nullptr)
            {
                std::vector<Net*> new_operand;

                auto sn = sign_net == nullptr ? operand.back() : sign_net;

                for (u32 idx = 0; idx < new_size; idx++)
                {
                    if (idx < operand.size())
                    {
                        new_operand.push_back(operand.at(idx));
                    }
                    else
                    {
                        new_operand.push_back(sn);
                    }
                }

                return new_operand;
            }

            std::vector<Net*> zero_extend_operand(const std::vector<Net*>& operand, const u32 new_size)
            {
                std::vector<Net*> new_operand;

                for (u32 idx = 0; idx < new_size; idx++)
                {
                    if (idx < operand.size())
                    {
                        new_operand.push_back(operand.at(idx));
                    }
                    else
                    {
                        new_operand.push_back(operand.front()->get_netlist()->get_gnd_nets().front());
                    }
                }

                return new_operand;
            }

            std::vector<Net*> apply_extension(const std::vector<Net*>& op, const u32 size, const u32 extension_type, Net* sign_net)
            {
                std::vector<Net*> new_op = op;

                switch (extension_type)
                {
                    case 0:
                        // zero extended
                        new_op = zero_extend_operand(new_op, size);
                        break;
                    case 1:
                        // sign extended
                        new_op = sign_extend_operand(new_op, size);
                        break;
                    case 2:
                        // sign extended up until the second highest bit
                        new_op = sign_extend_operand(new_op, size - 1, sign_net);
                        new_op = zero_extend_operand(new_op, size);
                        break;
                    case 3:
                        if ((op.size() == size) && (op.back() == sign_net))
                        {
                            new_op = op;
                        }
                        else
                        {
                            new_op = zero_extend_operand(new_op, size);
                        }
                }

                return new_op;
            }
        }    // namespace

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::create_sign_bit_variants(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            std::set<u32> sign_bit_positions;
            for (const auto& op : candidate.m_operands)
            {
                sign_bit_positions.insert(op.size() - 1);
            }

            std::vector<std::vector<std::vector<Net*>>> permuted_operands = {candidate.m_operands};

            for (const auto& sbp : sign_bit_positions)
            {
                std::vector<std::vector<std::vector<Net*>>> new_permuted_operands;
                for (const auto& permuted_op : permuted_operands)
                {
                    const auto new_temp = generate_operand_permutations(permuted_op, sbp);
                    new_permuted_operands.insert(new_permuted_operands.end(), new_temp.begin(), new_temp.end());
                }
                permuted_operands = new_permuted_operands;
            }

            for (const auto& op_set : permuted_operands)
            {
                auto permuted_candidate       = FunctionalCandidate(candidate);
                permuted_candidate.m_operands = op_set;
                new_candidates.push_back(permuted_candidate);
            }

            return OK(new_candidates);
        }

        namespace
        {
            std::vector<std::vector<u32>> combinations_with_repetittions(const std::vector<u32>& v, const std::vector<u32>& stack, const u32 k)
            {
                if (k == 0)
                {
                    return {stack};
                }

                std::vector<std::vector<u32>> result;
                for (u32 i = 0; i < v.size(); i++)
                {
                    auto new_stack = stack;
                    new_stack.push_back(v.at(i));

                    const auto new_combinations = combinations_with_repetittions(v, new_stack, k - 1);
                    result.insert(result.end(), new_combinations.begin(), new_combinations.end());
                }

                return result;
            }
        }    // namespace

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::create_input_extension_variants(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            std::vector<u32> possible_extensions;
            if (candidate.m_candidate_type == module_identification::CandidateType::constant_multiplication)
            {
                possible_extensions = {0, 1, 2};
            }
            else if (candidate.m_operands.size() == 1)
            {
                if (candidate.m_operands.front().size() < (candidate.m_output_nets.size() - 1))
                {
                    possible_extensions = {0, 1, 2};
                }
                else if (candidate.m_operands.front().size() < candidate.m_output_nets.size())
                {
                    possible_extensions = {0, 1};
                }
                else
                {
                    possible_extensions = {};
                }
            }
            else
            {
                possible_extensions = {0, 1, 2};
            }

            std::vector<std::vector<u32>> extension_sets;

            if (candidate.m_candidate_type == module_identification::CandidateType::constant_multiplication)
            {
                extension_sets = combinations_with_repetittions(possible_extensions, {}, candidate.m_operands.size());
            }
            else
            {
                for (const auto& pe : possible_extensions)
                {
                    extension_sets.push_back(std::vector<u32>(candidate.m_operands.size(), pe));
                }
            }

            if (extension_sets.empty())
            {
                const auto new_candidate = FunctionalCandidate(candidate);
                return OK({new_candidate});
            }

            std::set<u32> operand_lengths;
            for (const auto& op : candidate.m_operands)
            {
                operand_lengths.insert(op.size());
            }

            // NOTE: this is currently disabled because in the case that the lower bits of the outputs are cut off we need other ways to reconstruct a bitorder for the input operand as well.
            // if the amount of outputs is shorter than the largest input operand we add more possible output lengths. In the check we then extract only a Slice with the size of the original output
            const std::set<u32> possible_output_sizes = {
                u32(candidate.m_output_nets.size()),
                // std::max(max_operand_length, u32(candidate.m_output_nets.size())),
                // std::max(max_operand_length + 1, u32(candidate.m_output_nets.size())),
            };

            for (const auto& ex_s : extension_sets)
            {
                for (const auto& out_size : possible_output_sizes)
                {
                    auto new_candidate = FunctionalCandidate(candidate);
                    for (u32 op_idx = 0; op_idx < new_candidate.m_operands.size(); op_idx++)
                    {
                        if (candidate.m_candidate_type == module_identification::CandidateType::constant_multiplication)
                        {
                            // new_candidate.m_operands.at(op_idx) = apply_extension_const_mul(new_candidate.m_operands.at(op_idx), new_candidate.m_output_nets.size(), ex_s.at(op_idx), msb);
                            auto sign_net                       = candidate.m_operands.front().back();
                            new_candidate.m_operands.at(op_idx) = apply_extension(new_candidate.m_operands.at(op_idx), out_size, ex_s.at(op_idx), sign_net);
                        }
                        else
                        {
                            auto sign_net                       = new_candidate.m_operands.at(op_idx).back();
                            new_candidate.m_operands.at(op_idx) = apply_extension(new_candidate.m_operands.at(op_idx), out_size, ex_s.at(op_idx), sign_net);
                        }
                    }

                    // check for uniqueness to avoid duplicates
                    bool is_unique = true;
                    for (const auto& nc : new_candidates)
                    {
                        if (nc.m_operands == new_candidate.m_operands)
                        {
                            is_unique = false;
                            break;
                        }
                    }

                    if (is_unique)
                    {
                        new_candidates.push_back(new_candidate);
                    }
                }
            }

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::create_output_net_variant(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;
            new_candidates.emplace_back(candidate);

            const auto extended_output_nets = get_output_nets(candidate.m_gates, false);
            if (candidate.m_output_nets.size() != extended_output_nets.size())
            {
                const u32 difference = extended_output_nets.size() - candidate.m_output_nets.size();
                const u32 threshold  = 3;
                if (difference <= threshold)
                {
                    auto new_candidate          = FunctionalCandidate(candidate);
                    new_candidate.m_output_nets = extended_output_nets;

                    new_candidates.push_back(std::move(new_candidate));
                }
            }

            return OK(new_candidates);
        }

        //value check

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::check_output_size(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            FunctionalCandidate new_candidate = {candidate};

            if (candidate.m_output_nets.size() == 1)
            {
                new_candidates.emplace_back(new_candidate);
            }
            else
            {
                // check for output nets that only lead to outside gates
                std::vector<Net*> filtered_output_nets = get_output_nets(new_candidate.m_gates, true);

                if (filtered_output_nets.size() == 1)
                {
                    new_candidate.m_output_nets = filtered_output_nets;
                    new_candidates.push_back(new_candidate);
                }
            }

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::build_input_operand(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);
            std::vector<FunctionalCandidate> new_candidates;

            FunctionalCandidate new_candidate = {candidate};

            new_candidate.m_operands.push_back({});
            for (const auto& n : new_candidate.m_input_nets)
            {
                new_candidate.m_operands.front().push_back(n);
            }

            new_candidates.emplace_back(new_candidate);

            return OK(new_candidates);
        }

        // constant multiplication

        FunctionalCandidate FunctionalCandidate::add_n_shifted_operands(const FunctionalCandidate& candidate, const std::vector<i32>& shift_vals)
        {
            auto new_operands = candidate.m_operands;

            for (const auto& n : shift_vals)
            {
                // add n shifted operand
                if (n < 0)
                {
                    // shift right
                    const u32 n_abs = abs(n);
                    if (n_abs < candidate.m_operands.at(0).size())
                    {
                        // clone and shift input operand
                        std::vector<Net*> new_operand;
                        for (u32 idx = 0; idx < new_operands.at(0).size() - n_abs; idx++)
                        {
                            new_operand.push_back(new_operands.at(0).at(idx + n_abs));
                        }
                        new_operands.push_back(new_operand);
                    }
                }
                else
                {
                    // shift left
                    std::vector<Net*> new_operand;
                    for (u32 i = 0; i < (u32)n; i++)
                    {
                        new_operand.push_back(candidate.m_gates.front()->get_netlist()->get_gnd_nets().front());
                    }
                    for (const auto& net : new_operands.at(0))
                    {
                        new_operand.push_back(net);
                    }
                    new_operands.push_back(new_operand);
                }
            }

            auto new_candidate       = FunctionalCandidate(candidate);
            new_candidate.m_operands = new_operands;

            return new_candidate;
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::add_shifted_operand(CandidateContext& ctx, const FunctionalCandidate& candidate)
        {
            UNUSED(ctx);

            std::vector<FunctionalCandidate> new_candidates;

            static const std::map<std::vector<std::vector<u32>>, std::vector<std::vector<i32>>> finger_print_library = {
                {{{0, 4, 5}, {1, 6}, {2, 7}}, {{-5, -4}}},
                {{{0, 3, 5}, {1, 4, 6}, {2, 7}}, {{-5, -3}}},
                {{{0, 2, 5}, {1, 3, 6}, {4, 7}}, {{-5, -2}}},
                {{{0, 1, 5}, {2, 6}, {3, 7}}, {{-5, -1}}},
                {{{0, 5}, {1, 6}, {2, 7}}, {{-5}, {-5, 1}, {-5, 2}, {-5, 3}, {-5, 4}, {-5, 5}, {5}}},
                {{{0, 3, 4}, {1, 5}, {2, 6}}, {{-4, -3}}},
                {{{0, 2, 4}, {1, 3, 5}, {6}}, {{-4, -2}}},
                {{{0, 1, 4}, {2, 5}, {3, 6}}, {{-4, -1}}},
                {{{0, 4}, {1, 5}, {2, 6}}, {{-4}, {-4, 1}, {-4, 2}, {-4, 3}, {-4, 4}, {-4, 5}, {4}, {4, 5}}},
                {{{0, 2, 3}, {1, 4}, {5}}, {{-3, -2}}},
                {{{0, 1, 3}, {2, 4}, {5}}, {{-3, -1}}},
                {{{0, 3}, {1, 4}, {2, 5}}, {{-3}, {-3, 1}, {-3, 2}, {-3, 3}, {-3, 4}, {-3, 5}, {3}, {3, 4}, {3, 5}}},
                {{{0, 1, 2}, {3}, {4}}, {{-2, -1}}},
                {{{0, 2}, {1, 3}, {4}}, {{-2}, {-2, 1}, {-2, 2}, {-2, 3}, {-2, 4}, {-2, 5}, {2}, {2, 3}, {2, 4}, {2, 5}}},
                {{{0, 1}, {2}, {3}}, {{-1}, {-1, 1}, {-1, 2}, {-1, 3}, {-1, 4}, {-1, 5}, {1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}}}};

            // generate new influence_count with single inputs ignored
            std::vector<std::set<hal::Net*>> variable_nets;
            for (u32 o_idx = 0; o_idx < candidate.m_output_nets.size(); o_idx++)
            {
                const auto& n                = candidate.m_output_nets.at(o_idx);
                const auto variable_nets_res = ctx.get_variable_nets(n, candidate.m_control_mapping);
                if (variable_nets_res.is_error())
                {
                    return ERR_APPEND(variable_nets_res.get_error(),
                                      "cannot update input output stats: failed to retrieve input variable nets for net " + n->get_name() + " with ID " + std::to_string(n->get_id()));
                }

                variable_nets.push_back(variable_nets_res.get());
            }

            // INPUTS
            std::map<Net*, u32> input_nets_to_influenced_outputs;
            for (u32 o_idx = 0; o_idx < candidate.m_output_nets.size(); o_idx++)
            {
                if (variable_nets.size() <= 1)
                {
                    continue;
                }

                for (const auto& input_net : variable_nets.at(o_idx))
                {
                    input_nets_to_influenced_outputs[input_net] += 1;
                }
            }

            // sort input variables into bins
            std::map<u32, std::vector<Net*>> influence_count_to_input_nets;
            for (const auto& [var, count] : input_nets_to_influenced_outputs)
            {
                influence_count_to_input_nets[count].push_back(var);
            }

            if (influence_count_to_input_nets.size() < 3)
            {
                return OK(new_candidates);
            }

            // the finger print of a constant multiplication are the 3 bins of input nets with the highest amount of output bits influenced.
            // the bins are translated into the corresponding net indices inside the input operands
            std::vector<std::vector<u32>> finger_print;
            auto rit = influence_count_to_input_nets.rbegin();
            for (u32 i = 0; i < 3; i++, rit++)
            {
                const auto& [cnt, nets] = *rit;
                // this is a vector representing the net indices/position in the input operand of this candidate
                std::vector<u32> net_indices;
                for (const auto& n : nets)
                {
                    const auto& op  = candidate.m_operands.front();
                    const auto f_it = std::find(op.begin(), op.end(), n);
                    if (f_it == op.end())
                    {
                        // TODO check why this happens
                        return OK({});
                    }
                    const u32 index = f_it - op.begin();
                    net_indices.push_back(index);
                }

                std::sort(net_indices.begin(), net_indices.end());
                finger_print.push_back(net_indices);
            }

            if (const auto fpl_it = finger_print_library.find(finger_print); fpl_it != finger_print_library.end())
            {
                for (const auto& offsets : fpl_it->second)
                {
                    auto new_candidate_nm = add_n_shifted_operands(candidate, offsets);
                    new_candidate_nm.add_additional_data("OPERAND_SHIFTS", utils::join(", ", offsets));
                    new_candidates.push_back(new_candidate_nm);

                    // add candidate with buffered nets, but this makes only sense if we shifted left
                    if (!new_candidate_nm.m_single_input_to_output.empty())
                    {
                        auto buffered_new_candidate_nm = new_candidate_nm;
                        buffered_new_candidate_nm.add_additional_data("OPERAND_SHIFTS", utils::join(", ", offsets));

                        // add buffered signals to output
                        for (i32 i = 0; i < offsets.back(); i++)
                        {
                            if (i >= (i32)buffered_new_candidate_nm.m_operands.front().size())
                            {
                                continue;
                            }

                            Net* new_o_net = buffered_new_candidate_nm.m_operands.front().at(i);
                            if (const auto it = buffered_new_candidate_nm.m_single_input_to_output.find(new_o_net); it != buffered_new_candidate_nm.m_single_input_to_output.end())
                            {
                                new_o_net = it->second;
                            }
                            buffered_new_candidate_nm.m_output_nets.insert(buffered_new_candidate_nm.m_output_nets.begin() + i, new_o_net);
                        }
                        new_candidates.push_back(buffered_new_candidate_nm);
                    }
                }
            }

            return OK(new_candidates);
        }

        Result<std::vector<FunctionalCandidate>> FunctionalCandidate::create_candidates(StructuralCandidate* sc,
                                                                                        u32 max_control_signal,
                                                                                        CandidateContext& ctx,
                                                                                        module_identification::CandidateType candidate_type,
                                                                                        const std::vector<std::vector<Gate*>>& registers)
        {
            auto gates = sc->m_gates;
            std::vector<FunctionalCandidate> candidates;

            candidates.push_back(FunctionalCandidate(sc, max_control_signal, candidate_type));

            std::vector<std::function<Result<std::vector<FunctionalCandidate>>(CandidateContext & ctx, const FunctionalCandidate&)>> operations;
            switch (candidate_type)
            {
                case module_identification::CandidateType::equal:
                    operations = {
                        trim_to_single_output_net,
                        discard_equal_candidate,
                    };
                    break;
                case module_identification::CandidateType::less_equal:
                    operations = {
                        trim_to_single_output_net,
                        find_control_signals,
                        realize_control_signals,
                        create_sign_extension_variants,
                        order_input_operands,
                        create_sign_bit_variants,
                    };
                    break;
                case module_identification::CandidateType::adder:
                    operations = {
                        create_output_net_variant,
                        update_input_output_stats,
                        identify_control_signals,
                        realize_control_signals,
                        early_abort,
                        update_input_output_stats,
                        permute_single_input_signals,
                        build_input_operands,
                        order_output_signals,
                        add_single_input_signals,
                        create_sign_bit_variants,
                        create_input_extension_variants,
                        [registers](CandidateContext& c_ctx, const FunctionalCandidate& fc) -> Result<std::vector<FunctionalCandidate>> {
                            UNUSED(c_ctx);
                            const auto reordered_operands = reorder_commutative_operands(fc.m_operands, registers);
                            auto new_candidate            = FunctionalCandidate(fc);
                            new_candidate.m_operands      = reordered_operands;
                            return OK({fc, new_candidate});
                        },
                    };
                    break;
                case module_identification::CandidateType::value_check:
                    operations = {
                        check_output_size,
                        build_input_operand,
                    };
                    break;
                case module_identification::CandidateType::constant_multiplication:
                    operations = {
                        update_input_output_stats,
                        early_abort,
                        build_input_operands,
                        order_output_signals,
                        add_shifted_operand,
                        create_input_extension_variants,
                    };
                    break;
                case module_identification::CandidateType::counter:
                    operations = {
                        create_output_net_variant,
                        early_abort,
                        update_input_output_stats,
                        identify_control_signals,
                        realize_control_signals,
                        update_input_output_stats,
                        permute_single_input_signals,
                        build_input_operands,
                        order_output_signals,
                        add_single_input_signals,
                        create_input_extension_variants,
                    };
                    break;
                case module_identification::CandidateType::absolute:
                    operations = {
                        create_output_net_variant,
                        early_abort,
                        update_input_output_stats,
                        identify_control_signals,
                        create_operand_control_variations,
                        realize_control_signals,
                        update_input_output_stats,
                        permute_single_input_signals,
                        build_input_operands,
                        order_output_signals,
                        add_single_input_signals,
                        create_input_extension_variants,

                    };
                    break;
                default:
                    return ERR("no preprocessing available for candidate type" + enum_to_string(candidate_type));
            }

                // #define PRINT_OPS

#ifdef PRINT_OPS
            std::cout << "------------------------" << std::endl;
            std::cout << "Building candidates for carry " << gates.front()->get_name() << " and type: " << candidate_type << std::endl;
            std::cout << "------------------------" << std::endl;
#endif

            for (u32 op_idx = 0; op_idx < operations.size(); op_idx++)
            {
#ifdef PRINT_OPS
                std::cout << "------------------------" << std::endl;
                std::cout << "Excecuting operation " << sc->m_gates.front()->get_name() << " / " << sc->m_gates.front()->get_id() << ": " << op_idx << std::endl;
                std::cout << "------------------------" << std::endl;
#endif
                std::vector<FunctionalCandidate> new_candidates;
                for (const auto& current_candidate : candidates)
                {
                    auto resulting_candidates_res = operations.at(op_idx)(ctx, current_candidate);
                    if (resulting_candidates_res.is_error())
                    {
                        return ERR_APPEND(resulting_candidates_res.get_error(), ("failed functional candidate creation in operation with index " + std::to_string(op_idx)));
                    }
                    auto resulting_candidates = resulting_candidates_res.get();

                    for (auto& rc : resulting_candidates)
                    {
                        new_candidates.push_back(rc);
                    }
                }

#ifdef PRINT_OPS
                std::cout << "------------------------" << std::endl;
                std::cout << "New Candidates:  " << sc->m_gates.front()->get_name() << " / " << sc->m_gates.front()->get_id() << ": " << new_candidates.size() << std::endl;
                std::cout << "------------------------" << std::endl;
#endif

                candidates = new_candidates;

                if (candidates.empty())
                {
                    break;
                }
            }

#ifdef PRINT_OPS
            std::cout << "------------------------" << std::endl;
            std::cout << "Candidates for type " << sc->m_gates.front()->get_name() << " / " << sc->m_gates.front()->get_id() << ": " << candidate_type << std::endl;
            std::cout << "Total Candidates: " << candidates.size() << std::endl;
            std::cout << "------------------------" << std::endl;
#endif

            return OK(candidates);
        }
    }    // namespace module_identification
}    // namespace hal