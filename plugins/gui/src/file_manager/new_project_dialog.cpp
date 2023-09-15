#include "gui/file_manager/new_project_dialog.h"

#include "gui/gui_utils/graphics.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpacerItem>

QLineEdit* mEditProjectdir;
QComboBox* mComboGatelib;

namespace hal
{
    NewProjectDialog::NewProjectDialog(QWidget* parent) : QDialog(parent)
    {
        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
        QString suggestedGateLibraryPath;
        QString suggestedGateLibraryName;

        int suggestedProjectCount = 0;
        do
        {
            ++suggestedProjectCount;
            mProjectdir = QDir::current().absoluteFilePath(QString("hal_project_%1").arg(suggestedProjectCount));
        } while (QFileInfo(mProjectdir).exists());

        int irow            = 0;
        QGridLayout* layout = new QGridLayout(this);
        setWindowTitle("Netlist Import");
        layout->addWidget(new QLabel("Create new empty HAL project:", this), irow++, 0, Qt::AlignLeft);
        layout->addItem(new QSpacerItem(30, 30), irow++, 0);
        layout->setRowStretch(irow - 1, 20);

        layout->addWidget(new QLabel("Location of project directory:", this), irow++, 0, Qt::AlignLeft);
        QFrame* frameProjectdir    = new QFrame(this);
        QHBoxLayout* layProjectdir = new QHBoxLayout(frameProjectdir);
        mEditProjectdir            = new QLineEdit(mProjectdir, frameProjectdir);
        layProjectdir->addWidget(mEditProjectdir);
        QPushButton* butFiledialog = new QPushButton(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath), "", frameProjectdir);
        connect(butFiledialog, &QPushButton::clicked, this, &NewProjectDialog::handleFileDialogTriggered);
        layProjectdir->addWidget(butFiledialog);
        mEditProjectdir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(frameProjectdir, irow++, 0);
        layout->addItem(new QSpacerItem(30, 30), irow++, 0);

        QLabel* labGatelib = new QLabel("Gate library:", this);
        layout->addWidget(labGatelib, irow++, 0, Qt::AlignLeft);
        mComboGatelib = new QComboBox(this);
        mComboGatelib->addItem(QString());
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

        mComboGatelib->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(mComboGatelib, irow++, 0);

        layout->addItem(new QSpacerItem(30, 30), irow++, 0);
        layout->setRowStretch(irow - 1, 20);
        mCheckCopyGatelib = new QCheckBox("Copy gate library into project directory");
        layout->addWidget(mCheckCopyGatelib, irow++, 0, Qt::AlignLeft);
        layout->addItem(new QSpacerItem(30, 30), irow++, 0);
        layout->setRowStretch(irow - 1, 100);
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(dbb, irow++, 0, Qt::AlignRight);
        mOkButton = dbb->button(QDialogButtonBox::Ok);
        connect(mComboGatelib, &QComboBox::currentTextChanged, this, &NewProjectDialog::handleGateLibraryPathChanged);
        handleGateLibraryPathChanged(mComboGatelib->currentText());
    }

    QString NewProjectDialog::projectDirectory() const
    {
        return mEditProjectdir->text();
    }

    void NewProjectDialog::handleFileDialogTriggered()
    {
        QString dir = QFileDialog::getSaveFileName(this, "Enter new project directory", QFileInfo(mProjectdir).path(), "", nullptr, QFileDialog::ShowDirsOnly);
        if (dir.isEmpty())
            return;
        mEditProjectdir->setText(dir);
    }

    void NewProjectDialog::handleGateLibraryPathChanged(const QString& txt)
    {
        if (mGateLibraryMap.value(txt, -1) < 0)
        {
            mCheckCopyGatelib->setCheckState(Qt::Unchecked);
            mCheckCopyGatelib->setDisabled(true);
            if (mOkButton)
                mOkButton->setDisabled(true);
        }
        else
        {
            mCheckCopyGatelib->setEnabled(true);
            if (mOkButton)
                mOkButton->setEnabled(true);
        }
    }

    QString NewProjectDialog::gateLibraryPath() const
    {
        QString seltxt = mComboGatelib->currentText();
        int inx        = mGateLibraryMap.value(seltxt, -1);
        if (inx < 0)
            return QString();
        return mGateLibraryPath.at(inx);
    }

    bool NewProjectDialog::isCopyGatelibChecked() const
    {
        return mCheckCopyGatelib->isChecked();
    }
}    // namespace hal
