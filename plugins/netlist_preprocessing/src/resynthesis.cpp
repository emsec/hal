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
#include "netlist_preprocessing/helper_lib.h"
#include "netlist_preprocessing/plugin_netlist_preprocessing.h"
#include "netlist_preprocessing/utils/json.hpp"
#include "rapidjson/document.h"

#include <fstream>
#include <queue>
#include <regex>

namespace hal
{
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

            return ERR("could not query binary path: no binary found for ABC logic synthesis tool");
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
        std::string new_net_name(const Net* dst_net, const Net* new_net)
        {
            return new_net->get_name() + "_" + std::to_string(dst_net->get_id()) + "_NEW_NET";
        }

        std::string new_gate_name(const Gate* dst_gate, const Gate* new_gate)
        {
            return new_gate->get_name() + "_" + std::to_string(dst_gate->get_id()) + "_NEW_GATE";
        }

        Result<std::monostate> delete_subgraph(Netlist* nl, const std::vector<Gate*> subgraph)
        {
            // TODO currently only gates are deleted, not nets...
            for (const auto& g : subgraph)
            {
                if (!nl->delete_gate(g))
                {
                    return ERR("unable to delete subgraph: failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " in destination netlist");
                }
            }

            return OK({});
        }

        // NOTE there are about a hundred more checks that we could do here
        Result<std::monostate> replace_subgraph_with_netlist(const std::vector<Gate*>& subgraph,
                                                             const std::unordered_map<Net*, std::vector<Net*>>& global_io_mapping,
                                                             const Netlist* src_nl,
                                                             Netlist* dst_nl,
                                                             const bool delete_subgraph_gates)
        {
            std::unordered_map<std::string, Gate*> gate_name_to_gate;

            const auto dst_gl = dst_nl->get_gate_library();

            // add all gates of the source netlist to the destination netlist
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

                // edge case for global inputs
                if (src_n->is_global_input_net())
                {
                    if (const auto it = global_io_mapping.find(src_n); it != global_io_mapping.end())
                    {
                        const auto& net_connections = global_io_mapping.at(src_n);
                        if (net_connections.size() != 1)
                        {
                            return ERR("unable to replace subgraph with netlist: found " + std::to_string(net_connections.size()) + " net connections to the global input " + src_n->get_name()
                                       + ", this would lead to mutlti driven nets!");
                        }
                        new_net = net_connections.front();
                    }
                    else
                    {
                        return ERR("unable to replace subgraph with netlist: failed to locate mapped net in destination netlist for global io net " + src_n->get_name() + " with ID "
                                   + std::to_string(src_n->get_id()));
                    }
                }
                else if (src_n->is_global_output_net())
                {
                    if (const auto it = global_io_mapping.find(src_n); it != global_io_mapping.end())
                    {
                        const auto& net_connections = global_io_mapping.at(src_n);
                        new_net                     = net_connections.front();

                        // if a single global output leads to multiple nets in the dst netlist, that means that the nets are functionally equivalent and we can connect/merge them.
                        for (u32 i = 1; i < net_connections.size(); i++)
                        {
                            const auto& res = NetlistModificationDecorator(*dst_nl).connect_nets(new_net, net_connections.at(i));
                            if (res.is_error())
                            {
                                return ERR("unable to replace subgraph with netlist: failed to connect/merge all the net connections of net " + src_n->get_name() + " with ID "
                                           + std::to_string(src_n->get_id()));
                            }
                        }
                    }
                    else
                    {
                        return ERR("unable to replace subgraph with netlist: failed to locate mapped net in destination netlist for global io net " + src_n->get_name() + " with ID "
                                   + std::to_string(src_n->get_id()));
                    }
                }
                else if (src_n->is_gnd_net())
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
                else
                {
                    new_net                    = dst_nl->create_net("TEMP");
                    const std::string new_name = new_net_name(new_net, src_n);
                    new_net->set_name(new_name);
                }

                // connect net to sources
                for (const auto src_ep : src_n->get_sources())
                {
                    const auto org_src_name     = src_ep->get_gate()->get_name();
                    const auto org_src_pin_name = src_ep->get_pin()->get_name();
                    auto new_src_g              = gate_name_to_gate.at(org_src_name);
                    if (!new_net->add_source(new_src_g, org_src_pin_name))
                    {
                        return ERR("unable to replace subgraph with netlist: failed to add gate " + new_src_g->get_name() + " with ID " + std::to_string(new_src_g->get_id()) + " at pin "
                                   + org_src_pin_name + " as new source to net " + new_net->get_name() + " with ID " + std::to_string(new_net->get_id()));
                    }
                }

                // connect net to destinations
                for (const auto src_ep : src_n->get_destinations())
                {
                    const auto org_dst_name     = src_ep->get_gate()->get_name();
                    const auto org_dst_pin_name = src_ep->get_pin()->get_name();
                    auto new_dst_g              = gate_name_to_gate.at(org_dst_name);
                    if (!new_net->add_destination(new_dst_g, org_dst_pin_name))
                    {
                        return ERR("unable to replace subgraph with netlist: failed to add gate " + new_dst_g->get_name() + " with ID " + std::to_string(new_dst_g->get_id()) + " at pin "
                                   + org_dst_pin_name + " as new destination to net " + new_net->get_name() + " with ID " + std::to_string(new_net->get_id()));
                    }
                }
            }

            // delete subgraph gates if flag is set
            if (delete_subgraph_gates)
            {
                auto delete_res = delete_subgraph(dst_nl, subgraph);
                if (delete_res.is_error())
                {
                    return ERR_APPEND(delete_res.get_error(), "unable to replace subgraph with netlist: failed to delete subgraph");
                }
            }

            return OK({});
        }

        Result<std::monostate> replace_gate_with_netlist(Gate* g, const Netlist* src_nl, Netlist* dst_nl, const bool delete_gate = true)
        {
            std::unordered_map<Net*, std::vector<Net*>> global_io_mapping;

            for (const auto& g_i : src_nl->get_top_module()->get_input_pins())
            {
                const auto& pin_name = g_i->get_name();
                const auto i_net     = g->get_fan_in_net(pin_name);

                if (i_net == nullptr)
                {
                    return ERR("unable to replace gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " with netlist: failed to find net connected to gate pin " + pin_name);
                }

                global_io_mapping[g_i->get_net()].push_back(i_net);
            }

            for (const auto& g_o : src_nl->get_top_module()->get_output_pins())
            {
                const auto& pin_name = g_o->get_name();
                const auto o_net     = g->get_fan_out_net(pin_name);

                if (o_net == nullptr)
                {
                    return ERR("unable to replace gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " with netlist: failed to find net connected to gate pin " + pin_name);
                }

                global_io_mapping[g_o->get_net()].push_back(o_net);
            }

            return replace_subgraph_with_netlist({g}, global_io_mapping, src_nl, dst_nl, delete_gate);
        }
    }    // namespace

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

        Result<std::unique_ptr<Netlist>> generate_decomposed_netlist_from_boolean_function(const std::vector<std::pair<std::string, BooleanFunction>>& bfs, const GateLibrary* gl)
        {
            auto nl = netlist_factory::create_netlist(gl);

            std::map<std::string, Net*> var_name_to_net;
            for (const auto& [out_pin_name, bf] : bfs)
            {
                const auto bf_vars = bf.get_variable_names();
                for (const auto& var : bf_vars)
                {
                    if (var_name_to_net.find(var) != var_name_to_net.end())
                    {
                        continue;
                    }

                    Net* new_net = nl->create_net(var);
                    if (!new_net)
                    {
                        return ERR("unable to generate decomposed netlist from boolean function: failed to create net for boolean input var " + var);
                    }

                    new_net->mark_global_input_net();
                    var_name_to_net.insert({var, new_net});
                }

                auto new_out_net_res = build_gate_tree_from_boolean_function(nl.get(), bf, var_name_to_net, nullptr);
                if (new_out_net_res.is_error())
                {
                    return ERR_APPEND(new_out_net_res.get_error(), "unable to generate decomposed netlist from boolean function: failed to build gate tree for boolean function");
                }

                Net* new_out_net = new_out_net_res.get();
                new_out_net->mark_global_output_net();
                new_out_net->set_name(out_pin_name);

                // rename top module pins to the var names.
                // NOTE: this seems to be really depended on the order. Doing this earlier causes a crash. I did not fully understand what causes the top module pins to be created.
                for (const auto& [var, in_net] : var_name_to_net)
                {
                    auto in_pin = nl->get_top_module()->get_pin_by_net(in_net);
                    in_pin->set_name(var);
                }
                auto out_pin = nl->get_top_module()->get_pin_by_net(new_out_net);
                out_pin->set_name(out_pin_name);
            }

            return OK(std::move(nl));
        }

    }    // namespace

    Result<std::monostate> NetlistPreprocessingPlugin::decompose_gate(Netlist* nl, Gate* g, const bool delete_gate)
    {
        // build Boolean function for each output pin of the gate

        // TODO use vector of pairs
        std::vector<std::pair<std::string, BooleanFunction>> output_pin_name_to_bf;
        for (const auto pin : g->get_type()->get_output_pins())
        {
            const auto bf_res = g->get_resolved_boolean_function(pin, true);
            if (bf_res.is_error())
            {
                return ERR_APPEND(bf_res.get_error(),
                                  "unable to decompose gate" + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to build resolved boolean function for pin " + pin->get_name());
            }

            const auto bf = bf_res.get();
            output_pin_name_to_bf.push_back({pin->get_name(), bf});
        }

        auto resynth_res = generate_decomposed_netlist_from_boolean_function(output_pin_name_to_bf, nl->get_gate_library());
        if (resynth_res.is_error())
        {
            return ERR_APPEND(resynth_res.get_error(), "unable to decompose gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to generate decomposed netlist");
        }
        auto unique_resynth_nl = resynth_res.get();

        const auto replace_res = replace_gate_with_netlist(g, unique_resynth_nl.get(), nl, false);
        if (replace_res.is_error())
        {
            return ERR_APPEND(replace_res.get_error(), "unable to decompose  gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failed to replace gate with descomposed netlist");
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
        std::map<std::pair<const GateType*, std::vector<std::string>>, std::unique_ptr<Netlist>> gt_to_decomposed;

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
                        return ERR_APPEND(init_res.get_error(), "unable to decompose gates of type: failed to get init string from gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }
                    const auto init_vec = init_res.get();
                    if (init_vec.size() != 1)
                    {
                        return ERR("unable tor decompose gates of type: got " + std::to_string(init_vec.size()) + " init strings for gate" + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    init_data = init_vec;
                }

                // get resynth netlist from cache or create it
                if (const auto it = gt_to_decomposed.find({gt, init_data}); it != gt_to_decomposed.end())
                {
                    resynth_nl = it->second.get();
                }
                else
                {
                    // build Boolean function for each output pin of the gate type

                    // TODO use vector of pairs
                    std::vector<std::pair<std::string, BooleanFunction>> output_pin_name_to_bf;
                    for (const auto pin : g->get_type()->get_output_pins())
                    {
                        const auto bf_res = g->get_resolved_boolean_function(pin, true);
                        if (bf_res.is_error())
                        {
                            return ERR_APPEND(bf_res.get_error(),
                                              "unable to decompose gate type " + g->get_type()->get_name() + " for gate instance " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                                  + ": failed to build resolved boolean function for pin " + pin->get_name());
                        }

                        const auto bf = bf_res.get();
                        output_pin_name_to_bf.push_back({pin->get_name(), bf});
                    }

                    auto resynth_res = generate_decomposed_netlist_from_boolean_function(output_pin_name_to_bf, nl->get_gate_library());
                    if (resynth_res.is_error())
                    {
                        return ERR_APPEND(resynth_res.get_error(), "unable to decompose gates of type: failed to generate decomposed netlist for gate type " + gt->get_name());
                    }
                    auto unique_resynth_nl = resynth_res.get();
                    resynth_nl             = unique_resynth_nl.get();

                    gt_to_decomposed.insert({std::make_pair(gt, init_data), std::move(unique_resynth_nl)});
                }

                const auto replace_res = replace_gate_with_netlist(g, resynth_nl, nl, false);
                if (replace_res.is_error())
                {
                    return ERR_APPEND(replace_res.get_error(),
                                      "unable to decompose gates of type " + gt->get_name() + ": failed to replace gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
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

    namespace
    {
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
                                                                                        GateLibrary* target_lib,
                                                                                        const bool optimize_area)
        {
            const auto verilog_module = build_functional_verilog_module_from(bfs);

            auto base_path_res = utils::get_unique_temp_directory("resynthesis/");
            if (base_path_res.is_error())
            {
                return ERR_APPEND(base_path_res.get_error(), "unable to resynthesize boolean functions with yosys: failed to get unique temp directory");
            }
            const std::filesystem::path base_path                  = base_path_res.get();
            const std::filesystem::path functional_netlist_path    = base_path / "func_netlist.v";
            const std::filesystem::path resynthesized_netlist_path = base_path / "resynth_netlist.v";

            std::filesystem::create_directory(base_path);

            std::ofstream out(functional_netlist_path);
            out << verilog_module;
            out.close();

            auto yosys_query_res = yosys::query_binary_path();
            if (yosys_query_res.is_error())
            {
                return ERR_APPEND(yosys_query_res.get_error(), "unable to resynthesize boolean functions with yosys: failed to find yosys path");
            }

            const auto yosys_path     = yosys_query_res.get();
            const std::string command = yosys_path + " -q -p " + "\"read -sv " + functional_netlist_path.string() + "; hierarchy -top top; proc; fsm; opt; memory; opt; techmap; opt; abc -genlib "
                                        + genlib_path.string() + "; " + "write_verilog " + resynthesized_netlist_path.string() + "; clean\"";

            // TODO check again the proper way to start a subprocess
            system(command.c_str());

            auto resynth_nl = netlist_factory::load_netlist(resynthesized_netlist_path, target_lib);

            if (resynth_nl == nullptr)
            {
                return ERR("Unable to resynthesize boolean functions with yosys: failed to load resynthesized netlist at " + resynthesized_netlist_path.string());
            }

            // TODO check whether this is needed here or maybe move this somewhere else
            // yosys workaround for stupid net renaming
            for (const auto& pin : resynth_nl->get_top_module()->get_input_pins())
            {
                auto net = pin->get_net();
                net->set_name(pin->get_name());
            }

            // delete the created directory and the contained files
            std::filesystem::remove_all(base_path);

            return OK(std::move(resynth_nl));
        }

        Result<std::unique_ptr<Netlist>> generate_resynth_netlist_for_gate_level_subgraph(const Netlist* nl,
                                                                                          const std::vector<Gate*>& subgraph,
                                                                                          const std::filesystem::path& genlib_path,
                                                                                          GateLibrary* target_lib,
                                                                                          const bool optimize_area)
        {
            // TODO sanity check wether all gates in the subgraph have types that are in the genlib/target library
            if (nl == nullptr)
            {
                return ERR("unable to resynthesize gate level subgraph: netlist is a nullptr");
            }

            if (target_lib == nullptr)
            {
                return ERR("unable to resynthesize gate level subgraph: gate library is a nullptr");
            }

            auto subgraph_nl_res = SubgraphNetlistDecorator(*nl).copy_subgraph_netlist(subgraph, true);
            if (subgraph_nl_res.is_error())
            {
                return ERR_APPEND(subgraph_nl_res.get_error(), "unable to resynthesize gate level subgraph: failed to copy subgraph netlist");
            }
            const auto subgraph_nl = subgraph_nl_res.get();

            auto base_path_res = utils::get_unique_temp_directory("resynthesis/");
            if (base_path_res.is_error())
            {
                return ERR_APPEND(base_path_res.get_error(), "unable to resynthesize boolean functions with yosys: failed to get unique temp directory");
            }
            const std::filesystem::path base_path                  = base_path_res.get();
            const std::filesystem::path org_netlist_path           = base_path / "org_netlist.v";
            const std::filesystem::path resynthesized_netlist_path = base_path / "resynth_netlist.v";
            const std::filesystem::path yosys_helper_lib_path      = base_path / "yosys_helper_lib.v";

            std::filesystem::create_directory(base_path);

            if (!netlist_writer_manager::write(subgraph_nl.get(), org_netlist_path))
            {
                return ERR("unable to resynthesize gate level subgraph: failed to write netlist to file");
            }

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

            std::ofstream yosys_helper_lib_file(yosys_helper_lib_path);
            yosys_helper_lib_file << get_yosys_helper_lib();
            yosys_helper_lib_file.close();

            auto yosys_query_res = yosys::query_binary_path();
            if (yosys_query_res.is_error())
            {
                return ERR_APPEND(yosys_query_res.get_error(), "Unable to resynthesize boolean functions with yosys: failed to find yosys path");
            }

            const auto yosys_path     = yosys_query_res.get();
            const std::string command = yosys_path + " -q -p " + '"' + "read_verilog -sv " + org_netlist_path.string() + "; read_verilog " + yosys_helper_lib_path.string() + "; synth -flatten -top "
                                        + subgraph_nl->get_design_name() + "; abc -genlib " + genlib_path.string() + "; " + "write_verilog " + resynthesized_netlist_path.string() + ";" + '"';

            // log_debug("netlist_preprocessing", "yosys command: {}", command);

            system(command.c_str());

            auto resynth_nl = netlist_factory::load_netlist(resynthesized_netlist_path, target_lib);
            if (resynth_nl == nullptr)
            {
                return ERR("Unable to resynthesize gate level netlist with yosys: failed to load resynthesized netlist at " + resynthesized_netlist_path.string());
            }

            // delete the created directory and the contained files
            std::filesystem::remove_all(base_path);

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
                                                                                          GateLibrary* target_lib,
                                                                                          const bool optimize_area)
        {
            const auto outputs = get_outputs_of_subgraph(subgraph);

            std::unordered_map<std::string, BooleanFunction> bfs;
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

            return generate_resynth_netlist_for_boolean_functions(bfs, genlib_path, target_lib, optimize_area);
        }

        Result<std::unique_ptr<Netlist>> generate_resynth_netlist_for_gate(const Gate* g, GateLibrary* target_lib, const std::filesystem::path& genlib_path)
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

            auto resynth_res = generate_resynth_netlist_for_boolean_functions(output_pin_name_to_bf, genlib_path, target_lib, true);
            if (resynth_res.is_error())
            {
                return ERR_APPEND(resynth_res.get_error(),
                                  "unable to resynthesize lut type " + g->get_type()->get_name() + " for gate instance " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                      + ": failed to resynthesize boolean functions of gate");
            }

            return OK(resynth_res.get());
        }
    }    // namespace

    Result<std::monostate> NetlistPreprocessingPlugin::resynthesize_gate(Netlist* nl, Gate* g, GateLibrary* target_lib, const std::filesystem::path& genlib_path, const bool delete_gate)
    {
        if (g == nullptr)
        {
            return ERR("no valid gate selected (gate is nullptr)");
        }

        if (target_lib == nullptr)
        {
            return ERR("no valid target_lib selected (target_lib is nullptr)");
        }

        if (!utils::file_exists(genlib_path.string()))
        {
            return ERR("genlib does not exist");
        }

        auto resynth_res = generate_resynth_netlist_for_gate(g, target_lib, genlib_path);
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

    Result<u32> NetlistPreprocessingPlugin::resynthesize_gates(Netlist* nl, const std::vector<Gate*>& gates, GateLibrary* target_gl)
    {
        auto base_path_res = utils::get_unique_temp_directory("resynthesis/");
        if (base_path_res.is_error())
        {
            return ERR_APPEND(base_path_res.get_error(), "unable to resynthesize boolean functions with yosys: failed to get unique temp directory");
        }
        const std::filesystem::path base_path   = base_path_res.get();
        const std::filesystem::path genlib_path = base_path / "new_gate_library.genlib";
        std::filesystem::create_directory(base_path);

        const auto gl_save_res = gate_library_manager::save(genlib_path, target_gl, true);
        if (!gl_save_res)
        {
            return ERR("unable to resynthesize gates of type: failed to save gate library " + target_gl->get_name() + " to location " + genlib_path.string());
        }

        std::map<std::pair<const GateType*, std::vector<std::string>>, std::unique_ptr<Netlist>> gt_to_resynth;

        std::vector<Gate*> to_delete;
        u32 counter = 0;
        for (const auto& g : gates)
        {
            const auto& gt = g->get_type();

            Netlist* resynth_nl                = nullptr;
            std::vector<std::string> init_data = {};

            if (gt->has_property(GateTypeProperty::c_lut))
            {
                const auto init_res = g->get_init_data();
                if (init_res.is_error())
                {
                    return ERR_APPEND(init_res.get_error(), "unable to resynthesize gates of type: failed to get init string from gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                }
                const auto init_vec = init_res.get();
                if (init_vec.size() != 1)
                {
                    return ERR("unable tor resynthesize gates of type: got " + std::to_string(init_vec.size()) + " init strings for gate" + g->get_name() + " with ID " + std::to_string(g->get_id()));
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
                return ERR_APPEND(replace_res.get_error(), "unable to resynthesize gates of type " + gt->get_name() + ": failed for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
            }
            to_delete.push_back(g);
        }

        for (const auto& g : to_delete)
        {
            counter += 1;
            if (!nl->delete_gate(g))
            {
                return ERR("unable to resynthesize gates of type " + g->get_type()->get_name() + ": failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
            }
        }

        // delete the created directory and the contained files
        std::filesystem::remove_all(base_path);

        return OK(counter);
    }

    Result<u32> NetlistPreprocessingPlugin::resynthesize_gates_of_type(Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl)
    {
        const std::vector<Gate*> filtered_gates = nl->get_gates([gate_types](const auto& gate) {
            bool is_included = false;
            for (const auto& gt : gate_types)
            {
                if (gate->get_type() == gt)
                {
                    is_included = true;
                    break;
                }
            }

            return is_included;
        });

        return resynthesize_gates(nl, filtered_gates, target_gl);
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

        auto base_path_res = utils::get_unique_temp_directory("resynthesis/");
        if (base_path_res.is_error())
        {
            return ERR_APPEND(base_path_res.get_error(), "unable to resynthesize boolean functions with yosys: failed to get unique temp directory");
        }
        const std::filesystem::path base_path   = base_path_res.get();
        const std::filesystem::path genlib_path = base_path / "new_gate_library.genlib";
        std::filesystem::create_directory(base_path);

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

        std::unordered_map<Net*, std::vector<Net*>> global_io_mapping;

        // use top module pin names to find correponding nets in original netlist
        for (const auto& pin : resynth_nl->get_top_module()->get_input_pins())
        {
            auto net_it = name_to_net.find(pin->get_name());
            if (net_it == name_to_net.end())
            {
                return ERR("unable to resynthesize subgraphs of type: failed to locate net in destination netlist from global input " + pin->get_name() + " in resynthesized netlist");
            }
            global_io_mapping[pin->get_net()].push_back(net_it->second);
        }
        for (const auto& pin : resynth_nl->get_top_module()->get_output_pins())
        {
            auto net_it = name_to_net.find(pin->get_name());
            if (net_it == name_to_net.end())
            {
                return ERR("unable to resynthesize subgraphs of type: failed to locate net in destination netlist from global output " + pin->get_name() + " in resynthesized netlist");
            }
            global_io_mapping[pin->get_net()].push_back(net_it->second);
        }

        auto replace_res = replace_subgraph_with_netlist(subgraph, global_io_mapping, resynth_nl.get(), nl, false);
        if (replace_res.is_error())
        {
            return ERR_APPEND(replace_res.get_error(), "unable to resynthesize subgraphs of type: failed to replace subgrap with resynthesized netlist");
        }

        // delete subgraph gates
        auto delete_res = delete_subgraph(nl, subgraph);
        if (delete_res.is_error())
        {
            return ERR_APPEND(delete_res.get_error(), "unable to replace subgraph with netlist: failed to delete subgraph");
        }

        // delete the created directory and the contained files
        std::filesystem::remove_all(base_path);

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

        return resynthesize_subgraph(nl, subgraph, target_gl);
    }

    // The following is MUX optimizations that do not really belong here but use some of the resynth functions that are not exposed outside this file
    namespace
    {
        Result<u32> remove_encasing_inverters(Netlist* nl)
        {
            // check whether all inputs and output are inverted -> remove all inverters

            // TODO: this only considers HAL muxes, but i do not see a reason why. There is no resynthesis happening here
            std::vector<Gate*> muxes = nl->get_gates([](const Gate* g) { return (g->get_type()->get_name().find("HAL_MUX") != std::string::npos); });

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

        struct MuxFingerprint
        {
            GateType* type;
            std::set<GatePin*> inverters;

            bool operator<(const MuxFingerprint& other) const
            {
                return (other.type < type) || (other.type == type && other.inverters < inverters);
            }
        };

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

            auto base_path_res = utils::get_unique_temp_directory("resynthesis/");
            if (base_path_res.is_error())
            {
                return ERR_APPEND(base_path_res.get_error(), "unable to resynthesize boolean functions with yosys: failed to get unique temp directory");
            }
            const std::filesystem::path base_path   = base_path_res.get();
            const std::filesystem::path genlib_path = base_path / "mux_inv.genlib";
            std::filesystem::create_directory(base_path);

            const auto gl_save_res = gate_library_manager::save(genlib_path, mux_inv_gl, true);
            if (!gl_save_res)
            {
                return ERR("unable to unify muxe select signals: failed to save gate library " + mux_inv_gl->get_name() + " to location " + genlib_path.string());
            }

            const i64 initial_size = nl->get_gates().size();

            // resynthesize all muxes where any select signal is preceded by an inverter hoping to unify the structure with regards to other muxes conntected to the same select signal

            // TODO: as long as resynthezising the subgraph this can only consider HAL muxes
            std::vector<Gate*> muxes = nl->get_gates([](const Gate* g) { return (g->get_type()->get_name().find("HAL_MUX") != std::string::npos); });

            std::map<MuxFingerprint, std::unique_ptr<Netlist>> resynth_cache;

            for (const auto& g : muxes)
            {
                // MUX fingerprint for caching resynthesis results
                MuxFingerprint mf;
                mf.type = g->get_type();

                // mapping from MUX select pins to either the input net of the preceding inverter or the net directly connected to the select pin
                std::map<GatePin*, Net*> pin_to_input;

                auto select_pins = g->get_type()->get_pins([](const GatePin* pin) { return (pin->get_type() == PinType::select) && (pin->get_direction() == PinDirection::input); });

                std::vector<Gate*> preceding_inverters;
                for (const auto& pin : g->get_type()->get_input_pins())
                {
                    const auto pred      = g->get_predecessor(pin);
                    const auto is_select = (std::find(select_pins.begin(), select_pins.end(), pin) != select_pins.end());
                    if (!is_select || pred == nullptr || pred->get_gate() == nullptr || !pred->get_gate()->get_type()->has_property(GateTypeProperty::c_inverter)
                        || (pred->get_gate()->get_fan_in_endpoints().size() != 1))
                    {
                        pin_to_input.insert({pin, g->get_fan_in_net(pin)});
                    }
                    else
                    {
                        auto inv_gate = pred->get_gate();
                        preceding_inverters.push_back(inv_gate);
                        pin_to_input.insert({pin, inv_gate->get_fan_in_endpoints().front()->get_net()});
                        mf.inverters.insert(pin);
                    }
                }

                // if there is at least one inverter in front of the mux gate we build a subgraph containing all inverters and the mux gate and resynthesize
                if (!preceding_inverters.empty())
                {
                    const Netlist* resynth_nl;

                    auto subgraph = preceding_inverters;
                    subgraph.push_back(g);

                    // try to use cached resynth netlist
                    if (const auto it = resynth_cache.find(mf); it == resynth_cache.end())
                    {
                        std::unordered_map<std::string, BooleanFunction> bfs;
                        for (const auto& ep : g->get_fan_out_endpoints())
                        {
                            const auto bf_res = SubgraphNetlistDecorator(*nl).get_subgraph_function(subgraph, ep->get_net());
                            if (bf_res.is_error())
                            {
                                return ERR_APPEND(bf_res.get_error(),
                                                  "unable to unify muxes select signals: failed to build boolean function for mux " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                                      + "  at output " + ep->get_pin()->get_name());
                            }
                            auto bf = bf_res.get();

                            // replace all net id vars with generic vaiables refering to their connectivity to the mux
                            for (const auto& [pin, net] : pin_to_input)
                            {
                                auto sub_res = bf.substitute(BooleanFunctionNetDecorator(*net).get_boolean_variable_name(), BooleanFunction::Var(pin->get_name(), 1));
                                if (sub_res.is_error())
                                {
                                    return ERR_APPEND(sub_res.get_error(), "unable to unify muxes select signals: failed to substitute net_id variable with generic variable");
                                }
                                bf = sub_res.get();
                            }

                            bfs.insert({ep->get_pin()->get_name(), std::move(bf)});
                        }

                        auto resynth_res = generate_resynth_netlist_for_boolean_functions(bfs, genlib_path, mux_inv_gl, true);
                        if (resynth_res.is_error())
                        {
                            return ERR_APPEND(resynth_res.get_error(), "unable to unify  select signals of muxes: failed to resynthesize mux subgraph to netlist");
                        }
                        auto unique_resynth_nl = resynth_res.get();
                        resynth_nl             = unique_resynth_nl.get();
                        resynth_cache.insert({mf, std::move(unique_resynth_nl)});
                    }
                    else
                    {
                        resynth_nl = it->second.get();
                    }

                    std::unordered_map<Net*, std::vector<Net*>> global_io_mapping;

                    // use top module pin names to find correponding nets in original netlist
                    for (const auto& pin : resynth_nl->get_top_module()->get_input_pins())
                    {
                        auto net_it = pin_to_input.find(g->get_type()->get_pin_by_name(pin->get_name()));
                        if (net_it == pin_to_input.end())
                        {
                            return ERR("unable to unify muxes select signals:: failed to locate net in destination netlist from global input " + pin->get_name() + " in resynthesized netlist");
                        }
                        global_io_mapping[pin->get_net()].push_back(net_it->second);
                    }
                    for (const auto& pin : resynth_nl->get_top_module()->get_output_pins())
                    {
                        auto net = g->get_fan_out_net(pin->get_name());
                        if (net == nullptr)
                        {
                            return ERR("unable to unify muxes select signals:: failed to locate net in destination netlist from global output " + pin->get_name() + " in resynthesized netlist");
                        }
                        global_io_mapping[pin->get_net()].push_back(net);
                    }

                    auto replace_res = replace_subgraph_with_netlist(subgraph, global_io_mapping, resynth_nl, nl, false);
                    if (replace_res.is_error())
                    {
                        return ERR("unable to unify muxes select signals: failed to replace mux subgraph with resynthesized netlist");
                    }

                    // delete old subgraph gates that only fed into the mux
                    std::vector<Gate*> to_delete;
                    for (const auto g : subgraph)
                    {
                        bool has_no_outside_destinations   = true;
                        bool has_only_outside_destinations = true;
                        for (const auto& suc : g->get_successors())
                        {
                            const auto it = std::find(subgraph.begin(), subgraph.end(), suc->get_gate());
                            if (it == subgraph.end())
                            {
                                has_no_outside_destinations = false;
                            }

                            if (it != subgraph.end())
                            {
                                has_only_outside_destinations = false;
                            }
                        }

                        if (has_no_outside_destinations || has_only_outside_destinations)
                        {
                            to_delete.push_back(g);
                        }
                    }

                    for (const auto& g : to_delete)
                    {
                        if (!nl->delete_gate(g))
                        {
                            return ERR("unable to unify muxes select signals: failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " in destination netlist");
                        }
                    }
                }
            }

            // delete the created directory and the contained files
            std::filesystem::remove_all(base_path);

            const i64 new_size   = nl->get_gates().size();
            const i64 difference = std::abs(initial_size - new_size);

            return OK(u32(difference));
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

}    // namespace hal