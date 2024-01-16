#include "gui/gatelibrary_management/gatelibrary_manager.h"

#include "gui/gatelibrary_management/gatelibrary_graphics_view.h"
#include "gui/graphics_effects/shadow_effect.h"
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
        : QFrame(parent), mLayout(new QGridLayout()), mGateLibrary(nullptr)
    {
        QSplitter* split = new QSplitter(this);
        QWidget* rightWindow = new QWidget(split);
        QGridLayout* rlay = new QGridLayout(rightWindow);
        QDialogButtonBox* bbox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, rightWindow);

        mTableModel = new GatelibraryTableModel(this);
        mContentWidget = new GatelibraryContentWidget(mTableModel,split);

        //pages for the tab widget
        mGeneralTab = new GateLibraryTabGeneral(this);
        mPinTab = new GateLibraryTabPin(this);
        mFlipFlopTab = new GateLibraryTabFlipFlop(this);
        mBooleanFunctionTab = new GateLibraryTabBooleanFunction(this);


        //buttons
        mOkBtn = bbox->button(QDialogButtonBox::Ok);
        mOkBtn->setDisabled(true);
        mCancelBtn = bbox->button(QDialogButtonBox::Cancel);
        mCancelBtn->setEnabled(true);

        //adding pages to the tab widget
        mTabWidget = new QTabWidget(this);
        mTabWidget->addTab(mGeneralTab, "General");
        mTabWidget->addTab(mPinTab, "Pins");
        mTabWidget->addTab(mFlipFlopTab, "Flip Flops");
        mTabWidget->addTab(mBooleanFunctionTab, "Boolean Functions");

        mGraphicsView = new GatelibraryGraphicsView(this);
        QGraphicsScene* sc = new QGraphicsScene(mGraphicsView);
        sc->setSceneRect(0,0,300,1200);
        mGraphicsView->setScene(sc);

        rlay->addWidget(mTabWidget,0,0);
        rlay->addWidget(mGraphicsView,0,1);
        rlay->addWidget(bbox,1,0,1,2);

        // Add widgets to the layout
        split->addWidget(mContentWidget);
        split->addWidget(rightWindow);

        mLayout->addWidget(split);

        //signal - slots
//        connect(mAddBtn, &QPushButton::clicked, this, &GateLibraryManager::handleAddWizard);
        //connect(mEditBtn, &QPushButton::clicked, this, &GateLibraryManager::handleEditWizard);
 //       connect(mTableView, &QTableView::clicked, this, &GateLibraryManager::handleSelectionChanged);
        connect(mCancelBtn, &QPushButton::clicked, this, &GateLibraryManager::handleCancelClicked);
        connect(mContentWidget->mTableView, &QTableView::doubleClicked, this, &GateLibraryManager::handleEditWizard);
        connect(mContentWidget->mTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &GateLibraryManager::handleSelectionChanged);
        connect(mContentWidget->mAddAction, &QAction::triggered, this, &GateLibraryManager::handleAddWizard);
        connect(mContentWidget, &GatelibraryContentWidget::triggerEditType, this, &GateLibraryManager::handleEditWizard);
        setLayout(mLayout);
        repolish();    // CALL FROM PARENT
    }

    void GateLibraryManager::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

    }

    bool GateLibraryManager::initialize(const GateLibrary* gateLibrary)
    {
        if(!gateLibrary)
        {
            if(gNetlist && gNetlist->get_gate_library()){
                mGateLibrary = gNetlist->get_gate_library();
                mReadOnly = true;
            }
            else
            {
                QString title = "Load gate library";
                QString text  = "HAL Gate Library (*.hgl *.lib)";
                QString path  = QDir::currentPath();
                QFile gldpath(":/path/gate_library_definitions");
                if (gldpath.open(QIODevice::ReadOnly))
                    path = QString::fromUtf8(gldpath.readAll());

                //TODO fileDialog throws bad window error
                QString fileName = QFileDialog::getOpenFileName(nullptr, title, path, text, nullptr);
                if (fileName == nullptr)
                    return false;

                if (gPluginRelay->mGuiPluginTable)
                    gPluginRelay->mGuiPluginTable->loadFeature(FacExtensionInterface::FacGatelibParser);
                qInfo() << "selected file: " << fileName;

                auto gateLibrary = gate_library_manager::load(std::filesystem::path(fileName.toStdString()));

                for (auto const elem : gateLibrary->get_gate_types())
                {
                    qInfo() << QString::fromStdString(elem.second->get_name());
                }

                mGateLibrary = gateLibrary;
                mDemoNetlist = netlist_factory::create_netlist(gateLibrary);
            }
            mTableModel->loadFile(mGateLibrary);
        }
        mContentWidget->activate();
        return true;
    }

    void GateLibraryManager::handleEditWizard(const QModelIndex& gate)
    {
        Q_UNUSED(gate);

        GateLibraryWizard wiz(mGateLibrary, getSelectedGate());
        wiz.exec();
    }

    void GateLibraryManager::handleAddWizard()
    {
        //TODO: create an empty Gate
        GateLibraryWizard wiz(mGateLibrary);
        wiz.exec();
    }

    u32 GateLibraryManager::getNextGateId()
    {
        QSet<u32>* occupiedIds = new QSet<u32>;
        u32 freeId = 1;
        for (auto gt : mGateLibrary->get_gate_types()) {
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
        gateType = getSelectedGate();
        qInfo() << "selected " << QString::fromStdString(gateType->get_name());

        //update tabs
        mFlipFlopTab->update(gateType);
        mGeneralTab->update(gateType);
        mBooleanFunctionTab->update(gateType);
        mPinTab->update(gateType);
        if (mDemoNetlist)
        {
            Gate* g = mDemoNetlist->get_gate_by_id(1);
            if (g) mDemoNetlist->delete_gate(g);
            g = mDemoNetlist.get()->create_gate(1,gateType,"Instance of");
            mGraphicsView->showGate(g);
        }
    }

    void GateLibraryManager::handleCancelClicked()
    {
        Q_EMIT close();
    }

    GateType* GateLibraryManager::getSelectedGate()
    {
        PinProxyModel* proxyModel = mContentWidget->mPinProxyModel;
        QModelIndex index = mContentWidget->mTableView->currentIndex();
        QModelIndex sourceIndex = proxyModel->mapToSource(index);

        return mTableModel->getGateTypeAtIndex(sourceIndex.row());
    }

}
