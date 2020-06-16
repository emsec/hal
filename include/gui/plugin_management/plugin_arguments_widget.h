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

#ifndef PLUGIN_ARGUMENTS_WIDGET_H
#define PLUGIN_ARGUMENTS_WIDGET_H

#include <QFrame>
#include <QPair>
#include <QVector>

#include <memory>

class CLIPluginInterface;
class ProgramArguments;

class QFormLayout;
class QLineEdit;
class QPushButton;
class QVBoxLayout;

class plugin_arguments_widget : public QFrame
{
    Q_OBJECT

public:
    explicit plugin_arguments_widget(QWidget* parent = nullptr);

    ProgramArguments get_args();

    void setup_plugin_layout(const QString& plugin);

public Q_SLOTS:
    void setup(const QString& plugin_name);
    void handle_plugin_selected(int index);

    void handle_button_clicked(bool checked);
    void handle_text_edited(const QString& text);

private:
    char* to_heap_cstring(const QString& string);

    QFormLayout* m_form_layout;

    std::shared_ptr<CLIPluginInterface> m_plugin;

    QVector<QPair<QPushButton*, QLineEdit*>> m_vector;

    int m_current_index;
};

#endif    // PLUGIN_ARGUMENTS_WIDGET_H
