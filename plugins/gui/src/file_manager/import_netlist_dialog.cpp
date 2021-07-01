#include "gui/file_manager/import_netlist_dialog.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/gate_library/gate_library.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSpacerItem>
#include <QRegularExpression>

QLineEdit* mEditProjectdir;
QComboBox* mComboGatelib;
QCheckBox* mCheckMoveNetlist;

namespace hal {
    ImportNetlistDialog::ImportNetlistDialog(const QString& filename, QWidget *parent)
        : QDialog(parent)
    {
        int irow = 0;
        QGridLayout* layout = new QGridLayout(this);
        setWindowTitle("Import netlist");
        layout->addWidget(new QLabel("Create new hal project from netlist\n" + filename, this), irow++, 0, Qt::AlignLeft);
        layout->addItem(new QSpacerItem(30,30),irow++,0);

        layout->addWidget(new QLabel("Project directory:",this),irow++,0,Qt::AlignLeft);
        mProjectdir = filename;
        mProjectdir.remove(QRegularExpression("\\.\\w*$"));
        mEditProjectdir = new QLineEdit(mProjectdir,this);
        mEditProjectdir->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        layout->addWidget(mEditProjectdir,irow++,0);
        layout->addItem(new QSpacerItem(30,30),irow++,0);

        layout->addWidget(new QLabel("Gate library:",this),irow++,0,Qt::AlignLeft);
        mComboGatelib = new QComboBox(this);
        mComboGatelib->addItem("(Auto detect)");
        for (const GateLibrary* glib : gate_library_manager::get_gate_libraries())
        {
            mGateLibraryMap.insert(QString::fromStdString(glib->get_name()),mGateLibraries.size());
            mGateLibraries.append(glib);
        }
        mComboGatelib->addItems(mGateLibraryMap.keys());
        mComboGatelib->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        layout->addWidget(mComboGatelib,irow++,0);

        layout->addItem(new QSpacerItem(30,30),irow++,0);
        mCheckMoveNetlist = new QCheckBox("Move imported netlist into project directory");
        layout->addWidget(mCheckMoveNetlist,irow++,0,Qt::AlignLeft);
        mCheckCopyGatelib = new QCheckBox("Copy gate library into project directory");
        layout->addWidget(mCheckCopyGatelib,irow++,0,Qt::AlignLeft);
        layout->addItem(new QSpacerItem(30,30),irow++,0);
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
        connect(dbb,&QDialogButtonBox::accepted,this,&QDialog::accept);
        connect(dbb,&QDialogButtonBox::rejected,this,&QDialog::reject);
        layout->addWidget(dbb,irow++,0,Qt::AlignRight);
    }

    QString ImportNetlistDialog::projectDirectory() const
    {
        return mProjectdir;
    }

    QString ImportNetlistDialog::gateLibrary() const
    {
        return mComboGatelib->currentText();
    }

    bool ImportNetlistDialog::isMoveNetlistChecked() const
    {
        return mCheckMoveNetlist->isChecked();
    }

    bool ImportNetlistDialog::isCopyGatelibChecked() const
    {
        return mCheckCopyGatelib->isChecked();
    }
}
