//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

#include "gui/gui_utils/special_log_content_manager.h"
#include "hal_config.h"

namespace hal
{
    class MainWindow;
    class ContentWidget;
    class PythonConsoleWidget;
    class PythonEditor;
    class GraphTabWidget;
    class GroupingManagerWidget;
    class ContextManagerWidget;
    class SelectionDetailsWidget;
    class ModuleWidget;
    class LoggerWidget;
    class SettingsItemDropdown;
    class SettingsItemKeybind;


    /**
     * @ingroup gui
     * @brief Manages all ContentWidget%s
     *
     * The ContentManager class manages the creation and deletion of hal's subwidgets such as the python editor or the
     * selection details widget. There should only be one content manager at any time.
     */
    class ContentManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * The constructor of the class. The content manager's parent is the MainwWndow of hal.
         * The MainWindow must be known to the content manager so it can place the subwidgets in it.
         *
         * @param parent - The content manager's parent.
         */
        explicit ContentManager(MainWindow* parent);       
        ~ContentManager();

        /**
         * Deletes all subwidgets created by the manager. Used when closing a file.
         */
        void deleteContent();

        /**
         * Get hal's python editor.
         *
         * @return The python editor.
         */
        PythonEditor* getPythonEditorWidget();

        /**
         * @brief Get hal's graph tab widget.
         *
         * @return The graph tab widget.
         */
        GraphTabWidget* getGraphTabWidget();

        /**
         * Get hal's context manager.
         *
         * @return The context manager.
         */
        ContextManagerWidget* getContextManagerWidget();

        /**
         * Get hal's selection details widget.
         *
         * @return The selection details widget.
         */
        SelectionDetailsWidget* getSelectionDetailsWidget();

        /**
         * Get hal's grouping manager.
         *
         * @return The grouping manager.
         */
        GroupingManagerWidget* getGroupingManagerWidget();

        /**
         * Get hal's module tree widget.
         *
         * @return module tree widget
         */
        ModuleWidget* getModuleWidget();

        /**
         * Set main window title.
         *
         * @param filename name of netlist filename.
         */
        void setWindowTitle(const QString& filename);

    public Q_SLOTS:

        /**
         * This function is called when a file is opened (connected to a signal that is emitted by the file manager).
         * It creates the necessary subwidgets and places them in the mainwindow.
         *
         * @param fileName - The name of the opened file.
         */
        void handleOpenDocument(const QString& fileName);

    public:
        static SettingsItemKeybind* sSettingSearch;

    private:
        MainWindow* mMainWindow;
        QString mWindowTitle;

        QList<ContentWidget*> mContent;

        PythonConsoleWidget* mPythonConsoleWidget;
        PythonEditor* mPythonWidget;
        GraphTabWidget* mGraphTabWidget;
        ModuleWidget* mModuleWidget;
        ContextManagerWidget* mContextManagerWidget;
        GroupingManagerWidget* mGroupingManagerWidget;
        SelectionDetailsWidget* mSelectionDetailsWidget;
        LoggerWidget* mLoggerWidget;

        static SettingsItemDropdown* sSettingSortMechanism;
        static bool sSettingsInitialized;
        static bool initializeSettins();

#ifdef HAL_STUDY
        SpecialLogContentManager* mSpecialLogContentManager;
#endif
    };
}
