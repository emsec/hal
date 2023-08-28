#include "gui/file_manager/import_netlist_dialog.h"

#include "gui/gui_utils/graphics.h"
#include "gui/gatelibrary_management/gatelibrary_selection.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpacerItem>
#include <QStyle>

namespace hal
{
    ImportNetlistDialog::ImportNetlistDialog(const QString& filename, QWidget* parent) : QDialog(parent)
    {
        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
        QString gatelibFromHal;
        if (filename.endsWith(".hal"))
        {
            QFile halFile(filename);
            if (halFile.open(QIODevice::ReadOnly))
            {
                QJsonDocument halDoc      = QJsonDocument::fromJson(halFile.readAll());
                const QJsonObject& halObj = halDoc.object();
                if (halObj.contains("netlist") && halObj["netlist"].isObject())
                {
                    const QJsonObject& nlObj = halObj["netlist"].toObject();
                    if (nlObj.contains("gate_library") && nlObj["gate_library"].isString())
                        gatelibFromHal = nlObj["gate_library"].toString();
                }
            }
        }

        int irow            = 0;
        QGridLayout* layout = new QGridLayout(this);
        setWindowTitle("Netlist Import");
        layout->addWidget(new QLabel("Create new HAL project from netlist file\n'" + filename + "'.", this), irow++, 0, Qt::AlignLeft);
        layout->addItem(new QSpacerItem(30, 30), irow++, 0);
        layout->setRowStretch(irow - 1, 20);

        layout->addWidget(new QLabel("Location of project directory:", this), irow++, 0, Qt::AlignLeft);
        setSuggestedProjectDir(filename);

        QFrame* frameProjectdir    = new QFrame(this);
        QHBoxLayout* layProjectdir = new QHBoxLayout(frameProjectdir);
        mEditProjectdir            = new QLineEdit(mProjectdir, frameProjectdir);
        layProjectdir->addWidget(mEditProjectdir);
        QPushButton* butFiledialog = new QPushButton(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath), "", frameProjectdir);
        connect(butFiledialog, &QPushButton::clicked, this, &ImportNetlistDialog::handleFileDialogTriggered);
        layProjectdir->addWidget(butFiledialog);
        mEditProjectdir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(frameProjectdir, irow++, 0);
        layout->addItem(new QSpacerItem(30, 30), irow++, 0);

        mGatelibSelection = new GateLibrarySelection(gatelibFromHal, this);
        mGatelibSelection->setIcon(mSaveIconPath, mSaveIconStyle);
        layout->addWidget(mGatelibSelection, irow++, 0);
        //mGatelibSelection->setDisabled(filename.endsWith(".hal"));

        layout->addItem(new QSpacerItem(30, 30), irow++, 0);
        layout->setRowStretch(irow - 1, 20);
        mCheckMoveNetlist = new QCheckBox("Move imported netlist into project directory");
        layout->addWidget(mCheckMoveNetlist, irow++, 0, Qt::AlignLeft);
        mCheckCopyGatelib = new QCheckBox("Copy gate library into project directory");
        layout->addWidget(mCheckCopyGatelib, irow++, 0, Qt::AlignLeft);
        layout->addItem(new QSpacerItem(30, 30), irow++, 0);
        layout->setRowStretch(irow - 1, 100);
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(dbb, irow++, 0, Qt::AlignRight);
    }

    void ImportNetlistDialog::setSuggestedProjectDir(const QString& filename)
    {
        mProjectdir = filename;
        mProjectdir.remove(QRegularExpression("\\.\\w*$"));

        QString basedir = mProjectdir;
        int count       = 2;

        for (;;)    // loop until non existing directory found
        {
            if (!QFileInfo(mProjectdir).exists())
                return;
            mProjectdir = QString("%1_%2").arg(basedir).arg(count++);
        }
    }

    QString ImportNetlistDialog::projectDirectory() const
    {
        return mEditProjectdir->text();
    }

    void ImportNetlistDialog::handleFileDialogTriggered()
    {
        QString dir = QFileDialog::getSaveFileName(this, "Enter new project directory", QFileInfo(mProjectdir).path(), "", nullptr, QFileDialog::ShowDirsOnly);
        if (dir.isEmpty())
            return;
        mEditProjectdir->setText(dir);
    }

    void ImportNetlistDialog::handleGatelibSelected(bool singleFile)
    {
        if (singleFile)
        {
             mCheckCopyGatelib->setEnabled(true);
        }
        else
        {
            mCheckCopyGatelib->setCheckState(Qt::Unchecked);
            mCheckCopyGatelib->setDisabled(true);
        }
    }

    QString ImportNetlistDialog::gateLibraryPath() const
    {
        return mGatelibSelection->gateLibraryPath();
    }

    bool ImportNetlistDialog::isMoveNetlistChecked() const
    {
        return mCheckMoveNetlist->isChecked();
    }

    bool ImportNetlistDialog::isCopyGatelibChecked() const
    {
        return mCheckCopyGatelib->isChecked();
    }

}    // namespace hal
