#include "gui/gatelibrary_management/gatelibrary_manager.h"

#include "gui/graphics_effects/shadow_effect.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/plugin_system/fac_extension_interface.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <gui/gui_globals.h>
namespace hal
{
    GateLibraryManager::GateLibraryManager(QWidget* parent)
        : QFrame(parent), mLayout(new QGridLayout()), mGateLibrary(nullptr)
    {
        //TODO create layout and widgets
        mLayout->setSpacing(20);

        mTableView = new QTableView(this);
        mTableModel = new GatelibraryTableModel(this);
        mTableView->setModel(mTableModel);
        mTableView->verticalHeader()->setVisible(false);

        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::SingleSelection);

        //pages for the tab widget
        mGeneralTab = new GateLibraryTabGeneral(this);
        mPinTab = new GateLibraryTabPin(this);
        mFlipFlopTab = new GateLibraryTabFlipFlop(this);
        mBooleanFunctionTab = new GateLibraryTabBooleanFunction(this);


        //buttons
        mEditBtn = new QPushButton("Edit", this);
        mAddBtn = new QPushButton("Add", this);
        mCancelBtn = new QPushButton("Cancel", this);


        //adding pages to the tab widget
        mTabWidget = new QTabWidget(this);
        mTabWidget->addTab(mGeneralTab, "General");
        mTabWidget->addTab(mPinTab, "Pins");
        mTabWidget->addTab(mFlipFlopTab, "Flip Flops");
        mTabWidget->addTab(mBooleanFunctionTab, "Boolean Functions");


        // Add widgets to the layout
        mLayout->addWidget(mTableView,0,0,1,2);
        mLayout->addWidget(mEditBtn,1,0);
        mLayout->addWidget(mAddBtn,1,1);
        mLayout->addWidget(mTabWidget,0,2);
        mLayout->addWidget(mCancelBtn, 1, 2);


        //signal - slots
        connect(mAddBtn, &QPushButton::clicked, this, &GateLibraryManager::handleAddWizard);
        //connect(mEditBtn, &QPushButton::clicked, this, &GateLibraryManager::handleEditWizard);
        connect(mTableView, &QTableView::clicked, this, &GateLibraryManager::handleSelectionChanged);
        connect(mCancelBtn, &QPushButton::clicked, this, &GateLibraryManager::handleCancelClicked);
        connect(mTableView, &QTableView::doubleClicked, this, &GateLibraryManager::handleEditWizard);

        setLayout(mLayout);
        repolish();    // CALL FROM PARENT
    }

    void GateLibraryManager::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    bool GateLibraryManager::initialize(GateLibrary* gateLibrary)
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
            }
            mTableModel->loadFile(mGateLibrary);
            mEditBtn->setEnabled(!mReadOnly);
            mAddBtn->setEnabled(!mReadOnly);

        }
        return true;
    }

    void GateLibraryManager::handleEditWizard(const QModelIndex& gate)
    {
        GateLibraryWizard wiz(mGateLibrary, mTableModel->getGateTypeAtIndex(gate.row()));
        wiz.exec();
    }

    void GateLibraryManager::handleAddWizard()
    {
        GateLibraryWizard wiz;
        wiz.exec();
    }

    void GateLibraryManager::handleSelectionChanged(const QModelIndex& index)
    {
        GateType* gate;
        //get selected gate
        gate = mTableModel->getGateTypeAtIndex(index.row());
        qInfo() << "selected " << QString::fromStdString(gate->get_name());

        //update tabs
        mFlipFlopTab->update(gate);
        mGeneralTab->update(gate);
        mBooleanFunctionTab->update(gate);
        mPinTab->update(gate);
    }

    void GateLibraryManager::handleCancelClicked()
    {
        Q_EMIT close();
    }

}
