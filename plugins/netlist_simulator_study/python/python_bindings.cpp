#include "hal_core/python_bindings/python_bindings.h"

#include "netlist_simulator_study/netlist_simulator_study.h"
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
    PYBIND11_MODULE(netlist_simulator_study, m)
    {
        m.doc() = "hal netlist_simulator_study python bindings";
#else
    PYBIND11_PLUGIN(netlist_modifier)
    {
        py::module m("netlist_simulator_study", "hal netlist_simulator_study python bindings");
#endif

        // define all exposed functions
        py::class_<NetlistSimulatorStudyPlugin, RawPtrWrapper<NetlistSimulatorStudyPlugin>, BasePluginInterface>(m, "NetlistSimulatorStudyPlugin")
            .def_property_readonly("name", &NetlistSimulatorStudyPlugin::get_name)
            .def("get_name", &NetlistSimulatorStudyPlugin::get_name)
            .def_property_readonly("version", &NetlistSimulatorStudyPlugin::get_version)
            .def("get_version", &NetlistSimulatorStudyPlugin::get_version)
            // python handler to stat the simulation from a python script
            .def("init_simulation", &NetlistSimulatorStudyPlugin::init_simulation_from_python, py::arg("input_file"), py::arg("probe_nets"))
            .def(py::init<>());

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif
    }
}    // namespace hal