#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/gate.h"
#include "solve_fsm/plugin_solve_fsm.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "hal_core/python_bindings/python_bindings.h"

#include <map>
namespace py = pybind11;

namespace hal
{
    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(solve_fsm, m)
    {
        m.doc() = "hal SolveFsmPlugin python bindings";
#else
    PYBIND11_PLUGIN(solve_fsm)
    {
        py::module m("solve_fsm", "hal SolveFsmPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<SolveFsmPlugin, RawPtrWrapper<SolveFsmPlugin>, BasePluginInterface>(m, "SolveFsmPlugin")
            .def_property_readonly("name", &SolveFsmPlugin::get_name)
            .def("get_name", &SolveFsmPlugin::get_name)
            .def_property_readonly("version", &SolveFsmPlugin::get_version)
            .def("get_version", &SolveFsmPlugin::get_version)
            .def("solve_fsm", &SolveFsmPlugin::solve_fsm, py::arg("nl"), py::arg("state_reg"), py::arg("transition_logic"), py::arg("intial_state"), py::arg("timeout"), R"(
            Generates the state graph of a finite state machine in dot format of a given fsm using the z3 as a sat solver.

            :param halPy.Netlist nl: The netlist.
            :param list[halPy.Gate] state_reg: A list containing all the gates of the fsm representing the state register.
            :param list[halPy.Gate] transition_logic: A list containing all the gates of the fsm representing the transition_logic.
            :param dict{halPy.Gate, bool} initial_state - A mapping from the state registers to their initial value. If omitted the intial state will be set to 0.
            :param int timeout - Timeout value for the sat solvers. Defaults to 600 (unit unkown).
            :returns: The transition state graph in dot format.
            :rtype: str
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
