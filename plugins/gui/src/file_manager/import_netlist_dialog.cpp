#include "gui/file_manager/import_netlist_dialog.h"

#include "gui/gui_utils/graphics.h"
#include "gui/export/import_project_dialog.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>

namespace hal
{
    ImportNetlistDialog::ImportNetlistDialog(const QString& filename, QWidget* parent) : QDialog(parent)
    {
        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
        QString suggestedGateLibraryPath;
        QString suggestedGateLibraryName;
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
                        suggestedGateLibraryPath = nlObj["gate_library"].toString();
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
        mProjectdir = ImportProjectDialog::suggestedProjectDir(filename);

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

        QLabel* labGatelib = new QLabel("Gate library:", this);
        layout->addWidget(labGatelib, irow++, 0, Qt::AlignLeft);
        mComboGatelib = new QComboBox(this);
        if (filename.endsWith(".hal"))
        {
            mComboGatelib->setDisabled(true);
            labGatelib->setDisabled(true);
        }
        else
        {
            mComboGatelib->addItem("(Auto detect)");
            for (const std::filesystem::path& path : gate_library_manager::get_all_path())
            {
                int n         = mGateLibraryPath.size();
                QString qName = QString::fromStdString(path.filename());
                mGateLibraryMap.insert(qName, n);
                QString qPath = QString::fromStdString(path.string());
                mGateLibraryPath.append(qPath);
                if (qPath == suggestedGateLibraryPath)
                    suggestedGateLibraryName = qName;
            }
            if (suggestedGateLibraryName.isEmpty() && !suggestedGateLibraryPath.isEmpty())
            {
                // suggested gate library not found in default path
                QFileInfo info(suggestedGateLibraryPath);
                suggestedGateLibraryName = info.fileName();
                int n                    = mGateLibraryPath.size();
                mGateLibraryMap.insert(suggestedGateLibraryName, n);
                mGateLibraryPath.append(suggestedGateLibraryPath);
            }
            mComboGatelib->addItems(mGateLibraryMap.keys());
            if (!suggestedGateLibraryName.isEmpty())
                mComboGatelib->setCurrentText(suggestedGateLibraryName);
        }
        mComboGatelib->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(mComboGatelib, irow++, 0);

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
        connect(mComboGatelib, &QComboBox::currentTextChanged, this, &ImportNetlistDialog::handleGateLibraryPathChanged);
        handleGateLibraryPathChanged(mComboGatelib->currentText());
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

    void ImportNetlistDialog::handleGateLibraryPathChanged(const QString& txt)
    {
        if (mGateLibraryMap.value(txt, -1) < 0)
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
        int inx        = mGateLibraryMap.value(seltxt, -1);
        if (inx < 0)
            return QString();
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
}    // namespace hal
