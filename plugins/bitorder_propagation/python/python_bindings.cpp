#include "hal_core/python_bindings/python_bindings.h"

#include "bitorder_propagation/plugin_bitorder_propagation.h"
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
    PYBIND11_MODULE(bitorder_propagation, m)
    {
        m.doc() = "hal BitorderPropagationPlugin python bindings";
#else
    PYBIND11_PLUGIN(bitorder_propagation)
    {
        py::module m("bitorder_propagation", "hal BitorderPropagationPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<BitorderPropagationPlugin, RawPtrWrapper<BitorderPropagationPlugin>, BasePluginInterface>(m, "BitorderPropagationPlugin")
            .def_property_readonly("name", &BitorderPropagationPlugin::get_name)
            .def("get_name", &BitorderPropagationPlugin::get_name)
            .def_property_readonly("version", &BitorderPropagationPlugin::get_version)
            .def("get_version", &BitorderPropagationPlugin::get_version)
            .def_static(
                "propagate_bitorder",
                [](Netlist* nl, const std::pair<u32, std::string>& src, const std::pair<u32, std::string>& dst) -> std::optional<bool> {
                    const auto res = BitorderPropagationPlugin::propagate_bitorder(nl, src, dst);
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
                py::arg("src"),
                py::arg("dst"),
                R"(
                This funtion tries to propagate the bit order of the src pin group to the dst pin group

                :param hal_py.netlist nl: The netlist.
                :param tuple src: A pair of module id and pin group name representing the source.
                :param tuple dst: A pair of module id and pin group name representing the destination.
                :returns: A bool when there were new bitorders found.
                :rtype: bool
            )")
            .def_static(
                "propagate_bitorder",
                [](Netlist* nl, const std::vector<std::pair<u32, std::string>>& src, const std::vector<std::pair<u32, std::string>>& dst) -> std::optional<bool> {
                    const auto res = BitorderPropagationPlugin::propagate_bitorder(nl, src, dst);
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
                py::arg("src"),
                py::arg("dst"),
                R"(
                This funtion tries to propagate the bit order of the src pin groups to the dst pin groups

                :param hal_py.netlist nl: The netlist.
                :param list src: A list of pairs of module id and pin group name representing the source.
                :param list dst: A list of pairs of module id and pin group name representing the destination.
                :returns: A bool when there were new bitorders found.
                :rtype: bool
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
