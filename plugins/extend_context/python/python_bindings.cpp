#include "hal_core/python_bindings/python_bindings.h"

#include "extend_context/plugin_extend_context.h"
#include "hal_core/netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;
namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(extend_context, m)
    {
        m.doc() = "hal ExtendContextPlugin python bindings";
#else
    PYBIND11_PLUGIN(extend_context)
    {
        py::module m("extend_context", "hal ExtendContextPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE
        py::class_<ExtendContextPlugin, RawPtrWrapper<ExtendContextPlugin>, BasePluginInterface>(m, "ExtendContextPlugin")
            .def_property_readonly("name", &ExtendContextPlugin::get_name, R"(
                The name of the plugin.

                :type: str
                )")
            .def("get_name", &ExtendContextPlugin::get_name, R"(
                Get the name of the plugin.

                :returns: Plugin name.
                :rtype: str
                )")
            .def_property_readonly("version", &ExtendContextPlugin::get_version, R"(
                The version of the plugin.

                :type: str
                )")
            .def("get_version", &ExtendContextPlugin::get_version, R"(
                Get the version of the plugin.

                :returns: Plugin version.
                :rtype: str
                )")
;
#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
