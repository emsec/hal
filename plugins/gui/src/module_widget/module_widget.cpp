#include "gui/module_widget/module_widget.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/user_action/action_rename_object.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "gui/module_model/module_model.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/module_context_menu/module_context_menu.h"
#include "gui/plugin_relay/gui_plugin_manager.h"

#include <QApplication>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMenu>
#include <QModelIndex>
#include <QRegExp>
#include <QScrollBar>
#include <QSet>
#include <QShortcut>
#include <QTreeView>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QClipboard>

namespace hal
{
    ModuleWidget::ModuleWidget(QWidget* parent)
        : ContentWidget("Modules", parent),
          mTreeView(new ModuleTreeView(this)),
          mSearchbar(new Searchbar(this)),
          mToggleNetsAction(new QAction(this)),
          mToggleGatesAction(new QAction(this)),
          mRenameAction(new QAction(this)),
          mDeleteAction(new QAction(this)),
          mToggleExpandTreeAction(new QAction(this)),
          mModuleProxyModel(new ModuleProxyModel(this))

    {
        ensurePolished();

        connect(mTreeView, &QTreeView::customContextMenuRequested, this, &ModuleWidget::handleTreeViewContextMenuRequested);

        mToggleNetsAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mHideNetsIconPath));
        mToggleGatesAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mHideGatesIconPath));
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
        mRenameAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mRenameIconPath));
        mToggleExpandTreeAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mExpandedIconPath));

        mToggleNetsAction->setToolTip("Toggle net visibility");
        mToggleGatesAction->setToolTip("Toggle gate visibility");
        mDeleteAction->setToolTip("Delete module");
        mSearchAction->setToolTip("Search");
        mRenameAction->setToolTip("Rename");
        mToggleExpandTreeAction->setToolTip("Toggle expand all / collapse all");

        mModuleModel = new ModuleModel(this);
        mModuleProxyModel->setSourceModel(mModuleModel);

        mTreeView->setModel(mModuleProxyModel);
        mTreeView->setDefaultColumnWidth();
        mTreeView->setSortingEnabled(true);
        mTreeView->sortByColumn(0, Qt::AscendingOrder);
        mTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
        mTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mTreeView->setFrameStyle(QFrame::NoFrame);
        //mTreeView->header()->close();
        mTreeView->setExpandsOnDoubleClick(false);
        mTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
        mContentLayout->addWidget(mTreeView);

        mSearchbar->setColumnNames(mModuleModel->headerLabels());
        mContentLayout->addWidget(mSearchbar);
        mSearchbar->hide();

        enableDeleteAction(false); // enable upon selected module
        mIgnoreSelectionChange = false;

        gSelectionRelay->registerSender(this, name());

        //connect(mSearchbar, &Searchbar::textEdited, this, &ModuleWidget::filter);
        connect(mTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ModuleWidget::handleTreeSelectionChanged);
        connect(mTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &ModuleWidget::handleCurrentChanged);
        connect(mTreeView, &ModuleTreeView::doubleClicked, this, &ModuleWidget::handleItemDoubleClicked);
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &ModuleWidget::handleSelectionChanged, Qt::QueuedConnection);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleRemoved, this, &ModuleWidget::handleModuleRemoved);

        connect(mSearchAction, &QAction::triggered, this, &ModuleWidget::toggleSearchbar);

        connect(mSearchbar, &Searchbar::triggerNewSearch, this, &ModuleWidget::updateSearchIcon);
        connect(mSearchbar, &Searchbar::triggerNewSearch, mModuleProxyModel, &ModuleProxyModel::startSearch);
		
        mShortCutDeleteItem = new QShortcut(ContentManager::sSettingDeleteItem->value().toString(), this);
        mShortCutDeleteItem->setEnabled(false);

        connect(ContentManager::sSettingDeleteItem, &SettingsItemKeybind::keySequenceChanged, mShortCutDeleteItem, &QShortcut::setKey);
        connect(mShortCutDeleteItem, &QShortcut::activated, this, &ModuleWidget::deleteSelectedItem);
        connect(mDeleteAction, &QAction::triggered, this, &ModuleWidget::deleteSelectedItem);

        connect(qApp, &QApplication::focusChanged, this, &ModuleWidget::handleDeleteShortcutOnFocusChanged);


        connect(mToggleNetsAction, &QAction::triggered, this, &ModuleWidget::handleToggleNetsClicked);
        connect(mToggleGatesAction, &QAction::triggered, this, &ModuleWidget::handleToggleGatesClicked);
        connect(mToggleExpandTreeAction, &QAction::triggered, this, &ModuleWidget::handleToggleExpandTreeClicked);
        connect(mRenameAction, &QAction::triggered, this, &ModuleWidget::handleRenameClicked);

        mModuleModel->populateTree({gNetlist->get_top_module()->get_id()});
        mTreeView->expandAllModules();
    }

    void ModuleWidget::enableDeleteAction(bool enable)
    {
        mDeleteAction->setEnabled(enable);
        mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(enable?mActiveIconStyle:mDisabledIconStyle, mDeleteIconPath));
    }

    void ModuleWidget::handleToggleNetsClicked()
    {
        if(mModuleProxyModel->toggleFilterNets())
        {
            mToggleNetsAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mHideNetsIconPath));
        }
        else
        {
            mToggleNetsAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mShowNetsIconPath));
        }
    }

    void ModuleWidget::handleToggleGatesClicked()
    {
        if(mModuleProxyModel->toggleFilterGates())
        {
            mToggleGatesAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mHideGatesIconPath));
        }
        else
        {
            mToggleGatesAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mShowGatesIconPath));
        }
    }

    void ModuleWidget::handleToggleExpandTreeClicked()
    {
        if (mTreeView->toggleStateExpanded())
        {
            mTreeView->collapseAllModules();
            mToggleExpandTreeAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mCollapsedIconPath));
        }
        else
        {
            mTreeView->expandAllModules();
            mToggleExpandTreeAction->setIcon(gui_utility::getStyledSvgIcon(mActiveIconStyle, mExpandedIconPath));
        }
    }

    void ModuleWidget::handleRenameClicked()
    {
        ModuleItem* item = getModuleItemFromIndex(mTreeView->currentIndex());
        gNetlistRelay->changeElementNameDialog(item->getType(), item->id());
    }

    void ModuleWidget::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mToggleNetsAction);
        toolbar->addAction(mToggleGatesAction);
        toolbar->addAction(mRenameAction);
        toolbar->addAction(mDeleteAction);
        toolbar->addAction(mSearchAction);
        toolbar->addAction(mToggleExpandTreeAction);
    }

    QList<QShortcut*> ModuleWidget::createShortcuts()
    {
        mSearchShortcut = new QShortcut(mSearchKeysequence, this);
        connect(mSearchShortcut, &QShortcut::activated, mSearchAction, &QAction::trigger);

        QList<QShortcut*> list;
        list.append(mSearchShortcut);

        return list;
    }

    void ModuleWidget::toggleSearchbar()
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

    void ModuleWidget::filter(const QString& text)
    {
        QRegularExpression* regex = new QRegularExpression(text);
        if (regex->isValid())
        {
            mModuleProxyModel->setFilterRegularExpression(*regex);
            mTreeView->expandAllModules();
            QString output = "navigation regular expression '" + text + "' entered.";
            log_info("user", output.toStdString());
        }
    }

    void ModuleWidget::handleTreeViewContextMenuRequested(const QPoint& point)
    {
        QModelIndex index = mTreeView->indexAt(point);
        if (!index.isValid())
            return;

        ModuleItem* item = getModuleItemFromIndex(index);
        ModuleItem::TreeItemType type = item->getType();
        u32 id = item->id();

        QMenu context_menu;

        if(type == ModuleItem::TreeItemType::Module)
            ModuleContextMenu::addModuleSubmenu(&context_menu, id);
        else if(type == ModuleItem::TreeItemType::Gate)
            ModuleContextMenu::addGateSubmenu(&context_menu, id);
        else if(type == ModuleItem::TreeItemType::Net)
            ModuleContextMenu::addNetSubmenu(&context_menu, id);

        GuiPluginManager::addPluginSubmenus(&context_menu, gNetlist, 
            type==ModuleItem::TreeItemType::Module ? std::vector<u32>({id}) : std::vector<u32>(),
            type==ModuleItem::TreeItemType::Gate ? std::vector<u32>({id}) : std::vector<u32>(),
            type==ModuleItem::TreeItemType::Net ? std::vector<u32>({id}) : std::vector<u32>());

        context_menu.exec(mTreeView->viewport()->mapToGlobal(point));
    }

    void ModuleWidget::handleModuleRemoved(Module* module, u32 module_id)
    {
        UNUSED(module);
        UNUSED(module_id);

        //prevents the execution of "handleTreeSelectionChanged"
        //when a module is (re)moved the corresponding item in the tree is deleted and deselected, thus also triggering "handleTreeSelectionChanged"
        //this call due to the selection model signals is unwanted behavior because "handleTreeSelectionChanged" is ment to only react to an "real" action performed by the user on the tree itself
        mIgnoreSelectionChange = true;
    }

    void ModuleWidget::handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);
        ModuleItem* mi = getModuleItemFromIndex(current);
        if (!mi)
            enableDeleteAction(false);
        else
            enableDeleteAction(mi->getType() == ModuleItem::TreeItemType::Module);
    }

    void ModuleWidget::handleTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
    {
        Q_UNUSED(selected)
        Q_UNUSED(deselected)

        if (mIgnoreSelectionChange || mModuleModel->isModifying())
            return;

        gSelectionRelay->clear();

        QModelIndexList current_selection = mTreeView->selectionModel()->selectedIndexes();

        QSet<int> selectedMods;
        QSet<int> selectedGats;
        for (const auto& index : current_selection)
        {
            ModuleItem* mi = getModuleItemFromIndex(index);
            switch (mi->getType()) {
            case ModuleItem::TreeItemType::Module:
                selectedMods.insert(mi->id());
                gSelectionRelay->addModule(mi->id());
                break;
            case ModuleItem::TreeItemType::Gate:
                selectedGats.insert(mi->id());
                gSelectionRelay->addGate(mi->id());
                break;
            case ModuleItem::TreeItemType::Net:
                gSelectionRelay->addNet(mi->id());
                break;
            }
        }

        if (selectedMods.size() == 1)
        {
            gSelectionRelay->setFocus(SelectionRelay::ItemType::Module, *selectedMods.begin());
            enableDeleteAction(true);
        }
        else if (selectedGats.size() == 1)
        {
            gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate, *selectedGats.begin());
            enableDeleteAction(false);
        }
        else
        {
            enableDeleteAction(false);
        }

        gSelectionRelay->relaySelectionChanged(this);
    }

    void ModuleWidget::handleItemDoubleClicked(const QModelIndex& index)
    {
        ModuleItem* mi = getModuleItemFromIndex(index);
        switch(mi->getType()){
            case ModuleItem::TreeItemType::Module: gGraphContextManager->openModuleInView(mi->id(), false); break;
            case ModuleItem::TreeItemType::Gate: gGraphContextManager->openGateInView(mi->id()); break;
            case ModuleItem::TreeItemType::Net: gGraphContextManager->openNetEndpointsInView(mi->id()); break;
        }
    }

    void ModuleWidget::handleSelectionChanged(void* sender)
    {
        if (sender == this)
            return;

        mIgnoreSelectionChange = true;

        QItemSelection module_selection;

        for (auto module_id : gSelectionRelay->selectedModulesList())
        {
            for(ModuleItem* item : mModuleModel->getItems(module_id))
            {
                if(item)
                {
                    QModelIndex index = mModuleProxyModel->mapFromSource(mModuleModel->getIndexFromItem(item));
                    module_selection.select(index, index);
                }
            }
        }

        mTreeView->selectionModel()->select(module_selection, QItemSelectionModel::SelectionFlag::ClearAndSelect);

        mIgnoreSelectionChange = false;
    }

    ModuleItem* ModuleWidget::getModuleItemFromIndex(const QModelIndex& index)
    {
        return mModuleModel->getItem(mModuleProxyModel->mapToSource(index));
    }

    void ModuleWidget::updateSearchIcon()
    {
        if (mSearchbar->filterApplied() && mSearchbar->isVisible())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
    }

    ModuleModel* ModuleWidget::getModuleModel() const
    {
        return mModuleModel;
    }

    QString ModuleWidget::disabledIconStyle() const
    {
        return mDisabledIconStyle;
    }

    QString ModuleWidget::activeIconStyle() const
    {
        return mActiveIconStyle;
    }

    ModuleProxyModel* ModuleWidget::proxyModel()
    {
        return mModuleProxyModel;
    }

    QString ModuleWidget::showNetsIconPath() const
    {
        return mShowNetsIconPath;
    }

    QString ModuleWidget::hideNetsIconPath() const
    {
        return mHideNetsIconPath;
    }

    QString ModuleWidget::showGatesIconPath() const
    {
        return mShowGatesIconPath;
    }

    QString ModuleWidget::hideGatesIconPath() const
    {
        return mHideGatesIconPath;
    }

    QString ModuleWidget::searchIconPath() const
    {
        return mSearchIconPath;
    }

    QString ModuleWidget::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    QString ModuleWidget::searchActiveIconStyle() const
    {
        return mSearchActiveIconStyle;
    }

    QString ModuleWidget::deleteIconPath() const
    {
        return mDeleteIconPath;
    }

    QString ModuleWidget::renameIconPath() const
    {
        return  mRenameIconPath;
    }

    QString ModuleWidget::expandedIconPath() const
    {
        return  mExpandedIconPath;
    }

    QString ModuleWidget::collapsedIconPath() const
    {
        return mCollapsedIconPath;
    }

    void ModuleWidget::setDisabledIconStyle(const QString& style)
    {
        mDisabledIconStyle = style;
    }

    void ModuleWidget::setActiveIconStyle(const QString& style)
    {
        mActiveIconStyle = style;
    }

    void ModuleWidget::setShowNetsIconPath(const QString& path)
    {
        mShowNetsIconPath = path;
    }

    void ModuleWidget::setHideNetsIconPath(const QString& path)
    {
        mHideNetsIconPath = path;
    }

    void ModuleWidget::setShowGatesIconPath(const QString& path)
    {
        mShowGatesIconPath = path;
    }

    void ModuleWidget::setHideGatesIconPath(const QString& path)
    {
        mHideGatesIconPath = path;
    }

    void ModuleWidget::setSearchIconPath(const QString& path)
    {
        mSearchIconPath = path;
    }

    void ModuleWidget::setSearchIconStyle(const QString& style)
    {
        mSearchIconStyle = style;
    }

    void ModuleWidget::setSearchActiveIconStyle(const QString& style)
    {
        mSearchActiveIconStyle = style;
    }

    void ModuleWidget::setDeleteIconPath(const QString& path)
    {
        mDeleteIconPath = path;
    }

    void ModuleWidget::setRenameIconPath(const QString& path)
    {
        mRenameIconPath = path;
    }

    void ModuleWidget::setExpandedIconPath(const QString& path)
    {
        mExpandedIconPath = path;
    }

    void ModuleWidget::setCollapsedIconPath(const QString& path)
    {
        mCollapsedIconPath = path;
    }

    void ModuleWidget::deleteSelectedItem()
    {
        if(!mTreeView->currentIndex().isValid())
        {
            return;
        }

        ModuleItem* selectedItem = getModuleItemFromIndex(mTreeView->currentIndex());
        if(selectedItem->getParent() != nullptr)
        {
            if (getModuleItemFromIndex(mTreeView->currentIndex())->getType() == ModuleItem::TreeItemType::Module)
            {
                auto module = gNetlist->get_module_by_id(selectedItem->id());
                if(!module->is_top_module())
                    gNetlistRelay->deleteModule(getModuleItemFromIndex(mTreeView->currentIndex())->id());
            }
        }
    }

    void ModuleWidget::handleDeleteShortcutOnFocusChanged(QWidget* oldWidget, QWidget* newWidget)
    {
        Q_UNUSED(oldWidget);
        if(!newWidget) return;
        if(newWidget->parent() == this)
        {
            mShortCutDeleteItem->setEnabled(true);
            return;
        }
        else
        {
            mShortCutDeleteItem->setEnabled(false);
            return;
        }
    }
}
