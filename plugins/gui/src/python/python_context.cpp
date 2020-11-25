#include "gui/python/python_context.h"

#include "gui/gui_globals.h"
#include "gui/python/python_context_subscriber.h"
#include "hal_core/python_bindings/python_bindings.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <QDir>
#include <fstream>
#include <gui/python/python_context.h>

// Following is needed for PythonContext::checkCompleteStatement
#include "hal_config.h"

#include <Python.h>
#include <compile.h>
#include <errcode.h>
#include <grammar.h>
#include <node.h>
#include <parsetok.h>

extern grammar _PyParser_Grammar;

namespace hal
{
    PythonContext::PythonContext()
    {
        Q_UNUSED(mSender)
        mContext = nullptr;
        py::initialize_interpreter();
        initPython();
    }

    PythonContext::~PythonContext()
    {
        closePython();
        py::finalize_interpreter();
    }

    void PythonContext::setConsole(PythonConsole* c)
    {
        mConsole = c;
    }

    void PythonContext::initializeContext(py::dict* context)
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

        (*context)["netlist"] = gNetlistOwner;    // assign the shared_ptr here, not the raw ptr

        if (gGuiApi)
        {
            (*context)["gui"] = gGuiApi;
        }
    }

    void PythonContext::initPython()
    {
        //    using namespace py::literals;

        new py::dict();
        mContext = new py::dict(**py::globals());

        initializeContext(mContext);
        (*mContext)["console"] = py::module::import("hal_gui.console");
        (*mContext)["hal_gui"] = py::module::import("hal_gui");
    }

    void PythonContext::closePython()
    {
        delete mContext;
        mContext = nullptr;
    }

    void PythonContext::interpret(const QString& input, bool multiple_expressions)
    {
        if (input.isEmpty())
        {
            return;
        }

        if (input == "quit()")
        {
            forwardError("quit() cannot be used in this interpreter. Use console.reset() to restart it.\n");
            return;
        }

        if (input == "help()")
        {
            forwardError("help() cannot be used in this interpreter.\n");
            return;
        }

        if (input == "license()")
        {
            forwardError("license() cannot be used in this interpreter.\n");
            return;
        }
        log_info("python", "Python console execute: \"{}\".", input.toStdString());
        try
        {
            pybind11::object rc;
            if (multiple_expressions)
            {
                rc = py::eval<py::eval_statements>(input.toStdString(), *mContext, *mContext);
            }
            else
            {
                rc = py::eval<py::eval_single_statement>(input.toStdString(), *mContext, *mContext);
            }
            if (!rc.is_none())
            {
                forwardStdout(QString::fromStdString(py::str(rc).cast<std::string>()));
            }
            handleReset();
        }
        catch (py::error_already_set& e)
        {
            forwardError(QString::fromStdString(std::string(e.what())));
            e.restore();
            PyErr_Clear();
        }
        catch (std::exception& e)
        {
            forwardError(QString::fromStdString(std::string(e.what())));
        }
    }

    void PythonContext::interpretScript(const QString& input)
    {
        // py::print(py::globals());
        py::dict tmp_context(py::globals());
        initializeContext(&tmp_context);

        //log_info("python", "Python editor execute script:\n{}\n", input.toStdString());
#ifdef HAL_STUDY
        log_info("UserStudy", "Python editor execute script:\n{}\n", input.toStdString());
#endif
        forwardStdout("\n");
        forwardStdout("<Execute Python Editor content>");
        forwardStdout("\n");
        try
        {
            py::eval<py::eval_statements>(input.toStdString(), tmp_context, tmp_context);
        }
        catch (py::error_already_set& e)
        {
            forwardError(QString::fromStdString(std::string(e.what()) + "\n"));
            e.restore();
            PyErr_Clear();
        }
        catch (std::exception& e)
        {
            forwardError(QString::fromStdString(std::string(e.what()) + "#\n"));
        }

        if (mConsole)
        {
            mConsole->displayPrompt();
        }
    }

    void PythonContext::forwardStdout(const QString& output)
    {
        if (output != "\n")
        {
            log_info("python", "{}", utils::rtrim(output.toStdString(), "\r\n"));
        }
        if (mConsole)
        {
            mConsole->handleStdout(output);
        }
    }

    void PythonContext::forwardError(const QString& output)
    {
        log_error("python", "{}", output.toStdString());
        if (mConsole)
        {
            mConsole->handleError(output);
        }
    }

    void PythonContext::forwardClear()
    {
        if (mConsole)
        {
            mConsole->clear();
        }
    }

    std::vector<std::tuple<std::string, std::string>> PythonContext::complete(const QString& text, bool use_console_context)
    {
        std::vector<std::tuple<std::string, std::string>> ret_val;
        py::dict tmp_context;
        try
        {
            auto namespaces = py::list();
            if (use_console_context)
            {
                namespaces.append(*mContext);
                namespaces.append(*mContext);
            }
            else
            {
                tmp_context = py::globals();
                initializeContext(&tmp_context);
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
            forwardError(QString::fromStdString(std::string(e.what()) + "\n"));
            e.restore();
            PyErr_Clear();
        }

        return ret_val;
    }

    int PythonContext::checkCompleteStatement(const QString& text)
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

    void PythonContext::handleReset()
    {
        if (mTriggerReset)
        {
            closePython();
            initPython();
            forwardClear();
            mTriggerReset = false;
        }
    }

    void PythonContext::forwardReset()
    {
        mTriggerReset = true;
    }

    void PythonContext::updateNetlist()
    {
        (*mContext)["netlist"] = gNetlistOwner;    // assign the shared_ptr here, not the raw ptr
    }
}    // namespace hal
