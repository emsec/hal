#include "waveform_viewer/boolean_dialog.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <QGridLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QFrame>
#include <QVBoxLayout>

namespace hal {
    BooleanDialog::BooleanDialog(const QList<WaveData *> inputList, QWidget* parent)
        : QDialog(parent), mHandleTableEdit(true)
    {
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);

        setWindowTitle("Create Boolean Waveform");
        setMinimumWidth(640);

        QGridLayout* layout = new QGridLayout(this);
        QFrame* fExpression = new QFrame(this);
        fExpression->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        fExpression->setLineWidth(2);
        QVBoxLayout* layExpression = new QVBoxLayout(fExpression);
        mEnterExpression = new QRadioButton("Enter boolean expression:");
        connect(mEnterExpression,&QRadioButton::toggled,this,&BooleanDialog::handleExpressionToggled);
        layExpression->addWidget(mEnterExpression);
        mLineEdit = new QLineEdit(fExpression);
        if (inputList.isEmpty())
            dbb->button(QDialogButtonBox::Ok)->setDisabled(true);
        else
        {
            QString def(inputList.at(0)->name());
            for (int i=1; i<inputList.size(); i++)
                def += ("&" + inputList.at(i)->name());
            mLineEdit->setText(def);
            dbb->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
        layExpression->addWidget(mLineEdit);
        layout->addWidget(fExpression,0,0,1,2);

        QFrame* fTable = new QFrame(this);
        fTable->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        fTable->setLineWidth(2);
        QVBoxLayout* layTable = new QVBoxLayout(fTable);
        mEnterTable = new QRadioButton("Enter table with accepted combinations:");
        connect(mEnterTable,&QRadioButton::toggled,this,&BooleanDialog::handleTableToggled);
        layTable->addWidget(mEnterTable);
        mTableWidget = new QTableWidget(this);
        mTableWidget->setDisabled(true);
        if (inputList.isEmpty())
            mEnterTable->setDisabled(true);
        else
        {
            mTableWidget->setRowCount(inputList.size());
            mTableWidget->setColumnCount(2);
            for (int irow=0; irow<inputList.size(); irow++)
            {
                WaveData* wd = inputList.at(irow);
                mTableWidget->setItem(irow,0,new QTableWidgetItem(QString::number(wd->id())));
                mTableWidget->setItem(irow,1,new QTableWidgetItem(wd->name()));
                for (int icol=0; icol<2; icol++)
                    mTableWidget->item(irow,icol)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                mTableWidget->setColumnWidth(0,60);
                mTableWidget->setColumnWidth(1,360);
            }
            addEmptyTableColumn(2);
            mTableWidget->setHorizontalHeaderLabels({"ID", "Name", "1."});
        }
        connect(mTableWidget,&QTableWidget::cellChanged,this,&BooleanDialog::handleTableCellChanged);
        layTable->addWidget(mTableWidget);
        layout->addWidget(fTable,1,0,1,2);
        layout->addWidget(dbb,2,1);
        mEnterExpression->setChecked(true);
    }

    void BooleanDialog::addEmptyTableColumn(int icol)
    {
        if (mTableWidget->columnCount()<=icol)
        {
            mTableWidget->setColumnCount(icol+1);
            mTableWidget->setColumnWidth(icol,36);
            mTableWidget->setHorizontalHeaderItem(icol,new QTableWidgetItem(QString("%1.").arg(icol-1)));
        }
        for (int irow=0; irow < mTableWidget->rowCount(); irow++)
        {
            QTableWidgetItem* twi = new QTableWidgetItem();
            twi->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
            mTableWidget->setItem(irow,icol,twi);
        }
    }

    void BooleanDialog::handleTableCellChanged(int irow, int icol)
    {
        if (!mHandleTableEdit) return;
        mHandleTableEdit = false;
        bool rejected = true;
        int n = mTableWidget->columnCount();
        QString txt = mTableWidget->item(irow,icol)->text();
        if (txt == "0" || txt == "1")
            rejected = false;
        if (txt == "X" || txt == "x")
        {
            rejected = false;
            mTableWidget->item(irow,icol)->setText("0");
            int j = n-1;
            if (icol+1 >= n)
            {
                addEmptyTableColumn(n++);
                j = n-1;
            }
            for (int i=0; i<mTableWidget->rowCount(); i++)
                mTableWidget->item(i,j)->setText(i==irow?QString("1"):mTableWidget->item(i,icol)->text());
            if (j+1 >= n)
                addEmptyTableColumn(n);
        }
        if (rejected)
            mTableWidget->item(irow,icol)->setText("");
        else if (icol+1 >= n)
            addEmptyTableColumn(icol+1);
        mHandleTableEdit = true;
    }


    void BooleanDialog::activateExpression(bool enable)
    {
        if (enable)
            mEnterTable->setChecked(false);
        else
            mEnterExpression->setChecked(false);
        mTableWidget->setDisabled(enable);
        mLineEdit->setEnabled(enable);
    }

    void BooleanDialog::handleExpressionToggled(bool state)
    {
        activateExpression(state);
    }

    void BooleanDialog::handleTableToggled(bool state)
    {
        activateExpression(!state);
    }

    bool BooleanDialog::hasExpression() const
    {
        return mEnterExpression->isChecked();
    }

    QString BooleanDialog::expression() const
    {
        return mLineEdit->text().trimmed();
    }

    QList<int> BooleanDialog::tableValues() const
    {
        QSet<int> val;
        for (int icol = 2; icol < mTableWidget->columnCount(); icol++)
        {
            bool incomplete = false;
            int v = 0;
            for (int irow = 0; irow < mTableWidget->rowCount(); irow++)
            {
                QString txt = mTableWidget->item(irow,icol)->text();
                if (txt!="0")
                {
                    if (txt == "1")
                        v |= (1<<irow);
                    else
                    {
                        incomplete = true;
                        break;
                    }
                }
            }
            if (!incomplete)
                val.insert(v);
        }
        return val.toList();
    }
}
