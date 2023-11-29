#include "gui/gatelibrary_management/gatelibrary_manager.h"

#include "gui/frames/labeled_frame.h"
#include "gui/graphics_effects/shadow_effect.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/welcome_screen/open_file_widget.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/plugin_system/fac_extension_interface.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>

#include <gui/gui_globals.h>

#include <QTableWidget>
namespace hal
{
    GateLibraryManager::GateLibraryManager(QWidget* parent)
        : QFrame(parent), mLayout(new QGridLayout())
    {
        //TODO create layout and widgets
        mTableView = new QTableView(this);
        mTableModel = new GatelibraryTableModel(this);
        mTableView->verticalHeader()->setVisible(false);

        mTableView->setModel(mTableModel);
        mLayout->addWidget(mTableView);


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
            //TODO Check if a gate library is already in use
            // if yes then open it in read-only mode
            // else open file dialog and select one to parse

            QString title = "Load gate library";
            QString text  = "HAL Gate Library (*.hgl *.lib)";
            QString path  = QDir::currentPath();



            //TODO fileDialog throws bad window error
            QString fileName = QFileDialog::getOpenFileName(nullptr, title, path, text, nullptr);
            if(fileName == nullptr)
                return false;

            if (gPluginRelay->mGuiPluginTable)
                gPluginRelay->mGuiPluginTable->loadFeature(FacExtensionInterface::FacGatelibParser);
            qInfo() << "selected file: " << fileName;

            auto gateLibrary = gate_library_manager::load(std::filesystem::path(fileName.toStdString()));

            for (auto elem : gateLibrary->get_gate_types())
            {
                qInfo() << QString::fromStdString(elem.second->get_name());
            }

            mGateLibrary = gateLibrary;
            mTableModel->loadFile(fileName);

        }
        return true;
    }
}
