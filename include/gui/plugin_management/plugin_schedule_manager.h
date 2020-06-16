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

#ifndef PLUGIN_SCHEDULE_MANAGER_H
#define PLUGIN_SCHEDULE_MANAGER_H

#include <QObject>

class plugin_arguments_widget;
class ProgramArguments;

struct argument
{
    QString flag;
    QString description;
    QString value;
    bool checked;
};

class plugin_schedule_manager : public QObject
{
    using schedule = QList<QPair<QString, QList<argument>>>;

    Q_OBJECT

public:
    static plugin_schedule_manager* get_instance();

    schedule* get_schedule();
    ProgramArguments get_program_arguments(int index);
    int get_current_index();

    void set_current_index(int index);

    void add_plugin(const QString& plugin, int index);
    void move_plugin(int from, int to);
    void remove_plugin(int index);

    void save_schedule(const QString& name);
    void load_schedule(const QString& name);
    void delete_schedule(const QString& name);

    void run_schedule();

private:
    explicit plugin_schedule_manager(QObject* parent = nullptr);

    schedule m_schedule;
    QList<QPair<QString, schedule*>> m_saved_schedules;

    int m_current_index;
};

#endif    // PLUGIN_SCHEDULE_MANAGER_H
