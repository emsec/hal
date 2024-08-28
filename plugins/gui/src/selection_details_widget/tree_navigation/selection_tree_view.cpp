#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/gui_globals.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/module_context_menu/module_context_menu.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/user_action_compound.h"
#include "hal_core/netlist/grouping.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>

namespace hal
{
    SelectionTreeView::SelectionTreeView(QWidget* parent, bool isGrouping) : QTreeView(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setDefaultColumnWidth();
        header()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);

        mIsGrouping = isGrouping;

        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QTreeView::customContextMenuRequested, this, &SelectionTreeView::handleCustomContextMenuRequested);

        connect(this, &SelectionTreeView::itemDoubleClicked, this, &SelectionTreeView::handleTreeViewItemFocusClicked);
        connect(gNetlistRelay->getModuleColorManager(),&ModuleColorManager::moduleColorChanged,this,&SelectionTreeView::handleModuleColorChanged);
    }

    void SelectionTreeView::setDefaultColumnWidth()
    {
        setColumnWidth(0, 160);
        setColumnWidth(1, 40);
        setColumnWidth(2, 80);
        header()->setStretchLastSection(true);
    }

    void SelectionTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);

        const ModuleItem* sti = current.isValid() ? itemFromIndex(current) : nullptr;

        Q_EMIT triggerSelection(sti);
    }

    void SelectionTreeView::mouseDoubleClickEvent(QMouseEvent* event)
    {
        QPoint point = viewport()->mapFromGlobal(event->globalPos());
        ;

        QModelIndex index = indexAt(point);

        if (index.isValid())
        {
            ModuleItem* item = itemFromIndex(index);
            Q_EMIT itemDoubleClicked(item);
        }
    }

    ModuleItem* SelectionTreeView::itemFromIndex(const QModelIndex& index) const
    {
        SelectionTreeProxyModel* treeProxy = dynamic_cast<SelectionTreeProxyModel*>(model());
        if (!treeProxy) return nullptr;

        // topmost element if no valid index given
        QModelIndex proxyIndex = index.isValid() ? index : treeProxy->index(0, 0, rootIndex());

        if (!proxyIndex.isValid())
            return nullptr;

        QModelIndex modelIndex = treeProxy->mapToSource(proxyIndex);
        return static_cast<ModuleItem*>(modelIndex.internalPointer());
    }

    void SelectionTreeView::handleModuleColorChanged(u32 id)
    {
        Q_UNUSED(id);
        update();
    }

    void SelectionTreeView::handleCustomContextMenuRequested(const QPoint& point)
    {
        QModelIndex index = indexAt(point);

        if (index.isValid())
        {
            QMenu menu;

            ModuleItem* item = itemFromIndex(index);
            u32 id = item->id();
            ModuleItem::TreeItemType type = item->getType();

            if(type == ModuleItem::TreeItemType::Module)
                ModuleContextMenu::addModuleSubmenu(&menu, id);
            else if(type == ModuleItem::TreeItemType::Gate)
                ModuleContextMenu::addGateSubmenu(&menu, id);
            else if(type == ModuleItem::TreeItemType::Net)
                ModuleContextMenu::addNetSubmenu(&menu, id);

            GuiPluginManager::addPluginSubmenus(&menu, gNetlist, 
                type==ModuleItem::TreeItemType::Module ? std::vector<u32>({id}) : std::vector<u32>(),
                type==ModuleItem::TreeItemType::Gate ? std::vector<u32>({id}) : std::vector<u32>(),
                type==ModuleItem::TreeItemType::Net ? std::vector<u32>({id}) : std::vector<u32>());

            menu.exec(viewport()->mapToGlobal(point));
        }
    }

    void SelectionTreeView::populate(bool mVisible, u32 groupingId)
    {
        SelectionTreeProxyModel* treeProxy = dynamic_cast<SelectionTreeProxyModel*>(model());
        if (!treeProxy) return;
        ModuleModel* treeModel = dynamic_cast<ModuleModel*>(treeProxy->sourceModel());
        if (!treeModel) return;

        if (treeProxy->isGraphicsBusy())
            return;
        setSelectionMode(QAbstractItemView::NoSelection);
        selectionModel()->clear();

        if (!groupingId)
        {
            if(mVisible)
            {
                QVector<u32> modIds = QVector<u32>::fromList(gSelectionRelay->selectedModulesList());
                QVector<u32> gateIds = QVector<u32>::fromList(gSelectionRelay->selectedGatesList());
                QVector<u32> netIds = QVector<u32>::fromList(gSelectionRelay->selectedNetsList());
                treeModel->populateTree(modIds, gateIds, netIds);
            }
            else treeModel->clear();
        }
        else
        {
            Grouping* grouping = gNetlist->get_grouping_by_id(groupingId);
            QVector<u32> modIds = QVector<u32>::fromStdVector(grouping->get_module_ids());
            QVector<u32> gateIds = QVector<u32>::fromStdVector(grouping->get_gate_ids());
            QVector<u32> netIds = QVector<u32>::fromStdVector(grouping->get_net_ids());
            treeModel->populateTree(modIds, gateIds, netIds);
        }

        if (mVisible)
        {
            show();
            setSelectionMode(QAbstractItemView::SingleSelection);
            QModelIndex defaultSel = treeProxy->index(0, 0, rootIndex());
            if (defaultSel.isValid())
                selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        else
            hide();
    }
/*
    void SelectionTreeView::handleFilterTextChanged(const QString& filter_text)
    {
        SelectionTreeProxyModel* treeProxy = dynamic_cast<SelectionTreeProxyModel*>(model());
        if (!treeProxy) return;
        treeProxy->handleFilterTextChanged(filter_text);
        expandAll();
        QModelIndex defaultSel = treeProxy->index(0, 0, rootIndex());
        if (defaultSel.isValid())
            selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }*/

    SelectionTreeProxyModel* SelectionTreeView::proxyModel()
    {
        return dynamic_cast<SelectionTreeProxyModel*>(model());
    }

    void SelectionTreeView::handleTreeViewItemFocusClicked(const ModuleItem* sti)
    {
        u32 itemId = sti->id();

        switch (sti->getType())
        {
            case ModuleItem::TreeItemType::Module:
                gContentManager->getGraphTabWidget()->handleModuleFocus(itemId);
                break;
            case ModuleItem::TreeItemType::Gate:
                gContentManager->getGraphTabWidget()->handleGateFocus(itemId);
                break;
            case ModuleItem::TreeItemType::Net:
                gContentManager->getGraphTabWidget()->handleNetFocus(itemId);
                break;
            default: break;
        }
    }

}    // namespace hal
