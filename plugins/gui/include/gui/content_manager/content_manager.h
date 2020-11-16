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

#include <QGraphicsScene>
#include <QObject>
#include <QStringList>

#include "gui/netlist_watcher/netlist_watcher.h"
#include "gui/gui_utils/special_log_content_manager.h"
#include "hal_config.h"

namespace hal
{
    class MainWindow;
    class ContentWidget;
    class PythonEditor;
    class GraphTabWidget;
    class vigation_widget;
    class GroupingManagerWidget;
    class ContextManagerWidget;
    class NetlistWatcher;
    class SelectionDetailsWidget;

    class ContentManager : public QObject
    {
        Q_OBJECT

    public:
        explicit ContentManager(MainWindow* parent);

        ~ContentManager();

        PythonEditor* getPythonEditorWidget();

        GraphTabWidget* getGraphTabWidget();

        ContextManagerWidget* getContextManagerWidget();

        SelectionDetailsWidget* getSelectionDetailsWidget();

        GroupingManagerWidget* getGroupingManagerWidget();

        void hackDeleteContent();

    Q_SIGNALS:
        void saveTriggered();

    public Q_SLOTS:

        void handleOpenDocument(const QString& fileName);

        void handleCloseDocument();

        void handleFilsystemDocChanged(const QString& fileName);

        void handleSaveTriggered();

    private:
        MainWindow* mMainWindow;

        QString mWindowTitle;

        QList<ContentWidget*> mContent;

        NetlistWatcher* mNetlistWatcher;

        PythonEditor* mPythonWidget;

        GraphTabWidget* mGraphTabWid;

        ContextManagerWidget* mContextManagerWid;

        GroupingManagerWidget* mGroupingManagerWidget;

        SelectionDetailsWidget* mSelectionDetailsWidget;
#ifdef HAL_STUDY
        SpecialLogContentManager* mSpecialLogContentManager;
#endif
    };
}
