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

        virtual void setupToolbar(Toolbar* Toolbar) Q_DECL_OVERRIDE;
        virtual QList<QShortcut*> createShortcuts() Q_DECL_OVERRIDE;

        virtual void handleStdout(const QString& output) Q_DECL_OVERRIDE;
        virtual void handleError(const QString& output) Q_DECL_OVERRIDE;
        virtual void clear() Q_DECL_OVERRIDE;

        void handleActionOpenFile();
        void handleActionSaveFile();
        void handleActionSaveFileAs();
        void handleActionRun();
        void handleActionNewTab();
        void handleActionTabMenu();
        void handleActionCloseTab();
        void handleActionCloseAllTabs();
        void handleActionCloseOtherTabs();
        void handleActionCloseLeftTabs();
        void handleActionCloseRightTabs();
        void handleActionShowFile();
        void tabLoadFile(u32 index, QString fileName);

        /**
         * Used to forcefully save all python scripts on closing the main window. By manipulating the QCloseEvent
         * the closing of the main window can be aborted by the user.
         *
         * @param event - The QCloseEvent from the (!!!) MainWindow Object.
         */
        void handleMainWindowClose(QCloseEvent* event);

        //added so that the speciallogcontentmanager has access to all the code editors
        QTabWidget* getTabWidget();

        /**
         * Saves a tab given by its index. If ask_path is true, the user is ask for a new save location.
         *
         * @param ask_path - ask the user for a new save location
         * @param index - the tab index
         * @returns true if the tab was saved
         */
        bool saveFile(const bool ask_path, int index = -1);

        void discardTab(int index);
        bool confirmDiscardForRange(int start, int end, int exclude = -1);

        QString openIconPath() const;
        QString openIconStyle() const;

        QString saveIconPath() const;
        QString saveIconStyle() const;

        QString saveAsIconPath() const;
        QString saveAsIconStyle() const;

        QString runIconPath() const;
        QString runIconStyle() const;

        QString newFileIconPath() const;
        QString newFileIconStyle() const;

        QString toggleMinimapIconPath() const;
        QString toggleMinimapIconStyle() const;

        void setOpenIconPath(const QString& path);
        void setOpenIconStyle(const QString& style);

        void setSaveIconPath(const QString& path);
        void setSaveIconStyle(const QString& style);

        void setSaveAsIconPath(const QString& path);
        void setSaveAsIconStyle(const QString& style);

        void setRunIconPath(const QString& path);
        void setRunIconStyle(const QString& style);

        void setNewFileIconPath(const QString& path);
        void setNewFileIconStyle(const QString& style);

        void setToggleMinimapIconPath(const QString& path);
        void setToggleMinimapIconStyle(const QString& style);

        bool handleSerializationToHalFile(const std::filesystem::path& path, Netlist* netlist, rapidjson::Document& document);
        bool handleDeserializationFromHalFile(const std::filesystem::path& path, Netlist* netlist, rapidjson::Document& document);



    Q_SIGNALS:
        void forwardStdout(const QString& output);
        void forwardError(const QString& output);

    public Q_SLOTS:
        void toggleSearchbar();
        void handleTabCloseRequested(int index);
        void handleActionToggleMinimap();
        void handleModificationChanged(bool changed);
        void handleKeyPressed();
        void handleTextChanged();
        void handleSearchbarTextEdited(const QString& text);
        void handleCurrentTabChanged(int index);
        void handleTabFileChanged(QString path);

        void handleBaseFileModifiedReload();
        void handleBaseFileModifiedIgnore();
        void handleBaseFileModifiedOk();

    protected:
        bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;

    private:
        /**
         * Ask the user with a message box whether the snapshot file or the original file should be loaded
         *
         * @param file - the snapshot file
         * @return true if the snapshot file should be loaded
         */
        bool askLoadSnapshot(const QFileInfo original_path, const QFileInfo snapshot_path) const;

        /**
         * Ask the user with a message box whether to save the tab, discard the changes or cancel the request.
         *
         * @param tab_index - the index of the closed tab
         * @return QMessageBox::Save if 'Save' was clicked, QMessageBox::Discard if 'Discard' was clicked or
         *         QMessageBox::Cancel if 'Cancel' was clicked
         */
        QMessageBox::StandardButton askSaveTab(const int tab_index) const;

        /**
         * Ask the user with a message box whether to save the tab, or cancel the request. The user is not allowed to
         * discard changes. This request is used to forcefully save all tabs while closing hal.
         *
         * @param tab_index - the index of the closed tab
         * @return QMessageBox::Save if 'Save' was clicked or QMessageBox::Cancel if 'Cancel' was clicked
         */
        QMessageBox::StandardButton askForceSaveTab(const int tab_index) const;

        /**
         * Remove a snapshot file for a given file path. If there is not such snapshot, do nothing.
         *
         * @param original_path
         */
        void removeSnapshotFile(const QFileInfo original_path) const;



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
    };
}
