#include "hal_core/python_bindings/python_bindings.h"

#include "plugin_z3_utils.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "z3_utils.h"

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
        py_z3_utils.def("get_subgraph_function", &Z3UtilsPlugin::get_subgraph_function_py);

        py_z3_utils.def_static(
            "compare_netlists",
            [](const Netlist* netlist_a, const Netlist* netlist_b) -> std::optional<bool> {
                auto res = z3_utils::compare_netlists(netlist_a, netlist_b);
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
            R"(
            Compares two netlist by finding a corresponding partner for each sequential gate in the netlist and checking whether they are identical.
            This is done on a functional level by buidling the subgraph function of all their input nets considering all combinational gates of the netlist.
            In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.

            :param hal_py.Netlist netlist_a: First netlist to compare. 
            :param hal_py.Netlist netlist_b: Second netlist to compare. 
            :returns: A Boolean indicating whether the two netlists are functionally equivalent on success, None otherwise.
            :rtype: bool or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
