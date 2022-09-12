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

#include "hal_core/utilities/program_arguments.h"

#include <QObject>
#include <QTimer>
#include <QDir>

class QFileSystemWatcher;

namespace hal
{
    class SettingsItemCheckbox;
    class SettingsItemSpinbox;
   /**
    * @ingroup file_management
    * @brief Stores information about the currently opened netlist.
    *
     * The FileManager handles the status and information of the currently opened netlist file
     * including whether a file is opened or not. This class is implemented with a singleton pattern.
     */
    class FileManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Get the singleton instance of the filemanager.
         *
         * @returns The filemanager's instance.
         */
        static FileManager* get_instance();

        /**
         * Get the filename of the currently opened file. When no file is open an empty string is returned.
         *
         * @return The filename.
         */
        QString fileName() const;

        /**
         * Get the status if a file is currently open or not.
         *
         * @return True when a file is open.
         */
        bool fileOpen() const;

        /**
         * Watches the given file so that it is possible to notice modifications if changes were made outside of hal.
         * This function call also sets all important information about the file as well as
         * starts the timer to save the project in regular intervals.
         *
         * @param fileName - The file to be watched.
         */
        void watchFile(const QString& fileName);

        /**
         * This function is called by openDirectory when the hal was saved.
         *
         * @param projectDir - the project directory
         * @param file - the file name
         */
        void emitProjectSaved(QString& projectDir, QString& file);

        /**
         * Return values for static method directoryStatus
         */
        enum DirectoryStatus { ProjectDirectory=2, OtherDirectory=1, IsFile=0, NotSelectable=-1, NotExisting=-2 };

        /**
         * This function checks whether a given path name is a hal directory (contains .project.json)
         * @param pathname - the path name to be tested
         * @return DirectoryStatus enum (see above)
         */
        static DirectoryStatus directoryStatus(const QString& pathname);
    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when a file is successfully opened.
         *
         * @param fileName - The filename.
         */
        void fileOpened(const QString& fileName);

        /**
         * Q_SIGNAL that is emitted when a project is successfully opened.
         *
         * @param projectDir - The project directory
         * @param fileName - The filename.
         */
        void projectOpened(QString projectDir, QString fileName);

        /**
         * Q_SIGNAL that is emitted when a project gets saved.
         *
         * @param projectDir - The project directory
         * @param fileName - The filename.
         */
        void projectSaved(QString projectDir, QString fileName);

        /**
         * Q_SIGNAL that is emitted when the file changed.
         *
         * @param path - The new file.
         */
        void fileChanged(const QString& path);

        /**
         * Q_SIGNAL that is emitted when the file directory changed.
         *
         * @param path - The new directory.
         */
        void fileDirectoryChanged(const QString& path);

        /**
         * Q_SIGNAL that is emitted when the file was successfully closed.
         */
        void fileClosed();

        /**
         * Q_SIGNAL that is emitted when the file is about to be closed.
         *
         * @param fileName - The name of the file that will be closed.
         */
        void fileAboutToClose(const QString& fileName);

    public Q_SLOTS:

        /**
         * Opens the given filename. It handles the initialization and deserialization process of the netlists as well
         * as the shadowfile logic in an automated fashion.
         *
         * @param fileName - The file to be opened.
         * @param gatelibraryPath - Path to gate library, auto detection if empty
         */
        bool deprecatedOpenFile(QString filename, QString gatelibraryPath);


        /**
         * Imports the netlist file into a new hal project. User will be queried for project directory.
         *
         * @param filename - The netlist file to be imported.
         */
        void importFile(QString filename);


        /**
         * Opens the given project.
         *
         * @param projPath - The directory path of the project to be opened
         */
        void openProject(QString projPath);

        /**
         * Closes the file and resets all information regarding the closed file.
         */
        void closeFile();

        /**
         * Serializes the project to the shadowfile. Called in regular intervals that can be set via the settings.
         */
        void autosave();

    private Q_SLOTS:
        void handleFileChanged(const QString& path);
        void handleDirectoryChanged(const QString& path);

    private:
        /**
         * The private constructor for the singleton pattern.
         *
         * @param parent - The filemanager's parent.
         */
        FileManager(QObject* parent = nullptr);

        /**
         * This function is called by openFile when the netlist could be serialized successfully.
         *
         * @param file - the file's name.
         */
        void fileSuccessfullyLoaded(QString file);

        /**
         * This function is called by openDirectory when the hal project could be loaded successfully.
         *
         * @param projectDir - the project directory
         * @param file - the file name
         */
        void projectSuccessfullyLoaded(QString& projectDir, QString& file);

        /**
         * Sets the opened file at the top of the recently used files in the welcome screen through a setting file
         * that is saved in the gGuiState variable.
         *
         * @param file - The opened file.
         */
        void updateRecentFiles(const QString& file) const;

        /**
         * Displays the given error message in a messagebox.
         *
         * @param error_message - The message to be displayed.
         */
        void displayErrorMessage(QString error_message);

        /**
         * Deletes shadow (=autosave) project directory if existing.
         */
        void removeShadowDirectory();

        /**
         * Crash repair: recursively move shadow (=autosave) files into project directory.
         * @param shDir - The shadow directory
         */
        void moveShadowToProject(const QDir& shDir) const;

        QString mFileName;
        QFileSystemWatcher* mFileWatcher;
        bool mFileOpen;
        QTimer* mTimer;
        bool mAutosaveEnabled;
        int mAutosaveInterval;

        SettingsItemCheckbox* mSettingAutosave;
        SettingsItemSpinbox* mSettingAutosaveInterval;
    };
}
