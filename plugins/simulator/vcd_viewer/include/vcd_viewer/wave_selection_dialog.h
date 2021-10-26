#pragma once

#include <QDialog>
#include <QMap>
#include <QAbstractTableModel>
#include <QTableView>
#include <QStyledItemDelegate>
#include "netlist_simulator_controller/wave_data.h"

class QSortFilterProxyModel;

namespace hal {

    class WaveSelectionTable : public QAbstractTableModel
    {
        Q_OBJECT
        QList<const WaveData*> mWaveDataList;
    public:
        WaveSelectionTable(const QList<const WaveData*>& wdList, QObject* parent = nullptr);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        const WaveData* waveData(int irow) const { return mWaveDataList.at(irow); }
    };


    class WaveSelectionDialog : public QDialog
    {
        Q_OBJECT
        WaveSelectionTable* mWaveModel;
        QSortFilterProxyModel* mProxyModel;
        QTableView* mTableView;
        QMap<const WaveData*,int> mWaveDataMap;
    public:
        WaveSelectionDialog(const QMap<const WaveData*,int>& wdMap, QWidget* parent = nullptr);
        QList<int> selectedWaveIndices() const;
    };

}
