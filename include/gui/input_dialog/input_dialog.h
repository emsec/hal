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

#include "validator/stacked_validator.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace hal
{
    class InputDialog : public QDialog
    {
        public:
            InputDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
            InputDialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

            void set_window_title(const QString& title);

            void set_info_text(const QString& text);
            void set_input_text(const QString& text);

            void set_warning_text(const QString& text);

            void add_validator(validator* validator);
            void remove_validator(validator* validator);
            void clear_validators();

            QString text_value() const;

        private:
            QVBoxLayout* m_layout;
            QLabel* m_label_info_text;
            QLabel* m_label_warning_text;
            QLineEdit* m_input_text_edit;
            QPushButton* m_ok_button;

            QString m_warning_text = "";

            stacked_validator m_validator;

            void init();

            void show_warning_text();
            void hide_warning_text();

            void enable_ok_button();
            void disable_ok_button();

            void enable_progression();
            void disable_progression();

            void handle_ok_clicked();
            void handle_cancel_clicked();

            void handle_text_changed(const QString &text);
    };
}
