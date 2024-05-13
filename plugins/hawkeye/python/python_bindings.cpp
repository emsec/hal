#include "hal_core/python_bindings/python_bindings.h"

#include "hawkeye/plugin_hawkeye.h"
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
    PYBIND11_MODULE(hawkeye, m)
    {
        m.doc() = "hal HawkeyePlugin python bindings";
#else
    PYBIND11_PLUGIN(hawkeye)
    {
        py::module m("hawkeye", "hal HawkeyePlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<HawkeyePlugin, RawPtrWrapper<HawkeyePlugin>, BasePluginInterface> py_hawkeye_plugin(m, "HawkeyePlugin");
        py_hawkeye_plugin.def_property_readonly("name", &HawkeyePlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_name", &HawkeyePlugin::get_name, R"(
            Get the name of the plugin.

            :returns: Plugin name.
            :rtype: str
        )");

        py_hawkeye_plugin.def_property_readonly("description", &HawkeyePlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_description", &HawkeyePlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_hawkeye_plugin.def_property_readonly("version", &HawkeyePlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_version", &HawkeyePlugin::get_version, R"(
            Get the version of the plugin.

            :returns: Plugin version.
            :rtype: str
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
