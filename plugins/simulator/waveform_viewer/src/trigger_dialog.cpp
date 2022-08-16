#include "waveform_viewer/trigger_dialog.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <QGridLayout>
#include <QTableWidget>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

namespace hal {
    TriggerDialog::TriggerDialog(const QList<WaveData *> inputList, const QList<WaveData *> filterList, QWidget* parent)
        : QDialog(parent), mFilterList(filterList), mHandleTableEdit(true)
    {
        if (inputList.isEmpty()) return;

        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);

        setWindowTitle("Create Trigger Time Set");
        setMinimumWidth(640);

        QGridLayout* layout = new QGridLayout(this);

        QFrame* fTable = new QFrame(this);
        fTable->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        fTable->setLineWidth(2);
        QVBoxLayout* layTable = new QVBoxLayout(fTable);
        layTable->addWidget(new QLabel("Trigger only upon transiton to value VT (X = all values)", fTable));
        mTableWidget = new QTableWidget(fTable);
        mTableWidget->setRowCount(inputList.size());
        mTableWidget->setColumnCount(3);
        for (int irow=0; irow<inputList.size(); irow++)
        {
            WaveData* wd = inputList.at(irow);
            mTableWidget->setItem(irow,0,new QTableWidgetItem(QString::number(wd->id())));
            mTableWidget->setItem(irow,1,new QTableWidgetItem(wd->name()));
            mTableWidget->setItem(irow,2,new QTableWidgetItem("X"));
            for (int icol=0; icol<2; icol++)
                mTableWidget->item(irow,icol)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            mTableWidget->item(irow,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
            mTableWidget->setColumnWidth(0,60);
            mTableWidget->setColumnWidth(1,360);
            mTableWidget->setColumnWidth(2,36);
        }
        mTableWidget->setHorizontalHeaderLabels({"ID", "Name", "VT"});
        connect(mTableWidget,&QTableWidget::cellChanged,this,&TriggerDialog::handleTableCellChanged);
        layTable->addWidget(mTableWidget);
        layout->addWidget(fTable,0,0,1,2);

        QFrame* fExpression = new QFrame(this);
        fExpression->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        fExpression->setLineWidth(2);
        QVBoxLayout* layExpression = new QVBoxLayout(fExpression);
        mEnterFilter = new QRadioButton("Select waveform as filter:");
        connect(mEnterFilter,&QRadioButton::toggled,this,&TriggerDialog::handleFilterToggled);
        layExpression->addWidget(mEnterFilter);
        mSelectFilter = new QComboBox(fExpression);
        for (WaveData* wd : filterList)
        {
            QString txt = QString("%1[%2%3]")
                    .arg(wd->name())
                    .arg(wd->netType()==WaveData::BooleanNet?"bool ":"")
                    .arg(wd->id());
            mSelectFilter->addItem(txt,QVariant::fromValue<void*>(wd));
        }
        layExpression->addWidget(mSelectFilter);
        layout->addWidget(fExpression,1,0,1,2);

        layout->addWidget(dbb,2,1);
        mEnterFilter->setChecked(false);
    }

    void TriggerDialog::handleTableCellChanged(int irow, int icol)
    {
        if (!mHandleTableEdit) return;
        mHandleTableEdit = false;
        QString txt = mTableWidget->item(irow,icol)->text().trimmed();
        bool ok;
        int val = txt.toInt(&ok);
        if ((val < 0 || !ok) && txt != "X")
            mTableWidget->item(irow,icol)->setText("X");
        mHandleTableEdit = true;
    }

    QList<int> TriggerDialog::transitionToValue() const
    {
        QList<int> retval;
        for (int irow = 0; irow < mTableWidget->rowCount(); irow++)
        {
            QString txt = mTableWidget->item(irow,2)->text().trimmed();
            if (txt=="X")
                retval.append(-1);
            else
                retval.append(txt.toInt());
        }
        return retval;
    }

    void TriggerDialog::handleFilterToggled(bool state)
    {
        mSelectFilter->setEnabled(state);
    }

    WaveData* TriggerDialog::filterWave() const
    {
        if (!mEnterFilter->isChecked()) return nullptr;
        QVariant sel = mSelectFilter->currentData();
        if (!sel.isValid()) return nullptr;
        return static_cast<WaveData*>(sel.value<void*>());
    }
}
