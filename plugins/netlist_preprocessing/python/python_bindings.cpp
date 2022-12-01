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
            :returns: The number of removed gates on success, an error otherwise.
            :rtype: int or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
