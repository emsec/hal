#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_preprocessing/netlist_preprocessing.h"
#include "nlohmann_json/json.hpp"
#include "z3_utils/netlist_comparison.h"

#include <queue>

namespace hal
{
    namespace netlist_preprocessing
    {
        Result<u32> remove_unused_lut_inputs(Netlist* nl)
        {
            u32 num_eps = 0;

            // get net connected to GND
            const std::vector<Gate*>& gnd_gates = nl->get_gnd_gates();
            if (gnd_gates.empty())
            {
                return ERR("could not remove unused LUT endpoints from netlist with ID " + std::to_string(nl->get_id()) + ": no GND net available within netlist");
            }
            Net* gnd_net = gnd_gates.front()->get_fan_out_nets().front();

            // iterate all LUT gates
            for (const auto& gate : nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
            {
                std::vector<Endpoint*> fan_in                              = gate->get_fan_in_endpoints();
                std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();

                // skip if more than one function
                if (functions.size() != 1)
                {
                    continue;
                }

                // only pins used as variables in Boolean function are considered active
                auto active_pins = functions.begin()->second.get_variable_names();

                // if there are more fan-in nets than there are active pins, remove those that are not used within the Boolean function and reconnect to GND
                if (fan_in.size() > active_pins.size())
                {
                    for (const auto& ep : fan_in)
                    {
                        if (ep->get_net()->is_gnd_net() || ep->get_net()->is_vcc_net())
                        {
                            continue;
                        }

                        if (std::find(active_pins.begin(), active_pins.end(), ep->get_pin()->get_name()) == active_pins.end())
                        {
                            GatePin* pin = ep->get_pin();
                            if (!ep->get_net()->remove_destination(gate, pin))
                            {
                                log_warning(
                                    "netlist_preprocessing", "failed to remove unused input from LUT gate '{}' with ID {} from netlist with ID {}.", gate->get_name(), gate->get_id(), nl->get_id());
                                continue;
                            }
                            if (!gnd_net->add_destination(gate, pin))
                            {
                                log_warning("netlist_preprocessing",
                                            "failed to reconnect unused input of LUT gate '{}' with ID {} to GND in netlist with ID {}.",
                                            gate->get_name(),
                                            gate->get_id(),
                                            nl->get_id());
                                continue;
                            }
                            num_eps++;
                        }
                    }
                }
            }

            log_info("netlist_preprocessing", "removed {} unused LUT endpoints from netlist with ID {}.", num_eps, nl->get_id());
            return OK(num_eps);
        }

        // TODO make this check every pin of a gate and check whether the generated boolean function (with replaced gnd and vcc nets) is just a variable.
        //      Afterwards just connect input net to buffer destination. Do this for all pins and delete gate if it has no more successors and not global outputs
        Result<u32> remove_buffers(Netlist* nl)
        {
            u32 num_gates = 0;

            std::queue<Gate*> gates_to_be_deleted;

            for (const auto& gate : nl->get_gates())
            {
                std::vector<Endpoint*> fan_out = gate->get_fan_out_endpoints();

                GateType* gt = gate->get_type();

                // continue if of invalid base type
                if (!gt->has_property(GateTypeProperty::combinational) || gt->has_property(GateTypeProperty::power) || gt->has_property(GateTypeProperty::ground))
                {
                    continue;
                }

                // TODO check if multiple outputs but just one connected
                // continue if more than one fan-out net
                if (fan_out.size() != 1)
                {
                    continue;
                }

                // TODO multiple Boolean functions okay if just single one is used (see TODO above)
                // continue if more than one Boolean function
                std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();
                if (functions.size() != 1)
                {
                    continue;
                }

                // continue if Boolean function name does not match output pin
                Endpoint* out_endpoint = *(fan_out.begin());
                if (out_endpoint->get_pin()->get_name() != (functions.begin())->first)
                {
                    continue;
                }

                std::vector<Endpoint*> fan_in = gate->get_fan_in_endpoints();
                BooleanFunction func          = functions.begin()->second;

                // simplify Boolean function for constant 0 or 1 inputs (takes care of, e.g., an AND2 connected to an input and logic '1')
                const auto substitute_res = BooleanFunctionDecorator(func).substitute_power_ground_pins(gate);
                if (substitute_res.is_error())
                {
                    return ERR_APPEND(substitute_res.get_error(),
                                      "Cannot replace buffers: failed to substitute pins with constants at gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()));
                }

                func = substitute_res.get().simplify_local();

                bool failed                      = false;
                std::vector<std::string> in_pins = gt->get_input_pin_names();
                if (func.is_variable() && std::find(in_pins.begin(), in_pins.end(), func.get_variable_name().get()) != in_pins.end())
                {
                    Net* out_net = out_endpoint->get_net();

                    // check all input endpoints and ...
                    for (Endpoint* in_endpoint : fan_in)
                    {
                        Net* in_net = in_endpoint->get_net();
                        if (in_endpoint->get_pin()->get_name() == func.get_variable_name().get())
                        {
                            // const auto merge_res = netlist_utils::merge_nets(nl, in_net, out_net, true);
                            const auto merge_res = NetlistModificationDecorator(*nl).connect_nets(out_net, in_net);
                            if (merge_res.is_error())
                            {
                                log_warning("netlist_preprocessing", "{}", merge_res.get_error().get());
                                failed = true;
                            }
                        }
                        else
                        {
                            // completely remove the input endpoint otherwise
                            if (!in_net->remove_destination(in_endpoint))
                            {
                                log_warning("netlist_preprocessing",
                                            "failed to remove destination from input net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                            in_net->get_name(),
                                            in_net->get_id(),
                                            gate->get_name(),
                                            gate->get_id(),
                                            nl->get_id());
                                failed = true;
                            }
                        }

                        if (failed)
                        {
                            break;
                        }
                    }

                    if (!failed)
                    {
                        gates_to_be_deleted.push(gate);
                    }
                }
            }

            log_debug("netlist_preprocessing", "removing {} buffer gates...", gates_to_be_deleted.size());

            while (!gates_to_be_deleted.empty())
            {
                Gate* gate = gates_to_be_deleted.front();
                gates_to_be_deleted.pop();
                if (!nl->delete_gate(gate))
                {
                    log_warning("netlist_preprocessing", "failed to remove buffer gate '{}' with ID {} from netlist with ID {}.", gate->get_name(), gate->get_id(), nl->get_id());
                    continue;
                }
                num_gates++;
            }

            log_info("netlist_preprocessing", "removed {} buffer gates from netlist with ID {}.", num_gates, nl->get_id());
            return OK(num_gates);
        }

        namespace
        {
            std::unordered_map<Gate*, std::vector<std::string>> restore_ff_replacements(const Netlist* nl)
            {
                std::unordered_map<Gate*, std::vector<std::string>> replacements;

                for (auto& g : nl->get_gates())
                {
                    if (g->has_data("preprocessing_information", "replaced_gates"))
                    {
                        const auto& [_, s]                           = g->get_data("preprocessing_information", "replaced_gates");
                        std::vector<std::string> replaced_gate_names = nlohmann::json::parse(s);
                        replacements.insert({g, replaced_gate_names});
                    }
                }

                return replacements;
            }

            void update_ff_replacements(std::unordered_map<Gate*, std::vector<std::string>>& replacements)
            {
                for (auto& [g, r] : replacements)
                {
                    const nlohmann::json j = r;
                    const std::string s    = j.dump();

                    g->set_data("preprocessing_information", "replaced_gates", "string", s);
                }

                return;
            }

            void annotate_ff_survivor(std::unordered_map<Gate*, std::vector<std::string>>& replacements, Gate* survivor, Gate* to_be_replaced)
            {
                auto& it_s = replacements[survivor];

                if (const auto& it = replacements.find(to_be_replaced); it != replacements.end())
                {
                    for (const auto& s : it->second)
                    {
                        it_s.push_back(s);
                    }
                    replacements.erase(it);
                }

                it_s.push_back(to_be_replaced->get_name());

                return;
            }
        }    // namespace

        Result<u32> remove_redundant_gates(Netlist* nl, const std::function<bool(const Gate*)>& filter)
        {
            auto config = hal::SMT::QueryConfig();

#ifdef BITWUZLA_LIBRARY
            auto s_type = hal::SMT::SolverType::Bitwuzla;
            auto s_call = hal::SMT::SolverCall::Library;
            config      = config.with_solver(s_type).with_call(s_call);
#endif
            struct GateFingerprint
            {
                const GateType* type;
                std::map<GatePin*, Net*> ordered_fan_in = {};
                std::set<Net*> unordered_fan_in         = {};
                u8 truth_table_hw                       = 0;

                bool operator<(const GateFingerprint& other) const
                {
                    return (other.type < type) || (other.type == type && other.ordered_fan_in < ordered_fan_in)
                           || (other.type == type && other.ordered_fan_in == ordered_fan_in && other.unordered_fan_in < unordered_fan_in)
                           || (other.type == type && other.ordered_fan_in == ordered_fan_in && other.unordered_fan_in == unordered_fan_in && other.truth_table_hw < truth_table_hw);
                }
            };

            static std::vector<u8> hw_map = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

            u32 num_gates = 0;
            bool progress;

            std::vector<Gate*> target_gates;
            if (filter)
            {
                target_gates = nl->get_gates([filter](const Gate* g) {
                    const auto& type = g->get_type();
                    return (type->has_property(GateTypeProperty::combinational) || type->has_property(GateTypeProperty::ff)) && filter(g);
                });
            }
            else
            {
                target_gates = nl->get_gates([](const Gate* g) {
                    const auto& type = g->get_type();
                    return type->has_property(GateTypeProperty::combinational) || type->has_property(GateTypeProperty::ff);
                });
            }

            auto ff_replacements = restore_ff_replacements(nl);

            do
            {
                std::map<GateFingerprint, std::vector<Gate*>> fingerprinted_gates;

                progress = false;

                for (auto* gate : target_gates)
                {
                    GateFingerprint fingerprint;
                    fingerprint.type = gate->get_type();
                    if (fingerprint.type->has_property(GateTypeProperty::combinational))
                    {
                        const auto& fan_in_nets = gate->get_fan_in_nets();
                        fingerprint.unordered_fan_in.insert(fan_in_nets.cbegin(), fan_in_nets.cend());
                        if (fingerprint.type->has_property(GateTypeProperty::c_lut))
                        {
                            if (const auto res = gate->get_init_data(); res.is_ok())
                            {
                                const auto& init_str = res.get().front();
                                for (const auto c : init_str)
                                {
                                    u8 tmp = std::toupper(c) - 0x30;
                                    if (tmp > 9)
                                    {
                                        tmp -= 0x7;
                                    }
                                    fingerprint.truth_table_hw += hw_map.at(tmp);
                                }
                            }
                        }
                    }
                    else if (fingerprint.type->has_property(GateTypeProperty::ff))
                    {
                        for (const auto& ep : gate->get_fan_in_endpoints())
                        {
                            fingerprint.ordered_fan_in[ep->get_pin()] = ep->get_net();
                        }
                    }

                    fingerprinted_gates[fingerprint].push_back(gate);
                }

                std::vector<std::vector<Gate*>> duplicate_gates;
                for (const auto& [fingerprint, gates] : fingerprinted_gates)
                {
                    if (gates.size() == 1)
                    {
                        continue;
                    }

                    if (fingerprint.type->has_property(GateTypeProperty::combinational))
                    {
                        std::set<const Gate*> visited;
                        for (size_t i = 0; i < gates.size(); i++)
                        {
                            Gate* master_gate = gates.at(i);

                            if (visited.find(master_gate) != visited.cend())
                            {
                                continue;
                            }

                            std::vector<Gate*> current_duplicates = {master_gate};

                            for (size_t j = i + 1; j < gates.size(); j++)
                            {
                                Gate* current_gate = gates.at(j);
                                bool equal         = true;
                                for (const auto* pin : fingerprint.type->get_output_pins())
                                {
                                    const auto solver_res =
                                        master_gate->get_resolved_boolean_function(pin)
                                            .map<BooleanFunction>([pin, current_gate](BooleanFunction&& bf_master) {
                                                return current_gate->get_resolved_boolean_function(pin).map<BooleanFunction>([bf_master = std::move(bf_master)](BooleanFunction&& bf_current) mutable {
                                                    return BooleanFunction::Eq(std::move(bf_master), std::move(bf_current), 1);
                                                });
                                            })
                                            .map<BooleanFunction>([](auto&& bf_eq) -> Result<BooleanFunction> { return BooleanFunction::Not(std::move(bf_eq), 1); })
                                            .map<SMT::SolverResult>([&config](auto&& bf_not) -> Result<SMT::SolverResult> { return SMT::Solver({SMT::Constraint(std::move(bf_not))}).query(config); });

                                    if (solver_res.is_error() || !solver_res.get().is_unsat())
                                    {
                                        equal = false;
                                    }
                                }

                                if (equal)
                                {
                                    current_duplicates.push_back(current_gate);
                                    visited.insert(current_gate);
                                }
                            }

                            if (current_duplicates.size() > 1)
                            {
                                duplicate_gates.push_back(current_duplicates);
                            }
                        }
                    }
                    else if (fingerprint.type->has_property(GateTypeProperty::ff))
                    {
                        duplicate_gates.push_back(std::move(gates));
                    }
                }

                std::set<Gate*> affected_gates;
                for (auto& current_duplicates : duplicate_gates)
                {
                    std::sort(current_duplicates.begin(), current_duplicates.end(), [](const auto& g1, const auto& g2) { return g1->get_name().length() < g2->get_name().length(); });

                    auto* survivor_gate = current_duplicates.front();
                    std::map<GatePin*, Net*> out_pins_to_nets;
                    for (auto* ep : survivor_gate->get_fan_out_endpoints())
                    {
                        Net* out_net                    = ep->get_net();
                        out_pins_to_nets[ep->get_pin()] = out_net;
                        for (const auto* dst : out_net->get_destinations())
                        {
                            auto* dst_gate = dst->get_gate();
                            auto* dst_type = dst_gate->get_type();
                            if (dst_type->has_property(GateTypeProperty::combinational) || dst_type->has_property(GateTypeProperty::ff))
                            {
                                affected_gates.insert(dst_gate);
                            }
                        }
                    }

                    for (u32 k = 1; k < current_duplicates.size(); k++)
                    {
                        auto* current_gate = current_duplicates.at(k);
                        for (auto* ep : current_gate->get_fan_out_endpoints())
                        {
                            auto* ep_net = ep->get_net();
                            auto* ep_pin = ep->get_pin();

                            if (auto it = out_pins_to_nets.find(ep_pin); it != out_pins_to_nets.cend())
                            {
                                // survivor already has net connected to this output -> add destination to survivor's net
                                for (auto* dst : ep_net->get_destinations())
                                {
                                    auto* dst_gate = dst->get_gate();
                                    auto* dst_pin  = dst->get_pin();
                                    dst->get_net()->remove_destination(dst);
                                    it->second->add_destination(dst_gate, dst_pin);

                                    auto* dst_type = dst_gate->get_type();
                                    if (dst_type->has_property(GateTypeProperty::combinational) || dst_type->has_property(GateTypeProperty::ff))
                                    {
                                        affected_gates.insert(dst_gate);
                                    }
                                }
                                if (!nl->delete_net(ep_net))
                                {
                                    log_warning("netlist_preprocessing", "could not delete net '{}' with ID {} from netlist with ID {}.", ep_net->get_name(), ep_net->get_id(), nl->get_id());
                                }
                            }
                            else
                            {
                                // survivor does not feature net on this output pin -> connect this net to survivor
                                ep_net->add_source(survivor_gate, ep_pin);
                                out_pins_to_nets[ep_pin] = ep_net;
                                for (auto* dst : ep_net->get_destinations())
                                {
                                    auto* dst_gate = dst->get_gate();
                                    auto* dst_type = dst_gate->get_type();
                                    if (dst_type->has_property(GateTypeProperty::combinational) || dst_type->has_property(GateTypeProperty::ff))
                                    {
                                        affected_gates.insert(dst_gate);
                                    }
                                }
                            }
                        }

                        annotate_ff_survivor(ff_replacements, survivor_gate, current_gate);

                        affected_gates.erase(current_gate);
                        if (!nl->delete_gate(current_gate))
                        {
                            log_warning("netlist_preprocessing", "could not delete gate '{}' with ID {} from netlist with ID {}.", current_gate->get_name(), current_gate->get_id(), nl->get_id());
                        }
                        else
                        {
                            progress = true;
                            num_gates++;
                        }
                    }
                }
                target_gates = std::vector<Gate*>(affected_gates.cbegin(), affected_gates.cend());
            } while (progress);

            update_ff_replacements(ff_replacements);

            log_info("netlist_preprocessing", "removed {} redundant gates from netlist with ID {}.", num_gates, nl->get_id());
            return OK(num_gates);
        }

        Result<u32> remove_redundant_loops(Netlist* nl)
        {
            struct LoopFingerprint
            {
                std::map<const GateType*, u32> types;
                std::set<std::string> external_variable_names;
                std::set<const Net*> ff_control_nets;

                bool operator<(const LoopFingerprint& other) const
                {
                    return (other.types < types) || (other.types == types && other.external_variable_names < external_variable_names)
                           || (other.types == types && other.external_variable_names == external_variable_names && other.ff_control_nets < ff_control_nets);
                }
            };

            auto config = hal::SMT::QueryConfig();

#ifdef BITWUZLA_LIBRARY
            auto s_type = hal::SMT::SolverType::Bitwuzla;
            auto s_call = hal::SMT::SolverCall::Library;
            config      = config.with_solver(s_type).with_call(s_call);
#endif

            u32 num_gates = 0;

            auto ff_replacements = restore_ff_replacements(nl);

            static const std::set<PinType> ff_control_pin_types = {PinType::clock, PinType::enable, PinType::reset, PinType::set};

            // detect combinational loops that begin and end at the same FF
            // for some FFs, multiple combinational lops may exist; such loops wil be merged into a single one
            std::unordered_map<Gate*, std::unordered_set<Gate*>> loops_by_start_gate;
            for (auto* start_ff : nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::ff); }))
            {
                std::vector<Gate*> stack = {start_ff};
                std::vector<Gate*> previous_gates;
                std::unordered_set<Gate*> visited_gates;
                std::unordered_set<Gate*> cache;

                while (!stack.empty())
                {
                    auto* current_gate = stack.back();

                    if (!previous_gates.empty() && current_gate == previous_gates.back())
                    {
                        stack.pop_back();
                        previous_gates.pop_back();
                        continue;
                    }

                    visited_gates.insert(current_gate);

                    bool added = false;
                    for (const auto* suc_ep : current_gate->get_successors())
                    {
                        if (ff_control_pin_types.find(suc_ep->get_pin()->get_type()) != ff_control_pin_types.end())
                        {
                            continue;
                        }

                        auto* suc_gate = suc_ep->get_gate();
                        if (suc_gate == start_ff || cache.find(suc_gate) != cache.end())
                        {
                            loops_by_start_gate[start_ff].insert(current_gate);
                            cache.insert(current_gate);
                            for (auto it = ++(previous_gates.begin()); it != previous_gates.end(); it++)
                            {
                                cache.insert(*it);
                                loops_by_start_gate[start_ff].insert(*it);
                            }
                        }
                        else if (suc_gate->get_type()->has_property(GateTypeProperty::combinational))
                        {
                            if (visited_gates.find(suc_gate) == visited_gates.end())
                            {
                                stack.push_back(suc_gate);
                                added = true;
                            }
                        }
                    }

                    if (added)
                    {
                        previous_gates.push_back(current_gate);
                    }
                    else
                    {
                        stack.pop_back();
                    }
                }
            }

            std::map<LoopFingerprint, std::vector<std::pair<std::vector<Gate*>, BooleanFunction>>> fingerprinted_loops;
            for (const auto& [start_ff, comb_gates] : loops_by_start_gate)
            {
                LoopFingerprint fingerprint;

                // do not consider loop of more than 30 gates
                if (comb_gates.size() > 30)
                {
                    continue;
                }

                // collect FF control and data nets
                std::vector<const Endpoint*> data_in;
                for (const auto* ep : start_ff->get_fan_in_endpoints())
                {
                    auto pin_type = ep->get_pin()->get_type();
                    if (ff_control_pin_types.find(pin_type) != ff_control_pin_types.end())
                    {
                        fingerprint.ff_control_nets.insert(ep->get_net());
                    }
                    else if (pin_type == PinType::data)
                    {
                        data_in.push_back(ep);
                    }
                }

                if (data_in.size() != 1)
                {
                    continue;
                }

                // collect gate types
                fingerprint.types[start_ff->get_type()] = 1;
                for (const auto* g : comb_gates)
                {
                    const auto* gt = g->get_type();
                    if (const auto type_it = fingerprint.types.find(gt); type_it == fingerprint.types.end())
                    {
                        fingerprint.types[gt] = 0;
                    }
                    fingerprint.types[gt]++;
                }

                std::vector<const Gate*> comb_gates_vec(comb_gates.cbegin(), comb_gates.cend());
                if (auto function_res = SubgraphNetlistDecorator(*nl).get_subgraph_function(comb_gates_vec, data_in.front()->get_net()); function_res.is_ok())
                {
                    // get Boolean function variable names
                    BooleanFunction function            = function_res.get();
                    fingerprint.external_variable_names = function.get_variable_names();

                    // replace FF output net identifier from function variables (otherwise varies depending on FF, preventing later SMT check)
                    for (const auto* ep : start_ff->get_fan_out_endpoints())
                    {
                        if (const auto it = fingerprint.external_variable_names.find(BooleanFunctionNetDecorator(*(ep->get_net())).get_boolean_variable_name());
                            it != fingerprint.external_variable_names.end())
                        {
                            function = function.substitute(*it, ep->get_pin()->get_name());
                            fingerprint.external_variable_names.erase(it);
                        }
                    }

                    std::vector<Gate*> loop_gates = {start_ff};
                    loop_gates.insert(loop_gates.end(), comb_gates.begin(), comb_gates.end());
                    fingerprinted_loops[fingerprint].push_back(std::make_pair(loop_gates, std::move(function)));
                }
            }

            std::vector<std::vector<std::vector<Gate*>>> duplicate_loops;
            for (const auto& [_, loops] : fingerprinted_loops)
            {
                if (loops.size() == 1)
                {
                    continue;
                }

                std::set<u32> visited;
                for (u32 i = 0; i < loops.size(); i++)
                {
                    if (visited.find(i) != visited.cend())
                    {
                        continue;
                    }

                    const auto& master_loop = loops.at(i);

                    std::vector<std::vector<Gate*>> current_duplicates = {std::get<0>(master_loop)};

                    for (size_t j = i + 1; j < loops.size(); j++)
                    {
                        const auto& current_loop = loops.at(j);
                        const auto solver_res =
                            BooleanFunction::Eq(std::get<1>(master_loop).clone(), std::get<1>(current_loop).clone(), 1)
                                .map<BooleanFunction>([](auto&& bf_eq) -> Result<BooleanFunction> { return BooleanFunction::Not(std::move(bf_eq), 1); })
                                .map<SMT::SolverResult>([&config](auto&& bf_not) -> Result<SMT::SolverResult> { return SMT::Solver({SMT::Constraint(std::move(bf_not))}).query(config); });

                        if (solver_res.is_ok() && solver_res.get().is_unsat())
                        {
                            current_duplicates.push_back(std::get<0>(current_loop));
                            visited.insert(j);
                        }
                    }

                    if (current_duplicates.size() > 1)
                    {
                        duplicate_loops.push_back(std::move(current_duplicates));
                    }
                }
            }

            for (const auto& current_duplicates : duplicate_loops)
            {
                // TODO the "replace" logic where the output of the survivor ff is connected to new sources and the old gates are deleted is a duplicate of the above functionality
                const auto& survivor_loop = current_duplicates.front();
                auto* survivor_ff         = survivor_loop.front();

                std::map<GatePin*, Net*> out_pins_to_nets;
                for (auto* ep : survivor_ff->get_fan_out_endpoints())
                {
                    Net* out_net                    = ep->get_net();
                    out_pins_to_nets[ep->get_pin()] = out_net;
                }

                for (u32 i = 1; i < current_duplicates.size(); i++)
                {
                    auto* current_ff = current_duplicates.at(i).front();
                    for (auto* ep : current_ff->get_fan_out_endpoints())
                    {
                        auto* ep_net = ep->get_net();
                        auto* ep_pin = ep->get_pin();

                        if (auto it = out_pins_to_nets.find(ep_pin); it != out_pins_to_nets.cend())
                        {
                            // survivor already has net connected to this output -> add destination to survivor's net
                            for (auto* dst : ep_net->get_destinations())
                            {
                                auto* dst_gate = dst->get_gate();
                                auto* dst_pin  = dst->get_pin();
                                dst->get_net()->remove_destination(dst);
                                it->second->add_destination(dst_gate, dst_pin);
                            }
                            if (!nl->delete_net(ep_net))
                            {
                                log_warning("netlist_preprocessing", "could not delete net '{}' with ID {} from netlist with ID {}.", ep_net->get_name(), ep_net->get_id(), nl->get_id());
                            }
                        }
                        else
                        {
                            // survivor does not feature net on this output pin -> connect this net to survivor
                            ep_net->add_source(survivor_ff, ep_pin);
                            out_pins_to_nets[ep_pin] = ep_net;
                        }
                    }

                    annotate_ff_survivor(ff_replacements, survivor_ff, current_ff);

                    if (!nl->delete_gate(current_ff))
                    {
                        log_warning("netlist_preprocessing", "could not delete gate '{}' with ID {} from netlist with ID {}.", current_ff->get_name(), current_ff->get_id(), nl->get_id());
                    }
                    else
                    {
                        num_gates++;
                    }
                }
            }

            update_ff_replacements(ff_replacements);

            log_info("netlist_preprocessing", "removed {} redundant loops from netlist with ID {}.", num_gates, nl->get_id());
            return OK(num_gates);
        }

        Result<u32> remove_redundant_logic_trees(Netlist* nl)
        {
            struct TreeFingerprint
            {
                std::set<const Net*> external_inputs;
                // std::set<std::string> external_inputs;

                bool operator<(const TreeFingerprint& other) const
                {
                    return (other.external_inputs < external_inputs);
                }
            };

            const std::vector<Gate*> all_comb_gates_vec = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });
            // const std::unordered_set<Gate*> all_comb_gates_set = {all_comb_gates_vec.begin(), all_comb_gates_vec.end()};

            std::map<TreeFingerprint, std::set<Net*>> fingerprint_to_nets;
            for (const auto& g : all_comb_gates_vec)
            {
                for (const auto& out_ep : g->get_fan_out_endpoints())
                {
                    // const auto non_comb_destinations = out_ep->get_net()->get_destinations([](const auto& in_ep){ return !in_ep->get_gate()->get_type()->has_property(GateTypeProperty::combinational);});
                    // if (!non_comb_destinations.empty())
                    {
                        const auto& out_net = out_ep->get_net();
                        auto inputs_res     = SubgraphNetlistDecorator(*nl).get_subgraph_function_inputs(all_comb_gates_vec, out_net);
                        if (inputs_res.is_error())
                        {
                            return ERR_APPEND(inputs_res.get_error(),
                                              "Unable to remove redundant logic trees: failed to gather inputs for net " + out_net->get_name() + " with ID " + std::to_string(out_net->get_id()));
                        }
                        TreeFingerprint tf;
                        tf.external_inputs = inputs_res.get();
                        // tf.external_inputs = SubgraphNetlistDecorator(*nl).get_subgraph_function(all_comb_gates_vec, out_net).get().simplify().get_variable_names();

                        fingerprint_to_nets[tf].insert(out_net);
                    }
                }
            }

            std::vector<std::vector<Net*>> equality_classes;

            for (const auto& [_fingerprint, nets] : fingerprint_to_nets)
            {
                // TODO remove
                // std::cout << "Fingerprint(" << _fingerprint.external_inputs.size() << "): " << std::endl;
                // for (const auto& n : _fingerprint.external_inputs)
                // {
                //     std::cout << "\t" << n << std::endl;
                // }
                // std::cout << "Checking nets: " << std::endl;
                // for (const auto& n : nets)
                // {
                //     std::cout << "\t" << n->get_name() << std::endl;
                // }

                std::vector<Net*> current_candidate_nets = {nets.begin(), nets.end()};
                std::vector<Net*> next_candidate_nets;

                while (!current_candidate_nets.empty())
                {
                    const auto n = current_candidate_nets.back();
                    current_candidate_nets.pop_back();

                    std::vector<Net*> new_equality_class = {n};

                    for (const auto& m : current_candidate_nets)
                    {
                        auto comp_res = z3_utils::compare_nets(nl, nl, n, m);
                        if (comp_res.is_error())
                        {
                            return ERR_APPEND(comp_res.get_error(),
                                              "Unable to remove redundant logic trees: failed to compare net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + " with net "
                                                  + m->get_name() + " with ID " + std::to_string(m->get_id()));
                        }
                        const auto are_equal = comp_res.get();

                        if (are_equal)
                        {
                            new_equality_class.push_back(m);
                        }
                        else
                        {
                            next_candidate_nets.push_back(m);
                        }
                    }

                    equality_classes.push_back(new_equality_class);
                    current_candidate_nets = next_candidate_nets;
                    next_candidate_nets.clear();
                }
            }

            u32 counter = 0;
            for (const auto& eq_class : equality_classes)
            {
                // TODO remove
                // std::cout << "Equal nets: " << std::endl;
                // for (const auto& n : eq_class)
                // {
                //     std::cout << n->get_name() << std::endl;
                // }

                auto survivor_net = eq_class.front();

                for (u32 i = 1; i < eq_class.size(); i++)
                {
                    auto victim_net = eq_class.at(i);
                    for (const auto& dst : victim_net->get_destinations())
                    {
                        auto dst_gate = dst->get_gate();
                        auto dst_pin  = dst->get_pin();

                        if (!victim_net->remove_destination(dst))
                        {
                            return ERR("Unable to remove redundant logic trees: failed to remove destination of net " + victim_net->get_name() + " with ID " + std::to_string(victim_net->get_id())
                                       + " at gate " + dst_gate->get_name() + " with ID " + std::to_string(dst_gate->get_id()) + " and pin " + dst_pin->get_name());
                        }
                        if (!survivor_net->add_destination(dst_gate, dst_pin))
                        {
                            return ERR("Unable to remove redundant logic trees: failed to add destination to net " + survivor_net->get_name() + " with ID " + std::to_string(survivor_net->get_id())
                                       + " at gate " + dst_gate->get_name() + " with ID " + std::to_string(dst_gate->get_id()) + " and pin " + dst_pin->get_name());
                        }

                        counter += 1;
                    }
                }
            }

            auto clean_up_res = remove_unconnected_looped(nl);
            if (clean_up_res.is_error())
            {
                return ERR_APPEND(clean_up_res.get_error(), "Unable to remove redundant logic trees: failed to clean up dangling trees");
            }

            return OK(clean_up_res.get() + counter);
        }

        Result<u32> remove_unconnected_gates(Netlist* nl)
        {
            u32 num_gates = 0;
            std::vector<Gate*> to_delete;
            do
            {
                to_delete.clear();

                for (const auto& g : nl->get_gates())
                {
                    bool is_unconnected = true;
                    for (const auto& on : g->get_fan_out_nets())
                    {
                        if (!on->get_destinations().empty() || on->is_global_output_net())
                        {
                            is_unconnected = false;
                        }
                    }

                    if (is_unconnected)
                    {
                        to_delete.push_back(g);
                    }
                }

                for (const auto& g : to_delete)
                {
                    if (!nl->delete_gate(g))
                    {
                        log_warning("netlist_preprocessing", "could not delete gate '{}' with ID {} from netlist with ID {}.", g->get_name(), g->get_id(), nl->get_id());
                    }
                    else
                    {
                        num_gates++;
                    }
                }
            } while (!to_delete.empty());

            log_info("netlist_preprocessing", "removed {} unconnected gates from netlist with ID {}.", num_gates, nl->get_id());
            return OK(num_gates);
        }

        Result<u32> remove_unconnected_nets(Netlist* nl)
        {
            u32 num_nets = 0;

            std::vector<Net*> to_delete;

            for (const auto& n : nl->get_nets())
            {
                if (!n->is_global_input_net() && n->get_sources().empty() && !n->is_global_output_net() && n->get_destinations().empty())
                {
                    to_delete.push_back(n);
                }
            }

            for (const auto& n : to_delete)
            {
                if (!nl->delete_net(n))
                {
                    log_warning("netlist_preprocessing", "could not delete net '{}' with ID {} from netlist with ID {}.", n->get_name(), n->get_id(), nl->get_id());
                }
                else
                {
                    num_nets++;
                }
            }

            log_info("netlist_preprocessing", "removed {} unconnected nets from netlist with ID {}.", num_nets, nl->get_id());
            return OK(num_nets);
        }

        Result<u32> remove_unconnected_looped(Netlist* nl)
        {
            u32 total_removed = 0;

            while (true)
            {
                auto gate_res = remove_unconnected_gates(nl);
                if (gate_res.is_error())
                {
                    return ERR_APPEND(gate_res.get_error(), "unable to execute clean up loop: failed to remove unconnected gates");
                }

                auto net_res = remove_unconnected_nets(nl);
                if (net_res.is_error())
                {
                    return ERR_APPEND(net_res.get_error(), "unable to execute clean up loop: failed to remove unconnected nets");
                }

                const u32 removed = gate_res.get() + net_res.get();
                total_removed += removed;
                if (!removed)
                {
                    break;
                }
            }

            return OK(total_removed);
        }

        Result<u32> remove_consecutive_inverters(Netlist* nl)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            std::set<Gate*> gates_to_delete;
            for (auto* inv_gate : nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_inverter); }))
            {
                if (gates_to_delete.find(inv_gate) != gates_to_delete.end())
                {
                    continue;
                }

                const auto& connection_endpoints = inv_gate->get_fan_in_endpoints();
                if (connection_endpoints.size() != 1)
                {
                    log_warning("netlist_preprocessing", "could not handle gate '{}' with ID {} due to a fan-in size != 1", inv_gate->get_name(), inv_gate->get_id());
                    continue;
                }

                auto* middle_fan_in_ep = connection_endpoints.front();
                auto* middle_net       = middle_fan_in_ep->get_net();
                if (middle_net->get_sources().size() != 1)
                {
                    log_warning("netlist_preprocessing", "could not handle gate '{}' with ID {} due to a number of predecessors != 1", inv_gate->get_name(), inv_gate->get_id());
                    continue;
                }
                auto* pred_gate = middle_net->get_sources().front()->get_gate();

                if (pred_gate->get_type()->has_property(GateTypeProperty::c_inverter))
                {
                    const auto& fan_in = pred_gate->get_fan_in_endpoints();
                    if (fan_in.size() != 1)
                    {
                        log_warning("netlist_preprocessing", "could not handle gate '{}' with ID {} due to a fan-in size != 1", pred_gate->get_name(), pred_gate->get_id());
                        continue;
                    }
                    if (pred_gate->get_fan_out_endpoints().size() != 1)
                    {
                        log_warning("netlist_preprocessing", "could not handle gate '{}' with ID {} due to a fan-out size != 1", pred_gate->get_name(), pred_gate->get_id());
                        continue;
                    }
                    auto* in_net = fan_in.front()->get_net();

                    const auto& fan_out = inv_gate->get_fan_out_endpoints();
                    if (fan_out.size() != 1)
                    {
                        log_warning("netlist_preprocessing", "could not handle gate '{}' with ID {} due to a fan-out size != 1", inv_gate->get_name(), inv_gate->get_id());
                        continue;
                    }
                    auto* out_net = fan_out.front()->get_net();

                    for (auto* dst_ep : out_net->get_destinations())
                    {
                        auto* dst_pin  = dst_ep->get_pin();
                        auto* dst_gate = dst_ep->get_gate();

                        out_net->remove_destination(dst_ep);
                        in_net->add_destination(dst_gate, dst_pin);
                    }

                    middle_net->remove_destination(middle_fan_in_ep);

                    if (middle_net->get_num_of_destinations() == 0)
                    {
                        nl->delete_net(middle_net);
                        gates_to_delete.insert(pred_gate);
                    }

                    gates_to_delete.insert(inv_gate);
                }
            }

            u32 removed_ctr = 0;
            for (auto* g : gates_to_delete)
            {
                nl->delete_gate(g);
                removed_ctr++;
            }

            return OK(removed_ctr);
        }
    }    // namespace netlist_preprocessing
}    // namespace hal