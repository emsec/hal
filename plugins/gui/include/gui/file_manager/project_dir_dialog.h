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

#include <QFileDialog>
#include <QFrame>
#include <QSortFilterProxyModel>
#include <QAbstractButton>
#include <QPixmap>
#include "file_manager.h"

class QLabel;

namespace hal {

/*
    class ProjectDirProxy : public QSortFilterProxyModel
    {
        Q_OBJECT
        bool hasProjectDirectory(const QString& dir) const;
    public:
        ProjectDirProxy(QObject* parent = nullptr);
        bool filterAcceptsRow(int irow, const QModelIndex &parentInx) const override;
    };
*/
    class ProjectDirDialogStatus : public QFrame
    {
        Q_OBJECT;

        QLabel* mIcon;
        QLabel* mText;
        static QPixmap* sCheckMark;
        static QPixmap* sAttention;
        static QPixmap getPixmap(bool ok);
    public:
        ProjectDirDialogStatus(QWidget* parent=nullptr);
        void setMessage(const QString& path, FileManager::DirectoryStatus stat);
    };

    class ProjectDirDialog : public QFileDialog
    {
        Q_OBJECT

        QAbstractButton* mChooseButton;
        ProjectDirDialogStatus* mStatus;
        FileManager::DirectoryStatus mSelectedDirectoryStatus;

        bool isSelectable() const;

    private Q_SLOTS:
        void handleCurrentChanged(const QString& path);

    public:
        void accept() override;

        ProjectDirDialog(const QString& title, const QString& defaultDir, QWidget* parent = nullptr);

        bool eventFilter(QObject* obj, QEvent* event) override;
    };
}
