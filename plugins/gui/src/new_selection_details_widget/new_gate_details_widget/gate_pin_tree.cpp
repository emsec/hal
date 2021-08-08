#include "gui/new_selection_details_widget/new_gate_details_widget/gate_pin_tree.h"
#include "gui/new_selection_details_widget/models/pin_tree_model.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include "gui/new_selection_details_widget/py_code_provider.h"
#include <QDebug>

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
        PinTreeModel::itemType type = mPinModel->getTypeOfItem(clickedItem);

        if(type == PinTreeModel::itemType::pin)
            buildPythonMenuForPin(menu, clickedItem);
        else
            buildPythonMenuForPinGroup(menu, clickedItem);

        //PLAINTEXT: NAME, DIRECTION, TYPE
        menu.addSection("plaintext extraction");

        menu.addAction("Extract name as plain text",
            [clickedItem](){
            QApplication::clipboard()->setText(clickedItem->getData(PinTreeModel::sNameColumn).toString());
        });

        menu.addAction("Extract direction as plain text",
            [clickedItem](){
            QApplication::clipboard()->setText(clickedItem->getData(PinTreeModel::sDirectionColumn).toString());
        });

        menu.addAction("Extract type as plain text",
            [clickedItem](){
            QApplication::clipboard()->setText(clickedItem->getData(PinTreeModel::sTypeColumn).toString());
        });

        //add, when the clicked item is a pin, the add-to-selection choice if possible
        QList<int> netIds = mPinModel->getNetIDsOfTreeItem(clickedItem);
        if(type == PinTreeModel::itemType::pin && netIds.size() != 0)
        {
            menu.addSection("Misc");
            menu.addAction("Add net(s) to current selection",
                [this, netIds](){
                for(const int id : netIds)
                    gSelectionRelay->addNet(id);
                gSelectionRelay->relaySelectionChanged(this);
            });
        }

        menu.move(mapToGlobal(pos));
        menu.exec();

    }

    void GatePinTree::buildPythonMenuForPin(QMenu &menu, TreeItem *clickedPinItem)
    {
        // 1.) NET-OBJECT
        QList<int> netIdsOfItem = mPinModel->getNetIDsOfTreeItem(clickedPinItem);
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
//        menu.addAction(QIcon(":/icons/python"), "Extract net(s) name(s) as python code",
//            [pythonCommandName]()
//            {
//                QApplication::clipboard()->setText(pythonCommandName);
//            });

        // 2.) DIRECTION
        QString pythonCommandDirection = PyCodeProvider::pyCodeGateTypePinDirection(mPinModel->getCurrentGateID(), clickedPinItem->getData(PinTreeModel::sNameColumn).toString());
        menu.addAction(QIcon(":/icons/python"), "Extract pin direction as python code",
            [pythonCommandDirection]()
            {
                QApplication::clipboard()->setText(pythonCommandDirection);
            });


        // 3.) type
        QString pythonCommandType = PyCodeProvider::pyCodeGateTypePinType(mPinModel->getCurrentGateID(), clickedPinItem->getData(PinTreeModel::sNameColumn).toString());
        menu.addAction(QIcon(":/icons/python"), "Extract pin type as python code",
            [pythonCommandType]()
            {
                QApplication::clipboard()->setText(pythonCommandType);
            });

    }

    void GatePinTree::buildPythonMenuForPinGroup(QMenu &menu, TreeItem *clickedPinIGrouptem)
    {
        // 1. PYTHON LIST OF PIN GROUPS
        QString pythonList = "[";
        for(auto childPin : clickedPinIGrouptem->getChildren())
            pythonList += childPin->getData(PinTreeModel::sNameColumn).toString() + ", ";
        pythonList = pythonList.left(pythonList.size()-2);
        pythonList += "]";

        menu.addAction(QIcon(":/icons/python"), "Extract pingroup as python list",
            [pythonList]()
            {
                QApplication::clipboard()->setText(pythonList);
            });

        //2. DIRECTION and TYPE(determined by the pin(s) within the group)
        TreeItem* firstPinItemOfGroup = clickedPinIGrouptem->getChild(0);
        if(firstPinItemOfGroup)
        {
            QString pythonCommandGroupDirection = PyCodeProvider::pyCodeGateTypePinDirection(mPinModel->getCurrentGateID(),
                                                                                             firstPinItemOfGroup->getData(PinTreeModel::sNameColumn).toString());
            menu.addAction(QIcon(":/icons/python"), "Extract direction of pingroup as python code",
                [pythonCommandGroupDirection]()
                {
                    QApplication::clipboard()->setText(pythonCommandGroupDirection);
                });

            QString pythonCommandGroupType = PyCodeProvider::pyCodeGateTypePinType(mPinModel->getCurrentGateID(),
                                                                                             firstPinItemOfGroup->getData(PinTreeModel::sNameColumn).toString());
            menu.addAction(QIcon(":/icons/python"), "Extract type of pingroup as python code",
                [pythonCommandGroupType]()
                {
                    QApplication::clipboard()->setText(pythonCommandGroupType);
                });
        }
    }

}
