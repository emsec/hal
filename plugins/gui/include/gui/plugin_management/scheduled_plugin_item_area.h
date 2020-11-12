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
#include <QList>
#include <QPair>
#include <QVector>

class QVBoxLayout;

namespace hal
{
    class DropMarker;
    class ScheduledPluginItem;

    class ScheduledPluginItemArea : public QFrame
    {
        Q_OBJECT

    public:
        explicit ScheduledPluginItemArea(QWidget* parent = nullptr);

        void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
        void dragMoveEvent(QDragMoveEvent* event) Q_DECL_OVERRIDE;
        void dragLeaveEvent(QDragLeaveEvent* event) Q_DECL_OVERRIDE;
        void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

        void insertPlugin(const QString& name);

    Q_SIGNALS:
        void pluginAdded(QString plugin, int index);
        void pluginMoved(int from, int to);
        void pluginRemoved(int index);

        void pluginSelected(int index);

        void noScheduledPlugins();

    public Q_SLOTS:
        void handleItemClicked(ScheduledPluginItem* item);
        void handleItemDragStarted(ScheduledPluginItem* item);
        void handleItemRemoved(ScheduledPluginItem* item);

    private:
        QVBoxLayout* mLayout;
        QFrame* mSpacer;

        QVector<int> mYValues;
        QList<QPair<ScheduledPluginItem*, DropMarker*>> mList;

        DropMarker* mActiveMarker;

        bool mInternalDragActive;
        int mDragIndex;
    };
}
