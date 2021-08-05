#include "gui/new_selection_details_widget/new_gate_details_widget/gate_pin_tree.h"
#include "gui/new_selection_details_widget/models/pin_tree_model.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include "gui/new_selection_details_widget/py_code_provider.h"

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
        QMenu menu;

        //extract the net(s) if its a pin, a list of the pinnames if its a grouping
        if(mPinModel->getTypeOfItem(clickedItem) == PinTreeModel::itemType::pin)
        {
            QList<int> netIdsOfItem = mPinModel->getNetIDsOfTreeItem(clickedItem);
            QString pythonCommandNetIds, pythonCommandName;

            if(netIdsOfItem.size() == 1)
            {
                pythonCommandNetIds = PyCodeProvider::pyCodeNet(netIdsOfItem.at(0));
                pythonCommandName = PyCodeProvider::pyCodeNetName(netIdsOfItem.at(0));
            }
            else if(netIdsOfItem.size() == 2)
            {
                pythonCommandNetIds = "netInput = " + PyCodeProvider::pyCodeNet(netIdsOfItem.at(0)) + "\nnetOutput = " + PyCodeProvider::pyCodeNet(netIdsOfItem.at(1));
                pythonCommandName = "netInputName = " + PyCodeProvider::pyCodeNetName(netIdsOfItem.at(0)) + "\nnetOutputName = " + PyCodeProvider::pyCodeNetName(netIdsOfItem.at(1));
            }

            menu.addAction(QIcon(":/icons/python"), "Extract net(s) as python code",
                [pythonCommandNetIds]()
                {
                    QApplication::clipboard()->setText(pythonCommandNetIds);
                });

            //name can only be extracted as python code if its a net
            menu.addAction(QIcon(":/icons/python"), "Extract net(s) name(s) as python code",
                [pythonCommandName]()
                {
                    QApplication::clipboard()->setText(pythonCommandName);
                });

        }else
        {
            QString pythonList = "[";
            for(auto childPin : clickedItem->getChildren())
                pythonList += childPin->getData(PinTreeModel::sNameColumn).toString() + ", ";
            pythonList = pythonList.left(pythonList.size()-2);
            pythonList += "]";

            menu.addAction(QIcon(":/icons/python"), "Extract pingroup as python list",
                [pythonList]()
                {
                    QApplication::clipboard()->setText(pythonList);
                });
        }

        menu.move(mapToGlobal(pos));
        menu.exec();

    }

}
