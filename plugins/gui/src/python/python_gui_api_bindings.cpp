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
#include "gui/python/python_thread.h"

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

    py_console.def("clear", []() -> void { gPythonContext->scheduleClear(); });
    py_console.def("reset", []() -> void { gPythonContext->scheduleReset(); });

    //m.def("history", []() -> void { g_console->printHistory(g_console->m_cmdColor); });

    py::module m2 = py_console.def_submodule("redirector", "redirector");
    m2.def("write_stdout", [](std::string s) -> void { gPythonContext->forwardStdout(QString::fromStdString(s)); });
    m2.def("write_stderr", [](std::string s) -> void { gPythonContext->forwardError(QString::fromStdString(s)); });
    m2.def("thread_stdout", [](std::string s) -> void { if (gPythonContext->pythonThread()) gPythonContext->pythonThread()->handleStdout(QString::fromStdString(s)); });
    m2.def("thread_stderr", [](std::string s) -> void { if (gPythonContext->pythonThread()) gPythonContext->pythonThread()->handleError(QString::fromStdString(s)); });
    m2.def("thread_stdin", [](std::string s) -> std::string { return (gPythonContext->pythonThread()
                                                                      ?gPythonContext->pythonThread()->handleConsoleInput(QString::fromStdString(s))
                                                                      :std::string());});
    auto gui_input = m.def_submodule("gui_input", R"(
        GUI Input Widgets
    )");
    gui_input.def("inputString", [](std::string prompt = std::string("Please enter value"), std::string defval = std::string()) ->
            std::string { return (gPythonContext->pythonThread()
                                 ?gPythonContext->pythonThread()->handleStringInput(QString::fromStdString(prompt),QString::fromStdString(defval))
                                 :std::string());});
    gui_input.def("inputNumber", [](std::string prompt = std::string("Please enter number"), int defval = 0) ->
            int { return (gPythonContext->pythonThread()
                          ?gPythonContext->pythonThread()->handleNumberInput(QString::fromStdString(prompt),defval)
                          :0);});
    gui_input.def("inputGate", [](std::string prompt = std::string("Please select gate")) ->
            Gate* { return (gPythonContext->pythonThread()
                           ?gPythonContext->pythonThread()->handleGateInput(QString::fromStdString(prompt))
                           :nullptr);});
    gui_input.def("inputModule", [](std::string prompt = std::string("Please select module")) ->
            Module* { return (gPythonContext->pythonThread()
                             ?gPythonContext->pythonThread()->handleModuleInput(QString::fromStdString(prompt))
                             :nullptr);});
    gui_input.def("inputFilename", [](std::string prompt = std::string("Please select filename"), std::string filetype = std::string()) ->
            std::string { return (gPythonContext->pythonThread()
                             ?gPythonContext->pythonThread()->handleFilenameInput(QString::fromStdString(prompt), QString::fromStdString(filetype))
                             :std::string());});
    gui_input.def("wait_for_menu_selection", []() -> void {
                      if (gPythonContext->pythonThread())
                      gPythonContext->pythonThread()->getInput(PythonThread::WaitForMenuSelection,QString(),QVariant());});

    py::class_<GuiApi> py_gui_api(m, "GuiApi", R"(GUI API)");

    py::class_<GridPlacement>(py_gui_api,"GridPlacement",R"(
                      Helper class to determine placement of nodes on gui grid.
            )")

            .def(py::init<>(), R"(
                 Constructor for empty placement hash.
            )")

            .def("setGatePosition", &GridPlacement::setGatePosition, py::arg("gateId"), py::arg("point"), py::arg("swap") = false, R"(
                 Set position for gate identified by ID.

                 :param int gateId: Gate ID.
                 :param tuple(int,int) pos: New position.
                 :param bool swap: set the swap of positions of the nodes
            )")

            .def("setModulePosition", &GridPlacement::setModulePosition, py::arg("moduleId"), py::arg("point"), py::arg("swap") = false, R"(
                 Set position for module identified by ID.

                 :param int moduleId: Module ID.
                 :param tuple(int,int) pos: New position.
                 :param bool swap: set the swap of positions of the nodes
            )")

            .def("gatePosition", &GridPlacement::gatePosition, py::arg("gateId"), R"(
                 Query position for gate identified by ID.

                 :param int gateId: Gate ID.
                 :returns: Position of gate or None if gate not found in hash.
                 :rtype: tuple(int,int) or None
            )")

            .def("modulePosition", &GridPlacement::modulePosition, py::arg("moduleId"), R"(
                 Query position for module identified by ID.

                 :param int moduleId: Module ID.
                 :returns: Position of module or None if module not found in hash.
                 :rtype: tuple(int,int) or None
            )");

    py::class_<GuiApiClasses::View>(py_gui_api, "View")
    .def_static("isolateInNew", &GuiApiClasses::View::isolateInNew, py::arg("modules"), py::arg("gates"),R"(
        Isolates given modules and gates into a new view

        :param list[hal_py.module] modules: List of modules to be added.
        :param list[hal_py.Gate] gates: List of gates to be added.
        :returns: ID of created view or the existing one if view is exclusively bound to a module.
        :rtype: int
)")
    .def_static("rename", &GuiApiClasses::View::setName, py::arg("id"), py::arg("name"),R"(
        Renames the view specified by the given ID.

        :param int viewId: ID of the view.
        :param string name: New unique name.
        :returns: True on success otherwise False.
        :rtype: bool
)")
    .def_static("addTo", &GuiApiClasses::View::addTo, py::arg("id"), py::arg("modules"), py::arg("gates"),R"(
        Adds the given modules and gates to the view specified by the ID.

        :param int viewId: ID of the view.
        :param list[hal.py.module] modules: Modules to be added.
        :param list[hal.py.Gate] gates: Gates to be added.
        :returns: True on success, otherwise False.
        :rtype: bool
)")
    .def_static("deleteView", &GuiApiClasses::View::deleteView, py::arg("id"),R"(
        Deletes the view specified by the ID.

        :param int viewId: ID of the view.
        :returns: True on success, otherwise False.
        :rtype: bool
        )")
    .def_static("removeFrom", &GuiApiClasses::View::removeFrom, py::arg("id"), py::arg("modules"), py::arg("gates"),R"(
        Removes the given modules and gates from the view specified by the ID.

        :param int viewId: ID of the view.
        :param list[hal.py.module] modules: Modules to be removed.
        :param list[hal.py.Gate] gates: Gates to be removed.
        :returns: True on success, otherwise False.
        :rtype: bool
)")
    .def_static("getId", &GuiApiClasses::View::getId, py::arg("name"),R"(
        Returns the ID of the view with the given name if existing.

        :param string name: Name of the view.
        :returns: ID of the specified view or 0 if none is found.
        :rtype: int
)")
    .def_static("getName", &GuiApiClasses::View::getName, py::arg("id"), R"(
        Returns the name of the view with the given ID if existing.

        :param int viewId: ID of the view.
        :returns: Name of the view specified by the ID or empty string if none is found.
        :rtype: string
)")
    .def_static("getModules", &GuiApiClasses::View::getModules, py::arg("id"), R"(
        Returns all modules attached to the view.

        :param int viewId: ID of the view.
        :returns: List of the attached modules.
        :rtype: list[hal.py.module]
)")
    .def_static("getGates", &GuiApiClasses::View::getGates, py::arg("id"),R"(
        Returns all gates attached to the view

        :param int viewId: ID of the view.
        :returns: List of the attached gates.
        :rtype: list[hal.py.Gate]
)")
    .def_static("getIds", &GuiApiClasses::View::getIds, py::arg("modules"), py::arg("gates"),R"(
        Returns the ID of each View containing at least the given modules and gates.

        :param list[hal.py.module] modules: Required modules.
        :param list[hal.py.Gate] gates: Required gates.
        :returns: List of ID of views which contains modules and gates.
        :rtype: list[int]
)")
    .def_static("unfoldModule", &GuiApiClasses::View::unfoldModule, py::arg("view_id"), py::arg("module"), R"(
            Unfold a specific module. Hides the module, shows submodules and gates

            :param int viewId: ID of the view.
            :param Module* module: module to unfold
            :returns: True on success, otherwise False.
            :rtype: bool
)")
    .def_static("foldModule", &GuiApiClasses::View::foldModule, py::arg("view_id"), py::arg("module"), R"(
            Fold a specific module. Hides the submodules and gates, shows the specific module

            :param int viewId: ID of the view.
            :param Module* module: module to fold
            :returns: True on success, otherwise False.
            :rtype: bool
)")
    .def_static("getGridPlacement", &GuiApiClasses::View::getGridPlacement, py::arg("view_id"), R"(
            Get positions of all nodes in the view specified by id

            :param int viewId: ID of the view.
            :returns: GridPlacement of the specified view.
            :rtype: GridPlacement
)")
    .def_static("setGridPlacement", &GuiApiClasses::View::setGridPlacement, py::arg("view_id"), py::arg("grid placement"), R"(
            Set grid placement to the view specified by id

            :param int viewId ID of the view.
            :param GridPlacement* gp: grid placement.
            :rtype: bool
)")
    .def_static("getCurrentDirectory", &GuiApiClasses::View::getCurrentDirectory,R"(
        Gets the CurrentDirectory.

        :returns: ID of the current directory. 0, if it's the top level directory.
        :rtype: int
)")
    .def_static("setCurrentDirectory", &GuiApiClasses::View::setCurrentDirectory, py::arg("id"), R"(
        Sets the CurrentDirectory.
        
        :param int id ID of the new current directory.
)")
    .def_static("createNewDirectory", &GuiApiClasses::View::createNewDirectory, py::arg("name"), R"(
        Creates a new directory under the current directory.
        
        :param string name: Name of the new directory.
        :returns: ID of the new directory.
        :rtype: int
)")
    .def_static("deleteDirectory", &GuiApiClasses::View::deleteDirectory, py::arg("id"), R"(
        Deletes the directory specified by a given id.
        
        :param int id: ID of the directory to delete.
)")
    .def_static("moveView", &GuiApiClasses::View::moveView, py::arg("viewId"), py::arg("destinationDirectoryId") = py::none(), py::arg("row") = py::none(), R"(
        Moves a view to a directory.
        
        :param int viewId: ID of the view to move.
        :param int destinationDirectoryId: ID of the destination directory to which the view will be moved. 
            If None, the view is instead moved to the current directory.
        :param int row: The row index in the parent directory, where the view will be inserted.
)")
    .def_static("moveDirectory", &GuiApiClasses::View::moveDirectory, py::arg("directoryId"), py::arg("destinationDirectoryId") = py::none(), py::arg("row") = py::none(), R"(
        Moves a directory under another directory.
        
        :param int directoryId: ID of the directory to move.
        :param int destinationDirectoryId: ID of the destination directory to which the directory will be moved. 
            If None, the directory is instead moved to the current directory.
        :param int row: The row index in the parent directory, where the directory will be inserted.
)")
    .def_static("getChildDirectories", &GuiApiClasses::View::getChildDirectories, py::arg("directoryId"), R"(
        Returns the ids of all direct child directories of a given directory.
        
        :param int directoryId: ID of the parent directory, whose direct children will be returned
        :returns: List of the ids of all direct child directories of the specified directory. 
            Returns None, if the given directory does not exist.
        :rtype: list[int]|None
)")
    .def_static("getChildViews", &GuiApiClasses::View::getChildViews, py::arg("directoryId"), R"(
        Returns the ids of all direct child views of a given directory.
        
        :param int directoryId: ID of the parent directory, whose direct children will be returned
        :returns: List of the ids of all direct child views of the specified directory. 
            Returns None, if the given directory does not exist.
        :rtype: list[int]|None
)");


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
