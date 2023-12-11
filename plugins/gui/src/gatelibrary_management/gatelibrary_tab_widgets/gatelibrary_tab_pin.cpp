
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_pin.h"

#include "gui/gui_globals.h"

#include <QTreeView>
#include <gui/pin_model/pin_model.h>
#include <gui/pin_model/pin_delegate.h>

namespace hal
{

    GateLibraryTabPin::GateLibraryTabPin(QWidget* parent) : QWidget(parent)
    {
        mGridLayout = new QGridLayout(parent);

        auto pinDelegate = new PinDelegate(parent);
        mTreeView   = new QTreeView(parent);
        mPinModel = new PinModel(parent);

        mTreeView->setModel(mPinModel);
        mTreeView->setItemDelegate(pinDelegate);

        mGridLayout->addWidget(mTreeView);

        setLayout(mGridLayout);
    }

    void GateLibraryTabPin::update(GateType* gate)
    {

        if(!gate){
            //TODO make default look

            return;
        }

        mPinModel->setGate(gate);
        mTreeView->resizeColumnToContents(0);

    }


}
