#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/gate_details_widget.h"
#include "gui/selection_details_widget/net_details_widget.h"
#include "gui/selection_details_widget/module_details_widget.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/user_action/action_set_selection_focus.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/user_action/user_action_object.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"


#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "gui/searchbar/searchbar.h"
#include "gui/toolbar/toolbar.h"
#include "gui/gui_utils/graphics.h"
#include "gui/gui_utils/netlist.h"

#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QSplitter>
#include <QListWidget>
#include <QLineEdit>
#include <QMenu>
#include <QAction>


namespace hal
{
    const QString SelectionDetailsWidget::sAddToGrouping("Add to grouping ");

    SettingsItemCheckbox* SelectionDetailsWidget::sSettingHideEmpty =
            new SettingsItemCheckbox(
                "Hide Empty Section",
                "selection_details/hide_empty_sections",
                false,
                "Appearance:Selection Details",
                "Specifies wheter empty sections are hidden or shown in the Selection Details Widget."
                );

    SelectionDetailsWidget::SelectionDetailsWidget(QWidget* parent)
        : ContentWidget("Selection Details", parent), mNumberSelectedItems(0),
          mSelectionToGrouping(new QAction),
          mSelectionToModule(new QAction)
    {
        //needed to load the properties
        ensurePolished();

        mSplitter = new QSplitter(Qt::Horizontal, this);
        //mSplitter->setStretchFactor(0,5); /* Doesn't do anything? */
        //mSplitter->setStretchFactor(1,10);

        //container for left side of splitter containing a selection tree view and a searchbar
        QWidget* treeViewContainer = new QWidget(mSplitter);
        //treeViewContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); /*Does not work, but should?*/
        treeViewContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); /*Kinda works? Does not give as much space as previous implementation without container.*/

        QVBoxLayout* containerLayout = new QVBoxLayout(treeViewContainer);

        mSelectionTreeView  = new SelectionTreeView(treeViewContainer);
        mSelectionTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mSelectionTreeView->setMinimumWidth(280);
        mSelectionTreeView->hide();

        mSearchbar = new Searchbar(treeViewContainer);
        mSearchbar->hide();

        containerLayout->addWidget(mSelectionTreeView);
        containerLayout->addWidget(mSearchbar);
        containerLayout->setSpacing(0);
        containerLayout->setContentsMargins(0,0,0,0);


        mSelectionDetails = new QWidget(mSplitter);
        QVBoxLayout* selDetailsLayout = new QVBoxLayout(mSelectionDetails);

        mStackedWidget = new QStackedWidget(mSelectionDetails);

        mGateDetails = new GateDetailsWidget(mSelectionDetails);
        mStackedWidget->addWidget(mGateDetails);

        mNetDetails = new NetDetailsWidget(mSelectionDetails);
        mStackedWidget->addWidget(mNetDetails);

        mModuleDetails = new ModuleDetailsWidget(this);
        mStackedWidget->addWidget(mModuleDetails);

        mItemDeletedLabel = new QLabel(mSelectionDetails);
        mItemDeletedLabel->setText("Currently selected item has been removed. Please consider relayouting the Graph.");
        mItemDeletedLabel->setWordWrap(true);
        mItemDeletedLabel->setAlignment(Qt::AlignmentFlag::AlignTop);
        mStackedWidget->addWidget(mItemDeletedLabel);

        mNoSelectionLabel = new QLabel(mSelectionDetails);
        mNoSelectionLabel->setText("No Selection");
        mNoSelectionLabel->setWordWrap(true);
        mNoSelectionLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        mStackedWidget->addWidget(mNoSelectionLabel);

        mStackedWidget->setCurrentWidget(mNoSelectionLabel);

        selDetailsLayout->addWidget(mStackedWidget);

        mContentLayout->addWidget(mSplitter);

        //    m_table_widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        //    m_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //    m_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
        //    m_table_widget->setSelectionMode(QAbstractItemView::NoSelection);
        //    m_table_widget->setShowGrid(false);
        //    m_table_widget->setAlternatingRowColors(true);
        //    m_table_widget->horizontalHeader()->setStretchLastSection(true);
        //    m_table_widget->viewport()->setFocusPolicy(Qt::NoFocus);
        mSelectionToGrouping->setToolTip("Assign to grouping");
        mSelectionToModule->setToolTip("Move to module");
        mSelectionToGrouping->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToGroupingIconPath));
        mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToModuleIconPath));
        canMoveToModule(0);

        mSearchAction->setToolTip("Search");
        enableSearchbar(false);  // enable upon first non-zero selection
        mSelectionToGrouping->setDisabled(true);
        mSelectionToModule->setDisabled(true);

        mGateDetails->hideSectionsWhenEmpty(sSettingHideEmpty->value().toBool());
        mModuleDetails->hideSectionsWhenEmpty(sSettingHideEmpty->value().toBool());
        mNetDetails->hideSectionsWhenEmpty(sSettingHideEmpty->value().toBool());

        connect(sSettingHideEmpty, &SettingsItemCheckbox::boolChanged, mGateDetails, &GateDetailsWidget::hideSectionsWhenEmpty);
        connect(sSettingHideEmpty, &SettingsItemCheckbox::boolChanged, mModuleDetails, &ModuleDetailsWidget::hideSectionsWhenEmpty);
        connect(sSettingHideEmpty, &SettingsItemCheckbox::boolChanged, mNetDetails, &NetDetailsWidget::hideSectionsWhenEmpty);

        gSelectionRelay->registerSender(this, "SelectionDetailsWidget");
        connect(mSelectionToGrouping, &QAction::triggered, this, &SelectionDetailsWidget::selectionToGrouping);
        connect(mSelectionToModule, &QAction::triggered, this, &SelectionDetailsWidget::selectionToModuleMenu);
        connect(mSearchAction, &QAction::triggered, this, &SelectionDetailsWidget::toggleSearchbar);
        connect(mSelectionTreeView, &SelectionTreeView::triggerSelection, this, &SelectionDetailsWidget::handleTreeSelection);
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &SelectionDetailsWidget::handleSelectionUpdate);
        connect(mSearchbar, &Searchbar::textEdited, mSelectionTreeView, &SelectionTreeView::handleFilterTextChanged);
        connect(mSearchbar, &Searchbar::textEdited, this, &SelectionDetailsWidget::updateSearchIcon);
        connect(mSelectionTreeView, &SelectionTreeView::itemDoubleClicked, this, &SelectionDetailsWidget::handleTreeViewItemFocusClicked);
        connect(mSelectionTreeView, &SelectionTreeView::focusItemClicked, this, &SelectionDetailsWidget::handleTreeViewItemFocusClicked);
    }

    void SelectionDetailsWidget::selectionToModuleMenu()
    {
        if (gSelectionRelay->numberSelectedNodes() <= 0) return;

        ModuleDialog md = new ModuleDialog(this);
        if (md.exec() != QDialog::Accepted) return;

        if (md.isNewModule())
            SelectionDetailsWidget::selectionToModuleAction(-1);
        else
            SelectionDetailsWidget::selectionToModuleAction(md.selectedId());
    }

    void SelectionDetailsWidget::selectionToModuleAction(int actionCode)
    {

        ActionAddItemsToObject* actAddSelected =
                new ActionAddItemsToObject(gSelectionRelay->selectedModules(),
                                           gSelectionRelay->selectedGates());
        u32 targetModuleId = 0;

        if (actionCode < 0)
        {
            // add to new module
            std::unordered_set<Gate*> gatesSelected;
            std::unordered_set<Module*> modulesSelected;
            for (u32 id : gSelectionRelay->selectedGatesList())
                gatesSelected.insert(gNetlist->get_gate_by_id(id));

            for (u32 id : gSelectionRelay->selectedModulesList())
                modulesSelected.insert(gNetlist->get_module_by_id(id));

            Module* parentModule = gui_utility::firstCommonAncestor(modulesSelected, gatesSelected);
            if(!parentModule) return; //hotfix, when the top-level module is in modulesSelected, this will be a nullptr
            QString parentName            = QString::fromStdString(parentModule->get_name());
            bool ok;
            QString name = QInputDialog::getText(nullptr, "", "New module will be created under \"" + parentName + "\"\nModule Name:", QLineEdit::Normal, "", &ok);
            if (!ok || name.isEmpty()) return;
            ActionCreateObject* actNewModule = new ActionCreateObject(UserActionObjectType::Module, name);
            actNewModule->setParentId(parentModule->get_id());
            UserActionCompound* act = new UserActionCompound;
            act->setUseCreatedObject();
            act->addAction(actNewModule);
            act->addAction(actAddSelected);
            act->exec();
            targetModuleId = act->object().id();
        }
        else
        {
            // add to existing module
            targetModuleId = actionCode;
            actAddSelected->setObject(UserActionObject(targetModuleId,UserActionObjectType::Module));
            actAddSelected->exec();
        }

        gSelectionRelay->clear();
        gSelectionRelay->addModule(targetModuleId);
        gSelectionRelay->setFocus(SelectionRelay::ItemType::Module,targetModuleId);
        gSelectionRelay->relaySelectionChanged(this);
        gContentManager->getGraphTabWidget()->ensureSelectionVisible();
    }

    void SelectionDetailsWidget::selectionToGrouping()
    {
        QStringList groupingNames =
                gContentManager->getGroupingManagerWidget()->getModel()->groupingNames();
        if (groupingNames.isEmpty())
            selectionToNewGrouping();
        else
        {
            QMenu* contextMenu = new QMenu(this);

            QAction* newGrouping = contextMenu->addAction("Create new grouping from selected items");
            connect(newGrouping, &QAction::triggered, this, &SelectionDetailsWidget::selectionToNewGrouping);

            contextMenu->addSeparator();

            for (const QString& gn : groupingNames)
            {
                QAction* toGrouping = contextMenu->addAction(sAddToGrouping+gn);
                connect(toGrouping, &QAction::triggered, this, &SelectionDetailsWidget::selectionToExistingGrouping);
            }
            contextMenu->exec(mapToGlobal(geometry().topLeft()+QPoint(100,0)));
        }
    }

    void SelectionDetailsWidget::selectionToNewGrouping()
    {
        selectionToGroupingAction();
    }

    void SelectionDetailsWidget::selectionToExistingGrouping()
    {
        const QAction* action = static_cast<const QAction*>(QObject::sender());
        QString grpName = action->text();
        if (grpName.startsWith(sAddToGrouping)) grpName.remove(0,sAddToGrouping.size());
        selectionToGroupingAction(grpName);
    }

    UserAction* SelectionDetailsWidget::groupingUnassignActionFactory(const UserActionObject& obj) const
    {
        Grouping* assignedGrouping = nullptr;
        QSet<u32> mods, gats, nets;
        Module* mod;
        Gate*   gat;
        Net*    net;
        switch(obj.type()) {
        case UserActionObjectType::Module:
            mod = gNetlist->get_module_by_id(obj.id());
            if (mod) assignedGrouping = mod->get_grouping();
            mods.insert(mod->get_id());
            break;
        case UserActionObjectType::Gate:
            gat = gNetlist->get_gate_by_id(obj.id());
            if (gat) assignedGrouping = gat->get_grouping();
            gats.insert(gat->get_id());
            break;
        case UserActionObjectType::Net:
            net = gNetlist->get_net_by_id(obj.id());
            if (net) assignedGrouping = net->get_grouping();
            nets.insert(net->get_id());
            break;
        default:
            break;
        }
        if (!assignedGrouping) return nullptr; // nothing to do
        ActionRemoveItemsFromObject* retval = new ActionRemoveItemsFromObject(mods,gats,nets);
        retval->setObject(UserActionObject(assignedGrouping->get_id(),
                                           UserActionObjectType::Grouping));
        retval->setObjectLock(true);
        return retval;
    }

    void SelectionDetailsWidget::selectionToGroupingAction(const QString& existingGrpName)
    {
        UserActionCompound* compound = new UserActionCompound;
        u32 grpId = 0;
        if (existingGrpName.isEmpty())
        {
            compound->addAction(new ActionCreateObject(UserActionObjectType::Grouping));
            compound->setUseCreatedObject();
        }
        else
        {
            Grouping* grp = gContentManager->getGroupingManagerWidget()->
                    getModel()->groupingByName(existingGrpName);
            if (grp) grpId = grp->get_id();
        }
        for (const UserActionObject& obj : gSelectionRelay->toUserActionObject())
        {
            UserAction* act = groupingUnassignActionFactory(obj);
            if (act) compound->addAction(act);
        }
        ActionAddItemsToObject* act = new ActionAddItemsToObject(gSelectionRelay->selectedModules(),
                                                                 gSelectionRelay->selectedGates(),
                                                                 gSelectionRelay->selectedNets());
        act->setObject(UserActionObject(grpId,UserActionObjectType::Grouping));
        compound->addAction(act);
        compound->addAction(new ActionSetSelectionFocus);
        compound->exec();
    }

    void SelectionDetailsWidget::enableSearchbar(bool enable)
    {
        QString iconStyle = enable ? mSearchIconStyle : mDisabledIconStyle;
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(iconStyle, mSearchIconPath));
        if (!enable && mSearchbar->isVisible())
        {
            mSearchbar->hide();
            setFocus();
        }
        mSearchAction->setEnabled(enable);
    }

    void SelectionDetailsWidget::canMoveToModule(int nodes)
    {
        QString iconStyle = nodes > 0
                ? mToModuleIconStyle
                : mDisabledIconStyle;
        mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(iconStyle, mToModuleIconPath));
        mSelectionToModule->setEnabled(gContentManager->getGraphTabWidget()->selectCursor()==GraphTabWidget::Select
                                       && nodes > 0);
    }

    void SelectionDetailsWidget::handleSelectionUpdate(void* sender)
    {
        //called update methods with id = 0 to reset widget to the internal state of not updating because its not mVisible
        //when all details widgets are finished maybe think about more elegant way

        if (sender == this)
        {
            return;
        }

        SelectionTreeProxyModel* proxy = static_cast<SelectionTreeProxyModel*>(mSelectionTreeView->model());
        if (proxy->isGraphicsBusy()) return;

        mSearchbar->clear();
        updateSearchIcon();

        mNumberSelectedItems = gSelectionRelay->numberSelectedItems();
        QVector<const SelectionTreeItem*> defaultHighlight;

        if (mNumberSelectedItems)
        {
            // more than 1 item selected, populate and make mVisible
            mSelectionTreeView->populate(true);
            defaultHighlight.append(mSelectionTreeView->itemFromIndex());

            canMoveToModule(gSelectionRelay->numberSelectedNodes());
            enableSearchbar(true);

            bool toModuleEnabled = gContentManager->getGraphTabWidget()->selectCursor()==GraphTabWidget::Select;
            mSelectionToGrouping->setEnabled(true);
            mSelectionToModule->setEnabled(toModuleEnabled);
            mSelectionToGrouping->setIcon(gui_utility::getStyledSvgIcon(mToGroupingIconStyle, mToGroupingIconPath));
            mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(toModuleEnabled
                                                                      ? mToModuleIconStyle
                                                                      : mDisabledIconStyle,
                                                                      mToModuleIconPath));
        }
        else
        {
            // nothing selected
            singleSelectionInternal(nullptr);
            // clear and hide tree
            mSelectionTreeView->populate(false);
            canMoveToModule(0);
            enableSearchbar(false);
            mSelectionToGrouping->setDisabled(true);
            mSelectionToModule->setDisabled(true);
            mSelectionToGrouping->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToGroupingIconPath));
            mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToModuleIconPath));

            return;
        }


        if (gSelectionRelay->numberSelectedModules())
        {
            SelectionTreeItemModule sti(gSelectionRelay->selectedModulesList().at(0));
            singleSelectionInternal(&sti);
        }
        else if (gSelectionRelay->numberSelectedGates())
        {
            SelectionTreeItemGate sti(gSelectionRelay->selectedGatesList().at(0));
            singleSelectionInternal(&sti);
        }
        else if (gSelectionRelay->numberSelectedNets())
        {
            SelectionTreeItemNet sti(gSelectionRelay->selectedNetsList().at(0));
            singleSelectionInternal(&sti);
        }
        Q_EMIT triggerHighlight(defaultHighlight);
    }

    void SelectionDetailsWidget::handleTreeSelection(const SelectionTreeItem *sti)
    {
        singleSelectionInternal(sti);
        QVector<const SelectionTreeItem*> highlight;
        if (sti) highlight.append(sti);
        Q_EMIT triggerHighlight(highlight);
    }

    void SelectionDetailsWidget::singleSelectionInternal(const SelectionTreeItem *sti)
    {
        SelectionTreeItem::TreeItemType tp = sti
                ? sti->itemType()
                : SelectionTreeItem::NullItem;

        switch (tp) {
        case SelectionTreeItem::NullItem:
            mModuleDetails->update(0);
            mStackedWidget->setCurrentWidget(mNoSelectionLabel);
//            set_name("Selection Details");
            break;
        case SelectionTreeItem::ModuleItem:
            mModuleDetails->update(sti->id());
            mStackedWidget->setCurrentWidget(mModuleDetails);
//            if (mNumberSelectedItems==1) set_name("Module Details");
            break;
        case SelectionTreeItem::GateItem:
            mModuleDetails->update(0);
            mGateDetails->update(sti->id());
            mStackedWidget->setCurrentWidget(mGateDetails);
//            if (mNumberSelectedItems==1) set_name("Gate Details");
            break;
        case SelectionTreeItem::NetItem:
            mModuleDetails->update(0);
            mNetDetails->update(sti->id());
            mStackedWidget->setCurrentWidget(mNetDetails);
//            if (mNumberSelectedItems==1) set_name("Net Details");
            break;
        default:
            break;
        }
    }

    QList<QShortcut *> SelectionDetailsWidget::createShortcuts()
    {
        mSearchShortcut = new QShortcut(mSearchKeysequence, this);
        connect(mSearchShortcut, &QShortcut::activated, mSearchAction, &QAction::trigger);

        QList<QShortcut*> list;
        list.append(mSearchShortcut);

        return list;
    }

    void SelectionDetailsWidget::toggleSearchbar()
    {
        if (!mSearchAction->isEnabled())
            return;

        if (mSearchbar->isHidden())
        {
            mSearchbar->show();
            mSearchbar->setFocus();
        }
        else
        {
            mSearchbar->hide();
            setFocus();
        }
    }

    void SelectionDetailsWidget::updateSearchIcon()
    {
        if (mSearchbar->filterApplied() && mSearchbar->isVisible())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
    }

    void SelectionDetailsWidget::handleTreeViewItemFocusClicked(const SelectionTreeItem* sti)
    {
        u32 itemId = sti->id();

        switch (sti->itemType())
        {
            case SelectionTreeItem::TreeItemType::GateItem:     Q_EMIT focusGateClicked(itemId);     break;
            case SelectionTreeItem::TreeItemType::NetItem:      Q_EMIT focusNetClicked(itemId);      break;
            case SelectionTreeItem::TreeItemType::ModuleItem:   Q_EMIT focusModuleClicked(itemId);   break;
            default: break;
        }
    }

    void SelectionDetailsWidget::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mSelectionToGrouping);
        toolbar->addAction(mSelectionToModule);
        toolbar->addAction(mSearchAction);
    }

    SelectionTreeView* SelectionDetailsWidget::selectionTreeView()
    {
        return mSelectionTreeView;
    }

    QString SelectionDetailsWidget::disabledIconStyle() const
    {
        return mDisabledIconStyle;
    }

    void SelectionDetailsWidget::setDisabledIconStyle(const QString& style)
    {
        mDisabledIconStyle = style;
    }

    QString SelectionDetailsWidget::searchIconPath() const
    {
        return mSearchIconPath;
    }

    QString SelectionDetailsWidget::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    QString SelectionDetailsWidget::searchActiveIconStyle() const
    {
        return mSearchActiveIconStyle;
    }

    void SelectionDetailsWidget::setSearchIconPath(const QString& path)
    {
        mSearchIconPath = path;
    }

    void SelectionDetailsWidget::setSearchIconStyle(const QString& style)
    {
        mSearchIconStyle = style;
    }

    void SelectionDetailsWidget::setSearchActiveIconStyle(const QString& style)
    {
        mSearchActiveIconStyle = style;
    }

    QString SelectionDetailsWidget::toGroupingIconPath() const
    {
        return mToGroupingIconPath;
    }

    QString SelectionDetailsWidget::toGroupingIconStyle() const
    {
        return mToGroupingIconStyle;
    }

    void SelectionDetailsWidget::setToGroupingIconPath(const QString& path)
    {
        mToGroupingIconPath = path;
    }

    void SelectionDetailsWidget::setToGroupingIconStyle(const QString& style)
    {
        mToGroupingIconStyle = style;
    }
    
    QString SelectionDetailsWidget::toModuleIconPath() const
    {
        return mToModuleIconPath;
    }
    
    QString SelectionDetailsWidget::toModuleIconStyle() const
    {
        return mToModuleIconStyle;
    }
    
    void SelectionDetailsWidget::setToModuleIconPath(const QString& path)
    {
        mToModuleIconPath = path;
    }
    
    void SelectionDetailsWidget::setToModuleIconStyle(const QString& style)
    {
        mToModuleIconStyle = style;
    }
}
