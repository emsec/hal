#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/pins/module_pin.h"
#include "hal_core/utilities/result.h"
#include "hal_core/utilities/token_stream.h"
#include "rapidjson/document.h"

#include <fstream>
#include <queue>
#include <regex>

namespace hal
{
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

    namespace abc
    {
        Result<std::string> query_binary_path()
        {
            static const std::vector<std::string> abc_binary_paths = {
                "/usr/bin/berkeley-abc", "/usr/local/bin/berkeley-abc", "/opt/homebrew/bin/berkeley-abc", "/usr/bin/abc", "/usr/local/bin/abc", "/opt/homebrew/bin/abc", "/opt/abc/abc"};

            for (const auto& path : abc_binary_paths)
            {
                if (std::filesystem::exists(path))
                {
                    return OK(path);
                }
            }

            return ERR("could not query binary path: no binary found for yosys logic synthesis tool");
        }
    }    // namespace abc

    namespace yosys
    {
        Result<std::string> query_binary_path()
        {
            static const std::vector<std::string> yosys_binary_paths = {"/usr/bin/yosys", "/usr/local/bin/yosys", "/opt/homebrew/bin/yosys", "/opt/yosys/yosys"};

            for (const auto& path : yosys_binary_paths)
            {
                if (std::filesystem::exists(path))
                {
                    return OK(path);
                }
            }

            return ERR("could not query binary path: no binary found for yosys logic synthesis tool");
        }
    }    // namespace yosys

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

    Result<u32> NetlistPreprocessingPlugin::simplify_lut_inits(Netlist* nl)
    {
        u32 num_inits = 0;

        for (auto g : nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
        {
            auto res = g->get_init_data();
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "unable to simplify lut init string for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to get original INIT string");
            }

            const auto original_inits = res.get();

            if (original_inits.size() != 1)
            {
                return ERR("unable to simplify lut init string for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": found " + std::to_string(original_inits.size())
                           + " init data strings but expected exactly 1.");
            }

            const auto original_init = original_inits.front();

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
            g->set_data("preprocessing_information", "original_init", "string", original_init);

            // const auto bf_test = g->get_boolean_function(out_ep->get_pin());

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

    // TODO make this check every pin of a gate and check whether the generated boolean function (with replaced gnd and vcc nets) is just a variable.
    //      Afterwards just connect input net to buffer destination. Do this for all pins and delete gate if it has no more successors and not global outputs
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
            const auto substitute_res = BooleanFunctionDecorator(func).substitute_power_ground_pins(nl, gate);
            if (substitute_res.is_error())
            {
                return ERR_APPEND(substitute_res.get_error(),
                                  "Cannot replace buffers: failed to substitute pins with constants at gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
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
        auto config = hal::SMT::QueryConfig();

#ifdef BITWUZLA_LIBRARY
        auto s_type = hal::SMT::SolverType::Bitwuzla;
        auto s_call = hal::SMT::SolverCall::Library;
        config.with_solver(s_type).with_call(s_call);
#endif
        struct GateFingerprint
        {
            const GateType* type;
            std::vector<Net*> ordered_fan_in = {};
            std::set<Net*> unordered_fan_in  = {};
            u8 trust_table_hw                = 0;

            bool operator<(const GateFingerprint& other) const
            {
                return (other.type < type) || (other.type == type && other.ordered_fan_in < ordered_fan_in)
                       || (other.type == type && other.ordered_fan_in == ordered_fan_in && other.unordered_fan_in < unordered_fan_in)
                       || (other.type == type && other.ordered_fan_in == ordered_fan_in && other.unordered_fan_in == unordered_fan_in && other.trust_table_hw < trust_table_hw);
            }
        };

        static std::vector<u8> hw_map = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

        u32 num_gates = 0;
        bool progress;
        std::vector<Gate*> target_gates = nl->get_gates([](const Gate* g) {
            const auto* type = g->get_type();
            return type->has_property(GateTypeProperty::combinational) || type->has_property(GateTypeProperty::ff);
        });

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
                                    tmp -= 0x11;
                                }
                                fingerprint.trust_table_hw += hw_map.at(tmp);
                            }
                        }
                    }
                }
                else if (fingerprint.type->has_property(GateTypeProperty::ff))
                {
                    fingerprint.ordered_fan_in = gate->get_fan_in_nets();
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
            for (const auto& current_duplicates : duplicate_gates)
            {
                auto* surviver_gate = current_duplicates.front();
                std::map<GatePin*, Net*> out_pins_to_nets;
                for (auto* ep : surviver_gate->get_fan_out_endpoints())
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
                            // surviver already has net connected to this output -> add destination to surviver's net
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
                            // surviver does not feature net on this output pin -> connect this net to surviver
                            ep_net->add_source(surviver_gate, ep_pin);
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

        log_info("netlist_preprocessing", "removed {} redundant logic gates from netlist with ID {}.", num_gates, nl->get_id());
        return OK(num_gates);
    }

    Result<u32> NetlistPreprocessingPlugin::remove_redundant_loops(Netlist* nl)
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
        config.with_solver(s_type).with_call(s_call);
#endif

        u32 num_gates = 0;

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
            const auto& surviver_loop = current_duplicates.front();
            auto* surviver_ff         = surviver_loop.front();

            std::map<GatePin*, Net*> out_pins_to_nets;
            for (auto* ep : surviver_ff->get_fan_out_endpoints())
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
                    }
                    else
                    {
                        // surviver does not feature net on this output pin -> connect this net to surviver
                        ep_net->add_source(surviver_ff, ep_pin);
                        out_pins_to_nets[ep_pin] = ep_net;
                    }
                }

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

        log_info("netlist_preprocessing", "removed {} redundant loops from netlist with ID {}.", num_gates, nl->get_id());
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

    namespace
    {
        Result<u32> remove_encasing_inverters(Netlist* nl)
        {
            // check wether input and output are inverted -> remove all 3 inverters
            std::vector<Gate*> muxes = nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_mux); });

            u32 delete_count = 0;
            std::vector<Gate*> delete_gate_q;

            for (const auto& g : muxes)
            {
                if (g->get_successors().size() > 1)
                {
                    continue;
                }

                auto data_pins = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_type() == PinType::data) && (pin->get_direction() == PinDirection::input); });
                auto out_pins  = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_direction() == PinDirection::output); });

                if (data_pins.size() < 2)
                {
                    continue;
                }

                if (out_pins.size() != 1)
                {
                    continue;
                }

                bool preceded_by_inv = true;
                for (const auto& pin : data_pins)
                {
                    const auto pred = g->get_predecessor(pin);
                    if (pred == nullptr || pred->get_gate() == nullptr || !pred->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter))
                    {
                        preceded_by_inv = false;
                        break;
                    }
                }

                if (!preceded_by_inv)
                {
                    continue;
                }

                bool succeded_by_inv = true;
                for (const auto& pin : out_pins)
                {
                    const auto suc = g->get_successor(pin);
                    if (suc == nullptr || suc->get_gate() == nullptr || !suc->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter))
                    {
                        succeded_by_inv = false;
                        break;
                    }
                }

                if (!succeded_by_inv)
                {
                    continue;
                }

                // delete all connections from and to inverters (and inverter gates if they do not share any other connection)
                for (const auto& pin : data_pins)
                {
                    const auto pred = g->get_predecessor(pin);

                    // disconnect inverter output from mux
                    pred->get_net()->remove_destination(g, pin);

                    // connect inverter input net to mux
                    auto in_net = pred->get_gate()->get_fan_in_nets().front();
                    in_net->add_destination(g, pin);

                    // delete inverter gate if it does not have any successors
                    if (pred->get_gate()->get_successors().empty())
                    {
                        delete_gate_q.push_back(pred->get_gate());
                    }
                }

                for (const auto& pin : out_pins)
                {
                    const auto suc = g->get_successor(pin);

                    // disconnect inverter input from mux
                    suc->get_net()->remove_source(g, pin);

                    // connect inverter output net to mux
                    auto in_net = suc->get_gate()->get_fan_out_nets().front();
                    in_net->add_source(g, pin);

                    // delete inverter gate if it does not have any predecessors
                    if (suc->get_gate()->get_predecessors().empty())
                    {
                        delete_gate_q.push_back(suc->get_gate());
                    }
                }
            }

            for (auto g : delete_gate_q)
            {
                nl->delete_gate(g);
                delete_count++;
            }

            log_info("netlist_preprocessing", "removed {} encasing inverters", delete_count);

            return OK(delete_count);
        }

        Result<u32> unify_select_signals(Netlist* nl, GateLibrary* mux_inv_gl)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            if (mux_inv_gl == nullptr)
            {
                return ERR("gate library is a nullptr");
            }

            // resynthesize all muxes where any select signal is preceded by an inverter hoping to unify the structure with regards to other muxes conntected to the same select signal
            std::vector<Gate*> muxes = nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_mux); });

            u32 count = 0;
            std::vector<Gate*> delete_gate_q;

            for (const auto& g : muxes)
            {
                auto select_pins = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_type() == PinType::select) && (pin->get_direction() == PinDirection::input); });

                std::vector<Gate*> preceding_inverters;
                for (const auto& pin : select_pins)
                {
                    const auto pred = g->get_predecessor(pin);
                    if (pred == nullptr || pred->get_gate() == nullptr || !pred->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter))
                    {
                        continue;
                    }
                    preceding_inverters.push_back(pred->get_gate());
                }

                // if there is at least one inverter in front of the mux gate we build a subgraph containing all inverters and the mux gate and resynthesize
                if (!preceding_inverters.empty())
                {
                    auto subgraph = preceding_inverters;
                    subgraph.push_back(g);

                    log_info("netlist_preprocessing", "trying to simplify subgraph:");
                    for (const auto& subgraph_g : subgraph)
                    {
                        log_info("netlist_preprocessing", "\t{}", subgraph_g->get_name());
                    }

                    auto resynth_res = NetlistPreprocessingPlugin::resynthesize_subgraph(nl, subgraph, mux_inv_gl);
                    if (resynth_res.is_error())
                    {
                        return ERR_APPEND(resynth_res.get_error(),
                                          "unable to unify selecet signals: failed to resynthesize mux " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " and its preceding inverters");
                    }
                    count++;
                }
            }

            return OK(count);
        }
    }    // namespace

    Result<u32> NetlistPreprocessingPlugin::manual_mux_optimizations(Netlist* nl, GateLibrary* mux_inv_gl)
    {
        u32 res_count = 0;

        if (nl == nullptr)
        {
            return ERR("netlist is a nullptr");
        }

        if (mux_inv_gl == nullptr)
        {
            return ERR("gate library is a nullptr");
        }

        auto remove_res = remove_encasing_inverters(nl);
        if (remove_res.is_error())
        {
            return ERR_APPEND(remove_res.get_error(), "unable to apply manual mux optimizations: failed to remove encasing inverters");
        }
        res_count += remove_res.get();

        auto unify_res = unify_select_signals(nl, mux_inv_gl);
        if (unify_res.is_error())
        {
            return ERR_APPEND(unify_res.get_error(), "unable to apply manual mux optimizations: failed to unify select signals");
        }
        res_count += unify_res.get();

        return OK(res_count);
    }

    Result<u32> NetlistPreprocessingPlugin::propagate_constants(Netlist* nl)
    {
        Net* gnd_net = nl->get_gnd_gates().empty() ? nullptr : nl->get_gnd_gates().front()->get_fan_out_nets().front();
        Net* vcc_net = nl->get_vcc_gates().empty() ? nullptr : nl->get_vcc_gates().front()->get_fan_out_nets().front();

        u32 total_replaced_dst_count = 0;

        while (true)
        {
            u32 replaced_dst_count = 0;
            std::vector<Gate*> to_delete;
            for (const auto g : nl->get_gates([](const auto g) {
                     return g->get_type()->has_property(GateTypeProperty::combinational) && !g->get_type()->has_property(GateTypeProperty::ground)
                            && !g->get_type()->has_property(GateTypeProperty::power);
                 }))
            {
                bool has_global_output = false;
                for (const auto ep : g->get_fan_out_endpoints())
                {
                    if (ep->get_net()->is_global_output_net())
                    {
                        has_global_output = true;
                    }

                    auto bf_res = g->get_resolved_boolean_function(ep->get_pin(), false);
                    if (bf_res.is_error())
                    {
                        return ERR_APPEND(bf_res.get_error(),
                                          "unable to propagate constants: failed to generate boolean function at gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " for pin "
                                              + ep->get_pin()->get_name());
                    }
                    auto bf      = bf_res.get();
                    auto sub_res = BooleanFunctionDecorator(bf).substitute_power_ground_nets(nl);
                    if (sub_res.is_error())
                    {
                        return ERR_APPEND(bf_res.get_error(),
                                          "unable to propagate constants: failed to substitue power and ground nets in boolean function of gate " + g->get_name() + " with ID "
                                              + std::to_string(g->get_id()) + " for pin " + ep->get_pin()->get_name());
                    }
                    bf = sub_res.get();
                    bf = bf.simplify_local();

                    // if boolean function of output pin can be simplified to a constant connect all its successors to gnd/vcc instead
                    if (bf.is_constant())
                    {
                        Net* new_source;
                        if (bf.has_constant_value(0))
                        {
                            new_source = gnd_net;
                        }
                        else if (bf.has_constant_value(1))
                        {
                            new_source = vcc_net;
                        }
                        else
                        {
                            continue;
                        }

                        if (new_source == nullptr)
                        {
                            log_error("netlist_preprocessing", "failed to replace bf {} with constant net because netlist is missing GND gate or VCC gate");
                        }

                        std::vector<std::pair<Gate*, GatePin*>> to_replace;
                        for (auto dst : ep->get_net()->get_destinations())
                        {
                            to_replace.push_back({dst->get_gate(), dst->get_pin()});
                        }

                        for (const auto& [dst_g, dst_p] : to_replace)
                        {
                            ep->get_net()->remove_destination(dst_g, dst_p);
                            new_source->add_destination(dst_g, dst_p);

                            replaced_dst_count++;
                        }

                        nl->delete_net(ep->get_net());
                    }
                }

                if (!has_global_output && g->get_successors().empty())
                {
                    to_delete.push_back(g);
                }
            }

            for (auto g : to_delete)
            {
                nl->delete_gate(g);
            }

            if (replaced_dst_count == 0)
            {
                break;
            }

            log_info("netlist_preprocessing", "replaced {} destinations this with power/ground nets this iteration", replaced_dst_count);
            total_replaced_dst_count += replaced_dst_count;
        }

        log_info("netlist_preprocessing", "replaced {} destinations with power/ground nets in total", total_replaced_dst_count);
        return OK(total_replaced_dst_count);
    }

    namespace
    {
        Result<std::tuple<GateType*, std::vector<GatePin*>, std::vector<GatePin*>>>
            find_gate_type(const GateLibrary* gl, const std::set<GateTypeProperty>& properties, const u32 num_inputs, const u32 num_outputs)
        {
            const auto get_valid_input_pins = [](const GateType* gt) -> std::vector<GatePin*> {
                return gt->get_pins([](const GatePin* gp) { return (gp->get_direction() == PinDirection::input) && (gp->get_type() != PinType::power) && (gp->get_type() != PinType::ground); });
            };

            const auto get_valid_output_pins = [](const GateType* gt) -> std::vector<GatePin*> {
                return gt->get_pins([](const GatePin* gp) { return (gp->get_direction() == PinDirection::output) && (gp->get_type() != PinType::power) && (gp->get_type() != PinType::ground); });
            };

            // get types that match exactly with the properties and have the exact amount of input pins (excluding power pins)
            const auto candidates = gl->get_gate_types([properties, num_inputs, get_valid_input_pins, num_outputs, get_valid_output_pins](const GateType* gt) {
                return (gt->get_properties() == properties) && (get_valid_input_pins(gt).size() == num_inputs) && (get_valid_output_pins(gt).size() == num_outputs);
            });

            if (candidates.empty())
            {
                return ERR("Unable to find gate type matching the description");
            }

            GateType* valid_gate_type = candidates.begin()->second;

            return OK({valid_gate_type, get_valid_input_pins(valid_gate_type), get_valid_output_pins(valid_gate_type)});
        }

        // TODO change this to return a netlist. This would allow saving the decomposition of a specifc gate type
        Result<Net*> build_gate_tree_from_boolean_function(Netlist* nl, const BooleanFunction& bf, const std::map<std::string, Net*>& var_name_to_net, const Gate* org_gate = nullptr)
        {
            const auto create_gate_name = [](const Gate* new_gate, const Gate* original_gate) -> std::string {
                const std::string new_name = (original_gate == nullptr) ? "new_gate_" : original_gate->get_name() + "_decomposed_";
                return new_name + std::to_string(new_gate->get_id());
            };

            const auto create_net_name = [](const Net* new_net, const Gate* original_gate) -> std::string {
                const std::string new_name = (original_gate == nullptr) ? "new_net_" : original_gate->get_name() + "_decomposed_";
                return new_name + std::to_string(new_net->get_id());
            };

            if (bf.is_empty())
            {
                return ERR("cannot build gate tree for Boolean function: Boolean function is empty");
            }

            if (bf.is_index())
            {
                return ERR("cannot build gate tree for Boolean function: Boolean function is of type index");
            }

            if (bf.size() != 1)
            {
                return ERR("cannot build gate tree for Boolean function: Boolean function if of size " + std::to_string(bf.size()) + " but we only handle size 1");
            }

            if (bf.is_constant())
            {
                if (bf.has_constant_value(0))
                {
                    static Net* zero = nl->get_nets([](const Net* n) { return n->is_gnd_net(); }).front();
                    return OK(zero);
                }

                if (bf.has_constant_value(1))
                {
                    static Net* one = nl->get_nets([](const Net* n) { return n->is_vcc_net(); }).front();
                    return OK(one);
                }
            }

            if (bf.is_variable())
            {
                if (const auto it = var_name_to_net.find(bf.get_variable_name().get()); it == var_name_to_net.end())
                {
                    return ERR("Cannot build gate tree for Boolean function: Found variable " + bf.get_variable_name().get() + " with no corresponding net provided.");
                }
                else
                {
                    return OK(it->second);
                }
            }

            if (!bf.get_top_level_node().is_operation())
            {
                return ERR("Cannot build gate tree for Boolean function: cannot handle node type of top level node " + bf.get_top_level_node().to_string());
            }

            const auto operation  = bf.get_top_level_node().type;
            const auto parameters = bf.get_parameters();

            // TODO put this into a function that only searches for the gate types when they are actually needed
            static const auto inv_type_res = find_gate_type(nl->get_gate_library(), {GateTypeProperty::combinational, GateTypeProperty::c_inverter}, 1, 1);
            static const auto and_type_res = find_gate_type(nl->get_gate_library(), {GateTypeProperty::combinational, GateTypeProperty::c_and}, 2, 1);
            static const auto or_type_res  = find_gate_type(nl->get_gate_library(), {GateTypeProperty::combinational, GateTypeProperty::c_or}, 2, 1);
            static const auto xor_type_res = find_gate_type(nl->get_gate_library(), {GateTypeProperty::combinational, GateTypeProperty::c_xor}, 2, 1);

            if (inv_type_res.is_error())
            {
                return ERR("Cannot build gate tree for Boolean function: failed to find valid inverter gate type");
            }

            if (and_type_res.is_error())
            {
                return ERR("Cannot build gate tree for Boolean function: failed to find valid and gate type");
            }

            if (or_type_res.is_error())
            {
                return ERR("Cannot build gate tree for Boolean function: failed to find valid or gate type");
            }

            if (xor_type_res.is_error())
            {
                return ERR("Cannot build gate tree for Boolean function: failed to find valid xor gate type");
            }

            const std::map<u16, std::tuple<GateType*, std::vector<GatePin*>, std::vector<GatePin*>>> node_type_to_gate_type = {
                {BooleanFunction::NodeType::Not, inv_type_res.get()},
                {BooleanFunction::NodeType::And, and_type_res.get()},
                {BooleanFunction::NodeType::Or, or_type_res.get()},
                {BooleanFunction::NodeType::Xor, xor_type_res.get()},
            };

            std::vector<Net*> parameter_nets;
            for (const auto& p : parameters)
            {
                const auto tree_res = build_gate_tree_from_boolean_function(nl, p, var_name_to_net, org_gate);
                if (tree_res.is_error())
                {
                    return ERR_APPEND(tree_res.get_error(), "Cannot build gate tree for Boolean function: failed to do so for sub tree");
                }
                parameter_nets.push_back(tree_res.get());
            }

            Gate* new_gate  = nullptr;
            Net* output_net = nl->create_net("__TEMP_NET_NAME__DECOMPOSED__");
            output_net->set_name(create_net_name(output_net, org_gate));

            switch (operation)
            {
                case BooleanFunction::NodeType::Not:
                case BooleanFunction::NodeType::And:
                case BooleanFunction::NodeType::Or:
                case BooleanFunction::NodeType::Xor: {
                    auto [gt, in_pins, out_pins] = node_type_to_gate_type.at(operation);
                    new_gate                     = nl->create_gate(gt, "__TEMP_GATE_NAME__DECOMPOSED__");
                    for (u32 idx = 0; idx < parameter_nets.size(); idx++)
                    {
                        parameter_nets.at(idx)->add_destination(new_gate, in_pins.at(idx));
                    }
                    output_net->add_source(new_gate, out_pins.front());
                    break;
                }
                default:
                    break;
            }

            if (new_gate == nullptr)
            {
                return ERR("Cannot build gate tree for Boolean function: failed to create gate for operation " + bf.get_top_level_node().to_string());
            }

            new_gate->set_name(create_gate_name(new_gate, org_gate));

            if (org_gate != nullptr && !org_gate->get_module()->is_top_module())
            {
                org_gate->get_module()->assign_gate(new_gate);
            }

            return OK(output_net);
        }
    }    // namespace

    Result<std::monostate> NetlistPreprocessingPlugin::decompose_gate(Netlist* nl, Gate* g, const bool delete_gate)
    {
        // build Boolean function for each output pin of the gate
        std::map<std::string, BooleanFunction> output_pin_name_to_bf;
        for (const auto& out_ep : g->get_fan_out_endpoints())
        {
            const auto bf_res = g->get_resolved_boolean_function(out_ep->get_pin());
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(),
                                  "unable to decompose gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to resolve Boolean function for pin "
                                      + out_ep->get_pin()->get_name());
            }
            output_pin_name_to_bf.insert({out_ep->get_pin()->get_name(), bf_res.get()});
        }

        // map which variables in the Boolean function belong to which net
        std::map<std::string, Net*> var_name_to_net;
        for (const auto& in_ep : g->get_fan_in_endpoints())
        {
            var_name_to_net.insert({BooleanFunctionNetDecorator(*(in_ep->get_net())).get_boolean_variable_name(), in_ep->get_net()});
        }

        // build gate tree for each output function and merge the tree output net with the origianl output net
        for (const auto& [pin_name, bf] : output_pin_name_to_bf)
        {
            Net* output_net = g->get_fan_out_net(pin_name);
            if (output_net == nullptr)
            {
                continue;
            }

            const auto tree_res = build_gate_tree_from_boolean_function(nl, bf, var_name_to_net, g);
            if (tree_res.is_error())
            {
                return ERR_APPEND(tree_res.get_error(),
                                  "unable to decompose gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to build gate tree for output net at pin " + pin_name);
            }

            auto new_output_net = tree_res.get();

            const auto slave_net  = new_output_net->is_global_input_net() ? output_net : new_output_net;
            const auto master_net = new_output_net->is_global_input_net() ? new_output_net : output_net;
            const auto merge_res  = NetlistModificationDecorator(*nl).connect_nets(master_net, slave_net);
            // const auto merge_res = netlist_utils::merge_nets(nl, new_output_net, output_net, new_output_net->is_global_input_net());
            if (merge_res.is_error())
            {
                return ERR_APPEND(merge_res.get_error(),
                                  "unable to decompose gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to merge newly created output net with already existing one.");
            }
        }

        if (delete_gate)
        {
            if (!nl->delete_gate(g))
            {
                return ERR("unable to decompose gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to delete original gate.");
            }
        }

        return OK({});
    }

    Result<u32> NetlistPreprocessingPlugin::decompose_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types)
    {
        u32 counter = 0;
        for (const auto& gt : gate_types)
        {
            std::vector<Gate*> to_delete;
            for (const auto& g : nl->get_gates([gt](const Gate* g) { return g->get_type() == gt; }))
            {
                const auto decompose_res = decompose_gate(nl, g, false);
                if (decompose_res.is_error())
                {
                    return ERR_APPEND(decompose_res.get_error(),
                                      "unable to decompose gates of type " + gt->get_name() + ": failed for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                }
                to_delete.push_back(g);
            }

            for (const auto& g : to_delete)
            {
                counter += 1;
                if (!nl->delete_gate(g))
                {
                    return ERR("unable to decompose gates of type " + gt->get_name() + ": failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                }
            }
        }

        return OK(counter);
    }

    namespace
    {
        //const std::string resynth_name_suffix = "_RESYNTH";

        std::string new_net_name(const Net* dst_net, const Net* new_net)
        {
            return new_net->get_name() + "_" + std::to_string(dst_net->get_id()) + "_RESYNTH";
        }

        std::string new_gate_name(const Gate* dst_gate, const Gate* new_gate)
        {
            return new_gate->get_name() + "_" + std::to_string(dst_gate->get_id()) + "_RESYNTH";
        }

        // NOTE there are about a hundred more checks that we could do here
        Result<std::monostate> replace_subgraph_with_netlist(const std::vector<Gate*>& subgraph,
                                                             const std::unordered_map<Net*, Net*>& global_io_mapping,
                                                             const Netlist* src_nl,
                                                             Netlist* dst_nl,
                                                             const bool delete_subgraph_gates)
        {
            std::unordered_map<std::string, Gate*> gate_name_to_gate;

            const auto dst_gl = dst_nl->get_gate_library();

            // add all gates to the source netlist to the destination netlist
            for (const auto src_g : src_nl->get_gates())
            {
                const auto src_gt = src_g->get_type();
                const auto dst_gt = dst_gl->get_gate_type_by_name(src_gt->get_name());
                if (!dst_gt)
                {
                    return ERR("unable to replace subgraph with netlist: destination gate library " + dst_gl->get_name() + " does not contain the required gate type " + src_gt->get_name());
                }

                auto new_gate              = dst_nl->create_gate(dst_gt, "TEMP");
                const std::string new_name = new_gate_name(new_gate, src_g);
                new_gate->set_name(new_name);

                gate_name_to_gate.insert({src_g->get_name(), new_gate});
            }

            // connect all nets of the source netlist to the destination netlist
            for (const auto src_n : src_nl->get_nets())
            {
                Net* new_net = nullptr;

                // edge case for global io
                if (src_n->is_global_input_net() || src_n->is_global_output_net())
                {
                    if (const auto it = global_io_mapping.find(src_n); it != global_io_mapping.end())
                    {
                        new_net = global_io_mapping.at(src_n);
                    }
                    else
                    {
                        return ERR("unable to replace subgraph with netlist: failed to locate mapped net in destination netlist for global io net " + src_n->get_name() + " with ID "
                                   + std::to_string(src_n->get_id()));
                    }
                }

                // connect net to source
                if (src_n->is_gnd_net())
                {
                    // set new net to an existing gnd net
                    const auto gnd_gate = dst_nl->get_gnd_gates().front();
                    const auto out_net  = gnd_gate->get_fan_out_nets().front();
                    new_net             = out_net;
                }
                else if (src_n->is_vcc_net())
                {
                    const auto vcc_gate = dst_nl->get_vcc_gates().front();
                    const auto out_net  = vcc_gate->get_fan_out_nets().front();
                    new_net             = out_net;
                }
                else if (!src_n->is_global_input_net())
                {
                    if (!new_net)
                    {
                        new_net                    = dst_nl->create_net("TEMP");
                        const std::string new_name = new_net_name(new_net, src_n);
                        new_net->set_name(new_name);
                    }

                    for (const auto src_ep : src_n->get_sources())
                    {
                        const auto org_src_name     = src_ep->get_gate()->get_name();
                        const auto org_src_pin_name = src_ep->get_pin()->get_name();
                        auto new_src_g              = gate_name_to_gate.at(org_src_name);
                        new_net->add_source(new_src_g, org_src_pin_name);
                    }
                }

                // connect net to destinations
                if (!src_n->is_global_output_net())
                {
                    for (const auto src_ep : src_n->get_destinations())
                    {
                        const auto org_dst_name     = src_ep->get_gate()->get_name();
                        const auto org_dst_pin_name = src_ep->get_pin()->get_name();
                        auto new_dst_g              = gate_name_to_gate.at(org_dst_name);
                        new_net->add_destination(new_dst_g, org_dst_pin_name);
                    }
                }
            }

            // delete subgraph gates if flag is set
            if (delete_subgraph_gates)
            {
                for (const auto g : subgraph)
                {
                    dst_nl->delete_gate(g);
                }
            }

            return OK({});
        }

        Result<std::monostate> replace_gate_with_netlist(Gate* g, const Netlist* src_nl, Netlist* dst_nl, const bool delete_gate = true)
        {
            std::unordered_map<Net*, Net*> global_io_mapping;

            for (const auto& g_i : src_nl->get_global_input_nets())
            {
                const auto& pin_name = g_i->get_name();
                global_io_mapping.insert({g_i, g->get_fan_in_net(pin_name)});
            }

            for (const auto& g_o : src_nl->get_global_output_nets())
            {
                const auto& pin_name = g_o->get_name();
                global_io_mapping.insert({g_o, g->get_fan_out_net(pin_name)});
            }

            return replace_subgraph_with_netlist({g}, global_io_mapping, src_nl, dst_nl, delete_gate);
        }

        std::string build_functional_verilog_module_from(const std::unordered_map<std::string, BooleanFunction>& bfs)
        {
            std::unordered_set<std::string> input_variable_names;

            for (const auto& [name, bf] : bfs)
            {
                for (const auto& var_name : bf.get_variable_names())
                {
                    input_variable_names.insert(var_name);
                }
            }

            std::string verilog_str = "module top (";

            std::string var_str      = "";
            std::string io_str       = "";
            std::string function_str = "";

            for (const auto& input_var : input_variable_names)
            {
                var_str += (input_var + ", ");
                io_str += ("input " + input_var + ";\n");
            }

            for (const auto& [output_var, bf] : bfs)
            {
                var_str += (output_var + ", ");
                io_str += ("output " + output_var + ";\n");
                function_str += ("assign " + output_var + " = " + bf.to_string() + ";\n");
            }

            var_str = var_str.substr(0, var_str.size() - 2);

            verilog_str += var_str;
            verilog_str += ");\n";

            verilog_str += io_str;

            verilog_str += "\n";

            verilog_str += function_str;

            verilog_str += "\n";
            verilog_str += "endmodule\n";
            verilog_str += "\n";

            return verilog_str;
        }

        Result<std::unique_ptr<Netlist>> generate_resynth_netlist_for_boolean_functions(const std::unordered_map<std::string, BooleanFunction>& bfs,
                                                                                        const std::filesystem::path& genlib_path,
                                                                                        GateLibrary* hgl_lib,
                                                                                        const bool optimize_area)
        {
            const auto verilog_module = build_functional_verilog_module_from(bfs);

            // TODO make this more robust
            const std::filesystem::path base_path                  = std::filesystem::temp_directory_path() / "resynthesize_boolean_functions_with_yosys";
            const std::filesystem::path functional_netlist_path    = base_path / "func_netlist.v";
            const std::filesystem::path resynthesized_netlist_path = base_path / "resynth_netlist.v";

            std::filesystem::create_directory(base_path);

            log_debug("netlist_preprocessing", "Writing Verilog file to {} ...", functional_netlist_path.string());

            std::ofstream out(functional_netlist_path);
            out << verilog_module;
            out.close();

            auto yosys_query_res = yosys::query_binary_path();
            if (yosys_query_res.is_error())
            {
                return ERR_APPEND(yosys_query_res.get_error(), "Unable to resynthesize boolean functions with yosys: failed to find yosys path");
            }

            const auto yosys_path     = yosys_query_res.get();
            const std::string command = yosys_path + " -q -p " + "\"read -sv " + functional_netlist_path.string() + "; hierarchy -top top; proc; fsm; opt; memory; opt; techmap; opt; abc -genlib "
                                        + genlib_path.string() + "; " + "write_verilog " + resynthesized_netlist_path.string() + "; clean\"";

            log_debug("netlist_preprocessing", "yosys command: {}", command);

            system(command.c_str());

            auto resynth_nl = netlist_factory::load_netlist(resynthesized_netlist_path, hgl_lib);

            // yosys workaround for stupid net renaming
            for (const auto& pin : resynth_nl->get_top_module()->get_input_pins())
            {
                auto net = pin->get_net();
                net->set_name(pin->get_name());
                log_debug("netlist_preprocessing", "renamed net {} with pin name {}", net->get_name(), pin->get_name());
            }

            if (resynth_nl == nullptr)
            {
                return ERR("Unable to resynthesize boolean functions with yosys: failed to load resynthesized netlist at " + resynthesized_netlist_path.string());
            }

            return OK(std::move(resynth_nl));
        }

        Result<std::unique_ptr<Netlist>> generate_resynth_netlist_for_gate_level_subgraph(const Netlist* nl,
                                                                                          const std::vector<Gate*>& subgraph,
                                                                                          const std::filesystem::path& genlib_path,
                                                                                          GateLibrary* hgl_lib,
                                                                                          const bool optimize_area)
        {
            // TODO sanity check wether all gates in the subgraph have types that are in the genlib/target library
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            if (hgl_lib == nullptr)
            {
                return ERR("gate library is a nullptr");
            }

            auto subgraph_nl_res = SubgraphNetlistDecorator(*nl).copy_subgraph_netlist(subgraph);
            if (subgraph_nl_res.is_error())
            {
                return ERR_APPEND(subgraph_nl_res.get_error(), "unable to resynthesize gate level subgraph: failed to copy subgraph netlist");
            }
            const auto subgraph_nl = subgraph_nl_res.get();

            // TODO make this more robust
            const std::filesystem::path base_path                  = std::filesystem::temp_directory_path() / "resynthesize_boolean_functions_with_abc";
            const std::filesystem::path org_netlist_path           = base_path / "org_netlist.v";
            const std::filesystem::path resynthesized_netlist_path = base_path / "resynth_netlist.v";

            std::filesystem::create_directory(base_path);

            log_info("netlist_preprocessing", "Writing Verilog file to {} ...", org_netlist_path.string());

            netlist_writer_manager::write(subgraph_nl.get(), org_netlist_path);

            // NOTE this is a way to get rid of the stupid timescale annotation that is added by the verilog writer (for the simulator!?) but cannot be parsed by yosys...
            // Read the file into a vector of lines
            std::vector<std::string> lines;
            std::ifstream input_file(org_netlist_path);

            std::string line;
            while (std::getline(input_file, line))
            {
                lines.push_back(line);
            }
            input_file.close();

            // Remove the first line from the vector
            if (!lines.empty())
            {
                lines.erase(lines.begin());
            }

            // Write the modified lines back to the file
            std::ofstream output_file(org_netlist_path);
            for (const auto& line : lines)
            {
                output_file << line << std::endl;
            }
            output_file.close();

            auto abc_query_res = abc::query_binary_path();
            if (abc_query_res.is_error())
            {
                return ERR_APPEND(abc_query_res.get_error(), "Unable to resynthesize gate level netlist with abc: failed to find abc path");
            }

            const auto abc_path       = abc_query_res.get();
            const std::string command = abc_path + " -c " + '"' + "read_library " + genlib_path.string() + "; " + "read_verilog -m " + org_netlist_path.string() + "; "
                                        + "unmap; cleanup; sweep; strash; dc2; logic; " + "map" + (optimize_area ? " -a" : "") + "; " + "write_verilog " + resynthesized_netlist_path.string() + ";"
                                        + '"';

            system(command.c_str());

            auto resynth_nl = netlist_factory::load_netlist(resynthesized_netlist_path, hgl_lib);
            if (resynth_nl == nullptr)
            {
                return ERR("Unable to resynthesize gate level netlist with yosys: failed to load resynthesized netlist at " + resynthesized_netlist_path.string());
            }

            return OK(std::move(resynth_nl));
        }

        // TODO move to the netlist traversal decorator, currently existing in the machine learning branch
        std::vector<Net*> get_outputs_of_subgraph(const std::vector<Gate*>& subgraph)
        {
            std::unordered_set<Gate*> subgraph_set = {subgraph.begin(), subgraph.end()};
            std::unordered_set<Net*> outputs;

            for (const auto g : subgraph)
            {
                for (const auto ep : g->get_successors())
                {
                    // check whether gate has a successor outside the subgraph
                    if (subgraph_set.find(ep->get_gate()) == subgraph_set.end())
                    {
                        outputs.insert(ep->get_net());
                    }
                }
            }

            return {outputs.begin(), outputs.end()};
        }

        // TODO delete function and just build boolean functions in caller (or maybe dont, think about it)
        Result<std::unique_ptr<Netlist>> generate_resynth_netlist_for_functional_subgraph(const Netlist* nl,
                                                                                          const std::vector<Gate*>& subgraph,
                                                                                          const std::filesystem::path& genlib_path,
                                                                                          GateLibrary* hgl_lib,
                                                                                          const bool optimize_area)
        {
            const auto outputs = get_outputs_of_subgraph(subgraph);
            std::unordered_map<std::string, BooleanFunction> bfs;

            log_info("netlist_preprocessing", "building boolean function for {} output nets ...", outputs.size());

            for (const auto o_net : outputs)
            {
                const auto o_net_var_name = BooleanFunctionNetDecorator(*o_net).get_boolean_variable_name();

                const auto bf_res = SubgraphNetlistDecorator(*nl).get_subgraph_function(subgraph, o_net);
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(),
                                      "unable to resynth subgraph: failed to generate boolean function for output net " + o_net->get_name() + " with ID " + std::to_string(o_net->get_id()));
                }
                auto bf = bf_res.get();

                bfs.insert({o_net_var_name, bf});
            }

            return generate_resynth_netlist_for_boolean_functions(bfs, genlib_path, hgl_lib, optimize_area);
        }

        Result<std::unique_ptr<Netlist>> generate_resynth_netlist_for_gate(const Gate* g, GateLibrary* hgl_lib, const std::filesystem::path& genlib_path)
        {
            // build Boolean function for each output pin of the gate type
            std::unordered_map<std::string, BooleanFunction> output_pin_name_to_bf;
            for (const auto pin : g->get_type()->get_output_pins())
            {
                const auto bf_res = g->get_resolved_boolean_function(pin, true);
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(),
                                      "unable to resynthesize lut type " + g->get_type()->get_name() + " for gate instance " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                          + ": failed to build resolved boolean function for pin " + pin->get_name());
                }

                const auto bf = bf_res.get();
                output_pin_name_to_bf.insert({pin->get_name(), bf});
            }

            auto resynth_res = generate_resynth_netlist_for_boolean_functions(output_pin_name_to_bf, genlib_path, hgl_lib, true);
            if (resynth_res.is_error())
            {
                return ERR_APPEND(resynth_res.get_error(),
                                  "unable to resynthesize lut type " + g->get_type()->get_name() + " for gate instance " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                      + ": failed to resynthesize boolean functions of gate");
            }

            return OK(resynth_res.get());
        }
    }    // namespace

    Result<std::monostate> NetlistPreprocessingPlugin::resynthesize_gate(Netlist* nl, Gate* g, GateLibrary* hgl_lib, const std::filesystem::path& genlib_path, const bool delete_gate)
    {
        if (g == nullptr)
        {
            return ERR("no valid gate selected (gate is nullptr)");
        }

        if (hgl_lib == nullptr)
        {
            return ERR("no valid hgl_lib selected (hgl_lib is nullptr)");
        }

        if (!utils::file_exists(genlib_path.string()))
        {
            return ERR("genlib does not exist");
        }

        auto resynth_res = generate_resynth_netlist_for_gate(g, hgl_lib, genlib_path);
        if (resynth_res.is_error())
        {
            return ERR_APPEND(resynth_res.get_error(), "unable to resynthesize gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to generate reynthesized netlist");
        }
        const auto resynth_nl  = resynth_res.get();
        const auto replace_res = replace_gate_with_netlist(g, resynth_nl.get(), nl, false);
        if (replace_res.is_error())
        {
            return ERR_APPEND(replace_res.get_error(),
                              "unable to resynthesize gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to replace gate with resynthesized netlist");
        }

        if (delete_gate)
        {
            if (!nl->delete_gate(g))
            {
                return ERR("unable to decompose gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to delete original gate.");
            }
        }

        return OK({});
    }

    Result<u32> NetlistPreprocessingPlugin::resynthesize_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl)
    {
        const std::filesystem::path base_path   = std::filesystem::temp_directory_path() / "resynthesize_boolean_functions_with_yosys";
        const std::filesystem::path genlib_path = base_path / "new_gate_library.genlib";
        std::filesystem::create_directory(base_path);

        log_info("netlist_preprocessing", "Writing gate library to file {} ...", genlib_path.string());
        const auto gl_save_res = gate_library_manager::save(genlib_path, target_gl, true);
        if (!gl_save_res)
        {
            return ERR("unable to resynthesize gates of type: failed to save gate library " + target_gl->get_name() + " to location " + genlib_path.string());
        }

        std::map<std::pair<const GateType*, std::vector<std::string>>, std::unique_ptr<Netlist>> gt_to_resynth;

        u32 counter = 0;
        for (const auto& gt : gate_types)
        {
            std::vector<Gate*> to_delete;
            for (const auto& g : nl->get_gates([gt](const Gate* g) { return g->get_type() == gt; }))
            {
                const Netlist* resynth_nl = nullptr;
                std::vector<std::string> init_data;

                if (gt->has_property(GateTypeProperty::c_lut))
                {
                    const auto init_res = g->get_init_data();
                    if (init_res.is_error())
                    {
                        return ERR_APPEND(init_res.get_error(),
                                          "unable to resynthesize gates of type: failed to get init string from gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }
                    const auto init_vec = init_res.get();
                    if (init_vec.size() != 1)
                    {
                        return ERR("unable tor resynthesize gates of type: got " + std::to_string(init_vec.size()) + " init strings for gate" + g->get_name() + " with ID "
                                   + std::to_string(g->get_id()));
                    }

                    init_data = init_vec;
                }

                // get resynth netlist from cache or create it
                if (const auto it = gt_to_resynth.find({gt, init_data}); it != gt_to_resynth.end())
                {
                    resynth_nl = it->second.get();
                }
                else
                {
                    auto resynth_res = generate_resynth_netlist_for_gate(g, target_gl, genlib_path);
                    if (resynth_res.is_error())
                    {
                        return ERR_APPEND(resynth_res.get_error(), "unable to resynthesize gates of type: failed to resynthesize gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }
                    auto unique_resynth_nl = resynth_res.get();
                    resynth_nl             = unique_resynth_nl.get();
                    gt_to_resynth.insert({std::make_pair(gt, init_data), std::move(unique_resynth_nl)});
                }

                const auto replace_res = replace_gate_with_netlist(g, resynth_nl, nl, false);
                if (replace_res.is_error())
                {
                    return ERR_APPEND(replace_res.get_error(),
                                      "unable to resynthesize gates of type " + gt->get_name() + ": failed for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                }
                to_delete.push_back(g);
            }

            for (const auto& g : to_delete)
            {
                counter += 1;
                if (!nl->delete_gate(g))
                {
                    return ERR("unable to resynthesize gates of type " + gt->get_name() + ": failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                }
            }
        }

        return OK(counter);
    }

    Result<u32> NetlistPreprocessingPlugin::resynthesize_subgraph(Netlist* nl, const std::vector<Gate*>& subgraph, GateLibrary* target_gl)
    {
        if (nl == nullptr)
        {
            return ERR("netlist is a nullptr");
        }

        if (target_gl == nullptr)
        {
            return ERR("gate library is a nullptr");
        }

        const std::filesystem::path base_path   = std::filesystem::temp_directory_path() / "resynthesize_boolean_functions_with_yosys";
        const std::filesystem::path genlib_path = base_path / "new_gate_library.genlib";
        std::filesystem::create_directory(base_path);

        log_info("netlist_preprocessing", "Writing gate library to file {} ...", genlib_path.string());
        const auto gl_save_res = gate_library_manager::save(genlib_path, target_gl, true);
        if (!gl_save_res)
        {
            return ERR("unable to resynthesize gates of type: failed to save gate library " + target_gl->get_name() + " to location " + genlib_path.string());
        }

        // auto resynth_res = resynthesize_functional_subgraph_with_yosys(nl, subgraph, genlib_path, target_gl, true);
        auto resynth_res = generate_resynth_netlist_for_gate_level_subgraph(nl, subgraph, genlib_path, target_gl, true);
        if (resynth_res.is_error())
        {
            return ERR_APPEND(resynth_res.get_error(), "unable to resynthesize subgraphs of type: failed to resynthesize subgraph to netlist");
        }
        auto resynth_nl = resynth_res.get();

        std::unordered_map<std::string, Net*> name_to_net;
        for (const auto n : nl->get_nets())
        {
            name_to_net.insert({n->get_name(), n});
        }

        std::unordered_map<Net*, Net*> global_io_mapping;

        for (const auto& g_i : resynth_nl->get_global_input_nets())
        {
            auto net_it = name_to_net.find(g_i->get_name());
            if (net_it == name_to_net.end())
            {
                return ERR("unable to resynthesize subgraphs of type: failed to locate net in destination netlist from global input " + g_i->get_name() + " in resynthesized netlist");
            }
            global_io_mapping.insert({g_i, net_it->second});
        }

        for (const auto& g_o : resynth_nl->get_global_output_nets())
        {
            auto net_it = name_to_net.find(g_o->get_name());
            if (net_it == name_to_net.end())
            {
                return ERR("unable to resynthesize subgraphs of type: failed to locate net in destination netlist from global output " + g_o->get_name() + " in resynthesized netlist");
            }
            global_io_mapping.insert({g_o, net_it->second});
        }

        auto replace_res = replace_subgraph_with_netlist(subgraph, global_io_mapping, resynth_nl.get(), nl, false);
        if (replace_res.is_error())
        {
            return ERR_APPEND(replace_res.get_error(), "unable to resynthesize subgraphs of type: failed to replace subgrap with resynthesized netlist");
        }

        for (const auto g : subgraph)
        {
            log_info("netlist_preprocessing", "removing gate: {}", g->get_name());
            nl->delete_gate(g);
        }

        return OK(subgraph.size());
    }

    Result<u32> NetlistPreprocessingPlugin::resynthesize_subgraph_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl)
    {
        std::vector<Gate*> subgraph;
        for (const auto& gt : gate_types)
        {
            for (const auto& g : nl->get_gates([gt](const Gate* g) { return g->get_type() == gt; }))
            {
                subgraph.push_back(g);
            }
        }

        log_info("netlist_preprocessing", "Gathered subgraph with {} gates", subgraph.size());

        return resynthesize_subgraph(nl, subgraph, target_gl);
    }

    namespace
    {
        struct indexed_identifier
        {
            indexed_identifier(const std::string& p_identifier, const u32 p_index, const std::string& p_origin) : identifier{p_identifier}, index{p_index}, origin{p_origin}
            {
            }

            std::string identifier;
            u32 index;
            std::string origin;
        };

        // TODO when the verilog parser changes are merged into the master this will no longer be needed
        const std::string hal_instance_index_pattern         = "__\\[(\\d+)\\]__";
        const std::string hal_instance_index_pattern_reverse = "<HAL>(\\d+)<HAL>";

        std::string replace_hal_instance_index(const std::string& name)
        {
            std::regex re(hal_instance_index_pattern);

            std::string input = name;
            std::string index;
            std::smatch match;
            while (std::regex_search(input, match, re))
            {
                index = match[1];
                input = utils::replace(input, match.str(), "<HAL>" + index + "<HAL>");
            }

            return input;
        }

        std::string reconstruct_hal_instance_index(const std::string& name)
        {
            std::regex re(hal_instance_index_pattern_reverse);

            std::string input = name;
            std::string index;
            std::smatch match;
            while (std::regex_search(input, match, re))
            {
                index = match[1];
                input = utils::replace(input, match.str(), "__[" + index + "]__");
            }

            return input;
        }

        const std::string net_index_pattern  = "\\((\\d+)\\)";
        const std::string gate_index_pattern = "\\[(\\d+)\\]";

        // Extracts an index from a string by taking the last integer enclosed by parentheses
        std::optional<indexed_identifier> extract_index(const std::string& name, const std::string& index_pattern, const std::string& origin)
        {
            std::regex re(index_pattern);

            std::string input = name;
            std::optional<std::string> last_match;
            std::optional<u32> last_index;

            // Search for last match within string
            std::smatch match;
            while (std::regex_search(input, match, re))
            {
                // Capture integer and update input string to search from after the match
                last_index = std::stoi(match[1]);
                last_match = match.str();
                input      = match.suffix().str();
            }

            if (!last_index.has_value())
            {
                return std::nullopt;
            }

            const auto found_match = last_match.value();
            auto identifier_name   = name;
            identifier_name        = identifier_name.replace(name.rfind(found_match), found_match.size(), "");

            return std::optional<indexed_identifier>{{identifier_name, last_index.value(), origin}};
        }

        // annotate all found identifiers to a gate
        bool annotate_indexed_identifiers(Gate* gate, const std::vector<indexed_identifier>& identifiers)
        {
            std::string json_identifier_str =
                "[" + utils::join(", ", identifiers, [](const auto& i) { return std::string("[") + '"' + i.identifier + '"' + ", " + std::to_string(i.index) + ", " + '"' + i.origin + '"' + "]"; })
                + "]";

            return gate->set_data("preprocessing_information", "multi_bit_indexed_identifiers", "string", json_identifier_str);
        }

        // search for a net that connects to the gate at a pin of a specific type and tries to reconstruct an indexed identifier from its name or form a name of its merged wires
        std::vector<indexed_identifier> check_net_at_pin(const PinType pin_type, Gate* gate)
        {
            const auto typed_pins = gate->get_type()->get_pins([pin_type](const auto p) { return p->get_type() == pin_type; });

            std::vector<indexed_identifier> found_identfiers;

            for (const auto& pin : typed_pins)
            {
                const auto typed_net = (pin->get_direction() == PinDirection::output) ? gate->get_fan_out_net(pin) : gate->get_fan_in_net(pin);

                // 1) search the net name itself
                const auto net_name_index = extract_index(typed_net->get_name(), net_index_pattern, "net_name");
                if (net_name_index.has_value())
                {
                    found_identfiers.push_back(net_name_index.value());
                }

                // 2) search all the names of the wires that where merged into this net
                if (!typed_net->has_data("parser_annotation", "merged_nets"))
                {
                    continue;
                }

                const auto all_merged_nets_str = std::get<1>(typed_net->get_data("parser_annotation", "merged_nets"));

                if (all_merged_nets_str.empty())
                {
                    continue;
                }

                // parse json list of merged net names
                rapidjson::Document doc;
                doc.Parse(all_merged_nets_str.c_str());

                for (u32 i = 0; i < doc.GetArray().Size(); i++)
                {
                    const auto list = doc[i].GetArray();
                    for (u32 j = 0; j < list.Size(); j++)
                    {
                        const auto merged_wire_name = list[j].GetString();

                        const auto merged_wire_name_index = extract_index(merged_wire_name, net_index_pattern, "net_name");
                        if (merged_wire_name_index.has_value())
                        {
                            found_identfiers.push_back(merged_wire_name_index.value());
                        }
                    }
                }
            }

            return found_identfiers;
        }
    }    // namespace

    Result<u32> NetlistPreprocessingPlugin::reconstruct_indexed_ff_identifiers(Netlist* nl)
    {
        u32 counter = 0;
        for (auto& ff : nl->get_gates([](const auto g) { return g->get_type()->has_property(GateTypeProperty::ff); }))
        {
            std::vector<indexed_identifier> all_identifiers;

            // 1) Check whether the ff gate already has an index annotated in its gate name
            const auto cleaned_gate_name = replace_hal_instance_index(ff->get_name());
            const auto gate_name_index   = extract_index(cleaned_gate_name, gate_index_pattern, "gate_name");

            if (gate_name_index.has_value())
            {
                auto found_identifier       = gate_name_index.value();
                found_identifier.identifier = reconstruct_hal_instance_index(found_identifier.identifier);
                all_identifiers.push_back(found_identifier);
            }

            static const std::vector<PinType> relevant_pin_types = {PinType::state, PinType::neg_state, PinType::data};

            // 2) Check all relevant pin_types
            for (const auto& pt : relevant_pin_types)
            {
                const auto found_identifiers = check_net_at_pin(pt, ff);
                all_identifiers.insert(all_identifiers.end(), found_identifiers.begin(), found_identifiers.end());
            }

            if (!all_identifiers.empty())
            {
                counter++;
            }

            annotate_indexed_identifiers(ff, all_identifiers);
        }

        return OK(counter);
    }

    namespace
    {
        struct ComponentData
        {
            std::string name;
            std::string type;
            u64 x;
            u64 y;
        };

        TokenStream<std::string> tokenize(std::stringstream& ss)
        {
            const std::string delimiters = " ;-";
            std::string current_token;
            u32 line_number = 0;

            std::string line;
            bool escaped = false;

            std::vector<Token<std::string>> parsed_tokens;
            while (std::getline(ss, line))
            {
                line_number++;

                for (char c : line)
                {
                    // deal with escaping and strings
                    if (c == '\\')
                    {
                        escaped = true;
                        continue;
                    }
                    else if (escaped && std::isspace(c))
                    {
                        escaped = false;
                        continue;
                    }

                    if (((!std::isspace(c) && delimiters.find(c) == std::string::npos) || escaped))
                    {
                        current_token += c;
                    }
                    else
                    {
                        if (!current_token.empty())
                        {
                            parsed_tokens.emplace_back(line_number, current_token);
                            current_token.clear();
                        }

                        if (!std::isspace(c))
                        {
                            parsed_tokens.emplace_back(line_number, std::string(1, c));
                        }
                    }
                }

                if (!current_token.empty())
                {
                    parsed_tokens.emplace_back(line_number, current_token);
                    current_token.clear();
                }
            }

            return TokenStream(parsed_tokens, {}, {});
        }

        Result<std::unordered_map<std::string, ComponentData>> parse_tokens(TokenStream<std::string>& ts)
        {
            ts.consume_until("COMPONENTS");
            ts.consume("COMPONENTS");
            const auto component_count_str = ts.consume().string;
            ts.consume(";");

            u32 component_count;
            if (const auto res = utils::wrapped_stoul(component_count_str); res.is_ok())
            {
                component_count = res.get();
            }
            else
            {
                return ERR_APPEND(res.get_error(), "could not parse tokens: failed to read component count from token" + component_count_str);
            }

            std::cout << "Component count: " << component_count << std::endl;

            std::unordered_map<std::string, ComponentData> component_data;
            for (u32 c_idx = 0; c_idx < component_count; c_idx++)
            {
                // parse a line
                ComponentData new_data_entry;
                ts.consume("-");
                new_data_entry.name = ts.consume().string;
                new_data_entry.type = ts.consume().string;
                ts.consume("+");
                ts.consume("SOURCE");
                ts.consume("DIST");
                ts.consume("TIMING");
                ts.consume("+");
                ts.consume("PLACED");
                ts.consume("FIXED");
                ts.consume("(");

                if (const auto res = utils::wrapped_stoull(ts.consume().string); res.is_ok())
                {
                    new_data_entry.x = res.get();
                }
                else
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens: failed to read x coordinate from token");
                }

                if (const auto res = utils::wrapped_stoull(ts.consume().string); res.is_ok())
                {
                    new_data_entry.y = res.get();
                }
                else
                {
                    return ERR_APPEND(res.get_error(), "could not parse tokens: failed to read y coordinate from token");
                }

                ts.consume(")");

                ts.consume_current_line();

                component_data.insert({new_data_entry.name, new_data_entry});
            }

            return OK(component_data);
        }
    }    // namespace

    Result<std::monostate> NetlistPreprocessingPlugin::parse_def_file(Netlist* nl, const std::filesystem::path& def_file)
    {
        std::stringstream ss;
        std::ifstream ifs;
        ifs.open(def_file.string(), std::ifstream::in);
        if (!ifs.is_open())
        {
            return ERR("could not parse DEF (Design Exchange Format) file '" + def_file.string() + "' : unable to open file");
        }
        ss << ifs.rdbuf();
        ifs.close();

        auto ts = tokenize(ss);

        std::unordered_map<std::string, ComponentData> component_data;
        // parse tokens
        try
        {
            if (auto res = parse_tokens(ts); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "could not parse Design Exchange Format file '" + def_file.string() + "': unable to parse tokens");
            }
            else
            {
                component_data = res.get();
            }
        }
        catch (TokenStream<std::string>::TokenStreamException& e)
        {
            if (e.line_number != (u32)-1)
            {
                return ERR("could not parse Design Exchange Format file '" + def_file.string() + "': " + e.message + " (line " + std::to_string(e.line_number) + ")");
            }
            else
            {
                return ERR("could not parse Design Exchange Format file '" + def_file.string() + "': " + e.message);
            }
        }

        std::unordered_map<std::string, Gate*> name_to_gate;
        for (auto g : nl->get_gates())
        {
            name_to_gate.insert({g->get_name(), g});
        }

        u32 counter = 0;
        for (const auto& [gate_name, data] : component_data)
        {
            if (const auto& g_it = name_to_gate.find(gate_name); g_it != name_to_gate.end())
            {
                // TODO figure out whereever we are saving coordinates now...
                g_it->second->set_location_x(data.x);
                g_it->second->set_location_y(data.y);

                counter++;
            }
        }

        log_info("netlist_preprocessing", "reconstructed coordinates for {} / {} ({:.2}) gates", counter, nl->get_gates().size(), (double)counter / (double)nl->get_gates().size());

        return OK({});
    }

}    // namespace hal
