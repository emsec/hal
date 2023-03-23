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

#include "gui/python/python_console.h"
#include "gui/python/python_thread.h"
#include "gui/module_dialog/gate_select_model.h"
#include "gui/module_dialog/module_select_model.h"

#include <QString>
#include <QObject>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include "pybind11/embed.h"
#include "pybind11/eval.h"
#include "pybind11/functional.h"
#include "pybind11/operators.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#pragma GCC diagnostic pop

namespace hal
{
    class PythonContextSubscriber;
    class PythonEditor;
    class PythonThread;
    class LayoutLocker;

    class PythonGateSelectionReceiver : public GateSelectReceiver
    {
        Q_OBJECT
        PythonThread* mThread;
    public Q_SLOTS:
        void handleGatesPicked(const QSet<u32>& gats) override;
    public:
        PythonGateSelectionReceiver(PythonThread* thread, QObject* parent = nullptr)
            : GateSelectReceiver(parent), mThread(thread) {;}
    };

    class PythonModuleSelectionReceiver : public ModuleSelectReceiver
    {
        Q_OBJECT
        PythonThread* mThread;
    public Q_SLOTS:
        void handleModulesPicked(const QSet<u32>& gats) override;
    public:
        PythonModuleSelectionReceiver(PythonThread* thread, QObject* parent = nullptr)
            : ModuleSelectReceiver(parent), mThread(thread) {;}
    };

    namespace py = pybind11;
    /**
     * @ingroup python
     * @brief Interface for pybind11.
     *
     * This class provides an interface for using pybind11. It can call a python interpreter to run python expressions and
     * scripts. Moreover it provides a function to get auto-completion candidates that are available in the context.
     */
    class __attribute__((visibility("default"))) PythonContext : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         */
        PythonContext(QObject* parent=nullptr);

        /**
         * Destructor.
         */
        ~PythonContext();

        /**
         * Interprets an input string in python format in GUI thread.
         *
         * @param input - The input string in python format.
         */
        void interpretForeground(const QString& input);

        /**
         * Interprets an input string in python format in new thread.
         *
         * @param caller - The caller of interpreter, will be notified when finished
         * @param input - The input string in python format.
         * @param multiple_expressions - Must be set to <b>true</b> if the input contains multiple expressions
         *                               (i.e. is a compound statement).
         */
        void interpretBackground(QObject* caller, const QString& input, bool multiple_expressions = false);

        /**
         * Interprets a python script.
         *
         * @param caller - The caller of interpreter, will be notified when finished
         * @param input - The python script as a string.
         */
        void interpretScript(QObject* caller, const QString& input);

        /**
         * Forwards standard output to the python console.
         *
         * @param output - The output
         */
        void forwardStdout(const QString& output);

        /**
         * Forwards error output to the python console.
         *
         * @param output - The output
         */
        void forwardError(const QString& output);

        /**
         * Clears the python console.
         *
         */
        void scheduleClear();

        /**
         * Resets the python console.
         */
        void scheduleReset();

        /**
         * Assign this object the python console to work with.
         *
         * @param console - The python console to work with.
         */
        void setConsole(PythonConsole* console);

        /**
         * Compute a list of auto completion options for a given input. <br>
         * Each entry is a tuple (name_with_symbols, complete). <br>
         * E.g. for <i>text</i>="pri" one entry would be ("print", "nt")
         *
         * @param text - The text to complete
         * @param use_console_context - Set <b>true</b> if identifiers in the context of the python console should be
         *                              considered in the auto completion.
         * @returns a vector of all possible auto completion candidates.
         */
        std::vector<std::tuple<std::string, std::string>> complete(const QString& text, bool use_console_context);

        /**
         * Check if given statement is a complete or incomplete statement for compound statement processing.
         * @param[text] text - The text to check.
         * @returns -1 on error, 0 for an incomplete statement and 1 for an complete statement.
         */
        int checkCompleteStatement(const QString& text);

        //    void set_history_length();

        //    void read_history_file(const QString& file);
        //    void write_history_file(const QString& file);
        /**
         * Initializes the context to work with hal.
         */
        void initPython();

        /**
         * Close the current context.
         */
        void closePython();

        /**
         * Update the current netlist the python context works with.
         */
        void updateNetlist();

        PythonThread* pythonThread() const;

        static void initializeContext(py::dict* context);
        static void initializeScript(py::dict* context);

        void abortThread();
        void abortThreadAndWait();
        bool isThreadRunning() const { return mThread != nullptr; }

    private Q_SLOTS:
        void handleThreadFinished();
        void handleScriptOutput(const QString& txt);
        void handleScriptError(const QString& txt);
        void handleInputRequired(int type, const QString& prompt, const QVariant& defaultValue);
        void handleConsoleInputReceived(const QString& input);

    private:

        void handleReset();
        void handleClear();

        // these have to be pointers, otherwise they are destructed after py::finalize_interpreter and segfault
        // only one object for global and local is needed, as for the console we run it always in global scope wher globals() == locals()
        py::dict* mContext;

        PythonContextSubscriber* mSender;

        std::string mHistoryFile;

        PythonConsole* mConsole;
        bool mTriggerReset;
        bool mTriggerClear;
        LayoutLocker* mLayoutLocker;
        PythonThread* mThread;
        bool mThreadAborted;
        PyThreadState* mMainThreadState;
        QObject* mInterpreterCaller;

        void startThread(const QString& input, bool singleStatement);
    };
}    // namespace hal
