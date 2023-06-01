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
#include <QMap>
#include <QAbstractTableModel>
#include <QTableView>
#include <QStyledItemDelegate>
#include <QPushButton>
#include "netlist_simulator_controller/wave_data.h"

class QSortFilterProxyModel;

namespace hal {
    class WaveSelectionEntry
    {
        u32 mId;
        QString mName;
        u64 mSize;
    public:
        WaveSelectionEntry(u32 id_ = 0, const QString& nam = QString(), u64 n=0) : mId(id_), mName(nam), mSize(n) {;}
        u32 id() const { return mId; }
        QString name() const { return mName; }
        u64 size() const { return mSize; }
        bool operator==(const WaveSelectionEntry& other) const { return mName == other.mName; }
        bool operator<(const WaveSelectionEntry& other) const { return mName < other.mName; }
    };

    uint qHash(const WaveSelectionEntry& wse);

    class WaveSelectionTable : public QAbstractTableModel
    {
        Q_OBJECT
        QMap<WaveSelectionEntry,int> mWaveSelectionEntryMap;
    public:
        WaveSelectionTable(QObject* parent = nullptr);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        void setEntryMap(const QMap<WaveSelectionEntry,int>& entries);
        QMap<WaveSelectionEntry,int> entryMap(const QList<QModelIndex>& indexes) const;
    };


    class WaveSelectionDialog : public QDialog
    {
        Q_OBJECT
        WaveSelectionTable* mWaveModel;
        QSortFilterProxyModel* mProxyModel;
        QTableView* mTableView;
        QPushButton* mButAll;
        QPushButton* mButNone;
        QPushButton* mButSel;

    private Q_SLOTS:
        void handleCurrentGuiSelection();

    public:
        WaveSelectionDialog(const QMap<WaveSelectionEntry,int>& wseMap, QWidget* parent = nullptr);
        QMap<WaveSelectionEntry,int> selectedWaves() const;
    };

}
