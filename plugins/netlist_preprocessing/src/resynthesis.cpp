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
            return new_net->get_name() + "_" + std::to_string(dst_net->get_id()) + "_RESYNTH_NET";
        }

        std::string new_gate_name(const Gate* dst_gate, const Gate* new_gate)
        {
            return new_gate->get_name() + "_" + std::to_string(dst_gate->get_id()) + "_RESYNTH_GATE";
        }

        Result<std::monostate> delete_subgraph(Netlist* nl, const std::vector<Gate*>& subgraph)
        {
            // TODO currently only gates are deleted, not nets...
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
                //log_info("netlist_preprocessing", "deleting gate: {}", g->get_name());
                if (!nl->delete_gate(g))
                {
                    return ERR("unable to delete subgraph: failed to delete gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " in destination netlist");
                }
            }

            return OK({});
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
                        if (!new_net->add_source(new_src_g, org_src_pin_name))
                        {
                            return ERR("unable to replace subgraph with netlist: failed to add gate " + new_src_g->get_name() + " with ID " + std::to_string(new_src_g->get_id()) + " at pin "
                                       + org_src_pin_name + " as new source to net " + new_net->get_name() + " with ID " + std::to_string(new_net->get_id()));
                        }
                    }
                }

                // connect net to destinations
                if (src_n->get_destinations().size() > 0)
                {
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

            auto log_man = LogManager::get_instance();
            log_man->deactivate_channel("info");
            log_man->deactivate_channel("warning");
            auto resynth_nl = netlist_factory::load_netlist(resynthesized_netlist_path, hgl_lib);
            log_man->activate_channel("info");
            log_man->activate_channel("warning");

            if (resynth_nl == nullptr)
            {
                return ERR("Unable to resynthesize boolean functions with yosys: failed to load resynthesized netlist at " + resynthesized_netlist_path.string());
            }

            // yosys workaround for stupid net renaming
            for (const auto& pin : resynth_nl->get_top_module()->get_input_pins())
            {
                auto net = pin->get_net();
                net->set_name(pin->get_name());
                log_debug("netlist_preprocessing", "renamed net {} with pin name {}", net->get_name(), pin->get_name());
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
                return ERR("unable to resynthesize gate level subgraph: netlist is a nullptr");
            }

            if (hgl_lib == nullptr)
            {
                return ERR("unable to resynthesize gate level subgraph: gate library is a nullptr");
            }

            auto subgraph_nl_res = SubgraphNetlistDecorator(*nl).copy_subgraph_netlist(subgraph);
            if (subgraph_nl_res.is_error())
            {
                return ERR_APPEND(subgraph_nl_res.get_error(), "unable to resynthesize gate level subgraph: failed to copy subgraph netlist");
            }
            const auto subgraph_nl = subgraph_nl_res.get();

            // TODO make this more robust
            const std::filesystem::path base_path                  = std::filesystem::temp_directory_path() / "resynthesize_subgraph_with_yosys";
            const std::filesystem::path org_netlist_path           = base_path / "org_netlist.v";
            const std::filesystem::path resynthesized_netlist_path = base_path / "resynth_netlist.v";
            const std::filesystem::path yosys_helper_lib_path      = base_path / "yosys_helper_lib.v";

            std::filesystem::create_directory(base_path);

            log_info("netlist_preprocessing", "Writing Verilog file to {} ...", org_netlist_path.string());

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

            log_debug("netlist_preprocessing", "yosys command: {}", command);

            system(command.c_str());

            auto log_man = LogManager::get_instance();
            log_man->deactivate_channel("info");
            log_man->deactivate_channel("warning");
            auto resynth_nl = netlist_factory::load_netlist(resynthesized_netlist_path, hgl_lib);
            log_man->activate_channel("info");
            log_man->activate_channel("warning");

            if (resynth_nl == nullptr)
            {
                return ERR("Unable to resynthesize gate level netlist with yosys: failed to load resynthesized netlist at " + resynthesized_netlist_path.string());
            }

            // yosys workaround for stupid net renaming
            for (const auto& pin : resynth_nl->get_top_module()->get_input_pins())
            {
                auto net = pin->get_net();
                net->set_name(pin->get_name());
                log_debug("netlist_preprocessing", "renamed net {} with pin name {}", net->get_name(), pin->get_name());
            }

            // yosys workaround for stupid net renaming
            for (const auto& pin : resynth_nl->get_top_module()->get_output_pins())
            {
                auto net = pin->get_net();
                net->set_name(pin->get_name());
                log_debug("netlist_preprocessing", "renamed net {} with pin name {}", net->get_name(), pin->get_name());
            }

            // delete the two created files
            // std::filesystem::remove(org_netlist_path);
            // std::filesystem::remove(resynthesized_netlist_path);
            // std::filesystem::remove(yosys_helper_lib_path);

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

        const std::filesystem::path base_path   = std::filesystem::temp_directory_path() / "resynthesize_subgraph_with_yosys";
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

        // delete subgraph gates
        auto delete_res = delete_subgraph(nl, subgraph);
        if (delete_res.is_error())
        {
            return ERR_APPEND(delete_res.get_error(), "unable to replace subgraph with netlist: failed to delete subgraph");
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
}    // namespace hal