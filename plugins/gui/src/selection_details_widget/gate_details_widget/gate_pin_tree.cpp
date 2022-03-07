#include "gui/selection_details_widget/gate_details_widget/gate_pin_tree.h"
#include "gui/selection_details_widget/gate_details_widget/pin_tree_model.h"
//#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include "gui/python/py_code_provider.h"
#include "gui/graph_widget/graph_navigation_widget.h"
#include <QDebug>
#include <QMouseEvent>

namespace hal
{

    GatePinTree::GatePinTree(QWidget *parent) : SizeAdjustableTreeView(parent), mPinModel(new GatePinsTreeModel(this)),
         mGateID(-1), mClearSelection(false)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setSelectionMode(QAbstractItemView::NoSelection);
        setFocusPolicy(Qt::NoFocus);
        header()->setStretchLastSection(true);
        setModel(mPinModel);

        mNavigationTable = new GraphNavigationWidget(true);
        mNavigationTable->setWindowFlags(Qt::CustomizeWindowHint);
        mNavigationTable->hide();

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &GatePinTree::handleContextMenuRequested);
        connect(mNavigationTable, &GraphNavigationWidget::closeRequested, this, &GatePinTree::handleNavigationCloseRequested);
        connect(mNavigationTable, &GraphNavigationWidget::navigationRequested, this, &GatePinTree::handleNavigationJumpRequested);

    }

    void GatePinTree::setGate(u32 gateID)
    {
        Gate* g = gNetlist->get_gate_by_id(gateID);
        if(!g) return;

        mPinModel->setGate(g);
        mGateID = gateID;

        adjustSizeToContents();

        Q_EMIT updateText(QString("Pins (%1)").arg(mPinModel->getNumberOfDisplayedPins()));
    }

    void GatePinTree::setGate(Gate *g)
    {
        if(!g) return;

        mPinModel->setGate(g);
        mGateID = g->get_id();

        adjustSizeToContents();

        Q_EMIT updateText(QString("Pins (%1)").arg(mPinModel->getNumberOfDisplayedPins()));
    }

    void GatePinTree::removeContent()
    {
        mGateID = -1;
        mPinModel->clear();
    }

    void GatePinTree::mouseDoubleClickEvent(QMouseEvent *event)
    {
        SizeAdjustableTreeView::mouseDoubleClickEvent(event);

        QModelIndex idx = indexAt(event->pos());
        if(!idx.isValid())
            return;

        auto clickedItem = mPinModel->getItemFromIndex(idx);
        if(mPinModel->getTypeOfItem(clickedItem) != GatePinsTreeModel::itemType::pin)
            return;

        auto netId = mPinModel->getNetIDsOfTreeItem(clickedItem).front();
        auto clickedNet = gNetlist->get_net_by_id(netId);
        if(clickedNet)
        {
            auto direction = clickedItem->getData(GatePinsTreeModel::sDirectionColumn).toString();
            if(direction == "output" && !clickedNet->is_global_output_net())
            {
                mClearSelection = true;
                addSourceOurDestinationToSelection(netId, false);
            }else if (direction == "input" && !clickedNet->is_global_input_net())
            {
                mClearSelection = true;
                addSourceOurDestinationToSelection(netId, true);
            }
        }
    }

    void GatePinTree::handleContextMenuRequested(const QPoint &pos)
    {
        QModelIndex idx = indexAt(pos);
        if(!idx.isValid())
            return;

        TreeItem* clickedItem = mPinModel->getItemFromIndex(idx);
        QMenu menu;
        GatePinsTreeModel::itemType type = mPinModel->getTypeOfItem(clickedItem);
        bool isMiscSectionSet = false;//so that the misc-section is not set multiple times

        //PLAINTEXT: NAME, DIRECTION, TYPE
        menu.addAction("Extract name as plain text",
            [clickedItem](){
            QApplication::clipboard()->setText(clickedItem->getData(GatePinsTreeModel::sNameColumn).toString());
        });

        menu.addAction("Extract direction as plain text",
            [clickedItem](){
            QApplication::clipboard()->setText(clickedItem->getData(GatePinsTreeModel::sDirectionColumn).toString());
        });

        menu.addAction("Extract type as plain text",
            [clickedItem](){
            QApplication::clipboard()->setText(clickedItem->getData(GatePinsTreeModel::sTypeColumn).toString());
        });

        //Check if jump to source or destination is possible
        if(type == GatePinsTreeModel::itemType::pin && mPinModel->getNetIDsOfTreeItem(clickedItem).size()==1)
        {
            auto netId = mPinModel->getNetIDsOfTreeItem(clickedItem).front();
            auto clickedNet = gNetlist->get_net_by_id(netId);
            if(clickedNet)
            {
                //check if input or output pin (use enums, todo: save PinDirection in items as additional data(?))
                auto direction = clickedItem->getData(GatePinsTreeModel::sDirectionColumn).toString();
                if(direction == "output" && !clickedNet->is_global_output_net())
                {
                    menu.addSection("Misc");
                    isMiscSectionSet = true;
                    menu.addAction("Overwrite selection with destination gate", [this, netId](){
                        mClearSelection = true;
                        addSourceOurDestinationToSelection(netId, false);
                    });
                    menu.addAction("Add destination gate to selection", [this, netId](){
                        mClearSelection = false;
                        addSourceOurDestinationToSelection(netId, false);
                    });
                }
                else if (direction == "input" && !clickedNet->is_global_input_net())
                {
                    menu.addSection("Misc");
                    isMiscSectionSet = true;
                    menu.addAction("Overwrite selection with source gate", [this, netId](){
                        mClearSelection = true;
                        addSourceOurDestinationToSelection(netId, true);
                    });
                    menu.addAction("Add source gate to selection", [this, netId](){
                        mClearSelection = false;
                        addSourceOurDestinationToSelection(netId, true);
                    });
                }
            }
        }

        //Add nets to selection if possible
        QList<int> netIds;
        if(type == GatePinsTreeModel::itemType::pin)
        {
            netIds = mPinModel->getNetIDsOfTreeItem(clickedItem);
        }
        else
        {
            for(auto childItem : clickedItem->getChildren())
                netIds.append(mPinModel->getNetIDsOfTreeItem(childItem));
        }
        if(netIds.size() != 0)
        {
            QString desc = (netIds.size() == 1) ? "Add net to current selection" : "Add net(s) to current selection";
            if(!isMiscSectionSet)
            {
                menu.addSection("Misc");
                isMiscSectionSet = true;
            }
            menu.addAction(desc,
                [this, netIds](){
                for(const int id : netIds)
                    gSelectionRelay->addNet(id);
                gSelectionRelay->relaySelectionChanged(this);
            });
        }

        menu.addSection("Python");

        if(type == GatePinsTreeModel::itemType::pin)
            buildPythonMenuForPin(menu, clickedItem);
        else
            buildPythonMenuForPinGroup(menu, clickedItem);

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

        // 2.) DIRECTION
        QString pythonCommandDirection = PyCodeProvider::pyCodeGateTypePinDirection(mPinModel->getCurrentGateID(), clickedPinItem->getData(GatePinsTreeModel::sNameColumn).toString());
        menu.addAction(QIcon(":/icons/python"), "Extract pin direction as python code",
            [pythonCommandDirection]()
            {
                QApplication::clipboard()->setText(pythonCommandDirection);
            });


        // 3.) type
        QString pythonCommandType = PyCodeProvider::pyCodeGateTypePinType(mPinModel->getCurrentGateID(), clickedPinItem->getData(GatePinsTreeModel::sNameColumn).toString());
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
            pythonList += "\"" + childPin->getData(GatePinsTreeModel::sNameColumn).toString() + "\", ";
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
                                                                                             firstPinItemOfGroup->getData(GatePinsTreeModel::sNameColumn).toString());
            menu.addAction(QIcon(":/icons/python"), "Extract direction of pingroup as python code",
                [pythonCommandGroupDirection]()
                {
                    QApplication::clipboard()->setText(pythonCommandGroupDirection);
                });

            QString pythonCommandGroupType = PyCodeProvider::pyCodeGateTypePinType(mPinModel->getCurrentGateID(),
                                                                                             firstPinItemOfGroup->getData(GatePinsTreeModel::sNameColumn).toString());
            menu.addAction(QIcon(":/icons/python"), "Extract type of pingroup as python code",
                [pythonCommandGroupType]()
                {
                    QApplication::clipboard()->setText(pythonCommandGroupType);
                });
        }
    }

    void GatePinTree::addSourceOurDestinationToSelection(int netId, bool isInputPin)
    {
        auto net = gNetlist->get_net_by_id(netId);
        if(!net) return;

        auto sourcesOrDesti = isInputPin ? net->get_sources() : net->get_destinations();
        if(sourcesOrDesti.empty() || net->is_global_input_net() || net->is_global_output_net())
        {
            return; //for now, maybe simply add/oberwrite net (check global mClearSelection)?
        }
        else if (sourcesOrDesti.size() == 1)
        {
            if(mClearSelection)
                gSelectionRelay->clear();
            auto ep = *sourcesOrDesti.begin();
            gSelectionRelay->addGate(ep->get_gate()->get_id());
            auto pins = isInputPin ? ep->get_gate()->get_type()->get_output_pins() : ep->get_gate()->get_type()->get_input_pins();
            auto index = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep->get_pin()));
            gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate, ep->get_gate()->get_id(),isInputPin ? SelectionRelay::Subfocus::Right : SelectionRelay::Subfocus::Left, index);
            gSelectionRelay->relaySelectionChanged(this);
        }
        else
        {
            mNavigationTable->setup(Node(), net, isInputPin ? SelectionRelay::Subfocus::Left : SelectionRelay::Subfocus::Right);
            if(mNavigationTable->isEmpty())
                mNavigationTable->closeRequest();
            else
            {
                mNavigationTable->move(QCursor::pos());
                mNavigationTable->show();
                mNavigationTable->setFocus();
            }

        }
    }

    void GatePinTree::handleNavigationCloseRequested()
    {
        mNavigationTable->hide();
    }

    void GatePinTree::handleNavigationJumpRequested(const Node &origin, const u32 via_net, const QSet<u32> &to_gates, const QSet<u32> &to_modules)
    {
        Q_UNUSED(origin)
        Q_UNUSED(to_modules)

        mNavigationTable->hide();

        auto n = gNetlist->get_net_by_id(via_net);
        if(!n || to_gates.isEmpty())
            return;
        for(u32 id : to_gates)
            if(!gNetlist->get_gate_by_id(id))
                return;

        if(mClearSelection)
            gSelectionRelay->clear();
        for(u32 id : to_gates)
            gSelectionRelay->addGate(id);

        if(to_gates.size()==1)
        {
            auto g = gNetlist->get_gate_by_id(*to_gates.begin());
            u32 index = 0;
            bool isNavDirLeft = mNavigationTable->direction() == SelectionRelay::Subfocus::Left;
            auto pins = isNavDirLeft ? g->get_type()->get_output_pins() : g->get_type()->get_input_pins();
            for(auto pin : pins)
            {
                if(isNavDirLeft ? g->get_fan_out_net(pin) == n : g->get_fan_in_net(pin) == n)
                {
                    gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate, g->get_id(), isNavDirLeft ? SelectionRelay::Subfocus::Right : SelectionRelay::Subfocus::Left, index);
                    break;
                }
                index++;
            }
        }
        gSelectionRelay->relaySelectionChanged(this);
    }

}
