#include <QDir>
#include <fstream>
#include <gui/python/python_context.h>

#include "python/python_context.h"

#include "core/log.h"
#include "core/utils.h"

#include "gui/python/python_context_suberscriber.h"
#include "gui_globals.h"

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
                   "import hal_py\n",
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
