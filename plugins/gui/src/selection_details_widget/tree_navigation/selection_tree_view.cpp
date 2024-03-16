#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/gui_globals.h"
#include "gui/context_manager_widget/context_manager_widget.h"
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
        connect(this, &SelectionTreeView::focusItemClicked, this, &SelectionTreeView::handleTreeViewItemFocusClicked);
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

            if (item)
            {
                switch (item->getType())
                {
                    case ModuleItem::TreeItemType::Module:

                        menu.addAction(QIcon(":/icons/python"), "Extract Module as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_module_by_id(" + QString::number(item->id()) + ")");
                        });

                        menu.addAction("Isolate in new view", [this, item]() { Q_EMIT handleIsolationViewAction(item); });
                        if (mIsGrouping)
                            menu.addAction("Add to Selection", [this, item]() { Q_EMIT handleAddToSelection(item); });

                        break;
                    case ModuleItem::TreeItemType::Gate:

                        menu.addAction(QIcon(":/icons/python"), "Extract Gate as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + QString::number(item->id()) + ")");
                        });

                        menu.addAction("Isolate in new view", [this, item]() { Q_EMIT handleIsolationViewAction(item); });
                        if (mIsGrouping)
                            menu.addAction("Add to Selection", [this, item]() { Q_EMIT handleAddToSelection(item); });

                        break;
                    case ModuleItem::TreeItemType::Net:

                        menu.addAction(QIcon(":/icons/python"), "Extract Net as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_net_by_id(" + QString::number(item->id()) + ")");
                        });
                        if (mIsGrouping)
                            menu.addAction("Add to Selection", [this, item]() { Q_EMIT handleAddToSelection(item); });

                        break;
                    default:    // make compiler happy
                        break;
                }
            }

            menu.addAction("Focus item in Graph View", [this, item]() { Q_EMIT focusItemClicked(item); });

            menu.exec(viewport()->mapToGlobal(point));
        }
    }

    void SelectionTreeView::handleIsolationViewAction(const ModuleItem* sti)
    {
        Node nd;
        if (sti->getType() == ModuleItem::TreeItemType::Gate)
        {
            nd = Node(sti->id(),Node::Gate);
        }
        else if (sti->getType() == ModuleItem::TreeItemType::Module)
        {
            nd = Node(sti->id(),Node::Module);
        }
        else
        {
            return;
        }
        isolateInNewViewAction(nd);
    }

    void SelectionTreeView::handleAddToSelection(const ModuleItem* sti)
    {
        switch (sti->getType())
        {
            case ModuleItem::TreeItemType::Module:
            {
                gSelectionRelay->addModule(sti->id());
                break;
            }

            case ModuleItem::TreeItemType::Gate:
            {
                gSelectionRelay->addGate(sti->id());
                break;
            }

            case ModuleItem::TreeItemType::Net:
            {
                gSelectionRelay->addNet(sti->id());
                break;
            }

            default:
                // Unknown or invalid type
                return;
        }
        gSelectionRelay->relaySelectionChanged(this);
    }


    void SelectionTreeView::isolateInNewViewAction(Node nd)
    {
        QSet<u32> gateId;
        QSet<u32> moduleId;
        QString name;

        if (nd.type() == Node::Gate)
        {
            name = gGraphContextManager->nextViewName("Isolated View");
            gateId.insert(nd.id());
        }
        else if (nd.type() == Node::Module)
        {
            GraphContext* moduleContext =
                    gGraphContextManager->getContextByExclusiveModuleId(nd.id());
            if (moduleContext)
            {
                // open existing view
                gContentManager->getContextManagerWidget()->selectViewContext(moduleContext);
                gContentManager->getContextManagerWidget()->handleOpenContextClicked();
                return;
            }

            name = QString::fromStdString(gNetlist->get_module_by_id(nd.id())->get_name()) + " (ID: " + QString::number(nd.id()) + ")";
            moduleId.insert(nd.id());
        }
        else
        {
            return;
        }

        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::Context, name));
        act->addAction(new ActionAddItemsToObject(moduleId, gateId));
        act->exec();
        if (nd.type() == Node::Module)
        {
            GraphContext* moduleContext = gGraphContextManager->getContextById(act->object().id());
            moduleContext->setDirty(false);
            moduleContext->setExclusiveModuleId(nd.id());
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
