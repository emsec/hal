#include "hal_core/python_bindings/python_bindings.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "solve_fsm/plugin_solve_fsm.h"
#include "solve_fsm/solve_fsm.h"

#include <map>
namespace py = pybind11;

namespace hal
{
    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(solve_fsm, m)
    {
        m.doc() = "Plugin to automatically generate FSM state transition graphs for given FSMs.";
#else
    PYBIND11_PLUGIN(solve_fsm)
    {
        py::module m("solve_fsm", "Plugin to automatically generate FSM state transition graphs for given FSMs.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<SolveFsmPlugin, RawPtrWrapper<SolveFsmPlugin>, BasePluginInterface> py_solve_fsm(
            m, "SolveFsmPlugin", R"(This class provides an interface to integrate FSM solving as a plugin within the HAL framework.)");
        py_solve_fsm.def_property_readonly("name", &SolveFsmPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_solve_fsm.def("get_name", &SolveFsmPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: Plugin name.
            :rtype: str
        )");

        py_solve_fsm.def_property_readonly("version", &SolveFsmPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_solve_fsm.def("get_version", &SolveFsmPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: Plugin version.
            :rtype: str
        )");

        py_solve_fsm.def_property_readonly("description", &SolveFsmPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_solve_fsm.def("get_description", &SolveFsmPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        m.def(
            "solve_fsm_brute_force",
            [](Netlist* nl, const std::vector<Gate*>& state_reg, const std::vector<Gate*>& transition_logic, const std::string& graph_path = "")
                -> std::optional<std::map<u64, std::map<u64, BooleanFunction>>> {
                auto res = solve_fsm::solve_fsm_brute_force(nl, state_reg, transition_logic, graph_path);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("state_reg"),
            py::arg("transition_logic"),
            py::arg("graph_path") = std::string(""),
            R"(
                Generate the state transition graph of a given FSM using brute force.
                The result is a map from each state of the FSM to all of its transitions.
                A transition is given as each successor state as well as the Boolean condition that needs to be fulfilled for the transition to take place.
                Optionally also produces a DOT file representing the state transition graph.
                
                :param hal_py.Netlist nl: The netlist to operate on.
                :param list[hal_py.Gate] state_reg: A list of flip-flop gates that make up the state register of the FSM.
                :param list[hal_py.Gate] transition_logic: A list of combinational gates that make up the transition logic of the FSM.
                :param pathlib.Path graph_path: File path at which to store the DOT state transition graph. No file is created if the path is left empty. Defaults to an empty path.
                :returns: A dict from each state to its successor states as well as the condition for the respective transition to be taken on success, `None` otherwise.
                :rtype: dict[int,dict[int,hal_py.BooleanFunction]] or None
            )");

        m.def(
            "solve_fsm",
            [](Netlist* nl,
               const std::vector<Gate*>& state_reg,
               const std::vector<Gate*>& transition_logic,
               const std::map<Gate*, bool>& initial_state = {},
               const std::filesystem::path& graph_path    = "",
               const u32 timeout                          = 600000) -> std::optional<std::map<u64, std::map<u64, BooleanFunction>>> {
                auto res = solve_fsm::solve_fsm(nl, state_reg, transition_logic, initial_state, graph_path, timeout);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("state_reg"),
            py::arg("transition_logic"),
            py::arg("initial_state") = std::map<Gate*, bool>(),
            py::arg("graph_path")    = "",
            py::arg("timeout")       = 600000,
            R"(
                Generate the state transition graph of a given FSM using SMT solving.
                The result is a map from each state of the FSM to all of its transitions.
                A transition is given as each successor state as well as the Boolean condition that needs to be fulfilled for the transition to take place.
                Optionally also produces a DOT file representing the state transition graph.
                
                :param hal_py.Netlist nl: The netlist to operate on.
                :param list[hal_py.Gate] state_reg: A list of flip-flop gates that make up the state register of the FSM.
                :param list[hal_py.Gate] transition_logic: A list of combinational gates that make up the transition logic of the FSM.
                :param dict[hal_py.Gate,bool] initial_state: A dict from the state register flip-flops to their initial (Boolean) value. If an empty dict is provided, the initial state is set to 0. Defaults to an empty dict.
                :param pathlib.Path graph_path: File path at which to store the DOT state transition graph. No file is created if the path is left empty. Defaults to an empty path.
                :param int timeout: Timeout for the underlying SAT solvers. Defaults to 600000 ms.
                :returns: A dict from each state to its successor states as well as the condition for the respective transition to be taken on success, `None` otherwise.
                :rtype: dict[int,dict[int,hal_py.BooleanFunction]] or None
            )");

        m.def(
            "generate_dot_graph",
            [](const std::vector<Gate*>& state_reg,
               const std::map<u64, std::map<u64, BooleanFunction>>& transitions,
               const std::string& graph_path  = "",
               const u32 max_condition_length = 128,
               const u32 base                 = 10) -> std::optional<std::string> {
                auto res = solve_fsm::generate_dot_graph(state_reg, transitions, graph_path, max_condition_length, base);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("state_reg"),
            py::arg("transitions"),
            py::arg("graph_path")           = std::string(),
            py::arg("max_condition_length") = 128,
            py::arg("base")                 = 10,
            R"(
            Generates the state graph of a finite state machine from the transitions of that fsm.

            :param list[hal_py.Gate] state_reg: Vector contianing the state registers.
            :param dict[int, dict[int, hal_py.BooleanFunction]] transitions: Transitions of the fsm given as a map from origin state to all possible successor states and the corresponding condition.
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
