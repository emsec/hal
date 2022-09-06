#include "gui/python/python_thread.h"
#include <QDebug>

namespace hal {
    void PythonThread::run()
    {
        mPid = getpid();

        py::dict tmp_context(py::globals());
        PythonContext::initializeScript(&tmp_context);

        try
        {
            py::eval<py::eval_statements>(mScript.toStdString(), tmp_context, tmp_context);
        }
        catch (py::error_already_set& e)
        {
            mErrorMessage = QString::fromStdString(std::string(e.what()) + "\n");
            e.restore();
            PyErr_Clear();
        }
        catch (std::exception& e)
        {
            mErrorMessage = QString::fromStdString(std::string(e.what()) + "#\n");
        }
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
}
