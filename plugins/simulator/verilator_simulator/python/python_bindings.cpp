#include "hal_core/python_bindings/python_bindings.h"

#include "verilator_simulator/verilator_simulator.h"
#include "verilator_simulator/plugin_verilator_simulator.h"
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
    PYBIND11_MODULE(verilator_simulator, m)
    {
        m.doc() = "hal VerilatorSimulatorPlugin python bindings";
#else
    PYBIND11_PLUGIN(verilator_simulator)
    {
        py::module m("verilator_simulator", "hal VerilatorSimulatorPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<VerilatorSimulatorPlugin, RawPtrWrapper<VerilatorSimulatorPlugin>, BasePluginInterface>(m, "VerilatorSimulatorPlugin")
            .def_property_readonly("name", &VerilatorSimulatorPlugin::get_name)
            .def("get_name", &VerilatorSimulatorPlugin::get_name)
            .def_property_readonly("version", &VerilatorSimulatorPlugin::get_version)
            .def("get_version", &VerilatorSimulatorPlugin::get_version);

        auto py_verilator_simulator = m.def_submodule("VerilatorSimulator");
        py_verilator_simulator.def("convert_gate_library_to_verilog", &verilator_simulator::converter::convert_gate_library_to_verilog);
        // py_verilator_simulator.def("verify_counter", &verilator_simulator::verify_counter);

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
