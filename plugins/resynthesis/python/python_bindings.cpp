#include "hal_core/python_bindings/python_bindings.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "resynthesis/plugin_resynthesis.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(resynthesis, m)
    {
        m.doc() = "Provides functions to re-synthesize (parts of) a gate-level netlist.";
#else
    PYBIND11_PLUGIN(resynthesis)
    {
        py::module m("resynthesis", "Provides functions to re-synthesize (parts of) a gate-level netlist.");
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

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
