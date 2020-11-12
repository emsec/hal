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

#include "hal_core/utilities/program_arguments.h"

#include <QObject>
#include <QTimer>

class QFileSystemWatcher;

namespace hal
{
    class FileManager : public QObject
    {
        Q_OBJECT

    public:
        static FileManager* get_instance();

        void handleProgramArguments(const ProgramArguments& args);

        QString fileName() const;
        bool fileOpen() const;

        void watchFile(const QString& fileName);


    Q_SIGNALS:
        void fileOpened(const QString& fileName);
        void fileChanged(const QString& path);
        void fileDirectoryChanged(const QString& path);
        void fileClosed();

    public Q_SLOTS:
        void openFile(QString fileName);
        void closeFile();
        void autosave();
        void handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value);

    private Q_SLOTS:
        void handleFileChanged(const QString& path);
        void handleDirectoryChanged(const QString& path);

    private:
        FileManager(QObject* parent = nullptr);
        void fileSuccessfullyLoaded(QString fileName);
        void updateRecentFiles(const QString& file) const;
        void displayErrorMessage(QString error_message);
        QString getShadowFile(QString file);
        void removeShadowFile();

        QString mFileName;
        QString mShadowFileName;
        QFileSystemWatcher* mFileWatcher;
        bool mFileOpen;
        QTimer* mTimer;
        bool mAutosaveEnabled;
        int mAutosaveInterval;
    };
}
