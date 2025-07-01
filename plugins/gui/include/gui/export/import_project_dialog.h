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

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>

namespace hal
{
    class FileSelectWidget : public QFrame
    {
        Q_OBJECT

        QPushButton* mButton;
        QLineEdit* mEditor;
        bool mExistingDir;
        bool mValid;
    private Q_SLOTS:
        void handleActivateFileDialog();
        void handleTextChanged(const QString& txt);
    Q_SIGNALS:
        void selectionStatusChanged();
    public:
       FileSelectWidget(const QString& defaultEntry, bool existingDir, QWidget* parent = nullptr);
       bool valid() const {return mValid; }
       void setFixedEntry(const QString& entry);
       QString selection() const { return mEditor->text(); }
    };

    class ImportProjectDialog : public QDialog
    {
        Q_OBJECT

    public:
        enum ImportStatus { Ok = 0, NoFileSelected = -1, ErrorDecompress = -2, NotAHalProject = -3, NoImport = -99};
    private:
        ImportStatus mStatus;
        FileSelectWidget* mZippedFile;
        FileSelectWidget* mTargetDirectory;
        QLineEdit* mExtractProjectEdit;
        QDialogButtonBox* mButtonBox;
        QString mTargetProjectName;
        QString mExtractedProjectAbsolutePath;
        static void deleteFilesList(QStringList files);
        static void deleteFilesRecursion(QString dir);
    private Q_SLOTS:
        void handleSelectionStatusChanged();
    public:
        ImportProjectDialog(QWidget* parent = nullptr);
        bool importProject();
        void setZippedFile(const QString& filename);
        ImportStatus status() const { return mStatus; }
        QString extractedProjectAbsolutePath() const { return mExtractedProjectAbsolutePath; }
        static QString suggestedProjectDir(const QString& filename);
    };
}
