
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

        mTreeView   = new QTreeView(parent);
        mPinModel = new PinModel(parent);

        mTreeView->setModel(mPinModel);

        mGridLayout->addWidget(mTreeView);

        setLayout(mGridLayout);
    }

    GateLibraryTabPin::GateLibraryTabPin(QWidget* parent, bool editable)
    {
        mGridLayout = new QGridLayout(parent);

        mTreeView   = new QTreeView(parent);
        mPinModel = new PinModel(parent, editable);
        auto pinDelegate = new PinDelegate(parent);
        mTreeView->setItemDelegate(pinDelegate);
        mTreeView->setModel(mPinModel);

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

        //TODO resize to see children columns
        mTreeView->resizeColumnToContents(0);


    }


}
