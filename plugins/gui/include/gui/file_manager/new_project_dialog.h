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

#include <QDialog>
#include <QString>
#include <QList>
#include <QMap>
#include <QPushButton>

class QComboBox;
class QCheckBox;
class QLineEdit;

namespace hal {

    class NewProjectDialog : public QDialog
    {
        Q_OBJECT
        Q_PROPERTY(QString saveIconPath READ saveIconPath WRITE setSaveIconPath)
        Q_PROPERTY(QString saveIconStyle READ saveIconStyle WRITE setSaveIconStyle)
        QString mProjectdir;
        QLineEdit* mEditProjectdir;
        QComboBox* mComboGatelib;
        QCheckBox* mCheckCopyGatelib;

        QStringList mGateLibraryPath;
        QMap<QString,int> mGateLibraryMap;
        QString mSaveIconPath;
        QString mSaveIconStyle;
        QPushButton* mOkButton;
    private Q_SLOTS:
        void handleGateLibraryPathChanged(const QString& txt);
        void handleFileDialogTriggered();
    public:
        NewProjectDialog(QWidget* parent = nullptr);
        QString projectDirectory() const;
        QString gateLibraryPath() const;
        bool isCopyGatelibChecked() const;
        QString saveIconPath() const { return mSaveIconPath; }
        QString saveIconStyle() const { return mSaveIconStyle; }
        void setSaveIconPath(const QString& path) { mSaveIconPath = path; }
        void setSaveIconStyle(const QString& sty) { mSaveIconStyle = sty; }
    };
}
