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
#include "gui/gui_globals.h"

namespace hal {

    WaveSelectionDialog::WaveSelectionDialog(const QMap<WaveSelectionEntry, int> &wseMap, QWidget *parent)
        : QDialog(parent), mWaveSelectionMap(wseMap)
    {
        QGridLayout* layout = new QGridLayout(this);
        mButAll = new QPushButton("All waveform items", this);
        connect(mButAll,&QPushButton::clicked,this,&WaveSelectionDialog::handleSelectAll);
        layout->addWidget(mButAll,0,0);
        mButSel = new QPushButton("Current GUI selection", this);
        connect(mButSel,&QPushButton::clicked,this,&WaveSelectionDialog::handleCurrentGuiSelection);
        layout->addWidget(mButSel,0,1);
        mButNone = new QPushButton("Clear selection", this);
        connect(mButNone,&QPushButton::clicked,this,&WaveSelectionDialog::handleClearSelection);
        layout->addWidget(mButNone,0,2);

        mTableView = new QTableView(this);
        mProxyModel = new QSortFilterProxyModel(this);
        mWaveModel = new WaveSelectionTable(mWaveSelectionMap.keys(),mTableView);
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
        layout->addWidget(mTableView,1,0,1,2);
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(dbb,2,1);
        setWindowTitle("Add Waveform Net");
    }

    void WaveSelectionDialog::handleSelectAll()
    {
        mTableView->selectAll();
    }

    void WaveSelectionDialog::handleCurrentGuiSelection()
    {
        QSet<u32> guiNetSel = gSelectionRelay->selectedNets();

        const QAbstractItemModel* modl = mTableView->model(); // proxy model
        int nrows = modl->rowCount();
        mTableView->clearSelection();

        bool ok;

        for (int irow = 0; irow<nrows; irow++)
        {
            u32 gid = modl->data(modl->index(irow,0)).toUInt(&ok);
            if (!ok) continue;
            if (guiNetSel.contains(gid))
                mTableView->selectRow(irow);
        }

    }

    void WaveSelectionDialog::handleClearSelection()
    {
        mTableView->clearSelection();
    }

    QMap<WaveSelectionEntry,int> WaveSelectionDialog::selectedWaves() const
    {
        QMap<WaveSelectionEntry,int> retval;
        for (QModelIndex proxyInx : mTableView->selectionModel()->selectedIndexes())
        {
            QModelIndex modelInx = mProxyModel->mapToSource(proxyInx);
            const WaveSelectionEntry& wse = mWaveModel->waveSelectionEntry(modelInx.row());
            auto it = mWaveSelectionMap.find(wse);
            if (it != mWaveSelectionMap.constEnd())
                retval.insert(it.key(),it.value());
        }
        return retval;
    }

    //-------------------------------------------

    WaveSelectionTable::WaveSelectionTable(const QList<WaveSelectionEntry> &wseList, QObject* parent)
        : QAbstractTableModel(parent), mWaveSelectionEntry(wseList)
    {;}

    int WaveSelectionTable::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mWaveSelectionEntry.size();
    }

    int WaveSelectionTable::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    QVariant WaveSelectionTable::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if (index.row() >= mWaveSelectionEntry.size()) return QVariant();
        const WaveSelectionEntry& wse =  mWaveSelectionEntry.at(index.row());
        switch (index.column())
        {
        case 0: return wse.id();
        case 1: return wse.name();
        case 2: return QVariant((qulonglong)wse.size());
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

    uint qHash(const WaveSelectionEntry& wse) { return qHash(wse.name()); }

}
