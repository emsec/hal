#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "hal_core/python_bindings/python_bindings.h"

#include "genlib_writer/plugin_genlib_writer.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

    #ifdef PYBIND11_MODULE
    PYBIND11_MODULE(genlib_writer, m)
    {
        m.doc() = "hal GenlibWriterPlugin python bindings";
    #else
    PYBIND11_PLUGIN(genlib_writer)
    {
        py::module m("genlib_writer", "hal GenlibWriterPlugin python bindings");
    #endif    // ifdef PYBIND11_MODULE

        py::class_<GenlibWriterPlugin, RawPtrWrapper<GenlibWriterPlugin>, BasePluginInterface>(m, "GenlibWriterPlugin")
            .def_property_readonly("name", &GenlibWriterPlugin::get_name)
            .def("get_name", &GenlibWriterPlugin::get_name)
            .def_property_readonly("version", &GenlibWriterPlugin::get_version)
            .def("get_version", &GenlibWriterPlugin::get_version)
            ;

    #ifndef PYBIND11_MODULE
        return m.ptr();
    #endif    // PYBIND11_MODULE
    }
}
