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

#include <QFrame>

class QFrame;
class QHBoxLayout;
class QVBoxLayout;

namespace hal
{
    class LabeledFrame;
    class LoadedPluginsWidget;
    class PluginArgumentsWidget;
    class NoScheduledPluginsWidget;
    class ScheduledPluginsWidget;
    class Searchbar;

    class PluginScheduleWidget : public QFrame
    {
        Q_OBJECT

    public:
        explicit PluginScheduleWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void debug_stuff();
        void handle_no_scheduled_plugins();

    private:
        QVBoxLayout* m_vertical_layout;
        QFrame* m_searchbar_container;
        QHBoxLayout* m_container_layout;
        Searchbar* m_searchbar;
        QHBoxLayout* m_horizontal_layout;
        LabeledFrame* m_plugin_frame;
        LabeledFrame* m_schedule_frame;
        QWidget* m_schedule_frame_layout_container;
        QHBoxLayout* m_horizontal_schedule_frame_layout;
        QVBoxLayout* m_vertical_schedule_frame_layout;
        NoScheduledPluginsWidget* m_NoScheduledPluginsWidget;
        PluginArgumentsWidget* m_PluginArgumentsWidget;
        ScheduledPluginsWidget* m_ScheduledPluginsWidget;
        LoadedPluginsWidget* m_LoadedPluginsWidget;
    };
}
