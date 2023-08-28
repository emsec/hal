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

#include <QFrame>
#include <QAbstractTableModel>
#include <QModelIndex>

class QComboBox;
class QPushButton;
class QCheckBox;

namespace hal {
    class GateLibrarySelectionEntry
    {
        QString mName;
        QString mPath;
        int mCount;
    public:
        GateLibrarySelectionEntry(const QString& name_, const QString& path_, int cnt)
            : mName(name_), mPath(path_), mCount(cnt) {;}
        QVariant data(int column, bool fullPath) const;
        QString name() const {return mName; }
        QString path() const {return mPath; }
    };

    class GateLibrarySelectionTable : public QAbstractTableModel
    {
        Q_OBJECT
        QList<GateLibrarySelectionEntry> mEntries;
        bool mShowFullPath;
    public:
        GateLibrarySelectionTable(bool addAutoDetect, QObject* parent = nullptr);
        int columnCount(const QModelIndex& index = QModelIndex()) const override;
        int rowCount(const QModelIndex& index = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        void handleShowFullPath(bool checked);
        int addGateLibrary(const QString& path);
        QString gateLibraryPath(int inx) const;
        int getIndexByPath(const QString& path);
    };

    class GateLibrarySelection : public QFrame
    {
        Q_OBJECT
        QComboBox* mComboGatelib;
        QPushButton* mInvokeFileDialog;
        QCheckBox* mCheckFullPath;
    Q_SIGNALS:
        void gatelibSelected(bool singleFile);
    private Q_SLOTS:
        void handleInvokeFileDialog();
        void handleShowFullPath(bool checked);
    public:
        GateLibrarySelection(const QString& defaultGl, QWidget* parent = nullptr);
        QString gateLibraryPath() const;
        void setIcon(const QString& path, const QString& style);
        void setCurrent(const QString& glPath);
    };
}
