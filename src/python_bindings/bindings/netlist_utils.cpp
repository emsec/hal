#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_utils_init(py::module& m)
    {
        auto py_netlist_utils = m.def_submodule("NetlistUtils", R"(
            HAL Netlist Utility functions.
        )");

        py_netlist_utils.def(
            "get_subgraph_function",
            [](const Net* net, const std::vector<const Gate*>& subgraph_gates) -> BooleanFunction {
                auto res = netlist_utils::get_subgraph_function(net, subgraph_gates);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting subgraph function:\n{}", res.get_error().get());
                    return BooleanFunction();
                }
            },
            py::arg("net"),
            py::arg("subgraph_gates"),
            R"(
            Get the combined Boolean function of a subgraph of combinational gates starting at the source of the given net.
            The variables of the resulting Boolean function are made up of the IDs of the nets that influence the output ('net_[ID]').

            :param hal_py.Net net: The output net for which to generate the Boolean function.
            :param list[hal_py.Gate] subgraph_gates: The gates making up the subgraph.
            :returns:  The combined Boolean function of the subgraph on success, an empty Boolean function otherwise.
            :rtype: hal_py.BooleanFunction
        )");

        py_netlist_utils.def(
            "copy_netlist", [](const Netlist* nl) { return std::shared_ptr<Netlist>(netlist_utils::copy_netlist(nl)); }, py::arg("nl"), R"(
            Get a deep copy of an entire netlist including all of its gates, nets, modules, and groupings.

            :param hal_py.Netlist nl: The netlist to copy.
            :returns: The deep copy of the netlist.
            :rtype: hal_py.Netlist
        )");

        py_netlist_utils.def("get_ff_dependency_matrix", &netlist_utils::get_ff_dependency_matrix, py::arg("nl"), R"(
            Get the FF dependency matrix of a netlist.

            :param hal_py.Netlist nl: The netlist to extract the dependency matrix from.
            :returns: A pair consisting of std::map<u32, Gate*>, which includes the mapping from the original gate
            :rtype: pair(dict(int, hal_py.Gate), list[list[int]])
        )");

        py_netlist_utils.def("get_next_gates",
                             py::overload_cast<const Gate*, bool, int, const std::function<bool(const Gate*)>&>(&netlist_utils::get_next_gates),
                             py::arg("gate"),
                             py::arg("get_successors"),
                             py::arg("depth")  = 0,
                             py::arg("filter") = nullptr,
                             R"(
            Find predecessors or successors of a gate. If depth is set to 1 only direct predecessors/successors will be returned. 
            Higher number of depth causes as many steps of recursive calls. 
            If depth is set to 0 there is no limitation and the loop  continues until no more predecessors/succesors are found.
            If a filter function is given, the recursion stops whenever the filter function evaluates to False. 
            Only gates matching the filter will be added to the result vector.
            The result will not include the provided gate itself.

            :param hal_py.Gate gate: The initial gate.
            :param bool get_successors: True to return successors, False for Predecessors.
            :param int depth: Depth of recursion.
            :param lambda filter: User-defined filter function.
            :returns: List of predecessor/successor gates.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_next_gates",
                             py::overload_cast<const Net*, bool, int, const std::function<bool(const Gate*)>&>(&netlist_utils::get_next_gates),
                             py::arg("net"),
                             py::arg("get_successors"),
                             py::arg("depth")  = 0,
                             py::arg("filter") = nullptr,
                             R"(
            Find predecessors or successors of a net. If depth is set to 1 only direct predecessors/successors will be returned. 
            Higher number of depth causes as many steps of recursive calls. 
            If depth is set to 0 there is no limitation and the loop  continues until no more predecessors/succesors are found.
            If a filter function is given, the recursion stops whenever the filter function evaluates to False. 
            Only gates matching the filter will be added to the result vector.

            :param hal_py.Net net: The initial net.
            :param bool get_successors: True to return successors, False for Predecessors.
            :param int depth: Depth of recursion.
            :param lambda filter: User-defined filter function.
            :returns: List of predecessor/successor gates.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_next_sequential_gates",
                             py::overload_cast<const Gate*, bool, std::unordered_map<u32, std::vector<Gate*>>&>(&netlist_utils::get_next_sequential_gates),
                             py::arg("gate"),
                             py::arg("get_successors"),
                             py::arg("cache"),
                             R"(
            Find all sequential predecessors or successors of a gate.
            Traverses combinational logic of all input or output nets until sequential gates are found.
            The result may include the provided gate itself.
            The use of the this cached version is recommended in case of extensive usage to improve performance. 
            The cache will be filled by this function and should initially be provided empty.
            Different caches for different values of get_successors shall be used.
        
            :param hal_py.Gate gate: The initial gate.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :param dict[int, list[hal_py.Gate]] cache: The cache. 
            :returns: All sequential successors or predecessors of the gate.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_next_sequential_gates", py::overload_cast<const Gate*, bool>(&netlist_utils::get_next_sequential_gates), py::arg("gate"), py::arg("get_successors"), R"(
            Find all sequential predecessors or successors of a gate.
            Traverses combinational logic of all input or output nets until sequential gates are found.
            The result may include the provided gate itself.

            :param hal_py.Gate gate: The initial gate.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :returns: All sequential successors or predecessors of the gate.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_next_sequential_gates",
                             py::overload_cast<const Net*, bool, std::unordered_map<u32, std::vector<Gate*>>&>(&netlist_utils::get_next_sequential_gates),
                             py::arg("net"),
                             py::arg("get_successors"),
                             py::arg("cache"),
                             R"(
            Find all sequential predecessors or successors of a net.
            Traverses combinational logic of all input or output nets until sequential gates are found.
            The use of the cache is recommended in case of extensive usage of this function. 
            The cache will be filled by this function and should initially be provided empty.
            Different caches for different values of get_successors shall be used.

            :param hal_py.Net net: The initial net.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :param dict[int, list[hal_py.Gate]] cache: The cache. 
            :returns: All sequential successors or predecessors of the net.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def("get_next_sequential_gates", py::overload_cast<const Net*, bool>(&netlist_utils::get_next_sequential_gates), py::arg("net"), py::arg("get_successors"), R"(
            Find all sequential predecessors or successors of a net.
            Traverses combinational logic of all input or output nets until sequential gates are found.

            :param hal_py.Net net: The initial net.
            :param bool get_successors: If true, sequential successors are returned, otherwise sequential predecessors are returned.
            :returns: All sequential successors or predecessors of the net.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def("get_path",
                             py::overload_cast<const Gate*, bool, std::set<GateTypeProperty>, std::unordered_map<u32, std::vector<Gate*>>&>(&netlist_utils::get_path),
                             py::arg("gate"),
                             py::arg("get_successors"),
                             py::arg("stop_properties"),
                             py::arg("cache"),
                             R"(
            Find all gates on the predecessor or successor path of a gate.
            Traverses all input or output nets until gates of the specified base types are found.
            The result may include the provided gate itself.
            The use of the this cached version is recommended in case of extensive usage to improve performance. 
            The cache will be filled by this function and should initially be provided empty.
            Different caches for different values of get_successors shall be used.
        
            :param hal_py.Gate gate: The initial gate.
            :param bool get_successors: If true, the successor path is returned, otherwise the predecessor path is returned.
            :param set[hal_py.GateTypeProperty] stop_properties: Stop recursion when reaching a gate of a type with one of the specified properties.
            :param dict[int, list[hal_py.Gate]] cache: The cache. 
            :returns: All gates on the predecessor or successor path of the gate.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def(
            "get_path", py::overload_cast<const Gate*, bool, std::set<GateTypeProperty>>(&netlist_utils::get_path), py::arg("gate"), py::arg("get_successors"), py::arg("stop_properties"), R"(
            Find all gates on the predeccessor or successor path of a gate.
            Traverses all input or output nets until gates of the specified base types are found.
            The result may include the provided gate itself.

            :param hal_py.Gate gate: The initial gate.
            :param bool get_successors: If true, the successor path is returned, otherwise the predecessor path is returned.
            :param set[hal_py.GateTypeProperty] stop_properties: Stop recursion when reaching a gate of a type with one of the specified properties.
            :returns: All gates on the predecessor or successor path of the gate.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_path",
                             py::overload_cast<const Net*, bool, std::set<GateTypeProperty>, std::unordered_map<u32, std::vector<Gate*>>&>(&netlist_utils::get_path),
                             py::arg("net"),
                             py::arg("get_successors"),
                             py::arg("stop_properties"),
                             py::arg("cache"),
                             R"(
            Find all gates on the predecessor or successor path of a net.
            Traverses all input or output nets until gates of the specified base types are found.
            The use of the this cached version is recommended in case of extensive usage to improve performance. 
            The cache will be filled by this function and should initially be provided empty.
            Different caches for different values of get_successors shall be used.

            :param hal_py.Net net: The initial net.
            :param bool get_successors: If true, the successor path is returned, otherwise the predecessor path is returned.
            :param set[hal_py.GateTypeProperty] stop_properties: Stop recursion when reaching a gate of a type with one of the specified properties.
            :param dict[int, list[hal_py.Gate]] cache: The cache. 
            :returns: All gates on the predecessor or successor path of the net.
            :rtype: list[hal_py.Net]
        )");
        py_netlist_utils.def(
            "get_path", py::overload_cast<const Net*, bool, std::set<GateTypeProperty>>(&netlist_utils::get_path), py::arg("net"), py::arg("get_successors"), py::arg("stop_properties"), R"(
            Find all gates on the predecessor or successor path of a net.
            Traverses all input or output nets until gates of the specified base types are found.

            :param hal_py.Net net: The initial net.
            :param bool get_successors: If true, the successor path is returned, otherwise the predecessor path is returned.
            :param set[hal_py.GateTypeProperty] stop_properties: Stop recursion when reaching a gate of a type with one of the specified properties.
            :returns: All gates on the predecessor or successor path of the net.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def("get_nets_at_pins", netlist_utils::get_nets_at_pins, py::arg("gate"), py::arg("pins"), R"(
            Get the nets that are connected to a subset of pins of the specified gate.
        
            :param hal_py.Gate gate: The gate.
            :param list[hal_py.GatePin] pins: The targeted pins.
            :returns: The list of nets connected to the pins.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def(
            "remove_buffers",
            [](Netlist* netlist, bool analyze_inputs = false) -> i32 {
                auto res = netlist_utils::remove_buffers(netlist, analyze_inputs);
                if (res.is_ok())
                {
                    return (i32)res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while removing buffer gates from netlist:\n{}", res.get_error().get());
                    return -1;
                }
            },
            py::arg("netlist"),
            py::arg("analyze_inputs") = false,
            R"(
            Remove all buffer gates from the netlist and connect their fan-in to their fan-out nets.
            If enabled, analyzes every gate's inputs and removes fixed '0' or '1' inputs from the Boolean function.

            :param hal_py.Netlist netlist: The target netlist.
            :param bool analyze_inputs: Set True to dynamically analyze the inputs, False otherwise.
            :returns: The number of removed buffers on success, -1 otherwise.
            :rtype: int
        )");

        py_netlist_utils.def(
            "remove_unused_lut_endpoints",
            [](Netlist* netlist) -> i32 {
                auto res = netlist_utils::remove_unused_lut_endpoints(netlist);
                if (res.is_ok())
                {
                    return (i32)res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while removing unused LUT endpoints from netlist:\n{}", res.get_error().get());
                    return -1;
                }
            },
            py::arg("netlist"),
            R"(
            Remove all LUT fan-in endpoints that are not present within the Boolean function of the output of a gate.
        
            :param hal_py.Netlist netlist: The target netlist.
            :returns: The number of removed endpionts on success, -1 otherwise.
            :rtype: int
        )");

        py_netlist_utils.def("get_common_inputs", &netlist_utils::get_common_inputs, py::arg("gates"), py::arg("threshold") = 0, R"(
            Returns all nets that are considered to be common inputs to the provided gates.
            A threshold value can be provided to specify the number of gates a net must be connected to in order to be classified as a common input.
            If the theshold value is set to 0, a net must be input to all gates to be considered a common input.
        
            :param list[hal_py.Gate] gates: The gates.
            :param int threshold: The threshold value, defaults to 0.
            :returns: The common input nets.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def(
            "replace_gate",
            [](Gate* gate, GateType* target_type, std::map<GatePin*, GatePin*> pin_map) -> i32 {
                auto res = netlist_utils::replace_gate(gate, target_type, pin_map);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "error encountered while replacing gate:\n{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("gate"),
            py::arg("target_type"),
            py::arg("pin_map"),
            R"(
            Replace the given gate with a gate of the specified gate type.
            A dict from old to new pins must be provided in order to correctly connect the gates inputs and outputs.
            A pin can be omitted if no connection at that pin is desired.

            :param hal_py.Gate gate: The gate to be replaced.
            :param hal_py.GateType target_type: The gate type of the replacement gate.
            :param dict[hal_py.GatePin,hal_py.GatePin] pin_map: A dict from old to new pins.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_netlist_utils.def(
            "get_gate_chain",
            [](Gate* start_gate, const std::vector<const GatePin*>& input_pins = {}, const std::vector<const GatePin*>& output_pins = {}, const std::function<bool(const Gate*)>& filter = nullptr)
                -> std::vector<Gate*> {
                auto res = netlist_utils::get_gate_chain(start_gate, input_pins, output_pins, filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while detecting gate chain:\n{}", res.get_error().get());
                    return {};
                }
            },
            py::arg("start_gate"),
            py::arg("input_pins")  = std::vector<GatePin*>(),
            py::arg("output_pins") = std::vector<GatePin*>(),
            py::arg("filter")      = nullptr,
            R"(
            Find a sequence of identical gates that are connected via the specified input and output pins.
            The start gate may be any gate within a such a sequence, it is not required to be the first or the last gate.
            If input and/or output pins are specified, the gates must be connected through one of the input pins and/or one of the output pins.
            The optional filter is evaluated on every gate such that the result only contains gates matching the specified condition.

            :param hal_py.Gate start_gate: The gate at which to start the chain detection.
            :param list[hal_py.GatePin] input_pins: The input pins through which the gates must be connected. Defaults to an empty list.
            :param set[hal_py.GatePin] output_pins: The output pins through which the gates must be connected. Defaults to an empty list.
            :param lambda filter: An optional filter function to be evaluated on each gate.
            :returns: A list of gates that form a chain on success, an empty list on error.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def(
            "get_complex_gate_chain",
            [](Gate* start_gate,
               const std::vector<GateType*>& chain_types,
               const std::map<GateType*, std::vector<const GatePin*>>& input_pins,
               const std::map<GateType*, std::vector<const GatePin*>>& output_pins,
               const std::function<bool(const Gate*)>& filter = nullptr) -> std::vector<Gate*> {
                auto res = netlist_utils::get_complex_gate_chain(start_gate, chain_types, input_pins, output_pins, filter);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while detecting complex gate chain:\n{}", res.get_error().get());
                    return {};
                }
            },
            py::arg("start_gate"),
            py::arg("chain_types"),
            py::arg("input_pins"),
            py::arg("output_pins"),
            py::arg("filter") = nullptr,
            R"(
            Find a sequence of gates (of the specified sequence of gate types) that are connected via the specified input and output pins.
            The start gate may be any gate within a such a sequence, it is not required to be the first or the last gate.
            However, the start gate must be of the first gate type within the repeating sequence.
            If input and/or output pins are specified for a gate type, the gates must be connected through one of the input pins and/or one of the output pins.
            The optional filter is evaluated on every gate such that the result only contains gates matching the specified condition.

            :param hal_py.Gate start_gate: The gate at which to start the chain detection.
            :param list[hal_py.GateType] chain_types: The sequence of gate types that is expected to make up the gate chain.
            :param dict[hal_py.GateType,set[str]] input_pins: The input pins through which the gates are allowed to be connected.
            :param dict[hal_py.GateType,set[str]] output_pins: The output pins through which the gates are allowed to be connected.
            :param lambda filter: A filter that is evaluated on all candidates.
            :returns: A list of gates that form a chain.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_shortest_path", py::overload_cast<Gate*,Gate*,bool>(&netlist_utils::get_shortest_path), py::arg("start_gate"), py::arg("end_gate"), py::arg("search_both_directions") = false, R"(
            Find the shortest path (i.e., theresult set with the lowest number of gates) that connects the start gate with the end gate. 
            The gate where the search started from will be the first in the result vector, the end gate will be the last. 
            If there is no such path an empty vector is returned. If there is more than one path with the same length only the first one is returned.

            :param hal_py.Gate start_gate: The start gate for the path (might be the end if searching both directions)
            :param hal_py.Gate end_gate: The end gate for the path (might be the start if searching both directions)
            :bool search_both_directions: If true checking start <-> end, which ever direction is shorter
            :returns: A list of gates that form a chain on success, an empty list on error.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_shortest_path", py::overload_cast<Gate*,Module*>(&netlist_utils::get_shortest_path), py::arg("start_gate"), py::arg("end_module"), R"(
            Find the shortest path (i.e., theresult set with the lowest number of gates) that connects the start gate with any gate for the given module.
            The gate where the search started from will be the first in the result vector, the end gate will be the last.
            If there is no such path an empty vector is returned. If there is more than one path with the same length only the first one is returned.

            :param hal_py.Gate start_gate: The start gate for the path
            :param hal_py.Module end_module: The module which contains the end gate for the path
            :returns: A list of gates that form a chain on success, an empty list on error.
            :rtype: list[hal_py.Gate]
        )");
    }
}    // namespace hal
