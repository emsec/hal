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
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

QLineEdit* mEditProjectdir;
QComboBox* mComboGatelib;
QCheckBox* mCheckMoveNetlist;

namespace hal {
    ImportNetlistDialog::ImportNetlistDialog(const QString& filename, QWidget *parent)
        : QDialog(parent)
    {
        QString suggestedGateLibraryPath;
        QString suggestedGateLibraryName;
        if (filename.endsWith(".hal"))
        {
            QFile halFile(filename);
            if (halFile.open(QIODevice::ReadOnly))
            {
                QJsonDocument halDoc = QJsonDocument::fromJson(halFile.readAll());
                const QJsonObject& halObj = halDoc.object();
                if (halObj.contains("netlist") && halObj["netlist"].isObject())
                {
                    const QJsonObject& nlObj = halObj["netlist"].toObject();
                    if (nlObj.contains("gate_library") && nlObj["gate_library"].isString())
                        suggestedGateLibraryPath = nlObj["gate_library"].toString();
                }
            }
        }

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
        for (const std::filesystem::path path : gate_library_manager::get_all_path())
        {
            int n = mGateLibraryPath.size();
            QString qName = QString::fromStdString(path.filename());
            mGateLibraryMap.insert(qName,n);
            QString qPath = QString::fromStdString(path.string());
            mGateLibraryPath.append(qPath);
            if (qPath == suggestedGateLibraryPath) suggestedGateLibraryName = qName;
        }
        if (suggestedGateLibraryName.isEmpty() && !suggestedGateLibraryPath.isEmpty())
        {
            // suggested gate library not found in default path
            QFileInfo info(suggestedGateLibraryPath);
            suggestedGateLibraryName = info.fileName();
            int n = mGateLibraryPath.size();
            mGateLibraryMap.insert(suggestedGateLibraryName,n);
            mGateLibraryPath.append(suggestedGateLibraryPath);
        }
        mComboGatelib->addItems(mGateLibraryMap.keys());
        if (!suggestedGateLibraryName.isEmpty())
            mComboGatelib->setCurrentText(suggestedGateLibraryName);
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
        connect(mComboGatelib,&QComboBox::currentTextChanged,this,&ImportNetlistDialog::handleGateLibraryPathChanged);
        handleGateLibraryPathChanged(mComboGatelib->currentText());
    }

    QString ImportNetlistDialog::projectDirectory() const
    {
        return mEditProjectdir->text();
    }

    void ImportNetlistDialog::handleGateLibraryPathChanged(const QString& txt)
    {
        if (mGateLibraryMap.value(txt,-1) < 0)
        {
            mCheckCopyGatelib->setCheckState(Qt::Unchecked);
            mCheckCopyGatelib->setDisabled(true);
        }
        else
            mCheckCopyGatelib->setEnabled(true);
    }

    QString ImportNetlistDialog::gateLibraryPath() const
    {
        QString seltxt = mComboGatelib->currentText();
        int inx = mGateLibraryMap.value(seltxt,-1);
        if (inx < 0) return QString();
        return mGateLibraryPath.at(inx);
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
