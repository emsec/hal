// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/utilities/program_options.h"

#include "gui/content_layout_area/content_layout_area.h"
#include "hal_core/utilities/program_options.h"
#include "gui/action/action.h"
#include "gui/settings/main_settings_widget.h"
#include "gui/splitter/splitter.h"

#include <QLayout>
#include <QMenuBar>
#include <QObject>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>

namespace hal
{
    class plugin_manager_dialog;
    class PythonEditor;
    class FileManager;
    class ContentManager;
    class WelcomeScreen;
    class SettingsItemKeybind;
    class SettingsItemDropdown;
    class GuiPluginManager;

    /**
     * @ingroup gui
     * @brief The top level widget.
     *
     * <i>One widget to rule them all, <br>
     *   one widget to create them, <br>
     * One widget to bring them all <br>
     *   and in the dark theme bind them. </i>
     */
    class MainWindow : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString halIconPath READ halIconPath WRITE setHalIconPath)
        Q_PROPERTY(QString halIconStyle READ halIconStyle WRITE setHalIconStyle)

        Q_PROPERTY(QString newFileIconPath READ newFileIconPath WRITE setNewFileIconPath)
        Q_PROPERTY(QString newFileIconStyle READ newFileIconStyle WRITE setNewFileIconStyle)
        Q_PROPERTY(QString openFileIconPath READ openFileIconPath WRITE setOpenFileIconPath)
        Q_PROPERTY(QString openFileIconStyle READ openFileIconStyle WRITE setOpenFileIconStyle)
        Q_PROPERTY(QString openProjIconPath READ openProjIconPath WRITE setOpenProjIconPath)
        Q_PROPERTY(QString openProjIconStyle READ openProjIconStyle WRITE setOpenProjIconStyle)
        Q_PROPERTY(QString saveIconPath READ saveIconPath WRITE setSaveIconPath)
        Q_PROPERTY(QString saveIconStyle READ saveIconStyle WRITE setSaveIconStyle)
        Q_PROPERTY(QString saveAsIconPath READ saveAsIconPath WRITE setSaveAsIconPath)
        Q_PROPERTY(QString saveAsIconStyle READ saveAsIconStyle WRITE setSaveAsIconStyle)
        Q_PROPERTY(QString closeIconPath READ closeIconPath WRITE setCloseIconPath)
        Q_PROPERTY(QString closeIconStyle READ closeIconStyle WRITE setCloseIconStyle)
        Q_PROPERTY(QString quitIconPath READ quitIconPath WRITE setQuitIconPath)
        Q_PROPERTY(QString quitIconStyle READ quitIconStyle WRITE setQuitIconStyle)
        Q_PROPERTY(QString settingsIconPath READ settingsIconPath WRITE setSettingsIconPath)
        Q_PROPERTY(QString settingsIconStyle READ settingsIconStyle WRITE setSettingsIconStyle)
        Q_PROPERTY(QString pluginsIconPath READ pluginsIconPath WRITE setPluginsIconPath)
        Q_PROPERTY(QString pluginsIconStyle READ pluginsIconStyle WRITE setPluginsIconStyle)
        Q_PROPERTY(QString undoIconPath READ undoIconPath WRITE setUndoIconPath)
        Q_PROPERTY(QString undoIconStyle READ undoIconStyle WRITE setUndoIconStyle)
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)

    public:
        enum StyleSheetOption {Dark, Light};
        Q_ENUM(StyleSheetOption)
		
        /**
         * The Constructor.
         *
         * @param parent - The parent widget
         */
        explicit MainWindow(QWidget* parent = nullptr);


        // =====================================================================
        //   Q_PROPERTY functions
        // =====================================================================
        /** @name Q_PROPERTY Functions
         */
        ///@{

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
        QString openFileIconPath() const;
        /**
        * Q_PROPERTY READ function for the 'Open File'-icon style.
         *
        * @returns the 'Open File'-icon style
        */
        QString openFileIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Open Proj'-icon path.
         *
         * @returns the 'Open Proj'-icon path
         */
        QString openProjIconPath() const;
        /**
        * Q_PROPERTY READ function for the 'Open Proj'-icon style.
         *
        * @returns the 'Open Proj'-icon style
        */
        QString openProjIconStyle() const;

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
         * Q_PROPERTY READ function for the 'SaveAs File'-icon path.
         *
         * @returns the 'SaveAsFile'-icon path
         */
        QString saveAsIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'SaveAs File'-icon style.
         *
         * @returns the 'SaveAs File'-icon style
         */
        QString saveAsIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Close Project'-icon path.
         *
         * @returns the 'Close Project'-icon path
         */
        QString closeIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'Close Project'-icon style.
         *
         * @returns the 'Close Project'-icon style
         */
        QString closeIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'Quit HAL session'-icon path.
         *
         * @returns the 'Quit HAL session'-icon path
         */
        QString quitIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'Quit HAL session'-icon style.
         *
         * @returns the 'Quit HAL session'-icon style
         */
        QString quitIconStyle() const;

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
         * Q_PROPERTY READ function for the 'plugins'-icon path.
         *
         * @returns the 'plugins'-icon path
         */
        QString pluginsIconPath() const;
        /**
         * Q_PROPERTY READ function for the 'plugins'-icon style.
         *
         * @returns the 'plugins'-icon style
         */
        QString pluginsIconStyle() const;

        QString undoIconPath() const;
        QString undoIconStyle() const;
        QString disabledIconStyle() const;

        /**
         * Q_PROPERTY WRITE function for the hal icon path.
         *
         * @param path - The new path
         */
        void setHalIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the hal icon style.
         *
         * @param style - The new style
         */
        void setHalIconStyle(const QString &style);

        /**
         * Q_PROPERTY WRITE function for the 'New File'-icon path.
         *
         * @param path - The new path
         */
        void setNewFileIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'New File'-icon style.
         *
         * @param style - The new style
         */
        void setNewFileIconStyle(const QString &style);

        /**
        * Q_PROPERTY WRITE function for the 'Open File'-icon path.
         *
         * @param path - The new path
        */
        void setOpenFileIconPath(const QString& path);
        /**
        * Q_PROPERTY WRITE function for the 'Open File'-icon style.
         *
         * @param style - The new style
        */
        void setOpenFileIconStyle(const QString& style);

        /**
        * Q_PROPERTY WRITE function for the 'Open Proj'-icon path.
         *
         * @param path - The new path
        */
        void setOpenProjIconPath(const QString& path);
        /**
        * Q_PROPERTY WRITE function for the 'Open Proj'-icon style.
         *
         * @param style - The new style
        */
        void setOpenProjIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Save File'-icon path.
         *
         * @param path - The new path
         */
        void setSaveIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Save File'-icon style.
         *
         * @param style - The new style
         */
        void setSaveIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'SaveAs File'-icon path.
         *
         * @param path - The new path
         */
        void setSaveAsIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'SaveAs File'-icon style.
         *
         * @param style - The new style
         */
        void setSaveAsIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Close Project'-icon path.
         *
         * @param path - The new path
         */
        void setCloseIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Close Project'-icon style.
         *
         * @param style - The new style
         */
        void setCloseIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Quit HAL session'-icon path.
         *
         * @param path - The new path
         */
        void setQuitIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Quit HAL session'-icon style.
         *
         * @param style - The new style
         */
        void setQuitIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Schedule'-icon path.
         *
         * @param path - The new path
         */
        void setScheduleIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Schedule'-icon style.
         *
         * @param style - The new style
         */
        void setScheduleIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Run Script'-icon path.
         *
         * @param path - The new path
         */
        void setRunIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Duplicate'-icon path.
         *
         * @param style - The new style
         */
        void setRunIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Duplicate'-icon path.
         *
         * @param path - The new path
         */
        void setContentIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Duplicate'-icon style.
         *
         * @param style - The new style
         */
        void setContentIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Settings'-icon path.
         *
         * @param path - The new path
         */
        void setSettingsIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Settings'-icon style.
         *
         * @param style - The new style
         */
        void setSettingsIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'Plugins'-icon path.
         *
         * @param path - The new path
         */
        void setPluginsIconPath(const QString& path);
        /**
         * Q_PROPERTY WRITE function for the 'Plugins'-icon style.
         *
         * @param style - The new style
         */
        void setPluginsIconStyle(const QString& style);

        void setUndoIconPath(const QString& path);
        void setUndoIconStyle(const QString& style);
        void setDisabledIconStyle(const QString &style);

        /**
         * Add a content widget to the layout area.
         *
         * @param widget - The content widget to add
         * @param index - The position index within the anchor, the widget should be put in
         * @param anchor - The anchor, the widget should be assigned to
         */
        void addContent(ContentWidget* widget, int index, content_anchor anchor);

        /**
         * Remove a content widget from the layout area.
         *
         * @param widget - The content widget to remove
         */
        void removeContent(ContentWidget* widget);

        void clear();

        static SettingsItemDropdown* sSettingStyle;

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
         * Q_SLOT to open (toggle to the) settings menu.
         */
        void openSettings();

        /**
         * Q_SLOT to close the settings menu.
         */
        void closeSettings();

        /**
         * Q_SLOT to open (toggle to the) PluginManager menu.
         */
        void openPluginManager();

        /**
         * Q_SLOT to close the PluginManager menu.
         */
        void closePluginManager(const QString& invokeGui);

        /**
         * Q_SLOT to create a new empty netlist. Asks the user for the gate_library.
         * Emits FileManager::fileOpened() on success.
         */
        void handleActionNew();

        /**
         * Q_SLOT to import a netlist. Asks the user for the .hal/.vhdl/.v file to open.
         */
        void handleActionImportNetlist();

        /**
         * Q_SLOT to open a hal project located in project directory.
         */
        void handleActionOpenProject();

        /**
         * Q_SLOT to switch to the layout area after a file was opened (or a new one was created).
         *
         * @param fileName
         */
        void handleFileOpened(const QString& fileName);

        /**
         * Q_SLOT to switch to the layout area after a project was opened (or a new one was created).
         *
         * @param projDir - the project directory
         * @param fileName - the netlist file
         */
        void handleProjectOpened(const QString& projDir, const QString& fileName);

        /**
         * Q_SLOT to save the current project as a .hal file.
         * call to FileManager::emitSaveTriggered().
         */
        void handleSaveTriggered();

        /**
         * Q_SLOT to save the current project as a new .hal file.
         * Will query for new name.
         * call to FileManager::emitSaveTriggered().
         */
        void handleSaveAsTriggered();

        /**
         * Q_SLOT to provide a logic for the close action. (Currently unused)
         */
        void handleActionCloseFile();

        /**
         * Q_SLOT to start macro recording. Internally only a marker is set
         * since user interactions are recorded anyhow.
         */
        void handleActionStartRecording();

        /**
         * Q_SLOT to end macro recording saving user interactions to XML file
         */
        void handleActionStopRecording();

        /**
         * Q_SLOT to play recorded macro. Will query for XML file name.
         */
        void handleActionPlayMacro();

        /**
         * Q_SLOT to undo last interaction if it can be undone
         */
        void handleActionUndo();

        /**
         * Q_SLOT to show information about hal version and license
         */
        void handleActionAbout();

        /**
         * Q_SLOT to export netlist to file. Exporter will be determined by file extension
         */
        void handleActionExport();

        /**
         * Q_SLOT to export netlist to file. Exporter will be determined by file extension
         */
        void handleActionGatelibraryManager();

        void enableUndo(bool enable=true);

        void reloadStylsheet(int istyle);

    private Q_SLOTS:
        /**
         * Private slot to handle event log setting
         * @param enable dump all events to log if set
         */
        void handleEventLogEnabled(bool enable);

        void handleExportProjectTriggered();

        void handleImportProjectTriggered();

    private:
        /**
         * Overwritten Qt function to handle the close event.
         * Notifies the user about unsaved changes.
         *
         * @param event - Qts close event
         */
        void closeEvent(QCloseEvent* event);

        bool tryToCloseFile();

        /**
         * Internal handler for "save" and "save as" commands. If neccessary a new-file dialog
         * will query for hal project directory name.
         *
         * @param project directory, might be empty
         * @return project directory name from new-file dialog (if any)
         */
        QString saveHandler(const QString& projectDir = QString());

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

        MainSettingsWidget* mSettings;
        GuiPluginManager* mPluginManager;
        WelcomeScreen* mWelcomeScreen;
        QHBoxLayout* mToolBarLayout;
        QToolBar* mLeftToolBar;
        QToolBar* mRightToolBar;
        ContentLayoutArea* mLayoutArea;

        Action* mActionNew;
        Action* mActionOpenProject;
        Action* mActionImportNetlist;
        Action* mActionSave;
        Action* mActionSaveAs;
        Action* mActionGateLibraryManager;
        Action* mActionAbout;
        Action* mActionStartRecording;
        Action* mActionStopRecording;
        Action* mActionPlayMacro;
        Action* mActionUndo;
        Action* mActionExportProject;
        Action* mActionImportProject;

        Action* mActionSettings;
        Action* mActionPlugins;
        Action* mActionClose;
        Action* mActionQuit;

        QMenu* mMenuFile;
        QMenu* mMenuEdit;
        QMenu* mMenuMacro;
        QMenu* mMenuHelp;

        QString mHalIconStyle;
        QString mHalIconPath;

        QString mNewFileIconStyle;
        QString mNewFileIconPath;

        QString mOpenFileIconPath;
        QString mOpenFileIconStyle;

        QString mOpenProjIconPath;
        QString mOpenProjIconStyle;

        QString mSaveIconPath;
        QString mSaveIconStyle;

        QString mSaveAsIconPath;
        QString mSaveAsIconStyle;

        QString mCloseIconPath;
        QString mCloseIconStyle;

        QString mQuitIconPath;
        QString mQuitIconStyle;

        QString mSettingsIconPath;
        QString mSettingsIconStyle;
		
        QString mPluginsIconPath;
        QString mPluginsIconStyle;

   		QString mUndoIconPath;
        QString mUndoIconStyle;

       QString mDisabledIconStyle;
 
        SettingsItemKeybind* mSettingCreateFile;
        SettingsItemKeybind* mSettingOpenFile;
        SettingsItemKeybind* mSettingSaveFile;
        SettingsItemKeybind* mSettingUndoLast;

    };
}
