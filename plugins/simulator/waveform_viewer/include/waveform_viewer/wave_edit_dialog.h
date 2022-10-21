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
#include <QAbstractTableModel>
#include <QTableView>
#include <QStyledItemDelegate>
#include <QPixmap>
#include "netlist_simulator_controller/wave_data.h"

namespace hal {

    class WaveEditTable : public QAbstractTableModel
    {
        friend class WaveEditDialog;
        Q_OBJECT
        WaveData* mWaveDataInput;
        QMap<u64,int> mEditData;

        void truncateData(double tCursor);
        void recalcTransitions();
    public:
        WaveEditTable(WaveData* wd, double tCursor, QObject* parent = nullptr);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        void removeRow(int irow);
    };

    class WaveDeleteDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
        WaveEditTable* mTable;
        static QPixmap* sXdelete;
        static QPixmap* piXdelete();
    public:
        WaveDeleteDelegate(WaveEditTable*tab) : mTable(tab) {;}

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

        QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
    private Q_SLOTS:
            void deleteClicked();
    };

    class WaveEditDialog : public QDialog
    {
        Q_OBJECT
        WaveEditTable* mWaveModel;        
    public:
        WaveEditDialog(WaveData *wd, double tCursor, QWidget* parent = nullptr);
        const QMap<u64,int>& dataMap() const;
    };

}
