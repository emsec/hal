#include "hal_core/python_bindings/python_bindings.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "z3_utils/netlist_comparison.h"
#include "z3_utils/plugin_z3_utils.h"
#include "z3_utils/simplification.h"
#include "z3_utils/subgraph_function_generation.h"
#include "z3_utils/subgraph_function_generation_parallelized.h"
#include "z3_utils/z3_utils.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(z3_utils, m)
    {
        m.doc() = "hal Z3UtilsPlugin python bindings";
#else
    PYBIND11_PLUGIN(z3_utils)
    {
        py::module m("z3_utils", "hal Z3UtilsPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<Z3UtilsPlugin, RawPtrWrapper<Z3UtilsPlugin>, BasePluginInterface> py_z3_utils(m, "Z3UtilsPlugin");
        py_z3_utils.def_property_readonly("name", &Z3UtilsPlugin::get_name);
        py_z3_utils.def("get_name", &Z3UtilsPlugin::get_name);
        py_z3_utils.def_property_readonly("version", &Z3UtilsPlugin::get_version);
        py_z3_utils.def("get_version", &Z3UtilsPlugin::get_version);

        m.def(
            "get_subgraph_functions_test",
            [](const std::vector<Net*> subgraph_outputs) -> std::optional<std::vector<hal::BooleanFunction>> {
                z3::context ctx;

                const auto res = z3_utils::get_subgraph_z3_functions(GateTypeProperty::combinational, subgraph_outputs, ctx);
                if (res.is_error())
                {
                    log_error("z3_utils", "{}", res.get_error().get());
                    return std::nullopt;
                }

                std::vector<BooleanFunction> bfs;
                for (const auto& z : res.get())
                {
                    bfs.push_back(z3_utils::to_bf(z).get());
                }

                return bfs;
            },
            py::arg("subgraph_outputs"),
            R"(TEST)");

        m.def(
            "get_subgraph_functions_slow_test",
            [](const std::vector<Gate*> subgraph_gates, const std::vector<Net*> subgraph_outputs) -> std::optional<std::vector<hal::BooleanFunction>> {
                z3::context ctx;

                const auto res = z3_utils::get_subgraph_z3_functions(subgraph_gates, subgraph_outputs, ctx);
                if (res.is_error())
                {
                    log_error("z3_utils", "{}", res.get_error().get());
                    return std::nullopt;
                }

                std::vector<BooleanFunction> bfs;
                for (const auto& z : res.get())
                {
                    bfs.push_back(z3_utils::to_bf(z).get());
                }

                return bfs;
            },
            py::arg("subgraph_gates"),
            py::arg("subgraph_outputs"),
            R"(TEST)");

        m.def(
            "get_subgraph_z3_functions_parallelized_test",
            [](const std::vector<Net*> subgraph_outputs, const u32 num_threads) -> std::optional<std::vector<hal::BooleanFunction>> {
                z3::context ctx;

                const auto res = z3_utils::get_subgraph_z3_functions_parallelized(GateTypeProperty::combinational, subgraph_outputs, ctx, num_threads);
                if (res.is_error())
                {
                    log_error("z3_utils", "{}", res.get_error().get());
                    return std::nullopt;
                }

                std::vector<BooleanFunction> bfs;
                for (const auto& z : res.get())
                {
                    bfs.push_back(z3_utils::to_bf(z).get());
                }

                return bfs;
            },
            py::arg("subgraph_outputs"),
            py::arg("num_threads"),
            R"(TEST)");

        m.def(
            "get_subgraph_function",
            [](const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) -> std::optional<hal::BooleanFunction> {
                z3::context ctx;

                const auto res = z3_utils::get_subgraph_z3_function(subgraph_gates, subgraph_output, ctx);
                if (res.is_error())
                {
                    log_error("z3_utils", "{}", res.get_error().get());
                    return std::nullopt;
                }

                BooleanFunction bf = z3_utils::to_bf(res.get()).get();

                return bf;
            },
            py::arg("subgraph_gates"),
            py::arg("subgraph_output"),
            R"(
            Compare two nets from two different netlist. 
            This is done on a functional level by buidling the subgraph function of each net considering all combinational gates of the netlist.
            In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.

            :param list[hal_py.Gate] subgraph_gates: List containing the gates of the subgraph. 
            :param hal_py.Net subgraph_output: The output net of the subgraph that whose function should be generated 
            :returns: The Boolean function implemented by the subgraph on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        m.def(
            "compare_nets",
            [](const Netlist* netlist_a, const Netlist* netlist_b, const Net* net_a, const Net* net_b, const bool fail_on_unknown = true, const u32 solver_timeout = 10) -> std::optional<bool> {
                auto res = z3_utils::compare_nets(netlist_a, netlist_b, net_a, net_b, fail_on_unknown, solver_timeout);
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
            py::arg("netlist_a"),
            py::arg("netlist_b"),
            py::arg("net_a"),
            py::arg("net_b"),
            py::arg("fail_on_unknown") = true,
            py::arg("solver_timeout")  = 10,
            R"(
            Compare two nets from two different netlist. 
            This is done on a functional level by buidling the subgraph function of each net considering all combinational gates of the netlist.
            In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.

            :param hal_py.Netlist netlist_a: First netlist. 
            :param hal_py.Netlist netlist_b: Second netlist. 
            :param hal_py.Net net_a: First net. 
            :param hal_py.Net net_b: Second net. 
            :param bool fail_on_unknown: Determines whether the function returns false or true incase the SAT solver returns unknown.
            :param int solver_timeout; The timeout for each SAT solver query in seconds. 
            :returns: A Boolean indicating whether the two nets are functionally equivalent on success, None otherwise.
            :rtype: bool or None
        )");

        m.def(
            "compare_nets",
            [](const Netlist* netlist_a, const Netlist* netlist_b, const std::vector<std::pair<Net*, Net*>>& nets, const bool fail_on_unknown = true, const u32 solver_timeout = 10)
                -> std::optional<bool> {
                auto res = z3_utils::compare_nets(netlist_a, netlist_b, nets, fail_on_unknown, solver_timeout);
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
            py::arg("netlist_a"),
            py::arg("netlist_b"),
            py::arg("nets"),
            py::arg("fail_on_unknown") = true,
            py::arg("solver_timeout")  = 10,
            R"(
            Compare two nets from two different netlist. 
            This is done on a functional level by buidling the subgraph function of each net considering all combinational gates of the netlist.
            In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.

            :param hal_py.Netlist netlist_a: First netlist. 
            :param hal_py.Netlist netlist_b: Second netlist. 
            :param list[tuple(hal_py.Net, hal_py.Net)] nets : The pairs of nets to compare against each other.
            :param bool fail_on_unknown: Determines whether the function returns false or true incase the SAT solver returns unknown.
            :param int solver_timeout; The timeout for each SAT solver query in seconds. 
            :returns: A Boolean indicating whether the two nets are functionally equivalent on success, None otherwise.
            :rtype: bool or None
        )");

        m.def(
            "compare_netlists",
            [](const Netlist* netlist_a, const Netlist* netlist_b, const bool fail_on_unknown = true, const u32 solver_timeout = 10) -> std::optional<bool> {
                auto res = z3_utils::compare_netlists(netlist_a, netlist_b, fail_on_unknown, solver_timeout);
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
            py::arg("netlist_a"),
            py::arg("netlist_b"),
            py::arg("fail_on_unknown") = true,
            py::arg("solver_timeout")  = 10,
            R"(
            Compares two netlist by finding a corresponding partner for each sequential gate in the netlist and checking whether they are identical.
            This is done on a functional level by buidling the subgraph function of all their input nets considering all combinational gates of the netlist.
            In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.

            :param hal_py.Netlist netlist_a: First netlist to compare. 
            :param hal_py.Netlist netlist_b: Second netlist to compare. 
            :param bool fail_on_unknown: Determines whether the function returns false or true incase the SAT solver returns unknown.
            :param int solver_timeout; The timeout for each SAT solver query in seconds. 
            :returns: A Boolean indicating whether the two netlists are functionally equivalent on success, None otherwise.
            :rtype: bool or None
        )");

        m.def(
            "simplify",
            [](const BooleanFunction& bf) -> std::optional<BooleanFunction> {
                auto ctx  = z3::context();
                auto expr = z3_utils::from_bf(bf, ctx);

                auto res = z3_utils::simplify_local(expr);
                if (res.is_error())
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }

                auto bf_s = z3_utils::to_bf(res.get());
                if (bf_s.is_ok())
                {
                    return bf_s.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("bf"),
            R"(
            Simplifies a Boolean function using the Z3 solver.
            This is done by using the Z3 solver to simplify the function and then converting it back to a Boolean function.
            :param hal_py.BooleanFunction bf: The Boolean function to simplify.
            :returns: The simplified Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
