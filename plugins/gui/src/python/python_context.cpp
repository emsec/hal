#include "gui/python/python_context.h"

#include "gui/gui_globals.h"
#include "gui/python/python_context_subscriber.h"
#include "gui/python/python_thread.h"
#include "gui/python/python_editor.h"
#include "gui/module_dialog/gate_dialog.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/graph_widget/layout_locker.h"
#include "hal_core/python_bindings/python_bindings.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <QDir>
#include <QDebug>
#include <QInputDialog>
#include <QApplication>
#include <QFileDialog>
#include <fstream>
#include <gui/python/python_context.h>

// Following is needed for PythonContext::checkCompleteStatement
#include "hal_config.h"

#include <Python.h>
#include <compile.h>
#include <errcode.h>

#if PY_VERSION_HEX < 0x030900a0 // Python 3.9.0

#include <grammar.h>
#include <node.h>
#include <parsetok.h>
extern grammar _PyParser_Grammar;

#endif

namespace hal
{
    PythonContext::PythonContext(QObject *parent)
        : QObject(parent), mContext(nullptr), mSender(nullptr), mTriggerReset(false), mThread(nullptr),
          mThreadAborted(false), mInterpreterCaller(nullptr)
    {
        py::initialize_interpreter();
        initPython();

        // The Python interpreter is not thread safe, so it implements an internal
        // Global Interpreter Lock that means only one thread can actively be
        // executing Python code at a time (though the interpreter can switch between
        // threads, for example if one thread is blocked for I/O).
        // Take care to always handle the GIL correctly, or you will cause deadlocks,
        // weird issues with threads that are running but Python believes that they
        // don't exist, etc.
        // DO NOT EVER run Python code while not holding the GIL!
        // Wherever possible, use the PyGILState_Ensure / PyGILState_Release API to
        // acquire/release the GIL.

        // We must release the GIL, otherwise any spawning thread will deadlock.
        mMainThreadState = PyEval_SaveThread();

    }

    PythonContext::~PythonContext()
    {
        PyEval_RestoreThread(mMainThreadState);

        closePython();
        py::finalize_interpreter();
    }

    void PythonContext::setConsole(PythonConsole* c)
    {
        mConsole = c; // may be nullptr
        if (c)
        {
            connect(mConsole,&PythonConsole::inputReceived,this,&PythonContext::handleConsoleInputReceived);
        }
    }

    PythonThread* PythonContext::pythonThread() const
    {
        return mThread;
    }

    void PythonContext::abortThread()
    {
        if (!mThread) return;
        mThreadAborted = true;
        mThread->interrupt();
    }

    void PythonContext::abortThreadAndWait()
    {
        if (!mThread) return;
        abortThread();
        mThread->wait();
    }

    void PythonContext::initializeContext(py::dict* context)
    {
        // GIL must be held

        std::string command = "import __main__\n"
                              "import io, sys, threading, builtins\n";
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
                     "        if threading.current_thread() is threading.main_thread():\n"
                     "            console.redirector.write_stdout(stuff)\n"
                     "        else:\n"
                     "            console.redirector.thread_stdout(stuff)\n"
                     "class StdErrCatcher(io.TextIOBase):\n"
                     "    def __init__(self):\n"
                     "        pass\n"
                     "    def write(self, stuff):\n"
                     "        from hal_gui import console\n"
                     "        if threading.current_thread() is threading.main_thread():\n"
                     "            console.redirector.write_stderr(stuff)\n"
                     "        else:\n"
                     "            console.redirector.thread_stderr(stuff)\n"
                     "def redirect_input(prompt=\"Please enter input:\"):\n"
                     "    if threading.current_thread() is threading.main_thread():\n"
                     "        return \"input in main thread not supported\"\n"
                     "    else:\n"
                     "        return console.redirector.thread_stdin(prompt)\n"
                     "sys.stdout = StdOutCatcher()\n"
                     "sys.__stdout__ = sys.stdout\n"
                     "sys.stderr = StdErrCatcher()\n"
                     "sys.__stderr__ = sys.stderr\n"
                     "builtins.input = redirect_input\n"
                     "builtins.raw_input = redirect_input\n"
                     "import hal_py\n";

        py::exec(command, *context, *context);

        (*context)["netlist"] = gNetlistOwner;    // assign the shared_ptr here, not the raw ptr

        if (gGuiApi)
        {
            (*context)["gui"] = gGuiApi;
        }
    }

    void PythonContext::initializeScript(py::dict *context)
    {
        // GIL must be held

        std::string command = "import __main__\n"
                              "import io, sys, threading\n"
                              "from hal_gui.console import reset\n"
                              "from hal_gui.console import clear\n"
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
        // GIL must be held

        //    using namespace py::literals;

        new py::dict();
        mContext = new py::dict(**py::globals());

        initializeContext(mContext);
        (*mContext)["console"] = py::module::import("hal_gui.console");
        (*mContext)["hal_gui"] = py::module::import("hal_gui");
    }

    void PythonContext::closePython()
    {
        // GIL must be held
        delete mContext;
        mContext = nullptr;
    }

    void PythonContext::interpretBackground(QObject* caller, const QString& input, bool multiple_expressions)
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
        
        if (mThread)
        {
            forwardError("python thread already running. Please retry after other thread finished.\n");
            return;
        }
        log_info("python", "Python console execute: \"{}\".", input.toStdString());

        mInterpreterCaller = caller;
        startThread(input,!multiple_expressions);
    }

    void PythonContext::startThread(const QString& input, bool singleStatement)
    {
        mThread = new PythonThread(input,singleStatement,this);
        connect(mThread,&QThread::finished,this,&PythonContext::handleThreadFinished);
        connect(mThread,&PythonThread::stdOutput,this,&PythonContext::handleScriptOutput);
        connect(mThread,&PythonThread::stdError,this,&PythonContext::handleScriptError);
        connect(mThread,&PythonThread::requireInput,this,&PythonContext::handleInputRequired);
        if (mConsole) mConsole->setReadOnly(true);
        mLayoutLocker = new LayoutLocker;
        mThread->start();
    }

    void PythonContext::interpretForeground(const QString &input)
    {
        if (input.isEmpty())
        {
            return;
        }

        log_info("python", "Python console execute: \"{}\".", input.toStdString());

        // since we've released the GIL in the constructor, re-acquire it here before
        // running some Python code on the main thread
        PyGILState_STATE state = PyGILState_Ensure();
        //PyEval_RestoreThread(mMainThreadState);

        // TODO should the console also be moved to threads? Maybe actually catch Ctrl+C there
        // as a method to interrupt? Currently you can hang the GUI by running an endless loop
        // from the console.

        try
        {
            pybind11::object rc;
            rc = py::eval<py::eval_single_statement>(input.toStdString(), *mContext, *mContext);
            if (!rc.is_none())
            {
                forwardStdout(QString::fromStdString(py::str(rc).cast<std::string>()));
            }
            handleReset();
        }
        catch (py::error_already_set& e)
        {
            qDebug() << "interpret error set";
            forwardError(QString::fromStdString(std::string(e.what())));
            e.restore();
            PyErr_Clear();
        }
        catch (std::exception& e)
        {
            qDebug() << "interpret exception";
            forwardError(QString::fromStdString(std::string(e.what())));
        }

        // make sure we release the GIL, otherwise we interfere with threading
        PyGILState_Release(state);
        //mMainThreadState = PyEval_SaveThread();

    }

    void PythonContext::interpretScript(QObject* caller, const QString& input)
    {
        if (mThread)
        {
            log_warning("python", "Not executed, python script already running");
            return;
        }

        //log_info("python", "Python editor execute script:\n{}\n", input.toStdString());
#ifdef HAL_STUDY
        log_info("UserStudy", "Python editor execute script:\n{}\n", input.toStdString());
#endif
        forwardStdout("\n");
        forwardStdout("<Execute Python Editor content>");
        forwardStdout("\n");

        mInterpreterCaller = caller;
        startThread(input,false);
    }

    void PythonContext::handleScriptOutput(const QString& txt)
    {
        if (!txt.isEmpty())
            forwardStdout(txt);
    }

    void PythonContext::handleScriptError(const QString& txt)
    {
        if (!txt.isEmpty())
            forwardError(txt);
    }

    void PythonContext::handleInputRequired(int type, const QString& prompt, const QVariant &defaultValue)
    {
        bool confirm;
        if (mThread && !mThread->stdoutBuffer().isEmpty())
            mConsole->handleStdout(mThread->flushStdout());

        switch (type) {
        case PythonThread::ConsoleInput:
            mConsole->handleStdout(prompt + "\n");
            mConsole->setInputMode(true);
            mConsole->displayPrompt();
            mConsole->setReadOnly(false);
            break;
        case PythonThread::StringInput:
        {
            QString userInput = QInputDialog::getText(qApp->activeWindow(), "Python Script Input", prompt, QLineEdit::Normal, defaultValue.toString(), &confirm);
            if (!confirm) userInput.clear();
            if (mThread) mThread->setInput(userInput);
            break;
        }
        case PythonThread::NumberInput:
        {
            int userInput = QInputDialog::getInt(qApp->activeWindow(), "Python Script Input", prompt, defaultValue.toInt());
            if (mThread) mThread->setInput(userInput);
            break;
        }
        case PythonThread::GateInput:
        {
            QSet<u32> gats;
            for (const Gate* g : gNetlist->get_gates()) gats.insert(g->get_id());
            if (gSelectionRelay->numberSelectedGates()) gSelectionRelay->clearAndUpdate();
            PythonGateSelectionReceiver* pgs = new PythonGateSelectionReceiver(mThread,this);
            GateDialog gd(gats, prompt, pgs, qApp->activeWindow());
            Gate* gatSelect = (gd.exec() == QDialog::Accepted)
                    ? gNetlist->get_gate_by_id(gd.selectedId())
                    : nullptr;
            if (!gd.pickerModeActivated() && mThread) mThread->setInput(QVariant::fromValue<void*>(gatSelect));
            break;
        }
        case PythonThread::ModuleInput:
        {
            QSet<u32> mods;
            if (gSelectionRelay->numberSelectedModules()) gSelectionRelay->clearAndUpdate();
            PythonModuleSelectionReceiver* pms = new PythonModuleSelectionReceiver(mThread,this);
            ModuleDialog md({}, prompt, pms, qApp->activeWindow());
            Module* modSelect = (md.exec() == QDialog::Accepted)
                    ? gNetlist->get_module_by_id(md.selectedId())
                    : nullptr;
            if (!md.pickerModeActivated() && mThread) mThread->setInput(QVariant::fromValue<void*>(modSelect));
            break;
        }
        case PythonThread::FilenameInput:
        {
            QString userInput = QFileDialog::getOpenFileName(qApp->activeWindow(),prompt,".",defaultValue.toString());
            if (mThread) mThread->setInput(userInput);
            break;
        }
        default:
            break;
        }
    }

    void PythonGateSelectionReceiver::handleGatesPicked(const QSet<u32>& gats)
    {
        Gate* gatSelect = gats.isEmpty()
                ? nullptr
                : gNetlist->get_gate_by_id(*gats.constBegin());
        if (mThread) mThread->setInput(QVariant::fromValue<void*>(gatSelect));
        this->deleteLater();
    }

    void PythonModuleSelectionReceiver::handleModulesPicked(const QSet<u32>& mods)
    {
        Module* modSelect = mods.isEmpty()
                ? nullptr
                : gNetlist->get_module_by_id(*mods.constBegin());
        if (mThread) mThread->setInput(QVariant::fromValue<void*>(modSelect));
        this->deleteLater();
    }

    void PythonContext::handleConsoleInputReceived(const QString& input)
    {
        mConsole->setInputMode(false);
        mConsole->setReadOnly(true);
        mThread->setInput(input);
    }

    void PythonContext::handleThreadFinished()
    {
        if (mConsole) mConsole->setReadOnly(false);
        PythonEditor* calledFromEditor = dynamic_cast<PythonEditor*>(mInterpreterCaller);
        QString errmsg;
        if (!mThread)
        {
            mThreadAborted = false;
            return;
        }
        if (!mThreadAborted)
        {
            errmsg = mThread->errorMessage();
            if (!mThread->stdoutBuffer().isEmpty())
                forwardStdout(mThread->stdoutBuffer());
            if (!calledFromEditor && !mThread->result().isEmpty() && errmsg.isEmpty())
                forwardStdout(mThread->result());
        }
        else
        {
            forwardError("\nPython thread aborted\n");
            mThreadAborted = false;
        }

        mThread->deleteLater();
        mThread = 0;
        delete mLayoutLocker;

        if (!errmsg.isEmpty())
            forwardError(errmsg);

        if (calledFromEditor)
        {
            calledFromEditor->handleThreadFinished();
            if (mTriggerReset)
            {
                forwardError("\nreset() can only be used in the Python console!\n");
                mTriggerReset = false;
            }
            if (mTriggerClear)
            {
                forwardError("\nclear() can only be used in the Python console!\n");
                mTriggerClear = false;
            }
        }
        else if (mConsole)
        {
            handleReset();
            handleClear();
            mConsole->handleThreadFinished();
        }

        if (mConsole)
        {
            mConsole->setInputMode(PythonConsole::Standard);
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

    std::vector<std::tuple<std::string, std::string>> PythonContext::complete(const QString& text, bool use_console_context)
    {
        PyGILState_STATE state = PyGILState_Ensure();

        std::vector<std::tuple<std::string, std::string>> ret_val;
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
                py::dict tmp_context = py::globals();
                initializeContext(&tmp_context);
                namespaces.append(tmp_context);
                namespaces.append(tmp_context);
            }
            auto jedi   = py::module::import("jedi");
            py::object script = jedi.attr("Interpreter")(text.toStdString(), namespaces);
            py::object list;
            if (py::hasattr(script,"complete"))
                list = script.attr("complete")();
            else if (py::hasattr(script,"completions"))
                list   = script.attr("completions")();
            else
                log_warning("python", "Jedi autocompletion failed, neither complete() nor completions() found.");

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

        PyGILState_Release(state);

        return ret_val;
    }

    int PythonContext::checkCompleteStatement(const QString& text)
    {

        PyGILState_STATE state = PyGILState_Ensure();
        
        #if PY_VERSION_HEX < 0x030900a0 // Python 3.9.0
        // PEG not yet available, use PyParser

        node* n;
        perrdetail e;

        n = PyParser_ParseString(text.toStdString().c_str(), &_PyParser_Grammar, Py_file_input, &e);
        if (n == NULL)
        {
            if (e.error == E_EOF)
            {
                PyGILState_Release(state);
                return 0;
            }
            PyGILState_Release(state);
            return -1;
        }

        PyNode_Free(n);
        PyGILState_Release(state);
        return 1;

        #else

        // attempt to parse Python expression into AST using PEG
        PyCompilerFlags flags {PyCF_ONLY_AST, PY_MINOR_VERSION};
        PyObject* o = Py_CompileStringExFlags(text.toStdString().c_str(), "stdin", Py_file_input, &flags, 0);
        // if parsing failed (-> not a complete statement), nullptr is returned
        // (apparently no need to PyObject_Free(o) here)
        PyGILState_Release(state);
        return o != nullptr;

        #endif
    }

    void PythonContext::handleReset()
    {
        if (mTriggerReset)
        {
            PyGILState_STATE state = PyGILState_Ensure();
            closePython();
            initPython();
            PyGILState_Release(state);
            scheduleClear();
            mTriggerReset = false;
        }
    }

    void PythonContext::handleClear()
    {
        if (mTriggerClear)
        {
            if (mConsole)
            {
                mConsole->clear();
            }
            mTriggerClear = false;
        }
    }

    void PythonContext::scheduleReset()
    {
        mTriggerReset = true;
    }

    void PythonContext::scheduleClear()
    {
        mTriggerClear = true;
    }

    void PythonContext::updateNetlist()
    {
        PyGILState_STATE state = PyGILState_Ensure();
        (*mContext)["netlist"] = gNetlistOwner;    // assign the shared_ptr here, not the raw ptr
        PyGILState_Release(state);
    }

}    // namespace hal
