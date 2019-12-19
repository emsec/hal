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

#ifndef PYTHON_CONTEXT_H
#define PYTHON_CONTEXT_H

#include <QString>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include "pybind11/embed.h"
#include "pybind11/eval.h"
#pragma GCC diagnostic pop

#include "python_console.h"

class python_context_subscriber;

namespace py = pybind11;

class __attribute__((visibility("default"))) python_context
{
public:
    python_context();
    ~python_context();

    void interpret(const QString& input, bool multiple_expressions = false);
    void interpret_script(const QString& input);

    void forward_stdout(const QString& output);
    void forward_error(const QString& output);

    void forward_clear();

    void forward_reset();

    void set_console(python_console* console);

    std::vector<std::tuple<std::string, std::string>> complete(const QString& text, bool use_console_context);

    /**
     * Check if given statement is a complete or incomplete statement for compound statement processing.
     * @param[text] text - The text to check.
     * @returns -1 on error, 0 for an incomplete statement and 1 for an complete statement.
     */
    int check_complete_statement(const QString& text);

    //    void set_history_length();

    //    void read_history_file(const QString& file);
    //    void write_history_file(const QString& file);
    void init_python();
    void close_python();
    
    void update_netlist();

private:
    void initialize_context(py::dict* context);

    void handle_reset();

    // these have to be pointers, otherwise they are destructed after py::finalize_interpreter and segfault
    // only one object for global and local is needed, as for the console we run it always in global scope wher globals() == locals()
    py::dict* m_context;

    python_context_subscriber* m_sender;

    std::string m_history_file;

    python_console* m_console;
    bool m_trigger_reset = false;
};

#endif    // PYTHON_CONTEXT_H
