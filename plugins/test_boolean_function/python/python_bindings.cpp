#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "hal_core/python_bindings/python_bindings.h"

#include "test_boolean_function/plugin_test_boolean_function.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

    #ifdef PYBIND11_MODULE
    PYBIND11_MODULE(test_boolean_function, m)
    {
        m.doc() = "hal TestBooleanFunctionPlugin python bindings";
    #else
    PYBIND11_PLUGIN(test_boolean_function)
    {
        py::module m("test_boolean_function", "hal TestBooleanFunctionPlugin python bindings");
    #endif    // ifdef PYBIND11_MODULE

        py::class_<TestBooleanFunctionPlugin, RawPtrWrapper<TestBooleanFunctionPlugin>, BasePluginInterface>(m, "TestBooleanFunctionPlugin")
            .def_property_readonly("name", &TestBooleanFunctionPlugin::get_name)
            .def("get_name", &TestBooleanFunctionPlugin::get_name)
            .def_property_readonly("version", &TestBooleanFunctionPlugin::get_version)
            .def("get_version", &TestBooleanFunctionPlugin::get_version)
            ;

    #ifndef PYBIND11_MODULE
        return m.ptr();
    #endif    // PYBIND11_MODULE
    }
}
