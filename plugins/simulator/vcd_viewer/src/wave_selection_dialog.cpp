#include "vcd_viewer/wave_selection_dialog.h"
#include <QTableView>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QHeaderView>
#include <QSortFilterProxyModel>

namespace hal {

    WaveSelectionDialog::WaveSelectionDialog(const QMap<const WaveData *, int> &wdMap, QWidget *parent)
        : QDialog(parent), mWaveDataMap(wdMap)
    {
        QGridLayout* layout = new QGridLayout(this);
        mTableView = new QTableView(this);
        mProxyModel = new QSortFilterProxyModel(this);
        mWaveModel = new WaveSelectionTable(mWaveDataMap.keys(),mTableView);
        mProxyModel->setSourceModel(mWaveModel);
        mTableView->setModel(mProxyModel);
        mTableView->setSortingEnabled(true);
        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::MultiSelection);
        QHeaderView* hv = mTableView->horizontalHeader();
        hv->setSectionResizeMode(0,QHeaderView::Interactive);
        hv->setSectionResizeMode(1,QHeaderView::Stretch);
        hv->setSectionResizeMode(2,QHeaderView::Interactive);
        mTableView->setColumnWidth(0,36);
        mTableView->setColumnWidth(1,256);
        mTableView->setColumnWidth(2,36);
        layout->addWidget(mTableView,0,0,1,2);
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(dbb,1,1);
        setWindowTitle("Add Waveform Net");
    }

    QList<int> WaveSelectionDialog::selectedWaveIndices() const
    {
        QSet<int> retval;
        for (QModelIndex proxyInx : mTableView->selectionModel()->selectedIndexes())
        {
            QModelIndex modelInx = mProxyModel->mapToSource(proxyInx);
            const WaveData* wd = mWaveModel->waveData(modelInx.row());
            auto it = mWaveDataMap.find(wd);
            if (it != mWaveDataMap.constEnd())
                retval.insert(it.value());
        }
        return retval.toList();
    }

    //-------------------------------------------

    WaveSelectionTable::WaveSelectionTable(const QList<const WaveData *> &wdList, QObject* parent)
        : QAbstractTableModel(parent), mWaveDataList(wdList)
    {;}

    int WaveSelectionTable::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mWaveDataList.size();
    }

    int WaveSelectionTable::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    QVariant WaveSelectionTable::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if (index.row() >= mWaveDataList.size()) return QVariant();
        const WaveData* wd =  mWaveDataList.at(index.row());
        switch (index.column())
        {
        case 0: return wd->id();
        case 1: return wd->name();
        case 2: return wd->data().size();
        }
        return QVariant();
    }

    QVariant WaveSelectionTable::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
            return QAbstractTableModel::headerData(section, orientation, role);
        switch (section)
        {
        case 0: return "ID";
        case 1: return "Name";
        case 2: return "Events";
        }
        return QVariant();
    }

    Qt::ItemFlags WaveSelectionTable::flags(const QModelIndex &index) const
    {
        Q_UNUSED(index);
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
}
