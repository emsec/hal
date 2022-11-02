#include "hal_core/python_bindings/python_bindings.h"

#include "plugin_smt_utils.h"
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
    PYBIND11_MODULE(smt_utils, m)
    {
        m.doc() = "hal SmtUtilsPlugin python bindings";
#else
    PYBIND11_PLUGIN(smt_utils)
    {
        py::module m("smt_utils", "hal SmtUtilsPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<SmtUtilsPlugin, RawPtrWrapper<SmtUtilsPlugin>, BasePluginInterface>(m, "SmtUtilsPlugin")
            .def_property_readonly("name", &SmtUtilsPlugin::get_name)
            .def("get_name", &SmtUtilsPlugin::get_name)
            .def_property_readonly("version", &SmtUtilsPlugin::get_version)
            .def("get_version", &SmtUtilsPlugin::get_version);

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
