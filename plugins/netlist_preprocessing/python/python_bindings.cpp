#include "hal_core/python_bindings/python_bindings.h"

#include "netlist_preprocessing/plugin_netlist_preprocessing.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(netlist_preprocessing, m)
    {
        m.doc() = "hal NetlistPreprocessingPlugin python bindings";
#else
    PYBIND11_PLUGIN(netlist_preprocessing)
    {
        py::module m("netlist_preprocessing", "hal NetlistPreprocessingPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<NetlistPreprocessingPlugin, RawPtrWrapper<NetlistPreprocessingPlugin>, BasePluginInterface> py_netlist_preprocessing(m, "NetlistPreprocessingPlugin");
        py_netlist_preprocessing.def_property_readonly("name", &NetlistPreprocessingPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_netlist_preprocessing.def("get_name", &NetlistPreprocessingPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: Plugin name.
            :rtype: str
        )");

        py_netlist_preprocessing.def_property_readonly("version", &NetlistPreprocessingPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_netlist_preprocessing.def("get_version", &NetlistPreprocessingPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: Plugin version.
            :rtype: str
        )");

        py_netlist_preprocessing.def_static(
            "remove_unused_lut_inputs",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_unused_lut_inputs(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes all LUT fan-in endpoints that do not correspond to a variable within the Boolean function that determines the output of a gate.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed LUT endpoints on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "remove_buffers",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_buffers(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes buffer gates from the netlist and connect their fan-in to their fan-out nets.
                Considers all combinational gates and takes their inputs into account.
                For example, a 2-input AND gate with one input being connected to constant ``1`` will also be removed.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed buffers on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "remove_redundant_gates",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_redundant_gates(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes redundant gates from the netlist, i.e., gates that are functionally equivalent and are connected to the same input nets.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "remove_redundant_loops",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_redundant_loops(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes redundant sequential feedback loops.
                Sometimes flip-flops and some of their combinational fan-in form a feedback loop where the flip-flop input depends on its own output.
                For optimization, some synthesizers create multiple equivalent instances of these feedback loops.
                To simplify structural analysis, this function removes the redundant flip-flop gate of the loop from the netlist.
                Other preprocessing functions can then take care of the remaining combination gates of the loop.
                
                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "remove_redundant_logic_trees",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_redundant_logic_trees(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes redundant logic trees made up of combinational gates.
                If two trees compute the exact same function even if implemented with different gates we will disconnect one of the trees and afterwards clean up all dangling gates and nets. 
                
                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "remove_unconnected_gates",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_unconnected_gates(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes gates for which all fan-out nets do not have a destination and are not global output nets.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "remove_unconnected_nets",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_unconnected_nets(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes nets who have neither a source, nor a destination.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed nets on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "manual_mux_optimizations",
            [](Netlist* nl, GateLibrary* mux_inv_gl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::manual_mux_optimizations(nl, mux_inv_gl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("mux_inv_gl"),
            R"(
                Apply manually implemented optimizations to the netlist centered around muxes.
                Currently implemented optimizations include:
                 - removing inverters incase there are inverter gates infront and behind every data input and output of the mux
                 - optimizing and therefore unifying possible inverters preceding the select signals by resynthesizing

                :param halp_py.Netlist nl: The netlist to operate on.
                :param halp_py.GateLibrary mux_inv_gl: A gate library only containing mux and inverter gates used for resynthesis.
                :returns: The difference in total gates caused by these optimizations.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "propagate_constants",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::propagate_constants(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Builds for all gate output nets the Boolean function and substitutes all variables connected to vcc/gnd nets with the respective boolean value.
                If the function simplifies to a static boolean constant cut the connection to the nets destinations and directly connect it to vcc/gnd. 

                :partam hal_py.Netlist nl: The netlist to operate on.
                :returns: The number of rerouted nets on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "remove_consecutive_inverters",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_consecutive_inverters(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Removes two consecutive inverters and reconnects the input of the first inverter to the output of the second one.
                If the first inverter has additional successors, only the second inverter is deleted.

                :param hal_py.Netlist nl: The netlist to operate on.
                :returns: The number of removed inverter gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "simplify_lut_inits",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::simplify_lut_inits(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Replaces pins connected to GND/VCC with constants and simplifies the boolean function of a LUT by recomputing the INIT string.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of simplified INIT strings on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "decompose_gate",
            [](Netlist* nl, Gate* g, const bool delete_gate = true) -> bool {
                auto res = NetlistPreprocessingPlugin::decompose_gate(nl, g, delete_gate);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("nl"),
            py::arg("g"),
            py::arg("delete_gate") = true,
            R"(
                Builds the Boolean function of each output pin of the gate and constructs a gate tree implementing it.
                Afterwards the original output net is connected to the built gate tree and the gate is deleted if the ``delete_gate`` flag is set.

                For the decomposition we currently only support the base operands AND, OR, INVERT.
                The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param hal_py.Gate gate: The gate to decompose.
                :param bool delete_gate: Determines whether the original gate gets deleted by the function, defaults to ``True``. 
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        py_netlist_preprocessing.def_static(
            "decompose_gates_of_type",
            [](Netlist* nl, const std::vector<const GateType*>& gate_types) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::decompose_gates_of_type(nl, gate_types);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("gate_types"),
            R"(
                Decomposes each gate of the specified type by building the Boolean function for each output pin of the gate and constructing a gate tree implementing it.
                Afterwards the original gate is deleted and the output net is connected to the built gate tree.

                For the decomposition we currently only support the base operands AND, OR, INVERT.
                The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.GateType] gate_types: The gate types that should be decomposed.
                :returns: The number of decomposed gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "resynthesize_gate",
            [](Netlist* nl, Gate* g, GateLibrary* target_lib, const std::filesystem::path& genlib_path, const bool delete_gate) -> bool {
                auto res = NetlistPreprocessingPlugin::resynthesize_gate(nl, g, target_lib, genlib_path, delete_gate);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("nl"),
            py::arg("g"),
            py::arg("target_lib"),
            py::arg("genlib_path"),
            py::arg("delete_gate") = true,
            R"(
                Build the Boolean function of the gate and resynthesize a functional description of that function with a logic synthesizer.
                Afterwards the original gate is replaced by the technology mapped netlist produced by the synthesizer.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param hal_py.Gate g: The gate to resynthesize.
                :param hal_py.GateLibrary target_lib: Gatelibrary containing the gates used for technology mapping.
                :param path genlib_path: Path to file containg the target library in genlib format.
                :param bool delete_gate: Determines whether the original gate gets deleted by the function, defaults to true.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        py_netlist_preprocessing.def_static(
            "resynthesize_gates_of_type",
            [](Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl) -> -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::resynthesize_gates_of_type(nl, gate_types, target_gl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("gate_types"),
            py::arg("target_gl"),
            R"(
                Build the Boolean functions of all gates of the specified types and resynthesize a functional description of those functions with a logic synthesizer.
                Afterwards the original gates are replaced by the technology mapped netlists produced by the synthesizer.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.GateType] gate_types: The gate types specifying which gates should be resynthesized.
                :param hal_py.GateLibrary target_lib: Gatelibrary containing the gates used for technology mapping.
                :returns: The number of resynthesized gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "resynthesize_subgraph",
            [](Netlist* nl, const std::vector<Gate*>& subgraph, GateLibrary* target_gl) -> -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::resynthesize_subgraph(nl, subgraph, target_gl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("subgraph"),
            py::arg("target_gl"),
            R"(
                Build a verilog description of a subgraph of gates and synthesize a new technology mapped netlist of the whole subgraph with a logic synthesizer.
                Afterwards the original subgraph is replaced by the technology mapped netlist produced by the synthesizer.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.Gate] subgraph: The subgraph to be resynthesized.
                :param hal_py.GateLibrary target_lib: Gatelibrary containing the gates used for technology mapping.
                :returns: The number of resynthesized gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "resynthesize_subgraph_of_type",
            [](Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl) -> -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::resynthesize_subgraph_of_type(nl, gate_types, target_gl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("gate_types"),
            py::arg("target_gl"),
            R"(
                Build a verilog description of the subgraph consisting of all the gates of the specified types. 
                Then synthesize a new technology mapped netlist of the whole subgraph with a logic synthesizer.
                Afterwards the original subgraph is replaced by the technology mapped netlist produced by the synthesizer.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.GateType] gate_types: The gate types specifying which gates should be part of the subgraph.
                :param hal_py.GateLibrary target_lib: Gatelibrary containing the gates used for technology mapping.
                :returns: The number of resynthesized gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "reconstruct_indexed_ff_identifiers",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::reconstruct_indexed_ff_identifiers(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
                Tries to reconstruct a name and index for each flip flop that was part of a multibit wire in the verilog code.
                This is NOT a general netlist reverse engineering algorithm and ONLY works on synthesized netlists with names annotated by the synthesizer.
                This function mainly focuses netlists synthesized with yosys since yosys names the output wires of the flip flops but not the gate it self.
                We try to reconstruct name and index for each flip flop based on the name of its output nets.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of reconstructed names on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        py_netlist_preprocessing.def_static(
            "parse_def_file",
            [](Netlist* nl, const std::filesystem::path& def_file) -> bool {
                auto res = NetlistPreprocessingPlugin::parse_def_file(nl, def_file);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("nl"),
            py::arg("def_file"),
            R"(
                Parses a design exchange format file and extracts the coordinates of a placed design for each component/gate.
                The extracted coordinates get annotated to the gates.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param pathlib.Path def_file: The path to the def file
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
