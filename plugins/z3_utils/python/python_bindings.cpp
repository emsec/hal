#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "hal_core/python_bindings/python_bindings.h"

#include "plugin_z3_utils.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

    #ifdef PYBIND11_MODULE
    PYBIND11_MODULE(z3_utils, m)
    {
        m.doc() = "hal Z3UtilsPlugin python bindings";
    #else
    PYBIND11_PLUGIN(z3_utils)
    {
        py::module m("z3_utils", "hal Z3UtilsPlugin python bindings");
    #endif    // ifdef PYBIND11_MODULE

        py::class_<Z3UtilsPlugin, RawPtrWrapper<Z3UtilsPlugin>, BasePluginInterface>(m, "Z3UtilsPlugin")
            .def_property_readonly("name", &Z3UtilsPlugin::get_name)
            .def("get_name", &Z3UtilsPlugin::get_name)
            .def_property_readonly("version", &Z3UtilsPlugin::get_version)
            .def("get_version", &Z3UtilsPlugin::get_version)
            ;

    #ifndef PYBIND11_MODULE
        return m.ptr();
    #endif    // PYBIND11_MODULE
    }
}
