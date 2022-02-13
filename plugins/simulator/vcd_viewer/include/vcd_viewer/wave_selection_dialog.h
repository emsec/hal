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
        QList<WaveSelectionEntry> mWaveSelectionEntry;
    public:
        WaveSelectionTable(const QList<WaveSelectionEntry>& wseList, QObject* parent = nullptr);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        const WaveSelectionEntry& waveSelectionEntry(int irow) const { return mWaveSelectionEntry.at(irow); }
    };


    class WaveSelectionDialog : public QDialog
    {
        Q_OBJECT
        WaveSelectionTable* mWaveModel;
        QSortFilterProxyModel* mProxyModel;
        QTableView* mTableView;
        QMap<WaveSelectionEntry,int> mWaveSelectionMap;
        QPushButton* mButAll;
        QPushButton* mButNone;
        QPushButton* mButSel;

    private Q_SLOTS:
        void handleSelectAll();
        void handleCurrentGuiSelection();
        void handleClearSelection();

    public:
        WaveSelectionDialog(const QMap<WaveSelectionEntry,int>& wseMap, QWidget* parent = nullptr);
        QMap<WaveSelectionEntry,int> selectedWaves() const;
    };

}
