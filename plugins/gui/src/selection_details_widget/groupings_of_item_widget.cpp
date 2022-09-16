#include "gui/selection_details_widget/groupings_of_item_widget.h"
#include "gui/grouping/grouping_color_delegate.h"
#include "gui/gui_globals.h"
#include "gui/python/py_code_provider.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_set_object_color.h"
#include <QHeaderView>
#include <QtWidgets/QMenu>
#include <QApplication>
#include <QClipboard>
#include "gui/input_dialog/input_dialog.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include <QColorDialog>

namespace hal {
    GroupingsOfItemWidget::GroupingsOfItemWidget(QWidget *parent) : QTableView(parent),
    mGroupingsOfItemModel(new GroupingsOfItemModel(this))
    {

        this->setModel(mGroupingsOfItemModel);
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->setSelectionMode(QAbstractItemView::NoSelection);
        this->setFocusPolicy(Qt::NoFocus);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        this->verticalHeader()->setVisible(false);
        setItemDelegateForColumn(2,new GroupingColorDelegate(this));
        setFrameStyle(QFrame::NoFrame);
        updateAppearance();
        adjustTableSizes();
        
        connect(this, &QTableView::customContextMenuRequested, this, &GroupingsOfItemWidget::handleContextMenuRequest);
        connect(mGroupingsOfItemModel, &GroupingsOfItemModel::layoutChanged, this, &GroupingsOfItemWidget::handleLayoutChanged);
    }

    GroupingsOfItemModel* GroupingsOfItemWidget::getModel()
    {
        return mGroupingsOfItemModel;
    }

    void GroupingsOfItemWidget::setGate(Gate *gate)
    {
        if(gate == nullptr){
            return;
        }
        mCurrentObjectType = ItemType::Gate;
        mCurrentObjectId = gate->get_id();
        mGroupingsOfItemModel->setGate(gate);
        updateAppearance();
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
        updateAppearance();
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
        updateAppearance();
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


        /*====================================
                  Plaintext to Clipboard
          ====================================*/ 
        QString toClipboardText = e.name();
        menu.addAction(
            "Grouping name to clipboard",
            [toClipboardText]()
            {
                QApplication::clipboard()->setText( toClipboardText );
            }
        );

        toClipboardText = QString::number(e.id());
        menu.addAction(
            "Grouping ID to clipboard",
            [toClipboardText]()
            {
                QApplication::clipboard()->setText( toClipboardText );
            }
        );


        menu.addSection("Misc");

        menu.addAction("Change grouping name", [this, e](){changeNameTriggered(e);});
        menu.addAction("Change grouping color", [this, e](){changeColorTriggered(e);});

        /*====================================
               Remove item from grouping
          ====================================*/
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


        /*====================================
                   Python to Clipboard 
          ====================================*/ 
        menu.addSection("Python");
        QString pythonCode = PyCodeProvider::pyCodeGrouping(e.id());
        menu.addAction(
            QIcon(":/icons/python"),
            "Get grouping",
            [pythonCode]()
            {
                QApplication::clipboard()->setText( pythonCode );
            }
        );

        // Grouping List (TODO: When multiple groupings are supported...)
        
        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();

    }

    void GroupingsOfItemWidget::handleLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
    {
        Q_UNUSED(parents);
        Q_UNUSED(hint);
        updateAppearance();
        adjustTableSizes();
    }

    void GroupingsOfItemWidget::updateAppearance()
    {
        int n = mGroupingsOfItemModel->rowCount();
        notifyNewTitle(n);
        if(n < 1){
            this->horizontalHeader()->setVisible(false);
        }
        else{
            this->horizontalHeader()->setVisible(true);
        }
    }

    void GroupingsOfItemWidget::adjustTableSizes()
    {
        resizeColumnsToContents();  
        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); 

        // Config Height
        int h = horizontalHeader()->height() + 4;
        for (int i = 0; i < mGroupingsOfItemModel->rowCount(); i++)
            h += rowHeight(i);

        setMaximumHeight(h);
        setMinimumHeight(h);
    }

    void GroupingsOfItemWidget::notifyNewTitle(int elementCount)
    {
        if(elementCount < 1){
            Q_EMIT updateText(mFrameTitleNoItem);
        }
        else if(elementCount == 1){
            Q_EMIT updateText(mFrameTitleSingleItem);
        }
        else { // elementCount > 1
            Q_EMIT updateText(mFrameTitleMultipleItems.arg(elementCount));
        }
    }

    void GroupingsOfItemWidget::changeNameTriggered(GroupingTableEntry entry)
    {
        auto grpModel = gContentManager->getGroupingManagerWidget()->getModel();
        InputDialog ipd;
        ipd.setWindowTitle("Rename Grouping");
        ipd.setInfoText("Please select a new unique name for the grouping.");
        QString oldName = entry.name();
        grpModel->setAboutToRename(oldName);
        ipd.setInputText(oldName);
        ipd.addValidator(grpModel);

        if (ipd.exec() == QDialog::Accepted)
        {
            QString newName = ipd.textValue();
            if (newName != oldName)
            {
                ActionRenameObject* act = new ActionRenameObject(newName);
                act->setObject(UserActionObject(entry.id(), UserActionObjectType::Grouping));
                act->exec();
            }
        }
        grpModel->setAboutToRename(QString());
    }

    void GroupingsOfItemWidget::changeColorTriggered(GroupingTableEntry entry)
    {
        QColor color = entry.color();
        color = QColorDialog::getColor(color, this, "Select color for grouping " + entry.name());
        if(color.isValid())
        {
            ActionSetObjectColor* act = new ActionSetObjectColor(color);
            act->setObject(UserActionObject(entry.id(), UserActionObjectType::Grouping));
            act->exec();
        }
    }


} // namespace hal
