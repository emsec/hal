#include "hal_core/python_bindings/python_bindings.h"

#include "boolean_influence/boolean_influence.h"
#include "boolean_influence/plugin_boolean_influence.h"
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
            Compute the Boolean influence of each input variable of a Boolean function.
            The influence is approximated by evaluating the function on randomly sampled input assignments.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :param int num_evaluations: The number of evaluations that are performed for each input variable.
            :returns: A dict from each variable of the function to its Boolean influence on success, ``None`` otherwise.
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
            Compute the Boolean influence of each input variable of a Boolean function using only HAL-internal functionality.
            The influence is approximated by evaluating the function on randomly sampled input assignments.
            This variant is slower than ``get_boolean_influence``, but it is better suited for use in a multi-threaded environment.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :param int num_evaluations: The number of evaluations that are performed for each input variable.
            :returns: A dict from each variable of the function to its Boolean influence on success, ``None`` otherwise.
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
            Compute the Boolean influence of each input variable of a Boolean function using only z3 substitution and simplification.
            The influence is approximated by evaluating the function on randomly sampled input assignments.
            This variant is slower than ``get_boolean_influence``, but it is better suited for use in a multi-threaded environment.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :param int num_evaluations: The number of evaluations that are performed for each input variable.
            :returns: A dict from each variable of the function to its Boolean influence on success, ``None`` otherwise.
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
            Compute the Boolean influence of each input net of a subcircuit on one of its output nets.
            The Boolean function of the start net is built from the given gates, translated into C code, and then compiled and executed for speed.
            The influence is approximated by evaluating that function on randomly sampled input assignments.

            :param list[hal_py.Gate] gates: The gates of the subcircuit.
            :param hal_py.Net start_net: The output net of the subcircuit at which to start the analysis.
            :param int num_evaluations: The number of evaluations that are performed for each input variable.
            :returns: A dict from each input net of the subcircuit to its Boolean influence on the start net on success, ``None`` otherwise.
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
            Compute the Boolean influence of each net that drives the data input of the given flip-flop.
            The Boolean function of the data input net is built, translated into C code, and then compiled and executed for speed.
            The influence is approximated by evaluating that function on randomly sampled input assignments.

            :param hal_py.Gate gate: The flip-flop whose data input net is used to build the Boolean function.
            :param int num_evaluations: The number of evaluations that are performed for each input variable.
            :returns: A dict from each net of the function to its Boolean influence on the data input net on success, ``None`` otherwise.
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
            Compute the exact Boolean influence of each input variable of a Boolean function.
            In contrast to ``get_boolean_influence``, the function is evaluated on every possible input assignment instead of a random sample.
            This is only feasible for functions of at most 16 variables.

            :param hal_py.BooleanFunction bf: The Boolean function.
            :returns: A dict from each variable of the function to its Boolean influence on success, ``None`` otherwise.
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
            Compute the exact Boolean influence of each input net of a subcircuit on one of its output nets.
            In contrast to ``get_boolean_influences_of_subcircuit``, the function is evaluated on every possible input assignment instead of a random sample.
            This is only feasible for subcircuits with at most 16 input nets.

            :param list[hal_py.Gate] gates: The gates of the subcircuit.
            :param hal_py.Net start_net: The output net of the subcircuit at which to start the analysis.
            :returns: A dict from each input net of the subcircuit to its Boolean influence on the start net on success, ``None`` otherwise.
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
            Compute the exact Boolean influence of each net that drives the data input of the given flip-flop.
            In contrast to ``get_boolean_influences_of_gate``, the function is evaluated on every possible input assignment instead of a random sample.
            This is only feasible for data input functions of at most 16 nets.

            :param hal_py.Gate gate: The flip-flop whose data input net is used to build the Boolean function.
            :returns: A dict from each net of the function to its Boolean influence on the data input net on success, ``None`` otherwise.
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
            Get the flip-flop dependency matrix of a netlist, i.e., a matrix that holds an entry for every pair of flip-flops that are connected through combinational logic.

            :param hal_py.Netlist netlist: The netlist to extract the dependency matrix from.
            :param bool with_boolean_influence: Set ``True`` to use the Boolean influence as the matrix entry, ``False`` to use ``1.0`` for every connection.
            :returns: A tuple consisting of a dict from the original gate IDs to the corresponding matrix indices and the flip-flop dependency matrix itself, ``None`` otherwise.
            :rtype: tuple(dict[int,hal_py.Gate], list[list[float]]) or None
        )");
        ;

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
