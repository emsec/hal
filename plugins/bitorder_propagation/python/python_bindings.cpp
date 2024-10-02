#include "hal_core/python_bindings/python_bindings.h"

#include "bitorder_propagation/bitorder_propagation.h"
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
        m.doc() = "Tool to automatically propagate known bit orders to module pin groups of unknown bit order.";
#else
    PYBIND11_PLUGIN(bitorder_propagation)
    {
        py::module m("bitorder_propagation", "Tool to automatically propagate known bit orders to module pin groups of unknown bit order.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<BitorderPropagationPlugin, RawPtrWrapper<BitorderPropagationPlugin>, BasePluginInterface> py_bitorder_propagation_plugin(
            m, "BitorderPropagationPlugin", R"(This class provides an interface to integrate the bit-order propagation as a plugin within the HAL framework.)");

        py_bitorder_propagation_plugin.def_property_readonly("name", &BitorderPropagationPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_bitorder_propagation_plugin.def("get_name", &BitorderPropagationPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_bitorder_propagation_plugin.def_property_readonly("version", &BitorderPropagationPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_bitorder_propagation_plugin.def("get_version", &BitorderPropagationPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_bitorder_propagation_plugin.def_property_readonly("description", &BitorderPropagationPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_bitorder_propagation_plugin.def("get_description", &BitorderPropagationPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        m.def(
            "propagate_module_pingroup_bitorder",
            [](const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& src,
               const std::set<std::pair<Module*, PinGroup<ModulePin>*>>& dst,
               const bool enforce_continuous_bitorders = true) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                const auto res = bitorder_propagation::propagate_module_pingroup_bitorder(src, dst, enforce_continuous_bitorders);
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
            py::arg("enforce_continuous_bitorders") = true,
            R"(
                    Propagate known bit-order information from the given module pin groups to module pin groups of unknown bit order.
                    The known bit-order information is taken from the map from net to index given for each pair of module and pin group in ``src``.
                    After propagation, the algorithm tries to reconstruct valid bit orders from the propagated information.
         
                    :param dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] src: The known indices for the nets belonging to the given module pin groups. 
                    :param set[tuple(hal_py.Module,hal_py.ModulePinGroup)] dst: The pairs of module ID and pin group name with unknown bit order.
                    :param bool enforce_continuous_bitorders: Set ``True`` to only allow for continuous bit orders, ``^`` to also allow bit orders that are not continuous. Defaults to ``True``.
                    :returns: A dict containing all known bit orders (including new and already known ones) on success, ``None`` otherwise.
                    :rtype: dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] or None
                )");

        m.def(
            "reorder_module_pin_groups",
            [](const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& ordered_module_pin_groups) -> bool {
                const auto res = bitorder_propagation::reorder_module_pin_groups(ordered_module_pin_groups);
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
                Reorder and rename the pins of the pin groups according to the provided bit-order information. 

                :param dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] ordered_module_pin_groups: A mapping from pairs of modules and their pin groups to known bit-order information given as a mapping from nets to their index.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        m.def(
            "propagate_bitorder",
            [](Netlist* nl, const std::pair<u32, std::string>& src, const std::pair<u32, std::string>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                const auto res = bitorder_propagation::propagate_bitorder(nl, src, dst);
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
                Propagate known bit-order information from one module pin group to another module pin group of unknown bit order.
                The known bit-order information is taken from the order of pins in the pin group of ``src``.
                After propagation, the algorithm tries to reconstruct a valid bit order from the propagated information.
                The valid bit order is then annotated to the module pin group, i.e., the pins of the respective pin group are renamed and reordered.

                :param hal_py.netlist nl: The netlist containing the module.
                :param tuple(int,str) src: The pair of module ID and pin group name with known bit order.
                :param tuple(int,str) dst: The pair of module ID and pin group name with unknown bit order.
                :returns: A dict containing all known bit orders (including new and already known ones) on success, ``None`` otherwise.
                :rtype: dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] or None
            )");

        m.def(
            "propagate_bitorder",
            [](const std::pair<Module*, PinGroup<ModulePin>*>& src,
               const std::pair<Module*, PinGroup<ModulePin>*>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                const auto res = bitorder_propagation::propagate_bitorder(src, dst);
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
                Propagate known bit-order information from one module pin group to another module pin group of unknown bit order.
                The known bit-order information is taken from the order of pins in the pin group of ``src``.
                After propagation, the algorithm tries to reconstruct a valid bit order from the propagated information.
                The valid bit order is then annotated to the module pin group, i.e., the pins of the respective pin group are renamed and reordered.

                :param tuple(hal_py.Module,hal_py.ModulePinGroup) src: The pair of module and pin group with known bit order. 
                :param tuple(hal_py.Module,hal_py.ModulePinGroup) dst: The pair of module and pin group with unknown bit order.
                :returns: A dict containing all known bit orders (including new and already known ones) on success, ``None`` otherwise.
                :rtype: dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] or None
            )");

        m.def(
            "propagate_bitorder",
            [](Netlist* nl,
               const std::vector<std::pair<u32, std::string>>& src,
               const std::vector<std::pair<u32, std::string>>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                const auto res = bitorder_propagation::propagate_bitorder(nl, src, dst);
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
                Propagate known bit-order information from the given module pin groups to module pin groups of unknown bit order.
                The known bit-order information is taken from the order of pins in the pin groups of ``src``.
                After propagation, the algorithm tries to reconstruct valid bit orders from the propagated information.
                The valid bit orders are then annotated to the module pin groups, i.e., the pins of the respective pin groups are renamed and reordered.

                :param hal_py.netlist nl: The netlist containing the modules.
                :param list[tuple(int,str)] src: The pairs of module ID and pin group name with known bit order.
                :param list[tuple(int,str)] dst: The pairs of module ID and pin group name with unknown bit order.
                :returns: A dict containing all known bit orders (including new and already known ones) on success, ``None`` otherwise.
                :rtype: dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] or None
            )");

        m.def(
            "propagate_bitorder",
            [](const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& src,
               const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& dst) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> {
                const auto res = bitorder_propagation::propagate_bitorder(src, dst);
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
                Propagate known bit-order information from the given module pin groups to module pin groups of unknown bit order.
                The known bit-order information is taken from the order of pins in the pin groups of ``src``.
                After propagation, the algorithm tries to reconstruct valid bit orders from the propagated information.
                The valid bit orders are then annotated to the module pin groups, i.e., the pins of the respective pin groups are renamed and reordered.

                :param list[tuple(hal_py.Module,hal_py.ModulePinGroup)] src: The pairs of module and pin group with known bit order.
                :param list[tuple(hal_py.Module,hal_py.ModulePinGroup)] dst: The pairs of module and pin group with unknown bit order.
                :returns: A dict containing all known bit orders (including new and already known ones) on success, ``None`` otherwise.
                :rtype: dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] or None
            )");

        m.def(
            "export_bitorder_propagation_information",
            [](const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& src,
               const std::set<std::pair<Module*, PinGroup<ModulePin>*>>& dst,
               const std::string& export_filepath) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, u32>> {
                const auto res = bitorder_propagation::export_bitorder_propagation_information(src, dst, export_filepath);
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
            py::arg("export_filepath"),
            R"(
                    Export collected bitorder information like word composition, known bitorder and connectivity in .json format to solve with external tools.
         
                    :param dict[tuple(hal_py.Module,hal_py.ModulePinGroup),dict[hal_py.Net,int]] src: The known indices for the nets belonging to the given module pin groups. 
                    :param set[tuple(hal_py.Module,hal_py.ModulePinGroup)] dst: The pairs of module ID and pin group name with unknown bit order.
                    :param str export_filepath: The filepath where the .json file should be written to.
                    :returns: The mapping from each mdoule/pingroup pair to its index on success, ``None`` otherwise.
                    :rtype: dict[tuple(hal_py.Module, hal_py.ModulePinGroup), int] or None
                )");

        m.def(
            "export_bitorder_propagation_information",
            [](const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& src,
               const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& dst,
               const std::string& export_filepath) -> std::optional<std::map<std::pair<Module*, PinGroup<ModulePin>*>, u32>> {
                const auto res = bitorder_propagation::export_bitorder_propagation_information(src, dst, export_filepath);
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
            py::arg("export_filepath"),
            R"(
                    Export collected bitorder information like word composition, known bitorder and connectivity in .json format to solve with external tools.
         
                   :param tuple(hal_py.Module,hal_py.ModulePinGroup) src: The pair of module and pin group with known bit order. 
                    :param tuple(hal_py.Module,hal_py.ModulePinGroup) dst: The pair of module and pin group with unknown bit order.
                    :param str export_filepath: The filepath where the .json file should be written to.
                    :returns: The mapping from each mdoule/pingroup pair to its index on success, ``None`` otherwise.
                    :rtype: dict[tuple(hal_py.Module, hal_py.ModulePinGroup), int] or None
                )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
