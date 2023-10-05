#include "gui/gatelibrary_management/gatelibrary_selection.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/utilities/log.h"
#include <filesystem>
#include <QFileInfo>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>

namespace hal {
    GateLibrarySelection::GateLibrarySelection(const QString &defaultGl, QWidget* parent)
        : QFrame(parent)
    {
        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);

        QVBoxLayout* vlayout = new QVBoxLayout(this);
        QLabel* labGatelib = new QLabel("Gate library:", this);
        vlayout->addWidget(labGatelib);


        QHBoxLayout* hlayout = new QHBoxLayout;
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setFrameStyle(QFrame::Sunken | QFrame::Panel);

        mComboGatelib = new QComboBox(this);
        GateLibrarySelectionTable* glTable = new GateLibrarySelectionTable(defaultGl.isEmpty(),this);
        mComboGatelib->setModel(glTable);

        mComboGatelib->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        hlayout->addWidget(mComboGatelib);

        mWarningMsg = new QLabel(this);
        mWarningMsg->setObjectName("warningMsg");
        if (!defaultGl.isEmpty())
        {
            int inx = glTable->getIndexByPath(defaultGl);
            if (inx < 0)
            {
                mWarningMsg->setText("Gate library '" + defaultGl + "' not found,\nplease select gate library from list.");
            }
            else
            {
                mComboGatelib->setCurrentIndex(inx);
                if (glTable->isWarnSubstitute())
                   mWarningMsg->setText("Gate library '" + defaultGl + "' not found,\na substitute has been suggested.");
            }
        }


        mInvokeFileDialog = new QPushButton(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath),"",this);
        connect(mInvokeFileDialog, &QPushButton::clicked, this, &GateLibrarySelection::handleInvokeFileDialog);
        hlayout->addWidget(mInvokeFileDialog);

        vlayout->addLayout(hlayout);
        vlayout->addWidget(mWarningMsg);
        if (mWarningMsg->text().isEmpty())
            mWarningMsg->hide();

        mCheckFullPath = new QCheckBox("Show full path");
        mCheckFullPath->setChecked(false);
        connect(mCheckFullPath,&QCheckBox::toggled,this,&GateLibrarySelection::handleShowFullPath);
        vlayout->addWidget(mCheckFullPath);
        connect(mComboGatelib,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&GateLibrarySelection::handleGatelibIndexChanged);
    }

    void GateLibrarySelection::handleGatelibIndexChanged(int inx)
    {
        Q_UNUSED(inx);
        mWarningMsg->clear();
        mWarningMsg->hide();
    }

    void GateLibrarySelection::handleInvokeFileDialog()
    {
        QString glFilename = QFileDialog::getOpenFileName(this, "Select Gate Library", QDir::currentPath(), "HGL Files (*.hgl);;Lib Files (*.lib)");
        if (glFilename.isEmpty()) return;
        int inx = static_cast<GateLibrarySelectionTable*>(mComboGatelib->model())->addGateLibrary(glFilename);
        mComboGatelib->setCurrentIndex(inx);
    }

    QString GateLibrarySelection::gateLibraryPath() const
    {
        const GateLibrarySelectionTable* glst = static_cast<const GateLibrarySelectionTable*>(mComboGatelib->model());
        return glst->gateLibraryPath(mComboGatelib->currentIndex());
    }

    void GateLibrarySelection::handleShowFullPath(bool checked)
    {
        int inx = mComboGatelib->currentIndex();
        static_cast<GateLibrarySelectionTable*>(mComboGatelib->model())->handleShowFullPath(checked);
        mComboGatelib->setCurrentIndex(inx);
    }

    //-----------------------------------------------

    QVariant GateLibrarySelectionEntry::data(int column, bool fullPath) const
    {
        if (fullPath)
        {
            if (mCount<0)
                return mName;
            return mPath;
        }

        switch(column)
        {
        case 0:
            if (mCount <= 0) return mName;
            return QString("%1 (%2)").arg(mName).arg(mCount+1);
        case 1: return mPath;
        }
        return QVariant();
    }

    GateLibrarySelectionTable::GateLibrarySelectionTable(bool addAutoDetect, QObject *parent)
        : QAbstractTableModel(parent), mShowFullPath(false), mWarnSubstitute(false)
    {
        if (addAutoDetect)
            mEntries.append(GateLibrarySelectionEntry("(Auto detect)", "", -1));
        else
            mEntries.append(GateLibrarySelectionEntry("", "", -1));
        QMap<QString,int> nameMap;
        for (const std::filesystem::path& path : gate_library_manager::get_all_path())
        {
            QString name = QString::fromStdString(path.filename());
            mEntries.append(GateLibrarySelectionEntry(name, QString::fromStdString(path.string()), nameMap[name]++));
        }
    }

    int GateLibrarySelectionTable::columnCount(const QModelIndex& index) const
    {
        Q_UNUSED(index);
        return 2;
    }

    int GateLibrarySelectionTable::rowCount(const QModelIndex& index) const
    {
        Q_UNUSED(index);
        return mEntries.size();
    }

    QVariant GateLibrarySelectionTable::data(const QModelIndex& index, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if (index.row() >= mEntries.size()) return QVariant();
        return (mEntries.at(index.row()).data(index.column(),mShowFullPath));
    }

    int GateLibrarySelectionTable::addGateLibrary(const QString &path)
    {
        int inx = 0;
        QMap<QString,int> nameMap;

        for (const GateLibrarySelectionEntry& glse : mEntries)
        {
            if (glse.path() == path) return inx; // path already in table
            nameMap[glse.name()]++;
            inx++;
        }

        QString name = QFileInfo(path).fileName();
        beginResetModel();
        mEntries.append(GateLibrarySelectionEntry(name,path,nameMap[name]++));
        return inx;
    }

    int GateLibrarySelectionTable::getIndexByPath(const QString& path)
    {
        // try exact path
        int inx = 0;
        for (const GateLibrarySelectionEntry& glse : mEntries)
        {
            if (glse.path() == path) return inx;
            ++inx;
        }

        // existing gatelib path not in list : add it
        if (QFileInfo(path).exists())
            return addGateLibrary(path);

        // try name
        QString name = QFileInfo(path).fileName();
        inx = 0;
        for (const GateLibrarySelectionEntry& glse : mEntries)
        {
            if (glse.name() == name)
            {
                log_info("gui", "Requested gate library '{}' not found, suggest '{}' instead.", path.toStdString(), glse.path().toStdString());
                mWarnSubstitute = true;
                return inx;
            }
            ++inx;
        }

        log_info("gui", "Requested gate library '{}' not found.", path.toStdString());
        return -1;
    }


    QString GateLibrarySelectionTable::gateLibraryPath(int inx) const
    {
        if (inx < 0 || inx >= mEntries.size()) return QString();
        return mEntries.at(inx).path();
    }

    void GateLibrarySelectionTable::handleShowFullPath(bool checked)
    {
        beginResetModel();
        mShowFullPath = checked;
        endResetModel();
    }
}
