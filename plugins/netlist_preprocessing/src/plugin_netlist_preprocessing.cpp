#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/result.h"

#include <queue>

namespace hal
{
    namespace
    {
        std::string generate_hex_truth_table_string(const std::vector<BooleanFunction::Value>& tt)
        {
            std::string tt_str = "";

            u32 acc = 0;
            for (u32 i = 0; i < tt.size(); i++)
            {
                const BooleanFunction::Value bit = tt.at(i);
                if (bit == BooleanFunction::Value::ONE)
                {
                    acc += (1 << (i % 4));
                }

                if ((i % 4) == 3)
                {
                    std::stringstream stream;
                    stream << std::hex << acc;

                    tt_str = stream.str() + tt_str;

                    acc = 0;
                }
            }

            return tt_str;
        }
    }    // namespace

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistPreprocessingPlugin>();
    }

    std::string NetlistPreprocessingPlugin::get_name() const
    {
        return std::string("netlist_preprocessing");
    }

    std::string NetlistPreprocessingPlugin::get_version() const
    {
        return std::string("0.1");
    }

    Result<u32> NetlistPreprocessingPlugin::simplify_lut_inits(Netlist* nl)
    {
        u32 num_inits = 0;

        for (auto g : nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
        {
            // TODO remove
            // if (g->get_id() != 1102)
            // {
            //     continue;
            // }

            // skip if the gate type has more than one fan out endpoints
            if (g->get_type()->get_output_pins().size() != 1)
            {
                continue;
            }

            const auto out_ep = g->get_fan_out_endpoints().front();

            // skip if the gate has more than one boolean function
            if (g->get_boolean_functions().size() != 1)
            {
                continue;
            }

            const auto bf_org   = g->get_boolean_function(out_ep->get_pin());
            const auto org_vars = bf_org.get_variable_names();

            const auto bf_replaced   = BooleanFunctionDecorator(bf_org).substitute_power_ground_pins(nl, g).get();
            const auto bf_simplified = bf_replaced.simplify_local();

            const auto new_vars = bf_simplified.get_variable_names();

            if (org_vars.size() == new_vars.size())
            {
                continue;
            }

            auto bf_extended = bf_simplified.clone();
            for (const auto& in_pin : g->get_type()->get_input_pin_names())
            {
                if (new_vars.find(in_pin) == new_vars.end())
                {
                    auto bf_filler = BooleanFunction::Var(in_pin) | (~BooleanFunction::Var(in_pin));
                    bf_extended    = BooleanFunction::And(std::move(bf_extended), std::move(bf_filler), 1).get();
                }
            }

            const auto tt              = bf_extended.compute_truth_table().get();
            const auto new_init_string = generate_hex_truth_table_string(tt.front());

            // std::cout << "Org Init: " << g->get_init_data().get().front() << std::endl;
            // std::cout << "New Init: " << new_init_string << std::endl;

            g->set_init_data({new_init_string}).get();

            const auto bf_test = g->get_boolean_function(out_ep->get_pin());

            // std::cout << "Org: " << bf_org << std::endl;
            // std::cout << "Rep: " << bf_replaced << std::endl;
            // std::cout << "Simp: " << bf_simplified << std::endl;
            // std::cout << "Test: " << bf_test << std::endl;
            // std::cout << "Ext: " << bf_extended << std::endl;

            num_inits++;
        }

        log_info("netlist_preprocessing", "simplified {} LUT INIT strings inside of netlist with ID {}.", num_inits, nl->get_id());
        return OK(num_inits);
    }

    Result<u32> NetlistPreprocessingPlugin::remove_unused_lut_inputs(Netlist* nl)
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
        for (const auto& gate : nl->get_gates([](Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
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
                            log_warning(
                                "netlist_preprocessing", "failed to reconnect unused input of LUT gate '{}' with ID {} to GND in netlist with ID {}.", gate->get_name(), gate->get_id(), nl->get_id());
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

    Result<u32> NetlistPreprocessingPlugin::remove_buffers(Netlist* nl)
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

            // continue if more than one fan-out net
            if (fan_out.size() != 1)
            {
                continue;
            }

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

            // simplify Boolean function for constant 0 or 1 inputs (takes care of, e.g., an AND2 connected to an input and logic 1)
            for (Endpoint* ep : fan_in)
            {
                auto sources = ep->get_net()->get_sources();
                if (sources.size() != 1)
                {
                    break;
                }

                if (sources.front()->get_gate()->is_gnd_gate())
                {
                    if (auto sub_res = func.substitute(ep->get_pin()->get_name(), BooleanFunction::Const(BooleanFunction::ZERO)); sub_res.is_ok())
                    {
                        func = sub_res.get();
                    }
                    else
                    {
                        log_warning("netlist_preprocessing", "{}", sub_res.get_error().get());
                    }
                }
                else if (sources.front()->get_gate()->is_vcc_gate())
                {
                    if (auto sub_res = func.substitute(ep->get_pin()->get_name(), BooleanFunction::Const(BooleanFunction::ONE)); sub_res.is_ok())
                    {
                        func = sub_res.get();
                    }
                    else
                    {
                        log_warning("netlist_preprocessing", "{}", sub_res.get_error().get());
                    }
                }
            }

            func = func.simplify_local();

            bool failed                      = false;
            std::vector<std::string> in_pins = gt->get_input_pin_names();
            if (func.is_variable() && std::find(in_pins.begin(), in_pins.end(), func.get_variable_name().get()) != in_pins.end())
            {
                Net* out_net      = out_endpoint->get_net();
                Net* buffered_net = nullptr;

                bool is_global_output = out_net->is_global_output_net();
                std::map<Module*, std::tuple<std::string, std::string, u32, PinDirection, PinType>> module_pins;

                // check all input endpoints and ...
                for (Endpoint* in_endpoint : fan_in)
                {
                    Net* in_net = in_endpoint->get_net();
                    if (in_endpoint->get_pin()->get_name() == func.get_variable_name().get())
                    {
                        buffered_net = in_net;
                        // safe all module pin information from the net soon to be deleted
                        for (const auto& m : nl->get_modules())
                        {
                            if (const auto pin = m->get_pin_by_net(out_net); pin != nullptr)
                            {
                                module_pins.insert({m, {pin->get_group().first->get_name(), pin->get_name(), pin->get_group().second, pin->get_direction(), pin->get_type()}});
                            }
                        }

                        // ... reconnect outputs if the input is passed through the buffer
                        for (Endpoint* dst : out_net->get_destinations())
                        {
                            Gate* dst_gate   = dst->get_gate();
                            GatePin* dst_pin = dst->get_pin();
                            if (!out_net->remove_destination(dst))
                            {
                                log_warning("netlist_preprocessing",
                                            "failed to remove destination from output net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                            out_net->get_name(),
                                            out_net->get_id(),
                                            gate->get_name(),
                                            gate->get_id(),
                                            nl->get_id());
                                failed = true;
                                break;
                            }
                            if (!in_net->add_destination(dst_gate, dst_pin))
                            {
                                log_warning("netlist_preprocessing",
                                            "failed to add destination to input net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                            in_net->get_name(),
                                            in_net->get_id(),
                                            gate->get_name(),
                                            gate->get_id(),
                                            nl->get_id());
                                failed = true;
                                break;
                            }

                            if (failed)
                            {
                                break;
                            }
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

                // delete output net and buffer gate
                if (!failed && !nl->delete_net(out_net))
                {
                    log_warning("netlist_preprocessing",
                                "failed to remove output net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                out_net->get_name(),
                                out_net->get_id(),
                                gate->get_name(),
                                gate->get_id(),
                                nl->get_id());
                    continue;
                }

                // ... mark the passed through net as global output
                if (is_global_output)
                {
                    nl->mark_global_output_net(buffered_net);
                }

                // ... replace the output net with the passed through net for all previous module pins
                for (auto& [m, pin_info] : module_pins)
                {
                    if (auto pin = m->get_pin_by_net(buffered_net); pin != nullptr)
                    {
                        pin->set_name(std::get<1>(pin_info));
                        pin->set_type(std::get<4>(pin_info));

                        // remove pin from current pin group
                        auto current_pin_group = pin->get_group().first;
                        current_pin_group->remove_pin(pin).get();
                        if (current_pin_group->get_pins().empty())
                        {
                            m->delete_pin_group(current_pin_group);
                        }

                        // check for existing pingroup otherwise create it
                        auto pin_groups = m->get_pin_groups([pin_info](const auto& pg) { return pg->get_name() == std::get<0>(pin_info); });
                        PinGroup<ModulePin>* pin_group;
                        if (pin_groups.empty())
                        {
                            pin_group = m->create_pin_group(std::get<0>(pin_info), {}, PinDirection::none, std::get<4>(pin_info)).get();
                        }
                        else
                        {
                            pin_group = pin_groups.front();
                        }

                        pin_group->assign_pin(pin).get();
                        pin_group->move_pin(pin, std::get<2>(pin_info)).get();
                    }
                    else
                    {
                        return ERR("Cannot replace buffers: Failed to get pin " + std::get<1>(pin_info) + " at module " + m->get_name() + " with ID " + std::to_string(m->get_id()));
                    }
                }

                if (!failed)
                {
                    gates_to_be_deleted.push(gate);
                }
            }
            else if (func.is_constant() && (func.has_constant_value(0) || func.has_constant_value(1)))
            {
                auto* out_net = out_endpoint->get_net();

                const auto& gnd_gates = nl->get_gnd_gates();
                const auto& vcc_gates = nl->get_vcc_gates();
                if (gnd_gates.empty() || vcc_gates.empty())
                {
                    continue;
                }
                auto* gnd_net = gnd_gates.front()->get_fan_out_nets().front();
                auto* vcc_net = vcc_gates.front()->get_fan_out_nets().front();

                for (auto* in_endpoint : fan_in)
                {
                    auto* in_net = in_endpoint->get_net();

                    // remove the input endpoint otherwise
                    if (!in_net->remove_destination(gate, in_endpoint->get_pin()))
                    {
                        log_warning("netlist_preprocessing",
                                    "failed to remove destination from input net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                    in_net->get_name(),
                                    in_net->get_id(),
                                    gate->get_name(),
                                    gate->get_id(),
                                    nl->get_id());
                        failed = true;
                        break;
                    }
                }
                if (!failed && func.has_constant_value(0))
                {
                    for (auto* dst : out_net->get_destinations())
                    {
                        auto* dst_gate = dst->get_gate();
                        auto* dst_pin  = dst->get_pin();
                        if (!out_net->remove_destination(dst))
                        {
                            log_warning("netlist_preprocessing",
                                        "failed to remove destination from output net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                        out_net->get_name(),
                                        out_net->get_id(),
                                        gate->get_name(),
                                        gate->get_id(),
                                        nl->get_id());
                            failed = true;
                            break;
                        }
                        if (!gnd_net->add_destination(dst_gate, dst_pin))
                        {
                            log_warning("netlist_preprocessing",
                                        "failed to add buffer gate '{}' with ID {} as destination to GND net '{}' with ID {} in netlist with ID {}.",
                                        gnd_net->get_name(),
                                        gnd_net->get_id(),
                                        gate->get_name(),
                                        gate->get_id(),
                                        nl->get_id());
                            failed = true;
                            break;
                        }
                    }
                }
                else if (!failed && func.has_constant_value(1))
                {
                    for (Endpoint* dst : out_net->get_destinations())
                    {
                        Gate* dst_gate   = dst->get_gate();
                        GatePin* dst_pin = dst->get_pin();
                        if (!out_net->remove_destination(dst))
                        {
                            log_warning("netlist_preprocessing",
                                        "failed to remove destination from output net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                        out_net->get_name(),
                                        out_net->get_id(),
                                        gate->get_name(),
                                        gate->get_id(),
                                        nl->get_id());
                            failed = true;
                            break;
                        }
                        if (!vcc_net->add_destination(dst_gate, dst_pin))
                        {
                            log_warning("netlist_preprocessing",
                                        "failed to add buffer gate '{}' with ID {} as destination to VCC net '{}' with ID {} in netlist with ID {}.",
                                        vcc_net->get_name(),
                                        vcc_net->get_id(),
                                        gate->get_name(),
                                        gate->get_id(),
                                        nl->get_id());
                            failed = true;
                            break;
                        }
                    }
                }

                // delete output net and buffer gate
                if (!failed && !nl->delete_net(out_net))
                {
                    log_warning("netlist_preprocessing",
                                "failed to remove output net '{}' with ID {} of buffer gate '{}' with ID {} from netlist with ID {}.",
                                out_net->get_name(),
                                out_net->get_id(),
                                gate->get_name(),
                                gate->get_id(),
                                nl->get_id());
                    continue;
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

    Result<u32> NetlistPreprocessingPlugin::remove_redundant_logic(Netlist* nl)
    {
        const auto& nets   = nl->get_nets();
        auto nets_to_check = std::set<Net*>(nets.begin(), nets.end());

        u32 num_gates = 0;
        while (!nets_to_check.empty())
        {
            auto* current_net = *nets_to_check.begin();
            nets_to_check.erase(current_net);

            // only continue for nets with multiple destinations
            if (current_net->get_num_of_destinations() <= 1)
            {
                nets_to_check.erase(current_net);
                continue;
            }

            std::set<Gate*> visited_gates;
            std::vector<std::vector<Gate*>> gates_to_delete;
            auto destinations = current_net->get_destinations();
            for (u32 i = 0; i < destinations.size(); i++)
            {
                auto* master_destination = destinations.at(i);
                auto* master_gate        = master_destination->get_gate();

                // check if we have already identified the current master gate as duplicate of some other gate
                if (visited_gates.find(master_gate) != visited_gates.end())
                {
                    continue;
                }

                // skip everything that is not combinational, a FF, or a latch
                auto* master_type = master_gate->get_type();

                // cache master input nets and endpoints
                std::variant<std::vector<Net*>, std::map<GatePin*, Net*>> master_inputs;
                if (master_type->has_property(GateTypeProperty::combinational))
                {
                    // for combinational gates, the order of inputs will be considered by the final SMT check only (accounts for commutative Boolean functions)
                    auto tmp = master_gate->get_fan_in_nets();
                    std::sort(tmp.begin(), tmp.end());
                    master_inputs = std::move(tmp);
                }
                else if (master_type->has_property(GateTypeProperty::ff) || master_type->has_property(GateTypeProperty::latch))
                {
                    // for FF and latch gates, pins and nets must match exactly
                    std::map<GatePin*, Net*> tmp;
                    for (auto& ep : master_gate->get_fan_in_endpoints())
                    {
                        tmp[ep->get_pin()] = ep->get_net();
                    }
                    master_inputs = std::move(tmp);
                }
                else
                {
                    continue;
                }

                // identify duplicate gates
                std::vector<Gate*> duplicate_gates = {master_gate};
                for (u32 j = i + 1; j < destinations.size(); j++)
                {
                    auto* current_destination = destinations.at(j);
                    auto* current_gate        = current_destination->get_gate();

                    if (current_gate == master_gate)
                    {
                        continue;
                    }

                    // check if we have already identified the current gate as duplicate of some other gate
                    if (visited_gates.find(current_gate) != visited_gates.end())
                    {
                        continue;
                    }

                    // check against master gate type
                    auto* current_type = current_gate->get_type();
                    if (current_type != master_type)
                    {
                        continue;
                    }

                    // check current inputs against master input nets
                    std::variant<std::vector<Net*>, std::map<GatePin*, Net*>> current_inputs;
                    if (master_type->has_property(GateTypeProperty::combinational))
                    {
                        // for combinational gates, the order of inputs will be considered by the final SMT check only (accounts for commutative Boolean functions)
                        auto tmp = current_gate->get_fan_in_nets();
                        std::sort(tmp.begin(), tmp.end());
                        current_inputs = std::move(tmp);
                    }
                    else if (master_type->has_property(GateTypeProperty::ff) || master_type->has_property(GateTypeProperty::latch))
                    {
                        // for FF and latch gates, pins and nets must match exactly
                        std::map<GatePin*, Net*> tmp;
                        for (auto& ep : current_gate->get_fan_in_endpoints())
                        {
                            tmp[ep->get_pin()] = ep->get_net();
                        }
                        current_inputs = std::move(tmp);
                    }

                    if (current_inputs != master_inputs)
                    {
                        continue;
                    }

                    if (master_type->has_property(GateTypeProperty::combinational))
                    {
                        // SMT Boolean function equivalence check for combinational gates
                        bool skip_gate = false;
                        for (const auto* pin : master_type->get_output_pins())
                        {
                            const auto solver_res =
                                master_gate->get_resolved_boolean_function(pin)
                                    .map<BooleanFunction>([pin, current_gate](auto&& bf_master) {
                                        return current_gate->get_resolved_boolean_function(pin).map<BooleanFunction>(
                                            [bf_master = std::move(bf_master)](auto&& bf_current) mutable { return BooleanFunction::Eq(std::move(bf_master), bf_current.clone(), 1); });
                                    })
                                    .map<BooleanFunction>([](auto&& bf_eq) { return BooleanFunction::Not(bf_eq.clone(), 1); })
                                    .map<SMT::SolverResult>([](auto&& bf_not) -> Result<SMT::SolverResult> { return SMT::Solver({SMT::Constraint(std::move(bf_not))}).query(SMT::QueryConfig()); });

                            if (solver_res.is_error() || !solver_res.get().is_unsat())
                            {
                                skip_gate = true;
                                break;
                            }
                        }

                        if (skip_gate)
                        {
                            continue;
                        }
                    }

                    // gate is determined to be duplicate of other gate
                    duplicate_gates.push_back(current_gate);
                    visited_gates.insert(current_gate);
                }

                // remove duplicate gates
                if (duplicate_gates.size() > 1)
                {
                    gates_to_delete.push_back(duplicate_gates);
                }
            }

            for (auto& duplicate_gates : gates_to_delete)
            {
                auto* surviver_gate = duplicate_gates.front();
                std::map<GatePin*, Net*> out_pins_to_nets;
                for (auto* ep : surviver_gate->get_fan_out_endpoints())
                {
                    out_pins_to_nets[ep->get_pin()] = ep->get_net();
                    nets_to_check.insert(ep->get_net());
                }

                for (u32 k = 1; k < duplicate_gates.size(); k++)
                {
                    auto* current_gate = duplicate_gates.at(k);
                    for (auto* ep : current_gate->get_fan_out_endpoints())
                    {
                        auto* ep_net = ep->get_net();
                        auto* ep_pin = ep->get_pin();

                        if (auto it = out_pins_to_nets.find(ep_pin); it != out_pins_to_nets.end())
                        {
                            // surviver already has net connected to this output -> add destination to surviver's net
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
                            nets_to_check.erase(ep_net);
                        }
                        else
                        {
                            // surviver does not feature net on this output pin -> connect this net to surviver
                            ep_net->add_source(surviver_gate, ep_pin);
                            out_pins_to_nets[ep_pin] = ep_net;
                            nets_to_check.insert(ep_net);
                        }
                    }

                    if (!nl->delete_gate(current_gate))
                    {
                        log_warning("netlist_preprocessing", "could not delete gate '{}' with ID {} from netlist with ID {}.", current_gate->get_name(), current_gate->get_id(), nl->get_id());
                    }
                    else
                    {
                        num_gates++;
                    }
                }
            }
        }

        log_info("netlist_preprocessing", "removed {} redundant logic gates from netlist with ID {}.", num_gates, nl->get_id());
        return OK(num_gates);
    }

    Result<u32> NetlistPreprocessingPlugin::remove_unconnected_gates(Netlist* nl)
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

    Result<u32> NetlistPreprocessingPlugin::remove_unconnected_nets(Netlist* nl)
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
}    // namespace hal
