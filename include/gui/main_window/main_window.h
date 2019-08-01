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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "content_layout_area/content_layout_area.h"
#include "core/program_options.h"
#include "plugin_manager/plugin_manager_widget.h"
#include "plugin_manager/plugin_model.h"
#include "settings/main_settings_widget.h"
#include "splitter/splitter.h"
#include <QLayout>
#include <QMenuBar>
#include <QObject>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>
#include <core/program_options.h>

class plugin_model;
class plugin_manager_dialog;
class plugin_schedule_widget;
class python_editor;
class file_manager;
class hal_content_manager;
class dropdown_list;
class about_dialog;
class welcome_screen;

class main_window : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString open_icon_path READ open_icon_path WRITE set_open_icon_path)
    Q_PROPERTY(QString open_icon_style READ open_icon_style WRITE set_open_icon_style)
    Q_PROPERTY(QString save_icon_path READ save_icon_path WRITE set_save_icon_path)
    Q_PROPERTY(QString save_icon_style READ save_icon_style WRITE set_save_icon_style)
    Q_PROPERTY(QString schedule_icon_path READ schedule_icon_path WRITE set_schedule_icon_path)
    Q_PROPERTY(QString schedule_icon_style READ schedule_icon_style WRITE set_schedule_icon_style)
    Q_PROPERTY(QString run_icon_path READ run_icon_path WRITE set_run_icon_path)
    Q_PROPERTY(QString run_icon_style READ run_icon_style WRITE set_run_icon_style)
    Q_PROPERTY(QString content_icon_path READ content_icon_path WRITE set_content_icon_path)
    Q_PROPERTY(QString content_icon_style READ content_icon_style WRITE set_content_icon_style)
    Q_PROPERTY(QString settings_icon_path READ settings_icon_path WRITE set_settings_icon_path)
    Q_PROPERTY(QString settings_icon_style READ settings_icon_style WRITE set_settings_icon_style)

public:
    explicit main_window(QWidget* parent = nullptr);
    void add_content(content_widget* widget, int index, content_anchor anchor);

    QString open_icon_path() const;
    QString open_icon_style() const;

    QString save_icon_path() const;
    QString save_icon_style() const;

    QString schedule_icon_path() const;
    QString schedule_icon_style() const;

    QString run_icon_path() const;
    QString run_icon_style() const;

    QString content_icon_path() const;
    QString content_icon_style() const;

    QString settings_icon_path() const;
    QString settings_icon_style() const;

    void set_open_icon_path(const QString& path);
    void set_open_icon_style(const QString& style);

    void set_save_icon_path(const QString& path);
    void set_save_icon_style(const QString& style);

    void set_schedule_icon_path(const QString& path);
    void set_schedule_icon_style(const QString& style);

    void set_run_icon_path(const QString& path);
    void set_run_icon_style(const QString& style);

    void set_content_icon_path(const QString& path);
    void set_content_icon_style(const QString& style);

    void set_settings_icon_path(const QString& path);
    void set_settings_icon_style(const QString& style);

Q_SIGNALS:
    void save_triggered();

public Q_SLOTS:
    void on_action_quit_triggered();
    void on_action_close_document_triggered();
    void run_plugin_triggered(const QString& name);
    void toggle_schedule();
    void toggle_settings();
    void show_layout_area();
    void handle_action_open();
    void handle_file_opened(const QString& file_name);
    void handle_save_triggered();
    void handle_action_closed();

private:
    void closeEvent(QCloseEvent* event);
    void restore_state();
    void save_state();

    QVBoxLayout* m_layout;
    QMenuBar* m_menu_bar;
    QStackedWidget* m_stacked_widget;

    plugin_schedule_widget* m_schedule_widget;
    main_settings_widget* m_settings;
    welcome_screen* m_welcome_screen;
    QHBoxLayout* m_tool_bar_layout;
    QToolBar* m_left_tool_bar;
    QToolBar* m_right_tool_bar;
    content_layout_area* m_layout_area;

    QAction* m_action_open;
    QAction* m_action_save;
    QAction* m_action_save_as;
    QAction* m_action_about;
    QAction* m_action_schedule;
    QAction* m_action_run_schedule;
    QAction* m_action_content;
    QAction* m_action_settings;
    QAction* m_action_close;

    QMenu* m_menu_file;
    QMenu* m_menu_edit;
    QMenu* m_menu_help;

    about_dialog* m_about_dialog;

    plugin_model* m_plugin_model;

    hal_content_manager* m_content_manager;

    QString m_open_icon_path;
    QString m_open_icon_style;

    QString m_save_icon_path;
    QString m_save_icon_style;

    QString m_schedule_icon_path;
    QString m_schedule_icon_style;

    QString m_run_icon_path;
    QString m_run_icon_style;

    QString m_content_icon_path;
    QString m_content_icon_style;

    QString m_settings_icon_path;
    QString m_settings_icon_style;
};

#endif    // MAIN_WINDOW_H
