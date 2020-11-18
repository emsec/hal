#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/gate_details_widget.h"
#include "gui/selection_details_widget/net_details_widget.h"
#include "gui/selection_details_widget/module_details_widget.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/selection_history_navigator/selection_history_navigator.h"

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

    SelectionDetailsWidget::SelectionDetailsWidget(QWidget* parent)
        : ContentWidget("Selection Details", parent), mNumberSelectedItems(0),
          mRestoreLastSelection(new QAction),
          mSelectionToGrouping(new QAction),
          mSelectionToModule(new QAction),
          mSearchAction(new QAction),
          mHistory(new SelectionHistoryNavigator(5))
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

        mRestoreLastSelection->setToolTip("Restore last selection");
        mSelectionToGrouping->setToolTip("Assign to grouping");
        mSelectionToModule->setToolTip("Move to module");
        mSelectionToGrouping->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToGroupingIconPath));
        mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToModuleIconPath));
        canRestoreSelection();
        canMoveToModule(0);

        mSearchAction->setToolTip("Search");
        enableSearchbar(false);  // enable upon first non-zero selection
        mSelectionToGrouping->setDisabled(true);
        mSelectionToModule->setDisabled(true);

        gSelectionRelay->registerSender(this, "SelectionDetailsWidget");
        connect(mRestoreLastSelection, &QAction::triggered, this, &SelectionDetailsWidget::restoreLastSelection);
        connect(mSelectionToGrouping, &QAction::triggered, this, &SelectionDetailsWidget::selectionToGrouping);
        connect(mSelectionToModule, &QAction::triggered, this, &SelectionDetailsWidget::selectionToModuleMenu);
        connect(mSearchAction, &QAction::triggered, this, &SelectionDetailsWidget::toggleSearchbar);
        connect(mSelectionTreeView, &SelectionTreeView::triggerSelection, this, &SelectionDetailsWidget::handleTreeSelection);
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &SelectionDetailsWidget::handleSelectionUpdate);
        connect(mSearchbar, &Searchbar::textEdited, mSelectionTreeView, &SelectionTreeView::handleFilterTextChanged);
        connect(mSearchbar, &Searchbar::textEdited, this, &SelectionDetailsWidget::handleFilterTextChanged);
    }

    void SelectionDetailsWidget::restoreLastSelection()
    {
        gSelectionRelay->clear();
        mHistory->restorePreviousEntry();
        gSelectionRelay->relaySelectionChanged(nullptr);
        canRestoreSelection();
    }

    void SelectionDetailsWidget::selectionToModuleMenu()
    {
        if (gSelectionRelay->mSelectedModules.size() + gSelectionRelay->mSelectedGates.size() <= 0) return;
        QMenu* menu = new QMenu(this);
        QAction* action = menu->addAction("New module …");
        action->setData(-1);
        connect(action, &QAction::triggered, this, &SelectionDetailsWidget::selectionToModuleAction);
        menu->addSeparator();

        for (Module* module : gNetlist->get_modules())
        {
            bool canAdd = true;

            for (u32 mid : gSelectionRelay->mSelectedModules)
            {
                Module* m = gNetlist->get_module_by_id(mid);
                if (!m) continue;
                if (module == m || module->contains_module(m) || m->contains_module(module))
                {
                    canAdd = false;
                    break;
                }
            }
            if (canAdd)
                for (u32 gid : gSelectionRelay->mSelectedGates)
                {
                    Gate* g = gNetlist->get_gate_by_id(gid);
                    if (!g) continue;
                    if (module->contains_gate(g))
                    {
                        canAdd = false;
                        break;
                    }
                }
            // don't allow a gate to be moved into its current module
            // && don't allow a module to be moved into its current module
            // && don't allow a module to be moved into itself
            // (either check automatically passes if g respective m is nullptr, so we
            // don't have to create two loops)
            if (canAdd)
            {
                QString modName = QString::fromStdString(module->get_name());
                const u32 modId = module->get_id();
                action          = menu->addAction(modName);
                connect(action, &QAction::triggered, this, &SelectionDetailsWidget::selectionToModuleAction);
                action->setData(modId);
            }
        }
        menu->exec(mapToGlobal(geometry().topLeft()+QPoint(120,0)));
    }

    void SelectionDetailsWidget::selectionToModuleAction()
    {
        Module* targetModule = nullptr;
        const QAction* senderAction = static_cast<const QAction*>(sender());
        Q_ASSERT(senderAction);
        int actionCode = senderAction->data().toInt();
        if (actionCode < 0)
        {
            std::unordered_set<Gate*> gatesSelected;
            std::unordered_set<Module*> modulesSelected;
            for (u32 id : gSelectionRelay->mSelectedGates)
                gatesSelected.insert(gNetlist->get_gate_by_id(id));

            for (u32 id : gSelectionRelay->mSelectedModules)
                modulesSelected.insert(gNetlist->get_module_by_id(id));

            Module* parentModule = gui_utility::firstCommonAncestor(modulesSelected, gatesSelected);
            QString parentName            = QString::fromStdString(parentModule->get_name());
            bool ok;
            QString name = QInputDialog::getText(nullptr, "", "New module will be created under \"" + parentName + "\"\nModule Name:", QLineEdit::Normal, "", &ok);
            if (!ok || name.isEmpty()) return;
            targetModule = gNetlist->create_module(gNetlist->get_unique_module_id(), name.toStdString(), parentModule);
        }
        else
        {
            u32 mod_id = actionCode;
            targetModule = gNetlist->get_module_by_id(mod_id);
        }
        Q_ASSERT(targetModule);
        for (const auto& id : gSelectionRelay->mSelectedGates)
        {
            targetModule->assign_gate(gNetlist->get_gate_by_id(id));
        }
        for (const auto& id : gSelectionRelay->mSelectedModules)
        {
            gNetlist->get_module_by_id(id)->set_parent_module(targetModule);
        }

        auto gates   = gSelectionRelay->mSelectedGates;
        auto modules = gSelectionRelay->mSelectedModules;
        gSelectionRelay->clear();
        gSelectionRelay->relaySelectionChanged(this);
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
        Grouping* grp = gContentManager->getGroupingManagerWidget()->getModel()->addDefaultEntry();
        if (grp) selectionToGroupingInternal(grp);
    }

    void SelectionDetailsWidget::selectionToExistingGrouping()
    {
        const QAction* action = static_cast<const QAction*>(QObject::sender());
        QString grpName = action->text();
        if (grpName.startsWith(sAddToGrouping)) grpName.remove(0,sAddToGrouping.size());
        Grouping* grp =
                gContentManager->getGroupingManagerWidget()->getModel()->groupingByName(grpName);
        if (grp) selectionToGroupingInternal(grp);
    }

    void SelectionDetailsWidget::selectionToGroupingInternal(Grouping* grp)
    {
        for (u32 mid : gSelectionRelay->mSelectedModules)
        {
            Module* m = gNetlist->get_module_by_id(mid);
            if (m)
            {
                Grouping* mg = m->get_grouping();
                if (mg) mg->remove_module(m);
                grp->assign_module(m);
            }
        }
        for (u32 gid : gSelectionRelay->mSelectedGates)
        {
            Gate* g = gNetlist->get_gate_by_id(gid);
            if (g)
            {
                Grouping* gg = g->get_grouping();
                if (gg) gg->remove_gate(g);
                grp->assign_gate(g);
            }
        }
        for (u32 nid : gSelectionRelay->mSelectedNets)
        {
            Net* n = gNetlist->get_net_by_id(nid);
            if (n)
            {
                Grouping* ng = n->get_grouping();
                if (ng) ng->remove_net(n);
                grp->assign_net(n);
            }
        }
        gSelectionRelay->clear();
        gSelectionRelay->relaySelectionChanged(nullptr);
        canRestoreSelection();
    }

    void SelectionDetailsWidget::enableSearchbar(bool enable)
    {
        QString iconStyle = enable
                ? mSearchIconStyle
                : mDisabledIconStyle;
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(iconStyle, mSearchIconPath));
        if (!enable && mSearchbar->isVisible())
        {
            mSearchbar->hide();
            setFocus();
        }
        mSearchAction->setEnabled(enable);
    }

    void SelectionDetailsWidget::canRestoreSelection()
    {
        bool enable = mHistory->hasPreviousEntry();

        QString iconStyle = enable
                ? mSearchIconStyle
                : mDisabledIconStyle;

        mRestoreLastSelection->setIcon(gui_utility::getStyledSvgIcon(iconStyle, mRestoreIconPath));
        mRestoreLastSelection->setEnabled(enable);
    }

    void SelectionDetailsWidget::canMoveToModule(int nodes)
    {
        QString iconStyle = nodes > 0
                ? mToModuleIconStyle
                : mDisabledIconStyle;
        mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(iconStyle, mToModuleIconPath));
        mSelectionToModule->setEnabled(nodes > 0);
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
        proxy->handleFilterTextChanged(QString());
        handleFilterTextChanged(QString());

        mNumberSelectedItems = gSelectionRelay->mSelectedGates.size() + gSelectionRelay->mSelectedModules.size() + gSelectionRelay->mSelectedNets.size();
        QVector<const SelectionTreeItem*> defaultHighlight;

        if (mNumberSelectedItems)
        {
            // more than 1 item selected, populate and make mVisible
            mSelectionTreeView->populate(true);
            defaultHighlight.append(mSelectionTreeView->itemFromIndex());

            mHistory->storeCurrentSelection();
            canRestoreSelection();
            canMoveToModule(gSelectionRelay->mSelectedGates.size() + gSelectionRelay->mSelectedModules.size());
            enableSearchbar(true);
            mSelectionToGrouping->setEnabled(true);
            mSelectionToModule->setEnabled(true);
            mSelectionToGrouping->setIcon(gui_utility::getStyledSvgIcon(mToGroupingIconStyle, mToGroupingIconPath));
            mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(mToModuleIconStyle, mToModuleIconPath));
        }
        else
        {
            // nothing selected
            singleSelectionInternal(nullptr);
            // clear and hide tree
            mSelectionTreeView->populate(false);
            mHistory->emptySelection();
            canMoveToModule(0);
            enableSearchbar(false);
            mSelectionToGrouping->setDisabled(true);
            mSelectionToModule->setDisabled(true);
            mSelectionToGrouping->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToGroupingIconPath));
            mSelectionToModule->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mToModuleIconPath));

            return;
        }


        if (!gSelectionRelay->mSelectedModules.isEmpty())
        {
            SelectionTreeItemModule sti(*gSelectionRelay->mSelectedModules.begin());
            singleSelectionInternal(&sti);
        }
        else if (!gSelectionRelay->mSelectedGates.isEmpty())
        {
            SelectionTreeItemGate sti(*gSelectionRelay->mSelectedGates.begin());
            singleSelectionInternal(&sti);
        }
        else if (!gSelectionRelay->mSelectedNets.isEmpty())
        {
            SelectionTreeItemNet sti(*gSelectionRelay->mSelectedNets.begin());
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
        QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"),this);
        connect(search_shortcut, &QShortcut::activated, this, &SelectionDetailsWidget::toggleSearchbar);

        return (QList<QShortcut*>() << search_shortcut);
    }

    void SelectionDetailsWidget::toggleSearchbar()
    {
        if(mSearchbar->isHidden())
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

    void SelectionDetailsWidget::handleFilterTextChanged(const QString& filter_text)
    {
        if(filter_text.isEmpty())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
    }

    void SelectionDetailsWidget::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mRestoreLastSelection);
        toolbar->addAction(mSelectionToGrouping);
        toolbar->addAction(mSelectionToModule);
        toolbar->addAction(mSearchAction);
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

    QString SelectionDetailsWidget::restoreIconPath() const
    {
        return mRestoreIconPath;
    }

    QString SelectionDetailsWidget::restoreIconStyle() const
    {
        return mRestoreIconStyle;
    }

    void SelectionDetailsWidget::setRestoreIconPath(const QString& path)
    {
        mRestoreIconPath = path;
    }

    void SelectionDetailsWidget::setRestoreIconStyle(const QString& style)
    {
        mRestoreIconStyle = style;
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
