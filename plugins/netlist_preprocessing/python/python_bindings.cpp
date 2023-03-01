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
            :returns: The number of removed LUT endpoints on success, None otherwise.
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
            :returns: The number of removed buffers on success, None otherwise.
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
            :returns: The number of removed gates on success, None otherwise.
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
            :returns: The number of removed gates on success, None otherwise.
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
            :returns: The number of removed nets on success, None otherwise.
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
            Replaces pins connected to GND/VCC with constants and simplifies the booleanfunction of a LUT but recomputing the INIT string.

            :param hal_py.Netlist nl: The netlist to operate on. 
            :returns: The number of simplified INIT strings on success, an error otherwise.
            :rtype: int or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
