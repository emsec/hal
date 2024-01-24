#include "hal_core/python_bindings/python_bindings.h"

#include "bitwuzla_utils/bitwuzla_utils.h"
#include "bitwuzla_utils/plugin_bitwuzla_utils.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/result.h"
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
    PYBIND11_MODULE(bitwuzla_utils, m)
    {
        m.doc() = "hal BitwuzlaUtilsPlugin python bindings";
#else
    PYBIND11_PLUGIN(bitwuzla_utils)
    {
        py::module m("bitwuzla_utils", "hal BitwuzlaUtilsPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<BitwuzlaUtilsPlugin, RawPtrWrapper<BitwuzlaUtilsPlugin>, BasePluginInterface> py_bitwuzla_utils(m, "BitwuzlaUtilsPlugin");
        py_bitwuzla_utils.def_property_readonly("name", &BitwuzlaUtilsPlugin::get_name);
        py_bitwuzla_utils.def("get_name", &BitwuzlaUtilsPlugin::get_name);
        py_bitwuzla_utils.def_property_readonly("version", &BitwuzlaUtilsPlugin::get_version);
        py_bitwuzla_utils.def("get_version", &BitwuzlaUtilsPlugin::get_version);

        py_bitwuzla_utils.def_static(
            "test_on_net",
            [](const Netlist* nl, const Net* n) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });
                const auto bf_res     = bitwuzla_utils::get_subgraph_bitwuzla_function(comb_gates, n);
                if (bf_res.is_error())
                {
                    log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                    return;
                }

                const auto bf = bf_res.get();

                std::cout << "BF: " << bf << std::endl;
            },

            R"(

        )");

        py_bitwuzla_utils.def_static(
            "test_on_netlist",
            [](const Netlist* nl) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

                for (const auto& n : nl->get_nets())
                {
                    const auto bf_res = bitwuzla_utils::get_subgraph_bitwuzla_function(comb_gates, n);
                    if (bf_res.is_error())
                    {
                        log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                        return;
                    }

                    const auto bf = bf_res.get();

                    std::cout << "BF: " << bf << std::endl;
                }
            },

            R"(

        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
