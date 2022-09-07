#include "gui/python/python_thread.h"
#include <QDebug>

#include <pyerrors.h>
#include <ceval.h>
#include <pystate.h>

namespace hal {
    void PythonThread::run()
    {
        // If we have multiple threads, this will block until the interpreter
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
        qDebug() << "about to terminate thread..." << tid();
        PyGILState_STATE state = PyGILState_Ensure();
        // We interrupt the thread by forcibly injecting an exception
        // (namely the KeyboardInterrupt exception, but any other one works as well)
        int nThreads = PyThreadState_SetAsyncExc(tid(), PyExc_KeyboardInterrupt);
        if (nThreads == 0)
        {
            qDebug() << "Oh no! The Python interpreter doesn't know that thread.";
        }
        else if (nThreads > 1)
        {
            // apparently this can actually happen if you mess up the C<->Python bindings
            qDebug() << "Oh no! There seem to be multiple threads with the same ID!";
        }
        PyGILState_Release(state);
        qDebug() << "thread terminated";
    }
}
