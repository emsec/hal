#include "hal_core/python_bindings/python_bindings.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "resynthesis/plugin_resynthesis.h"
#include "resynthesis/resynthesis.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(resynthesis, m)
    {
        m.doc() = "Provides functions to decompose or re-synthesize combinational parts of a gate-level netlist.";
#else
    PYBIND11_PLUGIN(resynthesis)
    {
        py::module m("resynthesis", "Provides functions to decompose or re-synthesize combinational parts of a gate-level netlist.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<ResynthesisPlugin, RawPtrWrapper<ResynthesisPlugin>, BasePluginInterface> py_resynthesis_plugin(
            m, "ResynthesisPlugin", R"(This class provides an interface to integrate the netlist resynthesis as a plugin within the HAL framework.)");

        py_resynthesis_plugin.def_property_readonly("name", &ResynthesisPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_resynthesis_plugin.def("get_name", &ResynthesisPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_resynthesis_plugin.def_property_readonly("version", &ResynthesisPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_resynthesis_plugin.def("get_version", &ResynthesisPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_resynthesis_plugin.def_property_readonly("description", &ResynthesisPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_resynthesis_plugin.def("get_description", &ResynthesisPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_resynthesis_plugin.def_property_readonly("dependencies", &ResynthesisPlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )");

        py_resynthesis_plugin.def("get_dependencies", &ResynthesisPlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )");

        m.def(
            "decompose_gate",
            [](Netlist* nl, Gate* gate, const bool delete_gate = true) -> bool {
                auto res = resynthesis::decompose_gate(nl, gate, delete_gate);
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
            py::arg("gate"),
            py::arg("delete_gate") = true,
            R"(
                Decompose a combinational gate into a small circuit of AND, OR, XOR, and INVERT gates.
                For each output pin, the resolved Boolean function (only dependent on input pins) is determined.
                All these Boolean functions are then converted into a netlist using the previously mentioned primitive gates.
                The target gate is then replaced in the original netlist with the circuit that was just generated.
                The target gate is only deleted if ``delete_gate`` is set to ``True``.
                Gate replacement will fail if the gate library of the netlist does not contain suitable AND, OR, XOR, and INVERT gate types.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param hal_py.Gate gate: The gate to decompose.
                :param bool delete_gate: Set ``True`` to delete the original gate, ``False`` to keep it in the netlist. Defaults to ``True``.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        m.def(
            "decompose_gates_of_type",
            [](Netlist* nl, const std::vector<const GateType*>& gate_types) -> std::optional<u32> {
                auto res = resynthesis::decompose_gates_of_type(nl, gate_types);
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
                Decompose all combinational gates of the specified types into small circuits of AND, OR, XOR, and INVERT gates.
                For all output pins of each gate, the resolved Boolean function (only dependent on input pins) is determined.
                All these Boolean functions are then converted into a circuit using the previously mentioned primitive gates.
                The target gates are then replaced (and thereby deleted) in the original netlist with the circuit that was just generated.
                Gate replacement will fail if the gate library of the netlist does not contain suitable AND, OR, XOR, and INVERT gate types.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.GateType] gate_types: The gate types to be decomposed.
                :returns: The number of decomposed gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "resynthesize_gate",
            [](Netlist* nl, Gate* gate, GateLibrary* target_gl, const bool delete_gate) -> bool {
                auto res = resynthesis::resynthesize_gate(nl, gate, target_gl, delete_gate);
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
            py::arg("gate"),
            py::arg("target_gl"),
            py::arg("delete_gate") = true,
            R"(
                Re-synthesize a combinational gate by calling Yosys on a functional description of the gate using a reduced gate library.
                For all output pins of each gate, the resolved Boolean function (only dependent on input pins) is determined.
                All these Boolean functions are then written to an HDL file that is functionally equivalent to the target gate.
                This file is fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
                The provided gate library should be a subset of the gate library that was used to parse the netlist.
                The target gate is then replaced in the original netlist with the circuit that was just generated.
                The target gate is only deleted if ``delete_gate`` is set to ``True``.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param hal_py.Gate gate: The gate to re-synthesize.
                :param hal_py.GateLibrary target_gl: The gate library that is a subset of the gate library used to parse the netlist.
                :param bool delete_gate: Set ``True`` to delete the original gate, ``False`` to keep it in the netlist. Defaults to ``True``.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        m.def(
            "resynthesize_gates",
            [](Netlist* nl, const std::vector<Gate*>& gates, GateLibrary* target_gl) -> std::optional<u32> {
                auto res = resynthesis::resynthesize_gates(nl, gates, target_gl);
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
            py::arg("gates"),
            py::arg("target_gl"),
            R"(
                Re-synthesize all specified combinational gates by calling Yosys on a functional description of the gates using a reduced gate library.
                For all output pins of each gate, the resolved Boolean function (only dependent on input pins) is determined.
                All Boolean functions of a gate are then written to an HDL file that is functionally equivalent to the gate.
                These files are fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
                The provided gate library should be a subset of the gate library that was used to parse the netlist.
                The gates are then replaced in the original netlist with the circuits that were just generated.
                This process is repeated for every gate, hence they are re-synthesized in isolation.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param hal_py.Gate g: The gates to re-synthesize.
                :param hal_py.GateLibrary target_gl: The gate library that is a subset of the gate library used to parse the netlist.
                :returns: The number of re-synthesized gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "resynthesize_gates_of_type",
            [](Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl) -> std::optional<u32> {
                auto res = resynthesis::resynthesize_gates_of_type(nl, gate_types, target_gl);
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
                Re-synthesize all combinational gates of the specified types by calling Yosys on a functional description of the gates using a reduced gate library.
                For all output pins of each gate, the resolved Boolean function (only dependent on input pins) is determined.
                All Boolean functions of a gate are then written to an HDL file that is functionally equivalent to the gate.
                These files are fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
                The provided gate library should be a subset of the gate library that was used to parse the netlist.
                The gates are then replaced in the original netlist with the circuits that were just generated.
                This process is repeated for every gate, hence they are re-synthesized in isolation.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.GateType] gate_types: The gate types to be re-synthesized.
                :param hal_py.GateLibrary target_gl: The gate library that is a subset of the gate library used to parse the netlist.
                :returns: The number of re-synthesized gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "resynthesize_subgraph",
            [](Netlist* nl, const std::vector<Gate*>& subgraph, GateLibrary* target_gl) -> std::optional<u32> {
                auto res = resynthesis::resynthesize_subgraph(nl, subgraph, target_gl);
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
                Re-synthesize the combinational gates of the subgraph by calling Yosys on a Verilog netlist representation of the subgraph using a reduced gate library.
                All gates of the subgraph are written to a Verilog netlist file which is then fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
                The provided gate library should be a subset of the gate library that was used to parse the netlist.
                The gates are then replaced in the original netlist with the circuit that was just generated.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.Gate] subgraph: The subgraph to re-synthesize.
                :param hal_py.GateLibrary target_lib: The gate library that is a subset of the gate library used to parse the netlist.
                :returns: The number of re-synthesized gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "resynthesize_subgraph_of_type",
            [](Netlist* nl, const std::vector<const GateType*>& gate_types, GateLibrary* target_gl) -> std::optional<u32> {
                auto res = resynthesis::resynthesize_subgraph_of_type(nl, gate_types, target_gl);
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
                Re-synthesize the combinational gates of the specified types as a subgraph by calling Yosys on a Verilog netlist representation of the subgraph induced by these gates using a reduced gate library.
                All gates of the subgraph are written to a Verilog netlist file which is then fed to Yosys and subsequently synthesized to a netlist again by using the provided gate library.
                The provided gate library should be a subset of the gate library that was used to parse the netlist.
                The gates are then replaced in the original netlist with the circuit that was just generated.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.GateType] gate_types: The gate types to be re-synthesized.
                :param hal_py.GateLibrary target_lib: The gate library that is a subset of the gate library used to parse the netlist.
                :returns: The number of re-synthesized gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
