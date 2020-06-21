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

#include "python/python_context_subscriber.h"

#include <QTextEdit>
#include <memory>

namespace hal
{
    class PythonConsoleHistory;

    class PythonConsole : public QTextEdit, public PythonContextSubscriber
    {
        Q_OBJECT

    public:
        PythonConsole(QWidget* parent = nullptr);

        void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
        void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

        virtual void handle_stdout(const QString& output) Q_DECL_OVERRIDE;
        virtual void handle_error(const QString& output) Q_DECL_OVERRIDE;
        virtual void clear() Q_DECL_OVERRIDE;

        void display_prompt();

        void interpret_command();
        QString get_current_command();
        void replace_current_command(const QString& new_command);
        void append_to_current_command(const QString& new_command);

        bool selection_editable();

        void handle_up_key_pressed();
        void handle_down_key_pressed();

        void handle_tab_key_pressed();

        void insertAtEnd(const QString& text, QColor textColor);

    private:
        QColor m_prompt_color;
        QColor m_standard_color;
        QColor m_error_color;

        QString m_standard_prompt;
        QString m_compound_prompt;

        int m_prompt_block_number;
        int m_prompt_length;
        int m_prompt_end_position;
        int m_compound_prompt_end_position;

        bool m_in_compound_prompt;
        bool m_in_completion;

        QString m_current_compound_input;
        QString m_current_input;

        int m_current_history_index;
        int m_current_completer_index;

        std::shared_ptr<PythonConsoleHistory> m_history;
    };
}
