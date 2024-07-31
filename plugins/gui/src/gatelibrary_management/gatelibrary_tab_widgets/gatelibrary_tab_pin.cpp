
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_pin.h"

#include "gui/gui_globals.h"

#include <QTreeView>
#include <gui/pin_model/pin_model.h>
#include <gui/pin_model/pin_delegate.h>

namespace hal
{

    GateLibraryTabPin::GateLibraryTabPin(QWidget* parent) : GateLibraryTabInterface(parent)
    {
        mGridLayout = new QGridLayout(this);

        mTreeView   = new QTreeView(this);
        mPinModel = new PinModel(this);

        mTreeView->setModel(mPinModel);

        mGridLayout->addWidget(mTreeView);

        setLayout(mGridLayout);
    }

    GateLibraryTabPin::GateLibraryTabPin(QWidget* parent, bool editable)
    {
        mGridLayout = new QGridLayout(this);

        mTreeView   = new QTreeView(this);
        mPinModel = new PinModel(this, editable);
        auto pinDelegate = new PinDelegate(this);
        mTreeView->setItemDelegate(pinDelegate);
        mTreeView->setModel(mPinModel);

        mGridLayout->addWidget(mTreeView);

        setLayout(mGridLayout);
    }

    void GateLibraryTabPin::update(GateType* gate)
    {

        mPinModel->setGate(gate);

        //TODO resize to see children columns
        mTreeView->resizeColumnToContents(0);
    }

    QTreeView* GateLibraryTabPin::getTreeView()
    {
        return mTreeView;
    }

    PinModel* GateLibraryTabPin::getPinModel()
    {
        return mPinModel;
    }

}
