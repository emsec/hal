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

#ifndef PYTHON_WIDGET_H
#define PYTHON_WIDGET_H

#include "code_editor/code_editor.h"
#include "content_widget/content_widget.h"
#include "python/python_context_suberscriber.h"

class code_editor;
class searchbar;
class splitter;
class toolbar;

class QVBoxLayout;

class python_code_editor : public code_editor
{
    Q_OBJECT
public:
    void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;

    void handle_tab_key_pressed();
    void handle_shift_tab_key_pressed();
    void handle_return_pressed();
    void handle_backspace_pressed(QKeyEvent* e);
    void handle_delete_pressed(QKeyEvent* e);

    void handle_autocomplete();
    void perform_code_completion(std::tuple<std::string, std::string> completion);

private:
    void indent_selection(bool indentUnindent);
    int next_indent(bool indentUnindent, int current_indent);
};

class python_editor : public content_widget, public python_context_subscriber
{
    Q_OBJECT
    Q_PROPERTY(QString open_icon_path READ open_icon_path WRITE set_open_icon_path)
    Q_PROPERTY(QString open_icon_style READ open_icon_style WRITE set_open_icon_style)
    Q_PROPERTY(QString save_icon_path READ save_icon_path WRITE set_save_icon_path)
    Q_PROPERTY(QString save_icon_style READ save_icon_style WRITE set_save_icon_style)
    Q_PROPERTY(QString save_as_icon_path READ save_as_icon_path WRITE set_save_as_icon_path)
    Q_PROPERTY(QString save_as_icon_style READ save_as_icon_style WRITE set_save_as_icon_style)
    Q_PROPERTY(QString run_icon_path READ run_icon_path WRITE set_run_icon_path)
    Q_PROPERTY(QString run_icon_style READ run_icon_style WRITE set_run_icon_style)

public:
    explicit python_editor(QWidget* parent = nullptr);
    ~python_editor();

    virtual void setup_toolbar(toolbar* toolbar) Q_DECL_OVERRIDE;
    virtual QList<QShortcut*> create_shortcuts() Q_DECL_OVERRIDE;

    virtual void handle_stdout(const QString& output) Q_DECL_OVERRIDE;
    virtual void handle_error(const QString& output) Q_DECL_OVERRIDE;
    virtual void clear() Q_DECL_OVERRIDE;

    void handle_action_open_file();
    void handle_action_save_file();
    void handle_action_save_file_as();
    void handle_action_run();

    void save_file(const bool ask_path);

    QString open_icon_path() const;
    QString open_icon_style() const;

    QString save_icon_path() const;
    QString save_icon_style() const;

    QString save_as_icon_path() const;
    QString save_as_icon_style() const;

    QString run_icon_path() const;
    QString run_icon_style() const;

    void set_open_icon_path(const QString& path);
    void set_open_icon_style(const QString& style);

    void set_save_icon_path(const QString& path);
    void set_save_icon_style(const QString& style);

    void set_save_as_icon_path(const QString& path);
    void set_save_as_icon_style(const QString& style);

    void set_run_icon_path(const QString& path);
    void set_run_icon_style(const QString& style);

Q_SIGNALS:
    void forward_stdout(const QString& output);
    void forward_error(const QString& output);

public Q_SLOTS:
    void toggle_searchbar();

private:
    QVBoxLayout* m_layout;
    toolbar* m_toolbar;
    splitter* m_splitter;

    //debug code
    code_editor* m_editor_widget;

    searchbar* m_searchbar;

    QAction* m_action_open_file;
    QAction* m_action_run;
    QAction* m_action_save;
    QAction* m_action_save_as;

    QString m_open_icon_style;
    QString m_open_icon_path;

    QString m_save_icon_style;
    QString m_save_icon_path;

    QString m_save_as_icon_style;
    QString m_save_as_icon_path;

    QString m_run_icon_style;
    QString m_run_icon_path;

    QString m_file_name;
};

#endif    // PYTHON_WIDGET_H
