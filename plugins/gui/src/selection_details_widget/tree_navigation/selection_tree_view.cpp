#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"
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
    SelectionTreeView::SelectionTreeView(QWidget* parent) : QTreeView(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mSelectionTreeModel      = new SelectionTreeModel(this);
        mSelectionTreeProxyModel = new SelectionTreeProxyModel(this);
        mSelectionTreeProxyModel->setSourceModel(mSelectionTreeModel);
        setModel(mSelectionTreeProxyModel);
        setDefaultColumnWidth();
        header()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);

        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QTreeView::customContextMenuRequested, this, &SelectionTreeView::handleCustomContextMenuRequested);
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

                        break;
                    case SelectionTreeItem::TreeItemType::GateItem:

                        menu.addAction(QIcon(":/icons/python"), "Extract Gate as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + QString::number(item->id()) + ")");
                        });

                        menu.addAction("Isolate in new view", [this, item]() { Q_EMIT handleIsolationViewAction(item); });

                        break;
                    case SelectionTreeItem::TreeItemType::NetItem:

                        menu.addAction(QIcon(":/icons/python"), "Extract Net as python code (copy to clipboard)", [item]() {
                            QApplication::clipboard()->setText("netlist.get_net_by_id(" + QString::number(item->id()) + ")");
                        });

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
        u32 id       = sti->id();
        QString name = "";
        QSet<u32> gateId;
        QSet<u32> moduleId;

        if (sti->itemType() == SelectionTreeItem::TreeItemType::GateItem)
        {
            name = "Isolated Gate(ID: " + QString::number(id) + ")";
            gateId.insert(id);
        }
        else if (sti->itemType() == SelectionTreeItem::TreeItemType::ModuleItem)
        {
            name = "Isolated Module(ID: " + QString::number(id) + ")";
            moduleId.insert(id);
        }
        else
        {
            return;
        }

        u32 cnt = 0;
        while (true)
        {
            ++cnt;
            QString contextName = name + " " + QString::number(cnt);
            if (!gGraphContextManager->contextWithNameExists(contextName))
            {
                UserActionCompound* act = new UserActionCompound;
                act->setUseCreatedObject();
                act->addAction(new ActionCreateObject(UserActionObjectType::Context,
                                                      contextName));
                act->addAction(new ActionAddItemsToObject(moduleId, gateId));
                act->exec();
                return;
            }
        }
    }

    void SelectionTreeView::populate(bool mVisible)
    {
        if (mSelectionTreeProxyModel->isGraphicsBusy())
            return;
        setSelectionMode(QAbstractItemView::NoSelection);
        selectionModel()->clear();
        mSelectionTreeModel->fetchSelection(mVisible);
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

}    // namespace hal
