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

class QFileSystemWatcher;

class file_manager : public QObject
{
    Q_OBJECT

public:
    static file_manager* get_instance();

    void handle_program_arguments(const program_arguments& args);

    bool is_document_open();
    QString file_name() const;

Q_SIGNALS:
    void file_opened(const QString& file_name);
    void file_changed(const QString& path);
    void file_directory_changed(const QString& path);
    void file_closed();

public Q_SLOTS:
    void open_file(const QString& file_name);
    void close_file();

private Q_SLOTS:
    void handle_file_changed(const QString& path);
    void handle_directory_changed(const QString& path);

private:
    file_manager(QObject* parent = nullptr);
    void update_recent_files(const QString& file);
    void display_error_message(QString error_message);

    QString m_file_name;
    QFileSystemWatcher* m_file_watcher;
    bool m_file_open;
};

#endif    // FILE_MANAGER_H
