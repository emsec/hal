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

#include "gui/content_layout_area/content_layout_area.h"
#include "hal_core/utilities/program_options.h"
#include "gui/action/action.h"
#include "gui/plugin_manager/plugin_manager_widget.h"
#include "gui/plugin_manager/plugin_model.h"
#include "gui/settings/main_settings_widget.h"
#include "gui/splitter/splitter.h"
#include <QLayout>
#include <QMenuBar>
#include <QObject>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>
#include "hal_core/utilities/program_options.h"

namespace hal
{
    class PluginModel;
    class plugin_manager_dialog;
    class PluginScheduleWidget;
    class PythonEditor;
    class FileManager;
    class ContentManager;
    class AboutDialog;
    class WelcomeScreen;

    /**
     * One widget to rule them all,
     *   one widget to create them,
     * One widget to bring them all
     *   and in the dark theme bind them.
     *
     * (The top level widget)
     */
    class MainWindow : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString halIconPath READ halIconPath WRITE setHalIconPath)
        Q_PROPERTY(QString halIconStyle READ halIconStyle WRITE setHalIconStyle)

        Q_PROPERTY(QString newFileIconPath READ newFileIconPath WRITE setNewFileIconPath)
        Q_PROPERTY(QString newFileIconStyle READ newFileIconStyle WRITE setNewFileIconStyle)
        Q_PROPERTY(QString openIconPath READ openIconPath WRITE setOpenIconPath)
        Q_PROPERTY(QString openIconStyle READ openIconStyle WRITE setOpenIconStyle)
        Q_PROPERTY(QString saveIconPath READ saveIconPath WRITE setSaveIconPath)
        Q_PROPERTY(QString saveIconStyle READ saveIconStyle WRITE setSaveIconStyle)
        Q_PROPERTY(QString scheduleIconPath READ scheduleIconPath WRITE setScheduleIconPath)
        Q_PROPERTY(QString scheduleIconStyle READ scheduleIconStyle WRITE setScheduleIconStyle)
        Q_PROPERTY(QString runIconPath READ runIconPath WRITE setRunIconPath)
        Q_PROPERTY(QString runIconStyle READ runIconStyle WRITE setRunIconStyle)
        Q_PROPERTY(QString contentIconPath READ contentIconPath WRITE setContentIconPath)
        Q_PROPERTY(QString contentIconStyle READ contentIconStyle WRITE setContentIconStyle)
        Q_PROPERTY(QString settingsIconPath READ settingsIconPath WRITE setSettingsIconPath)
        Q_PROPERTY(QString settingsIconStyle READ settingsIconStyle WRITE setSettingsIconStyle)

    public:
        /**
         * The Constructor.
         *
         * @param parent - The parent widget
         */
        explicit MainWindow(QWidget* parent = nullptr);

        /**
         * Add a content widget to the layout area.
         *
         * @param widget - the content widget to add
         * @param index - the position index within the anchor, the widget should be put in
         * @param anchor - the anchor, the widget should be assigned to
         */
        void addContent(ContentWidget* widget, int index, content_anchor anchor);

        // =====================================================================
        //   Q_PROPERTY functions
        // =====================================================================

        /**
         * Q_PROPERTY READ function for the hal icon path.
         *
         * @returns the hal icon path
         */
        QString halIconPath() const;
        /**
         * Q_PROPERTY READ function for the hal icon style.
         *
         * @returns the hal icon style
         */
        QString halIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'New File'-icon path.
         *
         * @returns the 'New File'-icon path
         */
        QString newFileIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'New File'-icon style.
         *
         * @returns the 'New File'-icon style
         */
        QString newFileIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Open File'-icon path.
         *
         * @returns the 'Open File'-icon path
         */
        QString openIconPath() const;
        /**
        * Q_PROPERTY READ function for the 'Open File'-icon style.
         *
        * @returns the 'Open File'-icon style
        */
        QString openIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Save File'-icon path.
         *
         * @returns the 'SaveFile'-icon path
         */
        QString saveIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'Save File'-icon style.
         *
         * @returns the 'Save File'-icon style
         */
        QString saveIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Schedule'-icon path.
         *
         * @returns the 'Schedule'-icon path
         */
        QString scheduleIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'Schedule'-icon style.
         *
         * @returns the 'Schedule'-icon style
         */
        QString scheduleIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Run Script'-icon path.
         *
         * @returns the 'Run Script'-icon path
         */
        QString runIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'Run Script'-icon style.
         *
         * @returns the 'Run Script'-icon style
         */
        QString runIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Duplicate'-icon path.
         *
         * @returns the 'Duplicate'-icon path
         */
        QString contentIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'Duplicate'-icon style.
         *
         * @returns the 'Duplicate'-icon style
         */
        QString contentIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Settings'-icon path.
         *
         * @returns the 'Settings'-icon path
         */
        QString settingsIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'Settings'-icon style.
         *
         * @returns the 'Settings'-icon style
         */
        QString settingsIconStyle() const;


        /**
         * Q_PROPERTY WRITE function for the hal icon path.
         *
         * @param path - the new path
         * @returns the hal icon path
         */
        void setHalIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the hal icon style.
         *
         * @param style - the new style
         * @returns the hal icon style
         */
        void setHalIconStyle(const QString &style);

        /**
         * Q_PROPERTY WRITE function for the 'New File'-icon path.
         *
         * @param path - the new path
         * @returns the 'New File'-icon path
         */
        void setNewFileIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'New File'-icon style.
         *
         * @param style - the new style
         * @returns the 'New File'-icon style
         */
        void setNewFileIconStyle(const QString &style);

        /**
        * Q_PROPERTY WRITE function for the 'Open File'-icon path.
         *
         * @param path - the new path
        * @returns the 'Open File'-icon path
        */
        void setOpenIconPath(const QString& path);
        /**
        * Q_PROPERTY WRITE function for the 'Open File'-icon style.
         *
         * @param style - the new style
        * @returns the 'Open File'-icon style
        */
        void setOpenIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Save File'-icon path.
         *
         * @param path - the new path
         * @returns the 'SaveFile'-icon path
         */
        void setSaveIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Save File'-icon style.
         *
         * @param style - the new style
         * @returns the 'Save File'-icon style
         */
        void setSaveIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Schedule'-icon path.
         *
         * @param path - the new path
         * @returns the 'Schedule'-icon path
         */
        void setScheduleIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Schedule'-icon style.
         *
         * @param style - the new style
         * @returns the 'Schedule'-icon style
         */
        void setScheduleIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Run Script'-icon path.
         *
         * @param path - the new path
         * @returns the 'Run Script'-icon path
         */
        void setRunIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Duplicate'-icon path.
         *
         * @param style - the new style
         * @returns the 'Duplicate'-icon path
         */
        void setRunIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Duplicate'-icon path.
         *
         * @param path - the new path
         * @returns the 'Duplicate'-icon path
         */
        void setContentIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Duplicate'-icon style.
         *
         * @param style - the new style
         * @returns the 'Duplicate'-icon style
         */
        void setContentIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Settings'-icon path.
         *
         * @param path - the new path
         * @returns the 'Settings'-icon path
         */
        void setSettingsIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Settings'-icon style.
         *
         * @param style - the new style
         * @returns the 'Settings'-icon style
         */
        void setSettingsIconStyle(const QString& style);

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted, after the current project is stored as a .hal file.
         */
        void saveTriggered();

    public Q_SLOTS:
        /**
         * Q_SLOT to quit the program.
         */
        void onActionQuitTriggered();

        /**
         * Q_SLOT to close the current document.
         * It is called after the file manager closed the file.
         */
        void onActionCloseDocumentTriggered();

        /**
         * Q_SLOT to run a plugin.
         *
         * @param name - the name of the plugin
         */
        void runPluginTriggered(const QString& name);

        /**
         * Q_SLOT to open (toggle to the) plugin schedule menu. (Currently unused)
         */
        void toggleSchedule();

        /**
         * Q_SLOT to open (toggle to the) settings menu.
         */
        void toggleSettings();

        /**
         * Q_SLOT to close the settings menu.
         */
        void closeSettings();

        /**
         * Q_SLOT to create a new empty netlist. Asks the user for the gate_library.
         * Emits FileManager::fileOpened() on success.
         */
        void handleActionNew();

        /**
         * Q_SLOT to open a netlist. Asks the user for the .hal/.vhdl/.v file to open.
         */
        void handleActionOpen();

        /**
         * Q_SLOT to switch to the layout area after a file was opened (or a new one was created).
         *
         * @param fileName
         */
        void handleFileOpened(const QString& fileName);

        /**
         * Q_SLOT to save the current project as a .hal file.
         * Emits saveTriggered().
         */
        void handleSaveTriggered();

        /**
         * Q_SLOT to provide a logic for the close action. (Currently unused)
         */
        void handleActionClosed();

    private:
        /**
         * Overwritten Qt function to handle the close event.
         * Notifies the user about unsaved changes.
         *
         * @param event - Qts close event
         */
        void closeEvent(QCloseEvent* event);

        /**
         * Restores the window geometry of the MainWindow and all its children, configured with saveState().
         */
        void restoreState();

        /**
         * Saves the window geometry of the MainWindow and all its children.
         * This state can be restored calling saveState().
         */
        void saveState();

        QVBoxLayout* mLayout;
        QMenuBar* mMenuBar;
        QStackedWidget* mStackedWidget;

        PluginScheduleWidget* mScheduleWidget;
        MainSettingsWidget* mSettings;
        WelcomeScreen* mWelcomeScreen;
        QHBoxLayout* mToolBarLayout;
        QToolBar* mLeftToolBar;
        QToolBar* mRightToolBar;
        ContentLayoutArea* mLayoutArea;

        Action* mActionNew;
        Action* mActionOpen;
        Action* mActionSave;
        Action* mActionSaveAs;
        Action* mActionAbout;
   //     Action* mActionSchedule;
   //     Action* mActionRunSchedule;
   //     Action* mActionContent;
        Action* mActionSettings;
        Action* mActionClose;

        QMenu* mMenuFile;
        QMenu* mMenuEdit;
        QMenu* mMenuHelp;

        AboutDialog* mAboutDialog;

        PluginModel* mPluginModel;

        QString mHalIconStyle;
        QString mHalIconPath;

        QString mNewFileIconStyle;
        QString mNewFileIconPath;

        QString mOpenIconPath;
        QString mOpenIconStyle;

        QString mSaveIconPath;
        QString mSaveIconStyle;

        QString mScheduleIconPath;
        QString mScheduleIconStyle;

        QString mRunIconPath;
        QString mRunIconStyle;

        QString mContentIconPath;
        QString mContentIconStyle;

        QString mSettingsIconPath;
        QString mSettingsIconStyle;
    };
}
