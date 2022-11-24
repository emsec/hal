#include "hal_core/python_bindings/python_bindings.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "solve_fsm/plugin_solve_fsm.h"

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
            .def_static("solve_fsm_brute_force", [](Netlist* nl, const std::vector<Gate*> state_reg, const std::vector<Gate*> transition_logic, const std::string graph_path="") -> std::optional<std::map<u64, std::set<u64>>> {
                auto res = SolveFsmPlugin::solve_fsm_brute_force(nl, state_reg, transition_logic, graph_path);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            }, py::arg("nl"), py::arg("state_reg"), py::arg("transition_logic"), py::arg("graph_path"), R"(
            Generates the state graph of a finite state machine and returns a mapping from each state to all its possible successor states using a simple brute force approach.

            :param halPy.Netlist nl: The netlist.
            :param list[halPy.Gate] state_reg: A list containing all the gates of the fsm representing the state register.
            :param list[halPy.Gate] transition_logic: A list containing all the gates of the fsm representing the transition_logic.
            :param str graph_path: Path to the location where the state graph is saved in dot format.
            :returns: A mapping from each state to all its possible successors states.
            :rtype: dict()
        )")
            .def_static("solve_fsm", [](Netlist* nl, const std::vector<Gate*> state_reg, const std::vector<Gate*> transition_logic, const std::map<Gate*, bool> initial_state={}, const std::string graph_path="", const u32 timeout=600000) -> std::optional<std::map<u64, std::map<u64, BooleanFunction>>> {
                auto res = SolveFsmPlugin::solve_fsm(nl, state_reg, transition_logic, initial_state, graph_path, timeout);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            }, py::arg("nl"), py::arg("state_reg"), py::arg("transition_logic"), py::arg("intial_state"), py::arg("graph_path"), py::arg("timeout"), R"(
            Generates the state graph of a finite state machine and returns a mapping from each state to all its possible successor states using z3 as sat solver.

            :param halPy.Netlist nl: The netlist.
            :param list[halPy.Gate] state_reg: A list containing all the gates of the fsm representing the state register.
            :param list[halPy.Gate] transition_logic: A list containing all the gates of the fsm representing the transition_logic.
            :param dict{halPy.Gate, bool} initial_state: A mapping from the state registers to their initial value. If omitted the intial state will be set to 0.
            :param str graph_path: Path to the location where the state graph is saved in dot format.
            :param int timeout: Timeout value for the sat solver in seconds.
            :returns: A mapping from each state to all its possible successors states
            :rtype: dict()
        )")
            .def_static("generate_dot_graph", [](const std::vector<Gate*>& state_reg, const std::map<u64, std::map<u64, BooleanFunction>>& transitions, const std::string& graph_path="", const u32 max_condition_length=128, const u32 base=10) -> std::optional<std::string> {
                auto res = SolveFsmPlugin::generate_dot_graph(state_reg, transitions, graph_path, max_condition_length, base);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            }, py::arg("state_reg"), py::arg("transitions"), py::arg("graph_path"), py::arg("max_condition_length"), py::arg("base"), R"(
            Generates the state graph of a finite state machine from the transitions of that fsm.

            :param list[halPy.Gate] state_reg: Vector contianing the state registers.
            :param dict{int, dict{int, halPy.BooleanFunction}} transitions: Transitions of the fsm given as a map from origin state to all possible successor states and the corresponding condition.
            :param str graph_path: Path to the location where the state graph is saved in dot format.
            :param int max_condition_length: The maximum character length that is printed for boolean functions representing the conditions.
            :param int base: The base with that the states are formatted and printed.
            :returns: A string representing the dot graph.
            :rtype: str
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
