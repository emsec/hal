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
#include "netlist_preprocessing/utils/json.hpp"
#include "rapidjson/document.h"
#include "z3_utils.h"

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

            const auto bf_replaced_res = BooleanFunctionDecorator(bf_org).substitute_power_ground_pins(g);
            if (bf_replaced_res.is_error())
            {
                return ERR_APPEND(bf_replaced_res.get_error(),
                                  "cannot simplify LUT inits: failed to replace power and ground pins for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
            }
            const auto bf_replaced   = bf_replaced_res.get();
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
            const auto substitute_res = BooleanFunctionDecorator(func).substitute_power_ground_pins(gate);
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
            // TODO this functionality is not a buffer and is covered by propagate_constants
            /*
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
            */
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

    Result<u32> NetlistPreprocessingPlugin::remove_unconnected_looped(Netlist* nl)
    {
        u32 total_removed = 0;

        while (true)
        {
            auto gate_res = NetlistPreprocessingPlugin::remove_unconnected_gates(nl);
            if (gate_res.is_error())
            {
                return ERR_APPEND(gate_res.get_error(), "unable to execute clean up loop: failed to remove unconnected gates");
            }

            auto net_res = NetlistPreprocessingPlugin::remove_unconnected_nets(nl);
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

    Result<u32> NetlistPreprocessingPlugin::remove_redundant_gates(Netlist* nl, const std::function<bool(const Gate*)>& filter)
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

    Result<u32> NetlistPreprocessingPlugin::remove_redundant_logic_trees(Netlist* nl)
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
                                          "Unable to remove redundant logic trees: failed to compare net " + n->get_name() + " with ID " + std::to_string(n->get_id()) + " with net " + m->get_name()
                                              + " with ID " + std::to_string(m->get_id()));
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

    Result<u32> NetlistPreprocessingPlugin::propagate_constants(Netlist* nl)
    {
        if (nl == nullptr)
        {
            return ERR("netlist is a nullptr");
        }

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
                            // log_error("netlist_preprocessing", "failed to replace bf {} with constant net because netlist is missing GND gate or VCC gate");
                            return ERR("unable to propagate constants: netlist is missing gnd or vcc net!");
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

            log_debug("netlist_preprocessing", "replaced {} destinations this with power/ground nets this iteration", replaced_dst_count);
            total_replaced_dst_count += replaced_dst_count;
        }

        log_info("netlist_preprocessing", "replaced {} destinations with power/ground nets in total", total_replaced_dst_count);
        return OK(total_replaced_dst_count);
    }

    Result<u32> NetlistPreprocessingPlugin::remove_consecutive_inverters(Netlist* nl)
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

    Result<u32> NetlistPreprocessingPlugin::reconstruct_top_module_pin_groups(Netlist* nl)
    {
        std::map<std::string, std::map<u32, std::vector<ModulePin*>>> pg_name_to_indexed_pins;

        for (const auto& pin : nl->get_top_module()->get_pins())
        {
            auto reconstruct = extract_index(pin->get_name(), net_index_pattern, "");
            if (!reconstruct.has_value())
            {
                continue;
            }

            auto [pg_name, index, _] = reconstruct.value();

            pg_name_to_indexed_pins[pg_name][index].push_back(pin);
        }

        u32 reconstructed_counter = 0;
        for (const auto& [pg_name, indexed_pins] : pg_name_to_indexed_pins)
        {
            std::vector<ModulePin*> ordered_pins;

            bool valid_indices = true;
            // NOTE: since the map already orders the indices from low to high, if we iterate over it we also get the pins in the right order
            for (const auto& [_index, pins] : indexed_pins)
            {
                if (pins.size() > 1)
                {
                    valid_indices = false;
                    break;
                }

                ordered_pins.push_back(pins.front());
            }

            if (!valid_indices)
            {
                continue;
            }

            auto res = nl->get_top_module()->create_pin_group(pg_name, ordered_pins);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "cannot reconstruct top module pin groups: failed to create pin group " + pg_name);
            }

            reconstructed_counter++;
        }

        return OK(reconstructed_counter);
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

    Result<std::vector<Module*>> NetlistPreprocessingPlugin::create_multi_bit_gate_modules(Netlist* nl,
                                                                                           const std::map<std::string, std::map<std::string, std::vector<std::string>>>& concatenated_pin_groups)
    {
        std::vector<Module*> all_modules;
        for (const auto& [gt_name, pin_groups] : concatenated_pin_groups)
        {
            const auto& gt = nl->get_gate_library()->get_gate_type_by_name(gt_name);
            if (gt == nullptr)
            {
                return ERR("unable to create multi bit gate module for gate type " + gt_name + ": failed to find gate type with that name in gate library " + nl->get_gate_library()->get_name());
            }

            for (const auto& g : nl->get_gates([&gt](const auto& g) { return g->get_type() == gt; }))
            {
                auto m = nl->create_module("module_" + g->get_name(), g->get_module(), {g});

                for (const auto& [module_pg_name, gate_pg_names] : pin_groups)
                {
                    std::vector<ModulePin*> module_pins;
                    for (const auto& gate_pg_name : gate_pg_names)
                    {
                        const auto& gate_pg = g->get_type()->get_pin_group_by_name(gate_pg_name);

                        if (gate_pg == nullptr)
                        {
                            return ERR("unable to create multi-bit gate module for gate type " + gt_name + " and pin group " + gate_pg_name + ": failed to find pin group with that name");
                        }

                        std::vector<GatePin*> pin_list = gate_pg->get_pins();
                        if (!gate_pg->is_ascending())
                        {
                            std::reverse(pin_list.begin(), pin_list.end());
                        }

                        for (const auto& gate_pin : pin_list)
                        {
                            const auto net = (gate_pin->get_direction() == PinDirection::output) ? g->get_fan_out_net(gate_pin) : g->get_fan_in_net(gate_pin);
                            if (net == nullptr)
                            {
                                continue;
                            }

                            if (net->is_gnd_net() || net->is_vcc_net())
                            {
                                continue;
                            }

                            const auto module_pin = m->get_pin_by_net(net);

                            module_pins.push_back(module_pin);
                        }
                    }

                    m->create_pin_group(module_pg_name, module_pins);
                    u32 idx_counter = 0;
                    for (const auto& mp : module_pins)
                    {
                        m->set_pin_name(mp, module_pg_name + "_" + std::to_string(idx_counter));
                        idx_counter++;
                    }
                }

                all_modules.push_back(m);
            }
        }

        return OK(all_modules);
    }

    Result<std::vector<Net*>> NetlistPreprocessingPlugin::create_nets_at_unconnected_pins(Netlist* nl)
    {
        std::vector<Net*> created_nets;

        for (const auto& g : nl->get_gates())
        {
            for (const auto& p : g->get_type()->get_output_pins())
            {
                if (g->get_fan_out_net(p) == nullptr)
                {
                    auto new_net = nl->create_net("TEMP");
                    new_net->set_name("HAL_UNCONNECTED_" + std::to_string(new_net->get_id()));
                    new_net->add_source(g, p);

                    created_nets.push_back(new_net);
                }
            }
        }

        return OK(created_nets);
    }

    Result<u32> NetlistPreprocessingPlugin::unify_ff_outputs(Netlist* nl, const std::vector<Gate*>& ffs, GateType* inverter_type)
    {
        if (nl == nullptr)
        {
            return ERR("netlist is a nullptr");
        }

        if (inverter_type == nullptr)
        {
            const auto* gl = nl->get_gate_library();
            const auto inv_types =
                gl->get_gate_types([](const GateType* gt) { return gt->has_property(GateTypeProperty::c_inverter) && gt->get_input_pins().size() == 1 && gt->get_output_pins().size() == 1; });
            if (inv_types.empty())
            {
                return ERR("gate library '" + gl->get_name() + "' of netlist does not contain an inverter gate");
            }
            inverter_type = inv_types.begin()->second;
        }
        else
        {
            if (inverter_type->get_gate_library() != nl->get_gate_library())
            {
                return ERR("inverter gate type '" + inverter_type->get_name() + "' of gate library '" + inverter_type->get_gate_library()->get_name() + "' does not belong to gate library '"
                           + nl->get_gate_library()->get_name() + "' of provided netlist");
            }

            if (!inverter_type->has_property(GateTypeProperty::c_inverter))
            {
                return ERR("gate type '" + inverter_type->get_name() + "' of gate library '" + inverter_type->get_gate_library()->get_name() + "' is not an inverter gate type");
            }

            if (inverter_type->get_input_pins().size() != 1 || inverter_type->get_output_pins().size() != 1)
            {
                return ERR("inverter gate type '" + inverter_type->get_name() + "' of gate library '" + inverter_type->get_gate_library()->get_name()
                           + "' has an invalid number of input pins or output pins");
            }
        }

        auto inv_in_pin  = inverter_type->get_input_pins().front();
        auto inv_out_pin = inverter_type->get_output_pins().front();

        u32 ctr = 0;

        const std::vector<Gate*>& gates = ffs.empty() ? nl->get_gates() : ffs;

        for (auto* ff : gates)
        {
            auto* ff_type = ff->get_type();

            if (!ff_type->has_property(GateTypeProperty::ff))
            {
                continue;
            }

            GatePin* state_pin     = nullptr;
            GatePin* neg_state_pin = nullptr;

            for (auto* o_pin : ff_type->get_output_pins())
            {
                if (o_pin->get_type() == PinType::state)
                {
                    state_pin = o_pin;
                }
                else if (o_pin->get_type() == PinType::neg_state)
                {
                    neg_state_pin = o_pin;
                }
            }

            if (neg_state_pin == nullptr)
            {
                continue;
            }

            auto* neg_state_ep = ff->get_fan_out_endpoint(neg_state_pin);
            if (neg_state_ep == nullptr)
            {
                continue;
            }
            auto* neg_state_net = neg_state_ep->get_net();

            auto state_net = ff->get_fan_out_net(state_pin);
            if (state_net == nullptr)
            {
                state_net = nl->create_net(ff->get_name() + "__STATE_NET__");
                state_net->add_source(ff, state_pin);
            }

            auto* inv = nl->create_gate(inverter_type, ff->get_name() + "__NEG_STATE_INVERT__");
            state_net->add_destination(inv, inv_in_pin);
            neg_state_net->remove_source(neg_state_ep);
            neg_state_net->add_source(inv, inv_out_pin);
            ctr++;
        }

        return OK(ctr);
    }

}    // namespace hal