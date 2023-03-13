#include "hal_core/python_bindings/python_bindings.h"

#include "netlist_simulator/netlist_simulator.h"
#include "netlist_simulator/plugin_netlist_simulator.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

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
        py::module m("hal_simulator", "hal NetlistSimulatorPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<NetlistSimulatorPlugin, RawPtrWrapper<NetlistSimulatorPlugin>, BasePluginInterface>(m, "NetlistSimulatorPlugin")
            .def_property_readonly("name", &NetlistSimulatorPlugin::get_name, R"(
                The name of the plugin.

                :type: str
            )")

            .def("get_name", &NetlistSimulatorPlugin::get_name, R"(
                Get the name of the plugin.

                :returns: The name of the plugin.
                :rtype: str
            )")

            .def_property_readonly("version", &NetlistSimulatorPlugin::get_version, R"(
                The version of the plugin.

                :type: str
            )")

            .def("get_version", &NetlistSimulatorPlugin::get_version, R"(
                Get the version of the plugin.

                :returns: The version of the plugin.
                :rtype: str
            )");

        py::class_<NetlistSimulator>(m, "NetlistSimulator")
            /*
            .def("add_gates", &NetlistSimulator::add_gates, py::arg("gates"), R"(
                Add gates to the simulation set.
                Only elements in the simulation set are considered during simulation.

                :param list[hal_py.Gate] gates: The gates to add.
            )")

            .def("add_clock_frequency", &NetlistSimulator::add_clock_frequency, py::arg("clock_net"), py::arg("frequency"), py::arg("start_at_zero") = true, R"(
                Specify a net that carries the clock signal and set the clock frequency in hertz.

                :param hal_py.Net clock_net: The net that carries the clock signal.
                :param int frequency: The clock frequency in hertz.
                :param bool start_at_zero: Initial clock state is 0 if true, 1 otherwise.
            )")

            .def("add_clock_period", &NetlistSimulator::add_clock_period, py::arg("clock_net"), py::arg("period"), py::arg("start_at_zero") = true, R"(
                Specify a net that carries the clock signal and set the clock period in picoseconds.
        
                :param hal_py.Net clock_net: The net that carries the clock signal.
                :param int period: The clock period from rising edge to rising edge in picoseconds.
                :param bool start_at_zero: Initial clock state is 0 if true, 1 otherwise.
            )")

            .def("get_gates", &NetlistSimulator::get_gates, R"(
                Get all gates that are in the simulation set.

                :returns: The name.
                :rtype: set[hal_py.Gate]
            )")

            .def("get_input_nets", &NetlistSimulator::get_input_nets, R"(
                Get all nets that are considered inputs, i.e., not driven by a gate in the simulation set or global inputs.

                :returns: The input nets.
                :rtype: list[hal_py.Net]
            )")

            .def("get_output_nets", &NetlistSimulator::get_output_nets, R"(
                Get all output nets of gates in the simulation set that have a destination outside of the set or that are global outputs.

                :returns: The output nets.
                :rtype: list[hal_py.Net]
            )")
*/
            .def("set_input", &NetlistSimulator::set_input, py::arg("net"), py::arg("value"), R"(
                Set the signal for a specific wire to control input signals between simulation cycles.
            
                :param hal_py.Net net: The net to set a signal value for.
                :param hal_py.BooleanFunction.Value value: The value to set.
            )")

            .def("initialize_sequential_gates", py::overload_cast<const std::function<bool(const Gate*)>&>(&NetlistSimulator::initialize_sequential_gates), py::arg("filter") = nullptr, R"(
                Configure the sequential gates matching the (optional) user-defined filter condition with initialization data specified within the netlist.
                Schedules the respective gates for initialization, the actual configuration is applied during initialization of the simulator.
                This function can only be called before the simulation has started.
         
                :param lambda filter: The optional filter to be applied before initialization.
            )")

            .def("initialize_sequential_gates",
                 py::overload_cast<BooleanFunction::Value, const std::function<bool(const Gate*)>&>(&NetlistSimulator::initialize_sequential_gates),
                 py::arg("value"),
                 py::arg("filter") = nullptr,
                 R"(
                Configure the sequential gates matching the (optional) user-defined filter condition with the specified value.
                Schedules the respective gates for initialization, the actual configuration is applied during initialization of the simulator.
                This function can only be called before the simulation has started.
         
                :param hal_py.BooleanFunction.Value value: The value to initialize the selected gates with.
                :param lambda filter: The optional filter to be applied before initialization.
            )")

            .def("load_initial_values", &NetlistSimulator::load_initial_values, py::arg("value"), R"(
                Load the specified initial value into the current state of all sequential elements.

                :param hal_py.BooleanFunction.Value value: The initial value to load.
            )")

            .def("load_initial_values_from_netlist", &NetlistSimulator::load_initial_values_from_netlist, R"(
                Load the initial value specified within the netlist file into the current state of all sequential elements.
                This is especially relevant for FPGA netlists, since these may provide initial values to load on startup.
            )")

            .def("initialize", &NetlistSimulator::initialize, R"(
                Initialize the simulation.
                No additional gates or clocks can be added after this point.
            )")

            .def("simulate", &NetlistSimulator::simulate, py::arg("picoseconds"), R"(
                Simulate for a specific period, advancing the internal state.
                Automatically initializes the simulation if 'initialize' has not yet been called.
                Use \p set_input to control specific signals.
         
                :param int picoseconds: The duration to simulate.
            )")

            .def("reset", &NetlistSimulator::reset, R"(
                Reset the simulator state, i.e., treat all signals as unknown.
                Does not remove gates/nets from the simulation set.
            )")

            .def("set_simulation_state", &NetlistSimulator::set_simulation_state, py::arg("state"), R"(
                Set the simulator state, i.e., net signals, to a given state.
                Does not influence gates/nets added to the simulation set.
        
                :param netlist_simulator.Simulation state: The state to apply.
            )")

            .def("get_simulation_state", &NetlistSimulator::get_simulation_state, R"(
                Get the current simulation state.
        
                :returns: The current simulation state.
                :rtype: libnetlist_simulator.Simulation
            )")

            .def("set_iteration_timeout", &NetlistSimulator::set_iteration_timeout, py::arg("iterations"), R"(
                Set the iteration timeout, i.e., the maximum number of events processed for a single point in time.
                Useful to abort in case of infinite loops.
                A value of 0 disables the timeout.
        
                :param int iterations: The iteration timeout.
            )")

            .def("get_simulation_timeout", &NetlistSimulator::get_simulation_timeout, R"(
                Get the current iteration timeout value.

                :returns: The iteration timeout.
                :rtype: int
            )")

            .def("generate_vcd", &NetlistSimulator::generate_vcd, py::arg("path"), py::arg("start_time"), py::arg("end_time"), py::arg("nets") = std::set<u32>(), R"(
                Generates the a VCD file for parts the simulated netlist.

                :param pathlib.Path path: The path to the VCD file.
                :param int start_time: Start of the timeframe to write to the file (in picoseconds).
                :param int end_time: End of the timeframe to write to the file (in picoseconds).
                :param set[hal_py.Net] nets: Nets to include in the VCD file.
                :returns: True if the file gerneration was successful, false otherwise.
                :rtype: bool
            )");

        py::class_<Simulation>(m, "Simulation")
            .def(py::init<>())

            .def("get_net_value", &Simulation::get_net_value, py::arg("net"), py::arg("time"), R"(
                Get the signal value of a specific net at a specific point in time specified in picoseconds.
         
                :param hal_py.Net net: The net to inspect.
                :param int time: The time in picoseconds.
                :returns: The net's signal value.
                :rtype: hal_py.BooleanFunction.Value
            )")

            .def("add_event", &Simulation::add_event, py::arg("event"), R"(
                Adds a custom event to the simulation.
         
                :param netlist_simulator.Event event: The event to add.
            )")

            .def("get_events", &Simulation::get_events, R"(
                Get all events of the simulation.

                :returns: A map from net to associated events for that net sorted by time.
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
