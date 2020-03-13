#include <QDir>
#include <fstream>
#include <gui/python/python_context.h>

#include "python/python_context.h"

#include "core/log.h"
#include "core/utils.h"

#include "gui/python/python_context_suberscriber.h"
#include "gui_globals.h"
#include "gui_api/gui_api.h"

// Following is needed for python_context::check_complete_statement
#include <Python.h>
#include <compile.h>
#include <errcode.h>
#include <grammar.h>
#include <node.h>
#include <parsetok.h>

extern grammar _PyParser_Grammar;

PYBIND11_EMBEDDED_MODULE(console, m)
{
    m.def("clear", []() -> void { g_python_context->forward_clear(); });
    m.def("reset", []() -> void { g_python_context->forward_reset(); });

    //m.def("history", []() -> void { g_console->printHistory(g_console->m_cmdColor); });

    py::module m2 = m.def_submodule("redirector", "redirector");
    m2.def("write_stdout", [](std::string s) -> void { g_python_context->forward_stdout(QString::fromStdString(s)); });
    m2.def("write_stderr", [](std::string s) -> void { g_python_context->forward_error(QString::fromStdString(s)); });
}

PYBIND11_EMBEDDED_MODULE(gui, py_gui_api)
{
py_gui_api.def("get_selected_gate_ids", gui_api::get_selected_gate_ids, R"(
        Get the gate ids of currently selected gates in the graph view of the GUI.

        :returns: List of the ids of the currently selected gates.
        :rtype: list[int]
)");

py_gui_api.def("get_selected_net_ids", gui_api::get_selected_net_ids, R"(
        Get the net ids of currently selected nets in the graph view of the GUI.

        :returns: List of the ids of the currently selected nets.
        :rtype: list[int]
)");

py_gui_api.def("get_selected_module_ids", gui_api::get_selected_module_ids, R"(
        Get the module ids of currently selected modules in the graph view of the GUI.

        :returns: List of the ids of the currently selected modules.
        :rtype: list[int]
)");

py_gui_api.def("get_selected_item_ids", gui_api::get_selected_item_ids, R"(
        Get all item ids of the currently selected items in the graph view of the GUI.

        :returns: Tuple of lists of the currently selected items.
        :rtype: tuple(int, int, int)
)");

py_gui_api.def("get_selected_gates", gui_api::get_selected_gates, R"(
        Get the gates which are currently selected in the graph view of the GUI.

        :returns: List of currently selected gates.
        :rtype: list[hal_py.gate]
)");

py_gui_api.def("get_selected_nets", gui_api::get_selected_nets, R"(
        Get the nets which are currently selected in the graph view of the GUI.

        :returns: List of currently selected nets.
        :rtype: list[hal_py.net]
)");

py_gui_api.def("get_selected_modules", gui_api::get_selected_modules, R"(
       Get the modules which are currently selected in the graph view of the GUI.

       :returns: List of currently selected modules.
       :rtype: list[hal_py.module]
)");

py_gui_api.def("get_selected_items", gui_api::get_selected_items, R"(
       Get all selected items which are currently selected in the graph view of the GUI.

       :returns: Tuple of currently selected items.
       :rtype: tuple(hal_py.gate, hal_py.net, hal_py.module)
)");

/*
py_gui_api.def("print_selected_gates", gui_api::print_selected_gates, R"(
        Prints id and name of all selected gates to the python console.
)");

py_gui_api.def("print_selected_nets", gui_api::print_selected_nets, R"(
        Prints id and name of all selected nets to the python console.
)");

py_gui_api.def("print_selected_modules", gui_api::print_selected_modules, R"(
        Prints id and name of all selected modules to the python console.
)");

py_gui_api.def("print_selected_items", gui_api::print_selected_items, R"(
        Prints id and name of all selected gates, nets and modules to the python console.
)");
*/

py_gui_api.def("select_gate", py::overload_cast<u32, bool>(gui_api::select_gate), py::arg("gate_id"), py::arg("clear_current_selection") = true, R"(
       Select the gate with id 'gate_id' in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate with the id 'gate_id' will be added to the currently existing selection.

       :param int gate_id: The gate id of the gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
)");

py_gui_api.def("select_gate", py::overload_cast<const std::shared_ptr<gate>&, bool>(gui_api::select_gate), py::arg("gate"), py::arg("clear_current_selection") = true, R"(
       Select the gate in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate will be added to the currently existing selection.

       :param hal_py.gate gate: The gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
)");

py_gui_api.def("select_gate", py::overload_cast<const std::vector<u32>&, bool>(gui_api::select_gate), py::arg("gate_ids"), py::arg("clear_current_selection") = true, R"(
       Select the gates with the ids in list 'gate_ids' in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate with the id 'gate_id' will be added to the currently existing selection.

       :param list[int] gate_ids: List of gate ids of the gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
)");

py_gui_api.def("select_gate", py::overload_cast<const std::vector<std::shared_ptr<gate>>&, bool>(gui_api::select_gate), py::arg("gates"), py::arg("clear_current_selection") = true, R"(
       Select the gates in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates will be added to the currently existing selection.

       :param list[hal_py.gate] gates: The gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
)");

py_gui_api.def("select_net", py::overload_cast<u32, bool>(gui_api::select_net), py::arg("net_id"), py::arg("clear_current_selection") = true, R"(
       Select the net with id 'net_id' in the graph view of the GUI.
       If 'clear_current_selection' is false, the net with the id 'net_id' will be added to the currently existing selection.

       :param int net_id: The net id of the net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
)");

py_gui_api.def("select_net", py::overload_cast<const std::shared_ptr<net>&, bool>(gui_api::select_net), py::arg("net"), py::arg("clear_current_selection") = true, R"(
       Select the net in the graph view of the GUI.
       If 'clear_current_selection' is false, the net will be added to the currently existing selection.

       :param hal_py.net net: The net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
)");

py_gui_api.def("select_net", py::overload_cast<const std::vector<u32>&, bool>(gui_api::select_net), py::arg("net_ids"), py::arg("clear_current_selection") = true, R"(
       Select the nets with the ids in list 'net_ids' in the graph view of the GUI.
       If 'clear_current_selection' is false, the net with the id 'net_id' will be added to the currently existing selection.

       :param list[int] net_ids: List of net ids of the nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
)");

py_gui_api.def("select_net", py::overload_cast<const std::vector<std::shared_ptr<net>>&, bool>(gui_api::select_net), py::arg("nets"), py::arg("clear_current_selection") = true, R"(
       Select the nets in the graph view of the GUI.
       If 'clear_current_selection' is false, the nets will be added to the currently existing selection.

       :param list[hal_py.net] nets: The nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
)");

py_gui_api.def("select_module", py::overload_cast<u32, bool>(gui_api::select_module), py::arg("module_id"), py::arg("clear_current_selection") = true, R"(
       Select the module with id 'module_id' in the graph view of the GUI.
       If 'clear_current_selection' is false, the module with the id 'module_id' will be added to the currently existing selection.

       :param int module_id: The module id of the module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
)");

py_gui_api.def("select_module", py::overload_cast<const std::shared_ptr<module>&, bool>(gui_api::select_module), py::arg("module"), py::arg("clear_current_selection") = true, R"(
       Select the module in the graph view of the GUI.
       If 'clear_current_selection' is false, the module will be added to the currently existing selection.

       :param hal_py.module module: The module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
)");

py_gui_api.def("select_module", py::overload_cast<const std::vector<u32>&, bool>(gui_api::select_module), py::arg("module_ids"), py::arg("clear_current_selection") = true, R"(
       Select the modules with the ids in list 'module_ids' in the graph view of the GUI.
       If 'clear_current_selection' is false, the module with the id 'module_id' will be added to the currently existing selection.

       :param list[int] module_ids: List of module ids of the modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
)");

py_gui_api.def("select_module", py::overload_cast<const std::vector<std::shared_ptr<module>>&, bool>(gui_api::select_module), py::arg("modules"), py::arg("clear_current_selection") = true, R"(
       Select the modules in the graph view of the GUI.
       If 'clear_current_selection' is false, the modules will be added to the currently existing selection.

       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
)");

py_gui_api.def("select", py::overload_cast<const std::shared_ptr<gate>&, bool>(gui_api::select), py::arg("gate"), py::arg("clear_current_selection") = true, R"(
       Select the gate in the graph view of the GUI.
       If 'clear_current_selection' is false, the gate will be added to the currently existing selection.

       :param hal_py.gate gate: The gate to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gate.
)");

py_gui_api.def("select", py::overload_cast<const std::shared_ptr<net>&, bool>(gui_api::select), py::arg("net"), py::arg("clear_current_selection") = true, R"(
       Select the net in the graph view of the GUI.
       If 'clear_current_selection' is false, the net will be added to the currently existing selection.

       :param hal_py.net net: The net to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the net.
)");

py_gui_api.def("select", py::overload_cast<const std::shared_ptr<module>&, bool>(gui_api::select), py::arg("module"), py::arg("clear_current_selection") = true, R"(
       Select the module in the graph view of the GUI.
       If 'clear_current_selection' is false, the module will be added to the currently existing selection.

       :param hal_py.module module: The module to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the module.
)");

py_gui_api.def("select", py::overload_cast<const std::vector<std::shared_ptr<gate>>&, bool>(gui_api::select), py::arg("gates"), py::arg("clear_current_selection") = true, R"(
       Select the gates in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates will be added to the currently existing selection.

       :param list[hal_py.gate] gates: The gates to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the gates.
)");

py_gui_api.def("select", py::overload_cast<const std::vector<std::shared_ptr<net>>&, bool>(gui_api::select), py::arg("nets"), py::arg("clear_current_selection") = true, R"(
       Select the nets in the graph view of the GUI.
       If 'clear_current_selection' is false, the nets will be added to the currently existing selection.

       :param list[hal_py.net] nets: The nets to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the nets.
)");

py_gui_api.def("select", py::overload_cast<const std::vector<std::shared_ptr<module>>&, bool>(gui_api::select), py::arg("modules"), py::arg("clear_current_selection") = true, R"(
       Select the modules in the graph view of the GUI.
       If 'clear_current_selection' is false, the modules will be added to the currently existing selection.

       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
)");

py_gui_api.def("select", py::overload_cast<const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&, bool>(gui_api::select), py::arg("gate_ids"), py::arg("net_ids"), py::arg("module_ids"), py::arg("clear_current_selection") = true, R"(
       Select the gates, nets and modules with the passed ids in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates, nets and modules will be added to the currently existing selection.

       :param list[hal_py.gate] gates: The ids of the gates to be selected.
       :param list[hal_py.net] nets: The ids of the nets to be selected.
       :param list[hal_py.module] modules: The ids of the modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
)");

py_gui_api.def("select", py::overload_cast<const std::vector<std::shared_ptr<gate>>&, const std::vector<std::shared_ptr<net>>&, const std::vector<std::shared_ptr<module>>&, bool>(gui_api::select), py::arg("gates"), py::arg("nets"), py::arg("modules"), py::arg("clear_current_selection") = true, R"(
       Select the gates, nets and modules in the graph view of the GUI.
       If 'clear_current_selection' is false, the gates, nets and modules will be added to the currently existing selection.

       :param list[hal_py.gate] gates: The gates to be selected.
       :param list[hal_py.net] nets: The nets to be selected.
       :param list[hal_py.module] modules: The modules to be selected.
       :param bool clear_current_selection: Determines if the previous selection gets cleared before the selection of the modules.
)");

py_gui_api.def("deselect_gate", py::overload_cast<u32>(gui_api::deselect_gate), py::arg("gate_id"), R"(
       Deselect the gate with id 'gate_id' in the graph view of the GUI.

       :param int gate_id: The gate id of the gate to be selected.
)");

py_gui_api.def("deselect_gate", py::overload_cast<const std::shared_ptr<gate>&>(gui_api::deselect_gate), py::arg("gate"), R"(
       Deselect the gate in the graph view of the GUI.

       :param hal_py.gate gate: The gate to be deselected.
)");

py_gui_api.def("deselect_gate", py::overload_cast<const std::vector<u32>&>(gui_api::deselect_gate), py::arg("gate_ids"), R"(
       Deselect the gates with the ids in list 'gate_ids' in the graph view of the GUI.

       :param list[int] gate_ids: List of gate ids of the gates to be deselected.
)");

py_gui_api.def("deselect_gate", py::overload_cast<const std::vector<std::shared_ptr<gate>>&>(gui_api::deselect_gate), py::arg("gates"), R"(
       Deselect the gates in the graph view of the GUI.

       :param list[hal_py.gate] gates: The gates to be deselected.
)");

py_gui_api.def("deselect_net", py::overload_cast<u32>(gui_api::deselect_net), py::arg("net_id"), R"(
       Deselect the net with id 'net_id' in the graph view of the GUI.

       :param int net_id: The net id of the net to be selected.
)");

py_gui_api.def("deselect_net", py::overload_cast<const std::shared_ptr<net>&>(gui_api::deselect_net), py::arg("net"), R"(
       Deselect the net in the graph view of the GUI.

       :param hal_py.net net: The net to be deselected.
)");

py_gui_api.def("deselect_net", py::overload_cast<const std::vector<u32>&>(gui_api::deselect_net), py::arg("net_ids"), R"(
       Deselect the nets with the ids in list 'net_ids' in the graph view of the GUI.

       :param list[int] net_ids: List of net ids of the nets to be deselected.
)");

py_gui_api.def("deselect_net", py::overload_cast<const std::vector<std::shared_ptr<net>>&>(gui_api::deselect_net), py::arg("nets"), R"(
       Deselect the nets in the graph view of the GUI.

       :param list[hal_py.net] nets: The nets to be deselected.
)");

py_gui_api.def("deselect_module", py::overload_cast<u32>(gui_api::deselect_module), py::arg("module_id"), R"(
       Deselect the module with id 'module_id' in the graph view of the GUI.

       :param int module_id: The module id of the module to be selected.
)");

py_gui_api.def("deselect_module", py::overload_cast<const std::shared_ptr<module>&>(gui_api::deselect_module), py::arg("module"), R"(
       Deselect the module in the graph view of the GUI.

       :param hal_py.module module: The module to be deselected.
)");

py_gui_api.def("deselect_module", py::overload_cast<const std::vector<u32>&>(gui_api::deselect_module), py::arg("module_ids"), R"(
       Deselect the modules with the ids in list 'module_ids' in the graph view of the GUI.

       :param list[int] module_ids: List of module ids of the modules to be deselected.
)");

py_gui_api.def("deselect_module", py::overload_cast<const std::vector<std::shared_ptr<module>>&>(gui_api::deselect_module), py::arg("modules"), R"(
       Deselect the modules in the graph view of the GUI.

       :param list[hal_py.module] modules: The modules to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::shared_ptr<gate>&>(gui_api::deselect), py::arg("gate"), R"(
       Deselect the gate in the graph view of the GUI.

       :param hal_py.gate gate: The gate to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::shared_ptr<net>&>(gui_api::deselect), py::arg("net"), R"(
       Deselect the net in the graph view of the GUI.

       :param hal_py.net net: The net to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::shared_ptr<module>&>(gui_api::deselect), py::arg("module"), R"(
       Deselect the module in the graph view of the GUI.

       :param hal_py.module module: The module to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::vector<std::shared_ptr<gate>>&>(gui_api::deselect), py::arg("gates"), R"(
       Deselect the gates in the graph view of the GUI.

       :param list[hal_py.gate] gates: The gates to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::vector<std::shared_ptr<net>>&>(gui_api::deselect), py::arg("nets"), R"(
       Deselect the nets in the graph view of the GUI.

       :param list[hal_py.net] nets: The nets to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::vector<std::shared_ptr<module>>&>(gui_api::deselect), py::arg("modules"), R"(
       Deselect the modules in the graph view of the GUI.

       :param list[hal_py.module] modules: The modules to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&>(gui_api::deselect), py::arg("gate_ids"), py::arg("net_ids"), py::arg("module_ids"), R"(
       Deselect the gates, nets and modules with the passed ids in the graph view of the GUI.

       :param list[hal_py.gate] gates: The ids of the gates to be deselected.
       :param list[hal_py.net] nets: The ids of the nets to be deselected.
       :param list[hal_py.module] modules: The ids of the modules to be deselected.
)");

py_gui_api.def("deselect", py::overload_cast<const std::vector<std::shared_ptr<gate>>&, const std::vector<std::shared_ptr<net>>&, const std::vector<std::shared_ptr<module>>&>(gui_api::deselect), py::arg("gates"), py::arg("nets"), py::arg("modules"), R"(
       Deselect the gates, nets and modules in the graph view of the GUI.

       :param list[hal_py.gate] gates: The gates to be deselected.
       :param list[hal_py.net] nets: The nets to be deselected.
       :param list[hal_py.module] modules: The modules to be deselected.
)");

py_gui_api.def("deselect_all_items", py::overload_cast<>(gui_api::deselect_all_items), R"(
       Deselect all gates, nets and modules in the graph view of the GUI.
)");
}

python_context::python_context()
{
    Q_UNUSED(m_sender)
    m_context = nullptr;
    py::initialize_interpreter();
    init_python();
}

python_context::~python_context()
{
    close_python();
    py::finalize_interpreter();
}

void python_context::set_console(python_console* c)
{
    m_console = c;
}

void python_context::initialize_context(py::dict* context)
{
    py::exec("import __main__\n"
             "import io, sys\n"
             "from console import reset\n"
             "from console import clear\n"
             "class StdOutCatcher(io.TextIOBase):\n"
             "    def __init__(self):\n"
             "        pass\n"
             "    def write(self, stuff):\n"
             "        import console\n"
             "        console.redirector.write_stdout(stuff)\n"
             "class StdErrCatcher(io.TextIOBase):\n"
             "    def __init__(self):\n"
             "        pass\n"
             "    def write(self, stuff):\n"
             "        import console\n"
             "        console.redirector.write_stderr(stuff)\n"
             "sys.stdout = StdOutCatcher()\n"
             "sys.__stdout__ = sys.stdout\n"
             "sys.stderr = StdErrCatcher()\n"
             "sys.__stderr__ = sys.stderr\n"
             "sys.path.append('"
                 + core_utils::get_library_directory().string()
                 + "')\n"
                   "import hal_py\n"
             "import gui\n",
             *context,
             *context);

    (*context)["netlist"] = g_netlist;
}

void python_context::init_python()
{
    //    using namespace py::literals;

    new py::dict();
    m_context = new py::dict(**py::globals());

    initialize_context(m_context);
    (*m_context)["console"] = py::module::import("console");
}

void python_context::close_python()
{
    delete m_context;
    m_context = nullptr;
}

void python_context::interpret(const QString& input, bool multiple_expressions)
{
    if (input.isEmpty())
    {
        return;
    }

    if (input == "quit()")
    {
        forward_error("quit() cannot be used in this interpreter. Use console.reset() to restart it.\n");
        return;
    }

    if (input == "help()")
    {
        forward_error("help() cannot be used in this interpreter.\n");
        return;
    }

    if (input == "license()")
    {
        forward_error("license() cannot be used in this interpreter.\n");
        return;
    }
    log_info("python", "Python console execute: \"{}\".", input.toStdString());
    try
    {
        pybind11::object rc;
        if (multiple_expressions)
        {
            rc = py::eval<py::eval_statements>(input.toStdString(), *m_context, *m_context);
        }
        else
        {
            rc = py::eval<py::eval_single_statement>(input.toStdString(), *m_context, *m_context);
        }
        if (!rc.is_none())
        {
            forward_stdout(QString::fromStdString(py::str(rc).cast<std::string>()));
        }
        handle_reset();
    }
    catch (py::error_already_set& e)
    {
        forward_error(QString::fromStdString(std::string(e.what())));
        e.restore();
        PyErr_Clear();
    }
    catch (std::exception& e)
    {
        forward_error(QString::fromStdString(std::string(e.what())));
    }
}

void python_context::interpret_script(const QString& input)
{
    // py::print(py::globals());
    py::dict tmp_context(py::globals());
    initialize_context(&tmp_context);

    forward_stdout("\n");
    forward_stdout("<Execute Python Editor content>");
    forward_stdout("\n");
    log_info("python", "Python editor execute script:\n{}\n", input.toStdString());
    try
    {
        py::eval<py::eval_statements>(input.toStdString(), tmp_context, tmp_context);
    }
    catch (py::error_already_set& e)
    {
        forward_error(QString::fromStdString(std::string(e.what()) + "\n"));
        e.restore();
        PyErr_Clear();
    }
    catch (std::exception& e)
    {
        forward_error(QString::fromStdString(std::string(e.what()) + "#\n"));
    }

    if (m_console)
    {
        m_console->display_prompt();
    }
}

void python_context::forward_stdout(const QString& output)
{
    if (output != "\n")
    {
        log_info("python", "{}", core_utils::rtrim(output.toStdString(), "\r\n"));
    }
    if (m_console)
    {
        m_console->handle_stdout(output);
    }
}

void python_context::forward_error(const QString& output)
{
    log_error("python", "{}", output.toStdString());
    if (m_console)
    {
        m_console->handle_error(output);
    }
}

void python_context::forward_clear()
{
    if (m_console)
    {
        m_console->clear();
    }
}

std::vector<std::tuple<std::string, std::string>> python_context::complete(const QString& text, bool use_console_context)
{
    std::vector<std::tuple<std::string, std::string>> ret_val;
    try
    {
        auto namespaces = py::list();
        if (use_console_context)
        {
            namespaces.append(*m_context);
            namespaces.append(*m_context);
        }
        else
        {
            py::dict tmp_context(py::globals());
            initialize_context(&tmp_context);
            namespaces.append(tmp_context);
            namespaces.append(tmp_context);
        }
        auto jedi   = py::module::import("jedi");
        auto script = jedi.attr("Interpreter")(text.toStdString(), namespaces);
        auto list   = script.attr("completions")();

        for (const auto& entry : list)
        {
            auto a = entry.attr("name_with_symbols").cast<std::string>();
            auto b = entry.attr("complete").cast<std::string>();
            ret_val.emplace_back(a, b);
        }
    }
    catch (py::error_already_set& e)
    {
        forward_error(QString::fromStdString(std::string(e.what()) + "\n"));
        e.restore();
        PyErr_Clear();
    }
    return ret_val;
}

int python_context::check_complete_statement(const QString& text)
{
    node* n;
    perrdetail e;

    n = PyParser_ParseString(text.toStdString().c_str(), &_PyParser_Grammar, Py_file_input, &e);
    if (n == NULL)
    {
        if (e.error == E_EOF)
        {
            return 0;
        }
        return -1;
    }

    PyNode_Free(n);
    return 1;
}

void python_context::handle_reset()
{
    if (m_trigger_reset)
    {
        close_python();
        init_python();
        forward_clear();
        m_trigger_reset = false;
    }
}

void python_context::forward_reset()
{
    m_trigger_reset = true;
}

void python_context::update_netlist()
{
    (*m_context)["netlist"] = g_netlist;
}
