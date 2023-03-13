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
                "propagate_module_pingroup_bitorder",
                [](const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& known_bitorders,
                   const std::set<std::pair<Module*, PinGroup<ModulePin>*>>& unknown_bitorders,
                   const bool strict_consens_finding = false) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                    const auto res = BitorderPropagationPlugin::propagate_module_pingroup_bitorder(known_bitorders, unknown_bitorders, strict_consens_finding);
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
                py::arg("known_bitorders"),
                py::arg("unknown_bitorders"),
                py::arg("strict_consens_finding") = false,
                R"(
                Propagates known bit order information to module pin groups with unknown bit order.
                Afterwards the algorithm tries to reconstruct valid bit orders from the propagated information.
         
                :param dict known_bitorders: The known indices for the nets belonging to module pin groups. 
                :param unknown_bitorders set : The module pin groups with yet unknown bit order.
                :param strict_consens_finding bool: When set to true this option only allows for complete and continous bitorders, while false would allow for bit orders to be formed that are either not complete or not continous.
                :returns: A mapping of all the known bit orders consisting of the new and already known.
                :rtype: dict()
            )")
            .def_static(
                "reorder_module_pin_groups",
                [](const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& ordered_module_pin_groups) -> bool {
                    const auto res = BitorderPropagationPlugin::reorder_module_pin_groups(ordered_module_pin_groups);
                    if (res.is_ok())
                    {
                        return true;
                    }
                    else
                    {
                        log_error("python_context", "{}", res.get_error().get());
                        return false;
                    }
                },
                py::arg("ordered_module_pin_groups"),
                R"(
                This funtion tries to propagate the bit order of the src pin groups to the dst pin groups

                :param dict ordered_module_pin_groups: A mapping from all the modules and pin groups with known bit order information to the knonw bit order information mapping every net to its corresponding index.
                :returns:  true in case of sucess, false otherwise
                :rtype: bool
            )")
            .def_static(
                "propagate_bitorder",
                [](Netlist* nl,
                   const std::pair<u32, std::string>& src,
                   const std::pair<u32, std::string>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
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
                Propagates known bit order information to module pin groups with unknown bit order.
                Afterwards the algorithm tries to reconstruct valid bit orders from the propagated information.
                The valid bit orders are then annotated to the module pin groups.

                :param hal_py.netlist nl: The netlist.
                :param tuple src: A pair of module id and pin group name representing the source.
                :param tuple dst: A pair of module id and pin group name representing the destination.
                :returns: All wellformed bitorders
                :rtype: dict()
            )")
            .def_static(
                "propagate_bitorder",
                [](const std::pair<Module*, PinGroup<ModulePin>*>& src,
                   const std::pair<Module*, PinGroup<ModulePin>*>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                    const auto res = BitorderPropagationPlugin::propagate_bitorder(src, dst);
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
                py::arg("src"),
                py::arg("dst"),
                R"(
                Propagates known bit order information to module pin groups with unknown bit order.
                Afterwards the algorithm tries to reconstruct valid bit orders from the propagated information.
                The valid bit orders are then annotated to the module pin groups.

                :param hal_py.netlist nl: The netlist.
                :param tuple src: A pair of module and pin group representing the source.
                :param tuple dst: A pair of module and pin group representing the destination.
                :returns: All wellformed bitorders
                :rtype: dict()
            )")
            .def_static(
                "propagate_bitorder",
                [](Netlist* nl,
                   const std::vector<std::pair<u32, std::string>>& src,
                   const std::vector<std::pair<u32, std::string>>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
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
                Propagates known bit order information to module pin groups with unknown bit order.
                Afterwards the algorithm tries to reconstruct valid bit orders from the propagated information.
                The valid bit orders are then annotated to the module pin groups.

                :param hal_py.netlist nl: The netlist.
                :param list src: A list of pairs of module id and pin group name representing the sources.
                :param list dst: A list of pairs of module id and pin group name representing the destinations.
                :returns: :returns: All wellformed bitorders
                :rtype: dict()
            )")
            .def_static(
                "propagate_bitorder",
                [](const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& src,
                   const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                    const auto res = BitorderPropagationPlugin::propagate_bitorder(src, dst);
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
                py::arg("src"),
                py::arg("dst"),
                R"(
                Propagates known bit order information to module pin groups with unknown bit order.
                Afterwards the algorithm tries to reconstruct valid bit orders from the propagated information.
                The valid bit orders are then annotated to the module pin groups.

                :param hal_py.netlist nl: The netlist.
                :param list src: A list of pairs of modules and pin groups representing the sources.
                :param list dst: A list of pairs of modules and pin groups representing the destinations.
                :returns: :returns: All wellformed bitorders
                :rtype: dict()
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
