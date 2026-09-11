#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#ifdef COMPILER_CLANG
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor-modified"
#endif

#include "hal_core/python_bindings/python_bindings.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"

#include <array>
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

    m.def(
        "quit",
        [](int exit_code, bool discard_changes) {
            gPythonContext->requestQuit(exit_code, discard_changes);
            if (gPythonContext->pythonThread() && QThread::currentThread() == gPythonContext->pythonThread())
            {
                // end the calling script the way sys.exit does; the GUI quits once the script's output is out
                PyErr_SetObject(PyExc_SystemExit, py::int_(exit_code).ptr());
                throw py::error_already_set();
            }
        },
        py::arg("exit_code")       = 0,
        py::arg("discard_changes") = false,
        R"(
        Close the GUI and end the HAL process with the given exit code.

        Unless changes are to be discarded, the main window is closed the way the user would close it, so a prompt for unsaved changes may appear and cancel the request.

        :param int exit_code: The exit code of the process. Defaults to 0.
        :param bool discard_changes: Set ``True`` to end without asking about unsaved changes, ``False`` to close the main window with its usual prompt. Defaults to ``False``.
    )");

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
    gui_input.def("input_string", [](std::string prompt = std::string("Please enter value"), std::string defval = std::string()) ->
            std::string { return (gPythonContext->pythonThread()
                                 ?gPythonContext->pythonThread()->handleStringInput(QString::fromStdString(prompt),QString::fromStdString(defval))
                                 :std::string());});
    gui_input.def("input_number", [](std::string prompt = std::string("Please enter number"), int defval = 0) ->
            int { return (gPythonContext->pythonThread()
                          ?gPythonContext->pythonThread()->handleNumberInput(QString::fromStdString(prompt),defval)
                          :0);});
    gui_input.def("input_gate", [](std::string prompt = std::string("Please select gate")) ->
            Gate* { return (gPythonContext->pythonThread()
                           ?gPythonContext->pythonThread()->handleGateInput(QString::fromStdString(prompt))
                           :nullptr);});
    gui_input.def("input_module", [](std::string prompt = std::string("Please select module")) ->
            Module* { return (gPythonContext->pythonThread()
                             ?gPythonContext->pythonThread()->handleModuleInput(QString::fromStdString(prompt))
                             :nullptr);});
    gui_input.def("input_filename", [](std::string prompt = std::string("Please select filename"), std::string filetype = std::string()) ->
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

            .def("set_gate_position", &GridPlacement::setGatePosition, py::arg("gate_id"), py::arg("point"), py::arg("swap") = false, R"(
                 Set position for gate identified by ID.

                 :param int gate_id: Gate ID.
                 :param tuple(int,int) pos: New position.
                 :param bool swap: set the swap of positions of the nodes
            )")

            .def("set_module_position", &GridPlacement::setModulePosition, py::arg("module_id"), py::arg("point"), py::arg("swap") = false, R"(
                 Set position for module identified by ID.

                 :param int module_id: Module ID.
                 :param tuple(int,int) pos: New position.
                 :param bool swap: set the swap of positions of the nodes
            )")

            .def("gate_position", &GridPlacement::gatePosition, py::arg("gate_id"), R"(
                 Query position for gate identified by ID.

                 :param int gate_id: Gate ID.
                 :returns: Position of gate or ``None`` if gate not found in hash.
                 :rtype: tuple(int,int) or None
            )")

            .def("module_position", &GridPlacement::modulePosition, py::arg("module_id"), R"(
                 Query position for module identified by ID.

                 :param int module_id: Module ID.
                 :returns: Position of module or ``None`` if module not found in hash.
                 :rtype: tuple(int,int) or None
            )");

    py::class_<GuiApiClasses::View>(py_gui_api, "View")
    .def_static("isolate_in_new", &GuiApiClasses::View::isolateInNew, py::arg("modules"), py::arg("gates"),R"(
        Isolates given modules and gates into a new view

        :param list[hal_py.module] modules: List of modules to be added.
        :param list[hal_py.Gate] gates: List of gates to be added.
        :returns: ID of created view or the existing one if view is exclusively bound to a module.
        :rtype: int
)")
    .def_static("rename", &GuiApiClasses::View::setName, py::arg("id"), py::arg("name"),R"(
        Renames the view specified by the given ID.

        :param int view_id: ID of the view.
        :param string name: New unique name.
        :returns: ``True`` on success, ``False`` otherwise.
        :rtype: bool
)")
    .def_static("add_to", &GuiApiClasses::View::addTo, py::arg("id"), py::arg("modules"), py::arg("gates"),R"(
        Adds the given modules and gates to the view specified by the ID.

        :param int view_id: ID of the view.
        :param list[hal.py.module] modules: Modules to be added.
        :param list[hal.py.Gate] gates: Gates to be added.
        :returns: ``True`` on success, ``False`` otherwise.
        :rtype: bool
)")
    .def_static("delete_view", &GuiApiClasses::View::deleteView, py::arg("id"),R"(
        Deletes the view specified by the ID.

        :param int view_id: ID of the view.
        :returns: ``True`` on success, ``False`` otherwise.
        :rtype: bool
        )")
    .def_static("remove_from", &GuiApiClasses::View::removeFrom, py::arg("id"), py::arg("modules"), py::arg("gates"),R"(
        Removes the given modules and gates from the view specified by the ID.

        :param int view_id: ID of the view.
        :param list[hal.py.module] modules: Modules to be removed.
        :param list[hal.py.Gate] gates: Gates to be removed.
        :returns: ``True`` on success, ``False`` otherwise.
        :rtype: bool
)")
    .def_static("get_id", &GuiApiClasses::View::getId, py::arg("name"),R"(
        Returns the ID of the view with the given name if existing.

        :param string name: Name of the view.
        :returns: ID of the specified view or 0 if none is found.
        :rtype: int
)")
    .def_static("get_name", &GuiApiClasses::View::getName, py::arg("id"), R"(
        Returns the name of the view with the given ID if existing.

        :param int view_id: ID of the view.
        :returns: Name of the view specified by the ID or empty string if none is found.
        :rtype: string
)")
    .def_static("get_modules", &GuiApiClasses::View::getModules, py::arg("id"), R"(
        Returns all modules attached to the view.

        :param int view_id: ID of the view.
        :returns: List of the attached modules.
        :rtype: list[hal.py.module]
)")
    .def_static("get_gates", &GuiApiClasses::View::getGates, py::arg("id"),R"(
        Returns all gates attached to the view

        :param int view_id: ID of the view.
        :returns: List of the attached gates.
        :rtype: list[hal.py.Gate]
)")
    .def_static("get_ids", &GuiApiClasses::View::getIds, py::arg("modules"), py::arg("gates"),R"(
        Returns the ID of each View containing at least the given modules and gates.

        :param list[hal.py.module] modules: Required modules.
        :param list[hal.py.Gate] gates: Required gates.
        :returns: List of ID of views which contains modules and gates.
        :rtype: list[int]
)")
    .def_static("unfold_module", &GuiApiClasses::View::unfoldModule, py::arg("view_id"), py::arg("module"), R"(
            Unfold a specific module. Hides the module, shows submodules and gates

            :param int view_id: ID of the view.
            :param Module* module: module to unfold
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
)")
    .def_static("fold_module", &GuiApiClasses::View::foldModule, py::arg("view_id"), py::arg("module"), R"(
            Fold a specific module. Hides the submodules and gates, shows the specific module

            :param int view_id: ID of the view.
            :param Module* module: module to fold
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
)")
    .def_static("get_grid_placement", &GuiApiClasses::View::getGridPlacement, py::arg("view_id"), R"(
            Get positions of all nodes in the view specified by id

            :param int view_id: ID of the view.
            :returns: GridPlacement of the specified view.
            :rtype: GridPlacement
)")
    .def_static("set_grid_placement", &GuiApiClasses::View::setGridPlacement, py::arg("view_id"), py::arg("grid_placement"), R"(
            Set grid placement to the view specified by id

            :param int viewId ID of the view.
            :param hal_gui.GuiApi.GridPlacement grid_placement: The grid placement.
            :rtype: bool
)")
    .def_static("get_current_directory", &GuiApiClasses::View::getCurrentDirectory,R"(
        Gets the CurrentDirectory.

        :returns: ID of the current directory. 0, if it's the top level directory.
        :rtype: int
)")
    .def_static("set_current_directory", &GuiApiClasses::View::setCurrentDirectory, py::arg("id"), R"(
        Sets the CurrentDirectory.
        
        :param int id ID of the new current directory.
)")
    .def_static("create_new_directory", &GuiApiClasses::View::createNewDirectory, py::arg("name"), R"(
        Creates a new directory under the current directory.
        
        :param string name: Name of the new directory.
        :returns: ID of the new directory.
        :rtype: int
)")
    .def_static("delete_directory", &GuiApiClasses::View::deleteDirectory, py::arg("id"), R"(
        Deletes the directory specified by a given id.
        
        :param int id: ID of the directory to delete.
)")
    .def_static("move_view", &GuiApiClasses::View::moveView, py::arg("view_id"), py::arg("destination_directory_id") = py::none(), py::arg("row") = py::none(), R"(
        Moves a view to a directory.
        
        :param int view_id: ID of the view to move.
        :param int destination_directory_id: ID of the destination directory to which the view will be moved. 
            If ``None``, the view is instead moved to the current directory.
        :param int row: The row index in the parent directory, where the view will be inserted.
)")
    .def_static("move_directory", &GuiApiClasses::View::moveDirectory, py::arg("directory_id"), py::arg("destination_directory_id") = py::none(), py::arg("row") = py::none(), R"(
        Moves a directory under another directory.
        
        :param int directory_id: ID of the directory to move.
        :param int destination_directory_id: ID of the destination directory to which the directory will be moved. 
            If ``None``, the directory is instead moved to the current directory.
        :param int row: The row index in the parent directory, where the directory will be inserted.
)")
    .def_static("get_child_directories", &GuiApiClasses::View::getChildDirectories, py::arg("directory_id"), R"(
        Returns the ids of all direct child directories of a given directory.
        
        :param int directory_id: ID of the parent directory, whose direct children will be returned
        :returns: List of the ids of all direct child directories of the specified directory. 
            Returns ``None``, if the given directory does not exist.
        :rtype: list[int]|None
)")
    .def_static("get_child_views", &GuiApiClasses::View::getChildViews, py::arg("directory_id"), R"(
        Returns the ids of all direct child views of a given directory.
        
        :param int directory_id: ID of the parent directory, whose direct children will be returned
        :returns: List of the ids of all direct child views of the specified directory. 
            Returns ``None``, if the given directory does not exist.
        :rtype: list[int]|None
)");


    py_gui_api.def("get_selected_gate_ids", &GuiApi::getSelectedGateIds, R"(
        Get the gate ids of currently selected gates in the graph view of the GUI.

        :returns: List of the ids of the currently selected gates.
        :rtype: list[int]
)");

    py_gui_api.def("get_selected_net_ids", &GuiApi::getSelectedNetIds, R"(
        Get the net ids of currently selected nets in the graph view of the GUI.

        :returns: List of the ids of the currently selected nets.
        :rtype: list[int]
)");

    py_gui_api.def("get_selected_module_ids", &GuiApi::getSelectedModuleIds, R"(
        Get the module ids of currently selected modules in the graph view of the GUI.

        :returns: List of the ids of the currently selected modules.
        :rtype: list[int]
)");

    py_gui_api.def("get_selected_item_ids", &GuiApi::getSelectedItemIds, R"(
        Get all item ids of the currently selected items in the graph view of the GUI.

        :returns: Tuple of lists of the currently selected items.
        :rtype: tuple(int, int, int)
)");

    py_gui_api.def("get_selected_gates", &GuiApi::getSelectedGates, borrowed(), R"(
        Get the gates which are currently selected in the graph view of the GUI.

        :returns: List of currently selected gates.
        :rtype: list[hal_py.Gate]
)");

    py_gui_api.def("get_selected_nets", &GuiApi::getSelectedNets, borrowed(), R"(
        Get the nets which are currently selected in the graph view of the GUI.

        :returns: List of currently selected nets.
        :rtype: list[hal_py.Net]
)");

    py_gui_api.def("get_selected_modules", &GuiApi::getSelectedModules, borrowed(), R"(
       Get the modules which are currently selected in the graph view of the GUI.

       :returns: List of currently selected modules.
       :rtype: list[hal_py.Module]
)");

    py_gui_api.def("get_selected_items", &GuiApi::getSelectedItems, borrowed(), R"(
       Get all selected items which are currently selected in the graph view of the GUI.

       :returns: Tuple of currently selected items.
       :rtype: tuple(hal_py.Gate, hal_py.Net, hal_py.Module)
)");

    py_gui_api.def("select_gate", py::overload_cast<u32, bool, bool>(&GuiApi::selectGate), py::arg("gate_id"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gate with id 'gate_id' in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gate with the id 'gate_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param int gate_id: The gate id of the gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_gate", py::overload_cast<Gate*, bool, bool>(&GuiApi::selectGate), py::arg("gate"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gate in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gate will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Gate gate: The gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_gate", py::overload_cast<const std::vector<u32>&, bool, bool>(&GuiApi::selectGate), py::arg("gate_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates with the ids in list 'gate_ids' in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gate with the id 'gate_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[int] gate_ids: List of gate ids of the gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_gate", py::overload_cast<const std::vector<Gate*>&, bool, bool>(&GuiApi::selectGate), py::arg("gates"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gates will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_net", py::overload_cast<u32, bool, bool>(&GuiApi::selectNet), py::arg("net_id"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the net with id 'mNetId' in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the net with the id 'mNetId' will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param int net_id: The net id of the net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_net", py::overload_cast<Net*, bool, bool>(&GuiApi::selectNet), py::arg("net"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the net in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the net will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Net net: The net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_net", py::overload_cast<const std::vector<u32>&, bool, bool>(&GuiApi::selectNet), py::arg("net_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the nets with the ids in list 'net_ids' in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the net with the id 'mNetId' will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[int] net_ids: List of net ids of the nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_net", py::overload_cast<const std::vector<Net*>&, bool, bool>(&GuiApi::selectNet), py::arg("nets"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the nets in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the nets will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Net] nets: The nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_module", py::overload_cast<u32, bool, bool>(&GuiApi::selectModule), py::arg("module_id"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the module with id 'module_id' in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the module with the id 'module_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param int module_id: The module id of the module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_module", py::overload_cast<Module*, bool, bool>(&GuiApi::selectModule), py::arg("module"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the module in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the module will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.module module: The module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_module", py::overload_cast<const std::vector<u32>&, bool, bool>(&GuiApi::selectModule), py::arg("module_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the modules with the ids in list 'module_ids' in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the module with the id 'module_id' will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[int] module_ids: List of module ids of the modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select_module", py::overload_cast<const std::vector<Module*>&, bool, bool>(&GuiApi::selectModule), py::arg("modules"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the modules in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the modules will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<Gate*, bool, bool>(&GuiApi::select), py::arg("gate"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gate in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gate will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Gate gate: The gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<Net*, bool, bool>(&GuiApi::select), py::arg("net"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the net in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the net will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.Net net: The net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<Module*, bool, bool>(&GuiApi::select), py::arg("module"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the module in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the module will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param hal_py.module module: The module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Gate*>&, bool, bool>(&GuiApi::select), py::arg("gates"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gates will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Net*>&, bool, bool>(&GuiApi::select), py::arg("nets"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the nets in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the nets will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Net] nets: The nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Module*>&, bool, bool>(&GuiApi::select), py::arg("modules"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the modules in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the modules will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&, bool, bool>(&GuiApi::select), py::arg("gate_ids"), py::arg("net_ids"), py::arg("module_ids"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates, nets and modules with the passed ids in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gates, nets and modules will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The ids of the gates to be selected.
       :param list[hal_py.Net] nets: The ids of the nets to be selected.
       :param list[hal_py.module] modules: The ids of the modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("select", py::overload_cast<const std::vector<Gate*>&, const std::vector<Net*>&, const std::vector<Module*>&, bool, bool>(&GuiApi::select), py::arg("gates"), py::arg("nets"), py::arg("modules"), py::arg("clear_current_selection") = true, py::arg("navigate_to_selection") = true, R"(
       Select the gates, nets and modules in the graph view of the GUI.
       If 'clear_current_selection' is ``False``, the gates, nets and modules will be added to the currently existing selection.
       If 'navigate_to_selection' is ``False``, the graph view will not modify the graph view camera position to fit all selected items.

       :param list[hal_py.Gate] gates: The gates to be selected.
       :param list[hal_py.Net] nets: The nets to be selected.
       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
       :param bool navigate_to_selection: Determines if the graph view scrolls and zooms to show all selected items.
)");

    py_gui_api.def("deselect_gate", py::overload_cast<u32>(&GuiApi::deselectGate), py::arg("gate_id"), R"(
       Deselect the gate with id 'gate_id' in the graph view of the GUI.

       :param int gate_id: The gate id of the gate to be selected.
)");

    py_gui_api.def("deselect_gate", py::overload_cast<Gate*>(&GuiApi::deselectGate), py::arg("gate"), R"(
       Deselect the gate in the graph view of the GUI.

       :param hal_py.Gate gate: The gate to be deselected.
)");

    py_gui_api.def("deselect_gate", py::overload_cast<const std::vector<u32>&>(&GuiApi::deselectGate), py::arg("gate_ids"), R"(
       Deselect the gates with the ids in list 'gate_ids' in the graph view of the GUI.

       :param list[int] gate_ids: List of gate ids of the gates to be deselected.
)");

    py_gui_api.def("deselect_gate", py::overload_cast<const std::vector<Gate*>&>(&GuiApi::deselectGate), py::arg("gates"), R"(
       Deselect the gates in the graph view of the GUI.

       :param list[hal_py.Gate] gates: The gates to be deselected.
)");

    py_gui_api.def("deselect_net", py::overload_cast<u32>(&GuiApi::deselectNet), py::arg("net_id"), R"(
       Deselect the net with id 'mNetId' in the graph view of the GUI.

       :param int net_id: The net id of the net to be selected.
)");

    py_gui_api.def("deselect_net", py::overload_cast<Net*>(&GuiApi::deselectNet), py::arg("net"), R"(
       Deselect the net in the graph view of the GUI.

       :param hal_py.Net Net: The net to be deselected.
)");

    py_gui_api.def("deselect_net", py::overload_cast<const std::vector<u32>&>(&GuiApi::deselectNet), py::arg("net_ids"), R"(
       Deselect the nets with the ids in list 'net_ids' in the graph view of the GUI.

       :param list[int] net_ids: List of net ids of the nets to be deselected.
)");

    py_gui_api.def("deselect_net", py::overload_cast<const std::vector<Net*>&>(&GuiApi::deselectNet), py::arg("nets"), R"(
       Deselect the nets in the graph view of the GUI.

       :param list[hal_py.Net] nets: The nets to be deselected.
)");

    py_gui_api.def("deselect_module", py::overload_cast<u32>(&GuiApi::deselectModule), py::arg("module_id"), R"(
       Deselect the module with id 'module_id' in the graph view of the GUI.

       :param int module_id: The module id of the module to be selected.
)");

    py_gui_api.def("deselect_module", py::overload_cast<Module*>(&GuiApi::deselectModule), py::arg("module"), R"(
       Deselect the module in the graph view of the GUI.

       :param hal_py.module module: The module to be deselected.
)");

    py_gui_api.def("deselect_module", py::overload_cast<const std::vector<u32>&>(&GuiApi::deselectModule), py::arg("module_ids"), R"(
       Deselect the modules with the ids in list 'module_ids' in the graph view of the GUI.

       :param list[int] module_ids: List of module ids of the modules to be deselected.
)");

    py_gui_api.def("deselect_module", py::overload_cast<const std::vector<Module*>&>(&GuiApi::deselectModule), py::arg("modules"), R"(
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

    py_gui_api.def("deselect_all_items", py::overload_cast<>(&GuiApi::deselectAllItems), R"(
       Deselect all gates, nets and modules in the graph view of the GUI.
)");




    // The bindings follow the snake_case convention of hal_py. The former camel-case names stay as aliases that
    // warn once per name and process, like the deprecated NetlistUtils bindings, and go away in the release after next.
    const std::vector<std::array<const char*, 4>> deprecated_aliases = {
        {"GuiApi", "method", "deselectAllItems", "deselect_all_items"},
        {"GuiApi", "method", "deselectGate", "deselect_gate"},
        {"GuiApi", "method", "deselectModule", "deselect_module"},
        {"GuiApi", "method", "deselectNet", "deselect_net"},
        {"GuiApi", "method", "getSelectedGateIds", "get_selected_gate_ids"},
        {"GuiApi", "method", "getSelectedGates", "get_selected_gates"},
        {"GuiApi", "method", "getSelectedItemIds", "get_selected_item_ids"},
        {"GuiApi", "method", "getSelectedItems", "get_selected_items"},
        {"GuiApi", "method", "getSelectedModuleIds", "get_selected_module_ids"},
        {"GuiApi", "method", "getSelectedModules", "get_selected_modules"},
        {"GuiApi", "method", "getSelectedNetIds", "get_selected_net_ids"},
        {"GuiApi", "method", "getSelectedNets", "get_selected_nets"},
        {"GuiApi", "method", "selectGate", "select_gate"},
        {"GuiApi", "method", "selectModule", "select_module"},
        {"GuiApi", "method", "selectNet", "select_net"},
        {"GuiApi.GridPlacement", "method", "gatePosition", "gate_position"},
        {"GuiApi.GridPlacement", "method", "modulePosition", "module_position"},
        {"GuiApi.GridPlacement", "method", "setGatePosition", "set_gate_position"},
        {"GuiApi.GridPlacement", "method", "setModulePosition", "set_module_position"},
        {"GuiApi.View", "static", "addTo", "add_to"},
        {"GuiApi.View", "static", "createNewDirectory", "create_new_directory"},
        {"GuiApi.View", "static", "deleteDirectory", "delete_directory"},
        {"GuiApi.View", "static", "deleteView", "delete_view"},
        {"GuiApi.View", "static", "foldModule", "fold_module"},
        {"GuiApi.View", "static", "getChildDirectories", "get_child_directories"},
        {"GuiApi.View", "static", "getChildViews", "get_child_views"},
        {"GuiApi.View", "static", "getCurrentDirectory", "get_current_directory"},
        {"GuiApi.View", "static", "getGates", "get_gates"},
        {"GuiApi.View", "static", "getGridPlacement", "get_grid_placement"},
        {"GuiApi.View", "static", "getId", "get_id"},
        {"GuiApi.View", "static", "getIds", "get_ids"},
        {"GuiApi.View", "static", "getModules", "get_modules"},
        {"GuiApi.View", "static", "getName", "get_name"},
        {"GuiApi.View", "static", "isolateInNew", "isolate_in_new"},
        {"GuiApi.View", "static", "moveDirectory", "move_directory"},
        {"GuiApi.View", "static", "moveView", "move_view"},
        {"GuiApi.View", "static", "removeFrom", "remove_from"},
        {"GuiApi.View", "static", "setCurrentDirectory", "set_current_directory"},
        {"GuiApi.View", "static", "setGridPlacement", "set_grid_placement"},
        {"GuiApi.View", "static", "unfoldModule", "unfold_module"},
        {"gui_input", "module", "inputFilename", "input_filename"},
        {"gui_input", "module", "inputGate", "input_gate"},
        {"gui_input", "module", "inputModule", "input_module"},
        {"gui_input", "module", "inputNumber", "input_number"},
        {"gui_input", "module", "inputString", "input_string"}
    };
    py::exec(R"(
def _install_deprecated_alias(module, qualified, kind, old, new):
    owner = module
    for part in qualified.split("."):
        owner = getattr(owner, part)
    target = getattr(owner, new)
    state = {"warned": False}
    def wrapper(*args, **kwargs):
        if not state["warned"]:
            state["warned"] = True
            import hal_py
            hal_py.log_warning("python_context", "hal_gui." + qualified + "." + old + " is deprecated and will be removed in a future version, use " + new + " instead.")
        return target(*args, **kwargs)
    wrapper.__name__ = old
    wrapper.__qualname__ = qualified + "." + old
    wrapper.__doc__ = "Deprecated alias of ``" + new + "``, which will be removed in a future version."
    setattr(owner, old, staticmethod(wrapper) if kind == "static" else wrapper)
)", m.attr("__dict__"));
    for (const auto& [qualified, kind, old_name, new_name] : deprecated_aliases)
    {
        m.attr("_install_deprecated_alias")(m, qualified, kind, old_name, new_name);
    }
    py::delattr(m, "_install_deprecated_alias");

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
    }
}
