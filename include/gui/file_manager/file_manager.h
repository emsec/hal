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

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "core/program_arguments.h"

#include <QObject>
#include <QTimer>

class QFileSystemWatcher;

class file_manager : public QObject
{
    Q_OBJECT

public:
    static file_manager* get_instance();

    void handle_program_arguments(const program_arguments& args);

    QString file_name() const;
    bool file_open() const;

    void watch_file(const QString& file_name);


Q_SIGNALS:
    void file_opened(const QString& file_name);
    void file_changed(const QString& path);
    void file_directory_changed(const QString& path);
    void file_closed();

public Q_SLOTS:
    void open_file(QString file_name);
    void close_file();
    void autosave();

private Q_SLOTS:
    void handle_file_changed(const QString& path);
    void handle_directory_changed(const QString& path);
    void handle_global_setting_changed(void* sender, const QString& key, const QVariant& value);

private:
    file_manager(QObject* parent = nullptr);
    void file_successfully_loaded(QString file_name);
    void update_recent_files(const QString& file) const;
    void display_error_message(QString error_message);
    QString get_shadow_file(QString file);
    void remove_shadow_file();

    QString m_file_name;
    QString m_shadow_file_name;
    QFileSystemWatcher* m_file_watcher;
    bool m_file_open;
    QTimer* m_timer;
    bool m_autosave_enabled;
    int m_autosave_interval;
};

#endif // FILE_MANAGER_H
