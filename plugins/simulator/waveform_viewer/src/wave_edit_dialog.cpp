#include "waveform_viewer/wave_edit_dialog.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include "netlist_simulator_controller/saleae_file.h"
#include <QTableView>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QHeaderView>

namespace hal {

    WaveEditDialog::WaveEditDialog(WaveData *wd, double tCursor, QWidget *parent)
        : QDialog(parent)
    {
        QGridLayout* layout = new QGridLayout(this);
        QTableView* tv = new QTableView(this);
        mWaveModel = new WaveEditTable(wd,tCursor,tv);
        if (wd->bits() <= 1)
            tv->setItemDelegateForColumn(2, new WaveDeleteDelegate(mWaveModel));
        tv->setModel(mWaveModel);
        QHeaderView* hv = tv->horizontalHeader();
        hv->setSectionResizeMode(0,QHeaderView::Stretch);
        hv->setSectionResizeMode(1,QHeaderView::Interactive);
        hv->setSectionResizeMode(2,QHeaderView::Fixed);
        tv->setColumnWidth(1,24);
        tv->setColumnWidth(2,16);
        layout->addWidget(tv,0,0,1,2);
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        if (wd->bits() > 1)
            dbb->button(QDialogButtonBox::Ok)->setDisabled(true);
        else
            connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);
        layout->addWidget(dbb,1,1);
        setWindowTitle(wd->name());
    }

    const QMap<u64,int>& WaveEditDialog::dataMap() const
    {
        return mWaveModel->mEditData;
    }

    //-------------------------------------------

    QPixmap* WaveDeleteDelegate::sXdelete = nullptr;

    QWidget* WaveDeleteDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(parent);
        mTable->removeRow(index.row());
        return nullptr;
    }

    void WaveDeleteDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
    {
        if (!index.row() || index.row() >= mTable->rowCount()-1) return;
        QPixmap* pix = piXdelete();
        QRect rPlace(option.rect.x(),option.rect.y(),option.rect.height()/2,option.rect.height()/2);
        painter->drawPixmap(rPlace ,QPixmap(*pix));
    }

    QSize WaveDeleteDelegate::sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return piXdelete()->size();
    }

    void WaveDeleteDelegate::deleteClicked()
    {
        QPushButton* but = static_cast<QPushButton*>(sender());
        if (!but) return;
    }

    QPixmap* WaveDeleteDelegate::piXdelete()
    {
        if (!sXdelete) sXdelete = new QPixmap(":/icons/x_delete", "PNG");
        return sXdelete;
    }

    //-------------------------------------------
    WaveEditTable::WaveEditTable(WaveData *wd, double tCursor, QObject* parent)
        : QAbstractTableModel(parent), mWaveDataInput(wd)
    {
        uint64_t wsize = mWaveDataInput->fileSize();
        if (!wsize)
        {
            if (mWaveDataInput->data().isEmpty())
                mEditData.insert(0,0);
            else
            {
                mEditData = mWaveDataInput->data();
                truncateData(tCursor);
            }
        }
        else if (wsize <= (uint64_t) NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable())
        {
            if (mWaveDataInput->data().size() < (int) wsize)
                mWaveDataInput->loadSaleae();
            mEditData = mWaveDataInput->data();
            truncateData(tCursor);
        }
        else
        {
            int n = NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeEditor();
            SaleaeInputFile sif(mWaveDataInput->fileName());
            sif.get_file_position(tCursor);
            sif.skip_transitions(-n/2);
            SaleaeDataBuffer* sdb = sif.get_buffered_data(n);
            for (u64 i=0; i<sdb->mCount; i++)
            {
               mEditData.insert(sdb->mTimeArray[i],sdb->mValueArray[i]);
            }
            delete sdb;
        }
    }

    void WaveEditTable::truncateData(double tCursor)
    {
        int n =  NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeEditor();
        if (mEditData.size() <= n) return; // nothing to do
        auto it = mEditData.upperBound(tCursor);
        for (int i=0; i<n/2; i++)
            if (it != mEditData.begin())
                --it;
        for (auto itDel = mEditData.begin(); itDel != it; itDel = mEditData.erase(itDel)) {;}
        for (int i=0; i<n; i++)
            if (it != mEditData.end())
                ++it;
        for (auto itDel = it; itDel != mEditData.end(); itDel = mEditData.erase(itDel)) {;}
    }

    int WaveEditTable::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mEditData.size() + 1;
    }

    int WaveEditTable::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    QVariant WaveEditTable::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if (index.row() >= mEditData.size()) return QString();
        auto it = mEditData.constBegin() + index.row();
        if (it != mEditData.constEnd())
        {
            switch (index.column())
            {
            case 0: return QVariant::fromValue(it.key());
            case 1: return mWaveDataInput->strValue(it);
            case 2: return QString();
            default: return QVariant();
            }
        }
        return QString();
    }

    void WaveEditTable::recalcTransitions()
    {
        int val = 0; // determine start value on begin
        for (auto it = mEditData.begin(); it != mEditData.end(); ++it)
        {
            if (it == mEditData.begin())
                val = it.value();
            else
            {
                val = val ? 0 : 1;
                *it = val;
            }
        }
    }

    bool WaveEditTable::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (role != Qt::EditRole) return false;
        bool numberOk;
        int iValue = value.toInt(&numberOk);
        if (!numberOk || iValue < 0) return false;

        beginResetModel();
        if (!index.row() && index.column()==1)
        {
            if (iValue <= 1)
            {
                if (mEditData.isEmpty())
                    mEditData.insert(0,iValue);
                else
                    *(mEditData.begin()) = iValue;
            }
        }
        else
        {
            if (index.row() < mEditData.size())
            {
                auto it = mEditData.begin() + index.row();
                mEditData.erase(it);
            }
            mEditData[iValue] = 0;
        }
        recalcTransitions();
        endResetModel();
        return true;
    }

    QVariant WaveEditTable::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
            return QAbstractTableModel::headerData(section, orientation, role);
        switch (section)
        {
        case 0: return "Time [µs]";
        case 1: return "New Value";
        }
        return QVariant();
    }

    Qt::ItemFlags WaveEditTable::flags(const QModelIndex &index) const
    {
        if (mWaveDataInput->bits() > 1) return Qt::NoItemFlags;
        if (index.column()==1)
        {
            if (index.row()) return Qt::NoItemFlags;
            return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
        }
        if (index.column()==2 && (!index.row() || index.row() >= rowCount() - 1)) return Qt::NoItemFlags;
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    void WaveEditTable::removeRow(int irow)
    {
        int key = data(index(irow,0)).toInt();
        if (!key) return;

        beginResetModel();
        auto it = mEditData.begin() + irow;
        mEditData.erase(it);
        recalcTransitions();
        endResetModel();
    }
}
