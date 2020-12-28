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

#include "gui/code_editor/code_editor.h"
#include "gui/content_widget/content_widget.h"
#include "hal_core/utilities/hal_file_manager.h"
#include "gui/file_modified_bar/file_modified_bar.h"
#include "gui/action/action.h"
#include "gui/python/python_context_subscriber.h"

#include <QEvent>
#include <QFileSystemWatcher>
#include <QMap>
#include <QPushButton>
#include <QtCore/QFileInfo>
#include <QMessageBox>

class QVBoxLayout;
class QTabWidget;

namespace hal
{
    class CodeEditor;
    class Searchbar;
    class Splitter;
    class Toolbar;

    class PythonCodeEditor;

    /**
     * Widget that contains the python editor. It manages all CodeEditor objects (open tabs). The logic to save, open
     * and execute scripts as well as the automatically creation of snapshot files (backups) is handles by this class.
     */
    class PythonEditor : public ContentWidget, public PythonContextSubscriber
    {
        Q_OBJECT
        Q_PROPERTY(QString openIconPath READ openIconPath WRITE setOpenIconPath)
        Q_PROPERTY(QString openIconStyle READ openIconStyle WRITE setOpenIconStyle)
        Q_PROPERTY(QString saveIconPath READ saveIconPath WRITE setSaveIconPath)
        Q_PROPERTY(QString saveIconStyle READ saveIconStyle WRITE setSaveIconStyle)
        Q_PROPERTY(QString saveAsIconPath READ saveAsIconPath WRITE setSaveAsIconPath)
        Q_PROPERTY(QString saveAsIconStyle READ saveAsIconStyle WRITE setSaveAsIconStyle)
        Q_PROPERTY(QString runIconPath READ runIconPath WRITE setRunIconPath)
        Q_PROPERTY(QString runIconStyle READ runIconStyle WRITE setRunIconStyle)
        Q_PROPERTY(QString newFileIconPath READ newFileIconPath WRITE setNewFileIconPath)
        Q_PROPERTY(QString newFileIconStyle READ newFileIconStyle WRITE setNewFileIconStyle)
        Q_PROPERTY(QString toggleMinimapIconPath READ toggleMinimapIconPath WRITE setToggleMinimapIconPath)
        Q_PROPERTY(QString toggleMinimapIconStyle READ toggleMinimapIconStyle WRITE setToggleMinimapIconStyle)

    public:
        explicit PythonEditor(QWidget* parent = nullptr);
        ~PythonEditor();

        /**
         * Setups the toolbar of this content widget. Overwrites the function ContentWidget::setupToolbar.
         *
         * @param Toolbar - The toolbar to set up
         */
        virtual void setupToolbar(Toolbar* Toolbar) Q_DECL_OVERRIDE;

        /**
         * Create the shortcuts for this content widget. Overwrites the function ContentWidget::createShortcuts.
         *
         * @returns the list of created (and connected) shortcuts
         */
        virtual QList<QShortcut*> createShortcuts() Q_DECL_OVERRIDE;

        virtual void handleStdout(const QString& output) Q_DECL_OVERRIDE;
        virtual void handleError(const QString& output) Q_DECL_OVERRIDE;
        virtual void clear() Q_DECL_OVERRIDE;

        /**
         * Action to open a file. A dialogue will appear where the user can select the .py file he want to open.
         * In case the user opens an already open file the user is ask whether the changes should be overwritten or not.
         */
        void handleActionOpenFile();

        /**
         * Action to save the currently selected code editor (open tab). If the current tab is a new file the user is
         * ask for a path to save it.
         */
        void handleActionSaveFile();

        /**
         * Action to save the currently selected code editor (open tab). The user is ask where to save it.
         */
        void handleActionSaveFileAs();

        /**
         * Action to execute the python script of the currently selected code editor (open tab).
         */
        void handleActionRun();

        /**
         * Action to create an empty new tab.
         */
        void handleActionNewTab();

        /**
         * Action to create the menu when right-clicking on a tab. There the user may choose to close certain tabs or
         * open the directory of the selected tab in the system explorer.
         */
        void handleActionTabMenu();

        /**
         * Action to close the tab the mouse is on.
         */
        void handleActionCloseTab();

        /**
         * Action to close all opened tabs.
         * The user will be informed about all unsaved tabs that would be closed and may cancel the action.
         */
        void handleActionCloseAllTabs();

        /**
         * Action to close all tabs but the selected one (the mouse is on).
         * The user will be informed about all unsaved tabs that would be closed and may cancel the action.
         */
        void handleActionCloseOtherTabs();

        /**
         * Action to close all tabs that are positioned left from the one the mouse is on.
         * The user will be informed about all unsaved tabs that would be closed and may cancel the action.
         */
        void handleActionCloseLeftTabs();

        /**
         * Action to close all tabs that are positioned right from the one the mouse is on.
         * The user will be informed about all unsaved tabs that would be closed and may cancel the action.
         */
        void handleActionCloseRightTabs();

        /**
         * Action to open the directory of the file, the mouse is on, in the explorer.
         */
        void handleActionShowFile();

        /**
         * Open a file in the tab given by its index. Later on, all saved modification will be applied to the opened file.
         *
         * @param index - The index of the tab the file should opened in.
         * @param fileName - The absolute path of the file to open.
         */
        void tabLoadFile(u32 index, QString fileName);

        //added so that the speciallogcontentmanager has access to all the code editors
        /**
         * Access the underlying QTabWidget object, this class uses to handle the PythonCodeEditor tabs.
         *
         * @returns the QTabWidget object.
         */
        QTabWidget* getTabWidget();

        /**
         * Saves a tab given by its index. If ask_path is true, the user is ask for a new save location.
         *
         * @param ask_path - Ask the user for a new save location
         * @param index - The tab index
         * @returns true if the tab was saved
         */
        bool saveFile(const bool ask_path, int index = -1);

        /**
         * Close the tab given by its index. It will discard all unsaved changes of this tab without asking.
         *
         * @param index - The index of the tab
         */
        void discardTab(int index);

        /**
         * Check for a given index range, if all tabs of these indexes should be discarded. If there are no unsaved
         * changes they can be discarded. Otherwise the user is informed about unsaved tabs an may choose whether to
         * discard all changes or not.
         * The range will by passed in python style, e.g. start=1, end=5 includes indices {1,2,3,4}
         * Also the exclude parameter can be used to skip a certain index,
         * e.g. start=1, end=5, exclude=2 includes indices {1,3,4}
         *
         * @param start - The start position. (Tab index)
         * @param end - The end position. (Tab index) (not included)
         * @param exclude - May be passed to skip a certain index. Pass -1 to not exclude any indices.
         * @returns true if the tabs should be discarded.
         */
        bool confirmDiscardForRange(int start, int end, int exclude = -1);

        // =====================================================================
        //   Q_PROPERTY functions
        // =====================================================================

        /** @name Q_PROPERTY Functions
         */
        ///@{

        /**
         * Q_PROPERTY READ function for the 'open file'-icon path.
         *
         * @returns the 'open file'-icon path
         */
        QString openIconPath() const;

        /**
         * Q_PROPERTY READ function for the 'open file'-icon style.
         *
         * @returns the 'open file'-icon style
         */
        QString openIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'save file'-icon path.
         *
         * @returns the 'save file'-icon path
         */
        QString saveIconPath() const;

        /**
         * Q_PROPERTY READ function for the 'save file'-icon style.
         *
         * @returns the 'save file'-icon style
         */
        QString saveIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'save as'-icon path.
         *
         * @returns the 'save as'-icon path
         */
        QString saveAsIconPath() const;

        /**
         * Q_PROPERTY READ function for the 'save as'-icon style.
         *
         * @returns the 'save as'-icon style
         */
        QString saveAsIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'run'-icon path.
         *
         * @returns the 'run'-icon path
         */
        QString runIconPath() const;

        /**
         * Q_PROPERTY READ function for the 'run'-icon style.
         *
         * @returns the 'run'-icon style
         */
        QString runIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'new file'-icon path.
         *
         * @returns the 'new file'-icon path
         */
        QString newFileIconPath() const;

        /**
         * Q_PROPERTY READ function for the 'new file'-icon style.
         *
         * @returns the 'new file'-icon style
         */
        QString newFileIconStyle() const;

        /**
         * Q_PROPERTY READ function for the 'toggle minimap'-icon path.
         *
         * @returns the 'toggle minimap'-icon path
         */
        QString toggleMinimapIconPath() const;

        /**
         * Q_PROPERTY READ function for the 'toggle minimap'-icon style.
         *
         * @returns the 'toggle minimap'-icon style
         */
        QString toggleMinimapIconStyle() const;

        /**
         * Q_PROPERTY WRITE function for the 'open file'-icon path.
         *
         * @param path - the new path
         */
        void setOpenIconPath(const QString& path);

        /**
         * Q_PROPERTY WRITE function for the 'open file'-icon style.
         *
         * @param style - the new style
         */
        void setOpenIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'save file'-icon path.
         *
         * @param path - the new path
         */
        void setSaveIconPath(const QString& path);

        /**
         * Q_PROPERTY WRITE function for the 'save file'-icon style.
         *
         * @param style - the new style
         */
        void setSaveIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'save as'-icon path.
         *
         * @param path - the new path
         */
        void setSaveAsIconPath(const QString& path);

        /**
         * Q_PROPERTY WRITE function for the 'save as'-icon style.
         *
         * @param style - the new style
         */
        void setSaveAsIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'run'-icon path.
         *
         * @param path - the new path
         */
        void setRunIconPath(const QString& path);

        /**
         * Q_PROPERTY WRITE function for the 'run'-icon style.
         *
         * @param style - the new style
         */
        void setRunIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'new file'-icon path.
         *
         * @param path - the new path
         */
        void setNewFileIconPath(const QString& path);

        /**
         * Q_PROPERTY WRITE function for the 'new file'-icon style.
         *
         * @param style - the new style
         */
        void setNewFileIconStyle(const QString& style);

        /**
         * Q_PROPERTY WRITE function for the 'toggle minimap'-icon path.
         *
         * @param path - the new path
         */
        void setToggleMinimapIconPath(const QString& path);

        /**
         * Q_PROPERTY WRITE function for the 'toggle minimap'-icon style.
         *
         * @param style - the new style
         */
        void setToggleMinimapIconStyle(const QString& style);
        ///@}

        /**
         * Serialize the necessary information to the .hal file.
         * The file paths of the currently opened tabs were stores, as well as the index of the selected tab is saved.
         *
         * @param path - Unused
         * @param netlist - The netlist to serialize (unused)
         * @param document - The json document of the .hal file
         * @returns true on success
         */
        bool handleSerializationToHalFile(const std::filesystem::path& path, Netlist* netlist, rapidjson::Document& document);

        /**
         * Deserialize the necessary information from the .hal file that is loaded.
         * All tabs that appear in the .hal file were opened.
         *
         * @param path - Unused
         * @param netlist - The deserialization target
         * @param document - The json document of the .hal file
         * @returns true on success
         */
        bool handleDeserializationFromHalFile(const std::filesystem::path& path, Netlist* netlist, rapidjson::Document& document);

    Q_SIGNALS:
        void forwardStdout(const QString& output);
        void forwardError(const QString& output);

    public Q_SLOTS:
        /**
         * Q_SLOT to open/close the searchbar. (Connected to Strg+F by default)
         */
        void toggleSearchbar();

        /**
         * Q_SLOT to close the tab given by its index. If there are unsaved changes the user is ask whether to save
         * the changes, discard the changes or cancel the closing.
         * @param index
         */
        void handleTabCloseRequested(int index);

        /**
         * Q_SLOT to open/close the minimap (code outline).
         */
        void handleActionToggleMinimap();

        /**
         *  Unused Q_SLOT?
         *
         * @param changed
         */
        void handleModificationChanged(bool changed);

        /**
         * Q_SLOT to handle key presses within any PythonCodeEditor
         */
        void handleKeyPressed();

        /**
         * Q_SLOT to handle changed texts within any PythonCodeEditor. Used to mark that the tab was modified.
         */
        void handleTextChanged();

        /**
         * Q_SLOT to handle changes in the searchbar text. Used to update the search logic.
         */
        void handleSearchbarTextEdited(const QString& text);

        /**
         * Q_SLOT to handle that the index of a tab changes (e.g. because it was moved with the mouse)
         */
        void handleCurrentTabChanged(int index);

        /**
         * Q_SLOT to handle that the original file of an opened tab was modified (outside from hal).
         * A bar will appear that ask the user whether to reload the file or ignore the changes.
         * If the original file was moved or deleted the user will be informed.
         */
        void handleTabFileChanged(QString path);

        /**
         * Q_SLOT to handle that user chooses to reload the file that was modified from outside (outside from hal).
         */
        void handleBaseFileModifiedReload();
        /**
         * Q_SLOT to handle that user chooses to ignore the changes of a file that was modified
         * from outside (outside from hal).
         */
        void handleBaseFileModifiedIgnore();

        /**
         * Q_SLOT called while closing the information bar that informs the user after
         * the original file was moved or deleted.
         */
        void handleBaseFileModifiedOk();

        /**
         * Slot called after a .hal file (or a .v/.vhdl is parsed) (emitted by FileManager). Used to check for
         * existing snapshots.
         *
         * @param fileName - the opened file
         */
        void handleFileOpened(QString fileName);

        /**
         * Slot called when a file is about to be closed. (emitted by FileManager). Used to remove snapshots.
         */
        void handleFileAboutToClose(const QString& fileName);

    protected:
        /**
         * QObject event filter overwritten by the python editor. Used to capture right-clicks on the tabs to open
         * the tab menu (see handleActionTabMenu).
         *
         * @param obj - The watches object
         * @param event - The event to filter
         * @return
         */
        bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;

    private:
        /**
         * Ask the user with a message box whether to save the tab, discard the changes or cancel the request.
         *
         * @param tab_index - the index of the closed tab
         * @return QMessageBox::Save if 'Save' was clicked, QMessageBox::Discard if 'Discard' was clicked or
         *         QMessageBox::Cancel if 'Cancel' was clicked
         */
        QMessageBox::StandardButton askSaveTab(const int tab_index) const;

        // ============ FUNCTIONS FOR SNAPSHOT HANDLING ============

        /**
         * Parse a snapshot file
         *
         * @param snapshot_file_path - The path of the snapshot file
         * @returns a pair <original file path, snapshot content>
         */
        QPair<QString, QString> readSnapshotFile(QFileInfo snapshot_file_path) const;

        /**
         * Parse all found and store them in a single map
         *
         * @param snapshot_file_path - The path of the snapshot file
         * @returns a pair which contains all snapshots:
         *          fist: contains a map: original file path -> snapshot content
         *          second: a vector with snapshots with no original file path (unsaved tabs)
         */
        QPair<QMap<QString, QString>, QVector<QString>> loadAllSnapshots();

        /**
         * Writes a snapshot file
         *
         * @param snapshot_file_path - The path of the snapshot file
         * @param original_file_path  - The original file the snapshot is made for
         * @param content - The content to write
         * @returns true on success
         */
        bool writeSnapshotFile(QFileInfo snapshot_file_path, QString original_file_path , QString content) const;

        /**
         * Get the path to the directory where the snapshots for this netlist are stored
         *
         * @param create_if_non_existent if true, the directory will be created if not already existent.
         * @returns the snapshot directory. If the directory does not exist (with create_if_non_existent = false) or
         *          cannot be created, an empty string is returned
         */
        QString getSnapshotDirectory(const bool create_if_non_existent = true);

        /**
         * Updates the snapshots for all open tabs
         *
         * @param create_if_non_existent
         */
        void updateSnapshots();

        /**
         * Clear all snapshots
         *
         * @param remove_dir - If the parent directory (i.e. "<netlist_name>/") should be removed as well.
         *                     Used only at the end of the program
         */
        void clearAllSnapshots(bool remove_dir = false);

        /**
         * Decide whether the snapshot should be loaded/inserted or the original file should be used.
         * If both the original file and the snapshot file exist, the user decides which version should be loaded.
         *
         * @param saved_snapshots - The map (original_path -> content) of saved snapshots (snapshots of non empty paths)
         * @param original_path - The path of the original version
         * @return true if the snapshot file should be loaded/inserted
         */
        bool decideLoadSnapshot(const QMap<QString, QString>& saved_snapshots, const QFileInfo original_path) const;

        /**
         * Set the content of a Python editor to the content of a snapshot. The tab is marked as modified afterwards.
         *
         * @param idx - The index of the python editor, the snapshots should be loaded in
         * @param snapshot_content - The content of the snapshot
         */
        void setSnapshotContent(const int idx, const QString snapshot_content);

        /**
         * Ask the user with a message box whether the snapshot file or the original file should be loaded
         *
         * @param original_path - the path of the original file
         * @param original_content - the content of the original file
         * @param snapshot_content - the content of the snapshot file
         * @return true if the snapshot file should be loaded
         */
        bool askLoadSnapshot(const QString original_path, const QString original_content, const QString snapshot_content) const;

        /**
         * Ask the user with a message box whether the snapshot files should be ignored and deleted or not.
         * This box appers if the user reparses a .v/.vhdl file, but there are still snapshots
         *
         * @param original_path - the found snapshots
         * @return true if the snapshot file should be ignored and deleted. False if the snapshots should be loaded.
         */
        bool askDeleteSnapshots(const QPair<QMap<QString, QString>, QVector<QString>>& snapshots) const;

        /**
         * Remove a snapshot file for a given file path. If there is not such snapshot, do nothing.
         *
         * @param original_path
         */
        void removeSnapshotFile(PythonCodeEditor* editor) const;

        QVBoxLayout* mLayout;
        Toolbar* mToolbar;
        Splitter* mSplitter;

        Searchbar* mSearchbar;

        Action* mActionOpenFile;
        Action* mActionRun;
        Action* mActionSave;
        Action* mActionSaveAs;
        Action* mActionToggleMinimap;
        Action* mActionNewFile;

        QString mOpenIconStyle;
        QString mOpenIconPath;

        QString mSaveIconStyle;
        QString mSaveIconPath;

        QString mSaveAsIconStyle;
        QString mSaveAsIconPath;

        QString mRunIconStyle;
        QString mRunIconPath;

        QString mNewFileIconStyle;
        QString mNewFileIconPath;

        QString mToggleMinimapIconStyle;
        QString mToggleMinimapIconPath;

        QTabWidget* mTabWidget;
        int mTabRightclicked = -1;

        QFileSystemWatcher* mFileWatcher;
        QMap<QString, PythonCodeEditor*> mPathEditorMap;

        FileModifiedBar* mFileModifiedBar;

        int mNewFileCounter;

        long mLastClickTime;

        QString mLastOpenedPath;

        /**
         * Stores where the snapshots for the tabs are located
         * Map: tab -> snapshot path
         */
        QMap<PythonCodeEditor*, QString> mTabToSnapshotPath;
    };
}
