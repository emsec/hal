#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_utils_init(py::module& m)
    {
        auto py_netlist_utils = m.def_submodule("NetlistUtils", R"(
            HAL Netlist Utility functions.
        )");

        py_netlist_utils.def(
            "get_subgraph_function", py::overload_cast<const Net*, const std::vector<const Gate*>&>(&netlist_utils::get_subgraph_function), py::arg("net"), py::arg("subgraph_gates"), R"(
            Get the combined Boolean function of a specific net, considering an entire subgraph.<br>
            In other words, the Boolean functions of the subgraph gates that influence the target net are combined to one function.<br>
            The variables of the resulting Boolean function are the net IDs of the nets that influence the output.
            If this function is used extensively, consider using the above variant with a cache.

            :param hal_py.Net net: The output net for which to generate the Boolean function.
            :param list[hal_py.Gate] subgraph_gates: The gates making up the subgraph.
            :returns: The combined Boolean function of the subgraph.
            :rtype: hal_py.BooleanFunction
        )");

        py_netlist_utils.def(
            "copy_netlist", [](const Netlist* nl) { return std::shared_ptr<Netlist>(netlist_utils::copy_netlist(nl)); }, py::arg("nl"), R"(
            Get a deep copy of an entire netlist including all of its gates, nets, modules, and groupings.

            :param hal_py.Netlist nl: The netlist to copy.
            :returns: The deep copy of the netlist.
            :rtype: hal_py.Netlist
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

        py_netlist_utils.def("get_nets_at_pins", netlist_utils::get_nets_at_pins, py::arg("gate"), py::arg("pins"), py::arg("is_inputs"), R"(
            Get the nets that are connected to a subset of pins of the specified gate.
        
            :param hal_py.Gate gate: The gate.
            :param set[str] pins: The targeted pins.
            :param bool is_input: True to look for fan-in nets, false for fan-out.
            :returns: The set of nets connected to the pins.
            :rtype: set[hal_py.Net]
        )");

        py_netlist_utils.def("remove_buffers", netlist_utils::remove_buffers, py::arg("netlist"), py::arg("analyze_inputs") = false, R"(
            Remove all buffer gates from the netlist and connect their fan-in to their fan-out nets.
            If enabled, analyzes every gate's inputs and removes fixed '0' or '1' inputs from the Boolean function.

            :param hal_py.Netlist netlist: The target netlist.
            :param bool analyze_inputs: Set True to dynamically analyze the inputs, False otherwise.
        )");

        py_netlist_utils.def("remove_unused_lut_endpoints", netlist_utils::remove_unused_lut_endpoints, py::arg("netlist"), R"(
            Remove all LUT fan-in endpoints that are not present within the Boolean function of the output of a gate.
        
            :param hal_py.Netlist netlist: The target netlist.
        )");

        py_netlist_utils.def("rename_luts_according_to_function", netlist_utils::rename_luts_according_to_function, py::arg("netlist"), R"(
            Rename LUTs that implement simple functions to better reflect their functionality.
        
            :param hal_py.Netlist netlist: The target netlist.
        )");

        py_netlist_utils.def("get_common_inputs", netlist_utils::get_common_inputs, py::arg("gates"), py::arg("threshold") = 0, R"(
            Returns all nets that are considered to be common inputs to the provided gates.
            A threshold value can be provided to specify the number of gates a net must be connected to in order to be classified as a common input.
            If the theshold value is set to 0, a net must be input to all gates to be considered a common input.
        
            :param list[hal_py.Gate] gates: The gates.
            :param int threshold: The threshold value, defaults to 0.
            :returns: The common input nets.
            :rtype: list[hal_py.Net]
        )");

        py_netlist_utils.def("replace_gate", netlist_utils::replace_gate, py::arg("gate"), py::arg("target_type"), py::arg("pin_map"), R"(
            Replace the given gate with a gate of the specified gate type.
            A dict from old to new pins must be provided in order to correctly connect the gates inputs and outputs.
            A pin can be omitted if no connection at that pin is desired.

            :param hal_py.Gate gate: The gate to be replaced.
            :param hal_py.GateType target_type: The gate type of the replacement gate.
            :param dict[str,str] pin_map: A dict from old to new pin names.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_netlist_utils.def("get_gate_chain",
                             netlist_utils::get_gate_chain,
                             py::arg("start_gate"),
                             py::arg("input_pins")  = std::set<std::string>(),
                             py::arg("output_pins") = std::set<std::string>(),
                             py::arg("filter")      = nullptr,
                             R"(
            Find a repeating sequence of identical gates that connect through the specified pins.
            The start gate may be any gate within a chain of such sequences, it is not required to be the first or the last gate.
            A pair of input and output pins can be specified through which the gates are interconnected.
            If an empty set is given for input or output pins, every pin of the respective gate will be considered.
            Before adding a gate to the chain, an optional user-defined filter is evaluated on every candidate gate.

            :param hal_py.Gate start_gate: The gate at which to start the chain detection.
            :param set[str] input_pins: The input pins through which the gates are allowed to be connected.
            :param set[str] output_pins: The output pins through which the gates are allowed to be connected.
            :param lambda filter: A filter that is evaluated on all candidates.
            :returns: A list of gates that form a chain.
            :rtype: list[hal_py.Gate]
        )");

        py_netlist_utils.def("get_complex_gate_chain",
                             netlist_utils::get_complex_gate_chain,
                             py::arg("start_gate"),
                             py::arg("chain_types"),
                             py::arg("input_pins")  = std::map<GateType*, std::set<std::string>>(),
                             py::arg("output_pins") = std::map<GateType*, std::set<std::string>>(),
                             py::arg("filter")      = nullptr,
                             R"(
            Find a repeating sequence of gates that are of the specified gate types and connect through the specified pins.
            The start gate may be any gate within a chain of such sequences, it is not required to be the first or the last gate.
            However, the start gate must be of the first gate type of the repeating sequence.
            For every gate type, a pair of input and output pins can be specified through which the gates are interconnected.
            If a None is given for a gate type, any gate fulfilling the other properties will be considered.
            If an empty set is given for input or output pins, every pin of the respective gate will be considered.
            Before adding a gate to the chain, an optional user-defined filter is evaluated on every candidate gate.

            :param hal_py.Gate start_gate: The gate at which to start the chain detection.
            :param list[hal_py.GateType] chain_types: The sequence of gate types that is expected to make up the gate chain.
            :param dict[hal_py.GateType,set[str]] input_pins: The input pins through which the gates are allowed to be connected.
            :param dict[hal_py.GateType,set[str]] output_pins: The output pins through which the gates are allowed to be connected.
            :param lambda filter: A filter that is evaluated on all candidates.
            :returns: A list of gates that form a chain.
            :rtype: list[hal_py.Gate]
        )");
    }
}    // namespace hal
