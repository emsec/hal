#include "gui/gatelibrary_management/gatelibrary_manager.h"

#include "gui/gatelibrary_management/gatelibrary_graphics_view.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/main_window/main_window.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/plugin_system/fac_extension_interface.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QTabWidget>
#include <QTableView>

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QSplitter>
#include <QDialogButtonBox>
#include <gui/gui_globals.h>

namespace hal
{
    GateLibraryManager::GateLibraryManager(MainWindow *parent)
        : QFrame(parent), mFrameWidth(0), mLayout(new QGridLayout())
    {
        mSplitter = new QSplitter(this);
        QWidget* rightWindow = new QWidget(mSplitter);
        QGridLayout* rlay = new QGridLayout(rightWindow);
        QDialogButtonBox* bbox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, rightWindow);

        mTableModel = new GatelibraryTableModel(this);
        mContentWidget = new GatelibraryContentWidget(mTableModel,mSplitter);

        //pages for the tab widget
        mGeneralTab = new GateLibraryTabGeneral(this);
        mPinTab = new GateLibraryTabPin(false,this);
        mBooleanFunctionTab = new GateLibraryTabTruthTable(this);

        //buttons
        mOkBtn = bbox->button(QDialogButtonBox::Ok);
        mCancelBtn = bbox->button(QDialogButtonBox::Cancel);
        mCancelBtn->setEnabled(true);

        //adding pages to the tab widget
        mTabWidget = new QTabWidget(this);
        mTabWidget->addTab(mGeneralTab, "Gate Type");
        mTabWidget->addTab(mPinTab, "Pins");
        mTabWidget->addTab(mBooleanFunctionTab, "Truth Table"); // TODO : implement truth table

        mGraphicsView = new GatelibraryGraphicsView(this);
        QGraphicsScene* sc = new QGraphicsScene(mGraphicsView);
        sc->setSceneRect(0,0,300,1200);
        mGraphicsView->setScene(sc);

        rlay->addWidget(mTabWidget,0,0);
        rlay->addWidget(mGraphicsView,0,1);
        rlay->addWidget(bbox,1,0,1,2);

        // Add widgets to the layout
        mSplitter->addWidget(mContentWidget);
        mSplitter->addWidget(rightWindow);

        mLayout->addWidget(mSplitter);

        connect(mCancelBtn, &QPushButton::clicked, this, &GateLibraryManager::handleCancelClicked);
        connect(mOkBtn, &QPushButton::clicked, this, &GateLibraryManager::handleOkClicked);
        connect(mContentWidget, &GatelibraryContentWidget::triggerCurrentSelectionChanged, this, &GateLibraryManager::handleSelectionChanged);
        connect(mContentWidget->mAddAction, &QAction::triggered, this, &GateLibraryManager::handleAddWizard);
        connect(mContentWidget, &GatelibraryContentWidget::triggerEditType, this, &GateLibraryManager::handleEditWizard);
        connect(mContentWidget, &GatelibraryContentWidget::triggerDeleteType, this, &GateLibraryManager::handleDeleteType);
        connect(mContentWidget, &GatelibraryContentWidget::triggerDoubleClicked, this, &GateLibraryManager::handleEditWizard);

        setLayout(mLayout);
        repolish();    // CALL FROM PARENT
    }

    void GateLibraryManager::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    void GateLibraryManager::handleCreateAction()
    {
        mCreationMode = true; //needed for callUnsavedChangesWindow
        mContentWidget->mCreationMode = mCreationMode;
        std::filesystem::path path;
        QFile gldpath(":/path/gate_library_definitions");
        if (gldpath.open(QIODevice::ReadOnly))
        {
            path = QString::fromUtf8(gldpath.readAll()).toStdString();
            QStringList allNames = QDir(QString::fromStdString(path.string())).entryList();
            if(allNames.contains("unnamed_gate_library.hgl"))
            {
                u32 cnt = 1;
                while(allNames.contains(QString("unnamed_gate_library(%1).hgl").arg(cnt)))
                    cnt++;
                path += std::filesystem::path(QString("/unnamed_gate_library(%1).hgl").arg(cnt).toStdString());
            }
            else path += "/unnamed_gate_library.hgl";
        }

        if(gFileStatusManager->isGatelibModified())
        {
            if(!callUnsavedChangesWindow()) return;
            else window()->setWindowTitle(QString("GateLibrary %1").arg(QDir::home().relativeFilePath(QString::fromStdString(path.string()))));
        }
        mContentWidget->mGateLibrary = new GateLibrary(path, path.string());
        initialize(mContentWidget->mGateLibrary);
        mCreationMode = false;
    }

    void GateLibraryManager::handleSaveAction()
    {
        mContentWidget->handleSaveAction();
    }

    void GateLibraryManager::handleSaveAsAction()
    {
        mContentWidget->handleSaveAsAction();
    }

    bool GateLibraryManager::initialize(GateLibrary* gateLibrary, bool readOnly)
    {
        if (!mContentWidget->initialize(gateLibrary,readOnly))
            return false;

        if (gateLibrary) mProjectName = window()->windowTitle();

        mDemoNetlist = netlist_factory::create_netlist(mContentWidget->mGateLibrary);
        mGraphicsView->showGate(nullptr);
        updateTabs(nullptr);
        mTableModel->loadFile(mContentWidget->mGateLibrary);
        mContentWidget->activate(mContentWidget->mReadOnly);

        mContentWidget->toggleSelection(false);
        if (!mContentWidget->mReadOnly) gFileStatusManager->gatelibOpened();
        return true;
    }

    void GateLibraryManager::handleEditWizard(const QModelIndex& index)
    {
        if(mContentWidget->mReadOnly)
            return;
        GateType* gtEditable = mTableModel->getGateTypeAtIndex(index.row());
        mWizard = new GateLibraryWizard(mContentWidget->mGateLibrary, gtEditable);
        connect(mWizard, &GateLibraryWizard::triggerUnsavedChanges, mContentWidget, &GatelibraryContentWidget::handleUnsavedChanges);

        mWizard->exec();
        initialize(mContentWidget->mGateLibrary);

        for (int r=0; r<mTableModel->rowCount(); r++) {
            if(mTableModel->getGateTypeAtIndex(r) == gtEditable)
                mContentWidget->mTableView->selectRow(r);
        }
    }

    void GateLibraryManager::handleAddWizard()
    {
        mWizard = new GateLibraryWizard(mContentWidget->mGateLibrary);
        connect(mWizard, &GateLibraryWizard::triggerUnsavedChanges, mContentWidget, &GatelibraryContentWidget::handleUnsavedChanges);

        mWizard->exec();
        initialize(mContentWidget->mGateLibrary);

        for (int r=0; r<mTableModel->rowCount(); r++) {
            if(mTableModel->getGateTypeAtIndex(r) == mWizard->getRecentCreatedGate())
                mContentWidget->mTableView->selectRow(r);
        }
    }

    void GateLibraryManager::handleDeleteType(QModelIndex index)
    {
        GateType* gate = mTableModel->getGateTypeAtIndex(index.row());
        mContentWidget->mGateLibrary->remove_gate_type(gate->get_name());
        initialize(mContentWidget->mGateLibrary);
        gFileStatusManager->gatelibChanged();
        //qInfo() << "handleDeleteType " << QString::fromStdString(gate->get_name()) << ":" << gate->get_id();
    }

    u32 GateLibraryManager::getNextGateId()
    {
        QSet<u32>* occupiedIds = new QSet<u32>;
        u32 freeId = 1;
        for (auto gt : mContentWidget->mGateLibrary->get_gate_types()) {
            occupiedIds->insert(gt.second->get_id());
        }
        while(occupiedIds->contains(freeId))
        {
            freeId++;
        }
        return freeId;
    }

    void GateLibraryManager::handleSelectionChanged(const QModelIndex& index, const QModelIndex &prevIndex)
    {
        Q_UNUSED(prevIndex);
        GateType* gateType;
        //get selected gate
        gateType = mTableModel->getGateTypeAtIndex(index.row());
        //qInfo() << "selected " << QString::fromStdString(gateType->get_name());

        if(!mContentWidget->mReadOnly) mContentWidget->toggleSelection(true);
        //update tabs
        updateTabs(gateType);
        if (mDemoNetlist)
        {
            Gate* g = mDemoNetlist->get_gate_by_id(1);
            if (g) mDemoNetlist->delete_gate(g);
            g = mDemoNetlist.get()->create_gate(1,gateType,"Instance of");
            mGraphicsView->showGate(g);
        }
    }

    void GateLibraryManager::emitClose()
    {
        gFileStatusManager->gatelibClosed();
        Q_EMIT close();
    }


    void GateLibraryManager::handleCancelClicked()
    {
        if(!gFileStatusManager->isGatelibModified())
        {
            if(!mContentWidget->mReadOnly) window()->setWindowTitle("HAL");
            else window()->setWindowTitle(mProjectName);
            emitClose();
        }
        else
        {
            callUnsavedChangesWindow();
        }
    }

    void GateLibraryManager::handleOkClicked()
    {
        if(gFileStatusManager->isGatelibModified())
            mContentWidget->handleSaveAction();
        emitClose();
    }

    GateType* GateLibraryManager::getSelectedGate()
    {
        PinProxyModel* proxyModel = mContentWidget->mPinProxyModel;
        QModelIndex index = mContentWidget->mTableView->currentIndex();
        QModelIndex sourceIndex = proxyModel->mapToSource(index);

        return mTableModel->getGateTypeAtIndex(sourceIndex.row());
    }

    void GateLibraryManager::updateTabs(GateType* gateType)
    {
        mGeneralTab->update(gateType);
        mBooleanFunctionTab->update(gateType);
        mPinTab->update(gateType);
    }

    bool GateLibraryManager::isReadOnly() const
    {
        return mContentWidget->mReadOnly;
    }

    bool GateLibraryManager::callUnsavedChangesWindow()
    {
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("Unsaved changes");
        msgBox->setInformativeText("The current gate library has been modified. Do you want to save your changes or discard them?");
        msgBox->setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        int r = msgBox->exec();
        switch(r)
        {
            case QMessageBox::Save:
                mContentWidget->handleSaveAsAction();
                emitClose();
            break;
            case QMessageBox::Discard:
                if(!mCreationMode)
                {
                    gate_library_manager::remove(std::filesystem::path(mContentWidget->mGateLibrary->get_path()));
                    mDemoNetlist.reset(); //delete unique pointer
                    gFileStatusManager->gatelibSaved();
                    if(!mContentWidget->mReadOnly) window()->setWindowTitle("HAL");
                    else window()->setWindowTitle(mProjectName);
                    emitClose();
                }
                else
                {
                    gate_library_manager::remove(std::filesystem::path(mContentWidget->mGateLibrary->get_path()));
                    mDemoNetlist.reset(); //delete unique pointer
                    gFileStatusManager->gatelibSaved();
                }
            break;
            case QMessageBox::Cancel:
                msgBox->reject();
                return false;
        }
        return true;
    }

    void GateLibraryManager::resizeEvent(QResizeEvent* evt)
    {
        QFrame::resizeEvent(evt);
        if (evt->size().width() != mFrameWidth)
        {
            mFrameWidth = evt->size().width();
            QList<int> splitSizes;
            // divide available size in percent
            splitSizes << mFrameWidth * 27 / 100 << mFrameWidth * 73 / 100;
            mSplitter->setSizes(splitSizes);
        }
    }
}
