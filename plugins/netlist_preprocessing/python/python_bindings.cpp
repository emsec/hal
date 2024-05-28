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
                :returns: The number of removed LUT endpoints on success, `None` otherwise.
                :rtype: int or None
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
                For example, a 2-input AND gate with one input being connected to constant '1' will also be removed.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed buffers on success, `None` otherwise.
                :rtype: int or None
            )");

        py_netlist_preprocessing.def_static(
            "remove_redundant_logic",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::remove_redundant_logic(nl);
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
                :returns: The number of removed gates on success, `None` otherwise.
                :rtype: int or None
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
                Removes gates which outputs are all unconnected and not a global output net.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed gates on success, `None` otherwise.
                :rtype: int or None
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
                Remove nets which have no source and not destination.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed nets on success, `None` otherwise.
                :rtype: int or None
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
                :returns: The number of simplified INIT strings on success, `None` otherwise.
                :rtype: int or None
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
                Afterwards the original output net is connected to the built gate tree and the gate is deleted if the 'delete_gate' flag is set.

                For the decomposition we currently only support the base operands AND, OR, INVERT.
                The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param hal_py.Gate gate: The gate to decompose.
                :param bool delete_gate: Determines whether the original gate gets deleted by the function, defaults to `True`. 
                :returns: `True` on success, `False` otherwise.
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
                Decomposes each gate of the specified type by building the Boolean function for each output pin of the gate and contructing a gate tree implementing it.
                Afterwards the original gate is deleted and the output net is connected to the built gate tree.

                For the decomposition we currently only support the base operands AND, OR, INVERT.
                The function searches in the gate library for a fitting two input gate and uses a standard HAL gate type if none is found.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.GateType] gate_types: The gate types that should be decomposed.
                :returns: The number of decomposed gates on success, `None` otherwise.
                :rtype: int or None
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
                :returns: The number of reconstructed names on success, `None` otherwise.
                :rtype: int or None
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
                Parses a design exchange format file and extracts the coordinated of a placed design for each component/gate.
                The extracted coordinates get annotated to the gates.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param pathlib.Path def_file: The path to the def file
                :returns: `True` on success, `False` otherwise.
                :rtype: bool
            )");

        py_netlist_preprocessing.def_static(
            "unify_ff_outputs",
            [](Netlist* nl, const std::vector<Gate*>& ffs = {}, GateType* inverter_type = nullptr) -> std::optional<u32> {
                auto res = NetlistPreprocessingPlugin::unify_ff_outputs(nl, ffs, inverter_type);
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
            py::arg("ffs")           = std::vector<Gate*>(),
            py::arg("inverter_type") = nullptr,
            R"(
                Iterates all flip-flops of the netlist or specified by the user.
                If a flip-flop has a ``state`` and a ``neg_state`` output, a new inverter gate is created and connected to the ``state`` output net as an additional destination.
                Finally, the ``neg_state`` output net is disconnected from the ``neg_state`` pin and re-connected to the new inverter gate's output. 

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param list[hal_py.Gate] ffs: The flip-flops to operate on. Defaults to an empty vector, in which case all flip-flops of the netlist are considered.
                :param hal_py.GateType inverter_type:  The inverter gate type to use. Defaults to a ``None``, in which case the first inverter type found in the gate library is used.
                :returns: The number of rerouted ``neg_state`` outputs on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
