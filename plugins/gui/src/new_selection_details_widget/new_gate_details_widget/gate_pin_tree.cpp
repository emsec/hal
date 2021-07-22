#include "gui/new_selection_details_widget/new_gate_details_widget/gate_pin_tree.h"
#include "gui/new_selection_details_widget/models/pin_tree_model.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include <QHeaderView>
#include <QtDebug>

namespace hal
{

    GatePinTree::GatePinTree(QWidget *parent) : QTreeView(parent), mPinModel(new PinTreeModel(this)), mGateID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        header()->setStretchLastSection(true);
        setModel(mPinModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &GatePinTree::handleContextMenuRequested);

    }

    void GatePinTree::setContent(u32 gateID)
    {
        Gate* g = gNetlist->get_gate_by_id(gateID);
        if(!g) return;

        mPinModel->setGate(g);
        mGateID = gateID;
    }

    void GatePinTree::setContent(Gate *g)
    {
        if(!g) return;

        mPinModel->setGate(g);
        mGateID = g->get_id();
    }

    void GatePinTree::removeContent()
    {
        mGateID = -1;
        mPinModel->clear();
    }

    void GatePinTree::handleContextMenuRequested(const QPoint &pos)
    {
        QModelIndex idx = indexAt(pos);
        if(!idx.isValid())
            return;

        TreeItem* clickedItem = mPinModel->getItemFromIndex(idx);
        qDebug() << "clicked name of item " << clickedItem->getData(0).toString();

    }

}
