#include "hal_core/python_bindings/python_bindings.h"

#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(netlist_simulator_controller, m)
    {
        m.doc() = "hal NetlistSimulatorControllerPlugin python bindings";
#else
    PYBIND11_PLUGIN(netlist_simulator_controller)
    {
        py::module m("netlist_simulator_controller", "hal NetlistSimulatorControllerPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<NetlistSimulatorControllerPlugin, RawPtrWrapper<NetlistSimulatorControllerPlugin>, BasePluginInterface>(m, "NetlistSimulatorControllerPlugin")
            .def_property_readonly("name", &NetlistSimulatorControllerPlugin::get_name, R"(
                The name of the plugin.

                :type: str
            )")

            .def("get_name", &NetlistSimulatorControllerPlugin::get_name, R"(
                Get the name of the plugin.

                :returns: The name of the plugin.
                :rtype: str
            )")

            .def_property_readonly("version", &NetlistSimulatorControllerPlugin::get_version, R"(
                The version of the plugin.

                :type: str
            )")

            .def("get_version", &NetlistSimulatorControllerPlugin::get_version, R"(
                Get the version of the plugin.

                :returns: The version of the plugin.
                :rtype: str
            )")

            .def("create_simulator_controller", &NetlistSimulatorControllerPlugin::create_simulator_controller, R"(
                Create a netlist simulation controller instance.

                :returns: The simulator controller instance.
                :rtype: netlist_simulator_controller.NetlistSimulatorController
            )");

        py::class_<NetlistSimulatorController>(m, "NetlistSimulatorController")

            .def("add_gates", &NetlistSimulatorController::add_gates, py::arg("gates"), R"(
                Add gates to the simulation set.
                Only elements in the simulation set are considered during simulation.

                :param list[hal_py.Gate] gates: The gates to add.
            )")

            .def("add_clock_frequency", &NetlistSimulatorController::add_clock_frequency, py::arg("clock_net"), py::arg("frequency"), py::arg("start_at_zero") = true, R"(
                Specify a net that carries the clock signal and set the clock frequency in hertz.

                :param hal_py.Net clock_net: The net that carries the clock signal.
                :param int frequency: The clock frequency in hertz.
                :param bool start_at_zero: Initial clock state is 0 if true, 1 otherwise.
            )")

            .def("add_clock_period", &NetlistSimulatorController::add_clock_period, py::arg("clock_net"), py::arg("period"), py::arg("start_at_zero") = true, R"(
                Specify a net that carries the clock signal and set the clock period in picoseconds.
        
                :param hal_py.Net clock_net: The net that carries the clock signal.
                :param int period: The clock period from rising edge to rising edge in picoseconds.
                :param bool start_at_zero: Initial clock state is 0 if true, 1 otherwise.
            )")

            .def("get_gates", &NetlistSimulatorController::get_gates, R"(
                Get all gates that are in the simulation set.

                :returns: The name.
                :rtype: set[hal_py.Gate]
            )")

            .def("get_input_nets", &NetlistSimulatorController::get_input_nets, R"(
                Get all nets that are considered inputs, i.e., not driven by a gate in the simulation set or global inputs.

                :returns: The input nets.
                :rtype: list[hal_py.Net]
            )")

            .def("get_output_nets", &NetlistSimulatorController::get_output_nets, R"(
                Get all output nets of gates in the simulation set that have a destination outside of the set or that are global outputs.

                :returns: The output nets.
                :rtype: list[hal_py.Net]
            )")

            .def("set_input", &NetlistSimulatorController::set_input, py::arg("net"), py::arg("value"), R"(
                Set the signal for a specific wire to control input signals between simulation cycles.
            
                :param hal_py.Net net: The net to set a signal value for.
                :param hal_py.BooleanFunction.Value value: The value to set.
            )")

            .def("get_engine_names", &NetlistSimulatorController::get_engine_names, R"(
                Get a list of registered simulation engines.

                :returns: A list of registered sim.
                :rtype: list[str]
             )")

             .def("create_simulation_engine", &NetlistSimulatorController::create_simulation_engine, py::arg("name"), R"(
                  Select and set the engine for simulation. The list of available engines is provided by function get_engine_names().

                  :param str name: The name of the engine to be selected.
                  :returns: The engine created by factory method
                  :rtype: netlist_simulator_controller.SimulationEngine or None
              )")

             .def("run_simulation", &NetlistSimulatorController::run_simulation, R"("
                  Run the simulation

                  :returns: True if all simulation steps could be processed successfully, False otherwise
                  :rtype: bool
             )")
                /*
            .def("initialize_sequential_gates", py::overload_cast<const std::function<bool(const Gate*)>&>(&NetlistSimulator::initialize_sequential_gates), py::arg("filter") = nullptr, R"(
                Configure the sequential gates matching the (optional) user-defined filter condition with initialization data specified within the netlist.
                Schedules the respective gates for initialization, the actual configuration is applied during initialization of the simulator.
                This function can only be called before the simulation has started.
         
                :param lambda filter: The optional filter to be applied before initialization.
            )")

            .def("initialize_sequential_gates", py::overload_cast<BooleanFunction::Value, const std::function<bool(const Gate*)>&>(&NetlistSimulator::initialize_sequential_gates), py::arg("value"), py::arg("filter") = nullptr, R"(
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
*/

            .def("initialize", &NetlistSimulatorController::initialize, R"(
                Initialize the simulation.
                No additional gates or clocks can be added after this point.
            )")

            .def("simulate", &NetlistSimulatorController::simulate, py::arg("picoseconds"), R"(
                Simulate for a specific period, advancing the internal state.
                Automatically initializes the simulation if 'initialize' has not yet been called.
                Use \p set_input to control specific signals.
         
                :param int picoseconds: The duration to simulate.
            )")

            .def("reset", &NetlistSimulatorController::reset, R"(
                Reset the simulator state, i.e., treat all signals as unknown.
                Does not remove gates/nets from the simulation set.
            )")

            .def("request_generate_vcd", &NetlistSimulatorController::request_generate_vcd, py::arg("filename"), R"(
                Request to engine to generate VCD reult file upon successful completion.

                :param str filename: filename for requested VCD result file in engine working directory.")");
                /*
            .def("set_simulation_state", &NetlistSimulator::set_simulation_state, py::arg("state"), R"(
                Set the simulator state, i.e., net signals, to a given state.
                Does not influence gates/nets added to the simulation set.
        
                :param netlist_simulator_controller.Simulation state: The state to apply.
            )")

            .def("get_simulation_state", &NetlistSimulator::get_simulation_state, R"(
                Get the current simulation state.
        
                :returns: The current simulation state.
                :rtype: libnetlist_simulator_controller.Simulation
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

                :param hal_py.hal_path path: The path to the VCD file.
                :param int start_time: Start of the timeframe to write to the file (in picoseconds).
                :param int end_time: End of the timeframe to write to the file (in picoseconds).
                :param set[hal_py.Net] nets: Nets to include in the VCD file.
                :returns: True if the file gerneration was successful, false otherwise.
                :rtype: bool
            )")*/;

        py::class_<WaveEvent>(m, "WaveEvent")
            .def(py::init<>(), R"(Construct a new event.)")

            .def_readwrite("affected_net", &WaveEvent::affected_net, R"(
                The net affected by the event.

                :type: hal_py.Net
            )")

            .def_readwrite("new_value", &WaveEvent::new_value, R"(
                The new value caused by the event.

                :type: hal_py.BooleanFunction.Value
            )")

            .def_readwrite("time", &WaveEvent::time, R"(
                The time of the event.

                :type: int
            )")

            .def_readwrite("id", &WaveEvent::id, R"(
                The unique ID of the event.

                :type: int
            )")

            .def(py::self == py::self, R"(
                Tests whether two events are equal.

                :returns: True when both events are equal, false otherwise.
                :rtype: bool
            )")

            .def(py::self < py::self, R"(
                Tests whether one event happened before the other.
         
                :returns: True when this event happened before the other, false otherwise.
                :rtype: bool
            )");

        py::class_<SimulationEngine, RawPtrWrapper<SimulationEngine>> py_simulation_engine(m, "SimulationEngine", R"(
             The engine which does the active part of the simulation as seperate process or thread.
        )");

        py_simulation_engine.def("name", &SimulationEngine::name, R"(
                Get the name of the engine.

                :returns: The name of the engine.
                :rtype: str
        )");

        py_simulation_engine.def("get_state", &SimulationEngine::get_state, R"(
                Get the state of the engine.

                :returns: The state of the engine (Preparing=2, Running=1, Done=0, Failed=-1)
                :type: int
        )");

        py_simulation_engine.def("directory", &SimulationEngine::directory, R"(
                Get the working directory of the engine.

                :returns: The working directory of the engine.
                :rtype: str
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
