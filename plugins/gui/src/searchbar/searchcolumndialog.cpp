#include "gui/searchbar/searchcolumndialog.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QDebug>

namespace hal {

    SearchColumnDialog::SearchColumnDialog(const QStringList& colNames, const QList<int> &selected)
        : mDisableHandler(false)
    {
        //TODO FIX check logic
        setWindowTitle("Search in:");
        QVBoxLayout* layout = new QVBoxLayout(this);
        mCheckAllColumns = new QCheckBox("All columns", this);
        connect(mCheckAllColumns,&QCheckBox::stateChanged,this,&SearchColumnDialog::handleCheckStateChanged);
        layout->addWidget(mCheckAllColumns);

        QFrame* frame = new QFrame(this);
        frame->setLineWidth(3);
        frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
        QVBoxLayout* flay = new QVBoxLayout(frame);
        int inx = 0;
        for (const QString& col : colNames)
        {
            QCheckBox* cbox = new QCheckBox(col, frame);
            cbox->setChecked(selected.isEmpty() || selected.contains(inx));
            connect(cbox,&QCheckBox::stateChanged,this,&SearchColumnDialog::handleCheckStateChanged);
            flay->addWidget(cbox);
            mCheckColumn.append(cbox);
            ++inx;
        }
        layout->addWidget(frame);

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(mButtonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
        connect(mButtonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
        layout->addWidget(mButtonBox);
        handleCheckStateChanged(0);
    }

    void SearchColumnDialog::handleCheckStateChanged(int state)
    {
        if (mDisableHandler) return;

        bool allChecked = true;
        bool nullChecked = true;

        if (sender()==mCheckAllColumns && state == Qt::Checked)
        {
            mDisableHandler = true;
            for (QCheckBox* cbox : mCheckColumn)
                cbox->setChecked(true);
            mDisableHandler = false;
        }
        else if (sender()==mCheckAllColumns && state == Qt::Unchecked)
        {
            mDisableHandler = true;
            //check if everything is checked
            for(QCheckBox* cbox: mCheckColumn){
                if(!cbox->isChecked())
                {
                    allChecked = false;
                    break;
                }
            }
            if(allChecked){
                for (QCheckBox* cbox : mCheckColumn)
                    cbox->setChecked(false);
            }
            mDisableHandler = false;
        }


        for (const QCheckBox* cbox : mCheckColumn)
        {
            if (cbox->isChecked())
                nullChecked = false;
            else
                allChecked = false;
        }
        // if all columns are checked then check top checkbox - otherwise uncheck it
        if (mCheckAllColumns->isChecked() != allChecked)
        {
            mCheckAllColumns->setCheckState(allChecked ? Qt::Checked : Qt::Unchecked);
            mCheckAllColumns->setChecked(allChecked);
        }
        mButtonBox->button(QDialogButtonBox::Ok)->setDisabled(nullChecked);
    }

    QString SearchColumnDialog::selectedColumnNames() const
    {
        if (mCheckAllColumns->isChecked()) return "All columns";

        QString retval;
        for (const QCheckBox* cbox : mCheckColumn)
        {
            if (!cbox->isChecked()) continue;
            if (!retval.isEmpty()) retval += ',';
            retval += cbox->text();
        }
        return retval;
    }

    QList<int> SearchColumnDialog::selectedColumns() const
    {
        QList<int> retval;
        if (mCheckAllColumns->isChecked()) return retval;

        int inx = 0;
        for (const QCheckBox* cbox : mCheckColumn)
        {
            if (cbox->isChecked()) retval.append(inx);
            ++inx;
        }
        return retval;
    }

    void SearchColumnDialog::accept()
    {
        handleCheckStateChanged(0);
        QDialog::accept();
    }
}
