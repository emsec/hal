#include "hal_core/python_bindings/python_bindings.h"

#include "netlist_preprocessing/netlist_preprocessing.h"
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

        m.def(
            "remove_unused_lut_inputs",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_unused_lut_inputs(nl);
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

        m.def(
            "remove_buffers",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_buffers(nl);
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

        m.def(
            "remove_redundant_gates",
            [](Netlist* nl, const std::function<bool(const Gate*)>& filter = nullptr) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_redundant_gates(nl, filter);
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
            py::arg("filter") = nullptr,
            R"(
                Removes redundant gates from the netlist, i.e., gates that are functionally equivalent and are connected to the same input nets.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param lambda filter: Optional filter to fine-tune which gates are being replaced. Default to a ``None``.
                :returns: The number of removed gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "remove_redundant_loops",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_redundant_loops(nl);
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

        m.def(
            "remove_redundant_logic_trees",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_redundant_logic_trees(nl);
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

        m.def(
            "remove_unconnected_gates",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_unconnected_gates(nl);
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

        m.def(
            "remove_unconnected_nets",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_unconnected_nets(nl);
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

        m.def(
            "remove_unconnected_looped",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_unconnected_looped(nl);
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
                Calls remove_unconnected_gates / remove_unconnected_nets until there are no further changes.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of removed nets and gates on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "manual_mux_optimizations",
            [](Netlist* nl, GateLibrary* mux_inv_gl) -> std::optional<u32> {
                auto res = netlist_preprocessing::manual_mux_optimizations(nl, mux_inv_gl);
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
                 - removing inverters incase there are inverter gates in front and behind every data input and output of the mux
                 - optimizing and therefore unifying possible inverters preceding the select signals by resynthesizing

                :param halp_py.Netlist nl: The netlist to operate on.
                :param halp_py.GateLibrary mux_inv_gl: A gate library only containing mux and inverter gates used for resynthesis.
                :returns: The difference in the total number of gates caused by these optimizations.
                :rtype: int or ``None``
            )");

        m.def(
            "propagate_constants",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::propagate_constants(nl);
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

                :param hal_py.Netlist nl: The netlist to operate on.
                :returns: The number of rerouted nets on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "remove_consecutive_inverters",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::remove_consecutive_inverters(nl);
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

        m.def(
            "simplify_lut_inits",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::simplify_lut_inits(nl);
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

        m.def(
            "reconstruct_indexed_ff_identifiers",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::reconstruct_indexed_ff_identifiers(nl);
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
                Tries to reconstruct a name and index for each flip flop that was part of a multi-bit wire in the verilog code.
                This is NOT a general netlist reverse engineering algorithm and ONLY works on synthesized netlists with names annotated by the synthesizer.
                This function mainly focuses netlists synthesized with yosys since yosys names the output wires of the flip flops but not the gate it self.
                We try to reconstruct name and index for each flip flop based on the name of its output nets.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of reconstructed names on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "reconstruct_top_module_pin_groups",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = netlist_preprocessing::reconstruct_top_module_pin_groups(nl);
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
                Tries to reconstruct top module pin groups via indexed pin names.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :returns: The number of reconstructed pin groups on success, ``None`` otherwise.
                :rtype: int or ``None``
            )");

        m.def(
            "parse_def_file",
            [](Netlist* nl, const std::filesystem::path& def_file) -> bool {
                auto res = netlist_preprocessing::parse_def_file(nl, def_file);
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

        m.def(
            "create_multi_bit_gate_modules",
            [](Netlist* nl, const std::map<std::string, std::map<std::string, std::vector<std::string>>>& concatenated_pin_groups) -> std::vector<Module*> {
                auto res = netlist_preprocessing::create_multi_bit_gate_modules(nl, concatenated_pin_groups);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return {};
                }
            },
            py::arg("nl"),
            py::arg("concatenated_pin_groups"),
            R"(
                Create modules from large gates like RAMs and DSPs with the option to concat multiple gate pin groups to larger consecutive pin groups.

                :param hal_py.Netlist nl: The netlist to operate on. 
                :param  concatenated_pin_groups: 
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        m.def(
            "create_nets_at_unconnected_pins",
            [](Netlist* nl) -> std::vector<Net*> {
                auto res = netlist_preprocessing::create_nets_at_unconnected_pins(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return {};
                }
            },
            py::arg("nl"));

        m.def(
            "unify_ff_outputs",
            [](Netlist* nl, const std::vector<Gate*>& ffs = {}, GateType* inverter_type = nullptr) -> std::optional<u32> {
                auto res = netlist_preprocessing::unify_ff_outputs(nl, ffs, inverter_type);
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
