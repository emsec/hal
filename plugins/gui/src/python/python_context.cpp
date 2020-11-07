#include "gui/python/python_context.h"

#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "gui/gui_globals.h"
#include "gui/python/python_context_subscriber.h"
#include "hal_core/python_bindings/python_bindings.h"

#include <QDir>
#include <fstream>
#include <gui/python/python_context.h>

// Following is needed for PythonContext::check_complete_statement
#include <Python.h>
#include <compile.h>
#include <errcode.h>
#include <grammar.h>
#include <node.h>
#include <parsetok.h>
#include "hal_config.h"

extern grammar _PyParser_Grammar;

namespace hal
{
    PythonContext::PythonContext()
    {
        Q_UNUSED(m_sender)
        m_context = nullptr;
        py::initialize_interpreter();
        init_python();
    }

    PythonContext::~PythonContext()
    {
        close_python();
        py::finalize_interpreter();
    }

    void PythonContext::set_console(PythonConsole* c)
    {
        m_console = c;
    }

    void PythonContext::initialize_context(py::dict* context)
    {
        std::string command = "import __main__\n"
                              "import io, sys\n";
        for (auto path : utils::get_plugin_directories())
        {
            command += "sys.path.append('" + path.string() + "')\n";
        }
        command += "sys.path.append('" + utils::get_library_directory().string()
                   + "')\n"
                     "from hal_gui.console import reset\n"
                     "from hal_gui.console import clear\n"
                     "class StdOutCatcher(io.TextIOBase):\n"
                     "    def __init__(self):\n"
                     "        pass\n"
                     "    def write(self, stuff):\n"
                     "        from hal_gui import console\n"
                     "        console.redirector.write_stdout(stuff)\n"
                     "class StdErrCatcher(io.TextIOBase):\n"
                     "    def __init__(self):\n"
                     "        pass\n"
                     "    def write(self, stuff):\n"
                     "        from hal_gui import console\n"
                     "        console.redirector.write_stderr(stuff)\n"
                     "sys.stdout = StdOutCatcher()\n"
                     "sys.__stdout__ = sys.stdout\n"
                     "sys.stderr = StdErrCatcher()\n"
                     "sys.__stderr__ = sys.stderr\n"
                     "import hal_py\n";

        py::exec(command, *context, *context);

        (*context)["netlist"] = RawPtrWrapper(g_netlist);

        if (g_gui_api)
            (*context)["gui"] = g_gui_api;
    }

    void PythonContext::init_python()
    {
        //    using namespace py::literals;

        new py::dict();
        m_context = new py::dict(**py::globals());

        initialize_context(m_context);
        (*m_context)["console"] = py::module::import("hal_gui.console");
        (*m_context)["hal_gui"] = py::module::import("hal_gui");
    }

    void PythonContext::close_python()
    {
        delete m_context;
        m_context = nullptr;
    }

    void PythonContext::interpret(const QString& input, bool multiple_expressions)
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

    void PythonContext::interpret_script(const QString& input)
    {
        // py::print(py::globals());
        py::dict tmp_context(py::globals());
        initialize_context(&tmp_context);

        forward_stdout("\n");
        forward_stdout("<Execute Python Editor content>");
        forward_stdout("\n");
        //log_info("python", "Python editor execute script:\n{}\n", input.toStdString());
#ifdef HAL_STUDY
        log_info("UserStudy", "Python editor execute script:\n{}\n", input.toStdString());
#endif
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

    void PythonContext::forward_stdout(const QString& output)
    {
        if (output != "\n")
        {
            log_info("python", "{}", utils::rtrim(output.toStdString(), "\r\n"));
        }
        if (m_console)
        {
            m_console->handle_stdout(output);
        }
    }

    void PythonContext::forward_error(const QString& output)
    {
        log_error("python", "{}", output.toStdString());
        if (m_console)
        {
            m_console->handle_error(output);
        }
    }

    void PythonContext::forward_clear()
    {
        if (m_console)
        {
            m_console->clear();
        }
    }

    std::vector<std::tuple<std::string, std::string>> PythonContext::complete(const QString& text, bool use_console_context)
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

    int PythonContext::check_complete_statement(const QString& text)
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

    void PythonContext::handle_reset()
    {
        if (m_trigger_reset)
        {
            close_python();
            init_python();
            forward_clear();
            m_trigger_reset = false;
        }
    }

    void PythonContext::forward_reset()
    {
        m_trigger_reset = true;
    }

    void PythonContext::update_netlist()
    {
        (*m_context)["netlist"] = RawPtrWrapper(g_netlist);
    }
}    // namespace hal
