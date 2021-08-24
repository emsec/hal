#include "hal_core/python_bindings/python_bindings.h"

#include "vcd_viewer/vcd_viewer.h"
#include "vcd_viewer/plugin_vcd_viewer.h"
#include "netlist_simulator/netlist_simulator.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(vcd_viewer, m)
    {
        m.doc() = "hal VcdViewerPlugin python bindings";
#else
    PYBIND11_PLUGIN(vcd_viewer)
    {
        py::module m("vcd_viewer", "hal VcdViewerPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<VcdViewerPlugin, RawPtrWrapper<VcdViewerPlugin>, BasePluginInterface>(m, "VcdViewerPlugin")
            .def_property_readonly("name", &VcdViewerPlugin::get_name, R"(
                The name of the plugin.

                :type: str
            )")

            .def("get_name", &VcdViewerPlugin::get_name, R"(
                Get the name of the plugin.

                :returns: The name of the plugin.
                :rtype: str
            )")

            .def_property_readonly("version", &VcdViewerPlugin::get_version, R"(
                The version of the plugin.

                :type: str
            )")

            .def("get_version", &VcdViewerPlugin::get_version, R"(
                Get the version of the plugin.

                :returns: The version of the plugin.
                :rtype: str
            )");


#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
