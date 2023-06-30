#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/gui_globals.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/user_action_compound.h"

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
        mSelectionTreeModel      = new SelectionTreeModel(this);
        mSelectionTreeProxyModel = new SelectionTreeProxyModel(this);
        mSelectionTreeProxyModel->setSourceModel(mSelectionTreeModel);
        setModel(mSelectionTreeProxyModel);
        setDefaultColumnWidth();
        header()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);

        mIsGrouping = isGrouping;

        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QTreeView::customContextMenuRequested, this, &SelectionTreeView::handleCustomContextMenuRequested);

        connect(this, &SelectionTreeView::itemDoubleClicked, this, &SelectionTreeView::handleTreeViewItemFocusClicked);
        connect(this, &SelectionTreeView::focusItemClicked, this, &SelectionTreeView::handleTreeViewItemFocusClicked);
    }

    void SelectionTreeView::setDefaultColumnWidth()
    {
        setColumnWidth(SelectionTreeModel::sNameColumn, 160);
        setColumnWidth(SelectionTreeModel::sIdColumn, 40);
        setColumnWidth(SelectionTreeModel::sTypeColumn, 80);
        header()->setStretchLastSection(true);
    }

    void SelectionTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);

        const SelectionTreeItem* sti = current.isValid() ? itemFromIndex(current) : nullptr;

        Q_EMIT triggerSelection(sti);
    }

    void SelectionTreeView::mouseDoubleClickEvent(QMouseEvent* event)
    {
        QPoint point = viewport()->mapFromGlobal(event->globalPos());
        ;

        QModelIndex index = indexAt(point);

        if (index.isValid())
        {
            SelectionTreeItem* item = itemFromIndex(index);
            Q_EMIT itemDoubleClicked(item);
        }
    }

    SelectionTreeItem* SelectionTreeView::itemFromIndex(const QModelIndex& index) const
    {
        // topmost element if no valid index given
        QModelIndex proxyIndex = index.isValid() ? index : mSelectionTreeProxyModel->index(0, 0, rootIndex());

        if (!proxyIndex.isValid())
            return nullptr;

        QModelIndex modelIndex = mSelectionTreeProxyModel->mapToSource(proxyIndex);
        return static_cast<SelectionTreeItem*>(modelIndex.internalPointer());
    }

    void SelectionTreeView::handleCustomContextMenuRequested(const QPoint& point)
    {
        QModelIndex index = indexAt(point);

        if (index.isValid())
        {
            QMenu menu;

            SelectionTreeItem* item = itemFromIndex(index);

            if (item)
            {
                switch (item->itemType())
                {
                    case SelectionTreeItem::TreeItemType::ModuleItem:

                        menu.addAction(QIcon(":/icons/python"), "Extract Module as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_module_by_id(" + QString::number(item->id()) + ")");
                        });

                        menu.addAction("Isolate in new view", [this, item]() { Q_EMIT handleIsolationViewAction(item); });
                        if (mIsGrouping)
                            menu.addAction("Add to Selection", [this, item]() { Q_EMIT handleAddToSelection(item); });

                        break;
                    case SelectionTreeItem::TreeItemType::GateItem:

                        menu.addAction(QIcon(":/icons/python"), "Extract Gate as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + QString::number(item->id()) + ")");
                        });

                        menu.addAction("Isolate in new view", [this, item]() { Q_EMIT handleIsolationViewAction(item); });
                        if (mIsGrouping)
                            menu.addAction("Add to Selection", [this, item]() { Q_EMIT handleAddToSelection(item); });

                        break;
                    case SelectionTreeItem::TreeItemType::NetItem:

                        menu.addAction(QIcon(":/icons/python"), "Extract Net as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_net_by_id(" + QString::number(item->id()) + ")");
                        });
                        if (mIsGrouping)
                            menu.addAction("Add to Selection", [this, item]() { Q_EMIT handleAddToSelection(item); });

                        break;
                    default:    // make compiler happy and handle irrelevant MaxItem, NullItem
                        break;
                }
            }

            menu.addAction("Focus item in Graph View", [this, item]() { Q_EMIT focusItemClicked(item); });

            menu.exec(viewport()->mapToGlobal(point));
        }
    }

    void SelectionTreeView::handleIsolationViewAction(const SelectionTreeItem* sti)
    {
        Node nd;
        if (sti->itemType() == SelectionTreeItem::TreeItemType::GateItem)
        {
            nd = Node(sti->id(),Node::Gate);
        }
        else if (sti->itemType() == SelectionTreeItem::TreeItemType::ModuleItem)
        {
            nd = Node(sti->id(),Node::Module);
        }
        else
        {
            return;
        }
        isolateInNewViewAction(nd);
    }

    void SelectionTreeView::handleAddToSelection(const SelectionTreeItem* sti)
    {
        // Abhängig vom Typ des TreeItems fügen wir unterschiedliche Elemente zur Auswahl hinzu.
        switch (sti->itemType())
        {
            case SelectionTreeItem::ModuleItem:
            {
                // Downcast auf Modul und hinzufügen zur Auswahl.
                const SelectionTreeItemModule* moduleItem = static_cast<const SelectionTreeItemModule*>(sti);
                gSelectionRelay->addModule(moduleItem->id());
                break;
            }

            case SelectionTreeItem::GateItem:
            {
                // Downcast auf Tor und hinzufügen zur Auswahl.
                const SelectionTreeItemGate* gateItem = static_cast<const SelectionTreeItemGate*>(sti);
                gSelectionRelay->addGate(gateItem->id());
                break;
            }

            case SelectionTreeItem::NetItem:
            {
                // Downcast auf Netz und hinzufügen zur Auswahl.
                const SelectionTreeItemNet* netItem = static_cast<const SelectionTreeItemNet*>(sti);
                gSelectionRelay->addNet(netItem->id());
                break;
            }

            default:
                // Ungültiger oder unbekannter Auswahltyp.
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
            u32 cnt = 0;
            for (;;)
            {
                ++cnt;
                name = "Isolated View " + QString::number(cnt);
                if (!gGraphContextManager->contextWithNameExists(name))
                    break;
            }
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
        if (mSelectionTreeProxyModel->isGraphicsBusy())
            return;
        setSelectionMode(QAbstractItemView::NoSelection);
        selectionModel()->clear();
        mSelectionTreeModel->fetchSelection(mVisible, groupingId);
        if (mVisible)
        {
            show();
            setSelectionMode(QAbstractItemView::SingleSelection);
            QModelIndex defaultSel = mSelectionTreeProxyModel->index(0, 0, rootIndex());
            if (defaultSel.isValid())
                selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        else
            hide();
    }

    void SelectionTreeView::handleFilterTextChanged(const QString& filter_text)
    {
        mSelectionTreeProxyModel->handleFilterTextChanged(filter_text);
        expandAll();
        QModelIndex defaultSel = mSelectionTreeProxyModel->index(0, 0, rootIndex());
        if (defaultSel.isValid())
            selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }

    SelectionTreeProxyModel* SelectionTreeView::proxyModel()
    {
        return mSelectionTreeProxyModel;
    }

    void SelectionTreeView::handleTreeViewItemFocusClicked(const SelectionTreeItem* sti)
    {
        u32 itemId = sti->id();

        switch (sti->itemType())
        {
            case SelectionTreeItem::TreeItemType::GateItem:
                gContentManager->getGraphTabWidget()->handleGateFocus(itemId);
                break;
            case SelectionTreeItem::TreeItemType::NetItem:
                gContentManager->getGraphTabWidget()->handleNetFocus(itemId);
                break;
            case SelectionTreeItem::TreeItemType::ModuleItem:
                gContentManager->getGraphTabWidget()->handleModuleFocus(itemId);
                break;
            default: break;
        }
    }

}    // namespace hal
