//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "gui/python/python_console.h"

#include <QString>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include "pybind11/embed.h"
#include "pybind11/eval.h"
#include "pybind11/operators.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "pybind11/functional.h"
#pragma GCC diagnostic pop

namespace hal
{
    class PythonContextSubscriber;

    namespace py = pybind11;
    class __attribute__((visibility("default"))) PythonContext
    {
    public:
        PythonContext();
        ~PythonContext();

        void interpret(const QString& input, bool multiple_expressions = false);
        void interpretScript(const QString& input);

        void forwardStdout(const QString& output);
        void forwardError(const QString& output);

        void forwardClear();

        void forwardReset();

        void setConsole(PythonConsole* console);

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
        void initPython();
        void closePython();

        void updateNetlist();

    private:
        void initializeContext(py::dict* context);

        void handleReset();

        // these have to be pointers, otherwise they are destructed after py::finalize_interpreter and segfault
        // only one object for global and local is needed, as for the console we run it always in global scope wher globals() == locals()
        py::dict* mContext;

        PythonContextSubscriber* mSender;

        std::string mHistoryFile;

        PythonConsole* mConsole;
        bool mTriggerReset = false;
    };
}
