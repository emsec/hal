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

        py::class_<bitorder_propagation::BitOrder> py_bit_order(m, "BitOrder", R"(
            The bit order of a single module pin group, i.e., which net of the pin group carries which bit.
        )");

        py_bit_order.def(py::init<Module*, PinGroup<ModulePin>*, std::vector<std::pair<Net*, u32>>>(), py::arg("module"), py::arg("pin_group"), py::arg("order"), R"(
            Construct a bit order for a module pin group.

            :param hal_py.Module module: The module the pin group belongs to.
            :param hal_py.ModulePinGroup pin_group: The pin group.
            :param list[tuple(hal_py.Net,int)] order: The index of each net of the pin group.
        )");

        // The getter is built as a cpp_function here rather than handed over directly, because
        // def_property_readonly builds it itself without passing on any of the attributes that follow,
        // so a call policy given to the property never reaches the function that does the call.
        py_bit_order.def_property_readonly("module", py::cpp_function(&bitorder_propagation::BitOrder::get_module, py::is_method(py_bit_order), borrowed()), R"(
            The module that the pin group belongs to.

            :type: hal_py.Module
        )");

        py_bit_order.def_property_readonly("pin_group", py::cpp_function(&bitorder_propagation::BitOrder::get_pin_group, py::is_method(py_bit_order), borrowed()), R"(
            The pin group whose bit order this is.

            :type: hal_py.ModulePinGroup
        )");

        py_bit_order.def_property_readonly("order", py::cpp_function(&bitorder_propagation::BitOrder::get_order, py::is_method(py_bit_order), borrowed()), R"(
            The index of every net, ordered by index.

            :type: list[tuple(hal_py.Net,int)]
        )");

        py_bit_order.def("get_index", &bitorder_propagation::BitOrder::get_index, py::arg("net"), R"(
            Get the index of the given net.

            :param hal_py.Net net: The net.
            :returns: The index of the net, ``None`` if the net is not part of this bit order.
            :rtype: int or None
        )");

        py_bit_order.def("get_net_at", &bitorder_propagation::BitOrder::get_net_at, py::arg("index"), borrowed(), R"(
            Get the net at the given index.

            :param int index: The index.
            :returns: The net at the index, ``None`` if no net carries that index.
            :rtype: hal_py.Net or None
        )");

        py_bit_order.def_property_readonly("size", &bitorder_propagation::BitOrder::get_size, R"(
            The number of nets that the bit order covers.

            :type: int
        )");

        py_bit_order.def("is_continuous", &bitorder_propagation::BitOrder::is_continuous, R"(
            Check whether the indices run from 0 without leaving a gap.

            :returns: ``True`` if the order is continuous, ``False`` otherwise.
            :rtype: bool
        )");

        py_bit_order.def(py::self == py::self);
        py_bit_order.def(py::self != py::self);

        py::class_<bitorder_propagation::BitOrderResult> py_bit_order_result(m, "BitOrderResult", R"(
            The bit orders that are known, which is what a propagation reports: the ones it was given as well as the ones it worked out.

            Iterating over a result walks the bit orders by module ID and then by pin group ID, so it does not depend on where the modules and pin groups happen to be allocated.
        )");

        py_bit_order_result.def(py::init<>(), R"(Construct a result that holds no bit order.)");

        py_bit_order_result.def(py::init<std::vector<bitorder_propagation::BitOrder>>(), py::arg("bit_orders"), R"(
            Construct a result from the given bit orders.

            :param list[bitorder_propagation.BitOrder] bit_orders: The bit orders.
        )");

        py_bit_order_result.def("add", &bitorder_propagation::BitOrderResult::add, py::arg("bit_order"), R"(
            Add a bit order, replacing one that is already known for the same pin group.

            :param bitorder_propagation.BitOrder bit_order: The bit order.
        )");

        py_bit_order_result.def_property_readonly(
            "bit_orders", py::cpp_function(&bitorder_propagation::BitOrderResult::get_bit_orders, py::is_method(py_bit_order_result), borrowed()), R"(
            Every bit order, ordered by module ID and pin group ID.

            :type: list[bitorder_propagation.BitOrder]
        )");

        py_bit_order_result.def("get", &bitorder_propagation::BitOrderResult::get, py::arg("module"), py::arg("pin_group"), borrowed(), R"(
            Get the bit order of the given pin group.

            :param hal_py.Module module: The module the pin group belongs to.
            :param hal_py.ModulePinGroup pin_group: The pin group.
            :returns: The bit order, ``None`` if the pin group has no known bit order.
            :rtype: bitorder_propagation.BitOrder or None
        )");

        py_bit_order_result.def("contains", &bitorder_propagation::BitOrderResult::contains, py::arg("module"), py::arg("pin_group"), R"(
            Check whether the bit order of the given pin group is known.

            :param hal_py.Module module: The module the pin group belongs to.
            :param hal_py.ModulePinGroup pin_group: The pin group.
            :returns: ``True`` if the bit order is known, ``False`` otherwise.
            :rtype: bool
        )");

        py_bit_order_result.def("__len__", &bitorder_propagation::BitOrderResult::get_size);

        py_bit_order_result.def(
            "__iter__", [](const bitorder_propagation::BitOrderResult& self) { return py::make_iterator(self.begin(), self.end()); }, py::keep_alive<0, 1>());

        m.def(
            "propagate_module_pingroup_bitorder",
            [](const bitorder_propagation::BitOrderResult& src,
               const std::set<std::pair<Module*, PinGroup<ModulePin>*>>& dst,
               const bool enforce_continuous_bitorders = true) -> std::optional<bitorder_propagation::BitOrderResult> {
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
         
                    :param bitorder_propagation.BitOrderResult src: The bit orders that are already known. 
                    :param set[tuple(hal_py.Module,hal_py.ModulePinGroup)] dst: The pairs of module ID and pin group name with unknown bit order.
                    :param bool enforce_continuous_bitorders: Set ``True`` to only allow for continuous bit orders, ``^`` to also allow bit orders that are not continuous. Defaults to ``True``.
                    :returns: All known bit orders, the new ones as well as the ones already known, on success, ``None`` otherwise.
                    :rtype: bitorder_propagation.BitOrderResult or None
                )");

        m.def(
            "reorder_module_pin_groups",
            [](const bitorder_propagation::BitOrderResult& ordered_module_pin_groups) -> bool {
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

                :param bitorder_propagation.BitOrderResult ordered_module_pin_groups: The bit orders to apply.
                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

        m.def(
            "propagate_bitorder",
            [](Netlist* nl, const std::pair<u32, std::string>& src, const std::pair<u32, std::string>& dst) -> std::optional<bitorder_propagation::BitOrderResult> {
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
               const std::pair<Module*, PinGroup<ModulePin>*>& dst) -> std::optional<bitorder_propagation::BitOrderResult> {
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
               const std::vector<std::pair<u32, std::string>>& dst) -> std::optional<bitorder_propagation::BitOrderResult> {
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
               const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& dst) -> std::optional<bitorder_propagation::BitOrderResult> {
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
            [](const bitorder_propagation::BitOrderResult& src,
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
                    Export collected bitorder information like word composition, known bitorder and connectivity in ``.json`` format to solve with external tools.
         
                    :param bitorder_propagation.BitOrderResult src: The bit orders that are already known. 
                    :param set[tuple(hal_py.Module,hal_py.ModulePinGroup)] dst: The pairs of module ID and pin group name with unknown bit order.
                    :param str export_filepath: The filepath where the ``.json`` file should be written to.
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
                    Export collected bitorder information like word composition, known bitorder and connectivity in ``.json`` format to solve with external tools.
         
                   :param tuple(hal_py.Module,hal_py.ModulePinGroup) src: The pair of module and pin group with known bit order. 
                    :param tuple(hal_py.Module,hal_py.ModulePinGroup) dst: The pair of module and pin group with unknown bit order.
                    :param str export_filepath: The filepath where the ``.json`` file should be written to.
                    :returns: The mapping from each mdoule/pingroup pair to its index on success, ``None`` otherwise.
                    :rtype: dict[tuple(hal_py.Module, hal_py.ModulePinGroup), int] or None
                )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
