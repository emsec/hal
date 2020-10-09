#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "hal_core/python_bindings/python_bindings.h"

#include "netlist_simulator/plugin_netlist_simulator.h"
#include "netlist_simulator/netlist_simulator.h"

namespace py = pybind11;

namespace hal
{

    #ifdef PYBIND11_MODULE
    PYBIND11_MODULE(libnetlist_simulator, m)
    {
        m.doc() = "hal NetlistSimulatorPlugin python bindings";
    #else
    PYBIND11_PLUGIN(libnetlist_simulator)
    {
        py::module m("netlist_simulator", "hal NetlistSimulatorPlugin python bindings");
    #endif    // ifdef PYBIND11_MODULE

        py::class_<NetlistSimulatorPlugin, RawPtrWrapper<NetlistSimulatorPlugin>, BasePluginInterface>(m, "NetlistSimulatorPlugin")
            .def_property_readonly("name", &NetlistSimulatorPlugin::get_name)
            .def("get_name", &NetlistSimulatorPlugin::get_name)
            .def_property_readonly("version", &NetlistSimulatorPlugin::get_version)
            .def("get_version", &NetlistSimulatorPlugin::get_version)
            .def("create_simulator", &NetlistSimulatorPlugin::create_simulator)
            ;

        py::class_<NetlistSimulator>(m, "NetlistSimulator")
            .def("add_gates", &NetlistSimulator::add_gates)
            .def("add_clock_hertz", &NetlistSimulator::add_clock_hertz)
            .def("add_clock_period", &NetlistSimulator::add_clock_period)
            .def("get_gates", &NetlistSimulator::get_gates)
            .def("get_input_nets", &NetlistSimulator::get_input_nets)
            .def("get_output_nets", &NetlistSimulator::get_output_nets)
            .def("set_input", &NetlistSimulator::set_input)
            .def("load_initial_values", &NetlistSimulator::load_initial_values)
            .def("simulate", &NetlistSimulator::simulate)
            .def("reset", &NetlistSimulator::reset)
            .def("set_simulation_state", &NetlistSimulator::set_simulation_state)
            .def("get_simulation_state", &NetlistSimulator::get_simulation_state)
            .def("set_iteration_timeout", &NetlistSimulator::set_iteration_timeout)
            .def("get_simulation_timeout", &NetlistSimulator::get_simulation_timeout)
            ;

        py::class_<Simulation>(m, "Simulation")
            .def(py::init<>())
            .def("get_net_value", &Simulation::get_net_value)
            .def("add_event", &Simulation::add_event)
            .def("get_events", &Simulation::get_events)
            ;

        py::enum_<SignalValue>(m, "SignalValue")
            .value("X", SignalValue::X)
            .value("ZERO", SignalValue::ZERO)
            .value("ONE", SignalValue::ONE)
            .value("Z", SignalValue::Z)
            .export_values();

    #ifndef PYBIND11_MODULE
        return m.ptr();
    #endif    // PYBIND11_MODULE
    }
}
