#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "hal_core/python_bindings/python_bindings.h"

#include "perf_test/plugin_perf_test.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

    #ifdef PYBIND11_MODULE
    PYBIND11_MODULE(perf_test, m)
    {
        m.doc() = "hal PerfTestPlugin python bindings";
    #else
    PYBIND11_PLUGIN(perf_test)
    {
        py::module m("perf_test", "hal PerfTestPlugin python bindings");
    #endif    // ifdef PYBIND11_MODULE

        py::class_<PerfTestPlugin, RawPtrWrapper<PerfTestPlugin>, BasePluginInterface>(m, "PerfTestPlugin")
            .def_property_readonly("name", &PerfTestPlugin::get_name)
            .def("get_name", &PerfTestPlugin::get_name)
            .def_property_readonly("version", &PerfTestPlugin::get_version)
            .def("get_version", &PerfTestPlugin::get_version)
            ;

    #ifndef PYBIND11_MODULE
        return m.ptr();
    #endif    // PYBIND11_MODULE
    }
}
