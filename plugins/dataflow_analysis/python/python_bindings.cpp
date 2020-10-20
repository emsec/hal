#include "hal_core/python_bindings/python_bindings.h"

#include "dataflow_analysis/plugin_dataflow.h"
#include "hal_core/netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;
namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(dataflow, m)
    {
        m.doc() = "hal DataflowPlugin python bindings";
#else
    PYBIND11_PLUGIN(dataflow)
    {
        py::module m("dataflow", "hal Dataflow python bindings");
#endif    // ifdef PYBIND11_MODULE
        py::class_<plugin_dataflow, RawPtrWrapper<plugin_dataflow>, BasePluginInterface>(m, "plugin_dataflow")
            .def_property_readonly("name", &plugin_dataflow::get_name)
            .def("get_name", &plugin_dataflow::get_name)
            .def_property_readonly("version", &plugin_dataflow::get_version)
            .def("get_version", &plugin_dataflow::get_version)
            // .def("get_cli_options", &plugin_dataflow::get_cli_options)
            // .def("handle_cli_call", &plugin_dataflow::handle_cli_call)
            .def("execute", &plugin_dataflow::execute);
#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
