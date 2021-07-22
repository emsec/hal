#include "gui/new_selection_details_widget/groupings_of_item_widget.h"
#include "gui/grouping/grouping_color_delegate.h"
#include "gui/gui_globals.h"
#include "gui/new_selection_details_widget/py_code_provider.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include <QHeaderView>
#include <QtWidgets/QMenu>
#include <QApplication>
#include <QClipboard>

namespace hal {
    GroupingsOfItemWidget::GroupingsOfItemWidget(QWidget *parent) : QTableView(parent),
    mGroupingsOfItemModel(new GroupingsOfItemModel(this))
    {

        this->setModel(mGroupingsOfItemModel);
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->verticalHeader()->setVisible(false);
        setItemDelegateForColumn(2,new GroupingColorDelegate(this));
        //this->horizontalHeader()->setVisible(false);
        adjustTableSizes();

        SelectionTreeView* t = gContentManager->getSelectionDetailsWidget()->selectionTreeView();
        connect(t, &SelectionTreeView::triggerSelection, this, &GroupingsOfItemWidget::handleDetailsFocusChanged);
        connect(this, &QTableView::customContextMenuRequested, this, &GroupingsOfItemWidget::handleContextMenuRequest);
    }

    GroupingsOfItemModel* GroupingsOfItemWidget::getModel()
    {
        return mGroupingsOfItemModel;
    }

    void GroupingsOfItemWidget::handleDetailsFocusChanged(const SelectionTreeItem* sti)
    {
        if(sti == nullptr){
            return;
        }

        if(sti->itemType() == SelectionTreeItem::TreeItemType::GateItem){
            Gate* g = gNetlist->get_gate_by_id(sti->id());
            setGate(g);
        }
        else if(sti->itemType() == SelectionTreeItem::TreeItemType::NetItem){
            Net* n = gNetlist->get_net_by_id(sti->id());
            setNet(n);
        }
        else if(sti->itemType() == SelectionTreeItem::TreeItemType::ModuleItem){
            Module* m = gNetlist->get_module_by_id(sti->id());
            setModule(m);
        }
        clearSelection();
    }

    void GroupingsOfItemWidget::setGate(Gate *gate)
    {
        if(gate == nullptr){
            return;
        }
        mCurrentObjectType = ItemType::Gate;
        mCurrentObjectId = gate->get_id();
        mGroupingsOfItemModel->setGate(gate);
        adjustTableSizes();
    }

    void GroupingsOfItemWidget::setNet(Net* net)
    {
        if(net == nullptr){
            return;
        }
        mCurrentObjectType = ItemType::Net;
        mCurrentObjectId = net->get_id();
        mGroupingsOfItemModel->setNet(net);
        adjustTableSizes();
    }

    void GroupingsOfItemWidget::setModule(Module* module)
    {
        if(module == nullptr){
            return;
        }
        mCurrentObjectType = ItemType::Module;
        mCurrentObjectId = module->get_id();
        mGroupingsOfItemModel->setModule(module);
        adjustTableSizes();
    }
    

    void GroupingsOfItemWidget::resizeEvent(QResizeEvent *event)
    {
        QAbstractItemView::resizeEvent(event);
        adjustTableSizes();
    }

    void GroupingsOfItemWidget::handleContextMenuRequest(const QPoint &pos)
    {
        QModelIndex idx = indexAt(pos);
        // A pure data container can't be accessed via python
        if(!idx.isValid()){
            return;
        }

        GroupingTableEntry e = mGroupingsOfItemModel->getGroupingEntryAtRow(idx.row());
        QMenu menu;
        menu.setTitle(QString("%1 (%2)").arg(e.name(), e.id()));

        // Add python providers
        QString pythonCode = PyCodeProvider::pyCodeGrouping(e.id());
        menu.addAction(
            QIcon(":/icons/python"),
            "Extract grouping as python code (copy to clipboard)",
            [pythonCode]()
            {
                QApplication::clipboard()->setText( pythonCode );
            }
        );

        // Add python providers that are dependent from the color
        if(idx.column() == 0){
            pythonCode = PyCodeProvider::pyCodeGroupingName(e.id());
            menu.addAction(
                QIcon(":/icons/python"),
                "Extract grouping name as python code (copy to clipboard)",
                [pythonCode]()
                {
                    QApplication::clipboard()->setText( pythonCode );
                }
            );
        }

        if(idx.column() == 1){
            pythonCode = PyCodeProvider::pyCodeGroupingId(e.id());
            menu.addAction(
                QIcon(":/icons/python"),
                "Extract grouping id as python code (copy to clipboard)",
                [pythonCode]()
                {
                    QApplication::clipboard()->setText( pythonCode );
                }
            );
        }

        // Grouping List (TODO: When multiple groupings are supported...)
        
        // Remove Item from grouping
        if (mCurrentObjectType != ItemType::None && mCurrentObjectId > 0){
            QSet<u32> rmMods;
            QSet<u32> rmGates;
            QSet<u32> rmNets;
            QString actionText;
            if(mCurrentObjectType == ItemType::Gate){
                actionText = "Remove gate from grouping";
                rmGates.insert(mCurrentObjectId);
            }
                
            else if(mCurrentObjectType == ItemType::Module){
                actionText = "Remove module from grouping";
                rmMods.insert(mCurrentObjectId);
            }
                
            else if(mCurrentObjectType == ItemType::Net){
                actionText = "Remove net from grouping";
                rmNets.insert(mCurrentObjectId);
            }
                

            menu.addAction(
                actionText,
                [e,rmMods,rmGates,rmNets]()
                {
                    ActionRemoveItemsFromObject* rmItemAction = new ActionRemoveItemsFromObject(rmMods,rmGates,rmNets);
                    rmItemAction->setObject(UserActionObject(e.id(), UserActionObjectType::Grouping));
                    rmItemAction->setObjectLock(true);
                    rmItemAction->exec();
                }
            );
        }
        

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();

        // Add python 
        

        // Grouping python provider

        /*DataTableModel::DataEntry entry = mDataTableModel->getEntryAtRow(idx.row());

        QString menuText = "Exctract data as python code (copy to clipboard)";
        QString pythonCode;

        if(mCurrentObjectType == DataContainerType::GATE)
            pythonCode = PyCodeProvider::pyCodeGateData(mCurrentObjectId, entry.category, entry.key);
        else if(mCurrentObjectType == DataContainerType::NET)
            pythonCode = PyCodeProvider::pyCodeNetData(mCurrentObjectId, entry.category, entry.key);
        else if(mCurrentObjectType == DataContainerType::MODULE)
            pythonCode = PyCodeProvider::pyCodeModuleData(mCurrentObjectId, entry.category, entry.key);         

        QMenu menu;
        menu.addAction(QIcon(":/icons/python"), menuText,
           [pythonCode]()
           {
               QApplication::clipboard()->setText( pythonCode );
           }
        );
        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();*/
    }

    void GroupingsOfItemWidget::adjustTableSizes()
    {
        resizeColumnsToContents();  
        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); 
    }


} // namespace hal
