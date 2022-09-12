#include "gui/python/python_thread.h"
#include "gui/python/python_context.h"
#include <QDebug>

#include <pyerrors.h>
#include <ceval.h>
#include <pystate.h>
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"

namespace hal {
    PythonThread::PythonThread(const QString& script, QObject* parent)
        : QThread(parent), mScript(script)
    {;}

    void PythonThread::run()
    {
        // decides it's our turn.
        PyGILState_STATE state = PyGILState_Ensure();
        py::dict tmp_context(py::globals());
        PythonContext::initializeScript(&tmp_context);

        // Capture the Python-internal thread ID, which is needed if we want to
        // interrupt the thread later
        pybind11::object rc = py::eval("threading.get_ident()", tmp_context, tmp_context);
        mThreadID = rc.cast<unsigned long>();

        try
        {
            py::eval<py::eval_statements>(mScript.toStdString(), tmp_context, tmp_context);
        }
        catch (py::error_already_set& e)
        {
            qDebug() << "AlreadySet";
            mErrorMessage = QString::fromStdString(std::string(e.what()) + "\n");
            e.restore();
            PyErr_Clear();
        }
        catch (std::exception& e)
        {
            qDebug() << "Exception";
            mErrorMessage = QString::fromStdString(std::string(e.what()) + "#\n");
        }

        qDebug() << "Execution completed";

        PyGILState_Release(state);

        qDebug() << "GIL released";
    }

    void PythonThread::handleStdout(const QString& output)
    {
        Q_EMIT stdOutput(output);
    }

    void PythonThread::handleError(const QString& output)
    {
        Q_EMIT stdError(output);
    }

    void PythonThread::clear()
    {;}

    void PythonThread::interrupt() {
        qDebug() << "about to terminate thread..." << mThreadID;
        PyGILState_STATE state = PyGILState_Ensure();
        // We interrupt the thread by forcibly injecting an exception
        // (namely the KeyboardInterrupt exception, but any other one works as well)
        int nThreads = PyThreadState_SetAsyncExc(mThreadID, PyExc_KeyboardInterrupt);
        if (nThreads == 0)
        {
            qDebug() << "Oh no! The Python interpreter doesn't know that thread.";
        }
        else if (nThreads > 1)
        {
            // apparently this can acRetually happen if you mess up the C<->Python bindings
            qDebug() << "Oh no! There seem to be multiple threads with the same ID!";
        }
        PyGILState_Release(state);
        qDebug() << "thread terminated";
    }

    bool PythonThread::getInput(InputType type, QString prompt, QVariant defaultValue)
    {
        if (!mInputMutex.tryLock())
        {
            qDebug() << "Oh no! Function already locked waiting for input.";
            return false;
        }
        Q_EMIT requireInput(type,prompt,defaultValue);
        mInputMutex.lock(); // wait for set Input
        mInputMutex.unlock();
        return true;
    }

    std::string PythonThread::handleConsoleInput(const QString& prompt)
    {
        if (!getInput(ConsoleInput, prompt, QString())) return std::string();
        return mInput.toString().toStdString();
    }

    std::string PythonThread::handleStringInput(const QString& prompt, const QString& defval)
    {
        if (!getInput(StringInput, prompt, defval)) return std::string();
        return mInput.toString().toStdString();
    }

    int PythonThread::handleNumberInput(const QString& prompt, int defval)
    {
        if (!getInput(NumberInput, prompt, defval)) return 0;
        return mInput.toInt();
    }

    Module* PythonThread::handleModuleInput(const QString& prompt)
    {
        if (!getInput(ModuleInput, prompt, QString())) return 0;
        return static_cast<Module*>(mInput.value<void*>());
    }

    Gate* PythonThread::handleGateInput(const QString& prompt)
    {
        if (!getInput(GateInput, prompt, QString())) return 0;
        return static_cast<Gate*>(mInput.value<void*>());
    }

    void PythonThread::setInput(const QVariant &inp)
    {
        mInput = inp;
        mInputMutex.unlock();
    }
}
