#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"

namespace hal {
    SelectionTreeView::SelectionTreeView(QWidget *parent)
        : QTreeView(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_selectionTreeModel = new SelectionTreeModel(this);
        m_selectionTreeProxyModel = new SelectionTreeProxyModel(this);
        m_selectionTreeProxyModel->setSourceModel(m_selectionTreeModel);
        setModel(m_selectionTreeProxyModel);
        setDefaultColumnWidth();
        header()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);

        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QTreeView::customContextMenuRequested, this, &SelectionTreeView::handle_custom_context_menu_requested);
    }

    void SelectionTreeView::setDefaultColumnWidth()
    {
        setColumnWidth(SelectionTreeModel::NAME_COLUMN, 160);
        setColumnWidth(SelectionTreeModel::ID_COLUMN, 40);
        setColumnWidth(SelectionTreeModel::TYPE_COLUMN, 80);
        header()->setStretchLastSection(true);
//        header()->setSectionResizeMode(SelectionTreeModel::NAME_COLUMN, QHeaderView::Interactive);
//        header()->setSectionResizeMode(SelectionTreeModel::ID_COLUMN, QHeaderView::Interactive);
//        header()->setSectionResizeMode(SelectionTreeModel::TYPE_COLUMN, QHeaderView::Interactive);
    }

    void SelectionTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);

        const SelectionTreeItem* sti = current.isValid()
                ? itemFromIndex(current)
                : nullptr;

        Q_EMIT triggerSelection(sti);
    }


    SelectionTreeItem* SelectionTreeView::itemFromIndex(const QModelIndex& index) const
    {
        // topmost element if no valid index given
        QModelIndex proxyIndex = index.isValid()
                ? index
                : m_selectionTreeProxyModel->index(0,0,rootIndex());

        if (!proxyIndex.isValid()) return nullptr;

        QModelIndex modelIndex = m_selectionTreeProxyModel->mapToSource(proxyIndex);
        return static_cast<SelectionTreeItem*>(modelIndex.internalPointer());
    }

    void SelectionTreeView::handle_custom_context_menu_requested(const QPoint& point)
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
                case SelectionTreeItem::itemType_t::ModuleItem:

                    menu.addAction(QIcon(":/icons/python"), "Extract Module as python code (copy to clipboard)",[item](){
                        QApplication::clipboard()->setText("netlist.get_module_by_id(" + QString::number(item->id()) + ")");
                    });

                    break;
                case SelectionTreeItem::itemType_t::GateItem:

                    menu.addAction(QIcon(":/icons/python"), "Extract Gate as python code (copy to clipboard)",[item](){
                        QApplication::clipboard()->setText("netlist.get_gate_by_id(" + QString::number(item->id()) + ")");
                    });

                    break;
                case SelectionTreeItem::itemType_t::NetItem:

                    menu.addAction(QIcon(":/icons/python"), "Extract Net as python code (copy to clipboard)",[item](){
                        QApplication::clipboard()->setText("netlist.get_net_by_id(" + QString::number(item->id()) + ")");
                    });

                    break;
                }
            }

            menu.exec(viewport()->mapToGlobal(point));
        }
    }

    void SelectionTreeView::populate(bool visible)
    {
        if (m_selectionTreeProxyModel->isGraphicsBusy()) return;
        setSelectionMode(QAbstractItemView::NoSelection);
        selectionModel()->clear();
        m_selectionTreeModel->fetchSelection(visible);
        if (visible)
        {
            show();
            setSelectionMode(QAbstractItemView::SingleSelection);
            QModelIndex defaultSel = m_selectionTreeProxyModel->index(0,0,rootIndex());
            if (defaultSel.isValid())
                selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        else
            hide();
    }

    void SelectionTreeView::handle_filter_text_changed(const QString& filter_text)
    {
        m_selectionTreeProxyModel->handle_filter_text_changed(filter_text);
        QModelIndex defaultSel = m_selectionTreeProxyModel->index(0,0,rootIndex());
        if (defaultSel.isValid())
            selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}
