#include "gui/python/python_thread.h"
#include "gui/python/python_context.h"
#include <QDebug>

#include <pyerrors.h>
#include <ceval.h>
#include <pystate.h>
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include <QApplication>
#include <QDebug>
#include <QRegularExpression>

namespace hal {
    PythonMutex::PythonMutex()
    {
        mState = PyGILState_Ensure();
    }

    PythonMutex::~PythonMutex()
    {
        PyGILState_Release((PyGILState_STATE)mState);
    }

    PythonThread::PythonThread(const QString& script, bool singleStatement, QObject* parent)
        : QThread(parent), mScript(script), mSingleStatement(singleStatement),
          mAbortRequested(false), mSpamCount(0)
    {
      //    qDebug() << "+++PythonThread" << QString::number((quintptr) this, 16);
    }

    PythonThread::~PythonThread()
    {
      //    qDebug() << "---PythonThread" << QString::number((quintptr) this, 16);
    }

    void PythonThread::run()
    {
        // decides it's our turn.
        PythonMutex pmutex;
        py::dict tmp_context(py::globals());
        PythonContext::initializeScript(&tmp_context);

        // Capture the Python-internal thread ID, which is needed if we want to
        // interrupt the thread later
        pybind11::object rc = py::eval("threading.get_ident()", tmp_context, tmp_context);
        mPythonThreadID = rc.cast<unsigned long>();

        mElapsedTimer.start();
        try
        {
            py::object rc;
            if (mSingleStatement)
                rc = py::eval<py::eval_single_statement>(mScript.toStdString(), tmp_context, tmp_context);
            else
                rc = py::eval<py::eval_statements>(mScript.toStdString(), tmp_context, tmp_context);

            if (!rc.is_none())
                mResult = QString::fromStdString(py::str(rc).cast<std::string>());
        }
        catch (py::error_already_set& e)
        {
            qDebug() << "AlreadySet";
            mErrorMessage = QString::fromStdString(std::string(e.what()));

            // pybind11 prepends a coding line to the script, so reported line numbers are
            // one too high. Rewrite "At:\n  <string>(6):" to "At line 5:" if it matches,
            // otherwise leave the message unchanged.
            QRegularExpression re("At:\\s*\\n\\s*<string>\\((\\d+)\\):");
            QRegularExpressionMatch match = re.match(mErrorMessage);
            if (match.hasMatch())
            {
                int line = match.captured(1).toInt() - 1;
                if (line >= 1)
                    mErrorMessage.replace(match.capturedStart(0), match.capturedLength(0),
                                          QString("At line %1:").arg(line));
            }
            mErrorMessage += "\n";
            e.restore();
            PyErr_Clear();
        }
        catch (std::exception& e)
        {
            qDebug() << "Exception";
            mErrorMessage = QString::fromStdString(std::string(e.what()) + "#\n");
        }

       // running out of scope calls PyGILState_Release(state);
    }

    void PythonThread::handleStdout(const QString& output)
    {
        if (mElapsedTimer.elapsed() > 100)
        {
            if (!mStdoutBuffer.isEmpty())
                ++mSpamCount;
            else
                mSpamCount = 0;
            if (mSpamCount > 1000)
                mStdoutBuffer += output;
            else if (mSpamCount < 10 || mElapsedTimer.elapsed() > 1000)
            {
                Q_EMIT stdOutput(mStdoutBuffer + output);
                mStdoutBuffer.clear();
                mElapsedTimer.restart();
            }
            else
                mStdoutBuffer += output;
        }
        else
            mStdoutBuffer += output;
    }

    QString PythonThread::flushStdout()
    {
        QString retval = mStdoutBuffer;
        mStdoutBuffer.clear();
        mElapsedTimer.restart();
        return retval;
    }

    void PythonThread::handleError(const QString& output)
    {
        Q_EMIT stdError(output);
    }

    void PythonThread::clear()
    {;}

    void PythonThread::interrupt() {
        if (!mInputMutex.tryLock())
        {
            mAbortRequested = true;
            mInputMutex.unlock();
            return;
        }
        else
        {
            mInputMutex.unlock();
        }
        qDebug() << "about to terminate thread..." << mPythonThreadID;
        PythonMutex pmutex;
        // We interrupt the thread by forcibly injecting an exception
        // (namely the KeyboardInterrupt exception, but any other one works as well)
        int nThreads = PyThreadState_SetAsyncExc(mPythonThreadID, PyExc_KeyboardInterrupt);
        if (nThreads == 0)
        {
            qDebug() << "Oh no! The Python interpreter doesn't know that thread.";
        }
        else if (nThreads > 1)
        {
            // apparently this can actually happen if you mess up the C<->Python bindings
            qDebug() << "Oh no! There seem to be multiple threads with the same ID!";
        }
        qDebug() << "thread terminated";
        // running out of scope calls PyGILState_Release(state);
    }

    bool PythonThread::getInput(InputType type, QString prompt, QVariant defaultValue)
    {
        if (!mInputMutex.tryLock())
        {
            qDebug() << "Oh no! Function already locked waiting for input.";
            return false;
        }
        if (type != WaitForMenuSelection)
            Q_EMIT requireInput(type,prompt,defaultValue);
        mInputMutex.lock(); // wait for set Input
        mInputMutex.unlock();
        if (mAbortRequested)
        {
            throw std::runtime_error(std::string("Python script aborted by user"));
            return false;
        }
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

    std::string PythonThread::handleFilenameInput(const QString& prompt, const QString& filetype)
    {
        if (!getInput(FilenameInput, prompt, filetype)) return std::string();
        return mInput.toString().toStdString();
    }

    void PythonThread::setInput(const QVariant &inp)
    {
        mInput = inp;
        mInputMutex.unlock();
    }

    void PythonThread::unlock()
    {
       // unlocking if not locked might cause undefined behavior
       mInputMutex.tryLock();
       mInputMutex.unlock();
    }
}
