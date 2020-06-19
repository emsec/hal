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

#include "gui_def.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>

namespace hal
{
    class file_modified_bar : public QFrame
    {
        Q_OBJECT

    public:
        file_modified_bar(QWidget* parent = 0);

        void set_message(QString message);

    Q_SIGNALS:
        void reload_clicked();
        void ignore_clicked();
        void ok_clicked();

    public Q_SLOTS:
        void handle_file_changed(QString path);

    private:
        QHBoxLayout* m_layout;
        QLabel* m_message_label;
        QPushButton* m_reload_button;
        QPushButton* m_ignore_button;
        QPushButton* m_ok_button;

        void handle_reload_clicked();
        void handle_ignore_clicked();
        void handle_ok_clicked();

        void handle_file_content_modified(QString path);
        void handle_file_mov_or_del(QString path);
    };
}
