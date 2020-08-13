#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "plugin_netlist_simulator/plugin_netlist_simulator.h"

namespace py = pybind11;

namespace hal
{

    #ifdef PYBIND11_MODULE
    PYBIND11_MODULE(netlist_simulator, m)
    {
        m.doc() = "hal NetlistSimulatorPlugin python bindings";
    #else
    PYBIND11_PLUGIN(netlist_simulator)
    {
        py::module m("netlist_simulator", "hal NetlistSimulatorPlugin python bindings");
    #endif    // ifdef PYBIND11_MODULE

        py::class_<NetlistSimulatorPlugin, BasePluginInterface>(m, "netlist_simulator")
            .def_property_readonly("name", &NetlistSimulatorPlugin::get_name)
            .def("get_name", &NetlistSimulatorPlugin::get_name)
            .def_property_readonly("version", &NetlistSimulatorPlugin::get_version)
            .def("get_version", &NetlistSimulatorPlugin::get_version)
            ;

    #ifndef PYBIND11_MODULE
        return m.ptr();
    #endif    // PYBIND11_MODULE
    }
}
