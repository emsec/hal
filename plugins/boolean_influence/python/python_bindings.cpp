#include "hal_core/python_bindings/python_bindings.h"

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
        m.doc() = "hal BooleanInfluencePlugin python bindings";
#else
    PYBIND11_PLUGIN(boolean_influence)
    {
        py::module m("boolean_influence", "hal BooleanInfluencePlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<BooleanInfluencePlugin, RawPtrWrapper<BooleanInfluencePlugin>, BasePluginInterface>(m, "BooleanInfluencePlugin")
            .def_property_readonly("name", &BooleanInfluencePlugin::get_name)
            .def("get_name", &BooleanInfluencePlugin::get_name)
            .def_property_readonly("version", &BooleanInfluencePlugin::get_version)
            .def("get_version", &BooleanInfluencePlugin::get_version)
            .def_static(
                "get_boolean_influence",
                [](const BooleanFunction& bf, const u32 num_evaluations = 32000, const std::string& unique_identifier = "") -> std::optional<std::unordered_map<std::string, double>> {
                    const auto res = BooleanInfluencePlugin::get_boolean_influence(bf, num_evaluations, unique_identifier);
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
                py::arg("num_evaluations")   = 32000,
                py::arg("unique_identifier") = "",
                R"(
                The Boolean function gets translated to a z3::expr and afterwards efficent c code.
                The program is compiled and executed many times to meassure the Boolean influence of each input variable.

                :param hal_py.BooleanFunction bf: The Boolean function.
                :param int num_evaluations: The amount of evaluations that are performed for each input variable.
                :param str unique_identifier: A unique identifier that is applied to file names to prevent collisions during multi-threading.
                :returns: A dict from the variables that appear in the function to their Boolean influence on said function on success, None otherwise.
                :rtype: dict[str,float] or None
            )")
            .def_static(
                "get_boolean_influences_of_subcircuit",
                [](const std::vector<Gate*>& gates, const Net* start_net, const u32 num_evaluations = 32000, const std::string& unique_identifier = "") -> std::optional<std::map<Net*, double>> {
                    const auto res = BooleanInfluencePlugin::get_boolean_influences_of_subcircuit(gates, start_net, num_evaluations, unique_identifier);
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
                py::arg("num_evaluations")   = 32000,
                py::arg("unique_identifier") = "",
                R"(
                Generates the function of the net using only the given gates.
                Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evalated.

                :param list[hal_py.Gate] gates: The gates of the subcircuit.
                :param hal_py.Net start_net: The output net of the subcircuit at which to start the analysis.
                :param int num_evaluations: The amount of evaluations that are performed for each input variable.
                :param str unique_identifier: A unique identifier that is applied to file names to prevent collisions during multi-threading.
                :returns: A dict from the nets that appear in the function of the start net to their Boolean influence on said function on success, None otherwise.
                :rtype: dict[hal_py.Net,float] or None
            )")
            .def_static(
                "get_boolean_influences_of_gate",
                [](const Gate* gate, const u32 num_evaluations = 32000, const std::string& unique_identifier = "") -> std::optional<std::map<Net*, double>> {
                    const auto res = BooleanInfluencePlugin::get_boolean_influences_of_gate(gate, num_evaluations, unique_identifier);
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
                py::arg("num_evaluations")   = 32000,
                py::arg("unique_identifier") = "",
                R"(
                Generates the function of the dataport net of the given flip-flop.
                Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evalated.

                :param hal_py.Gate gate: The flip-flop which data input net is used to build the boolean function.
                :param int num_evaluations: The amount of evaluations that are performed for each input variable.
                :param str unique_identifier: A unique identifier that is applied to file names to prevent collisions during multi-threading.
                :returns: A dict from the nets that appear in the function of the data net to their Boolean influence on said function on success, None otherwise.
                :rtype: dict[hal_py.Net,float]
            )")
            .def_static(
                "get_boolean_influence_deterministic",
                [](const BooleanFunction& bf, const std::string& unique_identifier = "") -> std::optional<std::unordered_map<std::string, double>> {
                    const auto res = BooleanInfluencePlugin::get_boolean_influence_deterministic(bf, unique_identifier);
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
                py::arg("unique_identifier") = "",
                R"(
                The Boolean function gets translated to a z3::expr and afterwards efficent c code.
                The program is compiled and executed exactly once for every possible input mapping to accuratley determine the boolean influence of each variable.

                :param hal_py.BooleanFunction bf: The Boolean function.
                :param bool deterministic: Specifies whether the function is executed with random values or each possible input mapping is tested deterministically.
                :param str unique_identifier: A unique identifier that is applied to file names to prevent collisions during multi-threading.
                :returns: A dict from the variables that appear in the function to their Boolean influence on said function on success, None otherwise.
                :rtype: dict[str,float] or None
            )")
            .def_static(
                "get_boolean_influences_of_subcircuit_deterministic",
                [](const std::vector<Gate*>& gates, const Net* start_net, const std::string& unique_identifier = "") -> std::optional<std::map<Net*, double>> {
                    const auto res = BooleanInfluencePlugin::get_boolean_influences_of_subcircuit_deterministic(gates, start_net, unique_identifier);
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
                py::arg("unique_identifier") = "",
                R"(
                Generates the function of the net using only the given gates.
                Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evalated.

                :param list[hal_py.Gate] gates: The gates of the subcircuit.
                :param hal_py.Net start_net: The output net of the subcircuit at which to start the analysis.
                :param str unique_identifier: A unique identifier that is applied to file names to prevent collisions during multi-threading.
                :returns: A dict from the nets that appear in the function of the start net to their Boolean influence on said function on success, None otherwise.
                :rtype: dict[hal_py.Net,float] or None
            )")
            .def_static(
                "get_boolean_influences_of_gate_deterministic",
                [](const Gate* gate, const std::string& unique_identifier = "") -> std::optional<std::map<Net*, double>> {
                    const auto res = BooleanInfluencePlugin::get_boolean_influences_of_gate_deterministic(gate, unique_identifier);
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
                py::arg("unique_identifier") = "",
                R"(
                Generates the function of the dataport net of the given flip-flop.
                Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evalated.

                :param hal_py.Gate gate: The flip-flop which data input net is used to build the boolean function.
                :param str unique_identifier: A unique identifier that is applied to file names to prevent collisions during multi-threading.
                :returns: A dict from the nets that appear in the function of the data net to their Boolean influence on said function on success, None otherwise.
                :rtype: dict[hal_py.Net,float]
            )")
            .def_static(
                "get_ff_dependency_matrix",
                [](const Netlist* nl, bool with_boolean_influence) -> std::optional<std::pair<std::map<u32, Gate*>, std::vector<std::vector<double>>>> {
                    const auto res = BooleanInfluencePlugin::get_ff_dependency_matrix(nl, with_boolean_influence);
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
