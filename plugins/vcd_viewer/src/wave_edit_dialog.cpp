#include "vcd_viewer/wave_edit_dialog.h"
#include "vcd_viewer/wave_label.h"
#include <QTableView>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QHeaderView>

namespace hal {

    WaveEditDialog::WaveEditDialog(const WaveData& wd, QWidget *parent)
        : QDialog(parent)
    {
        QGridLayout* layout = new QGridLayout(this);
        QTableView* tv = new QTableView(this);
        mWaveModel = new WaveEditTable(wd,tv);
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
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(dbb,1,1);
        setWindowTitle(wd.name());
    }

    WaveData* WaveEditDialog::dataFactory() const
    {
        return mWaveModel->dataFactory();
    }

    //-------------------------------------------

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
        QPixmap* pix = WaveLabel::piXdelete();
        QRect rPlace(option.rect.x(),option.rect.y(),option.rect.height()/2,option.rect.height()/2);
        painter->drawPixmap(rPlace ,QPixmap(*pix));
    }

    QSize WaveDeleteDelegate::sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return WaveLabel::piXdelete()->size();
    }

    void WaveDeleteDelegate::deleteClicked()
    {
        QPushButton* but = static_cast<QPushButton*>(sender());
        if (!but) return;
    }

    //-------------------------------------------
    WaveData* WaveEditTable::dataFactory() const
    {
        return new WaveData(mWaveData);
    }

    WaveEditTable::WaveEditTable(const WaveData& wd, QObject* parent)
        : QAbstractTableModel(parent), mWaveData(wd)
    {
        if (mWaveData.isEmpty()) mWaveData[0] = 0;
    }

    int WaveEditTable::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mWaveData.size() + 1;
    }

    int WaveEditTable::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    QVariant WaveEditTable::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if (index.row() >= mWaveData.size()) return QString();
        auto it = mWaveData.constBegin() + index.row();
        if (it != mWaveData.constEnd())
        {
            switch (index.column())
            {
            case 0: return it.key();
            case 1: return mWaveData.textValue(it);
            case 2: return QString();
            default: return QVariant();
            }
        }
        return QString();
    }

    bool WaveEditTable::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (role != Qt::EditRole) return false;
        bool numberOk;
        int iValue = value.toInt(&numberOk);
        if (!numberOk) return false;

        if (!index.row() && index.column()==1)
        {
            if (iValue < 0 || iValue > 1 || iValue == mWaveData.value(0)) return false;
            mWaveData.setStartvalue(iValue);
            Q_EMIT(dataChanged(this->index(0,1),this->index(mWaveData.size(),1)));
            return true;
        }
        if (index.column() || mWaveData.contains(iValue)) return false;

        beginResetModel();
        mWaveData.insertToggleTime(iValue);
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
        if (index.column()==2)
           return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
        if (index.row() && index.column())
            return Qt::NoItemFlags;
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    void WaveEditTable::removeRow(int irow)
    {
        int key = data(index(irow,0)).toInt();
        if (!key) return;

        auto it = mWaveData.find(key);
        if (it==mWaveData.end()) return;

        beginResetModel();
        int val = it.value();
        it = mWaveData.erase(it);
        while (it != mWaveData.end())
        {
            int nextVal = it.value();
            *(it++) = val;
            val = nextVal;
        }
        endResetModel();
    }
}
