#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#ifdef COMPILER_CLANG
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor-modified"
#endif

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "pybind11/functional.h"

#include "gui/gui_globals.h"
#include "gui/gui_api/gui_api.h"

#pragma GCC diagnostic pop

namespace hal{

namespace py = pybind11;
#ifdef PYBIND11_MODULE

PYBIND11_MODULE(hal_gui, m)
{
    m.doc() = "hal python bindings";
#else
PYBIND11_PLUGIN(hal_gui)
    {
        py::module m("hal_gui", "hal gui python bindings");
#endif    // ifdef PYBIND11_MODULE

    auto py_console = m.def_submodule("console", R"(
        GUI Console
)");

    py_console.def("clear", []() -> void { gPythonContext->forwardClear(); });
    py_console.def("reset", []() -> void { gPythonContext->forwardReset(); });

    //m.def("history", []() -> void { g_console->printHistory(g_console->m_cmdColor); });

    py::module m2 = py_console.def_submodule("redirector", "redirector");
    m2.def("write_stdout", [](std::string s) -> void { gPythonContext->forwardStdout(QString::fromStdString(s)); });
    m2.def("write_stderr", [](std::string s) -> void { gPythonContext->forwardError(QString::fromStdString(s)); });


    py::class_<GuiApi> py_gui_api(m, "GuiApi", R"(GUI API)");

    py_gui_api.def("getSelectedGateIds", &GuiApi::getSelectedGateIds, R"(
        Get the gate ids of currently selected gates in the graph view of the GUI.

        :returns: List of the ids of the currently selected gates.
        :rtype: list[int]
)");

    py_gui_api.def("getSelectedNetIds", &GuiApi::getSelectedNetIds, R"(
        Get the net ids of currently selected nets in the graph view of the GUI.

        :returns: List of the ids of the currently selected nets.
        :rtype: list[int]
)");

    py_gui_api.def("getSelectedModuleIds", &GuiApi::getSelectedModuleIds, R"(
        Get the module ids of currently selected modules in the graph view of the GUI.

        :returns: List of the ids of the currently selected modules.
        :rtype: list[int]
)");

    py_gui_api.def("getSelectedItemIds", &GuiApi::getSelectedItemIds, R"(
        Get all item ids of the currently selected items in the graph view of the GUI.

        :returns: Tuple of lists of the currently selected items.
        :rtype: tuple(int, int, int)
)");

    py_gui_api.def("getSelectedGates", &GuiApi::getSelectedGates, R"(
        Get the gates which are currently selected in the graph view of the GUI.

        :returns: List of currently selected gates.
        :rtype: list[hal_py.Gate]
)");

    py_gui_api.def("getSelectedNets", &GuiApi::getSelectedNets, R"(
        Get the nets which are currently selected in the graph view of the GUI.

        :returns: List of currently selected nets.
        :rtype: list[hal_py.Net]
)");

    py_gui_api.def("getSelectedModules", &GuiApi::getSelectedModules, R"(
       Get the modules which are currently selected in the graph view of the GUI.

       :returns: List of currently selected modules.
       :rtype: list[hal_py.module]
)");

    py_gui_api.def("getSelectedItems", &GuiApi::getSelectedItems, R"(
       Get all selected items which are currently selected in the graph view of the GUI.

       :returns: Tuple of currently selected items.
       :rtype: tuple(hal_py.Gate, hal_py.Net, hal_py.module)
)");

    py_gui_api.def("selectGate", py::overload_cast<u32, bool, bool>(&GuiApi::selectGate), py::arg("gate_id"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gate with id 'gate_id' in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate with the id 'gate_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param int gate_id: The gate id of the gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectGate", py::overload_cast<Gate*, bool, bool>(&GuiApi::selectGate), py::arg("gate"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gate in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Gate gate: The gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectGate", py::overload_cast<const std::vector<u32>&, bool, bool>(&GuiApi::selectGate), py::arg("gate_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates with the ids in list 'gate_ids' in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate with the id 'gate_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[int] gate_ids: List of gate ids of the gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectGate", py::overload_cast<const std::vector<Gate*>&, bool, bool>(&GuiApi::selectGate), py::arg("gates"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectNet", py::overload_cast<u32, bool, bool>(&GuiApi::selectNet), py::arg("mNetId"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the net with id 'mNetId' in the graph view of the GUI.
       If 'clear_current_selection' is false, the net with the id 'mNetId' will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param int mNetId: The net id of the net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectNet", py::overload_cast<Net*, bool, bool>(&GuiApi::selectNet), py::arg("net"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the net in the graph view of the GUI.
       If 'clear_current_selection' is false, the net will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Net net: The net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectNet", py::overload_cast<const std::vector<u32>&, bool, bool>(&GuiApi::selectNet), py::arg("net_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the nets with the ids in list 'net_ids' in the graph view of the GUI.
       If 'clear_current_selection' is false, the net with the id 'mNetId' will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[int] net_ids: List of net ids of the nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectNet", py::overload_cast<const std::vector<Net*>&, bool, bool>(&GuiApi::selectNet), py::arg("nets"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the nets in the graph view of the GUI.
       If 'clear_current_selection' is false, the nets will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Net] nets: The nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectModule", py::overload_cast<u32, bool, bool>(&GuiApi::selectModule), py::arg("module_id"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the module with id 'module_id' in the graph view of the GUI.
       If 'clear_current_selection' is false, the module with the id 'module_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param int module_id: The module id of the module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectModule", py::overload_cast<Module*, bool, bool>(&GuiApi::selectModule), py::arg("module"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the module in the graph view of the GUI.
       If 'clear_current_selection' is false, the module will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.module module: The module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectModule", py::overload_cast<const std::vector<u32>&, bool, bool>(&GuiApi::selectModule), py::arg("module_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the modules with the ids in list 'module_ids' in the graph view of the GUI.
       If 'clear_current_selection' is false, the module with the id 'module_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[int] module_ids: List of module ids of the modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("selectModule", py::overload_cast<const std::vector<Module*>&, bool, bool>(&GuiApi::selectModule), py::arg("modules"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the modules in the graph view of the GUI.
       If 'clear_current_selection' is false, the modules will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<Gate*, bool, bool>(&GuiApi::select), py::arg("gate"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gate in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Gate gate: The gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<Net*, bool, bool>(&GuiApi::select), py::arg("net"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the net in the graph view of the GUI.
       If 'clear_current_selection' is false, the net will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Net net: The net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<Module*, bool, bool>(&GuiApi::select), py::arg("module"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the module in the graph view of the GUI.
       If 'clear_current_selection' is false, the module will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.module module: The module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Gate*>&, bool, bool>(&GuiApi::select), py::arg("gates"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Net*>&, bool, bool>(&GuiApi::select), py::arg("nets"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the nets in the graph view of the GUI.
       If 'clear_current_selection' is false, the nets will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Net] nets: The nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Module*>&, bool, bool>(&GuiApi::select), py::arg("modules"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the modules in the graph view of the GUI.
       If 'clear_current_selection' is false, the modules will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&, bool, bool>(&GuiApi::select), py::arg("gate_ids"), py::arg("net_ids"), py::arg("module_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates, nets and modules with the passed ids in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates, nets and modules will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The ids of the gates to be selected.
       :param list[hal_py.Net] nets: The ids of the nets to be selected.
       :param list[hal_py.module] modules: The ids of the modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Gate*>&, const std::vector<Net*>&, const std::vector<Module*>&, bool, bool>(&GuiApi::select), py::arg("gates"), py::arg("nets"), py::arg("modules"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates, nets and modules in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates, nets and modules will be added to the currently existing selection.
       If 'navigate_to_selection' is false, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The gates to be selected.
       :param list[hal_py.Net] nets: The nets to be selected.
       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("deselectGate", py::overload_cast<u32>(&GuiApi::deselectGate), py::arg("gate_id"), R"(
       Deselect the gate with id 'gate_id' in the graph view of the GUI.

       :param int gate_id: The gate id of the gate to be selected.
)");

    py_gui_api.def("deselectGate", py::overload_cast<Gate*>(&GuiApi::deselectGate), py::arg("gate"), R"(
       Deselect the gate in the graph view of the GUI.

       :param hal_py.Gate gate: The gate to be deselected.
)");

    py_gui_api.def("deselectGate", py::overload_cast<const std::vector<u32>&>(&GuiApi::deselectGate), py::arg("gate_ids"), R"(
       Deselect the gates with the ids in list 'gate_ids' in the graph view of the GUI.

       :param list[int] gate_ids: List of gate ids of the gates to be deselected.
)");

    py_gui_api.def("deselectGate", py::overload_cast<const std::vector<Gate*>&>(&GuiApi::deselectGate), py::arg("gates"), R"(
       Deselect the gates in the graph view of the GUI.

       :param list[hal_py.Gate] gates: The gates to be deselected.
)");

    py_gui_api.def("deselectNet", py::overload_cast<u32>(&GuiApi::deselectNet), py::arg("mNetId"), R"(
       Deselect the net with id 'mNetId' in the graph view of the GUI.

       :param int mNetId: The net id of the net to be selected.
)");

    py_gui_api.def("deselectNet", py::overload_cast<Net*>(&GuiApi::deselectNet), py::arg("net"), R"(
       Deselect the net in the graph view of the GUI.

       :param hal_py.Net Net: The net to be deselected.
)");

    py_gui_api.def("deselectNet", py::overload_cast<const std::vector<u32>&>(&GuiApi::deselectNet), py::arg("net_ids"), R"(
       Deselect the nets with the ids in list 'net_ids' in the graph view of the GUI.

       :param list[int] net_ids: List of net ids of the nets to be deselected.
)");

    py_gui_api.def("deselectNet", py::overload_cast<const std::vector<Net*>&>(&GuiApi::deselectNet), py::arg("nets"), R"(
       Deselect the nets in the graph view of the GUI.

       :param list[hal_py.Net] nets: The nets to be deselected.
)");

    py_gui_api.def("deselectModule", py::overload_cast<u32>(&GuiApi::deselectModule), py::arg("module_id"), R"(
       Deselect the module with id 'module_id' in the graph view of the GUI.

       :param int module_id: The module id of the module to be selected.
)");

    py_gui_api.def("deselectModule", py::overload_cast<Module*>(&GuiApi::deselectModule), py::arg("module"), R"(
       Deselect the module in the graph view of the GUI.

       :param hal_py.module module: The module to be deselected.
)");

    py_gui_api.def("deselectModule", py::overload_cast<const std::vector<u32>&>(&GuiApi::deselectModule), py::arg("module_ids"), R"(
       Deselect the modules with the ids in list 'module_ids' in the graph view of the GUI.

       :param list[int] module_ids: List of module ids of the modules to be deselected.
)");

    py_gui_api.def("deselectModule", py::overload_cast<const std::vector<Module*>&>(&GuiApi::deselectModule), py::arg("modules"), R"(
       Deselect the modules in the graph view of the GUI.

       :param list[hal_py.module] modules: The modules to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<Gate*>(&GuiApi::deselect), py::arg("gate"), R"(
       Deselect the gate in the graph view of the GUI.

       :param hal_py.Gate gate: The gate to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<Net*>(&GuiApi::deselect), py::arg("net"), R"(
       Deselect the net in the graph view of the GUI.

       :param hal_py.Net Net: The net to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<Module*>(&GuiApi::deselect), py::arg("module"), R"(
       Deselect the module in the graph view of the GUI.

       :param hal_py.module module: The module to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<const std::vector<Gate*>&>(&GuiApi::deselect), py::arg("gates"), R"(
       Deselect the gates in the graph view of the GUI.

       :param list[hal_py.Gate] gates: The gates to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<const std::vector<Net*>&>(&GuiApi::deselect), py::arg("nets"), R"(
       Deselect the nets in the graph view of the GUI.

       :param list[hal_py.Net] nets: The nets to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<const std::vector<Module*>&>(&GuiApi::deselect), py::arg("modules"), R"(
       Deselect the modules in the graph view of the GUI.

       :param list[hal_py.module] modules: The modules to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&>(&GuiApi::deselect), py::arg("gate_ids"), py::arg("net_ids"), py::arg("module_ids"), R"(
       Deselect the gates, nets and modules with the passed ids in the graph view of the GUI.

       :param list[hal_py.Gate] gates: The ids of the gates to be deselected.
       :param list[hal_py.Net] nets: The ids of the nets to be deselected.
       :param list[hal_py.module] modules: The ids of the modules to be deselected.
)");

    py_gui_api.def("deselect", py::overload_cast<const std::vector<Gate*>&, const std::vector<Net*>&, const std::vector<Module*>&>(&GuiApi::deselect), py::arg("gates"), py::arg("nets"), py::arg("modules"), R"(
       Deselect the gates, nets and modules in the graph view of the GUI.

       :param list[hal_py.Gate] gates: The gates to be deselected.
       :param list[hal_py.Net] nets: The nets to be deselected.
       :param list[hal_py.module] modules: The modules to be deselected.
)");

    py_gui_api.def("deselectAllItems", py::overload_cast<>(&GuiApi::deselectAllItems), R"(
       Deselect all gates, nets and modules in the graph view of the GUI.
)");

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
    }
}
