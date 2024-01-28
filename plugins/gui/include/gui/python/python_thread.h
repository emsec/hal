// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include <QThread>
#include <QString>
#include <QMutex>
#include <QVariant>
#include <QElapsedTimer>
#include "gui/python/python_context_subscriber.h"

namespace hal {
    class Module;
    class Gate;

    /**
     * Class to aquire and release GIL
     *
     * It is important that the GIL is not released before
     * Python code has removed all objects created during
     * execution. Therefore the GIL release is done at the
     * very end when running out of scope and calling the
     * destructor
     */
    class PythonMutex
    {
        int mState;
    public:
        /// Aquire GIL
        PythonMutex();

        /// Release GIL
        ~PythonMutex();
    };

    class PythonThread : public QThread, public PythonContextSubscriber
    {
    public:
        enum InputType {ConsoleInput, StringInput, NumberInput, ModuleInput, GateInput, FilenameInput, WaitForMenuSelection};
    private:
        Q_OBJECT
        QString mScript;
        QString mErrorMessage;
        QString mResult;
        QString mStdoutBuffer;
        unsigned long mPythonThreadID;
        QVariant mInput;
        QMutex mInputMutex;
        QElapsedTimer mElapsedTimer;
        bool mSingleStatement;
        bool mAbortRequested;
        int mSpamCount;
    Q_SIGNALS:
        void stdOutput(QString txt);
        void stdError(QString txt);
        void requireInput(int type, QString prompt, QVariant defaultValue);
    public:
        PythonThread(const QString& script, bool singleStatement, QObject* parent = nullptr);
        ~PythonThread();
        void run() override;
        void interrupt();
        QString errorMessage() const { return mErrorMessage; }
        QString result() const { return mResult; }
        void handleStdout(const QString& output) override;
        void handleError(const QString& output) override;
        std::string handleConsoleInput(const QString& prompt);
        std::string handleStringInput(const QString& prompt, const QString& defval);
        int         handleNumberInput(const QString& prompt, int defval);
        Module*     handleModuleInput(const QString& prompt);
        Gate*       handleGateInput(const QString& prompt);
        std::string handleFilenameInput(const QString& prompt, const QString& filetype);
        void clear() override;
        void setInput(const QVariant& inp);
        bool getInput(InputType type, QString prompt, QVariant defaultValue);
        void unlock();
        const QString& stdoutBuffer() const { return mStdoutBuffer; }
        QString flushStdout();
     };
}
