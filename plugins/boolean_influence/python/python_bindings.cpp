#include "boolean_influence/boolean_influence.h"
#include "boolean_influence/plugin_boolean_influence.h"
#include "hal_core/python_bindings/python_bindings.h"
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
    PYBIND11_MODULE(boolean_influence, m)
    {
        m.doc() = "Set of functions to determine the influence of variables of a Boolean function on its output.";
#else
    PYBIND11_PLUGIN(boolean_influence)
    {
        py::module m("boolean_influence", "Set of functions to determine the influence of variables of a Boolean function on its output.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<BooleanInfluencePlugin, RawPtrWrapper<BooleanInfluencePlugin>, BasePluginInterface> py_boolean_influence_plugin(m, "BooleanInfluencePlugin");

        py_boolean_influence_plugin.def_property_readonly("name", &BooleanInfluencePlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_boolean_influence_plugin.def("get_name", &BooleanInfluencePlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_boolean_influence_plugin.def_property_readonly("version", &BooleanInfluencePlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_boolean_influence_plugin.def("get_version", &BooleanInfluencePlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_boolean_influence_plugin.def_property_readonly("description", &BooleanInfluencePlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_boolean_influence_plugin.def("get_description", &BooleanInfluencePlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_boolean_influence_plugin.def_property_readonly("dependencies", &BooleanInfluencePlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )");

        py_boolean_influence_plugin.def("get_dependencies", &BooleanInfluencePlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )");

        m.def(
            "get_boolean_influence",
            [](const BooleanFunction& bf, const u32 num_evaluations = 32000) -> std::optional<std::unordered_map<std::string, double>> {
                const auto res = boolean_influence::get_boolean_influence(bf, num_evaluations);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of Boolean function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("bf"),
            py::arg("num_evaluations") = 32000,
            R"(
            Generates the Boolean influence of each input variable of a Boolean function using the internal HAL functions only
            The function is slower, but can be better used in multithreading enviroment.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :param int num_evaluations: The amount of evaluations that are performed for each input variable.
            :returns: A dict from the variables that appear in the function to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[str,float] or None
        )");

        m.def(
            "get_boolean_influence_with_hal_boolean_function_class",
            [](const BooleanFunction& bf, const u32 num_evaluations = 32000) -> std::optional<std::unordered_map<std::string, double>> {
                const auto res = boolean_influence::get_boolean_influence_with_hal_boolean_function_class(bf, num_evaluations);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of Boolean function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("bf"),
            py::arg("num_evaluations") = 32000,
            R"(
            The Boolean function gets translated to a z3::expr and afterwards efficient c code.
            The program is compiled and executed many times to measure the Boolean influence of each input variable.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :param int num_evaluations: The amount of evaluations that are performed for each input variable.
            :returns: A dict from the variables that appear in the function to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[str,float] or None
        )");

        m.def(
            "get_boolean_influence_with_z3_expr",
            [](const BooleanFunction& bf, const u32 num_evaluations = 32000) -> std::optional<std::unordered_map<std::string, double>> {
                const auto res = boolean_influence::get_boolean_influence_with_z3_expr(bf, num_evaluations);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of Boolean function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("bf"),
            py::arg("num_evaluations") = 32000,
            R"(
            Generates the Boolean influence of each input variable of a Boolean function using z3 expressions and substitutions/simplifications only.
            The function is slower, but can be better used in multithreading environment.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :param int num_evaluations: The amount of evaluations that are performed for each input variable.
            :returns: A dict from the variables that appear in the function to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[str,float] or None
        )");

        m.def(
            "get_boolean_influences_of_subcircuit",
            [](const std::vector<Gate*>& gates, const Net* start_net, const u32 num_evaluations = 32000) -> std::optional<std::map<Net*, double>> {
                const auto res = boolean_influence::get_boolean_influences_of_subcircuit(gates, start_net, num_evaluations);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of subcircuit function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gates"),
            py::arg("start_net"),
            py::arg("num_evaluations") = 32000,
            R"(
            Generates the function of the net using only the given gates.
            Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.

            :param list[hal_py.Gate] gates: The gates of the subcircuit.
            :param hal_py.Net start_net: The output net of the subcircuit at which to start the analysis.
            :param int num_evaluations: The amount of evaluations that are performed for each input variable.
            :returns: A dict from the nets that appear in the function of the start net to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[hal_py.Net,float] or None
        )");

        m.def(
            "get_boolean_influences_of_gate",
            [](const Gate* gate, const u32 num_evaluations = 32000) -> std::optional<std::map<Net*, double>> {
                const auto res = boolean_influence::get_boolean_influences_of_gate(gate, num_evaluations);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of flip-flop data fan-in:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("num_evaluations") = 32000,
            R"(
            Generates the function of the dataport net of the given flip-flop.
            Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.

            :param hal_py.Gate gate: The flip-flop which data input net is used to build the boolean function.
            :param int num_evaluations: The amount of evaluations that are performed for each input variable.
            :returns: A dict from the nets that appear in the function of the data net to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[hal_py.Net,float]
        )");

        m.def(
            "get_boolean_influence_deterministic",
            [](const BooleanFunction& bf) -> std::optional<std::unordered_map<std::string, double>> {
                const auto res = boolean_influence::get_boolean_influence_deterministic(bf);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of Boolean function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("bf"),
            R"(
            The Boolean function gets translated to a z3::expr and afterwards efficient c code.
            The program is compiled and executed exactly once for every possible input mapping to accurately determine the boolean influence of each variable.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :returns: A dict from the variables that appear in the function to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[str,float] or None
        )");

        m.def(
            "get_boolean_influences_of_subcircuit_deterministic",
            [](const std::vector<Gate*>& gates, const Net* start_net) -> std::optional<std::map<Net*, double>> {
                const auto res = boolean_influence::get_boolean_influences_of_subcircuit_deterministic(gates, start_net);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of subcircuit function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gates"),
            py::arg("start_net"),
            R"(
            Generates the function of the net using only the given gates.
            Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.

            :param list[hal_py.Gate] gates: The gates of the subcircuit.
            :param hal_py.Net start_net: The output net of the subcircuit at which to start the analysis.
            :returns: A dict from the nets that appear in the function of the start net to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[hal_py.Net,float] or None
        )");

        m.def(
            "get_boolean_influences_of_gate_deterministic",
            [](const Gate* gate) -> std::optional<std::map<Net*, double>> {
                const auto res = boolean_influence::get_boolean_influences_of_gate_deterministic(gate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of flip-flop data fan-in:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            R"(
            Generates the function of the dataport net of the given flip-flop.
            Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.

            :param hal_py.Gate gate: The flip-flop which data input net is used to build the boolean function.
            :returns: A dict from the nets that appear in the function of the data net to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[hal_py.Net,float]
        )");

        m.def(
            "get_boolean_influences_of_subcircuit_bitsliced",
            [](const std::vector<Gate*>& gates, const Net* start_net, const u32 bias) -> std::optional<std::map<Net*, double>> {
                const auto res = boolean_influence::get_boolean_influences_of_subcircuit_bitsliced(gates, start_net, bias);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of subcircuit function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gates"),
            py::arg("start_net"),
            py::arg("bias") = 0,
            R"(
            Generates the function of the net using only the given gates.
            Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.

            :param list[hal_py.Gate] gates: The gates of the subcircuit.
            :param hal_py.Net start_net: The output net of the subcircuit at which to start the analysis.
            :param int bias: A potential bias towards logical 1s instead of 0. This should help distinguish very small influences. P(1) = 1 - 2^-(bias+1). Defaults to 0
            :returns: A dict from the nets that appear in the function of the start net to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[hal_py.Net,float] or None
        )");

        m.def(
            "get_boolean_influences_of_gate_bitsliced",
            [](const Gate* gate, const u32 bias) -> std::optional<std::map<Net*, double>> {
                const auto res = boolean_influence::get_boolean_influences_of_gate_bitsliced(gate, bias);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot get Boolean influence of flip-flop data fan-in:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("gate"),
            py::arg("bias") = 0,
            R"(
            Generates the function of the dataport net of the given flip-flop.
            Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.

            :param hal_py.Gate gate: The flip-flop which data input net is used to build the boolean function.
            :param int bias: A potential bias towards logical 1s instead of 0. This should help distinguish very small influences. P(1) = 1 - 2^-(bias+1). Defaults to 0
            :returns: A dict from the nets that appear in the function of the data net to their Boolean influence on said function on success, None otherwise.
            :rtype: dict[hal_py.Net,float]
        )");

        m.def(
            "get_ff_dependency_matrix",
            [](const Netlist* nl, bool with_boolean_influence) -> std::optional<std::pair<std::map<u32, Gate*>, std::vector<std::vector<double>>>> {
                const auto res = boolean_influence::get_ff_dependency_matrix(nl, with_boolean_influence);
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
            py::arg("netlist"),
            py::arg("with_boolean_influence"),
            R"(
            Get the FF dependency matrix of a netlist, with or without boolean influences.

            :param hal_py.Netlist netlist: The netlist to extract the dependency matrix from.
            :param bool with_boolean_influence: True -- set boolean influence, False -- sets 1.0 if connection between FFs
            :returns: A pair consisting of std::map<u32, Gate*>, which includes the mapping from the original gate
            :rtype: pair(dict(int, hal_py.Gate), list[list[double]])
        )");
        ;

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
