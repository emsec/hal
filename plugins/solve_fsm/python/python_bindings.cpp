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

        py::class_<solve_fsm::Configuration> py_solve_fsm_configuration(m, "Configuration", R"(
            The configuration of a run of the FSM solver.

            Holds everything the solver needs to know about the FSM, including the netlist that implements it.
            The state register and the transition logic are mandatory, everything else is optional.

            States are encoded as integers, with the first flip-flop of the state register providing the least significant bit.
        )");

        py_solve_fsm_configuration.def(py::init<Netlist*>(), py::arg("nl"), R"(
            Construct a new FSM solver configuration for the given netlist.

            :param hal_py.Netlist nl: The netlist that implements the FSM.
        )");

        py_solve_fsm_configuration.def_readwrite("netlist", &solve_fsm::Configuration::netlist, R"(
            The netlist that implements the FSM.

            :type: hal_py.Netlist
        )");

        py_solve_fsm_configuration.def_readwrite("state_register", &solve_fsm::Configuration::state_register, R"(
            The flip-flops that make up the state register of the FSM.

            The first flip-flop provides the least significant bit of the state.
            Defaults to an empty list, but a state register is required for the solver to run.

            :type: list[hal_py.Gate]
        )");

        py_solve_fsm_configuration.def_readwrite("transition_logic", &solve_fsm::Configuration::transition_logic, R"(
            The combinational gates that compute the next state of the FSM.

            Defaults to an empty list, but transition logic is required for the solver to run.

            :type: list[hal_py.Gate]
        )");

        py_solve_fsm_configuration.def_readwrite("outputs", &solve_fsm::Configuration::outputs, R"(
            The outputs of the FSM, each given as a name and the nets that make up that output.

            The first net of an output provides its least significant bit, so a single-bit output is a list holding one net.
            Defaults to an empty list, in which case no outputs are computed.

            :type: list[tuple(str,list[hal_py.Net])]
        )");

        py_solve_fsm_configuration.def_readwrite("initial_state", &solve_fsm::Configuration::initial_state, R"(
            The initial value of each flip-flop of the state register.

            Only states reachable from the resulting initial state are explored.
            Defaults to an empty dict, in which case the FSM starts in state 0.

            :type: dict[hal_py.Gate,bool]
        )");

        py_solve_fsm_configuration.def_readwrite("timeout", &solve_fsm::Configuration::timeout, R"(
            The timeout for the underlying SMT solver in milliseconds. Defaults to 600000 ms.

            Has no effect when ``brute_force`` is set, as no SMT solver is used then.

            :type: int
        )");

        py_solve_fsm_configuration.def_readwrite("brute_force", &solve_fsm::Configuration::brute_force, R"(
            Enumerate all states instead of using an SMT solver. Defaults to ``False``.

            Brute forcing needs no external solver and is faster for small state registers, but its runtime doubles with every additional flip-flop.
            Both approaches produce the same state transition graph.

            :type: bool
        )");

        py_solve_fsm_configuration.def("with_state_register", &solve_fsm::Configuration::with_state_register, py::arg("state_register"), R"(
            Set the flip-flops that make up the state register of the FSM.

            :param list[hal_py.Gate] state_register: The flip-flops of the state register, least significant bit first.
            :returns: The updated FSM solver configuration.
            :rtype: solve_fsm.Configuration
        )");

        py_solve_fsm_configuration.def("with_transition_logic", &solve_fsm::Configuration::with_transition_logic, py::arg("transition_logic"), R"(
            Set the combinational gates that compute the next state of the FSM.

            :param list[hal_py.Gate] transition_logic: The gates of the transition logic.
            :returns: The updated FSM solver configuration.
            :rtype: solve_fsm.Configuration
        )");

        py_solve_fsm_configuration.def("with_outputs", &solve_fsm::Configuration::with_outputs, py::arg("outputs"), R"(
            Set the outputs of the FSM that the solver should evaluate in each state.

            :param list[tuple(str,list[hal_py.Net])] outputs: The outputs, each given as a name and the nets that make up that output, least significant bit first.
            :returns: The updated FSM solver configuration.
            :rtype: solve_fsm.Configuration
        )");

        py_solve_fsm_configuration.def("with_initial_state", &solve_fsm::Configuration::with_initial_state, py::arg("initial_state"), R"(
            Set the initial value of each flip-flop of the state register.

            :param dict[hal_py.Gate,bool] initial_state: The initial value of each flip-flop of the state register.
            :returns: The updated FSM solver configuration.
            :rtype: solve_fsm.Configuration
        )");

        py_solve_fsm_configuration.def("with_timeout", &solve_fsm::Configuration::with_timeout, py::arg("timeout"), R"(
            Set the timeout for the underlying SMT solver.

            :param int timeout: The timeout in milliseconds.
            :returns: The updated FSM solver configuration.
            :rtype: solve_fsm.Configuration
        )");

        py_solve_fsm_configuration.def("with_brute_force", &solve_fsm::Configuration::with_brute_force, py::arg("brute_force") = true, R"(
            Set whether to enumerate all states instead of using an SMT solver.

            :param bool brute_force: Set ``True`` to enumerate all states, ``False`` to use an SMT solver. Defaults to ``True``.
            :returns: The updated FSM solver configuration.
            :rtype: solve_fsm.Configuration
        )");

        py::class_<solve_fsm::StateTransitionGraph> py_state_transition_graph(m, "StateTransitionGraph", R"(
            The state transition graph of an FSM, i.e., the behavior that its netlist implements.

            States are encoded as integers, with the first flip-flop of the state register providing the least significant bit.
        )");

        py_state_transition_graph.def_readonly("transitions", &solve_fsm::StateTransitionGraph::transitions, R"(
            A dict from each state to its successor states, together with the condition under which the respective transition is taken.

            :type: dict[int,dict[int,hal_py.BooleanFunction]]
        )");

        py_state_transition_graph.def_readonly("outputs", &solve_fsm::StateTransitionGraph::outputs, R"(
            A dict from each state to the value of every output of the FSM in that state.

            The outputs of a state are given in the order in which they were configured.
            An output of a Moore FSM only depends on the state, so its Boolean function is constant.
            An output of a Mealy FSM may also depend on the inputs of the FSM, in which case its Boolean function still contains the input variables.

            Empty unless outputs were configured.

            :type: dict[int,list[tuple(str,hal_py.BooleanFunction)]]
        )");

        py_state_transition_graph.def_readonly("netlist", &solve_fsm::StateTransitionGraph::netlist, R"(
            The netlist that implements the FSM.

            :type: hal_py.Netlist
        )");

        py_state_transition_graph.def_readonly("state_register", &solve_fsm::StateTransitionGraph::state_register, R"(
            The flip-flops that make up the state register, in the order that determines the encoding of a state.

            The first flip-flop provides the least significant bit, so this is what maps a state back to the netlist.

            :type: list[hal_py.Gate]
        )");

        py_state_transition_graph.def_readonly("output_nets", &solve_fsm::StateTransitionGraph::output_nets, R"(
            The outputs of the FSM, each given as a name and the nets that make up that output.

            The first net of an output provides its least significant bit. Empty unless outputs were configured.

            :type: list[tuple(str,list[hal_py.Net])]
        )");

        py_state_transition_graph.def("get_state_size", &solve_fsm::StateTransitionGraph::get_state_size, R"(
            Get the number of flip-flops that make up the state register, i.e., the bit-size of a state.

            :returns: The bit-size of a state.
            :rtype: int
        )");

        py_state_transition_graph.def(
            "generate_dot_graph",
            [](const solve_fsm::StateTransitionGraph& self, const std::filesystem::path& graph_path, const u32 max_condition_length, const u32 base) -> std::optional<std::string> {
                auto res = self.generate_dot_graph(graph_path, max_condition_length, base);
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
            py::arg("graph_path")           = "",
            py::arg("max_condition_length") = 128,
            py::arg("base")                 = 10,
            R"(
            Render the state transition graph in the DOT format.

            Each state becomes a node labeled with its value and, if outputs were computed, with the value of every output in that state.
            Each transition becomes an edge labeled with its condition.
            If the ``dot_viewer`` plugin is available, the written graph is additionally offered to it for display.

            :param pathlib.Path graph_path: The file path at which to store the graph. No file is written if the path is left empty. Defaults to an empty path.
            :param int max_condition_length: The maximum number of characters printed for a Boolean function. Defaults to 128.
            :param int base: The base in which state and output values are printed, either 2 or 10. Defaults to 10.
            :returns: The graph in the DOT format on success, ``None`` otherwise.
            :rtype: str or None
        )");

        py_state_transition_graph.def(
            "to_string",
            [](const solve_fsm::StateTransitionGraph& self, const u32 base) -> std::optional<std::string> {
                auto res = self.to_string(base);
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
            py::arg("base") = 10,
            R"(
            Render the state transition graph as human-readable text, without truncating anything.

            Starts with a legend that maps each bit of the state to the flip-flop holding it, each output to the nets that make it up, and every net variable appearing in a Boolean function to the net it stands for.
            The legend is followed by one block per state holding its outputs and all of its outgoing transitions together with the full condition of each.

            :param int base: The base in which state and output values are printed, either 2 or 10. Defaults to 10.
            :returns: The state transition graph as text on success, ``None`` otherwise.
            :rtype: str or None
        )");

        py_state_transition_graph.def(
            "write_txt",
            [](const solve_fsm::StateTransitionGraph& self, const std::filesystem::path& file_path, const u32 base) -> bool {
                auto res = self.write_txt(file_path, base);
                if (res.is_ok())
                {
                    return true;
                }
                log_error("python_context", "{}", res.get_error().get());
                return false;
            },
            py::arg("file_path"),
            py::arg("base") = 10,
            R"(
            Write the state transition graph to a text file, without truncating anything.

            :param pathlib.Path file_path: The file path at which to store the text representation.
            :param int base: The base in which state and output values are printed, either 2 or 10. Defaults to 10.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        m.def(
            "solve_fsm",
            [](const solve_fsm::Configuration& config) -> std::optional<solve_fsm::StateTransitionGraph> {
                auto res = solve_fsm::solve_fsm(config);
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
            py::arg("config"),
            R"(
            Recover the state transition graph of an FSM from the netlist that implements it.

            Explores the states that are reachable from the initial state and determines, for each of them, which successor states it can reach and under which condition.
            If outputs are configured, the value of each output in each state is computed as well.

            No file is written. Use ``StateTransitionGraph.generate_dot_graph`` on the result to render the graph.

            :param solve_fsm.Configuration config: The configuration of the FSM solver run.
            :returns: The state transition graph of the FSM on success, ``None`` otherwise.
            :rtype: solve_fsm.StateTransitionGraph or None
        )");


#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
